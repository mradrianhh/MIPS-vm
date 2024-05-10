#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "internal/events/events.h"
#include "internal/device_table/device_table.h"
#include "devices/vcpu/vcpu.h"

int main()
{
    // Create a clock that loops while system is powered on.
    // At a set frequency, trigger the systick event.
    // All other devices, has a function subscribed to this event that gets triggered at each tick.
    // This way, everything happens synchronously, and we're able to 
    // emulate the nature of a computer.
    device_table_init();    
    events_init();

    event_create("init");
    event_create("update");
    event_create("shutdown");

    event_subscribe("init", vcpu_init);
    event_subscribe("update", vcpu_update);
    event_subscribe("shutdown", vcpu_shutdown);

    event_notify("init", NULL);

    int i = 0;
    while(i < 5){
        event_notify("update", NULL);
        sleep(3);
        i++;
    }

    event_notify("shutdown", NULL);

    return 0;
}