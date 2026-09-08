#include "System/Cache.h"

#define CACHE_LINE_LENGTH 32
#define CACHE_LINE_LENGTH_BITMASK 0x1f

#define DATA_CACHE_SIZE 4096
#define INSTRUCTION_CACHE_SIZE 8192

#define CACHE_NUM_WAYS 4

// https://developer.arm.com/documentation/ddi0201/d/programmer-s-model/cp15-register-map-summary/register-7--cache-operations-register
// Note the manual uses the term 'flush' to mean 'invalidate' (no cleaning).

void InvalidateDataCache()
{
    __asm("mov r0, 0");
    __asm("mcr p15, 0x0, r0, c7, c6, 0");
}

void CleanDataCache()
{
    int setId;
    int wayId;
    __asm("mov wayId, #0");
    do
    {
        __asm("mov setId, #0");
        do 
        {
            int mask = wayId | setId;
            __asm("mcr p15, 0, mask, c7, c10, 2");
            setId += CACHE_LINE_LENGTH;
        } while (setId < DATA_CACHE_SIZE / CACHE_NUM_WAYS);
        __asm("add wayId, wayId, 1 << 30");
    } while (wayId != 0);
}

void CleanInvalidateDataCache()
{
    int setId;
    int wayId;
    __asm("mov r12, #0");
    __asm("mov wayId, #0");
    do
    {
        __asm("mov setId, #0");
        do
        {
            int mask = wayId | setId;
            __asm("mcr p15, 0, r12, c7, c10, 4");
            __asm("mcr p15, 0, mask, c7, c14, 2");

            setId += CACHE_LINE_LENGTH;
        } while (setId < DATA_CACHE_SIZE / CACHE_NUM_WAYS);
        
        __asm("add wayId, wayId, 1 << 30");
    } while (wayId != 0);
}

void InvalidateDataCacheRange(const void* where, unsigned int len)
{
    int end = len + (int)where;
    unsigned char* dst = (unsigned char*)((unsigned int)where & ~CACHE_LINE_LENGTH_BITMASK);

    do 
    {
        __asm("mcr p15, 0x0, dst, c7, c6, 1");
        dst += CACHE_LINE_LENGTH;
    } while ((int)dst < end);
}

void CleanCacheRange(const void* where, unsigned int len)
{
    int end = len + (int)where;
    unsigned char* dst = (unsigned char*)((unsigned int)where & ~CACHE_LINE_LENGTH_BITMASK);

    do 
    {
        __asm("mcr p15, 0x0, dst, c7, c10, 1");
        dst += CACHE_LINE_LENGTH;
    } while ((int)dst < end);
}

void CleanInvalidateCacheRange(const void* where, unsigned int len)
{
    __asm("mov r12, 0"); // Need a blank register for use with drain write buffer
    int end = len + (int)where;
    unsigned char* dst = (unsigned char*)((unsigned int)where & ~CACHE_LINE_LENGTH_BITMASK);

    do
    {
        // "Drain write buffer"
        __asm("mcr p15, 0x0, r12, c7, c10, 4");
        __asm("mcr p15, 0x0, dst, c7, c14, 1");
        dst += CACHE_LINE_LENGTH;
    } while ((int)dst < end);
}

void DrainWriteBuffer()
{
    __asm("mov r0, 0");
    __asm("mcr p15, 0x0, r0, c7, c10, 4");
}

void InvalidateInstructionCache()
{
    __asm("mov r0, 0");
    __asm("mcr p15, 0x0, r0, c7, c5, 0");
}

void InvalidateInstructionCacheRange(const void* where, unsigned int len)
{
    int end = len + (int)where;
    unsigned char* dst = (unsigned char*)((unsigned int)where & ~CACHE_LINE_LENGTH_BITMASK);
    do
    { 
        __asm("mcr p15, 0x0, dst, c7, c5, 1");
        dst += CACHE_LINE_LENGTH;
    } while ((int)dst < end);
}