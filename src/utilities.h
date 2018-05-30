	#ifndef UTILITIES_H
#define UTILITIES_H

#include <unistd.h>

/**
 * Reads the content from the given file descriptor
 *
 * \param fd File descriptor
 * \param nBytes number of bytes to read
 */
char* readLn(int fd, size_t* nBytes);

/**
 * Separates a string to an array of strings
 *
 * \param string String to separate
 * \param len Length of string
 */
char** words(const char* string, size_t len);

size_t int2string(int num, char* string, size_t len);

char* strnstr(const char* haystack, const char* needle, size_t n);

char* str_n_dup(const char* str, size_t len);

char* str_dup(const char* str);

#endif /* UTILITIES_H */
