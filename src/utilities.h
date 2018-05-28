#ifndef UTILITIES_H
#define UTILITIES_H

#include <unistd.h>

char* readLn(int fd, size_t* nBytes);

char** words(const char* string, size_t len);

size_t int2string(int num, char* string, size_t len);

char* strnstr(const char* haystack, const char* needle, size_t n);

char* str_n_dup(const char* str, size_t len);

char* str_dup(const char* str);

#endif /* UTILITIES_H */
