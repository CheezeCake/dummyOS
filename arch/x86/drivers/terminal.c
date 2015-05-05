#include <stddef.h>
#include <stdint.h>

#include <kernel/vga.h>
#include <kernel/terminal.h>
#include <kernel/libk.h>

#define VGA_MEMORY 0xb8000

#define TERMINAL_LINES 25
#define TERMINAL_COLUMNS 80

#define TERMINAL_SETCHAR(term, c, color) { term.character = c; \
										 term.attribute = color; }

struct vga_memory
{
	uint8_t character;
	uint8_t attribute;
} __attribute__((packed));

static uint8_t terminal_line = 0;
static uint8_t terminal_column = 0;
static uint8_t terminal_color = 0;
static volatile struct vga_memory* terminal = (volatile struct vga_memory*)VGA_MEMORY;

void terminal_putchar(char c)
{
	const int current_index = terminal_line * TERMINAL_COLUMNS + terminal_column;

	switch (c) {
		case '\n':
			++terminal_line;
			terminal_column = 0;
			break;
		case '\b':
			if (terminal_column > 0) {
				TERMINAL_SETCHAR(terminal[current_index - 1], ' ', terminal_color);
				--terminal_column;
			}
			break;
		case '\r':
			terminal_column = 0;
			break;
		case '\t':
			terminal_column = terminal_column + 8 - (terminal_column % 8);
			break;
		default:
			TERMINAL_SETCHAR(terminal[current_index], c, terminal_color);
			++terminal_column;
	}

	if (terminal_column >= TERMINAL_COLUMNS) {
		terminal_column = 0;
		++terminal_line;
	}

	// scrolling
	if (terminal_line >= TERMINAL_LINES) {
		terminal_line = TERMINAL_LINES  - 1;

		// copy lines
		memcpy((struct vga_memory*)terminal,
				(struct vga_memory*)terminal + TERMINAL_COLUMNS,
				sizeof(struct vga_memory) * (TERMINAL_LINES - 1) * TERMINAL_COLUMNS);

		// reset last line
		for (int i = 0; i < TERMINAL_COLUMNS; i++) {
			const int index  = (TERMINAL_LINES - 1) * TERMINAL_COLUMNS + i;
			TERMINAL_SETCHAR(terminal[index], ' ', terminal_color);
		}
	}
}

int terminal_puts(const char* str)
{
	int len;
	for (len = 0; str[len]; len++)
		terminal_putchar(str[len]);

	return len;
}

void terminal_write(const char* data, size_t size)
{
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}

void terminal_clear(uint8_t color)
{
	terminal_line = 0;
	terminal_column = 0;
	terminal_color = color;

	for (int i = 0; i < TERMINAL_LINES * TERMINAL_COLUMNS; i++)
		TERMINAL_SETCHAR(terminal[i], 0, terminal_color);
}

void terminal_set_color(uint8_t color)
{
	terminal_color = color;
}

void terminal_init(void)
{
	terminal_clear(vga_make_color(COLOR_WHITE, COLOR_BLACK));
}

int terminal_printf(const char* format, ...)
{
	const int buffer_size = 256;
	char buffer[buffer_size];
	va_list ap;

	va_start(ap, format);
	vsnprintf(buffer, buffer_size, format, ap);
	va_end(ap);

	return terminal_puts(buffer);
}
