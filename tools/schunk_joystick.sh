#!/bin/bash

# Top-level repository directory.
DISTRO=$(dirname $(dirname $(readlink -e $0)))

# Tool executable locations.
SCHUNK_DRIVER_CMD=$DISTRO"/bazel-bin/src/schunk_driver"
LCM_JOYSTICK_CONTROL_CMD=$DISTRO"/bazel-bin/tools/lcm_joystick_control"
LCM_LOGGER_CMD=$DISTRO"/bazel-bin/tools/lcm_logger"

# Where to find the LCMT python codegen files.
SCHUNK_LCMT_DIR=".runfiles/drake_schunk_driver/src/drake"

set -ex

export PYTHONPATH=$LCM_PY_DIR:$PYTHONPATH

# Map the left stick x axis to position from 1 to 99 (mm).
POSITION_MAPPING="--mapping 0 target_position_mm 50 49"

# Map the left stick y axis to force from 1 to 79 (N).
FORCE_MAPPING="--mapping 1 force 40 -39"

# Run our various ancillary commands that respond nicely to signals.
$SCHUNK_DRIVER_CMD &
$LCM_LOGGER_CMD "$LCM_LOGGER_CMD"$SCHUNK_LCMT_DIR --logfile log.csv \
                --format csv SCHUNK_WSG_COMMAND SCHUNK_WSG_STATUS &

# Run the joystick process (which ignores SIGINT and waits for a "start"
# button press to exit) and when it ends clean up everything else.
$LCM_JOYSTICK_CONTROL_CMD \
    "$LCM_JOYSTICK_CONTROL_CMD"$SCHUNK_LCMT_DIR lcmt_schunk_wsg_command \
    --lcm_tag SCHUNK_WSG_COMMAND \
    $POSITION_MAPPING $FORCE_MAPPING --quit-on 9
trap '' INT TERM  # Needed to make the 'wait' below actually execute.
kill -TERM 0
wait
