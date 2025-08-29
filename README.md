# Purpose
STM32F103 device has 2048 bytes of read-only system flash memory, containing "burned-in" system bootloader.
Despite device fully supporting USB, the system bootloader doesn't support USB for programming.

The goal of this project is to write a minimalistic <=2Kb bootloader that acts as UART to USB converter and lets reprogram device through the same protocol as system bootloader uses, so it will be possible to use STM32CubeProgrammer without hooking USB-to-Serial converter.

It is impossible to directly replace the system bootloader because the system memory is read-only.
There is 64 kb flash, but boot always starts from flash begin by default, and taking over beginning region of flash will interfere with application.
The idea is to place the bootloader at the end of flash region so it transparently updates default ISR at flash start during reprogramming.

The system bootloader protocol is described in [AN3155: USART protocol used in the STM32 bootloader](https://www.st.com/resource/en/application_note/an3155-usart-protocol-used-in-the-stm32-bootloader-stmicroelectronics.pdf).

# Prerequisites
Only support for STM32F103 at the moment, but reliance on library may help to port to other devices.

STM32 platform library: [libopencm3](https://github.com/libopencm3/libopencm3) library.

Compiler toolchain: open-source [ARM Toolchain for Embedded](https://github.com/arm/arm-toolchain).

Makefile expects it to be installed into "C:\Program Files\ATfE-20.1.0"

# Project status
Status: pre-alpha.
USB communication driver is being implemented.
