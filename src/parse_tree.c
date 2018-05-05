#include "parse_tree.h"
#include "strings.h"
#include "logger.h"

#include <stdlib.h>
#include <string.h>

#define OFFSET_STR(s,o)  (s+o)
#define OUTPUT_START     ">>>"
#define OUTPUT_END       "<<<"
#define OUTPUT_START_LEN (1 + strlen(OUTPUT_START) + 1)
#define OUTPUT_END_LEN   (1 + strlen(OUTPUT_END))
#define IS_OUTPUT_START(line, len) (len > 2 && 0 == strncmp(line, OUTPUT_START, 3))
#define IS_OUTPUT_END(line, len)   (len > 2 && 0 == strncmp(line, OUTPUT_END, 3))

/* Commands */

typedef struct _command{
    int isDependant;
    String command;
    int hasOutput;
    String output;
    struct _command* pipe;
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
    int isCommandChain;
    int maxNumNodes;
    int curNumNodes;
    Node* nodes;
    int* commandNodes;
    int maxNumIdx;
    int curNumIdx;
};

static Command command_create       (String command, int isDependant);
static void    command_append_output(Command c, String s);
static void    command_destroy      (Command c);

static Comment comment_create (String comment);
static void    comment_destroy(Comment c);

static Node tree_node_create_comment(Comment comment);
static Node tree_node_create_command(Command command);
static void tree_node_append_output (Node node, String output);
static void tree_node_destroy       (Node node);

static int  parse_tree_add_command(ParseTree pt, Command command);
static void parse_tree_add_comment(ParseTree pt, Comment comment);

ParseTree parse_tree_create(int size){
    ParseTree pt = (ParseTree) malloc(sizeof(struct _parse_tree));
    pt->state = TEXT_MODE;
    pt->isCommandChain = 0;
    pt->maxNumNodes = size;
    pt->curNumNodes = 0;
    pt->nodes = (Node*) malloc(sizeof(struct _parse_tree_node) * size);
    pt->commandNodes = (int*) malloc(sizeof(int) * size);
    pt->maxNumIdx = size;
    pt->curNumIdx = 0;
    return pt;
}

void parse_tree_add_comment(ParseTree pt, Comment comment){
    if(pt->curNumNodes >= pt->maxNumNodes){
        pt->maxNumNodes *= 2;
        pt->nodes = realloc(pt->nodes, sizeof(struct _parse_tree_node) * pt->maxNumNodes);
    }
    pt->nodes[pt->curNumNodes++] = tree_node_create_comment(comment);
}

int parse_tree_add_command(ParseTree pt, Command command){
    if(pt->curNumNodes >= pt->maxNumNodes){
        pt->maxNumNodes *= 2;
        pt->nodes = realloc(pt->nodes, sizeof(struct _parse_tree_node) * pt->maxNumNodes);
    }
    pt->nodes[pt->curNumNodes]
        = tree_node_create_command(command);

    if(pt->curNumIdx >= pt->maxNumIdx){
        pt->maxNumIdx *= 2;
        pt->commandNodes = realloc(pt->commandNodes, sizeof(int) * pt->maxNumIdx);
    }
    pt->commandNodes[pt->curNumIdx++] = pt->curNumNodes;

    return pt->curNumNodes++;
}

#define LAST_COMMAND_NODE(pt) (pt->nodes[pt->commandNodes[pt->curNumIdx - 1]])

void parse_tree_chain_command(ParseTree pt, Command command){
    Node n = LAST_COMMAND_NODE(pt);
    Command cur = NULL;
    if(n == NULL){
        LOG_WARNING("Chaining to null node\n");
    }else if(n->type == N_COMMENT){
        LOG_WARNING("Chaining to comment node\n");
    }else if(NULL == (cur = n->c.command)){
        LOG_WARNING("Chaining to null command\n");
    }else{
        while(cur->pipe != NULL) cur = cur->pipe;
        cur->pipe = command;
        cur = cur->pipe;
    }
}

void parse_tree_append_output(ParseTree pt, String output){
    tree_node_append_output(LAST_COMMAND_NODE(pt), output);
}

int parse_tree_add_line(ParseTree pt, char* line, size_t length){
    int finishBatch = -1;
    if(IS_OUTPUT_START(line, length)){
        pt->state = OUTPUT_MODE;
        return finishBatch;
    }
    if(IS_OUTPUT_END(line, length)){
        pt->state = TEXT_MODE;
        return finishBatch;
    }
    String s;
    if(pt->state == TEXT_MODE){
        if(*line == '$'){
            Command c;
            if(line[1] == '|'){
                string_init(&s, line + 2, length - 2);
                c = command_create(s, 1);
                parse_tree_chain_command(pt, c);
            }else{
                string_init(&s, line + 1, length - 1);
                c = command_create(s, 0);
            }

            finishBatch = parse_tree_add_command(pt, c);
        }else{
            string_init(&s, line, length);
            parse_tree_add_comment(pt, comment_create(s));
        }
    }else{
        string_init(&s, line, length);
        parse_tree_append_output(pt, s);
    }
    return finishBatch;
}

