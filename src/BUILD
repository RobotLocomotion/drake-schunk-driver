# -*- python -*-
# This file contains rules for Bazel; see drake/doc/bazel.rst.

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "schunk_driver",
    srcs =  [
        "crc.h",
        "defaults.h",
        "position_force_control.h",
        "position_force_control.cc",
        "schunk_driver.cc",
        "wsg.h",
        "wsg_command_message.h",
        "wsg_command_message.cc",
        "wsg_command_sender.h",
        "wsg_command_sender.cc",
        "wsg_return_message.h",
        "wsg_return_message.cc",
        "wsg_return_receiver.h",
        "wsg_return_receiver.cc",
    ],
    linkstatic = 1,
    deps = [
        "@drake//lcmtypes:schunk",
        "@gflags//:gflags",
        "@lcm//:lcm",
    ]
)
