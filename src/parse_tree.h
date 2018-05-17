#ifndef PARSE_TREE_H
#define PARSE_TREE_H

#include "strings.h"
#include "list.h"

typedef struct _parse_tree* ParseTree;

typedef struct _command* Command;

/**
 * Creates a new instance of the parse tree.
 *
 * \param size The starting number of lines the tree holds
 */
ParseTree parse_tree_create(size_t size);

/**
 * \brief Parses and stores a line.
 *
 * Passing NULL in the line parameter or 0 in the length parameter simply
 * returns the number of the last batch.
 *
 * \param pt The parse_tree instance
 * \param line The line to parse.
 * \param length The length of the line.
 * \returns -1 unless a batch was finished in which case this returns the
 *          number of the finished batch.
 */
ssize_t parse_tree_add_line(ParseTree pt, char* line, size_t length);

/**
 * \brief Returns a batch of commands.
 *
 * A batch is a linked list of commands that depend on each other.
 *
 * \param pt The parse_tree instance.
 * \param batch The number of the batch.
 * \returns the batch.
 */
Command parse_tree_get_batch(ParseTree pt, size_t batch);

/**
 * Frees alocated memory used by the parse_tree.
 *
 * \param pt The parse_tree instance.
 */
void parse_tree_destroy(ParseTree pt);

/**
 * \brief Dumps the parse_tree as a NULL terminted array of NULL terminated strings.
 *
 * Each string is a line in the file that this tree represents.
 *
 * \param pt The parse_tree instance.
 * \returns The array of strings.
 */
char** parse_tree_dump(ParseTree pt);

/**
 * Prints the parse_tree to the screen. Used for debug purposes.
 *
 * \param pt The parse_tree instance.
 */
void parse_tree_print(ParseTree pt);

/**
 * Returns the command String in this command instance.
 *
 * \param c The command string.
 * \returns The string with the command.
 */
String command_get_command(Command c);

/**
 * Counts the length of the batch.
 *
 * \param c The command to count from.
 * \returns The length of the batch.
 */
size_t command_get_chained_num(Command c);

/**
 * \brief Returns the relative position of the command this command depends on.
 *
 * \param c The command
 * \returns The relative position of the command the given command depends on
 *          or 0 if the command does not depend on any others.
 */
size_t command_get_dependency(Command c);

/**
 * \brief Appends output to a command.
 *
 * \param c The command instance.
 * \param s The output to append.
 */
void command_append_output(Command c, String s);

/**
 * Returns the list of relative indexes of the commands that depend of
 * the given command.
 *
 * \param c The command instance.
 * \returns The list of relative indexes of the commands that depend of
 *          the given command.
 */
IdxList command_get_dependants(Command c);

/**
 * Returns the next command in the batch.
 *
 * \param c The command instance.
 * \returns The next command instance.
 */
Command command_pipe(Command c);

#endif /* PARSE_TREE_H */
