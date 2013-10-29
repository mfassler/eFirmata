#!/usr/bin/env python
# -*- coding: UTF-8 -*-

import socket
import struct
import time

deviceAddr = ('192.168.11.177', 2115)

mySocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM);
mySocket.connect(deviceAddr)


firmataOverUdpHeader = "eFirmata"
firmataOverUdpHeader += "GPI"
firmataOverUdpHeader += "\x00"  # protocol version 0

# Empty out the input buffer
mySocket.setblocking(0)
try:
    while mySocket.recv(1522):
        pass
except:
    pass

mySocket.setblocking(1)


mySocket.settimeout(1.1)

while 1:
    mySocket.send(firmataOverUdpHeader)
    pins = mySocket.recv(1522)
    print "%x %x %x %x %x" % struct.unpack('!LLLLL', pins)
    time.sleep(0.1)

