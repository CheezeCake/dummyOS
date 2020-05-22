#ifndef _ARCH_BROADCOM_BCM2835_MEMORY_H_
#define _ARCH_BROADCOM_BCM2835_MEMORY_H_

#include <arch/broadcom/bcm2835/peripherals.h>
#include <kernel/mm/memory.h>

#define BCM2835_PERIPHERALS_MEMORY_AREA MEMORY_AREA(PERIPHERALS_BASE_PHYS,	\
						    PERIPHERALS_SIZE,		\
						    PERIPHERALS_BASE, 		\
						    "MMIO")

#endif
