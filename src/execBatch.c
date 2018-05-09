#include "execBatch.h"
#include "utilities.h"

#include <unistd.h>
#include <sys/wait.h>

void execBatch(Command c, int* pipfd){
    if(fork()) return;
    close(pipfd[0]);
    int chainedNum = command_get_chained_num(c);
    int** inPipes = (int**) malloc(sizeof(int*) * (chainedNum - 1));
    for(int i = 0; i < (chainedNum - 1); i++){
        inPipes[i] = malloc(sizeof(int) * 2);
        pipe(inPipes[i]);
    }
    int** outPipes = (int**) malloc(sizeof(int*) * chainedNum);
    for(int i = 0; i < chainedNum; i++){
        outPipes[i] = malloc(sizeof(int) * 2);
        pipe(outPipes[i]);
    }
    for(int i = 0; c; c = command_pipe(c), i++){
        if(!fork()){
            if(i > 0){ // Redirect input to pipe and close read side of pipe
                dup2(inPipes[i-1][0], 0);
                close(inPipes[i-1][0]);
                close(inPipes[i-1][1]);
            }
            dup2(outPipes[i][1], 1); // Redirect output to pipe
            close(outPipes[i][1]);

            String cmd = command_get_command(c);
            char** command = words(cmd.s, cmd.length);
            execvp(command[0], command);
            _exit(1);
        }else{
            close(outPipes[i][1]);
        }
    }
    for(int i = 0; i < chainedNum; i++){
        int n;
        char buf[1024] = {0};
        while((n = read(outPipes[i][0], buf, 1024)) > 0){ // Read cmd output
            write(pipfd[1], buf, n); // Write to parent pipe for storage
            if(i < (chainedNum - 1))
                write(inPipes[i][1], buf, n); // Write to next cmd in chain
        }
        wait(NULL); //TODO figure out why this is needed
        write(pipfd[1], "\0", 1); // Write the terminating '\0'
        if(i< (chainedNum -1)) close(inPipes[i][1]); // Close input pipe
    }
    _exit(0);
}
