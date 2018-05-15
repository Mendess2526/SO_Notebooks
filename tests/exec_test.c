#include "execBatch.h"
#include "parse_tree.h"
#include "colors.h"
#include "utilities.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

void parse_and_dump(char* file){
    int fd = open(file, O_RDONLY);
    ParseTree pt = parse_tree_create(1);
    char* line;
    size_t n;
    while((line = readLn(fd, &n)) != NULL){
        parse_tree_add_line(pt, line, n);
        free(line);
    }
    parse_tree_print(pt);
    close(fd);
    // TEST EXEC
    int pp[2];
    pipe(pp);
    execBatch(parse_tree_get_batch(pt, 0), pp);
    close(pp[1]);
    parse_tree_destroy(pt);
    // READ RESULT

    char c;
    ssize_t nn;
    while(0 < (nn = read(pp[0], &c, 1))){
        if(c == '\0')
            printf(RED "Fuck you" RESET "\n");
        write(1, &c, (size_t) nn);
    }
}

int main(int argc, char** argv){
    if(argc < 2){
        printf(GREEN "Notebook 3" RESET "\n");
        parse_and_dump("notebook3.nb");
        printf("\n");
        return 0;
    }
    for(int i = 1; i < argc; i++){
        printf(GREEN "%s" RESET "\n", argv[i]);
        parse_and_dump(argv[i]);
    }
    return 0;
}
