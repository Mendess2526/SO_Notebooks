#include "utilities.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

ssize_t readln(int fd, void *buf, ssize_t nbyte){
    ssize_t size = 0;
    char c;
    char *buff = (char *) buf;
    while(size < nbyte && 1 == read(fd, &c, 1)){
        if(c == '\0' || c == '\n') return size;
        buff[size++] = c;
    }
    return size;
}
