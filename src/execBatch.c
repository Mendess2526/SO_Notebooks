#include "execBatch.h"
#include "utilities.h"
#include "pipes.h"
#include "logger.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>


/**
 * Checks if a String contains a '|' character.
 *
 * \param s String to check.
 * \returns 1 if contains 0 otherwise.
 */
static int has_pipes(String s);

/**
 * Checks if a String contains a '$' character.
 *
 * \param s String to check
 * \returns 1 if contains 0 otherwise.
 */
static int has_paralel(String s);

/**
 * Executes a single command.
 * \param cmd The command to execute.
 * \param i The index of the pipe it's input and output is gotten from.
 * \param inPipes The pipes where the input comes from.
 * \param outPipes The pipes where the output is writen to.
 */
static void execCommand(String cmd,
                        size_t i,
                        Pipes inPipes,
                        Pipes outPipes);

/**
 * Executes a list of commands in paralel, distributing the input
 * and aggregating de output.
 * \param cmd The list of commands to execute
 * \param i The index of the pipe it's input and output is gotten from.
 * \param inPipes The pipes where the input comes from.
 * \param outPipes The pipes where the output is writen to.
 */
static void execCommandParalel(String cmd,
                                size_t i,
                                Pipes inPipes,
                                Pipes outPipes);

/**
 * Executes a line of piped commands
 * \param cmd The line of piped commands
 * \param i The index of the pipe it's input and output is gotten from.
 * \param inPipes The pipes where the input comes from.
 * \param outPipes The pipes where the output is writen to.
 */
static void execCommandPipes(String cmd,
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
static void closePipes(Command c, Pipes inPipes, size_t i);

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
        String cmd = command_get_command(cur);
        if(has_paralel(cmd))
            execCommandParalel(cmd, i, inPipes, outPipes);
        else if(has_pipes(cmd))
            execCommandPipes(cmd, i, inPipes, outPipes);
        else
            execCommand(cmd, i, inPipes, outPipes);
        if(i > 0) close(pipes_index(inPipes, i - 1)[0]);
        close(pipes_index(outPipes, i)[1]);
    }

    size_t cmdCount = pipes_len(outPipes);
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
        int status;
        wait(&status);
        if(!WIFEXITED(status) || WEXITSTATUS(status) != 0){
            LOG_FATAL("Command failed: ");
            LOG_FATAL_STRING(command_get_command(cur));
            LOG_FATAL("\n");
            _exit(1);
        }
        write(pipfd[1], "\0", 1); // Write the separated '\0'
        if(i < (cmdCount - 1))
            closePipes(cur, inPipes, i); // Close input pipes
    }
    pipes_free(inPipes);
    pipes_free(outPipes);
    _exit(0);
}

int has_pipes(String s){
    for(size_t i = 0; i < s.length; i++)
        if(s.s[i] == '|') return 1;
    return 0;
}

int has_paralel(String s){
    for(size_t i = 0; i < s.length; i++)
        if(s.s[i] == '&') return 1;
    return 0;
}

void execCommand(String cmd, size_t i, Pipes inPipes, Pipes outPipes){
    if(fork()) return;
    if(i > 0){ // Redirect input to pipe and close write side of pipe
        dup2(pipes_index(inPipes, i - 1)[0], 0);
        close(pipes_index(inPipes, i - 1)[0]);
        for(size_t j = i; j > 0; j--)
            close(pipes_index(inPipes, j - 1)[1]);
    }
    dup2(pipes_index(outPipes, i)[1], 1); // Redirect output to pipe
    close(pipes_index(outPipes, i)[1]);
    close(pipes_index(outPipes, i)[0]);
    char** command = words(cmd.s, cmd.length);
    execvp(command[0], command);
    _exit(1);
}

