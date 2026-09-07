#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// stdlib.h
int rand();
void srand(int seed);

// string.h
void* memcpy(void* dst, void* src, unsigned int length);
void* memmove(void* dst, void* src, unsigned int length);
void* memset(void* dst, int value, unsigned int length);

unsigned int strlen(const char* str);
// The implementation for this is vectorized (when dst % 4 == src % 4),
// which makes it not immediately obvious that it really is strcpy.
// Key observation: for a uint x, we have
//     (x + 0xfefefeff) & ~x & 0x80808080 == 0
// if and only if all the bytes in x are nonzero.
char* strcpy(char* dst, const char* src);
char* strncpy(char* dst, const char* src, unsigned int count);

char* strcat(char* dst, const char* src);

// also vectorized like strcpy, but seems to have a bug(?).
// doesn't break the functionality but causes the process to revert to per-char
// comparison as soon as it encounters a character value of >= 129
int strcmp(const char* lhs, const char* rhs);
int strncmp(const char* lhs, const char* rhs, unsigned int count);
char* strchr(const char* str, int ch);
char* strrchr(const char* str, int ch);
char* strstr(const char* str, const char* substr);

// stdio.h
int sprintf(char* buffer, const char* format, ...);

#ifdef __cplusplus
}
#endif
