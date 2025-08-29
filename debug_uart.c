#include <stdint.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>

#include "hardware.h"
#include "debug.h"

void debugc(char c) {
	while (!(USART1_SR & USART_SR_TXE)) {} // wait for transmit buffer to become empty
	USART1_DR = c;
}

void debugs(const char* asciiz) {
	while (*asciiz)
		debugc(*asciiz++);
}

const char HEX[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

void debugh(uint32_t out, uint8_t bits) {
	for (int shift = bits; shift >= 4; ) {
		shift -= 4;
		debugc(HEX[(out >> shift) & 0xf]);
	}
}

int debug_init_uart(uint32_t sysclk, uint32_t baud) {
	PREG_CLEAR_SET(&RCC_APB2ENR, 0, RCC_APB2ENR_USART1EN | RCC_APB2ENR_IOPAEN);

	uint32_t reg = GPIOA_CRH;
	reg &= ~0x00000FF0; // clear PA9 and PA10 settings
	reg |= ((GPIO_CNF_OUTPUT_ALTFN_PUSHPULL << 2) | GPIO_MODE_OUTPUT_50_MHZ) << 4; // PA9 (TX) = alternate push-pull (<=50 mhz)
	reg |= ((GPIO_CNF_INPUT_FLOAT << 2) | GPIO_MODE_INPUT) << 8;                   // PA10 (RX) = input floating
	GPIOA_CRH = reg;

	USART1_BRR = sysclk / baud; // baud

	PREG_CLEAR_SET(&USART1_CR2, USART_CR2_STOPBITS_MASK, USART_CR2_STOPBITS_1); // 1 stop bit

//	USART_CR3(usart_base) = 0x00000080; // enable DMA tx

	reg = USART1_CR1;
	reg |=  USART_CR1_UE;  // enable usart1
	reg |=  USART_CR1_PCE; // enable parity check
	reg &= ~USART_CR1_PS; // even parity
	reg |=  USART_CR1_M;   // 9 bits (1 for parity)
	reg |=  USART_CR1_TE;  // enable TX
	reg &= ~USART_CR1_RE; // disable RX
	USART1_CR1 = reg;
	return 0;
}
