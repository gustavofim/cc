#include <stdio.h>
#include <string.h>

#include "gen.h"

/* Counters for labels */
int whilec = 0;
int ifc = 0;
int gotoc = 0;

void write(AST *node);

void
write_all(AST* node)
{
    int childc = get_child_count(node);
    for (int i = 0; i < childc; ++i) {
        write(get_child(node, i));
    }
}

int returned = 0;

void
write_func(AST *node)
{
    FtNode *func = get_ft_node_data(node);
    char *fname = ft_node_get_name(func);
    Type ftype = ft_node_get_type(func);
    AST *child = get_child(node, 0);
    printf(".method public static %s", fname);
    /* JVM main has fixed parameters and type */
    if (!strcmp(fname, "main")) {
        puts("([Ljava/lang/String;)V");
        puts("\t.limit locals 100");
        puts("\t.limit stack 100\n");
        int childc = get_child_count(child);
        AST *buf;
        for (int i = 0; i < childc; ++i) {
            buf = get_child(child, i);
            if (get_kind(buf) != RET_NODE)
                write(get_child(child, i));
        }
        puts("\treturn");
    } else {
        Type *ptypes = ft_node_get_param(func);
        int pcount = ft_node_get_n_param(func);
        int childc = get_child_count(node);
        AST *param;
        if (childc > 1)
            param = child;
        printf("(");
        for (int i = 0; i < pcount; ++i) {
            int arr = vt_node_get_size(get_vt_node_data(get_child(param, i)));
            if (arr)
                printf("[");
            if (ptypes[i] == INT_TYPE || (!arr && ptypes[i] == CHAR_TYPE)) {
                printf("I");
            } else if (ptypes[i] == FLOAT_TYPE) {
                printf("F");
            } else if (ptypes[i] == CHAR_TYPE) {
                printf("C");
            }
        }
        printf(")");
        if (ftype == INT_TYPE || ftype == CHAR_TYPE) {
            puts("I");
        } else if (ftype == FLOAT_TYPE) {
            puts("F");
        } else if (ftype == CHAR_TYPE) {
            puts("C");
        } else {
            puts("V");
        }
        puts("\t.limit locals 100");
        puts("\t.limit stack 100\n");
        if (childc > 1)
            write(get_child(node, 1));
        else
            write(child);
        if (!returned) {
            puts("\treturn");
        }
        returned = 0;
    }
    puts(".end method\n");
}

Type assign_type;
/* Boolean operations between non ints returns ints.
 * This is a hacky way to track if that's the case
 * without messing with the type checker.
 */
int intify = 0;

