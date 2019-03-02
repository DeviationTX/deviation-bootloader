/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2010 Gareth McMullin <gareth@blacksphere.co.nz>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/dfu.h>
#include "hardware.h"
#include "init.c"

/* This script will load as a 'Walkera' encrypted DFU for a supported transmitter
   and automatically replace the bootloader
*/
extern uint8_t *bootloader_data;
extern unsigned bootloader_data_size;

void check_power()
{
    static int32_t debounce = -1;
    if (PORT_pin_get(PWR_SWITCH_PIN)) {
        if (debounce >= 0)  // Ensure user has released the pin since boot
            debounce++;
    } else {
        debounce = 0;
    }
    if (debounce > 100000) {
        PORT_pin_clear(PWR_ENABLE_PIN);
        while(1);
    }
}

int main(void)
{
    rcc_clock_setup_in_hsi_out_48mhz();
    rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN);
    // GPIOA
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL,
        PWR_ENABLE_PIN.pin
        );
    gpio_set(GPIOA,
        PWR_ENABLE_PIN.pin
        );

    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO1);
    gpio_set(GPIOB, GPIO1);

    //Wait a litle while for siwtch to settle
    {
        volatile unsigned j = 1000000;
        while(--j) {
            check_power();
        }
    }

    flash_unlock();
    uintptr_t start_address = LOAD_ADDRESS;
    uint32_t page_size = (ROMSIZE >= 256) ? 0x0800 : 0x0400;
    for (uintptr_t addr = start_address; addr < (uintptr_t)(&small_vector_table); addr += page_size) {
        flash_erase_page(addr);
    }
    uintptr_t addr = (uintptr_t)&bootloader_data;
    for (uint32_t i = 0; i < bootloader_data_size; i += 2) {
        uint16_t *data = (uint16_t *)(addr + i);
        flash_program_half_word(start_address + i, *data);
    }
    // Ensure we don't reload this again
    addr = (uintptr_t)&small_vector_table;
    flash_program_half_word(addr, 0x0000);
    flash_program_half_word(addr+2, 0x0000);
    flash_program_half_word(addr+4, 0x0000);
    flash_program_half_word(addr+6, 0x0000);

    flash_lock();

    /* Boot the bootloaderd. */
    if ((*(volatile uint32_t *)LOAD_ADDRESS & 0x2FFE0000) == 0x20000000) {
        /* Set vector table base address. */
        SCB_VTOR = LOAD_ADDRESS & 0xFFFF;
        /* Initialise master stack pointer. */
        asm volatile("msr msp, %0"::"g"
            (*(volatile uint32_t *)LOAD_ADDRESS));
        /* Jump to bootloader. */
        (*(void (**)())(LOAD_ADDRESS + 4))();
    }
    /* Getting here is bad! */
    while(1) {
        volatile unsigned j = 1000000;
        while(--j) {
            check_power();
        }
        gpio_toggle(GPIOB, GPIO1);
    }
}
