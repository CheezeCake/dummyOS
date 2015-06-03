#include <kernel/terminal.h>
#include <kernel/cpu.h>
#include <kernel/log.h>

void kernel_main(void)
{
	terminal_init();
	for (int i = 0; i < 10; i++)
		terminal_puts("Hello, world !\n");
	for (int i = 0; i < 15; i++)
		terminal_puts("Ah ouais ?!\n");

	struct cpu_info* cpu = cpu_info();
	terminal_puts(cpu->cpu_vendor);
}
