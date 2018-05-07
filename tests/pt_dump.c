#include "parse_tree.h"
#include "utilities.h"
#include "colors.h"

#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>

void parse_and_dump(char *file){
    int fd = open(file, O_RDONLY);
    ParseTree pt = parse_tree_create(1);
    char* line;
    size_t n;
    while((line = readln(fd, &n)) != NULL){
        printf("%d, ", parse_tree_add_line(pt, line, n));
        free(line);
    }
    printf("%d", parse_tree_add_line(pt, NULL, 0));
    printf("\n");
    char** dump = parse_tree_dump(pt);
    int i = 0;
    while(dump[i]){
        printf("%s\n",dump[i]);
        free(dump[i++]);
    }
    free(dump);
    close(fd);
    parse_tree_destroy(pt);
}

int main(int argc, char** argv){
    if(argc < 2){
        printf(GREEN "Notebook 3" RESET "\n");
        parse_and_dump("notebook3.nb");
        printf("\n");
        return 0;
    }
    for(int i = 1; i < argc; i++){
        char filename[30];
        printf(GREEN "Notebook %s" RESET "\n", argv[i]);
        sprintf(filename, "notebook %s", argv[i]);
        parse_and_dump(filename);
    }
    return 0;
}
