# drake-schunk-driver
Driver software for the Schunk gripper.

## Prereqs

This effectively requires the source prequisites of Drake. For using on
Ubuntu, you may need to do something like:

```sh
git clone -o upstream https://github.com/RobotLocomotion/drake -b v1.9.0
cd drake
sudo ./setup/ubuntu/install_prereqs.sh
```

## Building

To build:

```sh
bazel build //...
```

## Configuring the gripper

Under Settings -> Command Interface:

 * Command Interface
  * Interface: UDP/IP
  * Enable CRC: Off
 * UDP Settings
  * UDP Client Address: 0.0.0.0
  * UDP Listening Port: 1500
  * UDP Remote Port: 1501

## Running the driver

`./bazel-bin/src/schunk_driver` -- this will expect the gripper to have the defualt IP (192.168.1.20) and ports (1500, 1501).

If the gripper configuration has been changed, the following arguments
can be useful:

 * `--gripper-addr` Specifies the IP address of the gripper (default:
   192.168.1.20)
 * `--gripper_port` The UDP Listening Port on the gripper (default: 1500)
 * `--local_port` The UDP Remote Port on the gripper (the local port
    which the driver will use on the host machine) (default: 1501)
