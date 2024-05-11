#define _POSIX_C_SOURCE 199309L
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>

#include "internal/events/events.h"
#include "internal/device_table/device_table.h"
#include "devices/vcpu/vcpu.h"
#include "devices/vmemory/vmemory.h"
#include "devices/vclock/TickEvent.h"

#define SEC_TO_NS_FACTOR (1000000000)
#define SEC_TO_NS(sec) ((sec) * SEC_TO_NS_FACTOR)
#define NS_TO_SEC(sec) ((double)(sec) / SEC_TO_NS_FACTOR)

static void tick();

TickEventArgs_t tick_args;

static struct timespec ts;
static struct timespec start_ts;
static struct timespec end_ts;
static struct timespec delay_ts = {.tv_nsec = 0, .tv_sec = 0};

static long actual_ns = 0;
static long delta_ns = 0;
static long frequency_ns = SEC_TO_NS(1);

int main()
{
    device_table_init();
    events_init();

    event_create("init");
    event_create("tick");
    event_create("shutdown");

    event_subscribe("init", vcpu_init);
    event_subscribe("init", vmemory_init);
    event_subscribe("tick", vcpu_update);
    event_subscribe("shutdown", vcpu_shutdown);
    event_subscribe("shutdown", vmemory_shutdown);

    event_notify("init", NULL);
/*
    char input = ' ';
    printf("Press a key to tick\n");
    while (input != 'q')
    {
        scanf(" %c", &input);
        tick();
    }
*/

    while(1)
    {
        tick();
    }

    event_notify("shutdown", NULL);

    return 0;
}

void tick()
{
    timespec_get(&start_ts, TIME_UTC);

    event_notify("tick", (void *)&tick_args);

    timespec_get(&end_ts, TIME_UTC);

    delta_ns = SEC_TO_NS(end_ts.tv_sec - start_ts.tv_sec) + (end_ts.tv_nsec - start_ts.tv_nsec);
    delay_ts.tv_nsec = frequency_ns - delta_ns;

    if (delay_ts.tv_nsec > 0)
    {
        if (nanosleep(&delay_ts, NULL))
        {
            printf("Timing delay failed. Terminating.\n");
            exit(1);
        }
    }
}