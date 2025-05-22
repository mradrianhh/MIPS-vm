#ifndef _MIPSVM_GUEST_COMMON_EVENTS_H_
#define _MIPSVM_GUEST_COMMON_EVENTS_H_
#define MAX_EVENT_SUBSCRIPTIONS         (10)
#define MAX_EVENT_SUBSCRIPTIONS_SIZE    (sizeof(_EventSubscription_t) * MAX_EVENT_SUBSCRIPTIONS)
#define MAX_EVENT_NAME_SIZE             (50)

#include <stdint.h>
#include <pthread.h>

#include "common/logger/logger.h"

typedef void(*event_handler)(const void *args);

typedef struct _Event _Event_t;
typedef struct _EventTable _EventTable_t;
typedef struct _EventSubscription _EventSubscription_t;

struct _Event
{
    char event_name[MAX_EVENT_NAME_SIZE];
    _Event_t *_next_event;
    _EventSubscription_t *_root_subscription;
};

struct _EventTable
{
    _Event_t *_root_event;
    LOGGER_t _logger;
    pthread_mutex_t _mutex;
};

struct _EventSubscription
{
    _EventSubscription_t *_next_subscription;
    event_handler _handler;
};

void events_init();

void events_shutdown();

int event_create(const char *event_name);

int event_remove(const char *event_name);

int event_notify(const char *event_name, const void *args);

int event_subscribe(const char *event_name, event_handler handler);

int event_unsubscribe(const char *event_name, event_handler handler);

#endif
