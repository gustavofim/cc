%output "parser.c"
%defines "parser.h"
%define parse.error verbose
%define parse.lac full

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "gen.h"

#include "func_table.h"
#include "str_table.h"
#include "var_table.h"

extern char yytext[];
extern int yylineno;
extern int column;

void yyerror(char const *s);
int yylex(void);
int yylex_destroy(void);

Type last_type;
Type func_type;
Type func_ret = VOID_TYPE;

extern int idn;
extern char last_id[100][200];

FuncTable *func_t;
StrTable *str_t;

int n_param = 0;
Type params[100];

int fun = 0;
int argn = -1;
int n_args[100];
Type args[100][100];
Type *func_param[100];

int check = 0; /* Used to differentiate var from func */
AST *new_var(char *name, Type type, int is_arr);
AST *new_func(char *name, Type type);
AST *check_var(char *name, int is_arr);
AST *check_func(char *name);
AST *unify_bin_node(AST* l, AST* r, NodeKind kind, const char* op, Unif (*uni)(Type,Type));
int is_decl = 0;
AST *check_assign(AST *l, AST *r);
int is_ret = 0;
AST *check_ret(AST *exp);
AST *check_arg(AST *arg);

AST *root;

int scope = 0;
int bkp = 0;
int skip = 0;
VarTable *var_ts[100];
VarTable *var_bkp[100];
void start_scope();
void end_scope();

int reg = 0;
%}
%define api.value.type {AST*}

%token IDENTIFIER INT_CONSTANT CHAR_CONSTANT FLOAT_CONSTANT STRING_LITERAL SIZEOF
%token LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP
%token SUB_ASSIGN

%token CHAR INT FLOAT VOID

%token IF ELSE WHILE CONTINUE BREAK RETURN

%precedence ')'
%precedence ELSE

%start translation_unit
%%

primary_expression
	: IDENTIFIER { check = 1; }
	| INT_CONSTANT { $$ = $1; check = 0; }
	| CHAR_CONSTANT { $$ = $1; check = 0; }
	| FLOAT_CONSTANT { $$ = $1; check = 0; }
	| STRING_LITERAL { $$ = $1; check = 0; }
	| '(' expression ')' { $$ = $2; check = 0; }
	;

postfix_expression
                         /* Checks var table if id is read */
	: primary_expression { if (check) { $$ = check_var(last_id[--idn], 0); } }
	| primary_expression {
        if (!check) {
            printf("ERROR (%d, %d): semantic error, constants are not subcriptable.\n", yylineno, column);
            exit(EXIT_FAILURE);
        }
    } '[' expression ']' { $$ = check_var(last_id[--idn], 1); add_child($$, $4); }
	| primary_expression '(' ')' { $$ = check_func(last_id[--idn]); }
	| primary_expression '(' {
        ++argn;

        FtNode *buf = NULL;
        //VtNode *temp = NULL;

        //int i = 0;
        //do {
        //    ++i;
        //    for (int j = scope; j >= 0; --j) {
        //        temp = vt_lookup(var_ts[j], last_id[idn-i]);
        //        if (temp) break;
        //    }
        //} while (temp);

        //buf = ft_lookup(func_t, last_id[idn-i]);
        do {
            buf = ft_lookup(func_t, last_id[fun++]);
        } while(!buf);

        if (!buf) {
            printf("ERROR (%d, %d): semantic error, function '%s' was not declared.---\n",
                    yylineno, column, last_id[fun]);
                    //yylineno, column, last_id[idn-i]);
            exit(EXIT_FAILURE);
        }

        func_param[argn] = ft_node_get_param(buf);


        //printf("%s\n",ft_node_get_name(buf));
        //printf("%d, %d\n", i, idn);
        //for (int i = 0; i < ft_node_get_n_param(buf); ++i)
        //    printf("%s ", get_text(func_param[argn][i]));
        //printf("\n");
    } argument_expression_list ')' { $$ = check_func(last_id[--idn]); add_child($$, $4); n_args[argn--] = 0; fun--; }
	;

