#pragma once

#include <cassert>
#include <chrono>
#include <cstring>
#include <iostream>

#include "defaults.h"
#include "wsg_command_message.h"
#include "wsg_command_sender.h"
#include "wsg_return_message.h"
#include "wsg_return_receiver.h"

namespace schunk_driver {

/// A convenience class for sending and receiving WSG messages.
class Wsg {
 public:
  Wsg(const char* local_addr, in_port_t local_port,
      const char* gripper_addr, in_port_t gripper_port)
      : rx_(local_addr, local_port, gripper_addr, gripper_port),
        tx_(local_addr, local_port, gripper_addr, gripper_port) {
  }

  Wsg() : Wsg(nullptr, kLocalPort, kGripperAddrStr, kGripperPort) {}

  /** Sends @p command and waits for at least @p timeout for a response.
   * @return that response, or nullptr if no response arrived in time. */
  std::unique_ptr<WsgReturnMessage> SendAndAwaitResponse(
      const WsgCommandMessage& command,
      double timeout) {
    std::cout << "sending " << command.command()
              << " and awaiting for " << timeout << " seconds." << std::endl;
    auto start_time = std::chrono::system_clock::now();
    tx_.Send(command);
    std::unique_ptr<WsgReturnMessage> response(nullptr);
    while (!response) {
      auto now = std::chrono::system_clock::now();
      if (std::chrono::duration_cast<std::chrono::milliseconds>(
              now - start_time).count() > (timeout * 1e3)) {
        break;
      }
      response = rx_.Receive();
      if (response) {
        if (response->command() != command.command()) {
          response.reset(nullptr);  // Throw away irrelevant message.
        } else if (response->status() == E_CMD_PENDING) {
          response.reset(nullptr);  // Wait for a final status message.
        }
      }
    }
    return(response);
  }

  enum HomeDirection { kDefault, kPositive, kNegative };
  bool Home(HomeDirection dir) {
    WsgCommandMessage command(schunk_driver::kHome, {dir});
    auto response = SendAndAwaitResponse(command, 4);
    return response && (response->status() == E_SUCCESS);
  }

  bool Tare() {
    WsgCommandMessage command(schunk_driver::kTareForceSensor, {});
    auto response = SendAndAwaitResponse(command, 4);
    // Allow E_NOT_AVAILABLE because it is not clear in the docs if the
    // internal current-based force sensor counts or not.
    return response && ((response->status() == E_SUCCESS) ||
                        (response->status() == E_NOT_AVAILABLE));
  }

  bool Grasp(float width_mm, float speed_mm_per_s) {
    std::vector<unsigned char> params(8);
    memcpy(params.data(), &width_mm, sizeof(float));
    memcpy(params.data() + sizeof(float), &speed_mm_per_s, sizeof(float));
    WsgCommandMessage command(schunk_driver::kGrasp, params);
    auto response = SendAndAwaitResponse(command, 6);
    return response && (response->status() == E_SUCCESS);
  }

  /// All of the GetWhatever commands have the same payload format, so this
  /// convenience function will update any of them to kUpdatePeriodMs.
  void TurnOnUpdates(Command command,
                     uint16_t update_period_ms, double timeout) {
    std::vector<unsigned char> payload;
    WsgCommandMessage message(command, payload);
    // Here, 1 == always send automatic updates.
    message.AppendToPayload(static_cast<unsigned char>(1));
    message.AppendToPayload(update_period_ms);
    assert(SendAndAwaitResponse(message, timeout));
  }

  WsgReturnReceiver& rx() { return rx_; }
  WsgCommandSender& tx() { return tx_; }

 private:
  WsgReturnReceiver rx_;
  WsgCommandSender tx_;
};

}  // namespace schunk_driver
