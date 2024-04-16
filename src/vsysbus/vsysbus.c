#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "vsysbus.h"

static void *vsysbus_loop(void *vargp);

static int rc;

int vsysbus_init(vSYSBUS_t *sysbus)
{
    printf("vSYSBUS initializing...\n");
    sysbus->running = VSYSBUS_START;
    if (rc = vsysbus_buffer_init(sysbus->buffer))
    {
        printf("Error: vsysbus_buffer_init() returned with rc=%d", rc);
        exit(1);
    }

    // TODO: Start vSYSBUS_BUFFER_CONTROLLER
    // vsysbus_buffer_controller_init(sysbus->buffer)
    // vsysbus_buffer_controller_start()

    return 0;
}

int vsysbus_start(vSYSBUS_t *sysbus)
{
    sysbus->running = VSYSBUS_START;
    printf("vSYSBUS starting...\n");

    pthread_create(&(sysbus->thread_id), NULL, vsysbus_loop, (void *)sysbus);

    return 0;
}

static void *vsysbus_loop(void *vargp)
{
    vSYSBUS_t *sysbus = (vSYSBUS_t *)vargp;

    while (sysbus->running)
    {
        vSYSBUS_BUFFER_PACKET_t input = vsysbus_buffer_fetch_packet(sysbus->buffer);
        if (!(input.packet + input.device_id))
        {
            printf("vSYSBUS processing packet { device_id: %d, packet: %d }\n", input.device_id, input.packet);
        }
        sleep(3);
    }
}

int vsysbus_stop(vSYSBUS_t *sysbus)
{
    sysbus->running = VSYSBUS_STOP;
    printf("vSYSBUS stopping...\n");

    pthread_join(sysbus->thread_id, NULL);

    return 0;
}