argument_expression_list
	: expression {
        args[argn][n_args[argn]++] = get_node_type($1);
        $$ = new_subtree(ARGS_NODE, NO_TYPE, 1, check_arg($1));
    }
	| argument_expression_list ',' expression {
        args[argn][n_args[argn]++] = get_node_type($3);
        add_child($1, check_arg($3));
        $$ = $1;
    }
	;

unary_expression
	: postfix_expression { $$ = $1; }
	| unary_operator unary_expression {
        Unif uni = un_unify(get_node_type($1), get_node_type($2));
        AST *temp = $1;

        if (uni.type == NO_TYPE) {
            printf("ERROR (%d, %d): semantic error, cannot operate over string literal.\n", yylineno, column);
            exit(EXIT_FAILURE);
        } else if (uni.type == FLOAT_TYPE) {
            temp = new_node(get_kind(temp), 0, FLOAT_TYPE);
            free_tree($1); 
            add_child(temp, $2);
        } else {
            add_child(temp, $2);
        }
        $$ = temp;
    }
	;

unary_operator
	: '-' { $$ = new_node(UNARY_MINUS_NODE, 0, INT_TYPE); }
	| '!' { $$ = new_node(NOT_NODE, 0, INT_TYPE); }
	;

multiplicative_expression
	: unary_expression { $$ = $1; }
	| multiplicative_expression '*' unary_expression { $$ = unify_bin_node($1, $3, TIMES_NODE, "*", bin_op_unify); }
	| multiplicative_expression '/' unary_expression { $$ = unify_bin_node($1, $3, OVER_NODE, "/", bin_op_unify); }
    | multiplicative_expression '%' unary_expression { $$ = unify_bin_node($1, $3, MOD_NODE, "%", bin_op_unify); }
	;

additive_expression
	: multiplicative_expression { $$ = $1; }
	| additive_expression '+' multiplicative_expression { $$ = unify_bin_node($1, $3, PLUS_NODE, "+", bin_op_unify); }
	| additive_expression '-' multiplicative_expression { $$ = unify_bin_node($1, $3, MINUS_NODE, "-", bin_op_unify); }
	;

relational_expression
	: additive_expression { $$ = $1; }
	| relational_expression '<' additive_expression { $$ = unify_bin_node($1, $3, LT_NODE, "<", bin_op_unify); }
	| relational_expression '>' additive_expression { $$ = unify_bin_node($1, $3, GT_NODE, ">", bin_op_unify); }
	| relational_expression LE_OP additive_expression { $$ = unify_bin_node($1, $3, LE_NODE, "<=", bin_op_unify); }
	| relational_expression GE_OP additive_expression { $$ = unify_bin_node($1, $3, GE_NODE, "<=", bin_op_unify); }
	;

equality_expression
	: relational_expression { $$ = $1; }
	| equality_expression EQ_OP relational_expression { $$ = unify_bin_node($1, $3, EQ_NODE, "==", bin_op_unify); }
	| equality_expression NE_OP relational_expression { $$ = unify_bin_node($1, $3, NE_NODE, "!=", bin_op_unify); }
	;

logical_and_expression
	: equality_expression { $$ = $1; }
	| logical_and_expression AND_OP equality_expression { $$ = unify_bin_node($1, $3, AND_NODE, "&&", bin_op_unify); }
	;

logical_or_expression
	: logical_and_expression { $$ = $1; }
	| logical_or_expression OR_OP logical_and_expression { $$ = unify_bin_node($1, $3, OR_NODE, "||", bin_op_unify); }
	;

expression
	: logical_or_expression { $$ = $1; }
	| unary_expression '=' logical_or_expression { $$ = check_assign($1, $3); }
	;

init_declarator
	: direct_declarator { $$ = $1; }
	| direct_declarator '=' expression { is_decl = 1; $$ = check_assign($1, $3); is_decl = 0; }
	;

type_specifier
	: CHAR  { last_type = CHAR_TYPE; }
	| INT   { last_type = INT_TYPE; }
	| FLOAT { last_type = FLOAT_TYPE; }
    | VOID  { last_type = VOID_TYPE; }
	;

