
#ifndef __DEFAULT_IP_ADDRESS_H
#define __DEFAULT_IP_ADDRESS_H

#define SELF_IP_ADDR {192, 168, 11, 177}


// If you want to receive debug-over-udp, set 
// your IP address and UDP port# here:
//  ("nc" is short for "netcat" -- plain, old' ASCII over UDP.)
#define NC_DEBUG_IP_ADDR {192, 168, 11, 5}
#define NC_DEBUG_PORT 0x1234
// and set this to 1 to turn it on:
#define NC_DEBUG_CONNECTED 0


#endif // __DEFAULT_IP_ADDRESS_H


