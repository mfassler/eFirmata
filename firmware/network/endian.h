


#define FLIP_ENDIAN_16(x) (( (x & 0xff00) >> 8 ) | ( (x & 0x00ff) << 8))

// Network is big-endian, but we are little-endian

#define ntohs(x) ( (((x) & 0xff00) >> 8 ) | (((x) & 0x00ff) << 8) )
#define htons(x) ( (((x) & 0xff00) >> 8 ) | (((x) & 0x00ff) << 8) )

#define ntohl(x) ( (((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >> 8) | (((x) & 0x0000ff00) << 8) | (((x) & 0x000000ff) << 24) )
#define htonl(x) ( (((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >> 8) | (((x) & 0x0000ff00) << 8) | (((x) & 0x000000ff) << 24) )


