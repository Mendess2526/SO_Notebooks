#include "parse_tree.h"
#include "utilities.h"
#include "pipes.h"
#include "execBatch.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc, char** argv){
    if(argc < 2){
        char message[] = "Usage: ./program <notebook.nb>\n";
        write(1, message, strlen(message));
        return 1;
    }
    //TODO maybe implement read from stdin '-'
    int fd = open(argv[1], O_RDONLY, 0644);
    char* buff;
    size_t len;
    Pipes pipes = pipes_create(20);
    ParseTree pt = parse_tree_create(20);
    while(NULL != (buff = readLn(fd, &len))){
        ssize_t batch = parse_tree_add_line(pt, buff, len);
        if(batch != -1){
            pipes_append(pipes);
            execBatch(parse_tree_get_batch(pt, (size_t) batch), pipes_last(pipes));
        }
    }
    close(fd);

    //TODO ler os pipes e preencher os batches correspondentes

    pipes_free(pipes);
    fd = creat(argv[1], 0644);
    char** dump = parse_tree_dump(pt);
    int i = 0;
    while(dump[i]){
        write(fd, dump[i], strlen(dump[i]));
        write(fd, "\n", 1);
        free(dump[i++]);
    }
    return 0;
}
