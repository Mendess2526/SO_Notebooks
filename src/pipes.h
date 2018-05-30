#ifndef PIPES_H
#define PIPES_H

#include <stdlib.h>

/** 
* List of pointers to pipes 
*/
typedef struct _pipes* Pipes;

/**
 * Creates a list of pipes.
 *
 * \param size Length of list
 */
Pipes pipes_create(size_t size);

/**
 * Appends element to the list
 *
 * \param p List
 */
int pipes_append(Pipes p);

/**
 * Get element from the list at the given index
 *
 * \param p List
 * \param idx Index to search
 */ 
int* pipes_index(Pipes p, size_t idx);

/**
 * Get size of the list 
 *
 * \param p List
 */
size_t pipes_len(Pipes p);

/**
 * Get last element from the list 
 *
 * \param p List
 */
int* pipes_last(Pipes p);

/**
 * Close file descriptors from pipe at the given index 
 *
 * \param p List
 * \param i index 
 */
void pipes_close(Pipes p, size_t i);

/**
 * Free list
 *
 * \param p List
 */
void pipes_free(Pipes p);

#endif
