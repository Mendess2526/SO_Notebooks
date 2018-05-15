#ifndef PIPES_H
#define PIPES_H

typedef struct _pipes* Pipes;

Pipes pipes_create(size_t size);

void pipes_append(Pipes p);

int* pipes_index(Pipes p, size_t idx);

int* pipes_last(Pipes p);

void pipes_free(Pipes p);

#endif
