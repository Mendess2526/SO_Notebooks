#ifndef STRINGS_H
#define STRINGS_H

#include <stdlib.h>
#include <string.h>

typedef struct string{
    char* s;
    size_t length;
}String;

void string_init(String* s, char* string, size_t length);

void string_append(String *dest, String src);

void string_free(String s);

#endif /* STRING_H */
