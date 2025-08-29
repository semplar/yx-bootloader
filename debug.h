#pragma once

#include <stdint.h>

// prints a single character to debug console. blocks if FIFO is full
void debugc(char c);

// prints a zero-terminated character string to debug console. blocks if FIFO is full
void debugs(const char* asciiz);

// prints hexadecimal number. bits should be multiple of 4
void debugh(uint32_t out, uint8_t bits);