void
write_func_use(AST *node)
{
    FtNode *func = get_ft_node_data(node);
    char *fname = ft_node_get_name(func);
    AST *args = get_child(node, 0);
    if (!strcmp(fname, "put")) {
        /* Printing to stdout: */
        AST *arg = get_child(args, 0);
        Type argt = get_node_type(arg);
        int arr = 0;
        if (get_kind(arg) == VAR_USE_NODE)
            arr = vt_node_get_size(get_vt_node_data(arg)) && !get_child_count(arg);
        write(arg);
        if (intify)
            argt = INT_TYPE;
        intify = 0;
        puts("\tgetstatic java/lang/System/out Ljava/io/PrintStream;");
        puts("\tswap");
        printf("\tinvokevirtual java/io/PrintStream/println");
        printf("(");
        if (arr)
            printf("[");
        if (argt == INT_TYPE) {
            puts("I)V\n");
        } else if (argt == FLOAT_TYPE) {
            puts("F)V\n");
        } else if (argt == CHAR_TYPE) {
            puts("C)V\n");
        } else {
            puts("Ljava/lang/String;)V\n");
        }
        return;
    } else if (!strcmp(fname, "get")) {
        /* Reading from stdin.... */
        if (assign_type != CHAR_TYPE) {
            puts("\tnew java/util/Scanner");
            puts("\tdup");
            puts("\tgetstatic java/lang/System/in Ljava/io/InputStream;");
            puts("\tinvokespecial java/util/Scanner/<init>(Ljava/io/InputStream;)V");
            if (assign_type == INT_TYPE) {
                puts("\tinvokevirtual java/util/Scanner/nextInt()I\n");
            } else if (assign_type == FLOAT_TYPE) {
                puts("\tinvokevirtual java/util/Scanner/nextFloat()F\n");
            } else {
                puts("\tinvokevirtual java/util/Scanner/nextLine()Ljava/lang/String;\n");
            }
        } else {
            /* Reads a char */
            puts("\tgetstatic java/lang/System/in Ljava/io/InputStream;");
            puts("\tinvokevirtual java/io/InputStream/read()I\n");
            /* Reads ... */
            puts("\tgetstatic java/lang/System/in Ljava/io/InputStream;");
            puts("\tinvokevirtual java/io/InputStream/read()I\n");
            /* ... and then pops the \n */
            puts("\tpop");
        }
    } else {
        Type ftype = ft_node_get_type(func);
        Type *atypes = ft_node_get_param(func);
        int acount = ft_node_get_n_param(func);
        int arr[acount];
        if (acount) {
            /* Setting args: */
            write(args);
            AST *arg;
            for (int i = 0; i < acount; ++i) {
               arg = get_child(args, i); 
               if (get_kind(arg) == VAR_USE_NODE
                   && vt_node_get_size(get_vt_node_data(arg))
                   && !get_child_count(arg)) {
                   arr[i] = 1;
               } else {
                   arr[i] = 0;
               }
            }
        }
        /* Calling %s function: */
        printf("\tinvokestatic Program/%s(", fname);
        for (int i = 0; i < acount; ++i) {
            if (arr[i])
                printf("[");
            if (atypes[i] == INT_TYPE || (!arr[i] && atypes[i] == CHAR_TYPE)) {
                printf("I");
            } else if (atypes[i] == FLOAT_TYPE) {
                printf("F");
            } else if (atypes[i] == CHAR_TYPE) {
                printf("C");
            } else if (atypes[i] == STR_TYPE) {
                printf("Ljava/lang/String;");
            }
        }
        printf(")");

        if (ftype == INT_TYPE || ftype == CHAR_TYPE) {
            puts("I");
        } else if (ftype == FLOAT_TYPE) {
            puts("F");
        } else if (ftype == CHAR_TYPE) {
            puts("C");
        } else if (ftype == STR_TYPE) {
            puts("Ljava/lang/String;");
        } else {
            puts("V");
        }
    }
}

void
write_str(AST *node)
{
    StNode *strnd = get_st_node_data(node);
    char *str = st_node_get_name(strnd);
    printf("\tldc \"%s\"\n", str);
}

void
write_ret(AST *node)
{
    returned = 1;
    AST *child = get_child(node, 0);
    if (!child) {
        puts("return");
        return;
    }
    write(child);
    Type t = get_node_type(node);
    if (t == INT_TYPE || t == CHAR_TYPE) {
        puts("\tireturn");
    } else if (t == FLOAT_TYPE) {
        puts("\tfreturn");
    }
}

void
write_int(AST *node)
{
    int val = get_data(node);
    printf("\tldc %d\n", val);
}

void
write_var(AST *node)
{
    VtNode *var = get_vt_node_data(node);
    int reg = vt_node_get_line(var);
    Type t = get_node_type(node);
    if (!vt_node_get_size(var)) {
        if (reg < 0) {
            printf("\tputstatic Program/%s ", vt_node_get_name(var));
            if (t == INT_TYPE || t == CHAR_TYPE) {
                puts("I");
            } else if (t == FLOAT_TYPE) {
                puts("F");
            }
        } else {
            if (t == INT_TYPE || t == CHAR_TYPE) {
                printf("\tistore %d\n", reg);
            } else if (t == FLOAT_TYPE) {
                printf("\tfstore %d\n", reg);
            }
        }
    } else {
        if (reg < 0) {
            printf("\tgetstatic Program/%s ", vt_node_get_name(var));
            if (t == INT_TYPE) {
                puts("[I");
            } else if (t == CHAR_TYPE) {
                puts("[C");
            } else if (t == FLOAT_TYPE) {
                puts("[F");
            }
        } else {
            printf("\taload %d\n", reg);
        }
        puts("\tswap");
        AST *child = get_child(node, 0);
        if (child)
            write(child);
        puts("\tswap");
        if (t == INT_TYPE) {
            puts("\tiastore");
        } else if (t == FLOAT_TYPE) {
            puts("\tfastore");
        } else if (t == CHAR_TYPE) {
            puts("\tcastore");
        } else if (t == STR_TYPE) {
            puts("\taastore");
        }
    }
    /* Write var \'%s\' -> local[%d]\n */
}

int gn = -1;
AST *globals[50];

