#include "parse_tree.h"

#include <stdlib.h>
#include <string.h>
#include "strings.h"

/* Commands */

typedef struct _command{
    String command;
    int hasOutput;
    String output;
}*Command;

Command command_create(String command, String output){
    Command c = (Command) malloc(sizeof(struct _command));
    c->command.length = command.length;
    c->command.s = (char*) malloc(sizeof(char) * command.length);
    strncpy(c->command.s, command.s, command.length);
    if(output.s == NULL || output.length <= 0){
        c->hasOutput = 0;
        c->output.s = NULL;
    }else{
        c->hasOutput = 1;
        c->output.s = malloc(sizeof(char) * output.length);
        strncpy(c->output.s, output.s, output.length);
    }
    c->output.length = output.length;
    return c;
}

void command_destroy(Command c){
    free(c->command.s);
    free(c->output.s);
    free(c);
}

/* Comments */

typedef struct _comment{
    char* comment;
    size_t commentSize;
}*Comment;

Comment comment_create(char* comment, size_t comSize){
    Comment c = (Comment) malloc(sizeof(struct _comment));
    c->commentSize = comSize;
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

Node tree_node_create_command(String command, String output){
    Node n = (Node) malloc(sizeof(struct _parse_tree_node));
    n->type = COMMAND;
    n->c.command = command_create(command, output);
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

void parse_tree_add_command(ParseTree pt, String command, String output){
    if(pt->load >= pt->numNodes){
        pt->numNodes *= 2;
        pt->nodes = realloc(pt->nodes, pt->numNodes);
    }
    pt->nodes[pt->load++]
        = tree_node_create_command(command, output);
}

void parse_tree_destroy(ParseTree pt){
    for(int i = 0; i < pt->numNodes; i++)
        if(pt->nodes[i]) tree_node_destroy(pt->nodes[i]);
    free(pt->nodes);
    free(pt);
}

/* Utils */
char* comment_dump(Comment c){
    char* comment = (char*) malloc(sizeof(char)* c->commentSize);
    strncpy(comment, c->comment, c->commentSize);
    return comment;
}

char* command_dump(Command c){
    size_t size = c->commandSize + c->outputSize;
    char* command = (char*) malloc(sizeof(char) * (size));
    strncpy(command, c->command, c->commandSize);
    strncpy(command + c->commandSize, c->output, c->outputSize);
    return command;
}

char** parse_tree_dump(ParseTree pt){
    char** file = (char**) malloc(sizeof(char*)*pt->load+1);
    file[pt->load] = NULL;
    for(int i = 0; i < pt->load; i++){
        switch(pt->nodes[i]->type){
            case COMMENT: file[i] = comment_dump(pt->nodes[i]->c.comment);
                          break;
            case COMMAND: file[i] = command_dump(pt->nodes[i]->c.command);
                          break;
            default: break;
        }
    }
    return file;
}
