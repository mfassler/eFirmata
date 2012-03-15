#!/usr/bin/python

import socket

# eFirmata at my computer desk:
desk_addr = "\x00\x02\xf7\xaa\xbf\xcd"
# eFirmata on the table connected to the actual lights:
table_addr = "\x00\x02\xf7\xaa\xff\xee"

fileName = '/tmp/eFirmataOut.sock'

fileSocket = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
fileSocket.connect(fileName)



## Set the PWMs (which then controls the brightness of the lights):
## (The "dead monkeys" is filler which serves no purpose.)

# All off:
#fileSocket.send(table_addr + "S\x11 \x00\x00\x00\x00\x00\x00 Dead Monkeys and stuff.")
#fileSocket.send(desk_addr + "S\x11 \x00\x00\x00\x00\x00\x00 Dead Monkeys and stuff.")

# All on:
#fileSocket.send(table_addr + "S\x11 \xff\xff\xff\xff\xff\xff Dead Monkeys and stuff.")
fileSocket.send(desk_addr + "S\x11 \xff\xff\xff\xff\xff\xff Dead Monkeys and stuff.")


