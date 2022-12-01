
#ifndef TYPES_H
#define TYPES_H

typedef enum {
    INT_TYPE,
    FLOAT_TYPE,
    CHAR_TYPE,
    STR_TYPE,
    VOID_TYPE,
    NO_TYPE, // Used when we need to pass a non-existing type to a function.
    ANY_TYPE
} Type;

const char* get_text(Type type);

typedef enum {  // Basic conversions between types.
    I2F,
    C2I,
    C2F,
    NONE,
} Conv;

typedef struct {
    Type type;
    Conv lc;  // Left conversion.
    Conv rc;  // Right conversion.
} Unif;

Unif bin_unify(Type lt, Type rt);
Unif bin_op_unify(Type lt, Type rt);
Unif un_unify(Type op, Type rt);

#endif // TYPES_H

