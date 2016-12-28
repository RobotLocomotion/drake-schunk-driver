import importlib
import os
import sys

# Locate a built copy of LCM in this directory's build and export trees.
THIS_FILE = os.path.abspath(__file__)
THIS_DIR = os.path.dirname(THIS_FILE)
DIST_DIR = os.path.dirname(THIS_DIR)
SRC_DIR = os.path.join(DIST_DIR, 'src')
BUILD_LIBS_DIR = os.path.join(DIST_DIR, "build/lib/")
BUILD_INSTALL_LIBS_DIR = os.path.join(DIST_DIR, "build/install/lib/")
BUILD_PYLIBS_DIR = os.path.join(DIST_DIR, "build/lib/python2.7")
LCM_SO_DIR = os.path.join(DIST_DIR, "externals/lcm/lcm")
sys.path.extend([
    LCM_SO_DIR,
    os.path.join(BUILD_PYLIBS_DIR, "dist-packages"),
    os.path.join(BUILD_PYLIBS_DIR, "site-packages")])
print sys.path

# Re-export all of the symbols in lcm.
from lcm import *


"""Given a directory and LCM type name, return the generated class of that LCM
type."""
def get_lcm_message_class(lcmt_directory, lcmt_module_name):
    lcmtypes_dir = os.path.abspath(lcmt_directory)
    appended = False
    if lcmtypes_dir not in sys.path:
        sys.path.append(lcmtypes_dir)
        appended = True
    lcm_message_module = importlib.import_module(lcmt_module_name)
    if appended:
        sys.path.pop()
    return lcm_message_module.__dict__[
        [c for c in dir(lcm_message_module) if c.startswith("lcmt_")][0]]


"""Return a list of every available LCM message class."""
def get_all_lcm_message_classes(lcmt_directory):
    lcmtypes_dir = os.path.abspath(lcmt_directory)
    lcmtypes_parent_dir = os.path.dirname(lcmtypes_dir)
    lcmtypes_parent_name = os.path.basename(lcmtypes_dir)
    appended = False
    if lcmtypes_parent_dir not in sys.path:
        sys.path.append(lcmtypes_parent_dir)
        appended = True
    lcm_parent_module = importlib.import_module(lcmtypes_parent_name)
    print dir(lcm_parent_module)
    if appended:
        sys.path.pop()

    names = [name for name in dir(lcm_parent_module)
             if '__' not in name]
    # TODO(sam.creasey) This is a hack because the bazel version of
    # the lcm python library generator doesn't create an __init__.py
    # which imports the message classes.
    if len(names) == 0:
        names = [name[:-3] for name in os.listdir(lcmtypes_dir)
                 if '__' not in name and name.endswith(".py")]
    return [get_lcm_message_class(lcmt_directory, name)
            for name in names]


"""Prints the contents of an LCM message."""
def debug_print_msg(msg, outfile=None):
    if outfile is None:
        outfile = sys.stdout
    print >>outfile, msg, ":"
    for slot in msg.__slots__:
        print >>outfile, "  slot", slot, "value", getattr(msg, slot)
