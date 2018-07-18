#include <cassert>
#include <ctime>

#include <math.h>
#include <unistd.h>
#include <sys/time.h>

#include "defaults.h"
#include "position_force_control.h"
#include "wsg.h"

// ROS
#include "ros/ros.h"

// ROS wsg50_msgs
#include "wsg50_msgs/WSG_50_command.h"
#include "wsg50_msgs/WSG_50_state.h"


// namespace {

// const char* kROStatusTopic = "SCHUNK_WSG_STATUS";
// const char* kROSCommandTopic = "SCHUNK_WSG_COMMAND";

// }  // namespace

// DEFINE_string(gripper_addr, schunk_driver::kGripperAddrStr,
//               "Address of the gripper to control");
// DEFINE_int32(gripper_port, schunk_driver::kGripperPort,
//              "Gripper UDP port");
// DEFINE_int32(local_port, schunk_driver::kLocalPort,
//              "Local UDP port");
// DEFINE_string(ros_command_topic, kROSCommandTopic,
//               "Topic to receive ROS command messages on");
// DEFINE_string(ros_status_topic, kROSStatusTopic,
//               "Topic to send ROS status messages on");

namespace schunk_driver {
/// This class implements a ROS endpoint that relays received LCM commands to
/// the Wsg and receieved Wsg status back over ROS.
class SchunkROSClient {
 public:

  SchunkROSClient(ros::NodeHandle nh, std::string gripper_address, std::string command_topic,
    std::string status_topic, int gripper_port, int local_port)
      : nh_(nh),
        command_topic_(command_topic),
        status_topic_(status_topic),
        pf_control_(std::unique_ptr<Wsg>(new Wsg(
            nullptr, local_port,
            gripper_address.c_str(), gripper_port))) {
  }

  ~SchunkROSClient() {}

  bool Initialize() {
    StatusCode status_code = pf_control_.DoCalibrationSteps();
    command_subscriber_ = nh_.subscribe(command_topic_, 1, 
                   &SchunkROSClient::HandleCommandMessage, this);
    status_publisher_ = this->nh_.advertise<wsg50_msgs::WSG_50_state>(status_topic_, 0);

    // setup the default command msg, which is to open the gripper
    command_msg_.position_mm = 100;
    command_msg_.force = 40;

    return status_code == StatusCode::E_SUCCESS;
  }

  void Task() {
    // Process all pending messages since our last Task() so that we
    // only act on the newest.

    pf_control_.Task();

    // Schunk only uses positive force; use absolute value of commanded force.
    pf_control_.SetPositionAndForce(command_msg_.position_mm,
                                    fabs(command_msg_.force));

    status_msg_.position_mm = pf_control_.position_mm();
    status_msg_.speed_mm_per_s = pf_control_.speed_mm_per_s();

    // Schunk returns only scalar force resisting its motion, so invert force
    // when motion is negative.
    status_msg_.force = (pf_control_.speed_mm_per_s() > 0
                                ? pf_control_.force()
                                : -pf_control_.force());

    status_msg_.header.stamp = ros::Time::now();

    this->status_publisher_.publish(status_msg_);

    // TODO(ggould-tri) handle finger data and how force measurement changes
    // with smart fingers (eg, does this switch from force before to after
    // stiction)
  }

 private:
  void HandleCommandMessage(const wsg50_msgs::WSG_50_command::ConstPtr & msg) {
    command_msg_ = *msg; //copy the message to local memory
  }

  ros::NodeHandle nh_;
  std::string command_topic_;
  std::string status_topic_;
  schunk_driver::PositionForceControl pf_control_;
  ros::Subscriber command_subscriber_;
  ros::Publisher status_publisher_;
  wsg50_msgs::WSG_50_command command_msg_;
  wsg50_msgs::WSG_50_state status_msg_;
  // lcmt_schunk_wsg_status lcm_status_;
  // lcmt_schunk_wsg_command lcm_command_;
};
}  // namespace schunk_driver


int main(int argc, char** argv) {

  // // sets things in "FLAGS"
  // gflags::ParseCommandLineFlags(&argc, &argv, true);

  ros::init(argc, argv, "schunk_wsg50_driver");
  ros::NodeHandle nh("~");
  
  // read in parameters
  std::string command_topic, status_topic, gripper_address;
  int gripper_port, local_port, loop_rate_Hz;

  ros::param::param<std::string>("command_topic", command_topic, "schunk_wsg_command");
  ros::param::param<std::string>("status_topic", status_topic, "schunk_wsg_status");

  // ros::param::param<std::string>("gripper_address", gripper_address, schunk_driver::kGripperAddrStr);

  ros::param::param<int>("gripper_port", gripper_port, schunk_driver::kGripperPort);
  ros::param::param<int>("local_port", local_port, schunk_driver::kLocalPort);

  ros::param::param<int>("loop_rate_Hz", loop_rate_Hz, 20);

  nh.getParam("gripper_address", gripper_address); 

  // gripper_address = "192.170.10.20"; // hack for now
  std::cout << "gripper_address = " << gripper_address << std::endl;
  std::cout << "gripper_port = " << gripper_port  << std::endl;
  std::cout << "local_port = " << local_port  << std::endl;
  std::cout << "command_topic = " << command_topic << std::endl ;
  std::cout << "status_topic = " << status_topic << std::endl ;

  // ROS_INFO("command_topic = %s \n", command_topic);
  // ROS_INFO("gripper_address = %s \n", gripper_address);
  // ROS_INFO("gripper_port = %s\n", gripper_port);
  

  ros::Rate loop_rate(loop_rate_Hz);
  std::cout << "constructing SchunkROSClient \n";
  schunk_driver::SchunkROSClient client(nh, gripper_address, command_topic, status_topic, gripper_port, local_port);
  std::cout << "attemping to connect to gripper\n"; 
  client.Initialize();
  std::cout << "connected to gripper\n"; 

  // assert(client.Initialize());
  std::cout << "starting to spin . . . \n";
  while(true) {
    ros::spinOnce();
    client.Task();
    // Note: too short of a sleep here can put the gripper into some
    // kind of error state.  The right thing to do is probably to
    // directly regulate how quickly we're sending commands, but a
    // 50ms delay on grasping is probably not the end of the world.
    loop_rate.sleep();
  }
  return 0;
}
