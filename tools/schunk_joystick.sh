#!/bin/bash

# Top-level repository directory.
DISTRO=$(dirname $(dirname $(readlink -e $0)))
# The location of the 'build' dir.
BUILD=$DISTRO"/build"

# Tool executable locations.
SCHUNK_DRIVER_CMD=$BUILD"/schunk_driver"
LCM_JOYSTICK_CONTROL_CMD=$DISTRO"/tools/lcm_joystick_control.py"
LCM_LOGGER_CMD=$DISTRO"/tools/lcm_logger.py"

# Where to find the LCMT python codegen files.
SCHUNK_LCMT_DIR=$BUILD"/schunk-driver-prefix/src/schunk-driver-build/lcmgen/lcmtypes/drake"

# Where to find liblcm.so.1.
# TODO(ggould-tri) Supposedly this will change and become unnecessary soon.
LCM_SO_DIR=$BUILD"/lib"

# Where to find "lcm/__init__.py".
LCM_PY_DIR=$BUILD"/lib/python2.7/site-packages/lcm/"

set -ex

export LD_LIBRARY_PATH=$LCM_SO_DIR:$LD_LIBRARY_PATH
export PYTHONPATH=$LCM_PY_DIR:$PYTHONPATH

# Map the left stick x axis to position from 1 to 99 (mm).
POSITION_MAPPING="--mapping 0 target_position_mm 50 49"

# Map the left stick y axis to force from 1 to 79 (N).
FORCE_MAPPING="--mapping 1 force 40 -39"

# Run our various ancillary commands that respond nicely to signals.
$SCHUNK_DRIVER_CMD &
$LCM_LOGGER_CMD $SCHUNK_LCMT_DIR --logfile log.csv --format csv \
                SCHUNK_WSG_COMMAND SCHUNK_WSG_STATUS &

# Run the joystick process (which ignores SIGINT and waits for a "start"
# button press to exit) and when it ends clean up everything else.
$LCM_JOYSTICK_CONTROL_CMD \
    $SCHUNK_LCMT_DIR lcmt_schunk_wsg_command --lcm_tag SCHUNK_WSG_COMMAND \
    $POSITION_MAPPING $FORCE_MAPPING --quit-on 9
trap '' INT TERM  # Needed to make the 'wait' below actually execute.
kill -TERM 0
wait
