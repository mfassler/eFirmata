#!/usr/bin/env python3

import socket
import select
import time
import signal
import sys
import struct

import numpy as np

# The oscilloscop display is rendered in either Matplotlib or Pygame
# (pygame is faster, but matplotlib has more features for less code)
import matplotlib.pyplot as plt
#import pygame


deviceAddr = ('192.168.11.177', 2117)

mySocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM);
mySocket.connect(deviceAddr)


def signal_handler(signal, frame):
    print(" Shutting down...")
    sys.exit()

signal.signal(signal.SIGINT, signal_handler)



def unpackChannelMetaData(oneChannelStr):
    units, dataDataType, realDataType, scaleType, errorType, = struct.unpack('!cccBBxxx', oneChannelStr[:8])

    twoPointStr = oneChannelStr[8:32]
    errorParameters = oneChannelStr[32:]  # not implemented yet

    ddtSize = struct.calcsize(dataDataType)
    dataValA, = struct.unpack(b'!' + dataDataType, twoPointStr[4-ddtSize:4])
    dataValB, = struct.unpack(b'!' + dataDataType, twoPointStr[16-ddtSize:16])

    rdtSize = struct.calcsize(realDataType)
    realValA, = struct.unpack(b'!' + realDataType, twoPointStr[12-rdtSize:12])
    realValB, = struct.unpack(b'!' + realDataType, twoPointStr[24-rdtSize:24])

    namedVals = {'units': units, 'dataDataType': dataDataType, 'realDataType': realDataType,
        'scale': {dataValA: realValA, dataValB: realValB}}

    return namedVals



def parseTOM_v0(inPacket):
    global myParams
    params = {}
    units, stepSizeDatatype, numChannels, bytesPerDescriptor = struct.unpack('!BcBB', inPacket[:4])

    dstSize = struct.calcsize(stepSizeDatatype)
    domainStepSize, = struct.unpack(b'!' + stepSizeDatatype, inPacket[12-dstSize:12])

    params['domain'] = {'units': chr(units & 0x7f), 'inv': (units & 0x80 == 0x80), 'size': domainStepSize}
    params['numChannels'] = numChannels

    params['channelMetaData'] = []
    for i in range(numChannels):
        iStart = 12 + i * bytesPerDescriptor
        iEnd = 12 + (i+1) * bytesPerDescriptor
        params['channelMetaData'].append(unpackChannelMetaData(inPacket[iStart:iEnd]))
    myParams = params
    return params


def parseTOM(inPacket):
    # We parse packets that begin with 'TOM'
    if inPacket[0:1] == b'\x00':  # version 0
        return parseTOM_v0(inPacket[1:])
    return {}




def parseTOD_v0(inPacket):
    bytesPerSample, numSamples, startSampleNumber = struct.unpack('!BxHL', inPacket[:8])

    stripedData = np.frombuffer(inPacket[8:(bytesPerSample*numSamples)+8], dtype=np.uint8)
    return (startSampleNumber, bytesPerSample, stripedData)


def parseTOD(inPacket):
    # We parse packets that begin with 'TOD'
    if inPacket[0:1] == b'\x00':  # version 0
        return parseTOD_v0(inPacket[1:])
    return None, None, None



TRIGGERMODE_OFF = 0
TRIGGERMODE_NOW = 1
TRIGGERMODE_RISING = 2
TRIGGERMODE_FALLING = 3
TRIGGERMODE_CONTINUOUS = 4 # not implemented yet

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
    elif triggerModeStr == "continuous":  #not implemented yet
        trigMode = TRIGGERMODE_CONTINUOUS
    else:
        trigMode = TRIGGERMODE_OFF


    trigNumSamplesReq = 531

    numChannels = 4
    chA = np.zeros(trigNumSamplesReq)
    chB = np.zeros(trigNumSamplesReq)
    chC = np.zeros(trigNumSamplesReq)
    chD = np.zeros(trigNumSamplesReq)
    metaData = {}

    # Packet structure is: Mode, Channel, Threshold, (dummy byte), NumberOfSamplesRequested
    #trigCmd = chr(trigMode) + chr(channel & 0xff) + chr(threshold & 0xff) + chr(0)
    #trigCmd += struct.pack('!L', trigNumSamplesReq)

    trigCmd = struct.pack('!BBcxLL', trigMode, channel, b'L', threshold, trigNumSamplesReq)

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
    firmataOverUdpHeader = b"eFirmata"
    # The firmata service we want: TOC (Triggered Oscilloscope Command):
    firmataOverUdpHeader += b"TOC"
    # Protocol version 0:
    firmataOverUdpHeader += b"\x00"
    # No Flags, no options:
    firmataOverUdpHeader += b"\x00\x00\x00\x00"
    # The firmata-over-udp header should be exactly 16 bytes:
    assert len(firmataOverUdpHeader) == 16, "BUG.  You broke it."

    mySocket.send(firmataOverUdpHeader + trigCmd)

    ## Right now, things are hard-coded for four channels, 256 samples per
    ## ethernet frame (so the data portion that we care about is always 1024 bytes)
    ## On each frame, the data is striped:  chA, chB, chC, chD, chA, chB, chC, chD, chA, ...

    readingList = [mySocket]
    sEnd = 0

    while 1:
        inputs, outputs, errors = select.select(readingList, [], [], 1.1)
        if inputs:
            for oneInput in inputs:
                if oneInput == mySocket:
                    inPacket = mySocket.recv(1522)
                    if inPacket[:3] == b'TOM':  # Triggered Oscilloscope Metadata
                        metaData = parseTOM(inPacket[3:])
                    elif inPacket[:3] == b'TOD':  # Triggered Oscilloscope Data
                        sStart, bytesPerSample, stripedData = parseTOD(inPacket[3:])
                        if bytesPerSample:
                            sEnd = sStart + len(stripedData) // bytesPerSample
                            chA[sStart:sEnd] = stripedData[::4] #.copy()
                            chB[sStart:sEnd] = stripedData[1::4] #.copy()
                            chC[sStart:sEnd] = stripedData[2::4] #.copy()
                            chD[sStart:sEnd] = stripedData[3::4] #.copy()
                    else:
                        print('wtf')
        else:
            print("timeout waiting for packet")
            break
        if sEnd >= trigNumSamplesReq:
            break

    return metaData, chA, chB, chC, chD


