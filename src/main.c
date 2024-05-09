#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>

#include "vcpu/vcpu.h"
#include "vclock/vclock.h"
#include "vmemory/vmemory.h"
#include "device_table/device_table.h"
#include "vnvic/interrupt_event.h"
#include "vmemory/vmemorybus.h"
#include "events/events.h"

#define FLUSH while (getchar() != '\n')

static void init();
static void start();
static void shutdown();

static void handle_test(const void *args);
static void handle_test2(const void *args);

static int rc;
static char input;

static vCPU_t vcpu;
static vMEMORY_t vmemory;
static vCLOCK_t vclock;

int main()
{
    init();

    event_create("test");
    event_create("test2");

    event_subscribe("test", handle_test);
    event_subscribe("test", handle_test2);
    event_subscribe("test2", handle_test2);

    start();

    printf("Press any key to quit...\n");
    scanf(" %c", &input);

    shutdown();

    return 0;
}

void handle_test(const void *args)
{
    char *message = (char *)args;
    printf("Test - %s\n", message);
}

void handle_test2(const void *args)
{
    char *message = (char *)args;
    printf("Test2 - %s\n", message);
}

// During init, devices should register with the device_table, create events, and initialize their own fields.
void init()
{
    // NOTE: Must initialize device_table first because devices are registered to device_table during initialization.
    if (rc = device_table_init())
    {
        printf("Error: device_table_init() returned with rc=%d", rc);
        exit(1);
    }

    // NOTE: Then we initialize the interrupt_event, so devices are able to attach to it during init.
    if (rc = interrupt_event_init())
    {
        printf("Error: interrupt_event_init() returned with rc=%d", rc);
        exit(1);
    }

    // NOTE: Then we initialize the events, so devices are able to attach to it during init.
    if (rc = events_init())
    {
        printf("Error: events_init() returned with rc=%d", rc);
        exit(1);
    }

    // NOTE: Order of initialization of devices does not matter.
    if (rc = vmemory_init(&vmemory))
    {
        printf("Error: vmemory_init() returned with rc=%d", rc);
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
}

void start()
{
    if (rc = vmemory_start(&vmemory))
    {
        printf("Error: vmemory_start() returned with rc=%d", rc);
        exit(1);
    }

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
}

void shutdown()
{
    if (rc = vclock_shutdown(&vclock))
    {
        printf("Error: vclock_shutdown() returned with rc=%d", rc);
        exit(1);
    }

    if (rc = vcpu_shutdown(&vcpu))
    {
        printf("Error: vcpu_shutdown() returned with rc=%d", rc);
        exit(1);
    }

    if (rc = vmemory_shutdown(&vmemory))
    {
        printf("Error: vmemory_shutdown() returned with rc=%d", rc);
        exit(1);
    }
}
