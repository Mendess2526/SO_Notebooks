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
        parse_tree_add_line(pt, line, n);
        free(line);
    }
    parse_tree_print(pt);
    close(fd);
    parse_tree_destroy(pt);
}

int main(){

    /* Notebook 0 */
    printf(GREEN "Notebook 0" RESET "\n");
    parse_and_dump("notebook0.nb");
    printf("\n");
    /* Notebook 1 */
    printf(GREEN "Notebook 1" RESET "\n");
    parse_and_dump("notebook1.nb");
    printf("\n");
    /* Notebook 2 */
    printf(GREEN "Notebook 2" RESET "\n");
    parse_and_dump("notebook2.nb");
    printf("\n");
    return 0;
}
