#!/usr/bin/env python

import argparse
import sys

import lcm_adapter as lcm

if len(sys.argv) < 2:
    print """The first argument to this command must be the directory
             of the generated python files for LCM messages."""
    sys.exit(1)

MESSAGE_CLASSES = lcm.get_all_lcm_message_classes(sys.argv[1])


def set_up_subscriptions(lcm_connection, args):
    """Sets up all of the LCMSubscription objects for the channels indicated in
    the arguments."""
    if args.format == "lcm":
        lcm_logger = lcm.EventLog(args.logfile, mode='w', overwrite=True)
    else:
        # Deliberately leaked; we'll let the GC handle the close() for us.
        logfile = (sys.stdout if args.logfile is None
                   else open(args.logfile, 'w'))
        write_csv_headers(logfile)

    def handle_message(channel, message):
        decoded = try_decode(message)
        if decoded is None:
            print "Received unreadable message on channel", channel
            return
        if args.format == "lcm":
            lcm_logger.write_event(decoded.timestamp, channel, message)
        elif args.format == "csv":
            fields = ([channel, type(decoded).__name__] +
                      [getattr(decoded, slot)
                       for slot in sorted(decoded.__slots__)])
            logfile.write(",".join(['"%s"' % f for f in fields]) + "\n")
        elif args.format == "pretty":
            lcm.debug_print_msg(decoded, logfile)
        else:
            assert False

    for channel in args.channel:
        print "subscribing to channel", channel
        lcm_connection.subscribe(channel, handle_message)

def try_decode(message):
    """Try to decode the message with each known message class; return
    the first successful decode, or None."""
    for c in MESSAGE_CLASSES:
        try:
            return c.decode(message)
        except ValueError:
            pass  # The message was probably of a different type.
    return None

def write_csv_headers(logfile):
    """Write header lines in the CSV file with the schema of the messages
    involved."""
    for c in MESSAGE_CLASSES:
        header_prefix = ["", c.__name__]
        header_elements = sorted(c.__slots__)
        logfile.write(",".join(
            ['"%s"' % h for h in (header_prefix + header_elements)]) + "\n")

def main(argv):
    parser = argparse.ArgumentParser(description='Log some local LCM traffic.')
    parser.add_argument(
        '-l', '--logfile', metavar='LOGFILE', type=str, default=None,
        help="File name for lcm log; default is stdout.")
    parser.add_argument(
        '-f', '--format', default='lcm', choices=('lcm', 'csv', 'pretty'),
        help="Log format: 'lcm' (binary), 'csv', or 'pretty' (human-readable)")
    parser.add_argument('channel', nargs="+", metavar='CHANNEL')
    args = parser.parse_args(argv[2:])

    lcm_connection = lcm.LCM()
    set_up_subscriptions(lcm_connection, args)
    while True:
        lcm_connection.handle()

if __name__ == "__main__":
    main(sys.argv)