direct_declarator
	: IDENTIFIER { $$ = new_var(last_id[--idn], last_type, 0); }
	| IDENTIFIER '[' expression ']' { $$ = new_var(last_id[--idn], last_type, 1); add_child($$, $3); }
	| IDENTIFIER '(' { start_scope(); skip = 1; } parameter_list ')' { $$ = new_func(last_id[--idn], func_type); add_child($$, $4); n_param = 0; }
	| IDENTIFIER '(' ')' { func_type = last_type; $$ = new_func(last_id[--idn], last_type); }
	;

parameter_list
	: { func_type = last_type; } type_specifier  direct_declarator { $$ = new_subtree(PARAM_NODE, NO_TYPE, 1, $3); params[n_param++] = last_type; }
	| parameter_list ',' type_specifier direct_declarator { add_child($1, $4); $$ = $1; params[n_param++] = last_type; }

	;

statement
	: compound_statement { $$ = $1; }
	| expression_statement { $$ = $1; }
	| selection_statement { $$ = $1; }
	| iteration_statement { $$ = $1; }
	| jump_statement { $$ = $1; }
	;

compound_statement
	: '{' { start_scope(); } block_item_list '}' { $$ = $3; end_scope(); }
	;

block_item_list
	: block_item { $$ = new_subtree(BLOCK_NODE, NO_TYPE, 1, $1); }
	| block_item_list block_item { add_child($1, $2); $$ = $1; }
	;

block_item
	: type_specifier init_declarator ';' { $$ = $2; }
	| statement { $$ = $1; }
	;

expression_statement
	: expression ';' { $$ = $1; }
	;

selection_statement
	: IF '(' expression ')' statement { $$ = new_subtree(IF_NODE, NO_TYPE, 2, $3, $5); }
	| IF '(' expression ')' statement ELSE statement { $$ = new_subtree(IF_NODE, NO_TYPE, 3, $3, $5, $7); }
	;

iteration_statement
	: WHILE '(' expression ')' statement { $$ = new_subtree(WHILE_NODE, NO_TYPE, 2, $3, $5); }
	;

jump_statement
	: CONTINUE ';' { $$ = new_node(CONT_NODE, 0, NO_TYPE); }
	| BREAK ';' { $$ = new_node(BREAK_NODE, 0, NO_TYPE); }
	| RETURN ';' { $$ = check_ret(NULL); }
	| RETURN expression ';' { $$ = check_ret($2); }
	;

translation_unit
	: external_declaration { $$ = root = new_subtree(PROGRAM_NODE, NO_TYPE, 1, $1); }
	| translation_unit external_declaration { add_child($1, $2); $$ = $1; }
	;

external_declaration
	: type_specifier direct_declarator compound_statement {
        add_child($2, $3); $$ = $2;
        if (func_type != VOID_TYPE && !is_ret) {
        printf("ERROR (%d, %d): semantic error, non void function didn't return.\n", yylineno, column);
        exit(EXIT_FAILURE);
            
        }
        is_ret = 0;
    }
	| type_specifier init_declarator ';' { $$ = $2; }
	;

%%
AST *
new_var(char *name, Type type, int is_arr)
{
    VtNode *buf = vt_lookup(var_ts[scope], name);
    if (buf) {
        printf("ERROR (%d, %d): semantic error, variable '%s' already declared at line %d.\n",
                yylineno, column, name, vt_node_get_line(buf));
        exit(EXIT_FAILURE);
    }

    if (scope)
        vt_add(var_ts[scope], name, reg++, type, is_arr);
    else
        vt_add(var_ts[scope], name, -1, type, is_arr);

    if (scope && is_arr)
        reg++;

    AST *ret = new_node(VAR_DECL_NODE, 0, type);
    buf = vt_lookup(var_ts[scope], name);
    set_vt_node_data(ret, buf);

    return ret;
}

