#ifndef _ARCH_BROADCOM_BCM2835_MAILBOX_MAILBOX_H
#define _ARCH_BROADCOM_BCM2835_MAILBOX_MAILBOX_H

#include <kernel/types.h>

struct mailbox_msg
{
	uint32_t chan:4;
	uint32_t data:28;
};

#define MAILBOX_ADDR_MSG(channel, vaddr_data) { .chan = channel, .data = paging_virt_to_phys((v_addr_t)vaddr_data) >> 4 }

enum mailbox_channel
{
	BCM2835_MAILBOX_FRAMEBUFFER_CHANNEL	= 1,
	BCM2835_MAILBOX_PROPERTY_TAGS_CHANNEL	= 8,
};

struct mailbox_msg mailbox_read(int chan);
void mailbox_write(struct mailbox_msg msg);

#endif
