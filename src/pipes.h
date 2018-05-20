#ifndef PIPES_H
#define PIPES_H

typedef struct _pipes* Pipes;

Pipes pipes_create(size_t size);

void pipes_append(Pipes p);

int* pipes_index(Pipes p, size_t idx);

size_t pipes_len(Pipes p);

int* pipes_last(Pipes p);

void pipes_free(Pipes p);

void read_from_pipes_write_batch(Command cmd, Pipes inPipes);

#endif
