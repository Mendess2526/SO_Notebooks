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
    kill(0, SIGINT);
    signal(SIGINT, nuke);
}

static int handleFlags(int argc, char** argv);

static ParseTree parse_and_exec(int fd, Pipes pipes, IdxList pids);

static void read_from_pipes_write_batch(Command cmd, int* pp);

void tree_to_file(ParseTree pt, char* filename);

PtrList FILE_NAMES;
short O_STDOUT;
short I_STDIN;
short SEQUENTIAL;

int main(int argc, char** argv){
    signal(SIGINT, nuke);
    if(argc < 2){
        char message[] = "Usage: ./program [OPTIONS] notebook.nb\n\n"
                         "use ./program -h for help\n";
        if(write(1, message, strlen(message)) == 0) return -1;
        return 1;
    }
    FILE_NAMES = ptr_list_create(argc);
    int mode = handleFlags(argc, argv);
    if(mode) return mode;
    // Open file to read from
    int fd;
    size_t numFiles = ptr_list_len(FILE_NAMES);
    size_t i = 0;
    while(i < numFiles){
        char* filename;
        if(I_STDIN){
            fd = 0;
            filename = NULL;
        }else{
            filename = ptr_list_index(FILE_NAMES, i);
            if(write(1, BLUE, strlen(BLUE)) == -1) _exit(-1);
            if(write(1, filename, strlen(filename)) == -1) _exit(-1);
            if(write(1, RESET "\n", strlen(RESET) + 1) == -1) _exit(-1);
            fd = open(filename, O_RDONLY, 0644);
        }

        // Parse file and execute batches
        Pipes pipes = pipes_create(20);
        IdxList pids = idx_list_create(20);
        ParseTree pt = parse_and_exec(fd, pipes, pids);
        if(pt){
            // Read outputs and write store them in the batches
            pipes_free(pipes);
            idx_list_free(pids);
            // Write the tree to a file
            tree_to_file(pt, filename);
            parse_tree_destroy(pt);
        }
        if(I_STDIN) I_STDIN = 0;
        else i++;
    }
    return 0;
}

void printHelp(char* arg){
    if(arg){
        char message[] = "Invalid argument: ";
        if(write(2, message, strlen(message)) == -1
            || write(2, arg, strlen(arg)) == -1
            || write(2, "\n", 1) == -1) _exit(-1);
    }else{
        char message[] = "./program [OPTIONS] file\n\n"
                         "If file is '-' will read from "UNDERLINE"stdin"RESET
                         " and write to "UNDERLINE"stdout"RESET"\n"
                         BOLD "OPTIONS:\n" RESET
                         "\t-o\tOutput to "UNDERLINE"stdout"RESET
                                " instead of "UNDERLINE"file"RESET"\n"
                         "\t-s\tExecute batches sequentially\n"
                         "\t-h\tDisplay this message\n";
        if(write(1, message, strlen(message)) == -1) _exit(-1);
    }
}

int handleFlags(int argc, char** argv){
    for(int i = 1; i < argc; i++){
        if(argv[i][0] == '-'){
            switch(argv[i][1]){
                case '\0': I_STDIN = 1; break;
                case 'o': O_STDOUT = 1; break;
                case 'h': printHelp(NULL); return 1;
                case 's': SEQUENTIAL = 1; break;
                default: printHelp(argv[i]); return 2;
            }
        }else{
            ptr_list_append(FILE_NAMES, argv[i]);
        }
    }
    return 0;
}

int waitForBatch(ParseTree pt, Pipes pipes, IdxList pids){
    pid_t pid;
    int status;
    while((pid = wait(&status)) > 0){
        if(!WIFEXITED(status) || WEXITSTATUS(status) != 0){
            for(size_t i = 0; i < idx_list_len(pids); i++){
                kill(idx_list_index(pids, i), SIGINT);
            }
            LOG_FATAL("Batch failed\n");
            return 1;
        }
        ssize_t i = idx_list_find(pids, (size_t) pid);
        if(i < 0) LOG_WARNING("Missing pid\n");
        else
            read_from_pipes_write_batch(parse_tree_get_batch(pt, (size_t) i),
                                        pipes_index(pipes, (size_t) i));
    }
    return 0;
}

ParseTree parse_and_exec(int fd, Pipes pipes, IdxList pids){
    ParseTree pt = parse_tree_create(20);
    char* buff = NULL;
    size_t len;
    size_t batchCount = 0;
    do{
        buff = readLn(fd, &len);
        ssize_t batch = parse_tree_add_line(pt, buff, len);
        if(batch == -2){
            parse_tree_destroy(pt);
            return NULL;
        }else if(batch != -1){
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
        if(SEQUENTIAL && waitForBatch(pt, pipes, pids)){
            parse_tree_destroy(pt);
            return NULL;
        }
    }
    if(!SEQUENTIAL && waitForBatch(pt, pipes, pids)){
        parse_tree_destroy(pt);
        return NULL;
    }
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

void tree_to_file(ParseTree pt, char* filename){
    int fd;
    if(O_STDOUT || filename == NULL)
        fd = 1;
    else
        fd = creat(filename, 0644);
    String dump = parse_tree_dump(pt);
    if(write(fd, dump.s, dump.length) == -1) _exit(1);
    string_free(dump);
}
