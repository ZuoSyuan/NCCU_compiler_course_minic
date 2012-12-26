%{
/*
C Compiler Lexer
James Van Boxtel
Compiler Design - CS 452
4/12/07
*/

#include "minic.h"
#include "y.tab.h"

void ignore_c_comments(); // ignores c style comments

//int yylineno = 1;

%}

/* This tells flex to read only one input file */
%option noyywrap

%%

 /* Single Char Tokens */
[\(\)\{\}\;\,\+\-\*\/\%\<\>\=\!\[\]\.] 	{return yytext[0];}

 /* Double Char Tokens */
\<\= 			{return LE;}
\>\=			{return GE;}
\=\= 			{return EQ;}
\!\= 			{return NE;}
\|\| 			{return OR;}
\&\& 			{return AND;}

 /* Reserved Words */
int                     {return INT_TOKEN;}
void                    {return VOID_TOKEN;}
bool                    {return BOOL_TOKEN;}
float                   {return FLOAT_TOKEN;}

if                      {return IF;}
else                    {return ELSE;}
while                   {return WHILE;}
return                  {return RETURN;}
for											{return FOR;}
new											{return NEW;}
size										{return SIZE;}


 /* Eat Whitespace */
[ \t\r]+                {;} 
[\n]			{yylineno++;}

 /* Comments */
\/\*([^*]|[\r\n]|(\*+([^\*\/]|[\r\n])))*\*+\/	{;}
\/\/.*                  {;}           		

 /* Literals */
true                    {yylval.i = 1; return BOOL_LITERAL;}
false                   {yylval.i = 0; return BOOL_LITERAL;}
[0-9]*     	 	{yylval.i = atoi(yytext); return INT_LITERAL;}
[0-9]*\.[0-9]+     	 	{yylval.f = atof(yytext); return FLOAT_LITERAL;}


 /* Identifiers */
[a-zA-Z_][a-zA-Z_0-9]*  {yylval.s = strdup(yytext); return IDENT; }

.			{yyerror("Unrecognized token");}

%%
              

