#pragma once

#include "wsg.h"
#include "wsg_return_message.h"

namespace schunk_driver {

/// Class that emulates position/force control of the Schunk gripper ("WSG").
/// Takes target position and force in and attempts to reach that position
/// with that force.  Emits the achieved position and applied force.
class PositionForceControl {
 public:
  /// Start position/force control on the given WSG device.  Takes ownership
  /// of @p wsg because nobody else should be poking at the device while this
  /// controller is operating.
  PositionForceControl(std::unique_ptr<Wsg> wsg);

  /// Performs initial configuration and calibration of the WSG.  This moves
  /// the gripper fingers, so don't do it while the fingers are grasping or
  /// impeded.  This should in theory be needed only at startup and very
  /// rarely thereafer.
  ///
  /// This is a blocking command and will return only when the calibration is
  /// complete or failed.
  ///
  /// Following this command, the fingers will be at their positive limit
  /// (fully open, with an ~110mm base separation, ~103mm finger separation
  /// with the default hard fingers) and zero target force.
  void DoCalibrationSteps();

  /// Sets the target position (in millimeters of base separation) and force
  /// (in Newtons, positive-outward).
  void SetPositionAndForce(double position_mm, double force);

  /// Process all available incoming data from the WSG.  This is meant to
  /// be called periodically by a higher-level task loop.
  void Task();

  /// Get the current position (in millimeters of base separation).
  double position_mm() const;

  /// Get the current speed (in millimeters per second).
  double speed_mm_per_s() const;

  /// Get the current applied force.  This is the force in Newtons applied by
  /// a grasped object to the gripper, positive outward, and so will be near
  /// zero when the fingers are in motion regardless of the (considerable)
  /// internal friction of the gripper and positive when an object is grasped.
  ///
  /// Note that per WSG documentation this force is an estimate based on
  /// current and so will be inaccurate and sensitive to temperature,
  /// twisting, and asymmetric loading of the fingers.
  double force() const;

 private:
  std::unique_ptr<Wsg> wsg_;

  // State of the gripper, according to most recent status messages received;
  // valid only after DoCalibrationSteps().
  uint32_t system_state_ {0};  //< Bit-union of StateFlag values.
  GraspingState grasping_state_ {kIdle};
  double last_position_mm_ {0};
  double last_applied_force_ {0};
  double last_speed_mm_per_s_ {0};

  // Current position/force command that the gripper is executing, or has
  // encountered an error while executing.
  double executing_target_position_mm_ {0};
  double executing_force_ {0};

  // Physical limit constants reported by the gripper; valid only after
  // DoCalibrationSteps().
  PhysicalLimits physical_limits_;
};

}
