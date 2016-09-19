#!/usr/bin/env python

import argparse
import sys

import lcm_adapter as lcm

if len(sys.argv) < 2:
    print """The first argument to this command must be the directory
             of the generated python files for LCM messages."""
    sys.exit(1)

MESSAGE_CLASSES = lcm.get_all_lcm_message_classes(sys.argv[1])


"""Sets up all of the LCMSubscription objects for the channels indicated in
the arguments."""
def set_up_subscriptions(lcm_connection, args):
    def handle_message(channel, message):
        decoded = try_decode(message)
        if decoded is None:
            print "Received unreadable message on channel", channel
            return
        lcm.debug_print_msg(decoded)

    for channel in args.channel:
        print "subscribing to channel", channel
        lcm_connection.subscribe(channel, handle_message)

"""Try to decode the message with each known message class; return the first
successful decode, or None."""
def try_decode(message):
    for c in MESSAGE_CLASSES:
        try:
            return c.decode(message)
        except ValueError:
            pass  # The message was probably of a different type.
    return None

def main(argv):
    parser = argparse.ArgumentParser(description='Log some local LCM traffic.')
    parser.add_argument('-f', '--file', metavar='LOGFILE',
                        type=argparse.FileType('w'), default=sys.stdout,
                        help="File name for lcm log; default is stdout.")
    parser.add_argument('channel', nargs="+", metavar='CHANNEL')
    args = parser.parse_args(argv[2:])
    lcm_connection = lcm.LCM()
    set_up_subscriptions(lcm_connection, args)
    while True:
        lcm_connection.handle()

if __name__ == "__main__":
    main(sys.argv)
