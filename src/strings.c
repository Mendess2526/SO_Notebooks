#include "strings.h"

#include <string.h>

void string_init(String* s, char* string, size_t length){
    s->s = (char*) malloc(sizeof(char) * length);
    memcpy(s->s, string, length);
    s->length = length;
}

void string_append(String* dest, String src){
    dest->s = (char*) realloc(dest->s,
            sizeof(char) * (dest->length + src.length));
    memcpy(dest->s + dest->length - 1, src.s, src.length);
    dest->length = dest->length + src.length;
}

void string_free(String s){
    free(s.s);
}
