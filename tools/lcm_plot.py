#!/usr/bin/env python

"""Plot various fields of a csv dump of an LCM stream.  For each field
requested, the field will be the y axis and the 'timestamp' field of the same
message will be the x axis.
"""
# TODO(ggould-tri) replace this with tlogplot if that is ever open-sourced.

import argparse
import csv
import matplotlib.pyplot as plt
import sys

def plot_events(events, fields_to_plot):
    """Takes the output of `extract_events` and pyplots the fields of that
    output that match @p fields_to_plot (a list of fully-qualified field
    names).
    """
    field_indices = {field_name: index
                     for (index, field_name) in enumerate(fields_to_plot)}
    field_xy_pairs = {field_name: []
                      for (index, field_name) in enumerate(fields_to_plot)}
    x_bounds = [float("inf"), float("-inf")]
    y_bounds = [float("inf"), float("-inf")]
    for event in events:
        timestamp = event["timestamp"]
        for event_field_name, event_field_value in event.iteritems():
            if event_field_name in field_xy_pairs:
                xy_pair = (int(timestamp), float(event_field_value))
                field_xy_pairs[event_field_name].append(xy_pair)
                x_bounds[0] = min(xy_pair[0], x_bounds[0])
                x_bounds[1] = max(xy_pair[0], x_bounds[1])
                y_bounds[0] = min(xy_pair[1], y_bounds[0])
                y_bounds[1] = max(xy_pair[1], y_bounds[1])
    for field_name, xy_pairs in field_xy_pairs.iteritems():
        plt.plot([pair[0] for pair in xy_pairs],
                 [pair[1] for pair in xy_pairs])
    plt.axis(x_bounds + y_bounds)

def extract_events(filehandle):
    """Reads csv-formatted messages (defined by header rows) from the given
    @p filehandle and yields each as a dict of { CHANNEL.type.field -> value }.
    """
    reader = csv.reader(filehandle)
    message_fields = {}  # A map of { typename -> [fieldname, ...] }.
    for row in reader:
        if len(row) < 2:  # Ill-formed row; discard.
            continue
        channel, message_type = row[:2]
        if not channel:  # This is a header row.
            message_fields[message_type] = row[2:]
        elif message_type in message_fields:
            fieldnames = message_fields[message_type]
            fieldname_prefix = channel + "." + message_type + "."
            event = {fieldname_prefix + fieldnames[i]: value
                     for (i, value) in enumerate(row[2:])}
            # Inject an unqualified "timestamp" field for ease of processing.
            timestamp_idx = fieldnames.index("timestamp")
            try:
                event["timestamp"] = row[timestamp_idx + 2]
                yield event
            except IndexError:
                pass  # Misformatted entry, usually an end truncation.
        else:
            pass  # Ignore unknown message types.

def main(argv):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        'csv_file', type=argparse.FileType('r'),
        help="CSV file to read data from")
    parser.add_argument(
        '-f', '--field', action='append',
        help="Field name to plot, in channel.message_type.fieldname form")
    args = parser.parse_args(argv[1:])
    events = extract_events(args.csv_file)
    plot_events(events, args.field)
    plt.show()


if __name__ == "__main__":
    main(sys.argv)
