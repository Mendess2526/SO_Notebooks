#include "strings.h"

#include <string.h>

void string_init(String* s, char* string, size_t length){
    s->s = (char*) malloc(sizeof(char)*length);
    memcpy(s->s, string, length);
    s->length = length;
}

void string_free(String s){
    free(s.s);
}
