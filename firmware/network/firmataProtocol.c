
#include <LPC17xx.h>
#include "bitTypes.h"
#include "debug.h"
#include "pwm.h"

#include "network/firmataProtocol.h"
#include "emac.h"


void parseIncomingFirmataPacket(void *ptr) {
	(void)ptr; // unused parameter

	debug("received firmata over ethernet");
}