void
write_var_decl(AST *node)
{
    VtNode *var = get_vt_node_data(node);
    Type t = vt_node_get_type(var);
    int reg = vt_node_get_line(var);
    int sz = vt_node_get_size(var);
    /* Nothing to do if it's not an array */
    if (!sz && reg >= 0)
        return;
    if (reg < 0) {
        char *name = vt_node_get_name(var);
        printf(".field public static %s ", name);
        int arr = vt_node_get_size(var);
        if (arr) {
            printf("[");
            globals[++gn] = node;
        }
        if (t == INT_TYPE || (!arr && t == CHAR_TYPE)) {
            puts("I\n");
        } else if (t == FLOAT_TYPE) {
            puts("F\n");
        } else if (t == CHAR_TYPE) {
            puts("C\n");
        }
    } else {
        write(get_child(node, 0));
        printf("\tistore %d\n", reg+1);
        printf("\tiload %d\n", reg+1);
        /* Store array and its size on locals reg and reg+1 */
        printf("\tnewarray ");
        if (t == INT_TYPE) {
            puts("int");
        } else if (t == FLOAT_TYPE) {
            puts("float");
        } else if (t == CHAR_TYPE) {
            puts("char");
        }
        printf("\tastore %d\n\n", reg);
    }
}

void
write_var_use(AST *node)
{
    VtNode *var = get_vt_node_data(node);
    int reg = vt_node_get_line(var);
    Type t = get_node_type(node);
    if (!vt_node_get_size(var)) {
        if (reg < 0) {
            printf("\tgetstatic Program/%s ", vt_node_get_name(var));
            if (t == INT_TYPE || t == CHAR_TYPE) {
                puts("I");
            } else if (t == FLOAT_TYPE) {
                puts("F");
            }
        } else {
            if (t == INT_TYPE || t == CHAR_TYPE) {
                printf("\tiload %d\n", reg);
            } else if (t == FLOAT_TYPE) {
                printf("\tfload %d\n", reg);
            }
        }
    } else {
        AST *child = get_child(node, 0);
        if (reg < 0) {
            printf("\tgetstatic Program/%s ", vt_node_get_name(var));
            if (t == INT_TYPE) {
                puts("[I");
            } else if (t == CHAR_TYPE) {
                puts("[C");
            } else if (t == FLOAT_TYPE) {
                puts("[F");
            }
            if (child) {
                write(child);
                if (t == INT_TYPE) {
                    puts("\tiaload");
                } else if (t == FLOAT_TYPE) {
                    puts("\tfaload");
                } else if (t == CHAR_TYPE) {
                    puts("\tcaload");
                } else if (t == STR_TYPE) {
                    puts("\taaload");
                }
            }
        } else {
            printf("\taload %d\n", reg);
            if (child) {
                write(child);
                if (t == INT_TYPE) {
                    puts("\tiaload");
                } else if (t == FLOAT_TYPE) {
                    puts("\tfaload");
                } else if (t == CHAR_TYPE) {
                    puts("\tcaload");
                } else if (t == STR_TYPE) {
                    puts("\taaload");
                }
            }
        }
    }
    /* Read var \'%s\' <- local[%d]\n */
}

void
write_assign(AST *node)
{
    AST* child = get_child(node, 0);
    /* Used to determine the type to be read by get() */
    assign_type = get_node_type(child);
    VtNode *cd = get_vt_node_data(child);
    if (get_kind(child) == VAR_DECL_NODE && vt_node_get_line(cd) < 0) {
        write(child);
        globals[++gn] = node;
        return;
    }
    write(get_child(node, 1));
    write_var(child);
    printf("\n");
}

void
write_float(AST *node)
{
    float val = get_float_data(node);
    printf("\tldc %.2f\n", val);
}

void
write_i2f(AST *node)
{
    write(get_child(node, 0));
    puts("\ti2f");
}

char *ops[6][2] = {
    { "\tiadd\n", "\tfadd\n" },
    { "\tisub\n", "\tfsub\n" },
    { "\timul\n", "\tfmul\n" },
    { "\tidiv\n", "\tfdiv\n" },
    { "\tirem\n", "\tfrem\n" },
    { "\tineg\n", "\tfneg\n" },
};

void
write_op(AST *node)
{
    int childc = get_child_count(node);
    Type t = get_node_type(node);
    NodeKind k = get_kind(node);
    for (int i = 0; i < childc; ++i)
        write(get_child(node, i));
    puts(ops[k-6][t]);
}

