#ifndef _8BITVM_INTERRUPT_EVENT_H_
#define _8BITVM_INTERRUPT_EVENT_H_
#define  MAX_INTERRUPT_EVENT_HANDLERS 10
#define  INTERRUPT_EVENT_HANDLERS_SIZE (sizeof(interrupt_handler_func_t) * MAX_INTERRUPT_EVENT_HANDLERS)

#include <stdint.h>
#include <pthread.h>

#include "logger/logger.h"

// InterruptArgs contains the interrupt_code raised, and the device_id of the device that raised it.
struct InterruptArgs
{
    uint8_t interrupt_code;
    uint8_t device_id;
};
typedef struct InterruptArgs InterruptArgs_t;

// interrupt_handler_func_t is the function signature for an interrupt handler.
typedef void(*interrupt_handler_func_t)(LOGGER_t *logger, InterruptArgs_t args);

// InterruptHandler is an internal construct used in managing interrupt subscriptions.
struct InterruptHandler
{
    // Used to identify a subscription.
    uint8_t device_id;
    // Used to log an interrupt.
    LOGGER_t *logger;
    // Function to call.
    interrupt_handler_func_t handler;
};
typedef struct InterruptHandler InterruptHandler_t;

// InterruptEvent contains a list of InterruptHandler's whose handler-functions is called
// when an event is triggered.
struct InterruptEvent
{
    // Pointer to start of interrupt-handlers list. For internal use.
    InterruptHandler_t *_handlers_start;
    // Pointer to the next available space of interrupt-handlers list. For internal use.
    InterruptHandler_t *_handlers_next;
    // Mutex to ensure multiple threads aren't changing or triggering the interrupt at the same time.
    pthread_mutex_t _mutex;
};
typedef struct InterruptEvent InterruptEvent_t;

// Initialize the interrupt event.
int interrupt_event_init();

// A device is able to retrieve a reference to the interrupt event.
InterruptEvent_t *interrupt_event_get();

// A device is able to create EventArgs_t.
InterruptArgs_t interrupt_args_create(uint8_t device_id, uint8_t interrupt_code);

// A device is able to subscribe to the interrupt event with its device_id, a reference to it's logger 
// and a handler-function.
// A device can only subscribe to the interrupt event once.
void interrupt_event_subscribe(InterruptEvent_t *event, uint8_t device_id, LOGGER_t *logger, interrupt_handler_func_t handler);

// A device is able to unsubscribe to an event with its device_id.
//void event_unsubscribe(Event_t *event, uint8_t device_id);

// A device is able to trigger an event.
void interrupt_event_trigger(InterruptEvent_t *event, InterruptArgs_t args);

#endif