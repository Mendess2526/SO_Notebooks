#include "parse_tree.h"
#include "logger.h"
#include "utilities.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define OUTPUT_START     ">>>"
#define OUTPUT_END       "<<<"
#define OUTPUT_START_LEN (1 + strlen(OUTPUT_START) + 1)
#define OUTPUT_END_LEN   (strlen(OUTPUT_END))
#define IS_OUTPUT_START(line, len) \
    ((len) > 2 && 0 == strncmp(line, OUTPUT_START, 3))
#define IS_OUTPUT_END(line, len)   \
    ((len) > 2 && 0 == strncmp(line, OUTPUT_END, 3))

/* Commands */
struct _command{
    size_t dependency;
    String command;
    String output;
    IdxList dependants;
    Command pipe;
};

/* Comments */

typedef struct _comment{
    String comment;
}* Comment;

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
    }c;
}* Node;

/* Tree */

typedef enum _parse_state{
    OUTPUT_MODE,
    TEXT_MODE
}ParseState;

struct _parse_tree{
    ParseState state;
    PtrList nodes;    /**< The node array */
    IdxList commands; /**< The indexes of the commands */
    IdxList batches;  /**< The indexes of the batches */
};

/** Global variable to count the number of lines */
static int LINE_NUMBER;
/** Global variable to store the current line, needed for error messages*/
static String CURRENT_LINE;

static Command command_create(char* line, size_t length, size_t dependency);

static void command_destroy(Command c);

static Comment comment_create(String comment);

static void comment_destroy(Comment c);

static Node tree_node_create_comment(Comment comment);

static Node tree_node_create_command(Command command);

static void tree_node_destroy(Node node);

static void parse_tree_add_command(ParseTree pt, Command command);

static void parse_tree_add_comment(ParseTree pt, Comment comment);

static int parse_tree_chain_command(ParseTree pt, Command command);

static ssize_t parse_tree_parse_command(ParseTree pt,
                                        char* line,
                                        size_t length);

ParseTree parse_tree_create(size_t size){
    LINE_NUMBER = 0;
    ParseTree pt = (ParseTree) malloc(sizeof(struct _parse_tree));
    pt->state = TEXT_MODE;
    pt->nodes = ptr_list_create(size);
    pt->commands = idx_list_create(size);
    pt->batches = idx_list_create(size);
    return pt;
}

ssize_t parse_tree_add_line(ParseTree pt, char* line, size_t length){
    ssize_t finishBatch = -1;
    LINE_NUMBER++;
    if(!line) return idx_list_len(pt->batches) - 1;
    if(IS_OUTPUT_START(line, length)){
        pt->state = OUTPUT_MODE;
        return finishBatch;
    }
    if(IS_OUTPUT_END(line, length)){
        pt->state = TEXT_MODE;
        return finishBatch;
    }
    string_init(&CURRENT_LINE, line, length);
    if(pt->state == TEXT_MODE){
        if(*line == '$'){
            finishBatch = parse_tree_parse_command(pt, line + 1, length - 1);
        }else{
            String s;
            string_init(&s, line, length);
            parse_tree_add_comment(pt, comment_create(s));
        }
    }
    string_free(CURRENT_LINE);
    return finishBatch;
}

Command parse_tree_get_batch(ParseTree pt, size_t batch){
    ssize_t idx = idx_list_index(pt->batches, batch);
    Node n = ptr_list_index(pt->nodes, (size_t) idx);
    if(!n){
        LOG_WARNING("Invalid node index\n");
        return NULL;
    }
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
    ptr_list_free(pt->nodes);
    idx_list_free(pt->commands);
    idx_list_free(pt->batches);
    free(pt);
}

static void parse_tree_add_comment(ParseTree pt, Comment comment){
    ptr_list_append(pt->nodes, tree_node_create_comment(comment));
}

static void parse_tree_add_command(ParseTree pt, Command command){
    ptr_list_append(pt->nodes, tree_node_create_command(command));
}

static Node get_dependency_node(ParseTree pt, size_t dependency){
    ssize_t idx = idx_list_index(pt->commands,
                                idx_list_len(pt->commands) - dependency);
    return ptr_list_index(pt->nodes, (size_t) idx);
}

static int parse_tree_chain_command(ParseTree pt, Command command){
    if(!command) return -2;
    Node n = get_dependency_node(pt, command->dependency);
    Command cur = NULL;
    if(n == NULL){
        LOG_PARSE_ERROR(CURRENT_LINE,
                LINE_NUMBER,
                "Impossible dependency",
                CURRENT_LINE.s[1] == '|' ? 1 : 2);
        return 1;
    }else if(n->type == N_COMMENT){
        LOG_WARNING("Chaining to comment node\n");
    }else if(NULL == (cur = n->c.command)){
        LOG_WARNING("Chaining to null command\n");
    }else if(strnstr(cur->command.s, ">", cur->command.length) != NULL
            && strnstr(cur->command.s, "2>", cur->command.length) == NULL){
        LOG_PARSE_ERROR(CURRENT_LINE, LINE_NUMBER,
                "Can't write to pipe and file at the same time", 1);
        return 1;
    }else{
        Command dep = cur;
        size_t offset = 1;
        for(; cur->pipe != NULL; cur = cur->pipe) offset++;
        cur->pipe = command;
        idx_list_append(dep->dependants, offset);
    }
    return 0;
}

