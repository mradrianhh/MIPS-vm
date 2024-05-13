#include <stdlib.h>
#include <string.h>

#include "interrupt_event.h"

static InterruptEvent_t event;

int interrupt_event_init()
{
    event._handlers_start = malloc(INTERRUPT_EVENT_HANDLERS_SIZE);
    event._handlers_next = event._handlers_start;
    memset(event._handlers_start, 0, INTERRUPT_EVENT_HANDLERS_SIZE);
    pthread_mutex_init(&event._mutex, NULL);

    return 0;
}

InterruptEvent_t *interrupt_event_get()
{
    return &event;
}

InterruptArgs_t interrupt_args_create(uint8_t device_id, uint8_t interrupt_code)
{
    InterruptArgs_t args = {
        .device_id = device_id,
        .interrupt_code = interrupt_code};

    return args;
}

void interrupt_event_subscribe(InterruptEvent_t *event, uint8_t device_id, LOGGER_t *logger, interrupt_handler_func_t handler)
{
    pthread_mutex_lock(&event->_mutex);
    event->_handlers_next->device_id = device_id;
    event->_handlers_next->logger = logger;
    event->_handlers_next->handler = handler;
    event->_handlers_next++;
    pthread_mutex_unlock(&event->_mutex);

    log_trace(logger, "Device (%d) subscribed to interrupt event.\n", device_id);
}

void interrupt_event_trigger(InterruptEvent_t *event, InterruptArgs_t args)
{
    pthread_mutex_lock(&event->_mutex);
    for (int i = 0; &event->_handlers_start[i] != event->_handlers_next; i++)
    {
        log_event(event->_handlers_start[i].logger,
                  "Event triggered. Args: { Interrupt Code: %d, Device ID: %d}.\n",
                  args.interrupt_code, args.device_id);
        event->_handlers_start[i].handler(event->_handlers_start[i].logger, args);
    }
    pthread_mutex_unlock(&event->_mutex);
}
