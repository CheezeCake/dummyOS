#include <arch/broadcom/bcm2835/mailbox/mailbox.h>
#include <arch/broadcom/bcm2835/mailbox/property.h>
#include <arch/paging.h>

/*
 * https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
 */

struct mailbox_property_buffer_header
{
	uint32_t size;
	uint32_t code;
};

enum mailbox_request_code
{
	MAILBOX_REQUEST,
};

static inline void get_property(const struct mailbox_msg msg)
{
	mailbox_write(msg);
	mailbox_read(BCM2835_MAILBOX_PROPERTY_TAGS_CHANNEL);
}


/*
 * https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface#get-arm-memory
 */

struct __attribute__((aligned(16)))  mailbox_property_arm_mem
{
	struct mailbox_property_buffer_header header;
	struct {
		uint32_t tag;
		uint32_t size;
		uint32_t code;
		struct {
			uint32_t base_addr;
			uint32_t size;
		} value;
	} tag;
	uint32_t end_tag;
};

int bcm2835_mailbox_property_get_arm_mem(size_t* bytes)
{
	struct mailbox_property_arm_mem req = {
		.header = { .size = sizeof(req), .code = MAILBOX_REQUEST },
		.tag = {
			.tag = 0x00010005,
			.size = sizeof(req.tag.value),
			.code = 0,
			.value = { 0, 0 },
		},
		.end_tag = 0,
	};
	const struct mailbox_msg msg =
		MAILBOX_ADDR_MSG(BCM2835_MAILBOX_PROPERTY_TAGS_CHANNEL, &req);

	get_property(msg);

	if (req.header.code == BCM2835_MAILBOX_PROPERTY_REQUEST_SUCCESS)
		*bytes = req.tag.value.size;

	return req.header.code;
}


/*
 * https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface#clocks
 */

struct __attribute__((aligned(16))) mailbox_property_get_clock_rate
{
	struct mailbox_property_buffer_header header;
	struct {
		uint32_t tag;
		uint32_t size;
		uint32_t code;
		struct {
			uint32_t clock_id;
			uint32_t rate;
		} value;
	} tag;
	uint32_t end_tag;
};

enum clock_id
{
	UART_CLOCK_ID = 0x000000002,
};

static int mailbox_property_get_clock_rate(enum clock_id cid, uint32_t* hz)
{
	struct mailbox_property_get_clock_rate req = {
		.header = { .size = sizeof(req), .code = MAILBOX_REQUEST },
		.tag = {
			.tag = 0x00030002,
			.size = sizeof(req.tag.value),
			.code = 0,
			.value = { .clock_id = cid, 0 },
		},
		.end_tag = 0,
	};
	const struct mailbox_msg msg =
		MAILBOX_ADDR_MSG(BCM2835_MAILBOX_PROPERTY_TAGS_CHANNEL, &req);

	get_property(msg);

	if (req.header.code == BCM2835_MAILBOX_PROPERTY_REQUEST_SUCCESS)
		*hz = req.tag.value.rate;

	return req.header.code;
}

int bcm2835_mailbox_property_get_uart_clock(uint32_t* hz)
{
	return mailbox_property_get_clock_rate(UART_CLOCK_ID, hz);
}
