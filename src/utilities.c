#include "utilities.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * Returns the index of a char in a string or -1 if it doesn't occur.
 * @param str The string.
 * @param c The character.
 * @param len The length of the string.
 * @return The index of a char in a string or -1 if it doesn't occur.
 */
static ssize_t indexOf(const char* str, char c, size_t len);

/**
 * Shifts the contents of a string to the left.
 * @param str The string.
 * @param offset The offset to shift from.
 * @param n The amount of characters to shift.
 */
static inline void strShift(char* str, size_t offset, size_t n);

/**
 * Resets a buffer. Frees the memory and sets it to NULL.
 * @param buff A pointer to the buffer
 * @param load The size to be reset.
 */
static inline void resetBuff(char** buff, size_t* load);

/**
 * Counts the amount of digits in an int.
 * @param n The int
 * @return The number of digits.
 */
static inline size_t countDigits(int n);

static ssize_t indexOf(const char* str, char c, size_t len){
    size_t i = 0;
    while(i < len){
        if(str[i] == c) return i;
        i++;
    }
    return -1;
}

static void strShift(char* str, size_t offset, size_t n){
    for(size_t i = 0; i < n; i++, offset++)
        str[i] = str[offset];
}

static void resetBuff(char** buff, size_t* load){
    free(*buff);
    *buff = NULL;
    *load = 0;
}

char* readLn(int fd, size_t* nBytes){
    static char* buff;
    static size_t buffSize;
    static size_t buffLoad;
    *nBytes = 0;
    if(fd < 0){
        resetBuff(&buff, &buffLoad);
        return NULL;
    }
    if(buff == NULL){
        buffSize = 1024;
        buffLoad = 0;
        buff = malloc(sizeof(char) * buffSize);
    }
    ssize_t newLineOffset;
    if((newLineOffset = indexOf(buff, '\n', buffLoad)) < 0){
        ssize_t n;
        while(0 < (n = read(fd, buff + buffLoad, buffSize - buffLoad))){
            char* chunk = buff + buffLoad;
            buffLoad += n;
            if((newLineOffset = indexOf(chunk, '\n', (size_t) n)) > -1) break;
            if(buffLoad >= buffSize){
                buffSize *= 2;
                buff = realloc(buff, sizeof(char) * buffSize);
            }
        }
    }
    if(newLineOffset > -1){
        char* ret = str_n_dup(buff, (size_t) newLineOffset);
        *nBytes = (size_t) newLineOffset;
        strShift(buff, (size_t) (newLineOffset + 1), buffLoad - newLineOffset - 1);
        buffLoad -= (newLineOffset + 1);
        return ret;
    }else{
        char* ret = str_n_dup(buff, buffLoad);
        *nBytes = buffLoad;
        resetBuff(&buff, &buffLoad);
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

static size_t countDigits(int n){
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

