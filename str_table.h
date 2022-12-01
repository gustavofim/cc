#ifndef ST_H
#define ST_H

#include "types.h"

typedef struct st StrTable;
typedef struct st_node StNode;

StrTable *st_create();
void st_add(StrTable *st, char *key);
StNode *st_lookup(StrTable *st, char *key);
char *st_node_get_name(StNode *nd);
void st_print(StrTable *st);
void st_destroy(StrTable *st);

#endif
