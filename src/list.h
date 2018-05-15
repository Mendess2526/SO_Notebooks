#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <unistd.h>

typedef struct _list* IdxList;

IdxList idx_list_create(size_t size);

void idx_list_append(IdxList l, size_t val);

ssize_t idx_list_index(IdxList l, size_t idx);

ssize_t idx_list_last(IdxList l);

size_t idx_list_len(IdxList l);

void idx_list_free(IdxList l);



typedef struct _ptr_list* PtrList;

PtrList ptr_list_create(size_t size);

void ptr_list_append(PtrList l, void* val);

void* ptr_list_index(PtrList l, size_t idx);

void* ptr_list_last(PtrList l);

size_t ptr_list_len(PtrList l);

void ptr_list_free(PtrList l);

#endif /* LIST_H */
