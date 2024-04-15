#include <stdlib.h>
#include <string.h>

#include "vsysbus_buffer.h"

int vsysbus_buffer_init(vSYSBUS_BUFFER_t buffer)
{
    buffer = malloc(vSYSBUS_BUFFER_SIZE);
    memset(buffer, 0, vSYSBUS_BUFFER_SIZE);
    return 0;
}

vSYSBUS_BUFFER_PACKET_t vsysbus_buffer_fetch_packet(vSYSBUS_BUFFER_t buffer)
{
    vSYSBUS_BUFFER_PACKET_t ret = buffer[0];
    buffer[0].device_id = 0;
    buffer[0].packet = 0;

    return ret;
}
