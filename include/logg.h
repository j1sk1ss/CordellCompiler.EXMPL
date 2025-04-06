#ifndef LOGG_H_
#define LOGG_H_

#include <stdio.h>
#include <stdarg.h>

#define LOG_FILE_NAME_SIZE  16
#define LOG_FILE_SIZE       100

#ifdef ERROR_LOGS
    #define print_error(message, ...)   log_message("ERROR", __FILE__, __LINE__, message, ##__VA_ARGS__)
#else
    #define print_error(message, ...)
#endif

#ifdef WARNING_LOGS
    #define print_warn(message, ...)    log_message("WARN", __FILE__, __LINE__, message, ##__VA_ARGS__)
#else
    #define print_warn(message, ...)
#endif

#ifdef INFO_LOGS
    #define print_info(message, ...)    log_message("INFO", __FILE__, __LINE__, message, ##__VA_ARGS__)
#else
    #define print_info(message, ...)
#endif

#ifdef DEBUG_LOGS
    #define print_debug(message, ...)   log_message("DEBUG", __FILE__, __LINE__, message, ##__VA_ARGS__)
#else
    #define print_debug(message, ...)
#endif

#ifdef IO_OPERATION_LOGS
    #define print_io(message, ...)      log_message("I/O", __FILE__, __LINE__, message, ##__VA_ARGS__)
#else
    #define print_io(message, ...)
#endif

#ifdef MEM_OPERATION_LOGS
    #define print_mm(message, ...)      log_message("MEM", __FILE__, __LINE__, message, ##__VA_ARGS__)
#else
    #define print_mm(message, ...)
#endif

#ifdef LOGGING_LOGS
    #define print_log(message, ...)     log_message("LOG", __FILE__, __LINE__, message, ##__VA_ARGS__)
#else
    #define print_log(message, ...)
#endif

#ifdef SPECIAL_LOGS
    #define print_spec(message, ...)    log_message("SPEC", __FILE__, __LINE__, message, ##__VA_ARGS__)
#else
    #define print_spec(message, ...)
#endif


/*
Write log to file descriptor.

Params:
- level - Log level.
- file - File descriptor.
- line - Code line number.
- message - Additional info message.
- args - Args.
*/
void _write_log(const char* level, const char* file, int line, const char* message, va_list args);

/*
Create log message.

- level - Log level.
- file - File name.
- line - Code line number.
- message - Additional info message.
*/
void log_message(const char* level, const char* file, int line, const char* message, ...);

#endif