#include <stdint.h>

//#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
//	// TODO: bit banding is possible
//#else
//	// modify peripheral register
//	TODO: single bit clears / sets can be optimized to single memory writes into bit-banding region on Cortex-M3, M4, M7
inline void PREG_CLEAR_SET(volatile uint32_t *REG_ADDR, uint32_t CLEAR_MASK, uint32_t SET_MASK) {
	*REG_ADDR = (*REG_ADDR & ~CLEAR_MASK) | SET_MASK;
}
//#endif

// requests MCU to reset the whole system
[[noreturn]]
void do_system_reset(void);

// jumps to default vector at 0x08000000
[[noreturn]]
void jump_sysmem(void);

// total number of interrupt handlers supported on a target device
#define ISR_VECTOR_LENGTH  76
// the interrupt vector placed in RAM
extern void (*isr_ram [ISR_VECTOR_LENGTH])(void);
