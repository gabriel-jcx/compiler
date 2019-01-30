%{
#include "lyutils.h"
#define YY_USER_ACTION { lexer::advance(); }
%}

%option 8bit
%option debug
%option nodefault
%option nounput
%option noyywrap
%option verbose
%option warn

LETTER         [A-Za-z_]
DIGIT          [0-9]
NUMBER         ({DIGIT}*)

PREPROC        (#{NUMBER} {STRING})
IDENT          ({LETTER}({LETTER}|{DIGIT})*)
BADIDENT       ({DIGIT}({LETTER}|{DIGIT})*)
CHAR           ('([^\\'\n]|\\[\\'"0nt])')
BADCHAR        (\"([^\\"\n]|\\[\\'"0nt])*)
STRING         (\"([^\\"\n]|\\[\\'"0nt])*\")
BADSTRING      (\"([^\\"\n]|\\[\\'"0nt])*(\\)([^\\"\n]|\\[\\'"0nti])*\")
COMMENT        ("//".*)
%%

"#".*       { lexer::include();}
[ \t]+      { }
\n          { lexer::newline(); }


"+"         { return yylval_token ('+'); }
"-"         { return yylval_token ('-'); }
"*"         { return yylval_token ('*'); }
"/"         { return yylval_token ('/'); }
"%"         { return yylval_token ('%'); }
"!"         { return yylval_token ('!'); }
"="         { return yylval_token ('='); }
"=="        { return yylval_token (TOK_EQ); }
"!="        { return yylval_token (TOK_NE); }
"<"         { return yylval_token (TOK_LT); }
"<="        { return yylval_token (TOK_LE); }
">"         { return yylval_token (TOK_GT); }
">="        { return yylval_token (TOK_GE); }
"->"        { return yylval_token (TOK_ARROW); }
","         { return yylval_token (','); }
";"         { return yylval_token (';'); }
"("         { return yylval_token ('('); }
")"         { return yylval_token (')'); }
"["         { return yylval_token ('['); }
"]"         { return yylval_token (']'); }
"{"         { return yylval_token ('{'); }
"}"         { return yylval_token ('}'); }
"[]"        { return yylval_token (TOK_ARRAY); }


"char"      { return yylval_token (TOK_CHAR); }
"if"        { return yylval_token (TOK_IF); }
"else"      { return yylval_token (TOK_ELSE); }
"while"     { return yylval_token (TOK_WHILE); }
"return"    { return yylval_token (TOK_RETURN); }
"int"       { return yylval_token (TOK_INT); }
"string"    { return yylval_token (TOK_STRING); }
"struct"    { return yylval_token (TOK_STRUCT); }
"void"      { return yylval_token (TOK_VOID); }
"new"       { return yylval_token (TOK_NEW); }
"null"      { return yylval_token (TOK_NULL); }
"bool"      { return yylval_token (TOK_BOOL); }
"not"       { return yylval_token ('!');     }


{CHAR}      { return yylval_token(TOK_CHARCON); }
{STRING}    { return yylval_token(TOK_STRINGCON); }
{NUMBER}    { return yylval_token(TOK_INTCON); }
{IDENT}     { return yylval_token(TOK_IDENT); }
{BADIDENT}  { lexer::badtoken(yytext); }
{BADSTRING} { lexer::badtoken(yytext); }
{BADCHAR}   { lexer::badtoken(yytext); }
{COMMENT}   {}



.           { lexer::badchar(*yytext); }

%%

