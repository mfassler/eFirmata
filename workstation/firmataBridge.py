#!/usr/bin/python

## This code needs to run as root in order to have access to 
## ethernet frames.  

## On normal Posix-like systems, regular users don't have direct
## access to ethernet frames.  This code is a bridge that runs
## as root that gives access to eFirmata frames.  


import socket
import os
import fcntl
import struct
import select


whichNetworkDevice = 'eth0'

rootUID = 0

### Who are we giving access to?
userGID = 477  #apache



## From activestate.com/Ben_Mackey:
def getHwAddr(ifname):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    info = fcntl.ioctl(s.fileno(), 0x8927,  struct.pack('256s', ifname[:15]))
    return info[18:24]

myMACaddr = getHwAddr(whichNetworkDevice)


#### Incoming file connections:
## (We see these as "input", but the user will see these as "output")
inFileNameControl = '/tmp/eFirmataOutControl.sock'
inFileName = '/tmp/eFirmataOut.sock'
inFileNameFast = '/tmp/eFirmataOutFast.sock'

inFileSocketControl = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
inFileSocket = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
inFileSocketFast = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)

for fileName in [ inFileNameControl, inFileName, inFileNameFast ]:
    try:
        os.remove(fileName)
    except:
        pass

inFileSocketControl.bind(inFileNameControl)
inFileSocket.bind(inFileName)
inFileSocketFast.bind(inFileNameFast)

os.chown(inFileNameControl, rootUID, userGID)
os.chown(inFileName,        rootUID, userGID)
os.chown(inFileNameFast,    rootUID, userGID)

os.chmod(inFileNameControl, 0770)
os.chmod(inFileName,        0770)
os.chmod(inFileNameFast,    0770)


#### Outgoing file connections
## (We see these as "output" but the user will see these as "input")
### Note:  the user has to create these sockets.  We just look for them. 
class OutDataToUser:
    def __init__(self):
        self.fileNameControl = '/tmp/eFirmataInControl.sock'
        self.fileName = '/tmp/eFirmataIn.sock'
        self.fileNameFast = '/tmp/eFirmataInFast.sock'

        self.socketControl = None
        self.socket = None
        self.socketFast = None

    def tryToSend(self, packet):
        try:
            self.socket.send(packet)
        except:
            try:
                self.socket = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
                self.socket.connect(self.fileName)
            except:
                pass
            else:
                try:
                    self.socket.send(packet)
                except:
                    pass
    def tryToSendFast(self, packet):
        try:
            self.socketFast.send(packet)
        except:
            try:
                self.socketFast = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
                self.socketFast.connect(self.fileNameFast)
            except:
                pass
            else:
                try:
                    self.socketFast.send(packet)
                except:
                    pass


unixOut = OutDataToUser()


#### Network connections (in and out):
firmataControl = socket.socket(socket.AF_PACKET, socket.SOCK_RAW)
firmata        = socket.socket(socket.AF_PACKET, socket.SOCK_RAW)
firmataFast    = socket.socket(socket.AF_PACKET, socket.SOCK_RAW)

firmataControl.bind( (whichNetworkDevice, 0x181b) )
firmata.bind(        (whichNetworkDevice, 0x181c) )
firmataFast.bind(    (whichNetworkDevice, 0x181d) )

ethertypeControl = "\x18\x1b"
ethertype        = "\x18\x1c"
ethertypeFast    = "\x18\x1d"



readingList = [inFileSocketControl, inFileSocket, inFileSocketFast, 
               firmataControl, firmata, firmataFast]

while 1:
    inputs, outputs, errors = select.select(readingList, [], [], 2.1)
    if inputs:
        for oneInput in inputs:
            if oneInput == inFileSocketControl:
                print "inFileSocketControl:"
            if oneInput == inFileSocket:
                print "inFileSocket:"
                data, addr = oneInput.recvfrom(1522)
                dstAddr = data[:6]
                dstData = data[6:]
                ## TODO:  calculate CRC
                firmata.send(dstAddr + myMACaddr + ethertype + dstData)
            if oneInput == inFileSocketFast:
                print "inFileSocketFast:"
            if oneInput == firmataControl:
                print "firmataControl:"
            if oneInput == firmata:
                data, addr = oneInput.recvfrom(1522)
                unixOut.tryToSend(data)
            if oneInput == firmataFast:
                data, addr = oneInput.recvfrom(1522)
                unixOut.tryToSendFast(data)
    else:  # timeout, no incoming data
        print "bored."



