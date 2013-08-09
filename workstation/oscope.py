#!/usr/bin/env python
# -*- coding: UTF-8 -*-

import socket
import os
import select
import time

import numpy as np

# The oscilloscop display is rendered in either Matplotlib or Pygame
# (pygame is faster, but matplotlib has more features for less code)
import matplotlib.pyplot as plt
import pygame


# The Ethernet MAC address of the eFirmata we are talking to:
redStripe_addr = "\x00\x02\xf7\xaa\xff\xee"


### firmataControl to send our request
### firmataFast to receive our data


##############################################################
####
###  BEGIN: create (and bind to) the socket for receiving data
##
fileName = '/tmp/eFirmataInFast.sock'

try:
    os.remove(fileName)
except:
    pass

mySocketRx = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
mySocketRx.bind( fileName )

##
###  END: create (and bind to) the socket for receiving data
####
##############################################################



# Our socket for sending data (the request for triggered ADC samples):
#
fileNameControl = '/tmp/eFirmataOutControl.sock'
fileSocketControl = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
fileSocketControl.connect(fileNameControl)


readingList = [mySocketRx]

# Trigger channel should be the ADC channel (1,2,4,5)
def getTriggeredSample(channel, threshold, direction):
    # packet structure is:  Level, Direction, Channel, Enabled
    # (enabled should always be 1)
    trigDir = direction and "\x01" or "\x00"
    trigCmd = chr(threshold & 0xff) + trigDir + chr(channel & 0xff) + "\x01"
    fileSocketControl.send(redStripe_addr + trigCmd)
    numPacketsReceived = 0
    myPackets = []
    while 1:
        if numPacketsReceived > 3:
            break
        inputs, outputs, errors = select.select(readingList, [], [], 11.5)
        if inputs:
            for oneInput in inputs:
                if oneInput == mySocketRx:
                    numPacketsReceived += 1
                    inPacket, addr = mySocketRx.recvfrom(1522)
                    if len(inPacket) > 20:
                        myPackets.append(inPacket[14:])
        else: #timeout
            break
    oneRun = myPackets[0][:1024] + myPackets[1][:1024]  # ignoring the next two packets for now
    chA = np.array(map(ord, oneRun[::4]))
    chB = np.array(map(ord, oneRun[1::4]))
    chC = np.array(map(ord, oneRun[2::4]))
    chD = np.array(map(ord, oneRun[3::4]))
    return chA, chB, chC, chD


def matplotlibStyle():
    plt.ion()
    fig = plt.figure()
    ax = fig.add_subplot(111)
    plt.ylim([-14, 270])

    # First time:
    chA, chB, chC, chD = getTriggeredSample(1, 100, 1)
    lineA, = ax.plot(chA, 'r,')
    lineB, = ax.plot(chB, 'g,')
    lineC, = ax.plot(chC, 'b')
    #lineD, = ax.plot(chD, 'y,')

    fig.canvas.draw()

    while 1:
        # every subsequent time:
        try:
            chA, chB, chC, chD = getTriggeredSample(1, 100, 1)
            lineA.set_ydata(chA)
            lineB.set_ydata(chB)
            lineC.set_ydata(chC)
            #lineD.set_ydata(chD)
        except:
            time.sleep(0.1)
        else:
            fig.canvas.draw()



# TODO: FIXME:  the width of this needs to come from somewhere...
wholeImageBuffer = np.ones((1024,276, 3), np.int32) * 255

def pygameStyle():
    chA, chB, chC, chD = getTriggeredSample(1, 100, 0)
    screenRez = (len(chA)*2, 276)
    screen = pygame.display.set_mode(screenRez, 0)

    colorBG = (255,255,255)
    colorA = (255,0,0)
    colorB = (0,255,0)
    colorC = (0,0,255)

    while 1:
        oldChA = chA #.copy()
        oldChB = chB #.copy()
        oldChC = chC #.copy()
        try:
            chA, chB, chC, chD = getTriggeredSample(1, 100, 0)
            chA = 266 - chA
            chB = 266 - chB
            chC = 266 - chC
            #chD = 266 - chD
        except:
            time.sleep(0.1)
            print ".",
        else:
            for i in xrange(len(chA)):
                iEvn = i*2
                iOdd = i*2+1
                wholeImageBuffer[iEvn, oldChA[i]] = colorBG
                wholeImageBuffer[iOdd, oldChA[i]] = colorBG
                wholeImageBuffer[iEvn, chA[i]] = colorA
                wholeImageBuffer[iOdd, chA[i]] = colorA
                wholeImageBuffer[iEvn, oldChB[i]] = colorBG
                wholeImageBuffer[iOdd, oldChB[i]] = colorBG
                wholeImageBuffer[iEvn, chB[i]] = colorB
                wholeImageBuffer[iOdd, chB[i]] = colorB
                wholeImageBuffer[iEvn, oldChC[i]] = colorBG
                wholeImageBuffer[iOdd, oldChC[i]] = colorBG
                wholeImageBuffer[iEvn,  chC[i]] = colorC
                wholeImageBuffer[iOdd,  chC[i]] = colorC
            pygame.surfarray.blit_array(screen, wholeImageBuffer)
            pygame.display.flip()

matplotlibStyle()
#pygameStyle()

