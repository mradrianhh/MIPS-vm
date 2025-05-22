#include "guest/common/events/events.h"
#include "guest/common/device_table/device_table.h"
#include "guest/devices/vcpu/vcpu.h"
#include "guest/devices/vmemory/vmemory.h"
#include "guest/devices/vmemory/vtlb.h"
#include "guest/devices/vclock/vclock.h"
#include "guest/common/loader/loader.h"

static void init();
static void configure();
static void shutdown();

int main()
{
    set_log_level(LOG_LEVEL_DEBUG);
    init();
    configure();
    vcpu_reset();
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
    vtlb_init();
    vmemory_init();
}

void configure()
{
    event_create("edge_changed");
    event_subscribe("edge_changed", vcpu_update);
    loader_flash_rom("../sdk/demo/hello_world/build/helloworld");
}

void shutdown()
{
    vmemory_shutdown();
    vcpu_shutdown();
    vclock_shutdown();
    events_shutdown();
}
