#include "hardware.h"

#include <libopencm3/stm32/rcc.h> // reset and clock control
#include <libopencm3/cm3/scb.h> // system control block

// exports from a linker script
extern char _sdata[];
extern char _edata[];
extern const char _ldata[];
extern char _sbss[];
extern char _ebss[];
extern void (_estack)(); // declared as a function because it goes into interrupt vector

[[noreturn]]
void do_system_reset(void) {
	SCB_AIRCR = SCB_AIRCR_VECTKEY | SCB_AIRCR_SYSRESETREQ; // request software reset
	for (;;) {} // wait for reset
}

[[noreturn]]
void jump_sysmem(void) { // TODO: this does not work, maybe should disable peripherals
//	register u32 SP asm("sp") = SYSMEM->STACK_BASE;
//	asm volatile("" :: "r" (SP)); // do not optimize away, remove unused warning
//	SYSMEM->RESET_HANDLER();
	for (;;) {}
}

extern int main(void);
extern void* memcpy(void*, const void*, unsigned int);
extern void* memset(void*, int, unsigned int);

void _start(void); // entry point

// this ISR vector is stored in flash, so kept very short and minimal
// during entry, we initialize a full ISR vector called isr_ram and configure MCU to use it
[[gnu::section(".isr_vector_flash")]]
[[gnu::used]]
static void (*const isr_flash[])(void) = {
    _estack,
    _start,
};

[[gnu::section(".isr_vector_ram")]]
_Alignas(4*128) void (*isr_ram [ISR_VECTOR_LENGTH])(void);

[[noreturn]]
void _start(void) {
	memcpy(isr_ram, isr_flash, sizeof(isr_flash)); // ISR from flash to RAM
	// fill the rest of ISR vector with pointer to do_system_reset()
	void (**isr_ram_ptr)() = isr_ram + sizeof(isr_flash) / sizeof(isr_flash[0]);
	while (isr_ram_ptr < isr_ram + ISR_VECTOR_LENGTH)
		*isr_ram_ptr++ = do_system_reset;
	SCB_VTOR = (uint32_t) isr_ram; // tell MCU to use ISR vector in RAM
	memcpy(_sdata, _ldata, _edata - _sdata); // copy preinitialized .data from flash to RAM
	memset(_sbss, 0, _ebss - _sbss); // zero-initialize .bss section in RAM

	if (main()) {
		// abnormal return: forever loop to keep state
		for (;;) asm volatile("wfi");
	} else {
		// normal return: slow down clock and wait some time to restart MCU
		// PREG_CLEAR_SET(&RCC_CFGR, RCC_CFGR_HPRE, RCC_CFGR_HPRE_DIV128); // 72Mhz -> 62.5khz
		for (unsigned int i = 0; i < 72000000; i++)
			asm volatile ("");
		do_system_reset();
	}
}
