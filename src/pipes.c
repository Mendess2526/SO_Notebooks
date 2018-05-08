#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

struct _pipes{
    int** pipes;
    int size;
    int load;
};

pipess pipes_create(int size){
    if(size < 1) size = 20;
    pipess p = (Pipes) malloc(sizeof(struct _pipes));
    p->size = size;
    p->load = 0;
    p->pipes = (int**) malloc(sizeof(int*) * size);
    return l;
}

void pipes_append(Pipes p){
	
    if(p->load >= p->size){
        p->size *= 2;
        p->pipes = realloc(p->pipes, sizeof(int*) * p->size);
    }
    p->pipes[p->load++] = (int *) malloc(sizeof(int) * 2);
}

int* pipes_index(Pipes p, int idx){
    return p->pipes[idx];
}

int* pipes_last(Pipes p){
    if(p->load == 0) return -1;
    return p->pipes[p->load - 1];
}

void pipes_free(Pipes p){
    free(p->pipes);
    free(p);
}
