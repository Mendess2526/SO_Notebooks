#include "parse_tree.h"
#include "utilities.h"

#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>

void parse_and_dump(char *file){
    int fd = open(file, O_RDONLY);
    ParseTree pt = parse_tree_create(1);
    char* line;
    size_t n;
    while((line = readln(fd, &n)) != NULL){
        parse_tree_add_line(pt, line, n);
        free(line);
    }
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

int main(){

    /* Notebook 0 */
    parse_and_dump("notebook0.nb");
    /* Notebook 1 */
    parse_and_dump("notebook1.nb");
    return 0;
}
