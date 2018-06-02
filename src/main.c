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
#include <stdint.h>
#include <stdio.h>
/**
 * Parses the arguments passed to the program to set the option flags.
 * @param argc The number of arguments.
 * @param argv The arguments.
 * @return 0 if no wrong flags were passed, 1 if help was requested, 2 if
 *         an invalid flag was passed.
 */
static int handleFlags(int argc, char** argv);
/**
 * Prints the help screen.
 * @param arg The invalid argument or NULL if help was requested.
 */
static void printHelp(char* arg);
/**
 * Parse and execute the file.
 * @param fd The file descriptor.
 * @return The parse tree with the
 */
static ParseTree parse_exec_update(int fd);
/**
 * Parse the file.
 * @param fd The file descriptor
 * @param pt The parse tree to parse with.
 * @param batchCount The number of batches after parsing
 * @return 1 if no errors occured 0 otherwise.
 */
static int parse(int fd, ParseTree pt, size_t* batchCount);
/**
 * Executes all the commands in the parse tree.
 * @param bc The number of batches.
 * @param pps The output pipes.
 * @param errPps The erro output pipes.
 * @param pids The pids of the processes created.
 * @param pt The parse tree to execute from.
 * @return 1 if no errors ocurred, 0 otherwise.
 */
static int exec(size_t bc, Pipes pps, Pipes errPps, IdxList pids, ParseTree pt);
/**
 * Update all batches with the output read from the pipes.
 * @param pt The parse tree to update.
 * @param pipes The pipes to read from.
 * @param pids The pids of the processes that wrote to the pipes.
 * @param errPipes The pipes where the command errors were writen to.
 * @return 0 if no command failed during execution or 1 otherwise.
 */
static int update(ParseTree pt, Pipes pipes, IdxList pids, Pipes errPipes);
/**
 * Kill all the processes in the given list.
 * @param pids The list of processes.
 * @param batchNum The number of the batch that caused this function call
 * @return 1
 */
static int killChildren(IdxList pids, size_t batchNum);
/**
 * Fill a batch's output from a pipe.
 * @param cmd The head of the batch.
 * @param pp The pipe to read from.
 */
static void commandOutputFromPipe(Command cmd, int* pp);
/**
 * Write the parse tree to a file
 * @param pt The parse tree.
 * @param filename The file name.
 */
static void tree_to_file(ParseTree pt, char* filename);
PtrList FILE_NAMES;
short O_STDOUT;

short I_STDIN;

short SEQUENTIAL;

static void nuke(int i){
    (void) i;
    kill(0, SIGINT);
}

int main(int argc, char** argv){
    signal(SIGINT, nuke);
    if(argc < 2){
        char message[] = "Usage: ./program [OPTIONS] notebook.nb\n\n"
                         "use ./program -h for help\n";
        if(write(1, message, strlen(message)) == 0) return -1;
        return 1;
    }
    FILE_NAMES = ptr_list_create((size_t) argc);
    int mode = handleFlags(argc, argv);
    if(mode) return mode - 1;
    // Open file to read from
    int fd;
    int failCount = 0;
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
        ParseTree pt = parse_exec_update(fd);
        if(pt){
            // Write the tree to a file
            tree_to_file(pt, filename);
            parse_tree_destroy(pt);
        }else{
            failCount++;
        }
        if(I_STDIN) I_STDIN = 0;
        else i++;
    }
    return failCount;
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

int killChildren(IdxList pids, size_t batchNum){
    LOG_FATAL("Batch ");
    char n[13];
    size_t len = int2string((int) batchNum, n, 12);
    n[len] = '\0';
    LOG_FATAL(n);
    LOG_FATAL(" failed\n");
    for(size_t i = 0; i < idx_list_len(pids); i++){
        ssize_t pid = idx_list_index(pids, i);
        if(pid > 0 && ((size_t) pid) < SIZE_MAX) kill((__pid_t) pid, SIGINT);
    }
    while(wait(NULL) > 0);
    return 1;
}

ParseTree parse_exec_update(int fd){
    ParseTree pt = parse_tree_create(20);
    size_t batchCount;
    int noErrors;
    noErrors = parse(fd, pt, &batchCount);
    if(!I_STDIN) close(fd);
    Pipes pipes = pipes_create(20);
    Pipes errPipes = pipes_create(20);
    IdxList pids = idx_list_create(20);
    if(noErrors){
        noErrors = exec(batchCount, pipes, errPipes, pids, pt);
    }
    signal(SIGINT, SIG_DFL);
    if(noErrors && !SEQUENTIAL && update(pt, pipes, pids, errPipes)){
        noErrors = 0;
    }
    if(!noErrors){
        parse_tree_destroy(pt);
        pt = NULL;
    }
    pipes_free(pipes);
    pipes_free(errPipes);
    idx_list_free(pids);
    return pt;
}

int parse(int fd, ParseTree pt, size_t* batchCount){
    char* buff = NULL;
    size_t len;
    size_t bc = 0;
    do{
        buff = readLn(fd, &len);
        ssize_t batch = parse_tree_add_line(pt, buff, len);
        if(batch == -2){
            readLn(-1, &len);
            *batchCount = bc;
            return 0;
        }else if(batch != -1){
            bc++;
        }
        free(buff);
    }while(NULL != buff);
    *batchCount = bc;
    return 1;
}

int exec(size_t bc, Pipes pps, Pipes errPps, IdxList pids, ParseTree pt){
    for(size_t i = 0; i < bc; i++){
        pipes_append(pps);
        pipes_append(errPps);
        pid_t pid = execBatch(parse_tree_get_batch(pt, i),
                pipes_last(pps), pipes_last(errPps));
        close(pipes_last(pps)[1]);
        close(pipes_last(errPps)[1]);
        if(pid > 0)
            idx_list_append(pids, (size_t) pid);
        else
            LOG_WARNING("Couldn't fork\n");
        if(SEQUENTIAL && update(pt, pps, pids, errPps)){
            return 0;
        }
    }
    return 1;
}

int update(ParseTree pt, Pipes pipes, IdxList pids, Pipes errPipes){
    pid_t pid;
    int status;
    while((pid = wait(&status)) > 0){
        ssize_t i = idx_list_find(pids, (size_t) pid);
        if(i < 0){
            LOG_WARNING("Missing pid\n");
        }else{
            idx_list_set(pids, (size_t) i, SIZE_MAX);
            char buf[1024];
            ssize_t n;
            int e = (!WIFEXITED(status) || (WEXITSTATUS(status) != 0));
            while((n = read(pipes_index(errPipes, (size_t) i)[0], buf, 1024)) > 0){
                e = 1;
                if(write(2, buf, (size_t) n) == -1) _exit(-1);
            }
            if(e) return killChildren(pids, (size_t) i);
            // Read outputs and write store them in the batches
            commandOutputFromPipe(parse_tree_get_batch(pt, (size_t) i),
                    pipes_index(pipes, (size_t) i));
        }
    }
    return 0;
}

void commandOutputFromPipe(Command cmd, int* pp){
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
