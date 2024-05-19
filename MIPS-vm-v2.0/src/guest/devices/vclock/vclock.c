#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>

#include "guest/common/events/events.h"
#include "vclock.h"
#include "EdgeChangedEvent.h"

#define SEC_TO_NS_FACTOR (1000000000)
#define SEC_TO_NS(sec) ((sec) * SEC_TO_NS_FACTOR)
#define NS_TO_SEC(sec) ((double)(sec) / SEC_TO_NS_FACTOR)

static vCLOCK_t vclock;
static EdgeChangedEventArgs_t edge_args;

static struct timespec start_ts;
static struct timespec end_ts;
static struct timespec delay_ts;
static long delta_ns;
static long edge_frequency_ns;

static void flip_edge();

void vclock_init(uint8_t freq)
{
    start_ts.tv_nsec = 0;
    start_ts.tv_sec = 0;
    end_ts.tv_nsec = 0;
    end_ts.tv_sec = 0;
    delay_ts.tv_nsec = 0;
    delay_ts.tv_sec = 0;
    delta_ns = 0;
    // SEC_TO_NS(1 / freq) gets the actual frequency in ns.
    // Dividing it by 2 gives the time between each edge.
    edge_frequency_ns = SEC_TO_NS(1 / freq) / 2;

    vclock.device_info = device_table_add(DEVICE_TYPE_CLOCK);
    vclock.logger.device_info = vclock.device_info;
    vclock.logger.file_name = "../logs/vclock.txt";
    logger_init(&vclock.logger);
    log_info(&vclock.logger, "Initializing.\n");
}

void vclock_start()
{
    while (1)
    {
        flip_edge();
    }
}

void vclock_shutdown()
{
    log_info(&vclock.logger, "Shutting down.\n");
    logger_shutdown(&vclock.logger);
}

void flip_edge()
{
    timespec_get(&start_ts, TIME_UTC);
    edge_args.edge = !edge_args.edge;
    event_notify("edge_changed", (void *)&edge_args);
    timespec_get(&end_ts, TIME_UTC);

    delta_ns = SEC_TO_NS(end_ts.tv_sec - start_ts.tv_sec) + (end_ts.tv_nsec - start_ts.tv_nsec);
    delay_ts.tv_nsec = edge_frequency_ns - delta_ns;

    if (delay_ts.tv_nsec > 0)
    {
        if (nanosleep(&delay_ts, NULL))
        {
            printf("Timing delay failed. Terminating.\n");
            exit(1);
        }
    }
}