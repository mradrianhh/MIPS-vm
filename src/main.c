#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>

#include "vcpu/vcpu.h"
#include "vclock/vclock.h"
#include "vmemory/vmemory_controller.h"
#include "device_table/device_table.h"
#include "vsysbus/vsysbus.h"

static void init();
static void start();
static void stop();

static int rc;
static char input;
static int running;

static vCPU_t vcpu;
static vMEMORY_CONTROLLER_t vmemory_controller;
static vCLOCK_t vclock;

int main()
{
    // init();
    // device_table_dump();
    // registers_dump(&vcpu);
    // memory_dump(&vmemory_controller);

    // start();

    if (rc = vsysbus_init())
    {
        printf("Error: vsysbus_init() returned with rc=%d", rc);
        exit(1);
    }

    char input;
    vSYSBUS_PACKET_t packet1 = {
        .device_id = 1,
        .data = 1,
    };
    vSYSBUS_PACKET_t packet2 = {
        .device_id = 2,
        .data = 2,
    };
    vSYSBUS_PACKET_t result;
    int rc = 0;
    while (1)
    {
        printf("| 1 - write sysbus 1 | 2 - read sysbus 1 | 3 - write sysbus 2 | 4 - read sysbus 2 |\n| d - sysbus dump | q - terminate |\n");
        scanf("%c", &input);
        switch (input)
        {
        case '1':
            vsysbus_write(&packet1);
            break;
        case '2':
            rc = vsysbus_read(1, &result);
            if (rc == 0)
            {
                printf("Test: Read package {Device ID: [%d] Data: [0x%02x]}\n", result.device_id, result.data);
            }
            else 
            {
                printf("Test: Read package returned with RC=%d\n", rc);
            }
            break;
        case '3':
            vsysbus_write(&packet2);
            break;
        case '4':
            rc = vsysbus_read(2, &result);
            if (rc == 0)
            {
                printf("Test: Read package {Device ID: [%d] Data: [0x%02x]}\n", result.device_id, result.data);
            }
            else 
            {
                printf("Test: Read package returned with RC=%d\n", rc);
            }
            break;
        case 'd':
            vsysbus_dump();
            break;
        case 'q':
            return 0;
            break;
        }
    }

    return 0;
    // pthread_exit(NULL);
}

void init()
{
    // NOTE: Must initialize device_table first because devices are registered to device_table during initialization.
    if (rc = device_table_init())
    {
        printf("Error: device_table_init() returned with rc=%d", rc);
        exit(1);
    }

    if (rc = vsysbus_init())
    {
        printf("Error: vsysbus_init() returned with rc=%d", rc);
        exit(1);
    }

    if (rc = vcpu_init(&vcpu))
    {
        printf("Error: vcpu_init() returned with rc=%d", rc);
        exit(1);
    }

    if (rc = vclock_init(&vclock))
    {
        printf("Error: vclock_init() returned with rc=%d", rc);
        exit(1);
    }

    if (rc = vmemory_controller_init(&vmemory_controller))
    {
        printf("Error: vmemory_controller_init() returned with rc=%d", rc);
        exit(1);
    }
}

void start()
{
    if (rc = vcpu_start(&vcpu))
    {
        printf("Error: vcpu_start() returned with rc=%d", rc);
        exit(1);
    }

    if (rc = vclock_start(&vclock))
    {
        printf("Error: vclock_start() returned with rc=%d", rc);
        exit(1);
    }

    if (rc = vmemory_controller_start(&vmemory_controller))
    {
        printf("Error: vmemory_controller_start() returned with rc=%d", rc);
        exit(1);
    }
}