#ifndef _8BITVM_VSYSBUS_BUFFER_H_
#define _8BITVM_VSYSBUS_BUFFER_H_

#include <stdint.h>

#define vSYSBUS_BUFFER_ITEM_NUM 100
#define vSYSBUS_BUFFER_SIZE sizeof(vSYSBUS_BUFFER_PACKET_t) * vSYSBUS_BUFFER_ITEM_NUM

typedef uint8_t PACKET_t;

struct vSYSBUS_BUFFER_PACKET {
    uint8_t device_id;
    PACKET_t packet;
};
typedef struct vSYSBUS_BUFFER_PACKET vSYSBUS_BUFFER_PACKET_t;
typedef vSYSBUS_BUFFER_PACKET_t* vSYSBUS_BUFFER_t;


int vsysbus_buffer_init(vSYSBUS_BUFFER_t buffer);

vSYSBUS_BUFFER_PACKET_t vsysbus_buffer_fetch_packet(vSYSBUS_BUFFER_t buffer);

#endif