#ifndef _LIBK_H_
#define _LIBK_H_

#include <stddef.h>

void* memcpy(void* dest, const void* src, size_t size);
void* memset(void *s, int c, size_t size);
int memcmp(const void* s1, const void* s2, size_t size);

size_t strlen(const char* str);
size_t strnlen(const char* str, size_t maxsize);
char* strcat(char* str, const char* append);
char* strncat(char* str, const char* append, size_t count);

char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t size);
size_t strlcpy(char* dest, const char* src, size_t size);

int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t size);

#endif
