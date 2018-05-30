#include "list.h"

/* lista de indices */
struct _list{
    size_t* array;
    size_t size;
    size_t load;
};

/*
    função que cria uma lista de indices
*/
IdxList idx_list_create(size_t size){
    if(size < 1) size = 20;
    IdxList l = (IdxList) malloc(sizeof(struct _list));
    l->size = size;
    l->load = 0;
    l->array = (size_t*) malloc(sizeof(size_t) * size);
    return l;
}

/*
    função que adiciona ao fim da lista um elemento
*/
void idx_list_append(IdxList l, size_t val){
    if(l->load >= l->size){
        l->size *= 2;
        l->array = realloc(l->array, sizeof(size_t) * l->size);
    }
    l->array[l->load++] = val;
}

/*
    função que retorna o elemento da lista num determinado indice
*/
ssize_t idx_list_index(IdxList l, size_t idx){
    if(idx > l->load) return -1;
    return l->array[idx];
}

/*
    função que retorna o tamanho da lista
*/
size_t idx_list_len(IdxList l){
    return l->load;
}

/*
    função que determina o ultimo elemento da lista
*/
ssize_t idx_list_last(IdxList l){
    if(l->load == 0) return -1;
    return l->array[l->load - 1];
}

/*
    função que determina o o indice onde se encontra um determinado elemento
*/
ssize_t idx_list_find(IdxList l, size_t idx){
    size_t i;
    for(i = 0; i < l->load; i++) if(l->array[i] == idx) break;
    if(i == l->load) return -1;
    return i;
}

/*
    função que liberta a memoria ocupada pela lista
*/
void idx_list_free(IdxList l){
    free(l->array);
    free(l);
}

/* lista de apontadores */
struct _ptr_list{
    void** array;
    size_t size;
    size_t load;
};

/*
    função que cria uma lista de apontadores
*/
PtrList ptr_list_create(size_t size){
    if(size < 1) size = 20;
    PtrList l = (PtrList) malloc(sizeof(struct _ptr_list));
    l->size = size;
    l->load = 0;
    l->array = malloc(sizeof(void*) * size);
    return l;
}

/*
    função que adiciona ao fim da lista um elemento
*/
void ptr_list_append(PtrList l, void* val){
    if(l->load >= l->size){
        l->size *= 2;
        l->array = realloc(l->array, sizeof(void*) * l->size);
    }
    l->array[l->load++] = val;
}

/*
    função que retorna o elemento da lista num determinado indice
*/
void* ptr_list_index(PtrList l, size_t idx){
    if(idx > l->load) return NULL;
    return l->array[idx];
}

/*
    função que retorna o tamanho da lista
*/
size_t ptr_list_len(PtrList l){
    return l->load;
}

/*
    função que liberta a memoria ocupada pela lista
*/
void ptr_list_free(PtrList l){
    free(l->array);
    free(l);
}
