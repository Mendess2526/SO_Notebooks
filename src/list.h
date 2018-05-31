#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <unistd.h>

/**
* A list of indexes
*/
typedef struct _list* IdxList;

/**
 * Creates an index list.
 *
 * \param size The initial size of the list
 */
IdxList idx_list_create(size_t size);

/**
 * Appends an element to the list
 *
 * \param l The list
 * \param val The value to append
 */
void idx_list_append(IdxList l, size_t val);

/**
 * Get the element at the given index
 *
 * \param l The list
 * \param idx The index
 */
ssize_t idx_list_index(IdxList l, size_t idx);

/**
 * Get the last element of the list.
 *
 * \param l The list
 */
ssize_t idx_list_last(IdxList l);

/**
 * Get the size of the list
 *
 * \param l The list
 */
size_t idx_list_len(IdxList l);

/**
 * Get the index of the first ocurrence of an element
 *
 * \param l The list
 * \param idx The value to search
 */
ssize_t idx_list_find(IdxList l, size_t idx);

/**
 * Free the memory used by this list
 *
 * \param l The list
 */
void idx_list_free(IdxList l);

/**
* A list of pointers
*/
typedef struct _ptr_list* PtrList;

/**
 * Creates a list of pointers.
 *
 * \param size The initial size of the list
 */
PtrList ptr_list_create(size_t size);

/**
 * Appends the element to the list
 *
 * \param l The list
 * \param val The value to append
 */
void ptr_list_append(PtrList l, void* val);

/**
 * Get the pointer stored at the given index
 *
 * \param l The list
 * \param idx The index
 */
void* ptr_list_index(PtrList l, size_t idx);

/**
 * Get the size of the list
 *
 * \param l The list
 */
size_t ptr_list_len(PtrList l);

/**
 * Free the memory used by this list
 *
 * \param l The list
 */
void ptr_list_free(PtrList l);

#endif /* LIST_H */
