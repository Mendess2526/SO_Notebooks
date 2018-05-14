#include "parse_tree.h"
#include "strings.h"
#include "logger.h"
#include "list.h"
#include "utilities.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define OUTPUT_START     ">>>"
#define OUTPUT_END       "<<<"
#define OUTPUT_START_LEN (1 + strlen(OUTPUT_START) + 1)
#define OUTPUT_END_LEN   (1 + strlen(OUTPUT_END))
#define IS_OUTPUT_START(line, len) \
    ((len) > 2 && 0 == strncmp(line, OUTPUT_START, 3))
#define IS_OUTPUT_END(line, len)   \
    ((len) > 2 && 0 == strncmp(line, OUTPUT_END, 3))

/* Commands */

struct _command{
    int dependency;
    String command;
    String output;
    IdxList dependants;
    Command pipe;
    Command prev;
};

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
    PtrList nodes;   /**< The node array */
    IdxList batches; /**< The indexes of the batches */
};

static Command command_create       (Command prev, String command, int dependency);
static void    command_destroy      (Command c);

static Comment comment_create (String comment);
static void    comment_destroy(Comment c);

static Node tree_node_create_comment(Comment comment);
static Node tree_node_create_command(Command command);
static void tree_node_destroy       (Node node);

static void parse_tree_add_command(ParseTree pt, Command command);
static void parse_tree_add_comment(ParseTree pt, Comment comment);
static void parse_tree_chain_command(ParseTree pt, Command command);
static int  parse_tree_parse_command(ParseTree pt, char* line, size_t length);

ParseTree parse_tree_create(int size){
    ParseTree pt = (ParseTree) malloc(sizeof(struct _parse_tree));
    pt->state = TEXT_MODE;
    pt->nodes = ptr_list_create(size);
    pt->batches = idx_list_create(size);
    return pt;
}

int parse_tree_add_line(ParseTree pt, char* line, size_t length){
    int finishBatch = -1;
    if(!line) return idx_list_len(pt->batches);
    if(IS_OUTPUT_START(line, length)){
        pt->state = OUTPUT_MODE;
        return finishBatch;
    }
    if(IS_OUTPUT_END(line, length)){
        pt->state = TEXT_MODE;
        return finishBatch;
    }
    if(pt->state == TEXT_MODE){
        if(*line == '$'){
            finishBatch = parse_tree_parse_command(pt, line + 1, length - 1);
        }else{
            String s;
            string_init(&s, line, length);
            parse_tree_add_comment(pt, comment_create(s));
        }
    }
    return finishBatch;
}

Command parse_tree_get_batch(ParseTree pt, int batch){
    Node n = ptr_list_index(pt->nodes, idx_list_index(pt->batches, batch));
    if(!n) LOG_WARNING("Invalid node index\n");
    if(n->type == N_COMMENT){
        LOG_WARNING("Comment node is not a batch\n");
        return NULL;
    }
    return n->c.command;
}

void parse_tree_destroy(ParseTree pt){
    size_t len = ptr_list_len(pt->nodes);
    for(size_t i = 0; i < len; i++)
        tree_node_destroy(ptr_list_index(pt->nodes, i));
    free(pt->nodes);
    idx_list_free(pt->batches);
    free(pt);
}

static void parse_tree_add_comment(ParseTree pt, Comment comment){
    ptr_list_append(pt->nodes, tree_node_create_comment(comment));
}

static void parse_tree_add_command(ParseTree pt, Command command){
    ptr_list_append(pt->nodes, tree_node_create_command(command));
}

static Node last_command_node(ParseTree pt){
    return ptr_list_index(pt->nodes, idx_list_last(pt->batches));
}

static void parse_tree_chain_command(ParseTree pt, Command command){
    Node n = last_command_node(pt);
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
        for(int backI = command->dependency; backI > 1 && cur; --backI, cur = cur->prev);
        if(!cur) LOG_FATAL("Invalid dependency\n"); //TODO print bad line?
        idx_list_append(cur->dependants, command->dependency);
    }
}

static int parse_tree_parse_command(ParseTree pt, char* line, size_t length){
    int finishBatch = -1;
    Command c;
    String s;
    if(line[0] == '|'){
        string_init(&s, line + 1, length - 1);
        c = command_create(last_command_node(pt)->c.command, s, 1);
        parse_tree_chain_command(pt, c);
    }else if(isdigit(line[0])){
        char* tail;
        int dep = strtol(line, &tail, 10);
        string_init(&s, tail + 1, length - ((tail + 1) - line));
        c = command_create(last_command_node(pt)->c.command, s, dep);
        parse_tree_chain_command(pt, c);
    }else{
        string_init(&s, line, length);
        c = command_create(NULL, s, 0);
        finishBatch = idx_list_len(pt->batches);
        idx_list_append(pt->batches, ptr_list_len(pt->nodes));
    }

    parse_tree_add_command(pt, c);

    return finishBatch;
}

/* Commands */

static Command command_create(Command prev, String command, int dependency){
    Command c = (Command) malloc(sizeof(struct _command));
    c->command = command;
    c->dependency = dependency;
    c->output.s = NULL;
    c->output.length = 0;
    c->dependants = idx_list_create(10);
    c->pipe =  NULL;
    c->prev = prev;
    return c;
}

