#include "position_force_control.h"

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

  // Set all limits to their maxima.
  
}


void PositionForceControl::SetPositionAndForce(
    double position_mm, double force) {
  // TODO(ggould-tri) The logic below to reverse-engineer position/force
  // control out of the Schunk is probably not valid for picking up squishy
  // objects like baloons or muffins.  However the documentation of the Schunk
  // doesn't give me any good ideas for how to do better.

  // If the gripper is in motion, send a stop command.
  
  // Set the maximum force to the commanded force.
  
  // If the target position is narrower than the current position, use a Grasp
  // command.  It may fail if there is no object to grasp, but it will still
  // attain the target position in doing so.
  
  // If the target position is wider than the current position, use a
  // Preposition command.
  
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
        memcpy(&force_float, msg->params().data(),
               sizeof(force_float));
        last_applied_force_ = force_float;
        break;
      }
      default: break;  // Discard uninteresting messages.
    }
  } while (msg);
}


double PositionForceControl::position_mm() { return last_position_mm_; }
double PositionForceControl::force() { return last_applied_force_; }

} // namespace schunk_driver
