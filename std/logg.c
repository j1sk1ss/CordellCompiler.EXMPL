#include "../include/logg.h"


void _write_log(const char* level, const char* file, int line, const char* message, va_list args) {
    if (level == NULL) level = "(null)";
    if (file == NULL) file = "(null)";
    if (message == NULL) message = "(null)";

    fprintf(stdout, "[%s] (%s:%i) ", level, file, line);
    vfprintf(stdout, message, args);
    fprintf(stdout, "\n");
}

void log_message(const char* level, const char* file, int line, const char* message, ...) {
    va_list args;
    va_start(args, message);
    _write_log(level, file, line, message, args);
    va_end(args);
}
