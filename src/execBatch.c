#include "execBatch.h"
#include "utilities.h"
#include "pipes.h"
#include "logger.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

static void execCommand(Command c,
                               size_t i,
                               Pipes inPipes,
                               Pipes outPipes);

static void writeToPipes(Command cur,
                                Pipes inPipes,
                                char* buf,
                                size_t n,
                                size_t i);

static void closePipes(Command cur,
                              Pipes inPipes,
                              size_t i);

void execBatch(Command c, int* pipfd){
    if(fork()) return;
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
        wait(NULL); //TODO figure out why this is needed
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
            close(pipes_index(outPipes, j)[1]);

        String cmd = command_get_command(c);
        char** command = words(cmd.s, cmd.length);
        execvp(command[0], command);
        _exit(1);
    }else{
        // close(pipes_index(outPipes, i)[1]);
    }
}

void writeToPipes(Command cur, Pipes inPipes, char* buf, size_t n, size_t i){
    IdxList deps = command_get_dependants(cur);
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

void closePipes(Command cur, Pipes inPipes, size_t i){
    IdxList deps = command_get_dependants(cur);
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