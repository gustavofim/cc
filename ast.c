
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "types.h"

#define CHILDREN_LIMIT 20 // Don't try this at home, kids... :P

struct node {
    NodeKind kind;
    union {
        int as_int;
        float as_float;
        char as_char;
		StNode *as_st;
		VtNode *as_vt;
		FtNode *as_ft;
    } data;
    Type type;
    int count;
    AST* child[CHILDREN_LIMIT];
};

AST* new_node(NodeKind kind, int data, Type type) {
    AST* node = malloc(sizeof * node);
    node->kind = kind;
    node->data.as_int = data;
    node->type = type;
    node->count = 0;
    for (int i = 0; i < CHILDREN_LIMIT; i++) {
        node->child[i] = NULL;
    }
    return node;
}

void add_child(AST *parent, AST *child) {
    if (parent->count == CHILDREN_LIMIT) {
        fprintf(stderr, "Cannot add another child!\n");
        exit(1);
    }
    parent->child[parent->count] = child;
    parent->count++;
}

AST* get_child(AST *parent, int idx) {
    return parent->child[idx];
}

AST* new_subtree(NodeKind kind, Type type, int child_count, ...) {
    if (child_count > CHILDREN_LIMIT) {
        fprintf(stderr, "Too many children as arguments!\n");
        exit(1);
    }

    AST* node = new_node(kind, 0, type);
    va_list ap;
    va_start(ap, child_count);
    for (int i = 0; i < child_count; i++) {
        add_child(node, va_arg(ap, AST*));
    }
    va_end(ap);
    return node;
}

NodeKind get_kind(AST *node) {
    return node->kind;
}

int get_data(AST *node) {
    return node->data.as_int;
}

void set_float_data(AST *node, float data) {
    node->data.as_float = data;
}

float get_float_data(AST *node) {
    return node->data.as_float;
}

void set_char_data(AST *node, char data) {
    node->data.as_char = data;
}

char get_char_data(AST *node) {
    return node->data.as_char;
}

void set_st_node_data(AST *node, StNode *data) {
    node->data.as_st = data;
}

StNode *get_st_node_data(AST *node) {
    return node->data.as_st;
}

void set_vt_node_data(AST *node, VtNode *data) {
    node->data.as_vt = data;
}

VtNode *get_vt_node_data(AST *node) {
    return node->data.as_vt;
}

void set_ft_node_data(AST *node, FtNode *data) {
    node->data.as_ft = data;
}

FtNode *get_ft_node_data(AST *node) {
    return node->data.as_ft;
}

Type get_node_type(AST *node) {
    return node->type;
}

int get_child_count(AST *node) {
    return node->count;
}

void free_tree(AST *tree) {
    if (tree == NULL) return;
    for (int i = 0; i < tree->count; i++) {
        free_tree(tree->child[i]);
    }
    free(tree);
}

// Dot output.

int nr;

extern StrTable *vt;

char* kind2str(NodeKind kind) {
    switch(kind) {
        case AND_NODE:      return "&&";
        case ARGS_NODE:     return "args";
        case ASSIGN_NODE:   return "=";
        case BLOCK_NODE:    return "block";
        case BREAK_NODE:    return "break";
        case CONT_NODE:     return "continue";
        case CHAR_VAL_NODE: return "";
        case EQ_NODE:       return "==";
        case GE_NODE:       return ">=";
        case GT_NODE:       return ">";
        case IF_NODE:       return "if";
        case INT_VAL_NODE:  return "";
        case LE_NODE:       return "<=";
        case LT_NODE:       return "<";
        case MINUS_NODE:    return "-";
        case NE_NODE:       return "!=";
        case NOT_NODE:      return "!";
        case OR_NODE:       return "||";
        case OVER_NODE:     return "/";
        case PLUS_NODE:     return "+";
        case PARAM_NODE:    return "param";
        case PROGRAM_NODE:  return "program";
        case RET_NODE:      return "return";
        case FLOAT_VAL_NODE: return "";
        case FUNC_DECL_NODE: return "func_decl";
        case FUNC_USE_NODE: return "func_use";
        case STR_VAL_NODE:  return "";
        case TIMES_NODE:    return "*";
        case UNARY_MINUS_NODE: return "-";
        case VAR_DECL_NODE: return "var_decl";
        case VAR_USE_NODE:  return "var_use";
        case WHILE_NODE:    return "while";
        case C2I_NODE:      return "C2I";
        case C2F_NODE:      return "C2F";
        case I2F_NODE:      return "I2F";
        default:            return "ERROR!!";
    }
}

