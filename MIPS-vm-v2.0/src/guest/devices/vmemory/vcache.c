#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "vcache.h"

void vcache_init(vCache_t *cache, size_t byte_size, size_t line_size)
{
    cache->line_size = line_size * 4;      // Convert from words to bytes.
    cache->entry_size = line_size * 4 + 4; // Add 4 bytes/1 word to fit tag.
    cache->byte_size = byte_size;
    cache->data = (uint8_t *)malloc(byte_size);
    if (cache->data == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for vCache data\n");
        exit(EXIT_FAILURE);
    }
}

uint32_t vcache_load(vCache_t *cache, uint32_t paddr)
{
    uint32_t tag = paddr & TAG_MASK;
    uint32_t line_index = paddr & LINE_INDEX;

    // Check that line_index is valid for this cache.
    if (line_index > cache->line_size / 4)
    {
        fprintf(stderr, "Error: invalid paddr. Lower two bits attempts to select word from cache line\n");
        fprintf(stderr, "that exceeds the number of words.\n");
        exit(EXIT_FAILURE);
    }

    // We search the cache. Current points at the current word in the cache. The first word should
    // be a tag.
    uint32_t *current = (uint32_t *)cache->data;
    while (current <= (uint32_t*)(cache->data + cache->byte_size))
    {
        // Check if the tag matches the paddr we're looking for.
        if (*current == tag)
        {
            // We found the entry. We increase the pointer so we point at the first word in cache line.
            current++;
            // Index into the cache line and return the value.
            return *(current + line_index);
        }
        // If not, we need to advance to the next entry. We need to increase the pointer past the 
        // size of the cache line.
        current += cache->line_size / 4;
    }

    // Cache miss: If we exit the loop, it means that we have searched the entire stack without finding anything.
    // For now, we just print an error and return 0.
    fprintf(stderr, "Cache miss for paddr 0x%08x\n", paddr);
    return 0;
}

void vcache_store(vCache_t *cache, uint32_t value, uint32_t paddr)
{
}

void vcache_destroy(vCache_t *cache)
{
    free(cache->data);
    cache->data = NULL;
}
