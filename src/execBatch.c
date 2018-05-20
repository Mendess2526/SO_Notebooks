#include "execBatch.h"
#include "utilities.h"
#include "pipes.h"
#include "logger.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

/**
 * Executes a single command.
 * \param c The command to execute.
 * \param i The index of the pipe it's input and output is gotten from.
 * \param inPipes The pipes where the input comes from.
 * \param outPipes The pipes where the output is writen to.
 */
static void execCommand(Command c,
                               size_t i,
                               Pipes inPipes,
                               Pipes outPipes);
/**
 * Writes to the input pipes of all the dependencies of the given command.
 *
 * \param c The command with the dependencies.
 * \param inPipes The input pipes.
 * \param buf The buffer with with the output to write.
 * \param n The length of the pipe.
 * \param i The position of the command in the batch.
 */
static void writeToPipes(Command c,
                                Pipes inPipes,
                                char* buf,
                                size_t n,
                                size_t i);
/**
 * Closes the input pipes of all the dependencies of the given command.
 *
 * \param c The command with the dependencies.
 * \param inPipes The pipes to close.
 * \param i The position of the command in the batch.
 */
static void closePipes(Command c,
                              Pipes inPipes,
                              size_t i);

int execBatch(Command c, int* pipfd){
    int pid = fork();
    if(pid) return pid;
    close(pipfd[0]);

    Pipes inPipes = pipes_create(3);
    Pipes outPipes = pipes_create(3);

    Command cur = c;
    for(size_t i = 0; cur; cur = command_pipe(cur), i++){
        if(i > 0) pipes_append(inPipes);
        pipes_append(outPipes);

        execCommand(cur, i, inPipes, outPipes);
    }

    size_t cmdCount = pipes_len(outPipes);
    for(size_t i = 0; i < cmdCount; i++){
        close(pipes_index(outPipes, i)[1]);
    }
    cur = c;
    for(size_t i = 0; i < cmdCount && cur; i++, cur = command_pipe(cur)){
        ssize_t n;
        char buf[1024] = {0};
        // Read cmd output
        while((n = read(pipes_index(outPipes, i)[0], buf, 1024)) > 0){
            // Write to parent pipe for storage
            write(pipfd[1], buf, (size_t) n);
            if(i < (cmdCount - 1)){
                writeToPipes(cur, inPipes, buf, (size_t) n, i);
            }
        }
        wait(NULL);
        write(pipfd[1], "\0", 1); // Write the separated '\0'
        if(i < (cmdCount - 1))
            closePipes(cur, inPipes, i); // Close input pipes
    }
    pipes_free(inPipes);
    pipes_free(outPipes);
    _exit(0);
}

void execCommand(Command c, size_t i, Pipes inPipes, Pipes outPipes){
    if(!fork()){
        if(i > 0){ // Redirect input to pipe and close write side of pipe
            dup2(pipes_index(inPipes, i - 1)[0], 0);
            close(pipes_index(inPipes, i - 1)[0]);
            for(size_t j = i; j > 0; j--)
                close(pipes_index(inPipes, j - 1)[1]);
        }
        dup2(pipes_index(outPipes, i)[1], 1); // Redirect output to pipe
        for(ssize_t j = i; j >= 0; j--)
            close(pipes_index(outPipes, (size_t) j)[1]);

        String cmd = command_get_command(c);
        char** command = words(cmd.s, cmd.length);
        execvp(command[0], command);
        _exit(1);
    }
}

void writeToPipes(Command c, Pipes inPipes, char* buf, size_t n, size_t i){
    IdxList deps = command_get_dependants(c);
    // Foreach command that depends
    for(size_t dep = 0; dep < idx_list_len(deps); dep++){
        ssize_t idx = idx_list_index(deps, dep);
        if(idx < 0){
            LOG_CRITICAL("Can't find pipe for this index");
        }else{
            size_t offset = i + ((size_t) idx) - 1;
            // Write to dep cmd in chain
            write(pipes_index(inPipes, offset)[1], buf, n);
        }
    }
}

void closePipes(Command c, Pipes inPipes, size_t i){
    IdxList deps = command_get_dependants(c);
    // Foreach command that depends
    for(size_t dep = 0; dep < idx_list_len(deps); dep++){
        ssize_t idx = idx_list_index(deps, dep);
        if(idx < 0){
            LOG_CRITICAL("Can't find pipe for this index");
        }else{
            size_t offset = i + ((size_t) idx) - 1;
            // Close to dep cmd in chain
            close(pipes_index(inPipes, offset)[1]);
        }
    }
}
