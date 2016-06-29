#! /usr/bin/python

import smbus
import time

bus = smbus.SMBus(1)

addr = 0x22

port_addrs = {
	1: 0x81,
	2: 0x82,
	3: 0x83,
	4: 0x84,
}

CPR = 100
PPR = 4 * CPR

prescaler = bus.read_byte_data(addr, 0x00)

if prescaler < 2 or prescaler > 10:
	raise Exception("oh really")

# 8080000: apporox atmega328p cpu frequency
# 1024:    timer1 divisor
# 16:      number of circular buffer cells of the time window
time_window = (1./8080000) * 1024 * (1 << prescaler) * 16

print "prescaler", prescaler
print "time_window", time_window

while True:
	for port_id in range(1, 5):
		regs = port_addrs[port_id]
		data = bus.read_i2c_block_data(addr, regs, 4)
		skipped_ticks, double_states, ppw_high, ppw_low = data
		ppw = (ppw_high << 8) + ppw_low
		# it's a signed int16
		if ppw > 32767:
			ppw -= 65536
		print port_id, ppw / time_window / PPR
	print
	time.sleep(1)
