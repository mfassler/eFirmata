#!/usr/bin/env python
# -*- coding: UTF-8 -*-

import socket
import struct
import time

deviceAddr = ('192.168.11.177', 2114)

mySocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM);
mySocket.connect(deviceAddr)


firmataOverUdpHeader = "eFirmata"
firmataOverUdpHeader += "GPO"
firmataOverUdpHeader += "\x00"  # protocol version 0

# Empty out the input buffer
mySocket.setblocking(0)
try:
    while mySocket.recv(1522):
        pass
except:
    pass

mySocket.setblocking(1)



# Blinky lights, on and off...
while 1:
    mySocket.send(firmataOverUdpHeader + "\x00\x00\x03\xc3\x00\x00\x00\x00")
    time.sleep(0.3)
    mySocket.send(firmataOverUdpHeader + "\x00\x00\x03\xc3\x00\x00\x03\xc3")
    time.sleep(0.3)


