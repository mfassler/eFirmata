
http://www.efirmata.net/

eFirmata allows general purpose I/O across the network.  It's similar
in spirit to Firmata (an Arduino project that allows you to control
GPIO pins from your PC), except that it runs over ethernet (hence the
"e-" prefix).

For example: You can use this code to turn an MBED into a four-channel, 
triggered oscilloscope.  (The MBED has a maximum sample rate of 200 KSPS,
multiplexed over four channels.  So that's 50,000 SPS per channel.)  

Another example:  You can use this code to control the PWM outputs of
the MBED to control lights and motors.  

All data is communicated over Ethernet.  ie:  MBED <---> Ethernet <---> PC

Compatible with the LPCXpresso.  (I do most of my work on the LPCXpresso, actually.)