static ssize_t parse_tree_parse_command(ParseTree pt,
                                        char* line,
                                        size_t length){
    ssize_t finishBatch = -1;
    Command c;
    if(line[0] == '|'){
        c = command_create(line + 1, length - 1, 1);
        if(parse_tree_chain_command(pt, c)) return -2;
    }else if(isdigit(line[0])){
        char* tail;
        size_t dep = (size_t) strtol(line, &tail, 10);
        if(*tail != '|'){
            LOG_PARSE_ERROR(CURRENT_LINE, LINE_NUMBER, "Missing pipe",
                            (int) ((tail + 1) - line));
            return -2;
        }
        c = command_create(tail + 1, length - ((tail + 1) - line), dep);
        if(parse_tree_chain_command(pt, c)) return -2;
    }else{
        c = command_create(line, length, 0);
        finishBatch = idx_list_len(pt->batches) - 1;
        idx_list_append(pt->batches, ptr_list_len(pt->nodes));
    }

    idx_list_append(pt->commands, ptr_list_len(pt->nodes));
    parse_tree_add_command(pt, c);

    return finishBatch;
}

/* Commands */
static Command command_create(char* line, size_t length, size_t dependency){
    Command c = (Command) malloc(sizeof(struct _command));
    char* error;
    if(dependency != 0 && (error = strnstr(line, "<", length)) != NULL){
        LOG_PARSE_ERROR(CURRENT_LINE, LINE_NUMBER,
                        "Can't read from pipe and file at the same time",
                        (int) (error - line + 2));
        return NULL;
    }
    c->dependency = dependency;
    string_init(&c->command, line, length);
    string_init(&c->output, NULL, 0);
    c->dependants = idx_list_create(10);
    c->pipe = NULL;
    return c;
}

String command_get_command(Command c){
    return c->command;
}

void command_append_output(Command c, String s){
    string_append(&c->output, s);
}

Command command_pipe(Command c){
    return c->pipe;
}

IdxList command_get_dependants(Command c){
    return c->dependants;
}

static void command_destroy(Command c){
    string_free(c->command);
    string_free(c->output);
    idx_list_free(c->dependants);
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
        case N_COMMENT:
            comment_destroy(node->c.comment);
            break;
        case N_COMMAND:
            command_destroy(node->c.command);
            break;
        default:
            break;
    }
    free(node);
}

/* Utils */

String comment_dump(Comment c){
    String final;
    string_init(&final, c->comment.s, c->comment.length);
    string_append_array(&final, "\n", 1);
    return final;
}

String command_dump(Command c){
    String cmd;
    string_init(&cmd, NULL, 0);

    string_append_array(&cmd, "$", 1);
    if(c->dependency){
        if(c->dependency > 1){
            char num[12];
            size_t len = int2string((int) c->dependency, num, 12);
            string_append_array(&cmd, num, len);
        }
        string_append_array(&cmd, "|", 1);
    }
    string_append(&cmd, c->command);
    if(c->output.s){
        string_append_array(&cmd, "\n"OUTPUT_START"\n", OUTPUT_START_LEN);
        string_append_array(&cmd, c->output.s,          c->output.length);
        string_append_array(&cmd, OUTPUT_END,           OUTPUT_END_LEN);
    }
    string_append_array(&cmd, "\n", 1);
    return cmd;
}

String parse_tree_dump(ParseTree pt){
    size_t numNodes = ptr_list_len(pt->nodes);
    String file;
    string_init(&file, NULL, 0);
    for(size_t i = 0; i < numNodes; i++){
        Node n = ptr_list_index(pt->nodes, i);
        switch(n->type){
            case N_COMMENT:
                string_append(&file, comment_dump(n->c.comment));
                break;
            case N_COMMAND:
                string_append(&file, command_dump(n->c.command));
                break;
            default:
                break;
        }
    }
    string_append_array(&file, "\0", 1);
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
    for(size_t i = 0, btc = 0; i < len; i++){
        ssize_t idx = idx_list_index(pt->batches, btc);
        if(idx < 0) LOG_WARNING("Invalid index accessed during print\n");
        int isFirst = (size_t) idx == i;
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
        case N_COMMENT:
            printComment(n->c.comment);
            break;
        case N_COMMAND:
            printCommand(n->c.command);
            break;
        default:
            break;
    }
}

void printComment(Comment c){
    size_t length = c->comment.length;
    c->comment.s[length] = '\0';
    printf(GREEN "\t%s" RESET "\n", c->comment.s);
    c->comment.length = length;
}

void printCommand(Command c){
    static int notFirst;
    printf(BLUE);
    if(notFirst)
        printf("\t");
    else
        notFirst = 1;

    printf("\t$");
    if(c->dependency > 1) printf("%ld", c->dependency);
    if(c->dependency) printf("|");
    char* cmd = malloc(sizeof(char) * (c->command.length + 1));
    strncpy(cmd, c->command.s, c->command.length);
    cmd[c->command.length] = '\0';
    printf("%s " RESET, cmd);

    printf("\n");
    if(c->output.s){
        printf(RED "\t\t>>>\n");
        char* out = malloc(sizeof(char) * (c->output.length + 1));
        out[c->output.length] = '\0';
        char* tk = strtok(out, "\n");
        do{
            printf("\t\t%s\n", tk);
            tk = strtok(NULL, "\n");
        }while(tk && tk[0] != '\n');
        free(out);
        printf("\t\t<<<\n" RESET);
    }

    if(idx_list_len(c->dependants) > 0){
        printf("\t\t");
        if(idx_list_len(c->dependants) > 0){
            for(size_t i = 0; i < idx_list_len(c->dependants); i++)
                printf(RED "%ld " RESET, idx_list_index(c->dependants, i));
        }
        printf("\n");
    }
    if(c->pipe) printCommand(c->pipe);
    else notFirst = 0;
}
