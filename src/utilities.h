#ifndef UTILITIES_H
#define UTILITIES_H

#include <unistd.h>

char* readLn(int fd, size_t* nBytes);

char** words(char *string, size_t len);

size_t int2string(int num, char* string, size_t len);

#endif /* UTILITIES_H */