AST *
new_func(char *name, Type type)
{
    FtNode *buf = ft_lookup(func_t, name);
    if (buf) {
        printf("ERROR (%d, %d): semantic error, variable '%s' already declared at line %d.\n",
                yylineno, column, name, ft_node_get_line(buf));
        exit(EXIT_FAILURE);
    }

    ft_add(func_t, name, yylineno, type, n_param, params);

    AST *ret = new_node(FUNC_DECL_NODE, 0, type);
    buf = ft_lookup(func_t, name);
    set_ft_node_data(ret, buf);

    return ret;
}

AST *
check_var(char *name, int is_arr)
{
    VtNode *buf;
    int i;
    for (i = scope; i >= 0; --i) {
        buf = vt_lookup(var_ts[i], name);
        if (!buf) {
            if (i == 0) {
                printf("ERROR (%d, %d): semantic error, variable '%s' was not declared.\n",
                        yylineno, column, name);
                exit(EXIT_FAILURE);
                return NULL; /* unreachable */
            }
        } else {
            if (!vt_node_get_size(buf) && is_arr) {
                printf("ERROR (%d, %d): semantic error, variable '%s' is not subscriptable.\n",
                        yylineno, column, name);
                exit(EXIT_FAILURE);
            }
            break;
        }
    }

    AST *ret = new_node(VAR_USE_NODE, 0, vt_node_get_type(buf));
    set_vt_node_data(ret, buf);
    return ret;
}

AST *
check_func(char *name)
{
    FtNode *buf = ft_lookup(func_t, name);
    if (!buf) {
        printf("ERROR (%d, %d): semantic error, function '%s' was not declared.\n",
                yylineno, column, name);
        exit(EXIT_FAILURE);
    }

    int n = ft_node_get_n_param(buf);
    if (n_args[argn] != n) {
        printf("ERROR (%d, %d): semantic error, %d args passed to function '%s', expected %d.\n",
                yylineno, column, n_args[argn], name, n);
        exit(EXIT_FAILURE);
    }

    AST *ret = new_node(FUNC_USE_NODE, 0, ft_node_get_type(buf));
    set_ft_node_data(ret, buf);
    return ret;
}

void type_error(const char* op, Type t1, Type rt) {
    printf("ERROR (%d, %d): semantic error, incompatible types for operator '%s', LHS is '%s' and RHS is '%s'.\n",
           yylineno, column, op, get_text(t1), get_text(rt));
    exit(EXIT_FAILURE);
}

AST* create_conv_node(Conv conv, AST *n) {
    switch(conv) {
        case I2F:  return new_subtree(I2F_NODE, FLOAT_TYPE,  1, n);
        case C2I:  return new_subtree(C2I_NODE, INT_TYPE,  1, n);
        case C2F:  return new_subtree(C2F_NODE, FLOAT_TYPE, 1, n);
        case NONE: return n;
        default:
            printf("INTERNAL ERROR: invalid conversion of types!\n");
            exit(EXIT_FAILURE);
    }
}

AST* unify_bin_node(AST* l, AST* r,
                    NodeKind kind, const char* op, Unif (*unify)(Type,Type)) {
    if (!strcmp(op, "&&") || !strcmp(op, "||"))
        return new_subtree(kind, INT_TYPE, 2, l, r);

    Type lt = get_node_type(l);
    Type rt = get_node_type(r);

    Unif unif = unify(lt, rt);
    if (unif.type == NO_TYPE) {
        type_error(op, lt, rt);
    }
    l = create_conv_node(unif.lc, l);
    r = create_conv_node(unif.rc, r);

    return new_subtree(kind, unif.type, 2, l, r);
}

