#ifndef PIPES_H
#define PIPES_H
/**
 * \file
 * TODO
 */
#include <stdlib.h>

/**
 * \brief A list of pipes
 */
typedef struct _pipes* Pipes;

/**
 * \brief Creates a list of pipes.
 *
 * \param size Initial size of the list
 */
Pipes pipes_create(size_t size);

/**
 * \brief Appends a new pipe to the list
 *
 * \param p The list
 */
int pipes_append(Pipes p);

/**
 * \brief Get the pipe stored at the given index
 *
 * \param p The list
 * \param idx The index
 */
int* pipes_index(Pipes p, size_t idx);

/**
 * \brief Get the size of the list
 *
 * \param p The list
 */
size_t pipes_len(Pipes p);

/**
 * \brief Get the last pipe in the list
 *
 * \param p List
 */
int* pipes_last(Pipes p);

/**
 * \brief Close both sides of the pipe at the given index
 *
 * \param p The list
 * \param i The index
 */
void pipes_close(Pipes p, size_t i);

/**
 * \brief Free the memory used by this list
 *
 * \param p List
 */
void pipes_free(Pipes p);

#endif
