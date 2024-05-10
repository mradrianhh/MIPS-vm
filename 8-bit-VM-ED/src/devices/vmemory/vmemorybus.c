#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "vmemorybus.h"

static int vmemorybus_write_in(vMEMORYBUS_PACKETS_t *packets);
static int vmemorybus_write_out(vMEMORYBUS_PACKETS_t *packets);
static int vmemorybus_read_in(vMEMORYBUS_PACKETS_t *packets);
static int vmemorybus_read_out(vMEMORYBUS_PACKETS_t *packets);

static vMEMORYBUS_t vmemorybus_in;
static vMEMORYBUS_t vmemorybus_out;

int vmemorybus_init()
{
    vmemorybus_in.start = malloc(vMEMORYBUS_BUFFER_SIZE);
    vmemorybus_in.next = vmemorybus_in.start;
    memset(vmemorybus_in.start, 0, vMEMORYBUS_BUFFER_SIZE);
    pthread_mutex_init(&(vmemorybus_in.mutex), NULL);

    vmemorybus_out.start = malloc(vMEMORYBUS_BUFFER_SIZE);
    vmemorybus_out.next = vmemorybus_out.start;
    memset(vmemorybus_out.start, 0, vMEMORYBUS_BUFFER_SIZE);
    pthread_mutex_init(&(vmemorybus_out.mutex), NULL);

    return 0;
}

int vmemorybus_write(vMEMORYBUS_SEL_t sel, vMEMORYBUS_PACKETS_t *packets)
{
    if(packets->control.unit_used == vMEMORYBUS_CONTROL_NOT_USED)
    {
        printf("vMEMORYBUS - Error: Package  {Data: 0x%02x Access: 0x%02x Control: 0x%02x} invalid.", packets->data, packets->access, packets->control.control_field);
        return 1;
    }
    
    switch (sel)
    {
    case vMEMORYBUS_SEL_IN:
        return vmemorybus_write_in(packets);
        break;
    case vMEMORYBUS_SEL_OUT:
        return vmemorybus_write_out(packets);
        break;
    default:
        return 1;
        break;
    }
}

static int vmemorybus_write_in(vMEMORYBUS_PACKETS_t *packets)
{
    pthread_mutex_lock(&(vmemorybus_in.mutex));
    if (vmemorybus_in.next > (vmemorybus_in.start + vMEMORYBUS_BUFFER_SIZE))
    {
        printf("Error: vMEMORYBUS buffer-limit exceeded. Packet {Data: 0x%02x Access: 0x%02x Control: 0x%02x} can't be delivered.\n", packets->data, packets->access, packets->control.control_field);
        return 1;
    }

    *(vmemorybus_in.next) = *(packets);
    vmemorybus_in.next++;
    if (vmemorybus_in.next > (vmemorybus_in.start + vMEMORYBUS_BUFFER_LIMIT_WARNING_TRESHOLD))
    {
        printf("Warning: vMEMORYBUS buffer-limit about to be reached. %lu/%lu bytes utilized.\n", (unsigned long)(vmemorybus_in.next - vmemorybus_in.start - sizeof(vMEMORYBUS_PACKETS_t)), vMEMORYBUS_BUFFER_SIZE);
    }
    pthread_mutex_unlock(&(vmemorybus_in.mutex));
    return 0;
}

static int vmemorybus_write_out(vMEMORYBUS_PACKETS_t *packets)
{
    pthread_mutex_lock(&(vmemorybus_out.mutex));
    if (vmemorybus_out.next > (vmemorybus_out.start + vMEMORYBUS_BUFFER_SIZE))
    {
        printf("Error: vMEMORYBUS buffer-limit exceeded. Packet {Data: 0x%02x Access: 0x%02x Control: 0x%02x} can't be delivered.\n", packets->data, packets->access, packets->control.control_field);
        return 1;
    }

    *(vmemorybus_out.next) = *(packets);
    vmemorybus_out.next++;
    if (vmemorybus_out.next > (vmemorybus_out.start + vMEMORYBUS_BUFFER_LIMIT_WARNING_TRESHOLD))
    {
        printf("Warning: vMEMORYBUS buffer-limit about to be reached. %lu/%lu bytes utilized.\n", (unsigned long)(vmemorybus_out.next - vmemorybus_out.start - sizeof(vMEMORYBUS_PACKETS_t)), vMEMORYBUS_BUFFER_SIZE);
    }
    pthread_mutex_unlock(&(vmemorybus_out.mutex));
    return 0;
}

int vmemorybus_read(vMEMORYBUS_SEL_t sel, vMEMORYBUS_PACKETS_t *packets)
{
    switch (sel)
    {
    case vMEMORYBUS_SEL_IN:
        return vmemorybus_read_in(packets);
        break;
    case vMEMORYBUS_SEL_OUT:
        return vmemorybus_read_out(packets);
        break;
    default:
        return 1;
        break;
    }
}

static int vmemorybus_read_in(vMEMORYBUS_PACKETS_t *packets)
{
    pthread_mutex_lock(&(vmemorybus_in.mutex));
    if (vmemorybus_in.next != vmemorybus_in.start)
    {
        vmemorybus_in.next--;
    }

    *packets = *(vmemorybus_in.next);
    vmemorybus_in.next->control.unit_used = vMEMORYBUS_CONTROL_NOT_USED;

    pthread_mutex_unlock(&(vmemorybus_in.mutex));
    return 0;
}

static int vmemorybus_read_out(vMEMORYBUS_PACKETS_t *packets)
{
    pthread_mutex_lock(&(vmemorybus_out.mutex));
    if (vmemorybus_out.next != vmemorybus_out.start)
    {
        vmemorybus_out.next--;
    }

    *packets = *(vmemorybus_out.next);
    vmemorybus_out.next->control.unit_used = vMEMORYBUS_CONTROL_NOT_USED;

    pthread_mutex_unlock(&(vmemorybus_out.mutex));
    return 0;
}