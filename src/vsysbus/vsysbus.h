#ifndef _8BITVM_VSYSBUS_BUFFER_H_
#define _8BITVM_VSYSBUS_BUFFER_H_

#include <stdint.h>
#include <pthread.h>

#define vSYSBUS_BUFFER_ITEM_NUM 100
#define vSYSBUS_BUFFER_SIZE sizeof(vSYSBUS_PACKET_t) * vSYSBUS_BUFFER_ITEM_NUM

struct vSYSBUS_PACKET
{
    uint8_t device_id;
    uint8_t packet;
};
typedef struct vSYSBUS_PACKET vSYSBUS_PACKET_t;

struct vSYSBUS
{
    vSYSBUS_PACKET_t *start;
    vSYSBUS_PACKET_t *current;
    vSYSBUS_PACKET_t *buffer;
    pthread_mutex_t mutex;
};
typedef struct vSYSBUS vSYSBUS_t;

int vsysbus_init();

int vsysbus_write(vSYSBUS_PACKET_t *packet);

vSYSBUS_PACKET_t *vsysbus_read(uint8_t device_id);

void vsysbus_dump();

#endif