#include "internal/events/events.h"
#include "internal/device_table/device_table.h"
#include "devices/vcpu/vcpu.h"
#include "devices/vmemory/vmemory.h"
#include "devices/vclock/vclock.h"

static void init();
static void configure();
static void shutdown();

int main()
{
    set_log_level(LOG_LEVEL_INFO);
    init();
    configure();
    vclock_start();
    shutdown();
    return 0;
}

void init()
{
    device_table_init();
    events_init();
    vclock_init(1);
    vcpu_init();
    vmemory_init();
}

void configure()
{
    event_create("edge_changed");
    event_subscribe("edge_changed", vcpu_update);
}

void shutdown()
{
    vmemory_shutdown();
    vcpu_shutdown();
    vclock_shutdown();
    events_shutdown();
}