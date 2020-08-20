#include <cassert>
#include <ctime>
#include <iostream>

#include <math.h>
#include <unistd.h>
#include <sys/time.h>

#include <gflags/gflags.h>
#include <lcm/lcm-cpp.hpp>

#include "drake/lcmt_schunk_wsg_command.hpp"
#include "drake/lcmt_schunk_wsg_status.hpp"

#include "defaults.h"
#include "position_force_control.h"
#include "wsg.h"

using drake::lcmt_schunk_wsg_command;
using drake::lcmt_schunk_wsg_status;

namespace {

const char* kLcmStatusChannel = "SCHUNK_WSG_STATUS";
const char* kLcmCommandChannel = "SCHUNK_WSG_COMMAND";

}  // namespace

DEFINE_string(gripper_addr, schunk_driver::kGripperAddrStr,
              "Address of the gripper to control");
DEFINE_int32(gripper_port, schunk_driver::kGripperPort,
             "Gripper UDP port");
DEFINE_int32(local_port, schunk_driver::kLocalPort,
             "Local UDP port");
DEFINE_string(lcm_command_channel, kLcmCommandChannel,
              "Channel to receive LCM command messages on");
DEFINE_string(lcm_status_channel, kLcmStatusChannel,
              "Channel to send LCM status messages on");

namespace schunk_driver {
/// This class implements an LCM endpoint that relays received LCM commands to
/// the Wsg and receieved Wsg status back over LCM.
class SchunkLcmClient {
 public:

  SchunkLcmClient()
      : lcm_(),
        pf_control_(std::unique_ptr<Wsg>(new Wsg(
            nullptr, FLAGS_local_port,
            FLAGS_gripper_addr.c_str(), FLAGS_gripper_port))) {
    assert(lcm_.good());
  }

  ~SchunkLcmClient() {}

  void Initialize() {
    pf_control_.DoCalibrationSteps();
    lcm_.subscribe(FLAGS_lcm_command_channel,
                   &SchunkLcmClient::HandleCommandMessage, this);
  }

  void Task() {
    // Process all pending messages since our last Task() so that we
    // only act on the newest.
    int result = -1;
    while ((result = lcm_.handleTimeout(0)) > 0) {}
    assert(result == 0);

    pf_control_.Task();
    // Schunk only uses positive force; use absolute value of commanded force.
    pf_control_.SetPositionAndForce(lcm_command_.target_position_mm,
                                    fabs(lcm_command_.force));
    lcm_status_.actual_position_mm = pf_control_.position_mm();
    lcm_status_.actual_speed_mm_per_s = pf_control_.speed_mm_per_s();

    // Schunk returns only scalar force resisting its motion, so invert force
    // when motion is negative.
    lcm_status_.actual_force = (pf_control_.speed_mm_per_s() > 0
                                ? pf_control_.force()
                                : -pf_control_.force());

    struct timeval tv;
    gettimeofday(&tv, nullptr);
    lcm_status_.utime = tv.tv_sec * 1000000L + tv.tv_usec;

    lcm_.publish(FLAGS_lcm_status_channel, &lcm_status_);

    // TODO(ggould-tri) handle finger data and how force measurement changes
    // with smart fingers (eg, does this switch from force before to after
    // stiction)
  }

 private:
  void HandleCommandMessage(const lcm::ReceiveBuffer* rbuf,
                            const std::string& chan,
                            const lcmt_schunk_wsg_command* command) {
    lcm_command_ = *command;
  }

  lcm::LCM lcm_;
  schunk_driver::PositionForceControl pf_control_;
  lcmt_schunk_wsg_status lcm_status_;
  lcmt_schunk_wsg_command lcm_command_{};
};
}  // namespace schunk_driver


int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  schunk_driver::SchunkLcmClient client;
  client.Initialize();

  while (true) {
    client.Task();
    // Note: too short of a sleep here can put the gripper into some
    // kind of error state.  The right thing to do is probably to
    // directly regulate how quickly we're sending commands, but a
    // 50ms delay on grasping is probably not the end of the world.
    usleep(50000);
  }
  return 0;
}
