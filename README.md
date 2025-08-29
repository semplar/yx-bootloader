# Purpose
STM32F103 device has 2048 bytes of system memory (read-only flash), containing bootloader talking to UART.
Despite device fully supporting USB, the bootloader doesn't support USB upload.

The goal of this project is to write a minimalistic bootloader <= 2Kb that acts as CDC ACM device (UART to USB converter) and let directly upload firmware to device with the same protocol as system bootloader uses.
The system bootloader protocol is described in: **AN3155: USART protocol used in the STM32 bootloader**.

# Dependencies and device support
Uses [libopencm3]([https://pages.github.com/](https://github.com/libopencm3/libopencm3)) library.
Only support for STM32F103 at the moment, but reliance on library may help to port to other devices.

# Project status
Pre alpha.
USB communication driver is being implemented.
