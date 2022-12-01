#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "func_table.h"

/*
    Function table implementation using the left-leaning red-black tree
    data structure.
*/

typedef enum { BLACK, RED } Color;

struct ft_node {
    char *key;
    int line;
    Type type;
    int n_param;
    Type params[100];

    /* RBT stuff */
    Color color;
    FtNode *right;
    FtNode *left;
};

struct ft {
    FtNode *root;
    int size;
};

FuncTable *
ft_create()
{
    FuncTable *ft = malloc(sizeof(*ft));
    ft->root = NULL;
    ft->size = 0;

    return ft;
}

static int
is_red(FtNode *node)
{
    if (!node)
        return BLACK;
    return node->color;
}

static FtNode *
rotate_left(FtNode *h) {
    FtNode *x = h->right;
    h->right = x->left;
    x->left = h;
    x->color = x->left->color;
    x->left->color = RED;
    return x;
}

static FtNode*
rotate_right(FtNode *h) {
    FtNode *x = h->left;
    h->left = x->right;
    x->right = h;
    x->color = x->right->color;
    x->right->color = RED;
    return x;
}

static void
flip_colors(FtNode *h) {
    h->color = RED;
    h->left->color = BLACK;
    h->right->color = BLACK;
}

static FtNode *
node_create(FuncTable *ft, char *key, int line, Type type, int np, Type ps[100])
{
    FtNode *new_node = malloc(sizeof(*new_node));
    new_node->key = strdup(key);
    new_node->line = line;
    new_node->type = type;

    if (np) {
        new_node->n_param = np;
        for (int i = 0; i < np; ++i)
            new_node->params[i] = ps[i];
    }

    new_node->color = RED;
    new_node->left = NULL;
    new_node->right = NULL;

    ft->size++;

    return new_node;
}

static FtNode *
node_add(FuncTable *ft, FtNode *node, char *key, int line, Type type, int np, Type ps[100])
{
    if (!node)
        return node_create(ft, key, line, type, np, ps);

    int cmp = strcmp(key, node->key);

    if (cmp > 0)
        node->right = node_add(ft, node->right, key, line, type, np, ps);
    else if (cmp < 0)
        node->left = node_add(ft, node->left, key, line, type, np, ps);

    if (is_red(node->right) && !is_red(node->left)) { node = rotate_left(node); }
    if (is_red(node->left) && is_red(node->left->left)) { node = rotate_right(node); }
    if (is_red(node->left) && is_red(node->right)) { flip_colors(node); }

    return node;
}

void
ft_add(FuncTable *ft, char *key, int line, Type type, int np, Type ps[100])
{
    key = strdup(key);

    ft->root = node_add(ft, ft->root, key, line, type, np, ps);
    free(key);
}

static FtNode *
search(FtNode *node, char *key)
{
    if (!node)
        return NULL;

    int cmp = strcmp(key, node->key);

    if (cmp > 0)
        return search(node->right, key);
    else if (cmp < 0)
        return search(node->left, key);
    else
        return node;
}

FtNode *
ft_lookup(FuncTable *ft, char *key)
{
    key = strdup(key);
    
    FtNode *found = search(ft->root, key);
    free(key);

    if (found)
        return found;
    return NULL;
}


Type
ft_node_get_type(FtNode *nd)
{
    return nd->type;
}

int
ft_node_get_line(FtNode *nd)
{
    return nd->line;
}

char *
ft_node_get_name(FtNode *nd)
{
    return nd->key;
}

int
ft_node_get_n_param(FtNode *nd)
{
    return nd->n_param;
}

Type *ft_node_get_param(FtNode *nd)
{
    return nd->params;
}

static void
print(FtNode *node, int indent)
{
    printf("\n");
    for (int i = 0; i < indent; i++)
        printf("|\t");

    if (node) {
        if (is_red(node)) {
                printf("╞══");
        } else {
                printf("├──");
        }
    } else {
        printf("└──Null");
        return;
    }

    printf("[%d: (%s) %s", node->line, get_text(node->type), node->key);
    if (node->n_param) {
        printf(" | Params: ");
        for (int i = 0; i < node->n_param; ++i)
            printf(" %s", get_text(node->params[i]));
    }
    printf("]");

    print(node->left, indent+1);
    print(node->right, indent);
}

void
ft_print(FuncTable *ft)
{
    printf("N nodes: %d\n", ft->size);
    print(ft->root, 0);
    printf("\n");
}

static void
node_destroy(FtNode *node)
{
    if (!node)
        return;

    node_destroy(node->left); 
    node_destroy(node->right); 
    free(node->key);
    free(node);
}

void
ft_destroy(FuncTable *ft)
{
    node_destroy(ft->root);
    free(ft);
}
