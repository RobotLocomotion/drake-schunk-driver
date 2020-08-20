#pragma once

#include <chrono>
#include <cstring>
#include <iostream>

#include "defaults.h"
#include "wsg_command_message.h"
#include "wsg_command_sender.h"
#include "wsg_return_message.h"
#include "wsg_return_receiver.h"

namespace schunk_driver {

struct SystemInfo {
  uint8_t type_{0};
  uint8_t hwrev_{0};
  uint16_t fw_version_{0};
  uint32_t serial_number_{0};
};

struct PhysicalLimits {
  float stroke_mm_ {0};
  float min_speed_mm_per_s_ {0};
  float max_speed_mm_per_s_ {0};
  float min_acc_mm_per_ss_ {0};
  float max_acc_mm_per_ss_ {0};
  float min_force_ {0};
  float nominal_force_ {0};
  float overdrive_force_ {0};
};

/// A convenience class for sending and receiving WSG messages.  There is no
/// guiding principle to the methods here; it is just the methods we happened
/// to need during development.
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
#ifdef DEBUG
    std::cout << "sending " << command.command()
              << " and awaiting for " << timeout << " seconds." << std::endl;
#endif
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
        } else {
          if (response->status() != E_SUCCESS) {
            std::cerr << "Non-success response "
                      << response->status() << std::endl;
          }
        }
      }
    }
    return(response);
  }

  /// Issues a stop command and does not await a response.
  void Stop() {
    tx_.Send(WsgCommandMessage(kStop, {}));
  }

  SystemInfo GetSystemInfo() {
    auto info_msg = SendAndAwaitResponse(
      WsgCommandMessage(kGetSystemInfo, {}), 0.1);
    if (!info_msg) {
      throw std::runtime_error("Getting system info failed.");
    }

    SystemInfo result;
    auto info_data = info_msg->params().data();
    memcpy(&result.type_, info_data + 0, sizeof(uint8_t));
    memcpy(&result.hwrev_, info_data + 1, sizeof(uint8_t));
    memcpy(&result.fw_version_, info_data + 2, sizeof(uint16_t));
    memcpy(&result.serial_number_, info_data + 4, sizeof(uint32_t));

    std::cout << "System info:\n";
    std::cout << "type: " << static_cast<int>(result.type_) << " ";
    std::cout << "hwrev: " << static_cast<int>(result.hwrev_) << " ";
    std::cout << "fw_version: 0x" << std::hex << result.fw_version_
              << std::dec << " ";
    std::cout << "serial: " << result.serial_number_ << "\n";
    return result;
  }

  PhysicalLimits GetPhysicalLimits() {
    auto limits_msg = SendAndAwaitResponse(
      WsgCommandMessage(kGetSystemLimits, {}), 0.1);

    if (!limits_msg) {
      throw std::runtime_error("Getting physical limits failed.");
    }

    PhysicalLimits result;
    auto limits_data = limits_msg->params().data();
    memcpy(&result.stroke_mm_, limits_data + 0, sizeof(float));
    memcpy(&result.min_speed_mm_per_s_, limits_data + 4, sizeof(float));
    memcpy(&result.max_speed_mm_per_s_, limits_data + 8, sizeof(float));
    memcpy(&result.min_acc_mm_per_ss_, limits_data + 12, sizeof(float));
    memcpy(&result.max_acc_mm_per_ss_, limits_data + 16, sizeof(float));
    memcpy(&result.min_force_, limits_data + 20, sizeof(float));
    memcpy(&result.nominal_force_, limits_data + 24, sizeof(float));
    memcpy(&result.overdrive_force_, limits_data + 28, sizeof(float));

    std::cout << "Physical limits:\n";
    std::cout << "stroke: " << result.stroke_mm_ << "\n";
    std::cout << "min_speed: " << result.min_speed_mm_per_s_ << " ";
    std::cout << "max_speed: " << result.max_speed_mm_per_s_ << "\n";
    std::cout << "min_acc: " << result.min_acc_mm_per_ss_ << " ";
    std::cout << "max_acc: " << result.max_acc_mm_per_ss_ << "\n";
    std::cout << "min_force: " << result.min_force_ << " ";
    std::cout << "nominal_force: " << result.nominal_force_ << " ";
    std::cout << "overdrive_force: " << result.overdrive_force_ << "\n";
    return result;
  }

  /// Directions the wsg can "home" in.  Positive/Default are outward.
  enum HomeDirection { kDefault, kPositive, kNegative };

  /** Issues a Home (move to an extreme and calibrate there) command.
   * This command blocks until the move is complete. */
  bool Home(HomeDirection dir) {
    WsgCommandMessage command(kHome, {dir});
    auto response = SendAndAwaitResponse(command, 4);
    return response && (response->status() == E_SUCCESS);
  }

  /** Tares the force meter (sets the current force as "zero"). */
  bool Tare() {
    WsgCommandMessage command(kTareForceSensor, {});
    auto response = SendAndAwaitResponse(command, 4);
    // Allow E_NOT_AVAILABLE because it is not clear in the docs if the
    // internal current-based force sensor counts or not.
    return response && ((response->status() == E_SUCCESS) ||
                        (response->status() == E_NOT_AVAILABLE));
  }

  /** Issues a Grasp command to the gripper.
   *
   * This command blocks until the move is complete. */
  bool Grasp(double width_mm, double speed_mm_per_s) {
    WsgCommandMessage command(kGrasp, {});
    command.AppendToPayload(static_cast<float>(width_mm));
    command.AppendToPayload(static_cast<float>(speed_mm_per_s));
    auto response = SendAndAwaitResponse(command, 6);
    return response && (response->status() == E_SUCCESS);
  }

  bool SetForceLimit(double force) {
    WsgCommandMessage command(kSetForceLimit, {});
    command.AppendToPayload(static_cast<float>(force));
    return !!SendAndAwaitResponse(command, 0.1);
  }

  void SetForceLimitNonblocking(double force) {
    WsgCommandMessage command(kSetForceLimit, {});
    command.AppendToPayload(static_cast<float>(force));
    tx_.Send(command);
  }

  bool SetAcceleration(double acceleration_mm_per_ss) {
    WsgCommandMessage command(kSetAccel, {});
    command.AppendToPayload(static_cast<float>(acceleration_mm_per_ss));
    return !!SendAndAwaitResponse(command, 0.1);
  }

  bool ClearSoftLimits() {
    return !!SendAndAwaitResponse(
        WsgCommandMessage(kClearSoftLimits, {}), 0.1);
  }

  enum PrepositionStopMode {kPrepositionClampOnBlock = 0,
                            kPrepositionStopOnBlock = 1};
  enum PrepositionMoveMode {kPrepositionAbsolute = 0,
                            kPrepositionRelative = 2};

  /** Issues a Preposition command to the gripper.
   *
   * This command blocks until the move is complete. */
  bool Preposition(PrepositionStopMode stop_mode, PrepositionMoveMode move_mode,
                   float width_mm, float speed_mm_per_s) {
    WsgCommandMessage command = PrepositionCommand(
        stop_mode, move_mode, width_mm, speed_mm_per_s);
    auto response = SendAndAwaitResponse(command, 6);
    return response && (response->status() == E_SUCCESS);
  }

  /** Issues a Preposition command to the gripper.
   *
   * This command does not block. */
  void PrepositionNonblocking(
      PrepositionStopMode stop_mode, PrepositionMoveMode move_mode,
      float width_mm, float speed_mm_per_s) {
    WsgCommandMessage command = PrepositionCommand(
        stop_mode, move_mode, width_mm, speed_mm_per_s);
#ifdef DEBUG
    std::cout << "sending " << command.command() << " nonblocking" << std::endl;
#endif
    tx_.Send(command);
  }

  WsgCommandMessage PrepositionCommand(
      PrepositionStopMode stop_mode, PrepositionMoveMode move_mode,
      double width_mm, double speed_mm_per_s) {
    WsgCommandMessage command(kPrePosition, {});
    unsigned char flags = stop_mode | move_mode;
    command.AppendToPayload(flags);
    command.AppendToPayload(static_cast<float>(width_mm));
    command.AppendToPayload(static_cast<float>(speed_mm_per_s));
    return command;
  }

  /** Sets update rate for any recurring status message.
   * All of the GetWhatever commands have the same payload format, so this
   * convenience function will update any of them to kUpdatePeriodMs.
   *
   * This command blocks (for about @p update_period_ms) until the
   * configuration is complete and a message has been received. */
  void TurnOnUpdates(Command command,
                     uint16_t update_period_ms, double timeout) {
    std::vector<unsigned char> payload;
    WsgCommandMessage message(command, payload);
    // Here, 1 == always send automatic updates.
    message.AppendToPayload(static_cast<unsigned char>(1));
    message.AppendToPayload(update_period_ms);
    if (!SendAndAwaitResponse(message, timeout)) {
      throw std::runtime_error("Enabling updates failed");
    }
  }

  WsgReturnReceiver& rx() { return rx_; }
  WsgCommandSender& tx() { return tx_; }

 private:
  WsgReturnReceiver rx_;
  WsgCommandSender tx_;
};

}  // namespace schunk_driver
