#ifndef LOGGER_H
#define LOGGER_H

#include "strings.h"

void LOG_FATAL(char* message);

#define LOG_CRITICAL(message) _log_CRITICAL(message); return;

void _log_CRITICAL(char* message);

void LOG_PARSE_ERROR(String line, int lineNumber, char* message, int errOffset);

void LOG_WARNING(char* message);

#endif /* LOGGER_H */
