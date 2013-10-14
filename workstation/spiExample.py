#!/usr/bin/env python
# -*- coding: UTF-8 -*-

import socket
import select
import time
import signal
import sys
import struct

deviceAddr = ('192.168.11.177', 2116)

mySocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM);
mySocket.connect(deviceAddr)


def getAllData():
    global mySocket
    allData = []
    mySocket.setblocking(0)
    try:
        data = mySocket.recv(1522)
    except:
        pass
    else:
        allData.append(data)
        while (data):
            try:
                data = mySocket.recv(1522)
            except:
                break
            else:
                allData.append(data)
    mySocket.setblocking(1)
    return ''.join(allData)


#mySocket.send("eFirmataSPI\x00\x1c        ;lkj ;lkj ;lkj  asdfasdf")


def sendSPI(msg):
    wait = 0
    CPOL = 1
    CPHA = 1

    flags = (wait << 2) | (CPOL << 1) | (CPHA << 0)

    firmataOverUdpHeader = "eFirmata"
    firmataOverUdpHeader += "SPI"
    firmataOverUdpHeader += "\x00"  # protocol version 0
    firmataOverUdpHeader += "\x00\x00\x00"  # reserved for future use
    firmataOverUdpHeader += chr(flags)

    return mySocket.send(firmataOverUdpHeader + msg[:255])



def sampleFromMcp(whichChannel):
    # This is for use with MCP3208 over SPI
    # The MCP3208 is an 8-channel ADC (12-bit)

    # Empty out our UDP socket
    getAllData()

    # From the data sheet... we have to think in bits, not bytes.
    # We need:
    #   5 bits of zero
    #   1 start bit (a "one")
    #   1 diff/single bit ("one" for 8 single inputs, or "zero" for 4 differential inputs)
    #       (this can be done on a channel by channel basis...)
    #   3 bits to select the channel (msb first)
    #   14 bits of anything, to bring our data back home

    cmdA = 0x06 | ((whichChannel & 0x04) >> 2)
    cmdB = (whichChannel & 0x03) << 6
    cmdC = 0

    cmd = chr(cmdA) + chr(cmdB) + chr(cmdC)

    sendSPI(cmd)

    mySocket.settimeout(0.2)
    resp = mySocket.recv(len(cmd))
    mySocket.settimeout(None)

    if len(resp):
        value = ((ord(resp[1]) & 0x0f) << 8) + (ord(resp[2]))
        return value
    else:
        return None



def run():
    getAllData()
    values = [0] * 8
    while 1:
        for i in [4,5,6,7]:
            values[i] = sampleFromMcp(i)
            #time.sleep(0.01)
        print values


run()

