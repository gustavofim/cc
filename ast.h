#ifndef AST_H
#define AST_H

#include "types.h"
#include "str_table.h"
#include "var_table.h"
#include "func_table.h"

typedef enum {
    EQ_NODE,
    NE_NODE,
    GT_NODE,
    LT_NODE,
    GE_NODE,
    LE_NODE,

    PLUS_NODE,
    MINUS_NODE,
    TIMES_NODE,
    OVER_NODE,
    MOD_NODE,
    UNARY_MINUS_NODE,

    AND_NODE,
    ARGS_NODE,
    ASSIGN_NODE,
    BLOCK_NODE,
    BREAK_NODE,
    CONT_NODE,
    CHAR_VAL_NODE,
    IF_NODE,
    INT_VAL_NODE,
    NOT_NODE,
    OR_NODE,
    PARAM_NODE,
    PROGRAM_NODE,
    RET_NODE,
    FLOAT_VAL_NODE,
    FUNC_DECL_NODE,
    FUNC_USE_NODE,
    STR_VAL_NODE,
    VAR_DECL_NODE,
    VAR_USE_NODE,
    WHILE_NODE,
    C2I_NODE,
    C2F_NODE,
    I2F_NODE,
} NodeKind;

struct node; // Opaque structure to ensure encapsulation.

typedef struct node AST;

AST* new_node(NodeKind kind, int data, Type type);

void add_child(AST *parent, AST *child);
AST* get_child(AST *parent, int idx);

AST* new_subtree(NodeKind kind, Type type, int child_count, ...);

NodeKind get_kind(AST *node);
char* kind2str(NodeKind kind);

int get_data(AST *node);
void set_float_data(AST *node, float data);
float get_float_data(AST *node);
void set_char_data(AST *node, char data);
char get_char_data(AST *node);
void set_st_node_data(AST *node, StNode *data);
StNode *get_st_node_data(AST *node);
void set_vt_node_data(AST *node, VtNode *data);
VtNode *get_vt_node_data(AST *node);
void set_ft_node_data(AST *node, FtNode *data);
FtNode *get_ft_node_data(AST *node);

Type get_node_type(AST *node);
int get_child_count(AST *node);

void print_tree(AST *ast);
void print_dot(AST *ast);

void free_tree(AST *ast);

#endif
