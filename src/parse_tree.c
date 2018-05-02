#include "parse_tree.h"

#include <stdlib.h>
#include <string.h>
#include "strings.h"

#define OFFSET_STR(s,o)  (s+o)
#define OUTPUT_START     "\n>>>\n"
#define OUTPUT_END       "\n<<<\n"
#define OUTPUT_START_LEN (strlen(OUTPUT_START))
#define OUTPUT_END_LEN   (strlen(OUTPUT_END))

#define IS_OUTPUT_START(line, len) (len > 2 && 0 == strcmp(line, OUTPUT_START))
#define IS_OUTPUT_END(line, len)   (len > 2 && 0 == strcmp(line, OUTPUT_START))

/* Commands */

typedef struct _command{
    String command;
    int hasOutput;
    String output;
}*Command;

/* Comments */

typedef struct _comment{
    String comment;
}*Comment;

/* Nodes */

typedef enum _node_type{
    N_COMMENT,
    N_COMMAND
}NodeType;

typedef struct _parse_tree_node{
    NodeType type;
    union{
        Comment comment;
        Command command;
    } c;
}*Node;

/* Tree */

typedef enum _parse_state{
    OUTPUT_MODE,
    TEXT_MODE
}ParseState;

struct _parse_tree{
    ParseState state;
    int numNodes;
    int load;
    Node* nodes;
};

static Command command_create       (String command);
static void    command_append_output(Command c, String s);
static void    command_destroy      (Command c);

static Comment comment_create (String comment);
static void    comment_destroy(Comment c);

static Node tree_node_create_comment(String comment);
static Node tree_node_create_command(String command);
static void tree_node_append_output (Node node, String output);
static void tree_node_destroy       (Node node);


ParseTree parse_tree_create(int size){
    ParseTree pt = (ParseTree) malloc(sizeof(struct _parse_tree));
    pt->state = TEXT_MODE;
    pt->numNodes = size;
    pt->load = 0;
    pt->nodes = (Node*) malloc(sizeof(struct _parse_tree_node) * size);
    return pt;
}

void parse_tree_add_comment(ParseTree pt, String comment){
    if(pt->load >= pt->numNodes){
        pt->numNodes *= 2;
        pt->nodes = realloc(pt->nodes, sizeof(struct _parse_tree_node) * pt->numNodes);
    }
    pt->nodes[pt->load++] = tree_node_create_comment(comment);
}

void parse_tree_add_command(ParseTree pt, String command){
    if(pt->load >= pt->numNodes){
        pt->numNodes *= 2;
        pt->nodes = realloc(pt->nodes, sizeof(struct _parse_tree_node) * pt->numNodes);
    }
    pt->nodes[pt->load++]
        = tree_node_create_command(command);
}

void parse_tree_append_output(ParseTree pt, String output){
    tree_node_append_output(pt->nodes[pt->load], output);
}

void parse_tree_add_line(ParseTree pt, char* line, size_t length){
    if(IS_OUTPUT_START(line, length)){
        pt->state = OUTPUT_MODE;
        return;
    }
    if(IS_OUTPUT_END(line, length)){
        pt->state = TEXT_MODE;
        return;
    }
    String s;
    string_init(&s, line, length);
    if(pt->state == TEXT_MODE){
        if(*line == '$'){
            parse_tree_add_command(pt, s);
        }else{
            parse_tree_add_comment(pt, s);
        }
    }else{
        parse_tree_append_output(pt, s);
    }
    string_free(s);
}

void parse_tree_destroy(ParseTree pt){
    for(int i = 0; i < pt->load; i++)
        if(pt->nodes[i]) tree_node_destroy(pt->nodes[i]);
    free(pt->nodes);
    free(pt);
}

/* Utils */

char* comment_dump(Comment c){
    char* comment = (char*) malloc(sizeof(char)* c->comment.length+1);
    strncpy(comment, c->comment.s, c->comment.length);
    comment[c->comment.length] = '\0';
    return comment;
}

char* command_dump(Command c){
    size_t size =
        c->command.length
        + OUTPUT_START_LEN
        + c->output.length
        + OUTPUT_END_LEN
        + 1;

    char* command = (char*) malloc(sizeof(char) * size);
    char* cmd = command;

    strncpy(cmd, c->command.s, c->command.length);
    cmd += c->command.length;
    if(c->hasOutput){
        strncpy(cmd, OUTPUT_START, OUTPUT_START_LEN);
        cmd += OUTPUT_START_LEN;

        strncpy(cmd, c->output.s, c->output.length);
        cmd += c->output.length;

        strncpy(cmd, OUTPUT_END, OUTPUT_END_LEN);
        cmd += OUTPUT_END_LEN;
    }
    *cmd = '\0';
    return command;
}

char** parse_tree_dump(ParseTree pt){
    char** file = (char**) malloc(sizeof(char*)*pt->load+1);
    file[pt->load] = NULL;
    for(int i = 0; i < pt->load; i++){
        switch(pt->nodes[i]->type){
            case N_COMMENT: file[i] = comment_dump(pt->nodes[i]->c.comment);
                          break;
            case N_COMMAND: file[i] = command_dump(pt->nodes[i]->c.command);
                          break;
            default: break;
        }
    }
    return file;
}

/* STATICS */

static Command command_create(String command){
    Command c = (Command) malloc(sizeof(struct _command));
    string_init(&c->command, command.s, command.length);
    c->hasOutput = 0;
    c->output.s = NULL;
    c->output.length = 0;
    return c;
}

static void command_append_output(Command c, String s){
    if(c->output.length == 0){
        string_init(&(c->output), s.s, s.length);
    }
    int newLength = c->output.length + s.length + 1;
    c->output.s = realloc(c->output.s, sizeof(char) * (newLength));
    c->output.s[c->output.length] = '\n';
    memcpy(c->output.s + c->output.length + 1, s.s, s.length);
    c->output.length = newLength;
}

static void command_destroy(Command c){
    free(c->command.s);
    free(c->output.s);
    free(c);
}

static Comment comment_create(String comment){
    Comment c = (Comment) malloc(sizeof(struct _comment));
    string_init(&c->comment, comment.s, comment.length);
    return c;
}

static void comment_destroy(Comment c){
    free(c->comment.s);
    free(c);
}

static Node tree_node_create_comment(String comment){
    Node n = (Node) malloc(sizeof(struct _parse_tree_node));
    n->type = N_COMMENT;
    n->c.comment = comment_create(comment);
    return n;
}

static Node tree_node_create_command(String command){
    Node n = (Node) malloc(sizeof(struct _parse_tree_node));
    n->type = N_COMMAND;
    n->c.command = command_create(command);
    return n;
}

static void tree_node_append_output(Node node, String output){
    command_append_output(node->c.command, output);
}

static void tree_node_destroy(Node node){
    switch(node->type){
        case N_COMMENT: comment_destroy(node->c.comment);
                      break;
        case N_COMMAND: command_destroy(node->c.command);
                      break;
        default: break;
    }
    free(node);
}
