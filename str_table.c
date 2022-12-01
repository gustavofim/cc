#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "str_table.h"

/*
    String table implementation using the left-leaning red-black tree
    data structure.
*/

typedef enum { BLACK, RED } Color;

struct st_node {
    char *key;

    /* RBT stuff */
    Color color;
    StNode *right;
    StNode *left;
};

struct st {
    StNode *root;
    int size;
};

StrTable *
st_create()
{
    StrTable *st = malloc(sizeof(*st));
    st->root = NULL;

    return st;
}

static int
is_red(StNode *node)
{
    if (!node)
        return BLACK;
    return node->color;
}

static StNode *
rotate_left(StNode *h) {
    StNode *x = h->right;
    h->right = x->left;
    x->left = h;
    x->color = x->left->color;
    x->left->color = RED;
    return x;
}

static StNode*
rotate_right(StNode *h) {
    StNode *x = h->left;
    h->left = x->right;
    x->right = h;
    x->color = x->right->color;
    x->right->color = RED;
    return x;
}

static void
flip_colors(StNode *h) {
    h->color = RED;
    h->left->color = BLACK;
    h->right->color = BLACK;
}

static StNode *
node_create(StrTable *st, char *key)
{
    StNode *new_node = malloc(sizeof(*new_node));
    new_node->key = strdup(key);

    new_node->color = RED;
    new_node->left = NULL;
    new_node->right = NULL;

    st->size++;

    return new_node;
}

static StNode *
node_add(StrTable *st, StNode *node, char *key)
{
    if (!node)
        return node_create(st, key);

    int cmp = strcmp(key, node->key);

    if (cmp > 0)
        node->right = node_add(st, node->right, key);
    else if (cmp < 0)
        node->left = node_add(st, node->left, key);

    if (is_red(node->right) && !is_red(node->left)) { node = rotate_left(node); }
    if (is_red(node->left) && is_red(node->left->left)) { node = rotate_right(node); }
    if (is_red(node->left) && is_red(node->right)) { flip_colors(node); }

    return node;
}

void
st_add(StrTable *st, char *key)
{
    key = strdup(key);

    st->root = node_add(st, st->root, key);
    free(key);
}

static StNode *
search(StNode *node, char *key)
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

StNode *
st_lookup(StrTable *st, char *key)
{
    key = strdup(key);
    
    StNode *found = search(st->root, key);
    free(key);

    if (found)
        return found;
    return NULL;
}

char *
st_node_get_name(StNode *nd)
{
    return nd->key;
}

static void
print(StNode *node, int indent)
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

    printf("[%s]", node->key);

    print(node->left, indent+1);
    print(node->right, indent);
}

void
st_print(StrTable *st)
{
    printf("N nodes: %d\n", st->size);
    print(st->root, 0);
    printf("\n");
}

static void
node_destroy(StNode *node)
{
    if (!node)
        return;

    node_destroy(node->left); 
    node_destroy(node->right); 
    free(node->key);
    free(node);
}

void
st_destroy(StrTable *st)
{
    node_destroy(st->root);
    free(st);
}
