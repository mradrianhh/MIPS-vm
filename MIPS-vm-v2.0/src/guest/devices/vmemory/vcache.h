#ifndef _MIPSVM_GUEST_DEVICES_MEMORY_VCACHE_H_
#define _MIPSVM_GUEST_DEVICES_MEMORY_VCACHE_H_

#define LINE_INDEX (0x3) // Get the two lower order bits. Used for selecting word from cache line.
#define TAG_MASK (0xFFFFFFFC) // Get the remaining higher order bits. Used for cache index.

#include "guest/common/logger/logger.h"
#include "guest/common/device_table/device_table.h"


typedef struct vCache vCache_t;

struct vCache
{
    // Total entry size(tag + cache line) in bytes(word * 4).
    size_t entry_size; 
    // Cache line size in bytes(word * 4). Number of words in cache line. 
    // Usually 4 words for I-cache and 1 word for D-cache.
    size_t line_size; 
    // Total byte size of the cache. Default is 8kB each for I-cache and D-cache.
    size_t byte_size;
    // Pointer to actual bytes in host-memory.
    uint8_t *data;
};

// Line size is the number of words stored in a cache line.
void vcache_init(vCache_t *cache, size_t byte_size, size_t line_size);
uint32_t vcache_load(vCache_t *cache, uint32_t paddr);
void vcache_store(vCache_t *cache, uint32_t value, uint32_t paddr);
void vcache_destroy(vCache_t *cache);

#endif
