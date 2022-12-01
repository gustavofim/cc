#ifndef FT_H
#define FT_H

#include "types.h"

typedef struct ft FuncTable;
typedef struct ft_node FtNode;

FuncTable *ft_create();
void ft_add(FuncTable *ft, char *key, int line, Type type, int np, Type ps[100]);
FtNode *ft_lookup(FuncTable *ft, char *key);
Type ft_node_get_type(FtNode *nd);
int ft_node_get_line(FtNode *nd);
char *ft_node_get_name(FtNode *nd);
int ft_node_get_n_param(FtNode *nd);
Type *ft_node_get_param(FtNode *nd);
void ft_print(FuncTable *ft);
void ft_destroy(FuncTable *ft);

#endif
