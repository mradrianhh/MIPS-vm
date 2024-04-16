#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "vcpu/vcpu.h"
#include "vclock/vclock.h"
#include "vsysbus/vsysbus.h"
#include "vmemory/vmemory_controller.h"
#include "device_table/device_table.h"

static void init();
static void start();
static void stop();

static int rc;
static char input;
static int running;

static vSYSBUS_t sysbus;
static vCPU_t vcpu;
static vMEMORY_CONTROLLER_t vmemory_controller;
static vCLOCK_t vclock;

int main()
{
    init();
    // start();
    device_table_dump();
    registers_dump(&vcpu);
    memory_dump(&vmemory_controller);

    return 0;
}

void init()
{
    // NOTE: Must initialize device_table first because devices are registered to device_table during initialization.
    if (rc = device_table_init())
    {
        printf("Error: device_table_init() returned with rc=%d", rc);
        exit(1);
    }

    if (rc = vsysbus_init(&sysbus))
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
    if (rc = vsysbus_start(&sysbus))
    {
        printf("Error: vsysbus_start() returned with rc=%d", rc);
        exit(1);
    }
}

void stop()
{
    if (rc = vsysbus_stop(&sysbus))
    {
        printf("Error: vsysbus_stop() returned with rc=%d", rc);
        exit(1);
    }
}