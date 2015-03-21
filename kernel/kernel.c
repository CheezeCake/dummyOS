#include <kernel/terminal.h>

void kernel_main()
{
	terminal_init();
	for (int i = 0; i < 10; i++)
		terminal_puts("Hello, world !\n");
	for (int i = 0; i < 14; i++)
		terminal_puts("Ah ouais ?!\n");
	terminal_puts("FUT !!!");
	terminal_puts("\b\b");
}
