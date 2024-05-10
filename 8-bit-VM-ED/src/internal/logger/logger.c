#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "logger.h"

static int log_write(const LOGGER_t *logger, const char *log_level, const char *format, va_list args);
static pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER;

int logger_init(LOGGER_t *logger)
{

    if ((logger->file_pointer = fopen(logger->file_name, "w")) == NULL)
    {
        printf("%s(%d)-LOGGER - Error: Can't open file '%s'.\n", convert_device_type_str(logger->device_info->device_type), logger->device_info->device_id, logger->file_name);
        return 1;
    }

    return 0;
}

int log_error(const LOGGER_t *logger, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_write(logger, "ERROR", format, args);
    va_end(args);

    return 0;
}

int log_event(const LOGGER_t *logger, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_write(logger, "EVENT", format, args);
    va_end(args);

    return 0;
}

int log_info(const LOGGER_t * logger, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_write(logger, "INFO", format, args);
    va_end(args);

    return 0;
}

int log_debug(const LOGGER_t *logger, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_write(logger, "DEBUG", format, args);
    va_end(args);

    return 0;
}

int log_trace(const LOGGER_t *logger, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_write(logger, "TRACE", format, args);
    va_end(args);

    return 0;
}

int logger_shutdown(LOGGER_t *logger)
{
    return fclose(logger->file_pointer);
}

static int log_write(const LOGGER_t *logger, const char *log_level, const char *format, va_list args)
{
    pthread_mutex_lock(&_mutex);
    int rc = 0;

    struct tm *timer_fmt;
    time_t timer;
    time(&timer);
    timer_fmt = gmtime(&timer);

    if (logger->device_info != NULL)
    {
        char *device_type_str = convert_device_type_str(logger->device_info->device_type);
        fprintf(logger->file_pointer, "[Thread: 0x%016lx] - [UTC: %02d-%02d-%04d %02d-%02d-%02d] - Device %s(%d) - %s: ", logger->device_info->device_tid, timer_fmt->tm_mday, (timer_fmt->tm_mon + 1), (timer_fmt->tm_year + 1900),
                timer_fmt->tm_hour, timer_fmt->tm_min, timer_fmt->tm_sec,
                device_type_str, logger->device_info->device_id, log_level);
        vfprintf(logger->file_pointer, format, args);
    }
    else
    {
        fprintf(logger->file_pointer, "[UTC: %02d-%02d-%04d %02d-%02d-%02d] - %s: ", timer_fmt->tm_mday, (timer_fmt->tm_mon + 1), (timer_fmt->tm_year + 1900),
                timer_fmt->tm_hour, timer_fmt->tm_min, timer_fmt->tm_sec, log_level);
        vfprintf(logger->file_pointer, format, args);
    }

    fflush(logger->file_pointer);
    pthread_mutex_unlock(&_mutex);

    return 0;
}
