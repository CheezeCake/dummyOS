#include <arch/cpu.h>
#include <dummyos/compiler.h>
#include <kernel/cpu.h>
#include <kernel/types.h>

enum arm_implementor
{
	ARM_IMPL_ARM = 0x41,
};

enum arm_part_number
{
	ARM_PART_ARM1176JZFS	= 0xb76,
	ARM_PART_CORTEXA7	= 0xc07,
};

struct arm_main_id_register
{
	uint32_t revision:4;
	uint32_t part_number:12;
	uint32_t architecture:4;
	uint32_t variant:4;
	uint32_t implementor:8;
};
static_assert_type_size(struct arm_main_id_register, 4);

struct cpu_info* cpu_info(void)
{
	static struct cpu_info arm_cpu_info = { .cpu_vendor = NULL };
	struct arm_main_id_register id = { 0 };

	__asm__ ("mrc p15, #0, %0, c0, c0, #0" : "=r" (id));

	if (id.implementor == ARM_IMPL_ARM) {
		switch (id.part_number) {
		case ARM_PART_CORTEXA7:
			arm_cpu_info.cpu_vendor = "ARM Cortex-A7";
			break;
		case ARM_PART_ARM1176JZFS:
			arm_cpu_info.cpu_vendor = "ARM1176JZF-S";
			break;
		}
	}

	return &arm_cpu_info;
}

struct A7_multiprocessor_affinity_register
{
	uint32_t cpuid:2;
	uint32_t reserved2_7:6;
	uint32_t cluster_id:4;
	uint32_t reserved12_23:12;
	uint32_t MT:1;
	uint32_t reserved24_29:5;
	uint32_t U:1;
	uint32_t _1:1;
};
static_assert_type_size(struct A7_multiprocessor_affinity_register, 4);

unsigned cpu_get_core_id(void)
{
	struct A7_multiprocessor_affinity_register reg;
	/* uint32_t reg; */

	__asm__ ("mrc p15, #0, %0, c0, c0, #5" : "=r" (reg));

	return reg.cpuid;
}
