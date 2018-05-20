#include "parse_tree.h"
#include "utilities.h"
#include "pipes.h"
#include "execBatch.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

typedef void (*sighandler_t)(int);

int kill(pid_t pid, int sig);

void nuke(int i){
    (void) i;
    kill(0, SIGKILL);
}

static void read_from_pipes_write_batch(Command cmd, int* pp);

int main(int argc, char** argv){
    signal(SIGINT, nuke);
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
    IdxList pids = idx_list_create(20);
    while(NULL != (buff = readLn(fd, &len))){
        ssize_t batch = parse_tree_add_line(pt, buff, len);
        if(batch != -1){
            pipes_append(pipes);
            pid_t pid = execBatch(parse_tree_get_batch(pt, (size_t) batch),
                                  pipes_last(pipes));
            idx_list_append(pids, pid);
        }
    }
    close(fd);

    pid_t pid;
    int status;
    while((pid = wait(&status)) > 0){
        size_t i;
        for(i = 0; i < idx_list_len(pids); i++)
            if(idx_list_index(pids, i) == pid) break;

        read_from_pipes_write_batch(parse_tree_get_batch(pt, (size_t) i),
                                    pipes_index(pipes, i));
    }

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

void read_from_pipes_write_batch(Command cmd, Pipes inPipes, size_t i, size_t n){
    char buf[512] = "";
    char c;
    while(read(pipes_index(inPipes, i)[0], &c , 1)>0){
        buf[n++]=c;
        if(buf[n] == '\0'){
            String *buffer;
            string_init(buffer,buf,n);
            command_append_output(cmd, *buffer);
            string_free(*buffer);
        }
    }
}
