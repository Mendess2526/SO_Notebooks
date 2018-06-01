#include "utilities.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

ssize_t indexOf(const char* str, char c, size_t len){
    size_t i = 0;
    while(i < len){
        if(str[i] == c) return i;
        i++;
    }
    return -1;
}

static inline void strshift(char* str, size_t offset, size_t n){
    for(size_t i = 0; i < n; i++, offset++)
        str[i] = str[offset];
}

char* readLn(int fd, size_t* nBytes){
    static char* buff;
    static size_t buffSize;
    static size_t buffLoad;
    *nBytes = 0;
    if(buff == NULL){
        buffSize = 1024;
        buffLoad = 0;
        buff = malloc(sizeof(char) * buffSize);
    }
    ssize_t newLineOffset;
    if((newLineOffset = indexOf(buff, '\n', buffLoad)) < 0){
        size_t n;
        while(0 < (n = read(fd, buff + buffLoad, buffSize - buffLoad))){
            char* chunk = buff + buffLoad;
            buffLoad += n;
            if((newLineOffset = indexOf(chunk, '\n', n)) > -1) break;
            if(buffLoad >= buffSize){
                buffSize *= 2;
                buff = realloc(buff, sizeof(char) * buffSize);
            }
        }
    }
    if(newLineOffset > -1){
        char* ret = str_n_dup(buff, newLineOffset);
        *nBytes = newLineOffset;
        strshift(buff, newLineOffset + 1, buffLoad - newLineOffset - 1);
        buffLoad -= (newLineOffset + 1);
        return ret;
    }else{
        char* ret = str_n_dup(buff, buffLoad);
        *nBytes = buffLoad;
        free(buff);
        buff = NULL;
        buffLoad = 0;
        return ret;
    }
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
    if(len < 1 || !str) return NULL;
    char* s = malloc(sizeof(char) * len);
    strncpy(s, str, len);
    return s;
}

char* str_dup(const char* str){
    if(!str) return NULL;
    return str_n_dup(str, strlen(str) + 1);
}

char* strnstr(const char* haystack, const char* needle, size_t n){
    for(size_t i = 0; i < n; i++){
        int succeeded = 1;
        for(size_t j = i, k = 0; succeeded && j < n && needle[k]; j++, k++)
            if(haystack[j] != needle[k]) succeeded = 0;
        if(succeeded) return (char*) &haystack[i];
    }
    return NULL;
}

