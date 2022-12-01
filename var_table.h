#ifndef VT_H
#define VT_H

#include "types.h"

typedef struct vt VarTable;
typedef struct vt_node VtNode;

VarTable *vt_create();
void vt_add(VarTable *vt, char *key, int line, Type type, int size);
VtNode *vt_lookup(VarTable *vt, char *key);
Type vt_node_get_type(VtNode *nd);
int vt_node_get_line(VtNode *nd);
char *vt_node_get_name(VtNode *nd);
int vt_node_get_size(VtNode *nd);
void vt_print(VarTable *vt);
void vt_destroy(VarTable *vt);

#endif
