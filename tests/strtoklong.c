#include <stdio.h>
#include <string.h>

int main(){
    char line[] = "grep .c &> file & wc";
    char* tokens[10];
    int i = 0;
    char* cur = line;
    char* c = strstr(line, "& ");
    while(c != NULL){
        *c = '\0';
        tokens[i++] = cur;
        cur = c + 2;
        c = strstr(cur, "& ");
    };
    tokens[i++] = cur;
    tokens[i] = NULL;
    for(i = 0; tokens[i] != NULL; i++){
        printf("%s\n", tokens[i]);
    }
}
