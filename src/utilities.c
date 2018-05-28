#include "utilities.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char* readLn(int fd, size_t* nBytes){
    size_t size = 0;
    size_t buffSize = 1;
    *nBytes = 0;
    char c;
    char* buff = NULL;
    while(0 < read(fd, &c, 1)){
        if(c == '\0' || c == '\n'){
            *nBytes = size;
            return buff;
        }
        if(size >= (buffSize - 1)){
            buffSize *= 2;
            buff = realloc(buff, sizeof(char) * buffSize);
        }
        buff[size++] = c;
    }
    *nBytes = size;
    return buff;
}

char** words(const char* string, size_t len){
    char* command = malloc(sizeof(char) * len + 1);
    strncpy(command, string, len);
    command[len] = '\0';
    int argc = 5;
    char** argv = malloc(argc * sizeof(char*));
    int i = 0;
    char* token = strtok(command, " ");
    do{
        if(i >= argc - 1)
            argv = realloc(argv, (argc *= 2) * sizeof(char*));

        argv[i++] = str_dup(token);

        token = strtok(NULL, " ");
    }while(token);
    argv[i] = NULL;
    free(command);
    return argv;
}

static inline size_t countDigits(int n){
    size_t count = 0;
    while(n != 0){
        n /= 10;
        ++count;
    }
    return count;
}

size_t int2string(int num, char* string, size_t len){
    size_t r = countDigits(num);
    if(r >= len) return 0;
    ssize_t i = r - 1;
    for(; i >= 0; i--){
        string[i] = (char) ('0' + num % 10);
        num /= 10;
    }
    return r;
}

char* str_n_dup(const char* str, size_t len){
    char* s = malloc(sizeof(char) * len);
    strncpy(s, str, len);
    return s;
}

char* str_dup(const char* str){
    return str_n_dup(str, strlen(str) + 1);
}

char* strnstr(const char* haystack, const char* needle, size_t n){
    for(size_t i = 0; i < n; i++){
        int succeded = 1;
        for(size_t j = i, k = 0; succeded && j < n && needle[k]; j++, k++)
            if(haystack[j] != needle[k]) succeded = 0;
        if(succeded) return (char*) &haystack[i];
    }
    return NULL;
}

