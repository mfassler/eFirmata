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

TRIGGERMODE_OFF = 0
TRIGGERMODE_ONESHOT = 1
TRIGGERMODE_RISING = 2
TRIGGERMODE_FALLING = 3
TRIGGERMODE_CONTINUOUS = 4

# Trigger channel should be the ADC channel (1,2,4,5)
def getTriggeredSample(channel, threshold, triggerModeStr):

    if triggerModeStr == "off":
        trigMode = TRIGGERMODE_OFF
    elif triggerModeStr == "oneshot":
        trigMode = TRIGGERMODE_ONESHOT
    elif triggerModeStr == "rising":
        trigMode = TRIGGERMODE_RISING
    elif triggerModeStr == "falling":
        trigMode = TRIGGERMODE_FALLING
    elif triggerModeStr == "continuous":
        trigMode = TRIGGERMODE_CONTINUOUS
    else:
        trigMode = TRIGGERMODE_OFF

    # a "frame" is 256 samples.  So number of samples we want is number of frames*256
    trigNumFramesReq = 2  #we want 512 samples

    # Packet structure is: Mode, Channel, Threshold, NumFramesRequested
    trigCmd = chr(trigMode) + chr(channel & 0xff) + chr(threshold & 0xff) + chr(trigNumFramesReq)

    fileSocketControl.send(redStripe_addr + trigCmd)
    numPacketsReceived = 0
    myPackets = []
    while 1:
        if numPacketsReceived >= trigNumFramesReq:
            break
        inputs, outputs, errors = select.select(readingList, [], [], 1.1)
        if inputs:
            for oneInput in inputs:
                if oneInput == mySocketRx:
                    numPacketsReceived += 1
                    inPacket, addr = mySocketRx.recvfrom(1522)
                    if len(inPacket) > 20:
                        myPackets.append(inPacket[14:])
        else: #timeout
            print "timeout waiting for packet"
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
    chA, chB, chC, chD = getTriggeredSample(1, 100, 'rising')
    lineA, = ax.plot(chA, 'r,')
    lineB, = ax.plot(chB, 'g,')
    lineC, = ax.plot(chC, 'b')
    #lineD, = ax.plot(chD, 'y,')

    fig.canvas.draw()

    while 1:
        # every subsequent time:
        try:
            chA, chB, chC, chD = getTriggeredSample(1, 100, 'rising')
            lineA.set_ydata(chA)
            lineB.set_ydata(chB)
            lineC.set_ydata(chC)
            #lineD.set_ydata(chD)
        except:
            time.sleep(0.1)
        else:
            fig.canvas.draw()



def pygameStyle():
    chA, chB, chC, chD = getTriggeredSample(1, 100, 'rising')

    # The window is 256px tall, but we add 10 pixels of margin (top,bottom) to
    # make it look nicer
    screenRez = (len(chA)*2, 276)
    screenRezColor = (len(chA)*2, 276, 3)

    wholeImageBuffer = np.ones(screenRezColor, np.int32) * 255
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
            chA, chB, chC, chD = getTriggeredSample(1, 100, 'oneshot')
            chA = 266 - chA #flip y axis; add 10px margin
            chB = 266 - chB #flip y axis; add 10px margin
            chC = 266 - chC #flip y axis; add 10px margin
            #chD = 266 - chD
        except:
            time.sleep(0.1)
            print ".",
        else:
            for i in xrange(len(chA)):
                iEvn = i*2
                iOdd = i*2+1
                # We use colorBG to erase the old pixels:
                wholeImageBuffer[iEvn, oldChA[i]] = colorBG
                wholeImageBuffer[iOdd, oldChA[i]] = colorBG
                # Draw the new pixels:
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


def pygameXYscope():
    chA, chB, chC, chD = getTriggeredSample(1, 100, "oneshot")

    # The window is 256x256, but we add 10 pixels of margin (top,bottom,r,l) to
    # make it look nicer
    screenRez = (276, 276)
    screenRezColor = (276, 276, 3)

    wholeImageBuffer = np.ones(screenRezColor, np.int32) * 255
    screen = pygame.display.set_mode(screenRez, 0)

    colorBG = (255,255,255)
    colorA = (255,0,0)
    colorB = (0,255,0)
    colorC = (0,0,255)
    while 1:
        oldChA = chA #.copy()
        oldChB = chB #.copy()
        try:
            chA, chB, chC, chD = getTriggeredSample(1, 100, 'oneshot')
            chA = chA + 10 # 10px of margin
            chB = 266 - chB #flip y-axis, plus 10px of margin
        except:
            time.sleep(0.1)
            print ".",
        else:
            # Erase the old pixels:
            for i in xrange(len(chA)):
                x = oldChA[i]
                y = oldChB[i]
                wholeImageBuffer[x,y] = colorBG
            # Draw the new pixels:
            for i in xrange(len(chA)):
                x = chA[i]
                y = chB[i]
                wholeImageBuffer[x,y] = colorA
            pygame.surfarray.blit_array(screen, wholeImageBuffer)
            pygame.display.flip()


matplotlibStyle()
#pygameStyle()
#pygameXYscope()

