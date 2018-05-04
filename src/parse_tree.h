#ifndef PARSE_TREE_H
#define PARSE_TREE_H

#include "strings.h"

typedef struct _parse_tree * ParseTree;

ParseTree parse_tree_create(int size);

int parse_tree_add_line(ParseTree pt, char* line, size_t length);

void parse_tree_destroy(ParseTree pt);

char** parse_tree_dump(ParseTree pt);

void parse_tree_print(ParseTree pt);

#endif /* PARSE_TREE_H */
