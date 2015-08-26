#ifndef _GDT_H_
#define _GDT_H_

#include <stdint.h>

#define GDT_SIZE 5

// indexes in GDT
#define KCODE 1
#define KDATA 2
#define UCODE 3
#define UDATA 4

/*
 * 8 byte GDT segment descriptor structure
 */
struct gdt_segment_descriptor
{
	uint16_t limit_15_0;

	uint16_t base_address_15_0;

	uint8_t base_23_16;
	uint8_t type:4;
	uint8_t descriptor_type:1;
	uint8_t dpl:2;
	uint8_t present:1;

	uint8_t limit_19_16:4;
	uint8_t avl:1;
	uint8_t zero:1;
	uint8_t operation_size:1;
	uint8_t granularity:1;

	uint8_t base_31_24;
} __attribute__((packed));

/*
 * GDT register
 */
struct gdtr
{
	uint16_t limit;
	uint32_t base_address;
} __attribute__((packed));


#define CODE_SEGMENT 0xb
#define DATA_SEGMENT 0x3

#define PRIVILEGE_KERNEL 0
#define PRIVILEGE_USER 3

void gdt_init(void);

#endif
