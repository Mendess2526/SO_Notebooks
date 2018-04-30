#include "parse_tree.h"

#include <stdlib.h>
#include <string.h>

/* Commands */

typedef struct _command{
    char* command;
    size_t commandSize;
    int hasOutput;
    char* output;
    size_t outputSize;
}*Command;

Command command_create(char* command, size_t commandSize, char* output, size_t outputSize){
    Command c = (Command) malloc(sizeof(struct _command));
    c->commandSize = commandSize;
    c->command = (char*) malloc(sizeof(char)*commandSize);
    strncpy(c->command,command,commandSize);
    if(output == NULL || outputSize <= 0){
        c->hasOutput = 0;
        c->output = NULL;
    }else{
        c->hasOutput = 1;
        c->output = malloc(sizeof(char)*outputSize);
        strncpy(c->output, output, outputSize);
    }
    c->outputSize = outputSize;
    return c;
}

void command_destroy(Command c){
    free(c->command);
    free(c->output);
    free(c);
}

/* Comments */

typedef struct _comment{
    char* comment;
    size_t commandSize;
}*Comment;

Comment comment_create(char* comment, size_t comSize){
    Comment c = (Comment) malloc(sizeof(struct _comment));
    c->commandSize = comSize;
    c->comment = (char*) malloc(sizeof(char)*comSize);
    strncpy(c->comment, comment, comSize);
    return c;
}

void comment_destroy(Comment c){
    free(c->comment);
    free(c);
}

/* Nodes */

typedef enum _node_type{
    COMMENT,
    COMMAND
}NodeType;

typedef struct _parse_tree_node{
    NodeType type;
    union{
        Comment comment;
        Command command;
    } c;
}*Node;

Node tree_node_create_comment(char* comment, size_t comSize){
    Node n = (Node) malloc(sizeof(struct _parse_tree_node));
    n->type = COMMENT;
    n->c.comment = comment_create(comment, comSize);
    return n;
}

Node tree_node_create_command(char* command, size_t commandSize, char* output, size_t outputSize){
    Node n = (Node) malloc(sizeof(struct _parse_tree_node));
    n->type = COMMAND;
    n->c.command = command_create(command, commandSize, output, outputSize);
    return n;
}

void tree_node_destroy(Node node){
    switch(node->type){
        case COMMAND: comment_destroy(node->c.comment);
                      break;
        case COMMENT: command_destroy(node->c.command);
                      break;
        default: break;
    }
    free(node);
}

/* Tree */

struct _parse_tree{
    int numNodes;
    int load;
    Node* nodes;
};

ParseTree parse_tree_create(int size){
    ParseTree pt = (ParseTree) malloc(sizeof(struct _parse_tree));
    pt->numNodes = size;
    pt->load = 0;
    pt->nodes = (Node*) calloc(size, sizeof(struct _parse_tree_node));
    return pt;
}

void parse_tree_add_comment(ParseTree pt, char* comment, int comSize){
    if(pt->load >= pt->numNodes){
        pt->numNodes *= 2;
        pt->nodes = realloc(pt->nodes, pt->numNodes);
    }
    pt->nodes[pt->load++] = tree_node_create_comment(comment, comSize);
}

void parse_tree_add_command(ParseTree pt, char* command, int commandSize, char* output, int outputSize){
    if(pt->load >= pt->numNodes){
        pt->numNodes *= 2;
        pt->nodes = realloc(pt->nodes, pt->numNodes);
    }
    pt->nodes[pt->load++] = tree_node_create_command(command, commandSize, output, outputSize);
}

void parse_tree_destroy(ParseTree pt){
    for(int i = 0; i < pt->numNodes; i++)
        if(pt->nodes[i]) tree_node_destroy(pt->nodes[i]);
    free(pt->nodes);
    free(pt);
}
