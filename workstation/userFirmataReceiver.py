#!/usr/bin/python

import socket
import struct
import os

def unpackSensorPacket(inPacket):
    sensorPacket = {}
    (sensorPacket['dest'],
     sensorPacket['src'],
     sensorPacket['prot'],
     sensorPacket['subProt'],
     sensorPacket['adcVal'],
     sensorPacket['xAccel0'],
     sensorPacket['yAccel0'],
     sensorPacket['zAccel0'],
     sensorPacket['xAccel1'],
     sensorPacket['yAccel1'],
     sensorPacket['zAccel1'],
     sensorPacket['happyMessage'],
     sensorPacket['fcs'] ) = struct.unpack('6s6s2s4sHHHHHHH24sI', inPacket)
    return sensorPacket


## MAC address of the eFirmata right here next to the keyboard:
nextToKeybd = '\x00\x02\xf7\xaa\xbf\xcd'

## MAC address of the eFirmata over there on the table:
onTheTable = '\x00\x02\xf7\xaa\xff\xee'


##############################################################
####
###  BEGIN: create (and bind to) the socket for receiving data
##
fileName = '/tmp/eFirmataIn.sock'

try:
    os.remove(fileName)
except:
    pass

mySocket = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
mySocket.bind( '/tmp/eFirmataIn.sock' )

##
###  END: create (and bind to) the socket for receiving data
####
##############################################################

recvPacket = {}

while 1:
    data, addr = mySocket.recvfrom(1522)
    recvPacket = unpackSensorPacket(data[:60])
    if recvPacket['src'] == nextToKeybd:
        print recvPacket


