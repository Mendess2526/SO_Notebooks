#ifndef LIST_H
#define LIST_H


typedef struct _list* IntList;

IntList intList_create(int size);

void intList_append(IntList l, int val);

int intList_index(IntList l, int idx);

int intList_last(IntList l);

void intList_free(IntList l);

#endif /* LIST_H */
