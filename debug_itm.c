#include <stdint.h>

#include "libopencm3/cm3/scs.h" // enable debug for stimulus output
#include "libopencm3/cm3/itm.h" // instrumentation trace macrocell, for debug output
#include "libopencm3/cm3/tpiu.h" // trace point interface unit, for debug output

#include "debug.h"

uint8_t debug_stim_port = 0;

void debugc(char c) {
	while (!ITM_STIM8(debug_stim_port)) {} // wait for stimulus port gets ready
	ITM_STIM8(debug_stim_port) = c;
}

void debugs(const char* asciiz) {
	while (*asciiz)
		debugc(debug_stim_port, *asciiz++);
}

int debug_init_itm(uint32_t sysclk, uint32_t port) {
	if (port >= 32)
		return 1;
	SCS_DEMCR |= SCS_DEMCR_TRCENA; // enable debug for stimulus output
	ITM_TCR |= ITM_TCR_ITMENA | ITM_TCR_TXENA | ITM_TCR_SWOENA | ITM_TCR_SYNCENA;
	ITM_TPR = 0xFFFFFFFF;
	ITM_TER[0] |= port << 0; // enable STIM(0) port
	TPIU_SPPR = TPIU_SPPR_ASYNC_NRZ;
	TPIU_ACPR = (sysclk / 2000000) - 1; // 72MHz clock, 2000KHz SWV baud
	debug_stim_port = port;
	return 0;
}

//	debugs("Debug message 1\n");
