#!/usr/bin/python

#
# Example user-level code to receive streaming audio from an
# eFirmata.  
#
# Assume that there is a microphone attached to AD0.5 of the
# eFirmata.  Assume that the data is being sent to us using
# the eFirmataFast protocol. 
#
# This code should be called like so:
#
#  python rawSound.py | aplay -B 0.005 -f U16_LE -r 24414

## (By my calculations, the sample rate from the eFirmata should
## be about 24038 KSPS, but aplay likes it at about 24400 KSPS.
## Not sure why...) 

import socket
import struct
import sys
import os

def unpackSoundPacket(inPacket):
    soundPacket = {}
    (soundPacket['dest'],
     soundPacket['src'],
     soundPacket['prot'],
     soundPacket['data'],
     soundPacket['fcs'] ) = struct.unpack('6s6s2s512sI', inPacket)
    return soundPacket


## MAC address of the eFirmata right here next to the keyboard:
nextToKeybd = '\x00\x02\xf7\xaa\xbf\xcd'

## MAC address of the eFirmata over there on the table:
onTheTable = '\x00\x02\xf7\xaa\xff\xee'


##############################################################
####
###  BEGIN: create (and bind to) the socket for receiving data
##
fileName = '/tmp/eFirmataInFast.sock'

try:
    os.remove(fileName)
except:
    pass

mySocket = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
mySocket.bind( '/tmp/eFirmataInFast.sock' )

##
###  END: create (and bind to) the socket for receiving data
####
##############################################################




while 1:
    data, addr = mySocket.recvfrom(1522)
    recvPacket = unpackSoundPacket(data[:532])
    if recvPacket['src'] == nextToKeybd:
        sys.stdout.write(recvPacket['data'])
        sys.stdout.flush()


