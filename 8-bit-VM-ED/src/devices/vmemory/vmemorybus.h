#ifndef _8BITVM_VMEMORYBUS_H_
#define _8BITVM_VMEMORYBUS_H_

#include <stdint.h>
#include <pthread.h>

#define vMEMORYBUS_BUFFER_ITEM_NUM 10
#define vMEMORYBUS_BUFFER_SIZE (sizeof(vMEMORYBUS_PACKETS_t) * vMEMORYBUS_BUFFER_ITEM_NUM)
#define vMEMORYBUS_BUFFER_LIMIT_WARNING_TRESHOLD (vMEMORYBUS_BUFFER_SIZE - (sizeof(vMEMORYBUS_PACKETS_t) * 1))
#define vMEMORYBUS_CONTROL_WRITE 0
#define vMEMORYBUS_CONTROL_READ 1
#define vMEMORYBUS_CONTROL_NOT_USED 0
#define vMEMORYBUS_CONTROL_USED 1

typedef enum vMEMORYBUS_SEL
{
    vMEMORYBUS_SEL_IN = 0,
    vMEMORYBUS_SEL_OUT = 1,
} vMEMORYBUS_SEL_t;

union vMEMORYBUS_PACKET_CONTROL_FIELD
{
    struct
    {
        uint8_t unit_used : 1;
        uint8_t access_op : 1;
        uint8_t : 6;
    };
    uint8_t control_field;
};
typedef union vMEMORYBUS_PACKET_CONTROL_FIELD vMEMORYBUS_PACKET_CONTROL_FIELD_t;

struct vMEMORYBUS_PACKETS
{
    uint8_t data;
    uint8_t access;
    vMEMORYBUS_PACKET_CONTROL_FIELD_t control;
};
typedef struct vMEMORYBUS_PACKETS vMEMORYBUS_PACKETS_t;

/// @struct
/// [start] points at the first address in the bus/buffer.
/// [current] points at the current position in bus/buffer.
/// [next] points at the next available position in bus/buffer.
struct vMEMORYBUS
{
    vMEMORYBUS_PACKETS_t *start;
    vMEMORYBUS_PACKETS_t *next;
    pthread_mutex_t mutex;
};
typedef struct vMEMORYBUS vMEMORYBUS_t;

int vmemorybus_init();

int vmemorybus_write(vMEMORYBUS_SEL_t sel, vMEMORYBUS_PACKETS_t *packets);

int vmemorybus_read(vMEMORYBUS_SEL_t sel, vMEMORYBUS_PACKETS_t *packets);

#endif