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
    init();
    device_table_dump();
    registers_dump(&vcpu);
    memory_dump(&vmemory_controller);

    start();

    //sleep(10);

    vsysbus_dump();

    pthread_exit(NULL);
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