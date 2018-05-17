#ifndef LOGGER_H
#define LOGGER_H

#include "strings.h"

/**
 * Logs a fatal error message and terminates the program
 *
 * \param message The message to log.
 */
void LOG_FATAL(char* message);

/**
 * Logs a critical error and returns from the current function.
 *
 * \param message The message to log.
 */
#define LOG_CRITICAL(message) _log_CRITICAL(message); return;

/**
 * Logs a critical error message. Should be used in conjunction with
 * the LOG_CRITICAL macro.
 *
 * \param message The message to log.
 */
void _log_CRITICAL(char* message);

/**
 * Logs a parse error and terminates the program.
 *
 * \param line The line where the error was found.
 * \param lineNumber The number of the line in the file.
 * \param message The error message.
 * \param errOffset The offset in the line where the error was found
 */
void LOG_PARSE_ERROR(String line, int lineNumber, char* message, int errOffset);

/**
 * Logs a warning message.
 *
 * \param message The message to log.
 */
void LOG_WARNING(char* message);

#endif /* LOGGER_H */
