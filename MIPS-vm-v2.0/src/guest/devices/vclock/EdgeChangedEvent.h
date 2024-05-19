#ifndef _MIPSVM_GUEST_DEVICES_EDGE_CHANGED_EVENT_H_
#define _MIPSVM_GUEST_DEVICES_EDGE_CHANGED_EVENT_H_

#include <stdint.h>

struct EdgeChangedEventArgs
{
    uint8_t edge : 1;
    uint8_t : 7;
};
typedef struct EdgeChangedEventArgs EdgeChangedEventArgs_t;

#endif