void execCommandParalel(String cmd, size_t i, Pipes inPipes, Pipes outPipes){
    // Close what I don't need
    if(fork()) return;
    if(i > 0){
        for(size_t j = i; j > 0; j--)
            close(pipes_index(inPipes, j - 1)[1]);
    }
    close(pipes_index(outPipes, i)[0]);
    // Create the inner pipes
    Pipes innerInPipes = pipes_create(2);
    Pipes innerOutPipes = pipes_create(2); pipes_append(innerOutPipes);
    pipes_close(innerOutPipes, 0);
    // Prepare the command string for tokenizing
    char* line = malloc(sizeof(char) * (cmd.length + 1));
    strncpy(line, cmd.s, cmd.length); line[cmd.length] = '\0';
    // Execute every token
    PtrList cmdsArray = ptr_list_create(4);
    for(char* c = strtok(line, "&"); c != NULL; c = strtok(NULL, "&")){
        ptr_list_append(cmdsArray, c);
    }
    ptr_list_append(cmdsArray, NULL);
    for(size_t j = 1; ptr_list_index(cmdsArray, j - 1) != NULL; j++){
        pipes_append(innerInPipes);
        pipes_append(innerOutPipes);
        String c;
        char* cmd = ptr_list_index(cmdsArray, j - 1);
        string_init(&c, cmd, strlen(cmd));
        if(has_pipes(c))
            execCommandPipes(c, j, innerInPipes, innerOutPipes);
        else
            execCommand(c, j, innerInPipes, innerOutPipes);
        close(pipes_index(innerInPipes, j - 1)[0]);
        close(pipes_index(innerOutPipes, j)[1]);
    }
    ssize_t n;
    char buf[1024];
    if(i > 0){
        dup2(pipes_index(inPipes, i - 1)[0], 0);
        close(pipes_index(inPipes, i - 1)[0]);
        while((n = read(0, buf, 1024)) > 0){
            for(size_t k = 0; k < pipes_len(innerInPipes); k++){
                write(pipes_index(innerInPipes, k)[1], buf, n);
            }
        }
        for(size_t k = 0; k < pipes_len(innerInPipes); k++)
            close(pipes_index(innerInPipes, k)[1]);
    }
    dup2(pipes_index(outPipes, i)[1], 1);
    close(pipes_index(outPipes, i)[1]);
    for(size_t k = 1; k < pipes_len(innerOutPipes); k++){
        while((n = read(pipes_index(innerOutPipes, k)[0], buf, 1024)) > 0){
            write(1, buf, n);
        }
        int status;
        wait(&status);
        if(!WIFEXITED(status) || WEXITSTATUS(status) != 0){
            LOG_FATAL("Paralel Command failed: ");
            LOG_FATAL(ptr_list_index(cmdsArray, k - 1));
            LOG_FATAL("\n");
            _exit(1);
        }
    }
    _exit(0);
}

void execCommandPipes(String cmd, size_t i, Pipes inPipes, Pipes outPipes){
    if(fork()) return;
    if(i > 0){
        dup2(pipes_index(inPipes, i - 1)[0], 0);
        close(pipes_index(inPipes, i - 1)[0]);
        for(size_t j = i; j > 0; j--)
            close(pipes_index(inPipes, j - 1)[1]);
    }
    char* cmds = malloc(sizeof(char) * (cmd.length + 1));
    strncpy(cmds, cmd.s, cmd.length);
    cmds[cmd.length] = '\0';
    int prePipe = -1;
    int posPipe[2];
    PtrList cmdsArray = ptr_list_create(4);
    for(char* c = strtok(cmds, "|"); c != NULL; c = strtok(NULL, "|")){
        ptr_list_append(cmdsArray, c);
    }
    for(size_t j = 0; j < ptr_list_len(cmdsArray); j++){
        pipe(posPipe);
        if(!fork()){
            char* c = ptr_list_index(cmdsArray, j);
            char** command = words(c, strlen(c));
            if(prePipe != -1) dup2(prePipe, 0);
            if(j >= ptr_list_len(cmdsArray) - 1){
                dup2(pipes_index(outPipes, i)[1], 1);
            }else{
                dup2(posPipe[1], 1);
            }
            close(pipes_index(outPipes, i)[1]);
            close(posPipe[1]);
            close(posPipe[0]);
            execvp(command[0], command);
            _exit(1);
        }
        close(posPipe[1]);
        if(prePipe != -1) close(prePipe);
        prePipe = posPipe[0];
    }
    _exit(0);
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
