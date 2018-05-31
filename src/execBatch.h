#ifndef EXEC_BATCH_H
#define EXEC_BATCH_H
/**
 * \file
 * \brief Executes batches and pipes their output to the calling process
 */
#include "parse_tree.h"

/**
 * \brief Executes a batch of commands and writes to the given pipes their
 * outputs.
 *
 * \param c The batch of commands.
 * \param pipfd The pipe where the outputs will be writen to.
 * \returns The PID of the process that executes the batch.
 */
int execBatch(Command c, int* pipfd, int* piperr);

#endif /* EXEC_BATCH_H */
