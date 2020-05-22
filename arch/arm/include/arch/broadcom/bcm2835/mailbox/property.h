#ifndef _ARCH_BROADCOM_BCM2835_MAILBOX_PROPERTY_H
#define _ARCH_BROADCOM_BCM2835_MAILBOX_PROPERTY_H

#include <kernel/types.h>

#define BCM2835_MAILBOX_PROPERTY_REQUEST_SUCCESS  0x80000000
#define BCM2835_MAILBOX_PROPERTY_REQUEST_FAILURE  0x80000001

int bcm2835_mailbox_property_get_arm_mem(size_t* bytes);
int bcm2835_mailbox_property_get_uart_clock(uint32_t* hz);

#endif
