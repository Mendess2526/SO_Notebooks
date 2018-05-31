#ifndef UTILITIES_H
#define UTILITIES_H

#include <unistd.h>

/**
 * Reads a line from the given file descriptor
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

/**
 * Convert an int to string
 * 
 * \param num Number to convert
 * \param string String to update
 * \param len Length of string
 */
size_t int2string(int num, char* string, size_t len);

/**
 * Copy to a new string n elements of the old string 
 *
 * \param str String to copy from
 * \param len Number of elements of the old string to copy
 */
char* str_n_dup(const char* str, size_t len);

/**
 * Copies a string 
 *
 * \param str String to copy from
 */
char* str_dup(const char* str);

/**
 * Verifies if n first elements of a string are into another one
 *
 * \param haystack The string to verify
 * \param needle The string where to verify
 * \param n Number of elements of the string haystack
 */
char* strnstr(const char* haystack, const char* needle, size_t n);
#endif /* UTILITIES_H */
