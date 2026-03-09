#!/usr/bin/env python3

import serial

ser = serial.Serial("/dev/cu.usbmodem11302", 38400, timeout=1)

while True:
    line = ser.readline().decode("ascii", errors="ignore").strip()
    if line:
        print(line)
