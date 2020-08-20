#pragma once

#include <memory>
#include <vector>

namespace schunk_driver {

/// Status codes
typedef enum {
  E_SUCCESS = 0, //!< No error
  E_NOT_AVAILABLE, //!< Device, service or data is not available
  E_NO_SENSOR, //!< No sensor connected
  E_NOT_INITIALIZED, //!< The device is not initialized
  E_ALREADY_RUNNING, //!< Service is already running
  E_FEATURE_NOT_SUPPORTED, //!< The asked feature is not supported
  E_INCONSISTENT_DATA, //!< One or more dependent parameters mismatch
  E_TIMEOUT, //!< Timeout error
  E_READ_ERROR, //!< Error while reading from a device
  E_WRITE_ERROR, //!< Error while writing to a device
  E_INSUFFICIENT_RESOURCES, //!< No memory available
  E_CHECKSUM_ERROR, //!< Checksum error
  E_NO_PARAM_EXPECTED, //!< No parameters expected
  E_NOT_ENOUGH_PARAMS, //!< Not enough parameters
  E_CMD_UNKNOWN, //!< Unknown command
  E_CMD_FORMAT_ERROR, //!< Command format error
  E_ACCESS_DENIED, //!< Access denied
  E_ALREADY_OPEN, //!< The interface is already open
  E_CMD_FAILED, //!< Command failed
  E_CMD_ABORTED, //!< Command aborted
  E_INVALID_HANDLE, //!< invalid handle
  E_NOT_FOUND, //!< device not found
  E_NOT_OPEN, //!< device not open
  E_IO_ERROR, //!< I/O error
  E_INVALID_PARAMETER, //!< invalid parameter
  E_INDEX_OUT_OF_BOUNDS, //!< index out of bounds
  E_CMD_PENDING, //!< Command execution needs more time
  E_OVERRUN, //!< Data overrun
  E_RANGE_ERROR, //!< Range error
  E_AXIS_BLOCKED, //!< Axis is blocked
  E_FILE_EXISTS //!< File already exists
} StatusCode;

/// State flag bits; for definitions see the command set reference.
typedef enum {
  // 31-21 reserved.
  SF_SCRIPT_FAILURE     = 1 << 20,
  SF_SCRIPT_RUNNING     = 1 << 19,
  SF_CMD_FAILURE        = 1 << 18,
  SF_FINGER_FAULT       = 1 << 17,
  SF_CURR_FAULT         = 1 << 16,
  SF_POWER_FAULT        = 1 << 15,
  SF_TEMP_FAULT         = 1 << 14,
  SF_TEMP_WARNING       = 1 << 13,
  SF_FAST_STOP          = 1 << 12,
  // 11 reserved.
  // 10 reserved.
  SF_FORCECNTL_MODE     = 1 << 9,
  // 8 reserved.
  SF_TARGET_POS_REACHED = 1 << 7,
  SF_AXIS_STOPPED       = 1 << 6,
  SF_SOFT_LIMIT_PLUS    = 1 << 5,
  SF_SOFT_LIMIT_MINUS   = 1 << 4,
  SF_BLOCKED_PLUS       = 1 << 3,
  SF_BLOCKED_MINUS      = 1 << 2,
  SF_MOVING             = 1 << 1,
  SF_REFERENCED         = 1 << 0,
} StateFlag;

/// Grasping states; for definitions see the command set reference.
typedef enum {
  kIdle = 0,
  kGrasping = 1,
  kNoPartFound = 2,
  kPartLost = 3,
  kHolding = 4,
  kReleasing = 5,
  kPositioning = 6,
  kError = 7
} GraspingState;


class WsgReturnMessage {
 public:
  WsgReturnMessage(int command, int status,
                   const std::vector<unsigned char>& params)
      : command_(command),
        status_(status),
        params_(params) {}

  int command() const { return command_; }
  int status() const { return status_; }
  const std::vector<unsigned char>& params() const { return params_; }

  static std::unique_ptr<WsgReturnMessage> Parse(
      std::vector<unsigned char>& buffer);

 private:
  const int command_;
  const int status_;
  const std::vector<unsigned char> params_;
};

}
