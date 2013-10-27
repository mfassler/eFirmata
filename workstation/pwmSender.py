#!/usr/bin/env python
# -*- coding: UTF-8 -*-

import socket
import struct

deviceAddr = ('192.168.11.177', 2116)


mySocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM);
mySocket.connect(deviceAddr)


def setPWMs(reqPwmValues):

    # Packet structure is: 
    #  32 bit mask (1 means we want to set the value, 0 means leave it untouched)
    #  uint8_t values[32];  <-- 32 octets, one value for each PWM

    pwmMask = 0 # 32 bits
    pwmValues = [0] * 32 # one byte per channel

    for i, oneValue in enumerate(reqPwmValues):
        if type(oneValue) == int and oneValue >= 0 and oneValue <= 255:
            pwmValues[i] = oneValue
            pwmMask |= (1 << i)

    # convert to a string:
    pwmCmd = struct.pack('!L', pwmMask) + ''.join(map(chr, pwmValues))

    # Clear out any old data first:
    mySocket.setblocking(0)
    try:
        nothing = mySocket.recv(1522)
    except:
        pass
    else:
        while(nothing):
            try:
                nothing = mySocket.recv(1522)
            except:
                break
    mySocket.setblocking(1)
    # Incoming data should be empty.  Let us continue...


    # For the firmata-over-UDP protocol, we must declare ourselves to be "eFirmata".
    firmataOverUdpHeader = "eFirmata"
    # The firmata service we want: PWM
    firmataOverUdpHeader += "PWM"
    # Protocol version 0:
    firmataOverUdpHeader += "\x00"
    # No Flags, no options:
    firmataOverUdpHeader += "\x00\x00\x00\x00"
    # The firmata-over-udp header should be exactly 16 bytes:
    assert len(firmataOverUdpHeader) == 16, "BUG.  You broke it."

    mySocket.send(firmataOverUdpHeader + pwmCmd)



