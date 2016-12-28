# -*- python -*-

# This file marks a workspace root for the Bazel build system. see
# http://bazel.io/ .

workspace(name = "drake_schunk_driver")

load("//tools/third_party/kythe/tools/build_rules/config:pkg_config.bzl", "pkg_config_package")

pkg_config_package(
    name = "glib",
    modname = "glib-2.0",
)

pkg_config_package(
    name = "python2",
    modname = "python2",
)

new_git_repository(
    name = "lcm",
    remote = "https://github.com/lcm-proj/lcm.git",
    commit = "9015dce5defd3902b1725bd091b80c0517774e40",
    build_file = "tools/lcm.BUILD",
)
