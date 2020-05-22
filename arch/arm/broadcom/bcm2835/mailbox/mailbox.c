#include <arch/broadcom/bcm2835/mailbox/mailbox.h>
#include <arch/broadcom/bcm2835/peripherals.h>

// https://github.com/raspberrypi/firmware/wiki/Mailboxes
// https://github.com/raspberrypi/firmware/wiki/Accessing-mailboxes

struct mailbox
{
	struct mailbox_msg msg;
	uint32_t reserved[3];
	uint32_t peek;
	uint32_t sender;
	struct {
		uint32_t reserved:30;
		bool empty:1;
		bool full:1;
	} status;
	uint32_t config;
};

static volatile struct mailbox* const mailbox = (volatile struct mailbox*)(PERIPHERALS_BASE + 0xb880);

enum
{
	VC_TO_ARM,
	ARM_TO_VC,
};

struct mailbox_msg mailbox_read(int chan)
{
	volatile struct mailbox* mb = &mailbox[VC_TO_ARM];
	struct mailbox_msg read;

	do {
		while (mb->status.empty) {}
		read = mb->msg;
	} while (read.chan != chan);

	return read;
}

void mailbox_write(struct mailbox_msg msg)
{
	volatile struct mailbox* mb = &mailbox[ARM_TO_VC];

	while (mb->status.full) {}
	mb->msg = msg;
}
