#include <libk/libk.h>

#define MAX_BASE_SUPPORTED 36

long int strtol(const char *nptr, char **endptr, int base)
{
	return strntol(nptr, strlen(nptr), endptr, base);
}

long int strntol(const char *nptr, size_t size, char **endptr, int base)
{
	size_t i = 0;
	long int value = 0;

	if (endptr)
		*endptr = (char*)nptr;

	if (base > MAX_BASE_SUPPORTED)
		return 0;

	// ignore leading spaces
	while (i < size && isspace(nptr[i]))
		++i;

	if (i >= size)
		return 0;

	int negative = (nptr[i] == '-');
	if (negative || nptr[i] == '+')
		++i;

	if (base == 16 && nptr[i] == '0' && nptr[i + 1] == 'x')
		i += 2;

	for (; i < size; ++i) {
		value *= base;

		char d = nptr[i];
		char minus;

		if (isdigit(d))
			minus = '0';
		else if (isupper(d))
			minus = 'A' - 10;
		else if (islower(d))
			minus = 'a' - 10;
		else
			break; // not a valid digit

		long int dval = d - minus;
		// invalid digit for base
		if (dval >= base)
			break;

		value += dval;
	}

	if (endptr)
		*endptr = (char*)nptr + i;

	return (negative) ? -value : value;
}
