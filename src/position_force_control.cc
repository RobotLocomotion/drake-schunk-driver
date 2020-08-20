#include "position_force_control.h"

#include <cmath>
#include <cstring>

#include "wsg.h"
#include "wsg_command_message.h"
#include "wsg_command_sender.h"
#include "wsg_return_message.h"
#include "wsg_return_receiver.h"


namespace schunk_driver {

const static uint16_t kUpdatePeriodMs = 20;
const static double kUpdateAdjustTimeout = 0.25;

const static double kForceDeadband = 5;
const static double kPositionDeadbandMm = 5;

PositionForceControl::PositionForceControl(std::unique_ptr<Wsg> wsg)
    : wsg_(std::move(wsg)) {}


void PositionForceControl::DoCalibrationSteps() {
  // Print the system info for the user and fail fast if contacting the
  // gripper fails.  Result not currently used otherwise.
  wsg_->GetSystemInfo();

  // Set up periodic status updates on every available state structure.
  // We don't use all of these but we have bandwidth to spare and this
  // ensures we'll have them available in pcap debugging.
  wsg_->TurnOnUpdates(kGetSystemState, kUpdatePeriodMs, kUpdateAdjustTimeout);
  wsg_->TurnOnUpdates(kGetGraspState, kUpdatePeriodMs, kUpdateAdjustTimeout);
  wsg_->TurnOnUpdates(kGetOpeningWidth, kUpdatePeriodMs, kUpdateAdjustTimeout);
  wsg_->TurnOnUpdates(kGetSpeed, kUpdatePeriodMs, kUpdateAdjustTimeout);
  wsg_->TurnOnUpdates(kGetForce, kUpdatePeriodMs, kUpdateAdjustTimeout);

  // Home the fingers (to calibrate extents) and tare the sensors.
  wsg_->Home(Wsg::kNegative);
  wsg_->Home(Wsg::kPositive);
  wsg_->Tare();

  // Get physical limits.
  physical_limits_ = wsg_->GetPhysicalLimits();

  // Set all limits to their maxima.
  wsg_->ClearSoftLimits();
  wsg_->SetAcceleration(physical_limits_.max_acc_mm_per_ss_);
}


void PositionForceControl::SetPositionAndForce(
    double commanded_position_mm, double commanded_force) {
  // Use the preposition command (which is SPECIFICALLY NOT INTENDED for this
  // use case) to emulate force control.

  commanded_position_mm = std::min(
      commanded_position_mm,
      static_cast<double>(physical_limits_.stroke_mm_));
  commanded_force = std::min(
      commanded_force,
      static_cast<double>(physical_limits_.overdrive_force_));

  bool must_recommand = false;

  // If the commanded force is outside of our force deadband, we must
  // recommand.
  if (fabs(commanded_force - force()) > kForceDeadband) {
    must_recommand = true;
  }

  // If the commanded position and executing position are in opposite
  // directions from the current position we must recommand.
  if (commanded_position_mm > position_mm() &&
      position_mm() > executing_target_position_mm_) {
    must_recommand = true;
  } else if (commanded_position_mm < position_mm() &&
             position_mm() < executing_target_position_mm_) {
    must_recommand = true;
  }


  if (!must_recommand) { return; }

  wsg_->SetForceLimitNonblocking(commanded_force);
  executing_force_ = commanded_force;

  // TODO(ggould-tri) consider using grip command when motion is inward; this
  // is more correct but probably requires handling many more result statuses.
  wsg_->PrepositionNonblocking(
      Wsg::kPrepositionClampOnBlock, Wsg::kPrepositionAbsolute,
      commanded_position_mm, physical_limits_.max_speed_mm_per_s_);
  executing_target_position_mm_ = commanded_position_mm;
}


void PositionForceControl::Task() {
  std::unique_ptr<WsgReturnMessage> msg;
  do {
    msg = wsg_->rx().Receive();
    if (!msg || msg->status() != E_SUCCESS) {
      continue;  // TODO(ggould-tri) any error handling at all.
    }
    switch (msg->command()) {
      case kGetSystemState: {
        memcpy(&system_state_, msg->params().data(), sizeof(system_state_));
        break;
      }
      case kGetGraspState: {
        grasping_state_ = static_cast<GraspingState>(msg->params()[0]);
        break;
      }
      case kGetOpeningWidth: {
        float opening_width_float;
        memcpy(&opening_width_float, msg->params().data(),
               sizeof(opening_width_float));
        last_position_mm_ = opening_width_float;
        break;
      }
      case kGetForce: {
        float force_float;
        memcpy(&force_float, msg->params().data(), sizeof(force_float));
        last_applied_force_ = force_float;
        break;
      }
      case kGetSpeed: {
        float speed_float;
        memcpy(&speed_float, msg->params().data(), sizeof(speed_float));
        last_speed_mm_per_s_ = speed_float;
        break;
      }
      default: break;  // Discard uninteresting messages.
    }
  } while (msg);
}


double PositionForceControl::position_mm() const {
  return last_position_mm_;
}

double PositionForceControl::force() const {
  return last_applied_force_;
}

double PositionForceControl::speed_mm_per_s() const {
  return last_speed_mm_per_s_;
}

} // namespace schunk_driver
