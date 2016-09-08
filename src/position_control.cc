#include "position_control.h"

#include <cassert>
#include <cstring>

#include "wsg.h"
#include "wsg_command_message.h"
#include "wsg_command_sender.h"
#include "wsg_return_message.h"
#include "wsg_return_receiver.h"


namespace schunk_driver {

const static uint16_t kUpdatePeriodMs = 5;
const static double kUpdateAdjustTimeout = 0.25;

PositionControl::PositionControl(std::unique_ptr<Wsg> wsg, float initial_force_limit)
    : wsg_(std::move(wsg)) {
  wsg_->SetForceLimit(initial_force_limit);
  
  // Home the fingers (to calibrate extents) and tare the sensors.
  wsg_->Home(Wsg::kNegative);
  wsg_->Home(Wsg::kPositive);
  wsg_->Tare();

  // Set up periodic status updates on every available state structure.
  // We don't use all of these but we have bandwidth to spare and this
  // ensures we'll have them available in pcap debugging.
  wsg_->TurnOnUpdates(kGetSystemState, kUpdatePeriodMs, kUpdateAdjustTimeout);
  wsg_->TurnOnUpdates(kGetGraspState, kUpdatePeriodMs, kUpdateAdjustTimeout);
  wsg_->TurnOnUpdates(kGetOpeningWidth, kUpdatePeriodMs, kUpdateAdjustTimeout);
  wsg_->TurnOnUpdates(kGetSpeed, kUpdatePeriodMs, kUpdateAdjustTimeout);
  wsg_->TurnOnUpdates(kGetForce, kUpdatePeriodMs, kUpdateAdjustTimeout);


  // Get physical limits.
  physical_limits_ = wsg_->GetPhysicalLimits();
}


void PositionControl::SetPosition(
    double position_mm, double speed_mm_per_s) {
  // Use the preposition command to move to this position
  // TODO(gizatt) is this appropriate for closing on an object?
  wsg_->Stop();
  wsg_->PrepositionNonblocking(
      Wsg::kPrepositionClampOnBlock, Wsg::kPrepositionAbsolute,
      position_mm, speed_mm_per_s);
}

void PositionControl::SetForceLimit(
  double force) {
  wsg_->SetForceLimit(force);
}

void PositionControl::Task() {
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
        memcpy(&force_float, msg->params().data(),
               sizeof(force_float));
        last_applied_force_ = force_float;
        break;
      }
      default: break;  // Discard uninteresting messages.
    }
  } while (msg);
}


double PositionControl::position_mm() { return last_position_mm_; }
double PositionControl::force() { return last_applied_force_; }

} // namespace schunk_driver
