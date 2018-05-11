#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct _pipes* Pipes;

struct _pipes{
    int** pipes;
    int size;
    int load;
};

Pipes pipes_create(int size){
    if(size < 1) size = 20;
    Pipes p = (Pipes) malloc(sizeof(struct _pipes));
    p->size = size;
    p->load = 0;
    p->pipes = (int**) malloc(sizeof(int*) * size);
    return p;
}

void pipes_append(Pipes p){
	
    if(p->load >= p->size){
        p->size *= 2;
        p->pipes = realloc(p->pipes, sizeof(int*) * p->size);
    }
    p->pipes[p->load] = (int *) malloc(sizeof(int) * 2);
    pipe(p->pipes[p->load++]);

}

int* pipes_index(Pipes p, int idx){
    return p->pipes[idx];
}

int* pipes_last(Pipes p){
    if(p->load == 0) return NULL;
    return p->pipes[p->load - 1];
}

void pipes_free(Pipes p){
    free(p->pipes);
    free(p);
}
