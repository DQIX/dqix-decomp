#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int abs(int);
long labs(long); // identical to abs() but also used

// stdlib.h
int rand();
void srand(int seed);

int abs(int);
long labs(long); // identical to abs() but also used

// string.h
void* memcpy(void* dst, const void* src, unsigned int length);
void* memmove(void* dst, const void* src, unsigned int length);
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


#define offsetof(type, member) ((unsigned int)(&((type*)0)->member))

typedef unsigned long long uint64_t;
typedef signed long long int64_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned char uint8_t;
typedef char int8_t;

typedef unsigned int uintptr_t;
typedef signed int intptr_t;

typedef unsigned int size_t;

#define INLINE_MEMCPY(to, from, size) \
    { \
        int i; \
        unsigned char* dst; \
        const unsigned char* src; \
        i = (size); \
        src = (const unsigned char*)(from); \
        dst = (unsigned char*)(to); \
        do { \
            i--; \
            *dst = *src; \
            dst++; \
            src++; \
        } while (i != 0); \
    }

#define INLINE_MEMCPY_4(to, from, size) \
    { \
        int i; \
        unsigned int* dst; \
        const unsigned int* src; \
        i = (size); \
        src = (const unsigned int*)(from); \
        dst = (unsigned int*)(to); \
        do { \
           i -= 4; \
           *dst = *src; \
           dst++; \
           src++; \
        } while (i != 0); \
    }

// appeases the ide, array1 = array2 is not officially allowed but mwcc
// allows it, and sometimes we have to use it
#ifdef __MWERKS__
#define COPY_ARRAY(dst, src) dst = src
#else
#define COPY_ARRAY(dst, src) INLINE_MEMCPY(dst, src, sizeof(dst))
#endif