int has_data(NodeKind kind) {
    switch(kind) {
        case CHAR_VAL_NODE:
        case INT_VAL_NODE:
        case FLOAT_VAL_NODE:
        case STR_VAL_NODE:
        case VAR_DECL_NODE:
        case VAR_USE_NODE:
            return 1;
        default:
            return 0;
    }
}

static void print_literal(char *str)
{
    int i = 0;
    putc('\'', stderr);
    while (str[i])
    {
		switch (str[i]) {
		case '\n':
			putc('\\', stderr);
            putc('n', stderr);
			break;
		case '\t':
			putc('\\', stderr);
            putc('t', stderr);
			break;
		case '\r':
			putc('\\', stderr);
            putc('r', stderr);
			break;
		case '\\':
			putc('\\', stderr);
			putc('\\', stderr);
			break;
		default:
            putc(str[i], stderr);
		}
        i++;
    }
    putc('\'', stderr);
}

int print_node_dot(AST *node) {
    int my_nr = nr++;

    fprintf(stderr, "node%d[label=\"", my_nr);

    //printf("%s\n", kind2str(node->kind));

    if (node->type != NO_TYPE) {
        fprintf(stderr, "(%s) ", get_text(node->type));
    }
    if (node->kind == VAR_DECL_NODE || node->kind == VAR_USE_NODE) {
        fprintf(stderr, "var: %s", vt_node_get_name(node->data.as_vt));
    } else if (node->kind == FUNC_DECL_NODE || node->kind == FUNC_USE_NODE) {
        fprintf(stderr, "func: %s", ft_node_get_name(node->data.as_ft));
    } else {
        fprintf(stderr, "%s", kind2str(node->kind));
    }
    if (has_data(node->kind)) {
        if (node->kind == FLOAT_VAL_NODE) {
            fprintf(stderr, "%.2f", node->data.as_float);
        } else if (node->kind == INT_VAL_NODE) {
            fprintf(stderr, "%d", node->data.as_int);
        } else if (node->kind == CHAR_VAL_NODE) {
            fprintf(stderr, "%c", node->data.as_char);
        } else if (node->kind == STR_VAL_NODE) {
            //fprintf(stderr, "'%s'", st_node_get_name(node->data.as_st));
            print_literal(st_node_get_name(node->data.as_st));
        } else if (node->kind == VAR_DECL_NODE || node->kind == VAR_USE_NODE) {
            fprintf(stderr, " line: %d", vt_node_get_line(node->data.as_vt));
        } else if (node->kind == FUNC_DECL_NODE || node->kind == FUNC_USE_NODE) {
            fprintf(stderr, " line: %d", vt_node_get_line(node->data.as_vt));
        }
    }
    fprintf(stderr, "\"];\n");

    for (int i = 0; i < node->count; i++) {
        int child_nr = print_node_dot(node->child[i]);
        fprintf(stderr, "node%d -> node%d;\n", my_nr, child_nr);
    }
    return my_nr;
}

void print_dot(AST *tree) {
    nr = 0;
    fprintf(stderr, "digraph {\ngraph [ordering=\"out\"];\n");
    print_node_dot(tree);
    fprintf(stderr, "}\n");
}
