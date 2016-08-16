#include <cmath>
#include <iostream>
#include <memory>
#include <unistd.h>

#include "position_force_control.h"
#include "wsg.h"


using namespace schunk_driver;

int main(int argc, char** argv) {
  schunk_driver::PositionForceControl pf_control(
      std::unique_ptr<Wsg>(new Wsg()));
  pf_control.DoCalibrationSteps();
  int i = 0;
  while(true) {
    i++;

    // Every 100 ms update our position and force.
    if ((i % 100) == 0) {
      pf_control.SetPositionAndForce(
          50 + (30 * sin(static_cast<double>(i) / 1000)),
          80);
    }

    // Every 1000ms log a message.
    if ((i % 1000) == 0) {
      std::cout << "Grapsing force " << pf_control.force() << " Newtons; "
                << "Width " << pf_control.position_mm() << "mm" << std::endl;
    }

    pf_control.Task();
    usleep(1000);
  }

  return 0;
}
