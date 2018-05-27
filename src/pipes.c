#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "parse_tree.h"
#include "strings.h"

typedef struct _pipes* Pipes;

struct _pipes{
    int** pipes;
    size_t size;
    size_t load;
};

Pipes pipes_create(size_t size){
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
    p->pipes[p->load] = (int*) malloc(sizeof(int) * 2);
    pipe(p->pipes[p->load++]);
}

int* pipes_index(Pipes p, size_t idx){
    return p->pipes[idx];
}

size_t pipes_len(Pipes p){
    return p->load;
}

int* pipes_last(Pipes p){
    if(p->load == 0) return NULL;
    return p->pipes[p->load - 1];
}

void pipes_close(Pipes p, size_t i){
    if(p->load > i){
        close(p->pipes[i][0]);
        close(p->pipes[i][1]);
    }
}

void pipes_free(Pipes p){
    for(size_t i = 0; i < p->load; i++)
        free(p->pipes[i]);
    free(p->pipes);
    free(p);
}
