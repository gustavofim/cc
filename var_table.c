#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "var_table.h"

/*
    Variable table implementation using the left-leaning red-black tree
    data structure.
*/

typedef enum { BLACK, RED } Color;

struct vt_node {
    char *key;
    int line;
    Type type;
    int size;

    /* RBT stuff */
    Color color;
    VtNode *right;
    VtNode *left;
};

struct vt {
    VtNode *root;
    int size;
};

VarTable *
vt_create()
{
    VarTable *vt = malloc(sizeof(*vt));
    vt->root = NULL;
    vt->size = 0;

    return vt;
}

static int
is_red(VtNode *node)
{
    if (!node)
        return BLACK;
    return node->color;
}

static VtNode *
rotate_left(VtNode *h) {
    VtNode *x = h->right;
    h->right = x->left;
    x->left = h;
    x->color = x->left->color;
    x->left->color = RED;
    return x;
}

static VtNode*
rotate_right(VtNode *h) {
    VtNode *x = h->left;
    h->left = x->right;
    x->right = h;
    x->color = x->right->color;
    x->right->color = RED;
    return x;
}

static void
flip_colors(VtNode *h) {
    h->color = RED;
    h->left->color = BLACK;
    h->right->color = BLACK;
}

static VtNode *
node_create(VarTable *vt, char *key, int line, Type type, int size)
{
    VtNode *new_node = malloc(sizeof(*new_node));
    new_node->key = strdup(key);
    new_node->line = line;
    new_node->type = type;
    new_node->size = size;

    new_node->color = RED;
    new_node->left = NULL;
    new_node->right = NULL;

    vt->size++;

    return new_node;
}

static VtNode *
node_add(VarTable *vt, VtNode *node, char *key, int line, Type type, int size)
{
    if (!node)
        return node_create(vt, key, line, type, size);

    int cmp = strcmp(key, node->key);

    if (cmp > 0)
        node->right = node_add(vt, node->right, key, line, type, size);
    else if (cmp < 0)
        node->left = node_add(vt, node->left, key, line, type, size);

    if (is_red(node->right) && !is_red(node->left)) { node = rotate_left(node); }
    if (is_red(node->left) && is_red(node->left->left)) { node = rotate_right(node); }
    if (is_red(node->left) && is_red(node->right)) { flip_colors(node); }

    return node;
}

void
vt_add(VarTable *vt, char *key, int line, Type type, int size)
{
    key = strdup(key);

    vt->root = node_add(vt, vt->root, key, line, type, size);
    free(key);
}

static VtNode *
search(VtNode *node, char *key)
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

VtNode *
vt_lookup(VarTable *vt, char *key)
{
    key = strdup(key);
    
    VtNode *found = search(vt->root, key);
    free(key);

    if (found)
        return found;
    return NULL;
}


Type
vt_node_get_type(VtNode *nd)
{
    return nd->type;
}

int
vt_node_get_line(VtNode *nd)
{
    return nd->line;
}

char *
vt_node_get_name(VtNode *nd)
{
    return nd->key;
}

int
vt_node_get_size(VtNode *nd)
{
    return nd->size;
}

static void
print(VtNode *node, int indent)
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

    printf("[%d: (%s) %s]", node->line, get_text(node->type), node->key);

    print(node->left, indent+1);
    print(node->right, indent);
}

void
vt_print(VarTable *vt)
{
    printf("N nodes: %d\n", vt->size);
    print(vt->root, 0);
    printf("\n");
}

static void
node_destroy(VtNode *node)
{
    if (!node)
        return;

    node_destroy(node->left); 
    node_destroy(node->right); 
    free(node->key);
    free(node);
}

void
vt_destroy(VarTable *vt)
{
    node_destroy(vt->root);
    free(vt);
}
