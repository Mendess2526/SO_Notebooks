#include "list.h"

#include <stdlib.h>

struct _list{
    int* array;
    int size;
    int load;
};

IntList intList_create(int size){
    if(size < 1) size = 20;
    IntList l = (IntList) malloc(sizeof(struct _list));
    l->size = size;
    l->load = 0;
    l->array = (int*) malloc(sizeof(int) * size);
    return l;
}

void intList_append(IntList l, int val){
    if(l->load >= l->size){
        l->size *= 2;
        l->array = realloc(l->array, sizeof(int) * l->size);
    }
    l->array[l->load++] = val;
}

int intList_index(IntList l, int idx){
    return l->array[idx];
}

int intList_len(IntList l){
    return l->load;
}

int intList_last(IntList l){
    if(l->load == 0) return -1;
    return l->array[l->load - 1];
}

void intList_free(IntList l){
    free(l->array);
    free(l);
}
