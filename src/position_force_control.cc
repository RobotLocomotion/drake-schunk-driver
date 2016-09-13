#include "position_force_control.h"

#include <cassert>
#include <cmath>
#include <cstring>

#include "wsg.h"
#include "wsg_command_message.h"
#include "wsg_command_sender.h"
#include "wsg_return_message.h"
#include "wsg_return_receiver.h"


namespace schunk_driver {

const static uint16_t kUpdatePeriodMs = 5;
const static double kUpdateAdjustTimeout = 0.25;

const static double kForceDeadband = 5;
const static double kPositionDeadbandMm = 5;

PositionForceControl::PositionForceControl(std::unique_ptr<Wsg> wsg)
    : wsg_(std::move(wsg)) {}


StatusCode PositionForceControl::DoCalibrationSteps() {
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
    double position_mm, double force) {
  // Use the preposition command (which is SPECIFICALLY NOT INTENDED for this
  // use case) to emulate force control.

  // TODO(ggould-tri) consider using grip command when motion is inward; this
  // is more correct but probably requires handling many more result statuses.

  // Do not recommand if new values are within epsilon of currently commanded
  // values, in order to avoid unnecessary Stop commands and resulting delay,
  // jerkiness, noise, and heat.
  if ((std::abs(force - current_target_force_) < kForceDeadband) &&
      (std::abs(position_mm - current_target_pos_mm_) < kPositionDeadbandMm)) {
    return;
  }

  // TODO(ggould-tri) adjust acceleration limit to some multiple of force.
  wsg_->SetForceLimit(force);
  wsg_->Stop();
  wsg_->PrepositionNonblocking(
      Wsg::kPrepositionClampOnBlock, Wsg::kPrepositionAbsolute,
      position_mm, physical_limits_.max_speed_mm_per_s_);
  current_target_force_ = force;
  current_target_pos_mm_ = position_mm;
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


double PositionForceControl::position_mm() { return last_position_mm_; }
double PositionForceControl::force() { return last_applied_force_; }
double PositionForceControl::speed_mm_per_s() { return last_speed_mm_per_s_; }

} // namespace schunk_driver
