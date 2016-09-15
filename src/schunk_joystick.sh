#!/bin/bash

# Where to find lcm_joystick_control.py.
LCM_JOYSTICK_CONTROL_CMD=$(dirname $(readlink -e $0))/lcm_joystick_control.py

# The location of the 'build' dir.
LOCAL_BUILD=$(readlink -e $(dirname $0)"/../build")

# Where to find the LCMT python codegen files.
SCHUNK_LCMT_DIR=$LOCAL_BUILD"/schunk-driver-prefix/src/schunk-driver-build/lcmgen/lcmtypes/schunk_driver"

# Where to find liblcm.so.1.
LCM_SO_DIR=$LOCAL_BUILD"/lib"

# Where to find "lcm/__init__.py".
LCM_PY_DIR=$LOCAL_BUILD"/lib/python2.7/site-packages/lcm/"

set -ex

export LD_LIBRARY_PATH=$LCM_SO_DIR:$LD_LIBRARY_PATH
export PYTHONPATH=$LCM_PY_DIR:$PYTHONPATH

$LCM_JOYSTICK_CONTROL_CMD \
    $SCHUNK_LCMT_DIR lcmt_schunk_command --lcm_tag SCHUNK_COMMAND \
    --mapping 0 target_position_mm 50 49 \
    --mapping 1 force 40 -39 \
    --quit-on 9
