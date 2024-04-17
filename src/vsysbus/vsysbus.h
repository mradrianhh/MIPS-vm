#ifndef _8BITVM_VSYSBUS_BUFFER_H_
#define _8BITVM_VSYSBUS_BUFFER_H_

#include <stdint.h>
#include <pthread.h>

#define vSYSBUS_BUFFER_ITEM_NUM 10
#define vSYSBUS_BUFFER_SIZE (sizeof(vSYSBUS_PACKET_t) * vSYSBUS_BUFFER_ITEM_NUM)
#define vSYSBUS_BUFFER_LIMIT_WARNING_TRESHOLD  (vSYSBUS_BUFFER_SIZE - (sizeof(vSYSBUS_PACKET_t) * 1))

struct vSYSBUS_PACKET
{
    uint8_t device_id;
    uint8_t data;
};
typedef struct vSYSBUS_PACKET vSYSBUS_PACKET_t;

/// @struct
/// [start] points at the first address in memory.
/// [current] points at the current position in memory.
/// [next] points at the next available position in memory.
struct vSYSBUS
{
    vSYSBUS_PACKET_t *start;
    vSYSBUS_PACKET_t *current;
    vSYSBUS_PACKET_t *next;
    pthread_mutex_t mutex;
};
typedef struct vSYSBUS vSYSBUS_t;

int vsysbus_init();

int vsysbus_write(vSYSBUS_PACKET_t *packet);

int vsysbus_read(uint8_t device_id, vSYSBUS_PACKET_t *packet);

void vsysbus_dump();

#endif