#include <cassert>
#include <ctime>

#include <math.h>
#include <unistd.h>
#include <sys/time.h>

#include <lcm/lcm-cpp.hpp>

#include "lcmtypes/schunk_driver/lcmt_schunk_command.hpp"
#include "lcmtypes/schunk_driver/lcmt_schunk_status.hpp"

#include "position_force_control.h"
#include "wsg.h"


namespace schunk_driver {

const char* kLcmStatusChannel = "SCHUNK_STATUS";
const char* kLcmCommandChannel = "SCHUNK_COMMAND";


/// This class implements an LCM endpoint that relays received LCM commands to
/// the Wsg and receieved Wsg status back over LCM.
class SchunkLcmClient {
 public:

  SchunkLcmClient()
      : lcm_(),
        pf_control_(std::unique_ptr<Wsg>(new Wsg())) {
    assert(lcm_.good());
  }

  ~SchunkLcmClient() {}

  bool Initialize() {
    pf_control_.DoCalibrationSteps();
    lcm_.subscribe(kLcmCommandChannel,
                   &SchunkLcmClient::HandleCommandMessage, this);
  }

  void Task() {
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
    lcm_status_.timestamp = tv.tv_sec * 1000000L + tv.tv_usec;

    lcm_.publish(kLcmStatusChannel, &lcm_status_);
    int result = lcm_.handleTimeout(1);
    assert(result >= 0);

    // TODO(ggould-tri) handle finger data and how force measurement changes
    // with smart fingers (eg, does this switch from force before to after
    // stiction)
  }

 private:
  void HandleCommandMessage(const lcm::ReceiveBuffer* rbuf,
                            const std::string& chan,
                            const lcmt_schunk_command* command) {
    lcm_command_ = *command;
  }

  lcm::LCM lcm_;
  schunk_driver::PositionForceControl pf_control_;
  lcmt_schunk_status lcm_status_;
  lcmt_schunk_command lcm_command_;
};
}  // namespace schunk_driver


int main(int argc, char** argv) {
  schunk_driver::SchunkLcmClient client;
  assert(client.Initialize());
  while(true) {
    client.Task();
    usleep(1000);
  }
  return 0;
}
