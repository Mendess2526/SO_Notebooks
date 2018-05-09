#ifndef UTILITIES_H
#define UTILITIES_H

#include <unistd.h>

char* readln(int fd, size_t* nbyte);

char** words(char *string, int len);

#endif /* UTILITIES_H */
