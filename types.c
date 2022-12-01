
#include "types.h"

static const char *TYPE_STRING[] = {
    "int",
    "float",
    "char",
    "str",
    "void",
    "no_type",
    "any_type"
};

const char* get_text(Type type) {
    return TYPE_STRING[type];
}

static const Unif bo_unif[4][4] = {
    { {INT_TYPE, NONE, NONE}, {FLOAT_TYPE, I2F, NONE},  {INT_TYPE, NONE, C2I},   {NO_TYPE, NONE, NONE}  },
    { {FLOAT_TYPE, NONE, I2F}, {FLOAT_TYPE, NONE, NONE}, {FLOAT_TYPE, NONE, C2F},  {NO_TYPE, NONE, NONE}  },
    { {INT_TYPE, C2I, NONE},  {FLOAT_TYPE, C2F, NONE},  {CHAR_TYPE, NONE, NONE}, {NO_TYPE, NONE, NONE}  },
    { {NO_TYPE, NONE, NONE},  {NO_TYPE, NONE, NONE},   {NO_TYPE, NONE, NONE},   {NO_TYPE, NONE, NONE} }
};

static const Unif ba_unif[4][4] = {
    { {INT_TYPE, NONE, NONE},  {FLOAT_TYPE, I2F, NONE},  {INT_TYPE, NONE, C2I},   {NO_TYPE, NONE, NONE}  },
    { {FLOAT_TYPE, NONE, I2F}, {FLOAT_TYPE, NONE, NONE}, {FLOAT_TYPE, NONE, C2F},   {NO_TYPE, NONE, NONE}  },
    { {NO_TYPE, NONE, NONE},   {NO_TYPE, NONE, NONE},    {CHAR_TYPE, NONE, NONE}, {NO_TYPE, NONE, NONE}  },
    { {NO_TYPE, NONE, NONE},   {NO_TYPE, NONE, NONE},    {NO_TYPE, NONE, NONE},   {NO_TYPE, NONE, NONE} }
};

Unif bin_unify(Type lt, Type rt) {
    return ba_unif[lt][rt];
}

Unif bin_op_unify(Type lt, Type rt) {
    return bo_unif[lt][rt];
}

static const Unif u_unif[4] = {{INT_TYPE, NONE, NONE}, {FLOAT_TYPE, NONE, NONE}, {INT_TYPE, NONE, C2I}, {NO_TYPE, NONE, NONE}};

Unif un_unify(Type op, Type rt) {
    if (op == VOID_TYPE) {
        Unif u = {rt, NONE, NONE};
        return u;
    }
    return u_unif[rt];
}
