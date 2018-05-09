#include "utilities.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

char** words(char *string, int len){
    char *command = malloc(sizeof(char) * len);
    strncpy(command, string, len);
    int argc = 5;
    char **argv = malloc(argc*sizeof(char *));
    int i=0;
    char *token = strtok(command," ");
    do{
        if(!(i<argc))
            argv = realloc(argv,(argc*=2)*sizeof(char *));

        argv[i++] = token;

        token = strtok(NULL," ");
    }while(token);
    argv[i] = NULL;
    return argv;
}


