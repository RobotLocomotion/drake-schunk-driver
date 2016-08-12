#pragma once

#include <vector>

namespace schunk_driver {

// These enum values were copied from the WSG Command Set Reference
// Manual.  They deviate from our code conventions because they match
// the definitions in the manual.

/// Commands; for definitions see the command set reference.
typedef enum {
  kLoop = 0x06,
  kDisconnectAnnounce = 0x07,
  kHome = 0x20,
  kPrePosition = 0x21,
  kStop = 0x22,
  kFastStop = 0x23,
  kAcknowledgeStopOrFault = 0x24,
  kGrasp = 0x25,
  kRelease = 0x26,
  kSetAccel = 0x30,
  kGetAccel = 0x31,
  kSetForceLimit = 0x32,
  kGetForceLimit = 0x33,
  kSetSoftLimits = 0x34,
  kGetSoftLimits = 0x35,
  kClearSoftLimits = 0x36,
  kTareForceSensor = 0x38,
  kGetSystemState = 0x40,
  kGetGraspState = 0x41,
  kGetGraspStats = 0x42,
  kGetOpeningWidth = 0x43,
  kGetSpeed = 0x44,
  kGetForce = 0x45,
  kGetTemperature = 0x46,
  kGetSystemInfo = 0x50,
  kSetDeviceTag = 0x51,
  kGetDeviceTag = 0x52,
  kGetSystemLimits = 0x53,
  kGetFingerInfo = 0x60,
  kGetFingerFlags = 0x61,
  kFingerPowerControl = 0x62,
  kGetFingerData = 0x63,
} Command;

class WsgCommandMessage {
 public:
  WsgCommandMessage(int command, const std::vector<unsigned char>& payload)
      : command_(command),
        payload_(payload) {}

  template<typename T>
  void AppendToPayload(const T& new_item);

  int command() const { return command_; }
  const std::vector<unsigned char> payload() const { return payload_; }

  void Serialize(std::vector<unsigned char>& buffer) const;

 private:
  const int command_;
  std::vector<unsigned char> payload_;
};

}  // namespace schunk_driver
