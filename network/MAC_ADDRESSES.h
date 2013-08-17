
#ifndef __MAC_ADDRESSES_H
#define __MAC_ADDRESSES_H

// Red stripe:
#define SELF_ADDR {0x00, 0x02, 0xf7, 0xaa, 0xff, 0xee}
#define SELF_IP_ADDR {192, 168, 11, 177}

// The one next to the keyboard here:
//#define SELF_ADDR {0x00,0x02,0xf7,0xaa,0xbf,0xcd}

// The Reflex Board:
//#define SELF_ADDR {0x00,0x02,0xf7,0xa2,0xa1,0x34}



// My PC (aiu):
#define DEST_ADDR {0xE0,0xCB,0x4E,0x47,0x7F,0x9B}

// Ethertypes for the eFirmata Protocol (firmata over ethernet)
#define EFIRMATA_PROTOCOL_CONTROL 0x181b
#define EFIRMATA_PROTOCOL 0x181c
#define EFIRMATA_PROTOCOL_FAST 0x181d

#endif // __MAC_ADDRESSES_H
