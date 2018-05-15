#include "list.h"

struct _list{
    size_t* array;
    size_t size;
    size_t load;
};

IdxList idx_list_create(size_t size){
    if(size < 1) size = 20;
    IdxList l = (IdxList) malloc(sizeof(struct _list));
    l->size = size;
    l->load = 0;
    l->array = (size_t*) malloc(sizeof(size_t) * size);
    return l;
}

void idx_list_append(IdxList l, size_t val){
    if(l->load >= l->size){
        l->size *= 2;
        l->array = realloc(l->array, sizeof(size_t) * l->size);
    }
    l->array[l->load++] = val;
}

ssize_t idx_list_index(IdxList l, size_t idx){
    if(idx > l->load) return -1;
    return l->array[idx];
}

size_t idx_list_len(IdxList l){
    return l->load;
}

ssize_t idx_list_last(IdxList l){
    if(l->load == 0) return -1;
    return l->array[l->load - 1];
}

void idx_list_free(IdxList l){
    free(l->array);
    free(l);
}

struct _ptr_list{
    void** array;
    size_t size;
    size_t load;
};

PtrList ptr_list_create(size_t size){
    if(size < 1) size = 20;
    PtrList l = (PtrList) malloc(sizeof(struct _ptr_list));
    l->size = size;
    l->load = 0;
    l->array = malloc(sizeof(void*) * size);
    return l;
}

void ptr_list_append(PtrList l, void* val){
    if(l->load >= l->size){
        l->size *= 2;
        l->array = realloc(l->array, sizeof(void*) * l->size);
    }
    l->array[l->load++] = val;
}

void* ptr_list_index(PtrList l, size_t idx){
    if(idx > l->load) return NULL;
    return l->array[idx];
}

size_t ptr_list_len(PtrList l){
    return l->load;
}

void* ptr_list_last(PtrList l){
    if(l->load == 0) return NULL;
    return l->array[l->load - 1];
}

void ptr_list_free(PtrList l){
    free(l->array);
    free(l);
}
