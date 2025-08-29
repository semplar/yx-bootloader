#include <stdint.h>

#include <libopencm3/stm32/rcc.h>   // reset and clock control
#include <libopencm3/stm32/flash.h>

#include "hardware.h"
#include "debug.h"
#include "usb.h"

// go into 72 MHz: expecting 8 MHz external oscillator (HSE)
void configure_clocks() {
	PREG_CLEAR_SET(&RCC_CR, RCC_CR_HSEBYP, RCC_CR_HSEON); // enable HSE, no HSE bypass

	uint32_t reg = RCC_CFGR;
	reg |=  RCC_CFGR_PLLSRC;   // use HSE for PLL input
	reg &= ~RCC_CFGR_PLLXTPRE; // HSE->PLL divider = /1 = 8 MHz input to PLL
	reg &= ~RCC_CFGR_PLLMUL;
	reg |=  RCC_CFGR_PLLMUL_PLL_CLK_MUL9 << RCC_CFGR_PLLMUL_SHIFT; // PLL multiplier x9 -> PLL = 72 MHz
	reg &= ~RCC_CFGR_HPRE;     // no AHB prescaler = 72 MHz
	reg &= ~RCC_CFGR_PPRE1;
	reg |=  RCC_CFGR_PPRE_DIV2 << RCC_CFGR_PPRE1_SHIFT;  // APB1 prescaler /2 = 36 MHz (max allowed for APB1)
	reg &= ~RCC_CFGR_USBPRE;   // USB prescaler /1.5 = 48 MHz
	RCC_CFGR = reg;
	while (!(RCC_CR & RCC_CR_HSERDY)) {}    // wait for HSE to stabilize

	PREG_CLEAR_SET(&RCC_CR, 0, RCC_CR_PLLON); // enable PLL
	while (!(RCC_CR & RCC_CR_PLLRDY)) {}    // wait for PLL to lock on HSE

	reg = FLASH_ACR;
	reg &= ~FLASH_ACR_LATENCY_MASK;
	reg |=  FLASH_ACR_LATENCY_2WS << FLASH_ACR_LATENCY_SHIFT; // set 2 wait states flash latency as required for 48Mhz < SYSCLK <= 72MHz
	reg |=  FLASH_ACR_PRFTBE;  // enable prefetch buffer to compensate for higher latency
	FLASH_ACR = reg;

	reg = RCC_CFGR;
	reg &= ~RCC_CFGR_SW;
	reg |=  RCC_CFGR_SWS_SYSCLKSEL_PLLCLK << RCC_CFGR_SW_SHIFT; // use PLL clock as system clock
	RCC_CFGR = reg;
	while ((RCC_CFGR & RCC_CFGR_SWS) != (RCC_CFGR_SWS_SYSCLKSEL_PLLCLK << RCC_CFGR_SWS_SHIFT)) {} // wait for system clock to switch to PLL
}

extern int debug_init_uart(uint32_t sysclk, uint32_t baud);

int main(void) {
	configure_clocks();
	debug_init_uart(72000000, 921600);
	debugs("debug uart on\n");
	usb_poweron();
	debugs("usb poweron done\n");
	debugs("forever loop\n");
	for (;;) {}
	return 0;
}
