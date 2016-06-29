#! /usr/bin/python

import RPi.GPIO as GPIO
from time import sleep

GPIO.setmode(GPIO.BCM)

GPIO.setup(17, GPIO.OUT)
GPIO.setup(27, GPIO.OUT)

n = 0.00001

while True:
	GPIO.output(17, True)
	sleep(n)
	GPIO.output(27, True)
	sleep(n)
	GPIO.output(17, False)
	sleep(n)
	GPIO.output(27, False)
	sleep(n)
