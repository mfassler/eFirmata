
This is example code that runs on a PC to talk to one or more eFirmata
devices.

The bridge (efirmataBridge.py) will need to run as root.  


Background:
----------
eFirmata talks directly to ethernet.  In a typical Linux (/Posix) environment,
regular users cannot talk directly to ethernet.  So:  firmataBridge.py runs
as root to talk to the network.  

eFirmata hopes to learn from the USB protocol.  So the eFirmata protocol is
divided into three types:  control messages, normal messages, fast messages.  

So, firmataBridge.py has these connections:

          NETWORK                                 LOCAL PC
                               _____
                              |     |
    <-- ethernet_0x181b -->   |  B  |   ---> /tmp/eFirmataInControl.sock
                              |  R  |   <--- /tmp/eFirmataOutControl.sock
    <-- ethernet_0x181c -->   |  I  |   ---> /tmp/eFirmataIn.sock
                              |  D  |   <--- /tmp/eFirmataOut.sock
    <-- ethernet_0x181d -->   |  G  |   ---> /tmp/eFirmataInFast.sock
                              |  E  |   <--- /tmp/eFirmataOutFast.sock
                              |_____|


User-level programs can talk to the sockets on the "local PC" side.  


(This is the one piece of complexity that I regret.  But, then again,
even USB devices need drivers which are installed by root.  But everything
else about eFirmata strives to be as small, simple and lightweight as
possible.)  

(Eventually, I intend to rewrite the bridge in C, and make it as fast
as possible.)


Typically, multiple eFirmata devices are connected (via ethernet) to
a single workstation.  The user-level applications will need to know
the MAC addresses of the eFirmata devices.  


