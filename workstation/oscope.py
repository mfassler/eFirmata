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
#redStripe_addr = "\x00\x02\xf7\xaa\xff\xee"
deviceEthernetAddress = "\x00\x02\xf7\xaa\xff\xee"


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



TRIGGERMODE_OFF = 0
TRIGGERMODE_NOW = 1
TRIGGERMODE_RISING = 2
TRIGGERMODE_FALLING = 3
TRIGGERMODE_CONTINUOUS = 4

# Trigger channel should be the ADC channel (1,2,4,5)
def getTriggeredSample(channel, threshold, triggerModeStr):

    if triggerModeStr == "off":
        trigMode = TRIGGERMODE_OFF
    elif triggerModeStr == "now":
        trigMode = TRIGGERMODE_NOW
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

    sMax = trigNumFramesReq * 256 # maximum number of samples

    numChannels = 4
    chA = np.zeros(sMax)
    chB = np.zeros(sMax)
    chC = np.zeros(sMax)
    chD = np.zeros(sMax)

    # Packet structure is: Mode, Channel, Threshold, NumFramesRequested
    trigCmd = chr(trigMode) + chr(channel & 0xff) + chr(threshold & 0xff) + chr(trigNumFramesReq)


    # Clear out any old data first:
    mySocketRx.setblocking(0)
    try:
        nothing = mySocketRx.recv(1522)
    except:
        pass
    else:
        while(nothing):
            try:
                nothing = mySocketRx.recv(1522)
            except:
                break
    mySocketRx.setblocking(1)
    # Incoming data should be empty.  Let us continue...


    fileSocketControl.send(deviceEthernetAddress + trigCmd)

    ## Right now, things are hard-coded for four channels, 256 samples per
    ## ethernet frame (so the data portion that we care about is always 1024 bytes)
    ## On each frame, the data is striped:  chA, chB, chC, chD, chA, chB, chC, chD, chA, ...
    #DPACKET_SIZE = 1024
    sStart = 0
    sEnd = sStart + 256

    readingList = [mySocketRx]

    while 1:
        inputs, outputs, errors = select.select(readingList, [], [], 1.1)
        if inputs:
            for oneInput in inputs:
                if oneInput == mySocketRx:
                    inPacket = mySocketRx.recv(1522)
                    if len(inPacket) > 20:
                        # These boundaries, 14:1038, are hard-coded into the eFirmata protocol
                        stripedData = np.frombuffer(inPacket[14:1038], dtype=np.uint8)
                        chA[sStart:sEnd] = stripedData[::4] #.copy()
                        chB[sStart:sEnd] = stripedData[1::4] #.copy()
                        chC[sStart:sEnd] = stripedData[2::4] #.copy()
                        chD[sStart:sEnd] = stripedData[3::4] #.copy()
                        sStart += 256
                        sEnd = sStart + 256
        else:
            print "timeout waiting for packet"
            break
        if sEnd > sMax:
            break

    return chA, chB, chC, chD


def matplotlibStyle():
    plt.ion()
    fig = plt.figure()
    ax = fig.add_subplot(111)
    plt.ylim([-14, 270]) # little bit of whitespace for visual margin

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
        oldChA = chA
        oldChB = chB
        oldChC = chC
        #oldChD = chD
        try:
            chA, chB, chC, chD = getTriggeredSample(1, 100, 'rising')
            chA = 266 - chA #flip y axis; add 10px margin
            chB = 266 - chB #flip y axis; add 10px margin
            chC = 266 - chC #flip y axis; add 10px margin
            #chD = 266 - chD #flip y axis; add 10px margin
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
    chA, chB, chC, chD = getTriggeredSample(1, 100, 'now')

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
        oldChA = chA
        oldChB = chB
        try:
            chA, chB, chC, chD = getTriggeredSample(1, 100, 'now')
            chA = chA + 10 # 10px of margin
            chB = 266 - chB  # flip y-axis, plus 10px of margin
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