def matplotlibStyle():
    physicalUnits = {'s': 'Seconds', 'V': 'Volts', 'A': 'Amperes'}
    cnt = 0
    plt.ion()
    fig = plt.figure()
    ax = fig.add_subplot(111)

    global myMetaData


    # First time:
    metaData, chA, chB, chC, chD = getTriggeredSample(1, 100, 'rising')

    # We will set the vertical scale according to chA:
    metaChA = metaData['channelMetaData'][0]
    dValues = list(metaChA['scale'].keys())
    dValues.sort()
    dRange = dValues[1] - dValues[0]
    vRange = metaChA['scale'][dValues[1]] - metaChA['scale'][dValues[0]]
    yScaleAdj = vRange / dRange
    yScaleOffset = 0.0
    yTicks = np.linspace(dValues[0], dValues[1], 5)
    yTickLabels = yTicks * yScaleAdj + yScaleOffset
    ax.set_yticks(yTicks)
    ax.set_yticklabels(np.round(yTickLabels, 2))

    # show the full range (vertically), plus a little whitespace
    extraMargin = dRange * 0.055
    plt.ylim([dValues[0] - extraMargin, dValues[1] + extraMargin])

    if 'units' in metaChA:
        if metaChA['units'] in physicalUnits:
            plt.ylabel(physicalUnits[metaChA['units']])
        else:
            plt.ylabel(metaChA['units'])

    ## The horizontal axis
    if 'units' in metaData['domain']:
        if metaData['domain']['units'] in physicalUnits:
            plt.xlabel(physicalUnits[metaData['domain']['units']])
        else:
            plt.xlabel(metaData['domain']['units'])


    # Create the time scale:
    dScale = 1.0
    if 'size' in metaData['domain']:
        if 'inv' in metaData['domain'] and metaData['domain']['inv']:
            dScale = 1.0/metaData['domain']['size']
        else:
            dScale = metaData['domain']['size']
    t = np.arange(len(chA)) * dScale

    lineA, = ax.plot(t, chA, 'r,')
    lineB, = ax.plot(t, chB, 'g,')
    lineC, = ax.plot(t, chC, 'b')
    lineD, = ax.plot(t, chD, 'y,')

    fig.canvas.draw()

    while 1:
        # every subsequent time:
        try:
            metaData, chA, chB, chC, chD = getTriggeredSample(1, 100, 'rising')
            myMetaData = metaData
            lineA.set_ydata(chA)
            lineB.set_ydata(chB)
            lineC.set_ydata(chC)
            lineD.set_ydata(chD)
        except:
            time.sleep(0.1)
        else:
            fig.canvas.draw()
        #    cnt += 1
        #if cnt == 5:
        #    break



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
            print(".",)
        else:
            for i in range(len(chA)):
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
            print(".",)
        else:
            # Erase the old pixels:
            for i in range(len(chA)):
                x = oldChA[i]
                y = oldChB[i]
                wholeImageBuffer[x,y] = colorBG
            # Draw the new pixels:
            for i in range(len(chA)):
                x = chA[i]
                y = chB[i]
                wholeImageBuffer[x,y] = colorA
            pygame.surfarray.blit_array(screen, wholeImageBuffer)
            pygame.display.flip()

matplotlibStyle()
#pygameStyle()
#pygameXYscope()

