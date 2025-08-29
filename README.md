# Purpose
STM32F103 device has 2048 bytes of system memory (read-only flash), containing bootloader talking to UART.
Despite device fully supporting USB, the system bootloader doesn't support USB upload.

The goal of this project is to write a minimalistic bootloader <= 2Kb that acts as CDC ACM device (UART to USB converter) and let directly upload firmware to device with the same protocol as system bootloader uses.

It is impossible to replace the system bootloader because system memory is read-only.
The idea is to place the bootloader at the end of Flash region and make it transparently update default ISR at 0x08000000 during reprogramming.

The system bootloader protocol is described in: **AN3155: USART protocol used in the STM32 bootloader**.

# Prerequisites
Only support for STM32F103 at the moment, but reliance on library may help to port to other devices.

Compiler toolchain: open-source [ARM Toolchain for Embedded](https://github.com/arm/arm-toolchain).

STM32 platform library: [libopencm3](https://github.com/libopencm3/libopencm3) library.

Makefile expects it to be installed into "C:\Program Files\ATfE-20.1.0"

# Project status
Status: pre-alpha.
USB communication driver is being implemented.
