#include "utilities.h"
#include <string.h>
#include <stdio.h>


void cleanCommand(char** words){
    for(size_t i = 0; words[i]; i++){
        if(strstr(words[i], "<") != NULL || strstr(words[i], ">") != NULL){
            words[i] = NULL;
            return;
        }
    }
}

int main(){
    char line[] = "ls /etc/ &> lolkek";
    char** word = words(line, strlen(line));
    for(int i = 0; word[i]; i++) printf("+%s\n", word[i]);
    cleanCommand(word);
    for(int i = 0; word[i]; i++) printf("-%s\n", word[i]);
}
