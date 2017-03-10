# -*- python -*-

# This file marks a workspace root for the Bazel build system. see
# http://bazel.io/ .

workspace(name = "drake_schunk_driver")

load("//tools:github.bzl", "github_archive")
load("//tools/third_party/kythe/tools/build_rules/config:pkg_config.bzl", "pkg_config_package")

github_archive(
    name = "gflags",
    repository = "gflags/gflags",
    commit = "a69b2544d613b4bee404988710503720c487119a",
    sha256 = "8b3836d5ca34a2da4d6375cf5f2030c719b508ca16014fcc9d5e9b295b56a6c1",
)

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
