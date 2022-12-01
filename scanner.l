%option yylineno
%option outfile="scanner.c"
%option noyywrap
%option nounput

D   [0-9]
L   [a-zA-Z_]

%{
#include <stdio.h>
#include <string.h>

#include "ast.h"

#include "parser.h"

#include "str_table.h"

extern StrTable *str_t;

int column = 0;

int idn = 0;
char last_id[100][200];

void count();
void comment();
%}

%%
"/*"			{ comment(); }
"//"[^\n]*      { /* consume //-comment */ }


"break"			{ count(); return(BREAK); }
"char"			{ count(); return(CHAR); }
"continue"		{ count(); return(CONTINUE); }
"else"			{ count(); return(ELSE); }
"float"			{ count(); return(FLOAT); }
"if"			{ count(); return(IF); }
"int"			{ count(); return(INT); }
"void"			{ count(); return(VOID); }
"return"		{ count(); return(RETURN); }
"sizeof"		{ count(); return(SIZEOF); }
"while"			{ count(); return(WHILE); }

{L}({L}|{D})*		        { count(); strcpy(last_id[idn++], yytext); return(IDENTIFIER); }

0[0-7]*		                { count(); yylval = new_node(INT_VAL_NODE, atoi(yytext), INT_TYPE); return(INT_CONSTANT); } // int
[1-9]{D}*		            { count(); yylval = new_node(INT_VAL_NODE, atoi(yytext), INT_TYPE); return(INT_CONSTANT); } // int
'(\\.|[^\\'\n])+'	        {
                                count();
                                yylval = new_node(CHAR_VAL_NODE, 0, CHAR_TYPE);
                                set_char_data(yylval, yytext[1]);
                                return(CHAR_CONSTANT);
                            } // char
{D}*"."{D}+     	        {
                                count();
                                yylval = new_node(FLOAT_VAL_NODE, 0, FLOAT_TYPE);
                                set_float_data(yylval, (float)atof(yytext));
                                return(FLOAT_CONSTANT);
                            } // real
{D}+"."{D}*     	        {
                                count();
                                yylval = new_node(FLOAT_VAL_NODE, 0, FLOAT_TYPE);
                                set_float_data(yylval, (float)atof(yytext));
                                return(FLOAT_CONSTANT);
                            } // real

\"(\\.|[^\\"\n])*\"	    {
                            count();
                            char *buf = yytext + 1;
                            buf[strlen(buf) - 1] = '\0';
                            st_add(str_t, buf);
                            yylval = new_node(STR_VAL_NODE, 0, STR_TYPE);
                            set_st_node_data(yylval, st_lookup(str_t, buf));
                            return(STRING_LITERAL);
                        }
    

"&&"			{ count(); return(AND_OP); }
"||"			{ count(); return(OR_OP); }
"<="			{ count(); return(LE_OP); }
">="			{ count(); return(GE_OP); }
"=="			{ count(); return(EQ_OP); }
"!="			{ count(); return(NE_OP); }
";"			    { count(); return(';'); }
("{"|"<%")		{ count(); return('{'); }
("}"|"%>")		{ count(); return('}'); }
","			    { count(); return(','); }
"="			    { count(); return('='); }
"("			    { count(); return('('); }
")"			    { count(); return(')'); }
("["|"<:")		{ count(); return('['); }
("]"|":>")		{ count(); return(']'); }
"&"			    { count(); return('&'); }
"!"			    { count(); return('!'); }
"-"			    { count(); return('-'); }
"+"			    { count(); return('+'); }
"*"			    { count(); return('*'); }
"/"			    { count(); return('/'); }
"%"			    { count(); return('%'); }
"<"			    { count(); return('<'); }
">"			    { count(); return('>'); }

[ \t\v\r\n\f]   { count(); }
.			    {
    printf("ERROR (%d, %d): lexical error, unknown symbol %s\n", yylineno, column, yytext);
    exit(1);
}

%%
void
comment()
{
	char c, prev = 0;
  
	while ((c = input()) != 0)      /* (EOF maps to 0) */
	{
		if (c == '/' && prev == '*')
			return;
		prev = c;
	}
	printf("Unterminated comment");
    exit(1);
}

void
count()
{
	int i;

	for (i = 0; yytext[i] != '\0'; i++)
		if (yytext[i] == '\n')
			column = 0;
		else if (yytext[i] == '\t')
			column += 8 - (column % 8);
		else
			column++;
}
