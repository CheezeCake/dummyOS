#include <libk/libk.h>

int isalnum(int c)
{
	return (isalpha(c) || isdigit(c));
}

int isalpha(int c)
{
	return (isupper(c) || islower(c));
}

int isdigit(int c)
{
	return (c >= '0' && c <= '9');
}

int isupper(int c)
{
	return (c >= 'A' && c <= 'Z');
}

int islower(int c)
{
	return (c >= 'a' && c <= 'z');
}

int isblank(int c)
{
	return (c == ' ' || c == '\t');
}

int isspace(int c)
{
	return (isblank(c) || c == '\f' || c == '\n' || c == '\r' || c == '\v');
}
