#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vsysbus.h"

static vSYSBUS_t vsysbus;

int vsysbus_init()
{
    printf("vSYSBUS initializing...\n");
    vsysbus.buffer = malloc(vSYSBUS_BUFFER_SIZE);
    vsysbus.start = vsysbus.buffer;
    vsysbus.current = vsysbus.buffer;
    memset(vsysbus.buffer, 0, vSYSBUS_BUFFER_SIZE);
    pthread_mutex_init(&vsysbus.mutex, NULL);

    return 0;
}

int vsysbus_write(vSYSBUS_PACKET_t *packet)
{
    pthread_mutex_lock(&vsysbus.mutex);
    vsysbus.buffer = vsysbus.current;
    vsysbus.buffer->device_id = packet->device_id;
    vsysbus.buffer->data = packet->data;
    vsysbus.current++;
    pthread_mutex_unlock(&vsysbus.mutex);
    return 0;
}

vSYSBUS_PACKET_t *vsysbus_read(uint8_t device_id)
{
    pthread_mutex_lock(&vsysbus.mutex);
    vsysbus.buffer = vsysbus.start;
    for (int i = 0; &vsysbus.buffer[i] != vsysbus.current; i++)
    {
        if (vsysbus.buffer[i].device_id == device_id)
        {
            pthread_mutex_unlock(&vsysbus.mutex);
            return &vsysbus.buffer[i];
        }
    }
    pthread_mutex_unlock(&vsysbus.mutex);
    return NULL;
}

void vsysbus_dump()
{
    printf("*\n");
    printf("* vSYSBUS Dump\n");
    printf("*\n");

    pthread_mutex_lock(&vsysbus.mutex);
    vsysbus.buffer = vsysbus.start;
    printf("Start *: %d\n", (int)vsysbus.start);
    printf("Current *: %d\n", (int)vsysbus.current);

    for (int i = 0; &vsysbus.buffer[i] != vsysbus.current; i++)
    {
        printf("vSYSBUS *: %d\t", (int)&vsysbus.buffer[i]);
        printf("Device ID: %d\tData: [0x%02x]\n", vsysbus.buffer[i].device_id, vsysbus.buffer[i].data);
    }
    pthread_mutex_unlock(&vsysbus.mutex);
}
