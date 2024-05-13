#ifndef _8BITVM_TICKEVENT_H_
#define _8BITVM_TICKEVENT_H_

#include <stdint.h>

struct EdgeChangedEventArgs
{
    uint8_t edge : 1;
    uint8_t : 7;
};
typedef struct EdgeChangedEventArgs EdgeChangedEventArgs_t;

#endif