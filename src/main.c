#include "parse_tree.h"
#include "utilities.h"
#include "pipes.h"
#include "execBatch.h"
#include "logger.h"

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
            close(pipes_last(pipes)[1]);
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

void read_from_pipes_write_batch(Command cmd, int* pp){
    size_t size = 1024;
    size_t load = 0;
    char* buf = malloc(sizeof(char) * size);
    ssize_t n;
    while((n = read(pp[0], buf + load, size)) > 0){
        load += n;
        if(load >= size){
           size *= 2;
           buf = realloc(buf, sizeof(char) * size);
        }
    }
    size_t offset = 0;
    while(cmd){
        n = strlen(buf + offset); // strlen stops at '\0'
        if(n < 1){
            LOG_WARNING("Strlen failed read less then 1 byte: Buf + offset: ");
            LOG_CRITICAL(buf + offset);
        }
        String output;
        string_init(&output, buf + offset, n);
        command_append_output(cmd, output);
        string_free(output);
        cmd = command_pipe(cmd);
        offset += n + 1;
    }
}
