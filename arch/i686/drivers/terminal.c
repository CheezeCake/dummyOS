#include <stddef.h>
#include <stdint.h>

#include <kernel/vga.h>
#include <kernel/terminal.h>

#define VGA_MEMORY 0xb8000

#define TERMINAL_LINES 25
#define TERMINAL_COLUMNS 80

#define TERMINAL_SETCHAR(term, c, color) term.character = c; \
										 term.attribute = color

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
				TERMINAL_SETCHAR(terminal[index - 1], ' ', terminal_color);
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

	if (terminal_line >= TERMINAL_LINES) {
		terminal_line = TERMINAL_LINES  - 1;

		for (int i = 0; i < TERMINAL_LINES - 1; i++) {
			for (int j = 0; j < TERMINAL_COLUMNS; j++) {
				const int index = i * TERMINAL_COLUMNS + j;
				const int index_next_line = index + TERMINAL_COLUMNS;
				TERMINAL_SETCHAR(terminal[index],
						terminal[index_next_line].character,
						terminal[index_next_line].attribute);
			}
		}

		for (int i = 0; i < TERMINAL_COLUMNS; i++) {
			const int index  = (TERMINAL_LINES - 1) * TERMINAL_COLUMNS + i;
			TERMINAL_SETCHAR(terminal[index], ' ', terminal_color);
		}
	}
}

void terminal_puts(const char* str)
{
	while (*str)
		terminal_putchar(*(str++));
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

	for (int i = 0; i < TERMINAL_LINES * TERMINAL_COLUMNS; i++) {
		TERMINAL_SETCHAR(terminal[i], 0, terminal_color);
	}
}

void terminal_set_color(uint8_t color)
{
	terminal_color = color;
}

void terminal_init(void)
{
	terminal_clear(vga_make_color(COLOR_WHITE, COLOR_BLACK));
}
