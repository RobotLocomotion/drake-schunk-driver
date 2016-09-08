#pragma once

#include "wsg.h"
#include "wsg_return_message.h"

namespace schunk_driver {

/// Provides a position control interface as well as asynchronous
/// position and force reporting.
class PositionControl {
 public:
  /// Start position/force control on the given WSG device.  Takes ownership
  /// of @p wsg because nobody else should be poking at the device while this
  /// controller is operating.
  PositionControl(std::unique_ptr<Wsg> wsg, float initial_force_limit);

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
  StatusCode DoCalibrationSteps();

  /// Sets the target position (in millimeters of base separation) and force
  /// (in Newtons, positive-outward).
  void SetPosition(double position_mm, double speed_mm_per_s);

  /// Sets the force limit (in Newtons)
  void SetForceLimit(double force);

  /// Process all available incoming data from the WSG.  This is meant to
  /// be called periodically by a higher-level task loop.
  void Task();

  /// Get the current position (in millimeters of base separation).
  double position_mm();

  /// Get the current applied force.  This is the force in Newtons applied by
  /// a grasped object to the gripper, positive outward, and so will be near
  /// zero when the fingers are in motion regardless of the (considerable)
  /// internal friction of the gripper and positive when an object is grasped.
  ///
  /// Note that per WSG documentation this force is an estimate based on
  /// current and so will be inaccurate and sensitive to temperature,
  /// twisting, and asymmetric loading of the fingers.
  double force();

 private:
  std::unique_ptr<Wsg> wsg_;

  uint32_t system_state_;  //< Bit-union of StateFlag values.
  GraspingState grasping_state_;

  double target_position_mm_;
  double last_position_mm_ = -1;
  double last_applied_force_ = -1;

  PhysicalLimits physical_limits_;
};

}
