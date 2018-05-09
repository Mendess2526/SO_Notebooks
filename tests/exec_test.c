#include "execBatch.h"
#include "parse_tree.h"
#include "colors.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int main(){
    ParseTree pt = parse_tree_create(3);
    parse_tree_add_line(pt, "$ ls", 4);
    parse_tree_add_line(pt, "$| sort", 7);
    parse_tree_print(pt);
    int pp[2];
    pipe(pp);
    execBatch(parse_tree_get_batch(pt, 0), pp);
    close(pp[1]);
    char c;
    int n;
    while(0 < (n = read(pp[0], &c, 1))){
        if(c == '\0')
            printf(RED "Fuck you" RESET "\n");
        write(1, &c, n);
    }

    return 0;
}
