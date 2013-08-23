#!/usr/bin/env python
# -*- coding: UTF-8 -*-

import socket

deviceAddr = ('192.168.11.177', 2115)


mySocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM);
mySocket.connect(deviceAddr)


def setPWMs(pwmValues):

    # Packet structure is: 
    #  32 bit mask (1 means we want to set the value, 0 means leave it untouched)
    #  uint8_t values[32];  <-- 32 octets, one value for each PWM

    pwmMask = '\x00\x00\x00\x3f' # we will set the first 6 PWMs
    pwmValues0_5 = ''.join(map(chr, pwmValues))
    pwmValues6_31 = '\x00' * 26

    pwmCmd = pwmMask + pwmValues0_5 + pwmValues6_31


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



