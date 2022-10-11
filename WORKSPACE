# -*- python -*-

workspace(name = "drake_schunk_driver")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

(DRAKE_COMMIT, DRAKE_CHECKSUM) = (
    "v1.9.0",
    "a80a38d51bf2608489f7b44a882d00b76dac9765590afbfcf0c17090c16501ab",
)
# Before changing the COMMIT, temporarily uncomment the next line so that Bazel
# displays the suggested new value for the CHECKSUM.
# DRAKE_CHECKSUM = "0" * 64

# Or to build against a local checkout of Drake, at the bash prompt set an
# environment variable before building:
#  export SCHUNK_LOCAL_DRAKE_PATH=/home/user/stuff/drake

# Load an environment variable.
load("//:environ.bzl", "environ_repository")
environ_repository(name = "environ", vars = ["SCHUNK_LOCAL_DRAKE_PATH"])
load("@environ//:environ.bzl", SCHUNK_LOCAL_DRAKE_PATH = "SCHUNK_LOCAL_DRAKE_PATH")

# This declares the `@drake` repository as an http_archive from github,
# iff SCHUNK_LOCAL_DRAKE_PATH is unset.  When it is set, this declares a
# `@drake_ignored` package which is never referenced, and thus is ignored.
http_archive(
    name = "drake" if not SCHUNK_LOCAL_DRAKE_PATH else "drake_ignored",
    sha256 = DRAKE_CHECKSUM,
    strip_prefix = "drake-{}".format(DRAKE_COMMIT),
    urls = [x.format(DRAKE_COMMIT) for x in [
        "https://github.com/RobotLocomotion/drake/archive/{}.tar.gz",
    ]],
)

# This declares the `@drake` repository as a local directory,
# iff SCHUNK_LOCAL_DRAKE_PATH is set.  When it is unset, this declares a
# `@drake_ignored` package which is never referenced, and thus is ignored.
local_repository(
    name = "drake" if SCHUNK_LOCAL_DRAKE_PATH else "drake_ignored",
    path = SCHUNK_LOCAL_DRAKE_PATH,
)
print("Using SCHUNK_LOCAL_DRAKE_PATH={}".format(SCHUNK_LOCAL_DRAKE_PATH)) if SCHUNK_LOCAL_DRAKE_PATH else None  # noqa

load("@drake//tools/workspace:default.bzl", "add_default_repositories")

# WARNING: We allow `vtk`, but should take care to avoid linking in multiple
# versions of VTK!
add_default_repositories()
