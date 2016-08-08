#pragma once

#include <chrono>
#include <cstring>

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

  bool Grasp(float width_mm, float speed_mm_per_s) {
    std::vector<unsigned char> params(8);
    memcpy(params.data(), &width_mm, sizeof(float));
    memcpy(params.data() + sizeof(float), &speed_mm_per_s, sizeof(float));
    WsgCommandMessage command(schunk_driver::kGrasp, params);
    auto response = SendAndAwaitResponse(command, 6);
    return response && (response->status() == E_SUCCESS);
  }

 private:
  WsgReturnReceiver rx_;
  WsgCommandSender tx_;
};

}  // namespace schunk_driver
