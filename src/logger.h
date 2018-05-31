#ifndef LOGGER_H
#define LOGGER_H
/**
 * \file
 * Hellper functions to log errors to the screen.
 */
#include "strings.h"

/**
 * \brief Logs a fatal error message.
 *
 * \param message The message to log.
 */
void LOG_FATAL(char* message);

/**
 * \brief Logs a fatal error message.
 *
 * \param message The message to log.
 */
void LOG_FATAL_STRING(String message);

/**
 * \brief Logs a critical error and returns from the current function.
 *
 * \param message The message to log.
 */
#define LOG_CRITICAL(message) _log_CRITICAL(message); return;

/**
 * \brief Logs a critical error message. Should be used in conjunction with
 * the LOG_CRITICAL macro.
 *
 * \param message The message to log.
 */
void _log_CRITICAL(char* message);

/**
 * \brief Logs a parse error and terminates the program.
 *
 * \param line The line where the error was found.
 * \param lineNumber The number of the line in the file.
 * \param message The error message.
 * \param errOffset The offset in the line where the error was found
 */
void LOG_PARSE_ERROR(String line, int lineNumber, char* message, int errOffset);

/**
 * \brief Logs a warning message.
 *
 * \param message The message to log.
 */
void LOG_WARNING(char* message);

/**
 * \brief Logs a warning message.
 *
 * \param message The message to log.
 */
void LOG_WARNING_STRING(String message);

#endif /* LOGGER_H */
