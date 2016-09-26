#!/usr/bin/env python

"""Maps joystick axes to lcm message parameters.  This must typically be
wrapped in a script that sets LD_LIBRARY_PATH and PYTHONPATH and provides a
path and classname for the LCM codegen output.
"""

import argparse
import copy
import pygame
import subprocess
import sys
import time

if len(sys.argv) < 3:
    print """The first two arguments to this command must be the directory
             and module name of the python module providing the LCM type
             that you wish to map to the joystick."""
    sys.exit(1)

import lcm_adapter as lcm

LCMTYPES_DIR = sys.argv[1]
LCM_MODULE_NAME = sys.argv[2]
lcm_message_class = lcm.get_lcm_message_class(LCMTYPES_DIR, LCM_MODULE_NAME)


class JoystickEventProcessor:
    def __init__(self):
        # TODO(ggould-tri) Support other numbers of joysticks?
        assert pygame.joystick.get_count() == 1
        self.joystick = pygame.joystick.Joystick(0)
        self.joystick.init()
        self.mappings = {}
        self.quit_on = None

    def add_axis_mapping(self, joy_axis, message_field, offset, scale):
        self.mappings[joy_axis] = (message_field, offset, scale)

    def set_quit_button(self, button):
        self.quit_on = button

    # TODO(ggould-tri) Support button mappings?

    def process_event(self, event, last_msg):
        new_msg = copy.copy(last_msg)

        if event.type == pygame.JOYAXISMOTION:
            if event.axis in self.mappings:
                (message_field, offset, scale) = self.mappings[event.axis]
                setattr(new_msg, message_field, offset + (scale * event.value))
        elif event.type == pygame.JOYBUTTONDOWN:
            if event.button == self.quit_on:
                pygame.quit()
                sys.exit()

        # Magically fill in the field named "timestamp" if any.
        if hasattr(new_msg, "timestamp"):
            new_msg.timestamp = int(time.time() * 1e6)
        return new_msg


class Publisher:
    def __init__(self, lcm_channel_name, event_processor):
        self.last_msg = lcm_message_class()
        self.lc = lcm.LCM()
        self.lcm_channel_name = lcm_channel_name
        self.event_processor = event_processor
        print 'Ready'

    def start(self):
        while True:
            event = pygame.event.wait()
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()
            else:
                self.last_msg = self.event_processor.process_event(
                    event, self.last_msg)
                self.lc.publish(self.lcm_channel_name, self.last_msg.encode())


def main():
    pygame.init()

    # We parsed the first two arguments (the directory and module name for our
    # LCM class) already; we will re-parse them here for completeness and
    # documentation clarity.
    #
    # Our remaining arguments map joystick axes to LCM fields.  These are
    # entered via "--mapping" clusters written thus:
    #  --mapping axis fieldname offset scale
    # For instance:
    #  --mapping 0 force -1 2
    # Creates a mapping of axis 0 to the "force" field of the LCM message,
    # with the axis range 0..1 remapped to the force range of -1..1.


    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument(
      '--lcm_tag', default='DRIVING_COMMAND',
      help='tag ("channel") to publish the LCM messages with')
    parser.add_argument(
        '--mapping', nargs=4, action='append',
        help=('Takes four arguments, an axis, message field, offset, and '
              'scale; creates a mapping of the given joystick axis to the '
              'named LCM field.'))
    parser.add_argument(
        '--quit-on', type=int, default=9,
        help=("button to cause this program to quit; by default uses button "
              "9 which is the start button on logitech sticks."))

    args = parser.parse_args(sys.argv[3:])

    driver = JoystickEventProcessor()
    for mapping in args.mapping:
        (axis, field, offset, scale) = mapping
        driver.add_axis_mapping(int(axis), field, float(offset), float(scale))
    driver.set_quit_button(args.quit_on)

    publisher = Publisher(args.lcm_tag, driver)
    publisher.start()

    print "stop"
    return 0


if __name__ == '__main__':
    sys.exit(main())
