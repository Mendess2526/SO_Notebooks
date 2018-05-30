#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <unistd.h>

/** 
* Index list 
*/
typedef struct _list* IdxList;

/**
 * Creates an index list.
 *
 * \param size Length of list
 */
IdxList idx_list_create(size_t size);

/**
 * Appends element to the list
 *
 * \param l List
 * \param val Value to insert
 */
void idx_list_append(IdxList l, size_t val);

/**
 * Get element from the list at the given index
 *
 * \param l List
 * \param idx Index to search
 */
ssize_t idx_list_index(IdxList l, size_t idx);

/**
 * Get last element from the list 
 *
 * \param l List
 */
ssize_t idx_list_last(IdxList l);

/**
 * Get size of the list 
 *
 * \param l List
 */
size_t idx_list_len(IdxList l);

/**
 * Get the index where an certain element is
 *
 * \param l List
 * \param idx Value to search
 */
ssize_t idx_list_find(IdxList l, size_t idx);

/**
 * Free list
 *
 * \param l List
 */
void idx_list_free(IdxList l);

/** 
* List of pointers 
*/
typedef struct _ptr_list* PtrList;

/**
 * Creates an index list.
 *
 * \param size Length of list
 */
PtrList ptr_list_create(size_t size);

/**
 * Appends element to the list
 *
 * \param l List
 * \param val Value to insert
 */
void ptr_list_append(PtrList l, void* val);

/**
 * Get element from the list at the given index
 *
 * \param l List
 * \param idx Index to search
 */
void* ptr_list_index(PtrList l, size_t idx);

/**
 * Get size of the list 
 *
 * \param l List
 */
size_t ptr_list_len(PtrList l);

/**
 * Free list
 *
 * \param l List
 */
void ptr_list_free(PtrList l);

#endif /* LIST_H */
