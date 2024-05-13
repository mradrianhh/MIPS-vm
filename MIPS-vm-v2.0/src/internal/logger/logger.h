#ifndef _8BITVM_LOGGER_H_
#define _8BITVM_LOGGER_H_

#include <stdio.h>
#include <pthread.h>

#include "internal/device_table/device_table.h"

#define MAX_FILENAME_SIZE 256

enum LogLevel
{
    LOG_LEVEL_NONE = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_INFO = 2,
    LOG_LEVEL_EVENT = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_TRACE = 5,
    LOG_LEVEL_FULL = 15
};
typedef enum LogLevel LogLevel_t;

struct LOGGER
{
    DEVICE_TABLE_ENTRY_t *device_info;
    FILE *file_pointer;
    char *file_name;
};
typedef struct LOGGER LOGGER_t;

int logger_init(LOGGER_t *logger);

int set_log_level(LogLevel_t log_level);

int log_error(const LOGGER_t *logger, const char *format, ...);

int log_event(const LOGGER_t *logger, const char *format, ...);

int log_info(const LOGGER_t * logger, const char *format, ...);

int log_debug(const LOGGER_t *logger, const char *format, ...);

int log_trace(const LOGGER_t *logger, const char *format, ...);

int logger_shutdown(LOGGER_t *logger);


#endif