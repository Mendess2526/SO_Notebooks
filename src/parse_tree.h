#ifndef PARSE_TREE_H
#define PARSE_TREE_H

#include "strings.h"

typedef struct _parse_tree * ParseTree;

typedef struct _command* Command;

ParseTree parse_tree_create(int size);

int parse_tree_add_line(ParseTree pt, char* line, size_t length);

Command parse_tree_get_batch(ParseTree pt, int batch);

void parse_tree_destroy(ParseTree pt);

char** parse_tree_dump(ParseTree pt);

void parse_tree_print(ParseTree pt);

String command_get_command(Command c);

int command_get_chained_num(Command c);

void command_append_output(Command c, String s);

Command command_pipe(Command c);

#endif /* PARSE_TREE_H */
