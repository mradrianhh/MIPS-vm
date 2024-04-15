// The sysbus is an event loop where devices can toss out events, and subscribers can notify to them.
#ifndef _8BITVM_VSYSBUS_H_
#define _8BITVM_VSYSBUS_H_

#include <stdint.h>
#include <pthread.h>

#include "vsysbus_buffer.h"

#define VSYSBUS_START 0
#define VSYSBUS_STOP  1

struct vSYSBUS {
    pthread_t thread_id;
    vSYSBUS_BUFFER_t buffer;
    int running;
};
typedef struct vSYSBUS vSYSBUS_t;

int vsysbus_init(vSYSBUS_t* sysbus);

int vsysbus_start(vSYSBUS_t* sysbus);

int vsysbus_stop(vSYSBUS_t* sysbus);

#endif