AST* check_assign(AST *l, AST *r) {
    Type lt = get_node_type(l);
    Type rt = get_node_type(r);

    if (rt == ANY_TYPE)
        return new_subtree(ASSIGN_NODE, NO_TYPE, 2, l, r);

    if (lt == STR_TYPE  && rt != STR_TYPE)  type_error("=", lt, rt);

    VtNode *buf = get_vt_node_data(l);
    if (vt_node_get_size(buf) && is_decl) {
        printf("ERROR (%d, %d): semantic error, arrays can't be initialized on declaration.\n", 
                yylineno, column);
        exit(EXIT_FAILURE);
    }
        
    Unif unif = bin_unify(lt, rt);
    if (unif.type == NO_TYPE) {
        type_error("=", lt, rt);
    }
    l = create_conv_node(unif.lc, l);
    r = create_conv_node(unif.rc, r);

    return new_subtree(ASSIGN_NODE, NO_TYPE, 2, l, r);
}

AST *
check_ret(AST *exp)
{
    is_ret = 1;
    if (exp)
        func_ret = get_node_type(exp);

    int err = 0;
    Unif buf;

    if (func_ret != VOID_TYPE && func_type != VOID_TYPE) {
        buf = bin_unify(func_type, func_ret);
        if (buf.type == NO_TYPE || buf.lc != NONE) {
            err = 1;
        }
    } else if (func_ret != func_type) {
        err = 1;
    }

    if (err) {
        printf("ERROR (%d, %d): semantic error, return type '%s' incompatible with function type '%s'.\n",
                yylineno, column, get_text(func_ret), get_text(func_type));
        exit(EXIT_FAILURE);
    }

    func_ret = VOID_TYPE;

    if (exp == NULL)
        return new_node(RET_NODE, 0, NO_TYPE);

    return new_subtree(RET_NODE, buf.type,  1, create_conv_node(buf.rc, exp));
}

AST *
check_arg(AST *arg)
{
    if (func_param[argn][n_args[argn]-1] == ANY_TYPE)
        return arg;

    Unif unif = bin_unify(func_param[argn][n_args[argn]-1], args[argn][n_args[argn]-1]);
    if (unif.type == NO_TYPE || unif.lc != NONE) {
        printf("ERROR (%d, %d): semantic error, argument type '%s' expected '%s'.\n",
                yylineno, column, get_text(args[argn][n_args[argn]-1]), get_text(func_param[argn][n_args[argn]-1]));
        exit(EXIT_FAILURE);
    }

    return create_conv_node(unif.rc, arg);
}

void
start_scope()
{
    /* In case scope begins in function declarator and not in compound statement */
    if (skip) {
        skip = 0;
        return;
    }
    var_ts[++scope] = var_bkp[++bkp] = vt_create();
}

void
end_scope()
{
    scope--;
}

void
yyerror(char const *s)
{
    printf("ERROR (%d, %d): %s\n", yylineno, column, s);
    exit(1);
}

int
main()
{
    var_ts[scope] = var_bkp[bkp] = vt_create();

    func_t = ft_create();
    str_t = st_create();

    /* ANY_TYPE for now indicates polymorphism (ignore type checking) */
    Type put_p[100] = { ANY_TYPE };
    ft_add(func_t, "put", 0, VOID_TYPE, 1, put_p);
    ft_add(func_t, "get", 0, ANY_TYPE, 0, NULL);

    yyparse();
    //printf("PARSE SUCCESSFUL!\n");


    /* Prints tables if CC_DOT == 1 */
    int print = 0;
    char *buf = getenv("CC_ST");
    if (buf) print = atoi(buf);
    if (print == 1) /* print symbol tables */ {
        printf("# Functions ===============\n");
        ft_print(func_t);
        printf("\n# Strings ===============\n");
        st_print(str_t);
        for (int i = 0; i <= bkp; i++) {
            printf("\n# Vars (%d) ===============\n", i);
            vt_print(var_bkp[i]);
        }
    }

    /* Generate tree, instead of bytecode, if CC_DOT == 1 */
    int gendot = 0;
    buf = getenv("CC_DOT");
    if (buf) gendot = atoi(buf);
    if (gendot == 1) print_dot(root);
    else gen_code(root);

    for (int i = 0; i <= bkp; i++)
        vt_destroy(var_bkp[i]);
    st_destroy(str_t);
    ft_destroy(func_t);
    free_tree(root);

    return 0;
}