void
write_bin_bool(AST *node)
{
    int childc = get_child_count(node);
    AST *child;
    NodeKind k = get_kind(node);
    Type t;
    /* Ugly conversion to 1s and 0s */
    for (int i = 0; i < childc; ++i) {
        int l1 = gotoc++, l2 = gotoc++;
        child = get_child(node, i);
        t = get_node_type(child);
        write(child);
        if (t == FLOAT_TYPE && !intify) {
            puts("\tldc 0.0");
            puts("\tfcmpg");
        }
        intify = 0;
        printf("\tifeq label%d\n", l1);
        puts("\tldc 1");
        printf("\tgoto label%d\n", l2);
        printf("label%d:\n", l1);
        puts("\tldc 0");
        printf("label%d:\n", l2);
    }
    if (k == AND_NODE)
        puts("\tiand\n");
    else
        puts("\tior\n");
}

char *cmp[6][2] = {
    { "\tif_icmpeq label", "\tfcmpg\n\tifeq label" },
    { "\tif_icmpne label", "\tfcmpg\n\tifne label" },
    { "\tif_icmpgt label", "\tfcmpg\n\tifgt label" },
    { "\tif_icmplt label", "\tfcmpg\n\tiflt label" },
    { "\tif_icmpge label", "\tfcmpg\n\tifge label" },
    { "\tif_icmple label", "\tfcmpg\n\tifle label" },
};

void
write_cmp(AST *node)
{
    int childc = get_child_count(node);
    Type t = get_node_type(node);
    NodeKind k = get_kind(node);
    intify = 1;
    for (int i = 0; i < childc; ++i)
        write(get_child(node, i));
    int l1 = gotoc++, l2 = gotoc++;
    printf("%s%d\n", cmp[k][t], l1);
    puts("\tldc 0");
    printf("\tgoto label%d\n", l2);
    printf("label%d:\n", l1);
    puts("\tldc 1");
    printf("label%d:\n\n", l2);
}

void
write_not(AST *node)
{
    Type t = get_node_type(node);
    write(get_child(node, 0));
    if (intify)
        t = INT_TYPE;
    intify = 1;
    /* Negating value */
    int l1 = gotoc++, l2 = gotoc++;
     if (t == FLOAT_TYPE) {
        puts("\tldc 0.0");
        puts("\tfcmpg");
    }
    printf("\tifeq label%d\n", l1);
    puts("\tldc 0");
    printf("\tgoto label%d\n", l2);
    printf("label%d:\n", l1);
    puts("\tldc 1");
    printf("label%d:\n", l2);
}

void
write_char(AST *node)
{
    int val = get_char_data(node);
    printf("\tldc %d\n", val);
}

int iw = -1;
int whiles[50];

void
write_while(AST *node)
{
    int l = whilec++;
    whiles[++iw] = l;
    printf("while%d:\n", l);
    AST *child = get_child(node, 0);
    //NodeKind t = get_kind(child);
    Type t = get_node_type(child);
    write(child);
    if (t == FLOAT_TYPE) {
        int l1 = gotoc++, l2 = gotoc++;
        puts("\tldc 0.0");
        puts("\tfcmpg");
        printf("\tifeq label%d\n", l1);
        puts("\tldc 1");
        printf("\tgoto label%d\n", l2);
        printf("label%d:\n", l1);
        puts("\tldc 0");
        printf("label%d:\n", l2);
    } else if (t == STR_TYPE) {
        puts("\tpop");
        puts("\tldc 1");
    } else {
        intify = 0;
    }
    printf("\tifeq endwhile%d\n", l);
    write(get_child(node, 1));
    printf("\tgoto while%d\n", l);
    printf("endwhile%d:\n\n", l);
    iw--;
}

void
write_if(AST *node)
{
    int l = ifc++;
    AST *child = get_child(node, 0);
    int childc = get_child_count(node);
    Type t = get_node_type(child);
    NodeKind k = get_kind(child);
    write(child);
    if (t == FLOAT_TYPE && !intify) {
        int l1 = gotoc++, l2 = gotoc++;
        puts("\tldc 0.0");
        puts("\tfcmpg");
        printf("\tifeq label%d\n", l1);
        puts("\tldc 1");
        printf("\tgoto label%d\n", l2);
        printf("label%d:\n", l1);
        puts("\tldc 0");
        printf("label%d:\n", l2);
    } else if (k == STR_VAL_NODE) {
        puts("\tpop");
        puts("\tldc 1");
    } else {
        intify = 0;
    }
    /* If-then-else */
    if (childc == 3) {
        printf("\tifeq else%d\n", l);
        write(get_child(node, 1));
        printf("\tgoto endif%d\n", l);
        printf("else%d:\n", l);
        write(get_child(node, 2));
        printf("endif%d:\n\n", l);
    /* If-then */
    } else {
        printf("\tifeq endif%d\n", l);
        write(get_child(node, 1));
        printf("endif%d:\n\n", l);
    }
}

