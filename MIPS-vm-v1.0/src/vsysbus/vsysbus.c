#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vsysbus.h"

static vSYSBUS_t vsysbus;

int vsysbus_init()
{
    printf("vSYSBUS initializing...\n");
    vsysbus.start = malloc(vSYSBUS_BUFFER_SIZE);
    vsysbus.current = NULL;
    vsysbus.next = vsysbus.start;
    memset(vsysbus.start, 0, vSYSBUS_BUFFER_SIZE);
    pthread_mutex_init(&(vsysbus.mutex), NULL);

    return 0;
}

int vsysbus_write(vSYSBUS_PACKET_t *packet)
{
    printf("next: %p | current: %p | start: %p\n", (void *)vsysbus.next, (void *)vsysbus.current, (void *)vsysbus.start);
    if (vsysbus.next > (vsysbus.start + vSYSBUS_BUFFER_SIZE))
    {
        printf("Error: vSYSBUS buffer-limit exceeded. Packet {Device ID: %d - Data: 0x%02x} can't be delivered.\n", packet->device_id, packet->data);
        return 1;
    }

    pthread_mutex_lock(&(vsysbus.mutex));
    vsysbus.next->device_id = packet->device_id;
    vsysbus.next->data = packet->data;
    vsysbus.current = vsysbus.next;
    vsysbus.next++;
    if (vsysbus.next > (vsysbus.start + vSYSBUS_BUFFER_LIMIT_WARNING_TRESHOLD))
    {
        printf("Warning: vSYSBUS buffer-limit about to be reached. %lu/%lu bytes utilized.\n", (unsigned long)(vsysbus.current - vsysbus.start), vSYSBUS_BUFFER_SIZE);
    }
    pthread_mutex_unlock(&(vsysbus.mutex));
    return 0;
}

int vsysbus_read(uint8_t device_id, vSYSBUS_PACKET_t *packet)
{
    printf("next: %p | current: %p | start: %p\n", (void *)vsysbus.next, (void *)vsysbus.current, (void *)vsysbus.start);
    pthread_mutex_lock(&(vsysbus.mutex));
    if (vsysbus.next == vsysbus.start)
    {
        pthread_mutex_unlock(&(vsysbus.mutex));
        packet = NULL;
        return 0;
    }

    int distance_from_end = 0;
    for (; vsysbus.current->device_id != device_id && vsysbus.current >= vsysbus.start; vsysbus.current--)
    {
        distance_from_end++;
    }

    if (vsysbus.current->device_id == device_id)
    {
        *packet = *(vsysbus.current);
        // shift all values above current-pointer down once, and adjust next-pointer.
        for(int i = 0; i < distance_from_end; i++)
        {
            *(vsysbus.current) = *(++(vsysbus.current));
        }
        vsysbus.next--;
        pthread_mutex_unlock(&(vsysbus.mutex));
        return 0;
    }

    pthread_mutex_unlock(&(vsysbus.mutex));
    packet = NULL;
    return 0;
}

void vsysbus_dump()
{
    printf("*\n");
    printf("* vSYSBUS Dump\n");
    printf("*\n\n");

    pthread_mutex_lock(&(vsysbus.mutex));
    vSYSBUS_PACKET_t *reader = vsysbus.start;
    printf("Start *: %p\n", (void *)vsysbus.start);
    printf("Current *: %p\n", (void *)vsysbus.current);
    printf("Next *: %p\n\n", (void *)vsysbus.next);

    while (reader != vsysbus.next)
    {
        printf("[%p]: {Device ID: %d Data: [0x%02x]}\n", (void *)reader, reader->device_id, reader->data);
        reader++;
    }
    pthread_mutex_unlock(&(vsysbus.mutex));
}