void parse_tree_destroy(ParseTree pt){
    for(int i = 0; i < pt->curNumNodes; i++)
        tree_node_destroy(pt->nodes[i]);
    free(pt->nodes);
    free(pt->commandNodes);
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
    size_t size = 1; // $
    if(c->isDependant) size += 1; // |
    size += c->command.length; // Command string
    if(c->hasOutput) size += OUTPUT_START_LEN // Output
                        + c->output.length
                        + OUTPUT_END_LEN;
    size += 1; // '\0'

    char* command = (char*) malloc(sizeof(char) * size);
    char* cmd = command;

    strncpy(cmd, "$", c->command.length);
    cmd += 1;
    if(c->isDependant){
        strncpy(cmd, "|", c->command.length);
        cmd += 1;
    }
    strncpy(cmd, c->command.s, c->command.length);
    cmd += c->command.length;
    if(c->hasOutput){
        strncpy(cmd, "\n"OUTPUT_START"\n", OUTPUT_START_LEN);
        cmd += OUTPUT_START_LEN;

        strncpy(cmd, c->output.s, c->output.length);
        cmd += c->output.length;

        strncpy(cmd, "\n"OUTPUT_END, OUTPUT_END_LEN);
        cmd += OUTPUT_END_LEN;
    }
    *cmd = '\0';
    return command;
}

char** parse_tree_dump(ParseTree pt){
    char** file = (char**) malloc(sizeof(char*) * (pt->curNumNodes + 1));
    file[pt->curNumNodes] = NULL;
    for(int i = 0; i < pt->curNumNodes; i++){
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

static Command command_create(String command, int isDependant){
    Command c = (Command) malloc(sizeof(struct _command));
    c->command = command;
    c->isDependant = isDependant;
    c->hasOutput = 0;
    c->output.s = NULL;
    c->output.length = 0;
    c->pipe =  NULL;
    return c;
}

static void command_append_output(Command c, String s){
    c->hasOutput = 1;
    if(c->output.length == 0) c->output = s;
    else{
        String newLine = {0,1};
        char nl = '\n';
        newLine.s = &nl;
        string_append(&c->output, newLine);
        string_append(&c->output, s);
    }
}

static void command_destroy(Command c){
    free(c->command.s);
    free(c->output.s);
    free(c);
}

static Comment comment_create(String comment){
    Comment c = (Comment) malloc(sizeof(struct _comment));
    c->comment = comment;
    return c;
}

static void comment_destroy(Comment c){
    free(c->comment.s);
    free(c);
}

static Node tree_node_create_comment(Comment comment){
    Node n = (Node) malloc(sizeof(struct _parse_tree_node));
    n->type = N_COMMENT;
    n->c.comment = comment;
    return n;
}

static Node tree_node_create_command(Command command){
    Node n = (Node) malloc(sizeof(struct _parse_tree_node));
    n->type = N_COMMAND;
    n->c.command = command;
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

/* ------------------- Printing --------------------- */

static void printCommand(Command c);
static void printComment(Comment c);
static void printNode(Node n);

#include "colors.h"
#include <stdio.h>

void parse_tree_print(ParseTree pt){
    printf("State: %d\n", pt->state);
    printf("isCommandChain %d\n", pt->isCommandChain);
    printf("%d/%d nodes\n", pt->curNumNodes, pt->maxNumNodes);
    printf("Nodes:\n");
    for(int i = 0; i<pt->curNumNodes; i++)
        printNode(pt->nodes[i]);
}

void printNode(Node n){
    printf("\tType: %d", n->type);
    switch(n->type){
        case N_COMMENT: printComment(n->c.comment);
                        break;
        case N_COMMAND: printCommand(n->c.command);
                        break;
        default: break;
    }
}

void printComment(Comment c){
    int length = c->comment.length;
    c->comment.s[length] = '\0';
    printf(GREEN "\t%s" RESET "\n", c->comment.s);
    c->comment.length = length;
}
void printCommand(Command c){
    static int notFirst;
    int length = c->command.length;
    c->command.s[c->command.length] = '\0';
    printf(BLUE);
    if(notFirst)
        printf("\t");
    else
        notFirst = 1;
    printf("\t$");
    if(c->isDependant) printf("|");
    printf("%s " RESET, c->command.s);

    c->command.length = length;
    if(c->hasOutput){
        printf(RED "\n>>>\n");
        length = c->output.length;
        c->output.s[c->output.length] = '\0';
        printf("%s\n", c->output.s);
        c->output.length = length;
        printf("<<<\n" RESET);
    }
    if(c->pipe) printCommand(c->pipe);
    else notFirst = 0;
    printf("\n");
}
