#pragma once

void InvalidateDataCache();
void CleanDataCache();
void CleanInvalidateDataCache();
void InvalidateDataCacheRange(const void* where, unsigned int len);
void CleanCacheRange(const void* where, unsigned int len);
void CleanInvalidateCacheRange(const void* where, unsigned int len);
void DrainWriteBuffer();
void InvalidateInstructionCache();
void InvalidateInstructionCacheRange(const void* where, unsigned int len);