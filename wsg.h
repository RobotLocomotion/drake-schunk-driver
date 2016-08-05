#pragma once

#include <chrono>

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
      // TODO(ggould-tri) discard received that don't match command?
    }
    return(response);
  }

 private:
  WsgReturnReceiver rx_;
  WsgCommandSender tx_;
};

}  // namespace schunk_driver
