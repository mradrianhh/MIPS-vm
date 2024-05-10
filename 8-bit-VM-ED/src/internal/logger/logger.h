#ifndef _8BITVM_LOGGER_H_
#define _8BITVM_LOGGER_H_

#include <stdio.h>
#include <pthread.h>

#include "internal/device_table/device_table.h"

#define MAX_FILENAME_SIZE 256

struct LOGGER
{
    DEVICE_TABLE_ENTRY_t *device_info;
    FILE *file_pointer;
    char *file_name;
};
typedef struct LOGGER LOGGER_t;

int logger_init(LOGGER_t *logger);

int log_error(const LOGGER_t *logger, const char *format, ...);

int log_event(const LOGGER_t *logger, const char *format, ...);

int log_info(const LOGGER_t * logger, const char *format, ...);

int log_debug(const LOGGER_t *logger, const char *format, ...);

int log_trace(const LOGGER_t *logger, const char *format, ...);

int logger_shutdown(LOGGER_t *logger);


#endif