#ifndef STRINGS_H
#define STRINGS_H
/**
 * \file
 * TODO
 */
#include <stdlib.h>
#include <string.h>

/**
 * \brief A representation of a String, storing the array of char's
 * and the it's length.
 */
typedef struct string{
    char* s;            /**< The array of char */
    size_t length;      /**< The length of the array */
}String;

/**
 * \brief Initializes the given string, allocating the array and copying
 * length number of char from string.
 *
 * \param s A pointer to the string.
 * \param string The string to copy.
 * \param length The number of char's to copy.
 */
void string_init(String* s, char* string, size_t length);

/**
 * \brief Appends the string src to dest
 *
 * \param dest Destination string
 * \param src String to append
 */
void string_append(String* dest, String src);

/**
 * \brief Appends an array of char to the string.
 *
 * \param dest The string to append to.
 * \param array The array to append.
 * \param len The size of the array.
 */
void string_append_array(String* dest, char* array, size_t len);

/**
 * \brief Free the memory used by a String
 *
 * \param s The string to free.
 */
void string_free(String s);

#endif /* STRING_H */
