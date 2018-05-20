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

void nuke(int i){
    (void) i;
    kill(0, SIGKILL);
}

static ParseTree parse_and_exec(int fd, Pipes pipes, IdxList pids);

static void read_from_pipes_write_batch(Command cmd, int* pp);

void tree_to_file(ParseTree pt, char* filename);

int main(int argc, char** argv){
    signal(SIGINT, nuke);
    if(argc < 2){
        char message[] = "Usage: ./program <notebook.nb>\n";
        write(1, message, strlen(message));
        return 1;
    }
    //TODO maybe implement read from stdin '-'
    // Open file to read from
    int fd = open(argv[1], O_RDONLY, 0644);

    // Parse file and execute batches
    Pipes pipes = pipes_create(20);
    IdxList pids = idx_list_create(20);
    ParseTree pt = parse_and_exec(fd, pipes, pids);
    close(fd);

    // Read outputs and write store them in the batches
    pid_t pid;
    while((pid = wait(NULL)) > 0){
        ssize_t i = idx_list_find(pids, (size_t) pid);
        if(i < 0) LOG_WARNING("Missing pid\n");
        else
            read_from_pipes_write_batch(parse_tree_get_batch(pt, (size_t) i),
                                        pipes_index(pipes, (size_t) i));
    }
    pipes_free(pipes);

    // Write the tree to a file
    tree_to_file(pt, argv[1]);
    parse_tree_destroy(pt);
    return 0;
}

ParseTree parse_and_exec(int fd, Pipes pipes, IdxList pids){
    ParseTree pt = parse_tree_create(20);
    char* buff = NULL;
    size_t len;
    do{
        buff = readLn(fd, &len);
        ssize_t batch = parse_tree_add_line(pt, buff, len);
        if(batch != -1){
            pipes_append(pipes);
            pid_t pid = execBatch(parse_tree_get_batch(pt, (size_t) batch),
                                  pipes_last(pipes));
            close(pipes_last(pipes)[1]);
            if(pid > 0)
                idx_list_append(pids, (size_t) pid);
            else
                LOG_WARNING("Couldn't fork\n");
        }
    }while(NULL != buff);
    return pt;
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
        size_t len = strlen(buf + offset); // strlen stops at '\0'
        if(n < 1){
            LOG_WARNING("strlen failed read less then 1 byte: Buf + offset: ");
            LOG_CRITICAL(buf + offset);
        }
        String output;
        string_init(&output, buf + offset, len);
        command_append_output(cmd, output);
        string_free(output);
        cmd = command_pipe(cmd);
        offset += len + 1;
    }
}

void tree_to_file(ParseTree pt, char* filename){
    int fd = creat(filename, 0644);
    char** dump = parse_tree_dump(pt);
    int i = 0;
    while(dump[i]){
        write(fd, dump[i], strlen(dump[i]));
        write(fd, "\n", 1);
        free(dump[i++]);
    }
}
