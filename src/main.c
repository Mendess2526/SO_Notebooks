#include "parse_tree.h"
#include "utilities.h"
#include "pipes.h"
#include "execBatch.h"
#include "logger.h"
#include "colors.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

void nuke(int i){
    (void) i;
    //kill(0, SIGKILL);
}

static int handleFlags(int argc, char** argv);

static ParseTree parse_and_exec(int fd, Pipes pipes, IdxList pids);

static void read_from_pipes_write_batch(Command cmd, int* pp);

void tree_to_file(ParseTree pt, char* filename);

char* FILE_NAME;
short O_STDOUT;
short I_STDIN;
short SEQUENTIAL;

int main(int argc, char** argv){
//    signal(SIGINT, nuke);
    if(argc < 2){
        char message[] = "Usage: ./program [OPTIONS] notebook.nb\n\n"
                         "use ./program -h for help\n";
        write(1, message, strlen(message));
        return 1;
    }
    int mode = handleFlags(argc, argv);
    if(mode) return mode;
    // Open file to read from
    int fd;
    if(I_STDIN)
       fd = 0;
    else
       fd = open(FILE_NAME, O_RDONLY, 0644);

    // Parse file and execute batches
    Pipes pipes = pipes_create(20);
    IdxList pids = idx_list_create(20);
    ParseTree pt = parse_and_exec(fd, pipes, pids);

    // Read outputs and write store them in the batches
    pipes_free(pipes);
    idx_list_free(pids);
    // Write the tree to a file
    tree_to_file(pt, FILE_NAME);
    parse_tree_destroy(pt);
    return 0;
}

void printHelp(char* arg){
    if(arg){
        char message[] = "Invalid argument: ";
        write(2, message, strlen(message));
        write(2, arg, strlen(arg));
        write(2, "\n", 1);
    }else{
        char message[] = "./program [OPTIONS] file\n\n"
                         "If file is '-' will read from "UNDERLINE"stdin"RESET
                         " and write to "UNDERLINE"stdout"RESET"\n"
                         BOLD "OPTIONS:\n" RESET
                         "\t-o\tOutput to "UNDERLINE"stdout"RESET
                                " instead of "UNDERLINE"file"RESET"\n"
                         "\t-s\tExecute batches sequentialy\n"
                         "\t-h\tDisplay this message\n";
        write(1, message, strlen(message));
    }
}

int handleFlags(int argc, char** argv){
    for(int i = 1; i < argc; i++){
        if(argv[i][0] == '-'){
            switch(argv[i][1]){
                case '\0': I_STDIN = 1; O_STDOUT = 1; break;
                case 'o': O_STDOUT = 1; break;
                case 'h': printHelp(NULL); return 1;
                case 's': SEQUENTIAL = 1; break;
                default: printHelp(argv[i]); return 2;
            }
        }else{
            FILE_NAME = argv[i];
        }
    }
    return 0;
}

void waitForBatch(ParseTree pt, Pipes pipes, IdxList pids){
    pid_t pid;
    int status;
    while((pid = wait(&status)) > 0){
        if(!WIFEXITED(status) || WEXITSTATUS(status) != 0){
            LOG_FATAL("Batch failed\n");
            _exit(1);
        }
        ssize_t i = idx_list_find(pids, (size_t) pid);
        if(i < 0) LOG_WARNING("Missing pid\n");
        else
            read_from_pipes_write_batch(parse_tree_get_batch(pt, (size_t) i),
                                        pipes_index(pipes, (size_t) i));
    }
}

ParseTree parse_and_exec(int fd, Pipes pipes, IdxList pids){
    ParseTree pt = parse_tree_create(20);
    char* buff = NULL;
    size_t len;
    size_t batchCount = 0;
    do{
        buff = readLn(fd, &len);
        ssize_t batch = parse_tree_add_line(pt, buff, len);
        if(batch != -1){
            batchCount++;
        }
        free(buff);
    }while(NULL != buff);
    if(!I_STDIN) close(fd);
    for(size_t i = 0; i < batchCount; i++){
        pipes_append(pipes);
        pid_t pid = execBatch(parse_tree_get_batch(pt, i),
                              pipes_last(pipes));
        close(pipes_last(pipes)[1]);
        if(pid > 0)
            idx_list_append(pids, (size_t) pid);
        else
            LOG_WARNING("Couldn't fork\n");
        if(SEQUENTIAL) waitForBatch(pt, pipes, pids);
    }
    if(!SEQUENTIAL) waitForBatch(pt, pipes, pids);
    return pt;
}

void read_from_pipes_write_batch(Command cmd, int* pp){
    size_t size = 1024;
    size_t load = 0;
    char* buf = malloc(sizeof(char) * size);
    ssize_t n;
    while((n = read(pp[0], buf + load, size - load)) > 0){
        load += n;
        if(load >= size){
           size *= 2;
           buf = realloc(buf, sizeof(char) * size);
        }
    }
    size_t offset = 0;
    while(cmd){
        size_t len = strlen(buf + offset); // strlen stops at '\0'
        if(len < 1) break;
        String output;
        string_init(&output, buf + offset, len);
        command_append_output(cmd, output);
        string_free(output);
        cmd = command_pipe(cmd);
        offset += len + 1;
    }
    free(buf);
}

void pick_and_write_color(char* line){
    switch(line[0]){
        case '$': write(1, YELLOW, strlen(YELLOW)); break;
        case '>':
        case '<': write(1, RED, strlen(RED)); break;
    }
}

void tree_to_file(ParseTree pt, char* filename){
    int fd;
    if(O_STDOUT)
        fd = 1;
    else
        fd = creat(filename, 0644);
    char** dump = parse_tree_dump(pt);
    int i = 0;
    while(dump[i]){
        write(fd, dump[i], strlen(dump[i]));
        write(fd, "\n", 1);
        free(dump[i++]);
    }
    free(dump);
}
