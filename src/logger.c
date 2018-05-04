#include "logger.h"
#include "colors.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

static inline char* paintMessage(char* color, char* message){
    int cLen = strlen(color);
    int mLen = strlen(message);
    int rLen = strlen(RESET);
    char* m = malloc(sizeof(char) * (cLen + mLen + rLen + 1));
    strncpy(m, color, cLen);
    strncpy(m + cLen, message, mLen);
    strncpy(m + cLen + mLen, RESET, rLen);
    m[cLen+mLen+rLen] = '\0';
    return m;
}

inline void LOG_FATAL(char* message){
    char* paintedMsg = paintMessage(RED, message);
    write(2, paintedMsg, strlen(paintedMsg));
    _exit(1);
}

inline void LOG_CRITICAL(char* message){
    char* paintedMsg = paintMessage(YELLOW, message);
    write(2, paintedMsg, strlen(paintedMsg));
}

inline void LOG_WARNING(char* message){
    char* paintedMsg = paintMessage(PURPLE, message);
    write(2, paintedMsg, strlen(paintedMsg));
}