void
write_break(AST *node)
{
    printf("\tgoto endwhile%d\n", whiles[iw]);
}

void
write_cont(AST *node)
{
    printf("\tgoto while%d\n", whiles[iw]);
}

void
write(AST *node)
{
    switch (get_kind(node)) {
    case AND_NODE:
        write_bin_bool(node);
        break;
    case ARGS_NODE:
        write_all(node);
        break;
    case ASSIGN_NODE:
        write_assign(node);
        break;
    case BLOCK_NODE:
        write_all(node);
        break;
    case BREAK_NODE:
        write_break(node);
        break;
    case CONT_NODE:
        write_cont(node);
        break;
    case CHAR_VAL_NODE:
        write_char(node);
        break;
    case EQ_NODE:
        write_cmp(node);
        break;
    case GE_NODE:
        write_cmp(node);
        break;
    case GT_NODE:
        write_cmp(node);
        break;
    case IF_NODE:
        write_if(node);
        break;
    case INT_VAL_NODE:
        write_int(node);
        break;
    case LE_NODE:
        write_cmp(node);
        break;
    case LT_NODE:
        write_cmp(node);
        break;
    case MINUS_NODE:
        write_op(node);
        break;
    case MOD_NODE:
        write_op(node);
        break;
    case NE_NODE:
        write_cmp(node);
        break;
    case NOT_NODE:
        write_not(node);
        break;
    case OR_NODE:
        write_bin_bool(node);
        break;
    case OVER_NODE:
        write_op(node);
        break;
    case PLUS_NODE:
        write_op(node);
        break;
    case PROGRAM_NODE:
        write_all(node);
        break;
    case RET_NODE:
        write_ret(node);
        break;
    case FLOAT_VAL_NODE:
        write_float(node);
        break;
    case FUNC_DECL_NODE:
        write_func(node);
        break;
    case FUNC_USE_NODE:
        write_func_use(node);
        break;
    case STR_VAL_NODE:
        write_str(node);
        break;
    case TIMES_NODE:
        write_op(node);
        break;
    case UNARY_MINUS_NODE:
        write_op(node);
        break;
    case VAR_DECL_NODE:
        write_var_decl(node);
        break;
    case VAR_USE_NODE:
        write_var_use(node);
        break;
    case WHILE_NODE:
        write_while(node);
        break;
    case I2F_NODE:
        write_i2f(node);
        break;
    case C2I_NODE:
        write(get_child(node, 0));
        break;
    case C2F_NODE:
        write(get_child(node, 0));
        puts("\ti2f");
        break;
    default:
        break;
    }
}

void
gen_code(AST *root)
{
    puts(".class public Program");
    puts(".super java/lang/Object\n");
    write(root);
    /* Globals initialization */
    if (gn >= 0) {
        puts(".method static <clinit>()V");
        puts("\t.limit locals 100");
        puts("\t.limit stack 100\n");
        for (int i = 0; i <= gn; ++i) {
            Type t = get_node_type(globals[i]);
            if (get_kind(globals[i]) == VAR_DECL_NODE) {
                write(get_child(globals[i], 0));
                printf("\tnewarray ");
                if (t == INT_TYPE) {
                    puts("int");
                } else if (t == FLOAT_TYPE) {
                    puts("float");
                } else {
                    puts("char");
                }
                printf("\tputstatic Program/%s ",
                        vt_node_get_name(get_vt_node_data(globals[i])));
                if (t == INT_TYPE) {
                    puts("[I\n");
                } else if (t == CHAR_TYPE) {
                    puts("[C\n");
                } else if (t == FLOAT_TYPE) {
                    puts("[F\n");
                }
            } else {
                VtNode *global = get_vt_node_data(get_child(globals[i], 0));
                t = vt_node_get_type(global);
                write(get_child(globals[i], 1));
                printf("\tputstatic Program/%s ", vt_node_get_name(global));
                if (t == INT_TYPE || t == CHAR_TYPE) {
                    puts("I\n");
                } else if (t == FLOAT_TYPE) {
                    puts("F\n");
                }
            }
        }
        puts("\treturn");
        puts(".end method");
    }
}