String command_get_command(Command c){
    return c->command;
}

void command_append_output(Command c, String s){
    if(c->output.length == 0) c->output = s;
    else{
        String newLine;
        string_init(&newLine, "\n", 1);
        string_append(&c->output, newLine);
        string_append(&c->output, s);
    }
}

Command command_pipe(Command c){
    return c->pipe;
}

int command_get_chained_num(Command c){
    int i = 0;
    while(c){
        c = c->pipe;
        i++;
    }
    return i;
}

int command_get_dependency(Command c){
    return c->dependency;
}

IdxList command_get_dependants(Command c){
    return c->dependants;
}

static void command_destroy(Command c){
    string_free(c->command);
    string_free(c->output);
    free(c);
}

/* Comment */

static Comment comment_create(String comment){
    Comment c = (Comment) malloc(sizeof(struct _comment));
    c->comment = comment;
    return c;
}

static void comment_destroy(Comment c){
    string_free(c->comment);
    free(c);
}

/* Nodes */

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

/* Utils */

char* comment_dump(Comment c){
    char* comment = (char*) malloc(sizeof(char)* c->comment.length+1);
    strncpy(comment, c->comment.s, c->comment.length);
    comment[c->comment.length] = '\0';
    return comment;
}

char* command_dump(Command c){
    size_t size = 1; // $
    if(c->dependency) size += 1; // |
    if(c->dependency > 1) size += 12; // dep num
    size += c->command.length; // Command string
    if(c->output.s) size += OUTPUT_START_LEN // Output
                        + c->output.length
                        + OUTPUT_END_LEN;
    size += 1; // '\0'

    char* command = (char*) malloc(sizeof(char) * size);
    char* cmd = command;

    strncpy(cmd, "$", c->command.length);
    cmd += 1;
    if(c->dependency){
        if(c->dependency > 1){
           char num[12];
           size_t len = int2string(c->dependency, num, 12);
           strncpy(cmd, num, len);
           cmd += len;
        }
        strncpy(cmd, "|", c->command.length);
        cmd += 1;
    }
    strncpy(cmd, c->command.s, c->command.length);
    cmd += c->command.length;
    if(c->output.s){
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
    size_t numNodes = ptr_list_len(pt->nodes);
    char** file = (char**) malloc(sizeof(char*) * (numNodes + 1));
    file[numNodes] = NULL;
    for(size_t i = 0; i < numNodes; i++){
        Node n = ptr_list_index(pt->nodes, i);
        switch(n->type){
            case N_COMMENT: file[i] = comment_dump(n->c.comment);
                          break;
            case N_COMMAND: file[i] = command_dump(n->c.command);
                          break;
            default: break;
        }
    }
    return file;
}

/* ------------------- Printing --------------------- */

static void printCommand(Command c);
static void printComment(Comment c);
static void printNode(Node n);

#include "colors.h"
#include <stdio.h>

void parse_tree_print(ParseTree pt){
    size_t len = ptr_list_len(pt->nodes);
    printf("State: %d\n", pt->state);
    printf("%ld nodes\n", len);
    printf("Nodes:\n");
    for(size_t i = 0, btc = 0; i<len; i++){
        ssize_t idx = idx_list_index(pt->batches, btc);
        if(idx < 0) LOG_WARNING("Invalid index accessed during print\n");
        size_t isFirst = (size_t) idx == i;
        if(isFirst){
            printf(YELLOW "fst" RESET);
            btc++;
        }
        printNode(ptr_list_index(pt->nodes, i));
    }
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
    size_t length = c->comment.length;
    c->comment.s[length] = '\0';
    printf(GREEN "\t%s" RESET "\n", c->comment.s);
    c->comment.length = length;
}

char* mystrndup(char* str, size_t len){
    char* s = malloc(sizeof(char) * len);
    strncpy(s, str, len);
    return s;
}

void printCommand(Command c){
    static int notFirst;
    size_t length = c->command.length;
    c->command.s[c->command.length] = '\0';
    printf(BLUE);
    if(notFirst)
        printf("\t");
    else
        notFirst = 1;

    printf("\t$");
    if(c->dependency > 1) printf("%d", c->dependency);
    if(c->dependency) printf("|");
    printf("%s " RESET, c->command.s);

    printf("\n");
    c->command.length = length; // Fix cheating
    if(c->output.s){
        printf(RED "\t\t>>>\n");
        length = c->output.length;
        c->output.s[c->output.length] = '\0';
        char* out = mystrndup(c->output.s, c->output.length);
        char* tk = strtok(out,"\n");
        do{
            printf("\t\t%s\n", tk);
            tk = strtok(NULL, "\n");
        }while(tk && tk[0]!='\n');
        free(out);
        c->output.length = length;
        printf("\t\t<<<\n" RESET);
    }
    printf("\t\t");
    for(size_t i = 0; i < idx_list_len(c->dependants); i++){
        printf(RED "%ld " RESET, idx_list_index(c->dependants, i));
    }
    printf("\n");
    if(c->pipe) printCommand(c->pipe);
    else notFirst = 0;
}
