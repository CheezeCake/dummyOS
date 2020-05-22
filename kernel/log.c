#include <kernel/log.h>
#include <libk/libk.h>

static int default_puts(const char* s)
{
	return 0;
}

static void default_putchar(char c)
{
}

static const struct log_ops default_logger = {
	.puts = default_puts,
	.putchar = default_putchar,
};

static const struct log_ops* logger = &default_logger;

void log_register(const struct log_ops* ops)
{
	logger = ops;
}


int log_printf(const char* format, ...)
{
	const int buffer_size = 256;
	char buffer[buffer_size];
	va_list ap;

	va_start(ap, format);
	vsnprintf(buffer, buffer_size, format, ap);
	va_end(ap);

	return log_puts(buffer);
}

int log_puts(const char* s)
{
	return logger->puts(s);
}

void log_putchar(char c)
{
	logger->putchar(c);
}
