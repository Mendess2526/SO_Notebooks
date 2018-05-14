#include "execBatch.h"
#include "utilities.h"
#include "pipes.h"

#include <unistd.h>
#include <sys/wait.h>

static inline void execCommand(Command c, int i, Pipes inPipes, Pipes outPipes);

void execBatch(Command c, int* pipfd){
    if(fork()) return;
    close(pipfd[0]);
    int chainedNum = command_get_chained_num(c);

    Pipes inPipes = pipes_create((chainedNum - 1));
    Pipes outPipes = pipes_create(chainedNum);

    Command cur = c;
    for(int i = 0; cur; cur = command_pipe(cur), i++){
        if(i > 0) pipes_append(inPipes);
        pipes_append(outPipes);

        execCommand(cur, i, inPipes, outPipes);
    }
    cur = c;
    for(int i = 0; i < chainedNum && cur; i++, cur = command_pipe(cur)){
        int n;
        char buf[1024] = {0};
        while((n = read(pipes_index(outPipes, i)[0], buf, 1024)) > 0){ // Read cmd output
            write(pipfd[1], buf, n); // Write to parent pipe for storage
            if(i < (chainedNum - 1)){
                IdxList deps = command_get_dependants(cur);
                for(size_t dep = 0; dep < idx_list_len(deps); dep++){ // Foreach command that depends
                    int offset = i + idx_list_index(deps, dep) - 1;
                    write(pipes_index(inPipes, offset)[1], buf, n); // Write to dep cmd in chain
                }
            }
        }
        wait(NULL); //TODO figure out why this is needed
        write(pipfd[1], "\0", 1); // Write the separated '\0'
        if(i< (chainedNum -1)) close(pipes_index(inPipes, i)[1]); // Close input pipe
    }
    _exit(0);
}

static inline void execCommand(Command c, int i, Pipes inPipes, Pipes outPipes){
    if(!fork()){
        if(i > 0){ // Redirect input to pipe and close write side of pipe
            dup2(pipes_index(inPipes,i-1)[0], 0);
            close(pipes_index(inPipes,i-1)[0]);
            close(pipes_index(inPipes,i-1)[1]);
        }
        dup2(pipes_index(outPipes,i)[1], 1); // Redirect output to pipe
        close(pipes_index(outPipes,i)[1]);

        String cmd = command_get_command(c);
        char** command = words(cmd.s, cmd.length);
        execvp(command[0], command);
        _exit(1);
    }else{
        close(pipes_index(outPipes, i)[1]);
    }
}

