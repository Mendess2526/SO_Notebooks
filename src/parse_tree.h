#ifndef PARSE_TREE_H
#define PARSE_TREE_H

#include "strings.h"

typedef struct _parse_tree * ParseTree;

ParseTree parse_tree_create(int size);

void parse_tree_add_comment(ParseTree pt, String comment);

void parse_tree_add_command(ParseTree pt, String command, String output);

void parse_tree_destroy(ParseTree pt);

char** parse_tree_dump(ParseTree pt);

#endif /* PARSE_TREE_H */
