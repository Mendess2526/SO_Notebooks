#include "utilities.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

char* readln(int fd, size_t* nbyte){
    size_t size = 0;
    size_t buffSize = 1;
    *nbyte = 0;
    char c;
    char *buff = NULL;
    while(0 < read(fd, &c, 1)){
        if(c == '\0' || c == '\n'){
            *nbyte = size;
            return buff;
        }
        if(size >= (buffSize - 1)){
            buffSize *= 2;
            buff = realloc(buff, sizeof(char) * buffSize);
        }
        buff[size++] = c;
    }
    *nbyte = size;
    return buff;
}
