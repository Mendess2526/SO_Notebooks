#ifndef UTILITIES_H
#define UTILITIES_H
/**
 * \file
 * TODO
 */
#include <unistd.h>

/**
 * \brief Reads a line from the given file descriptor
 *
 * \param fd File descriptor
 * \param nBytes number of bytes to read
 * \returns The line read or NULL if nothing could be read
 */
char* readLn(int fd, size_t* nBytes);

/**
 * \brief Creates an array of words from the given string
 *
 * \param string The string to separate
 * \param len The length of string
 * \returns The array of strings
 */
char** words(const char* string, size_t len);

/**
 * \brief Convert an int to a string
 *
 * \param num The number
 * \param string The buffer to store the converted number
 * \param len The size of the buffer
 * \returns The amount of char's ocupied by the string
 */
size_t int2string(int num, char* string, size_t len);

/**
 * \brief Copy to a new string n elements of the given
 *
 * \param str The string to copy from
 * \param len The number of elements to copy
 * \returns The copy
 */
char* str_n_dup(const char* str, size_t len);

/**
 * \brief Creates a copy of a string
 *
 * \param str The string to copy
 * \returns The copy
 */
char* str_dup(const char* str);

/**
 * \brief Returns a pointer to the first ocurrence of the needle in the
 * haystack or NULL if needle doesn't ocurr in the first n char's
 *
 * \param haystack The string to search
 * \param needle The string to find
 * \param n The length of haystack to check
 * \returns A pointer to the first ocurrence or NULL if not found
 */
char* strnstr(const char* haystack, const char* needle, size_t n);

#endif /* UTILITIES_H */
