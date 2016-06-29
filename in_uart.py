#! /usr/bin/python

import serial
import time
import argparse


parser = argparse.ArgumentParser()
parser.add_argument("--notx", help="use this flag when tx line is not connected and auto_print != 0", action="store_true")
args = parser.parse_args()

port = serial.Serial("/dev/ttyUSB0", baudrate=1000000, timeout=3.0)

CPR = 100
PPR = 4 * CPR

prescaler = 6

while not args.notx:
	port.write("set prescaler %d\r\n" % prescaler)
	if "OK" == port.readline().strip():
		break

# 8080000: apporox atmega328p cpu frequency
# 1024:    timer1 divisor
# 16:      number of circular buffer cells of the time window
time_window = (1./8080000) * 1024 * (1 << prescaler) * 16

print "prescaler", prescaler
print "time_window", time_window

while True:
	if not args.notx:
		port.write("\r\n")
	line = port.readline().strip()
	ports = line.split(' ')
	if len(ports) != 4:
		continue
	for p in ports:
		kv = p.split('=')
		if len(kv) != 2:
			break
		port_id, rest = kv
		if len(port_id) != 1:
			break
		numbers = rest.split(',')
		port_id = int(port_id)
		skipped_ticks, double_states = int(numbers[0], 16), int(numbers[1], 16)
		ppw = int(numbers[2], 16)
		# it's a signed int16
		if ppw > 32767:
			ppw -= 65536
		print port_id, ppw / time_window / PPR
	if not args.notx:
		time.sleep(1.0)

