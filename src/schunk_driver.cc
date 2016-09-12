#include <iostream>
#include <cassert>
#include <vector>
#include <pthread.h>
#include <unistd.h>

#include <lcm/lcm-cpp.hpp>
#include "lcmtypes/schunk_driver/lcmt_schunk_command.hpp"
#include "lcmtypes/schunk_driver/lcmt_schunk_status.hpp"

#include "position_control.h"

using namespace std;
using namespace schunk_driver;

const double kSendPeriod = 0.01;

schunk_driver::PositionControl * pc;
bool running = true;
lcm::LCM * lc;

static double getUnixTime(void)
{
    struct timespec tv;

    if(clock_gettime(CLOCK_REALTIME, &tv) != 0) return 0;

    return (tv.tv_sec + (tv.tv_nsec / 1000000000.0));
}

void handle_position_command(const lcm::ReceiveBuffer* rbuf, 
                           const std::string& channel, 
                           const lcmt_schunk_command* msg, 
                           void * user) {
  printf("Recv message, force to %3.2f and pos to %3.2f sp %3.2f\n", msg->force, msg->target_position_mm, msg->target_speed_mm_per_s);
  pc->SetForceLimit(msg->force);
  pc->SetPosition(msg->target_position_mm, msg->target_speed_mm_per_s);
}

void * lcm_handler(void *thread_id){
  while(running){
    lc->handle();
    usleep(1);
  }
  pthread_exit(NULL);
}

void * report_status(void *thread_id)
{
  long tid;
  tid = (long) thread_id;
  cout << "Publisher started with thread ID, " << tid << endl;
  int i=0;
  double last_send = getUnixTime();
  while (running){
    pc->Task();

    double now = getUnixTime();
    if (now - last_send > kSendPeriod){
      last_send = now;
      printf("Pos: %2.2f, Force: %2.2f\n", pc->position_mm(), pc->force());
   
      schunk_driver::lcmt_schunk_status msg;
      msg.timestamp = 0;
      msg.actual_position_mm = pc->position_mm();
      msg.actual_force = pc->force();
      msg.actual_speed_mm_per_s = 0; // TODO(gizatt) fill this out
      lc->publish("WSG_STATUS", &msg);
    }
    usleep(10);
  }
  pthread_exit(NULL);
}

int main(int argc, char** argv) {
  lc = new lcm::LCM;
  if (!lc->good()){
    printf("LCM init error! D:\n");
    exit(-1);
  }
  lc->subscribeFunction("WSG_POSITION_COMMAND", handle_position_command, (void *) NULL);

  pc = new PositionControl(std::unique_ptr<Wsg>(new Wsg()), 10.0);

  // spawn off thread to publish state
  pthread_t publisher_thread;
  pthread_t lc_thread;
  pthread_create(&publisher_thread, NULL, report_status, NULL);
  pthread_create(&lc_thread, NULL, lcm_handler, NULL);

  try {
    while (1){
      usleep(10);
    }
  } catch (const std::exception& e) {
    running = false;
  }

  void * status;
  pthread_join(publisher_thread, &status);
  pthread_join(lc_thread, &status);
  pthread_exit(NULL);
  return 0;
}
