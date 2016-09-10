#!/usr/bin/python

import lcm
import schunk_driver
from schunk_driver.lcmt_schunk_command import lcmt_schunk_command
import sys

if len(sys.argv) != 4:
	print "Usage: publish_schunk_command <position, mm> <force lim, N> <speed, mm/s>"
	sys.exit()

msg = lcmt_schunk_command()
msg.timestamp = 0
msg.target_position_mm = float(sys.argv[1])
msg.force = float(sys.argv[2])
msg.target_speed_mm_per_s = float(sys.argv[3])
lc = lcm.LCM()
lc.publish("WSG_POSITION_COMMAND", msg.encode())