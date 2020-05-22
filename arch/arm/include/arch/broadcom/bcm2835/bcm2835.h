#ifndef _ARCH_BROADCOM_BCM2835_BCM2835_H_
#define _ARCH_BROADCOM_BCM2835_BCM2835_H_

#include <kernel/types.h>

int bcm2835_machine_init_common(void);
size_t bcm2835_physical_mem(void);
const struct machine_ops* bcm2835_machine_ops(void);

#endif
