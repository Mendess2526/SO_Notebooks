#include "execBatch.h"
#include "utilities.h"
#include "pipes.h"
#include "logger.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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
static int has_parallel(String s);

/**
 * Removes redirections from a command.
 *
 * \param words The array of strings
 */
static void cleanCommand(char** words);

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
 * Executes a list of commands in parallel, distributing the input
 * and aggregating de output.
 * \param cmd The list of commands to execute
 * \param i The index of the pipe it's input and output is gotten from.
 * \param inPipes The pipes where the input comes from.
 * \param outPipes The pipes where the output is writen to.
 */
static void execCommandParallel(String cmd,
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
        if(has_parallel(cmd))
            execCommandParallel(cmd, i, inPipes, outPipes);
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
            n = write(pipfd[1], buf, (size_t) n);
            if(i < (cmdCount - 1)){
                writeToPipes(cur, inPipes, buf, (size_t) n, i);
            }
            if(n == -1) _exit(-1);
        }
        int status;
        wait(&status);
        if(!WIFEXITED(status)){
            LOG_FATAL("Command failed: ");
            LOG_FATAL_STRING(command_get_command(cur));
            LOG_FATAL("\n");
            _exit(1);
        }
        if(write(pipfd[1], "\0", 1) == -1) _exit(-1); // Write the separated '\0'
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

int has_parallel(String s){
    for(size_t i = 0; i < s.length; i++)
        if(s.s[i] == '&' && s.length > i + 1 && s.s[i+1] != '>') return 1;
    return 0;
}

typedef struct _redirects{
    char* std_out;
    short append_out;
    char* std_err;
    short append_err;
    char* std_in;
}Redirects;

static void command_parse_redirects(Redirects* redirects,
                                    const char* line,
                                    size_t len){
    char** tokens = words(line, len);
    size_t i = 0;
    while(tokens[i]){
        if(!strcmp(tokens[i], "<")){
            free(tokens[i++]);
            redirects->std_in = str_dup(tokens[i]);
        }else if(!strcmp(tokens[i], ">")){
            free(tokens[i++]);
            redirects->std_out = str_dup(tokens[i]);
            redirects->append_out = 0;
        }else if(!strcmp(tokens[i], ">>")){
            free(tokens[i++]);
            redirects->std_out = str_dup(tokens[i]);
            redirects->append_out = 1;
        }else if(!strcmp(tokens[i], "2>")){
            free(tokens[i++]);
            redirects->std_err = str_dup(tokens[i]);
            redirects->append_err = 0;
        }else if(!strcmp(tokens[i], "2>>")){
            free(tokens[i++]);
            redirects->std_err = str_dup(tokens[i]);
            redirects->append_out = 1;
        }else if(!strcmp(tokens[i], "&>")){
            free(tokens[i++]);
            redirects->std_out = str_dup(tokens[i]);
            redirects->std_err = str_dup(tokens[i]);
            redirects->append_out = redirects->append_err = 0;
        }else if(!strcmp(tokens[i], "&>>")){
            free(tokens[i++]);
            redirects->std_out = str_dup(tokens[i]);
            redirects->std_err = str_dup(tokens[i]);
            redirects->append_out = redirects->append_err = 1;
        }
        free(tokens[i++]);
    }
    free(tokens);
}

void cleanCommand(char** words){
    for(size_t i = 0; words[i]; i++){
        if(strstr(words[i], "<") != NULL || strstr(words[i], ">") != NULL){
            words[i] = NULL;
            return;
        }
    }
}

void execCommand(String cmd, size_t i, Pipes inPipes, Pipes outPipes){
    if(fork()) return;
    Redirects redirects = {0};
    command_parse_redirects(&redirects, cmd.s, cmd.length);
    if(redirects.std_in){
        int in = open(redirects.std_in, O_RDONLY);
        if(in < 0){
            LOG_WARNING_STRING(cmd);
            LOG_WARNING(": no such file or directory: ");
            LOG_WARNING(redirects.std_in);
            LOG_WARNING("\n");
            _exit(1);
        }
        dup2(in, 0);
        close(in);
    }
    if(i > 0){ // Redirect input to pipe and close write side of pipe
        if(!redirects.std_in) dup2(pipes_index(inPipes, i - 1)[0], 0);
        close(pipes_index(inPipes, i - 1)[0]);
        for(size_t j = i; j > 0; j--)
            close(pipes_index(inPipes, j - 1)[1]);
    }
    if(redirects.std_out){
        int flags = redirects.append_out
            ? O_WRONLY | O_APPEND | O_CREAT
            : O_WRONLY | O_TRUNC  | O_CREAT;
        int out = open(redirects.std_out, flags, 0664);
        dup2(out, 1);
        close(out);
    }else{
        dup2(pipes_index(outPipes, i)[1], 1); // Redirect output to pipe
    }
    close(pipes_index(outPipes, i)[1]);
    close(pipes_index(outPipes, i)[0]);
    if(redirects.std_err){
        int flags = redirects.append_err
            ? O_WRONLY | O_APPEND | O_CREAT
            : O_WRONLY | O_TRUNC  | O_CREAT;
        int err = open(redirects.std_err, flags, 0664);
        dup2(err, 2);
        close(err);
    }

    char** command = words(cmd.s, cmd.length);
    cleanCommand(command);
    execvp(command[0], command);
    kill(getpid(), SIGKILL);
    _exit(1);
}

void execCommandParallel(String cmd, size_t i, Pipes inPipes, Pipes outPipes){
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
    PtrList cmdArray = ptr_list_create(4);
    char* cur = line;
    char* c = strstr(line, "& ");
    while(c != NULL){
        *c = '\0';
        ptr_list_append(cmdArray, cur);
        cur = c + 2;
        c = strstr(cur, "& ");
    };
    ptr_list_append(cmdArray, cur);
    ptr_list_append(cmdArray, NULL);
    for(size_t j = 1; ptr_list_index(cmdArray, j - 1) != NULL; j++){
        pipes_append(innerInPipes);
        pipes_append(innerOutPipes);
        String s;
        c = ptr_list_index(cmdArray, j - 1);
        string_init(&s, c, strlen(c));
        if(has_pipes(s))
            execCommandPipes(s, j, innerInPipes, innerOutPipes);
        else
            execCommand(s, j, innerInPipes, innerOutPipes);
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
                if(write(pipes_index(innerInPipes, k)[1], buf, (size_t) n) < 0)
                    _exit(-1);
            }
        }
        for(size_t k = 0; k < pipes_len(innerInPipes); k++)
            close(pipes_index(innerInPipes, k)[1]);
    }
    dup2(pipes_index(outPipes, i)[1], 1);
    close(pipes_index(outPipes, i)[1]);
    for(size_t k = 1; k < pipes_len(innerOutPipes); k++){
        while((n = read(pipes_index(innerOutPipes, k)[0], buf, 1024)) > 0){
            if(write(1, buf, (size_t) n) == -1) _exit(-1);
        }
        int status;
        wait(&status);
        if(!WIFEXITED(status)){
            LOG_FATAL("Parallel Command failed: ");
            LOG_FATAL(ptr_list_index(cmdArray, k - 1));
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
        if(pipe(posPipe) == -1){
            LOG_FATAL("Pipe failed: ");
            LOG_FATAL(ptr_list_index(cmdsArray, j));
            _exit(-2);
        }
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
            kill(getpid(), SIGKILL);
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
            if(write(pipes_index(inPipes, offset)[1], buf, n) == -1)
                _exit(-1);
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
