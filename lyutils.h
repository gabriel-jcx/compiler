// $Id: lyutils.h,v 1.11 2017-10-11 14:19:04-07 - - $
// Chenxing Ji
// cji13
// Nelson Yeap
// nyeap

#ifndef __UTILS_H__
#define __UTILS_H__

// Lex and Yacc interface utility.

#include <string>
#include <vector>
using namespace std;

#include <stdio.h>

#include "astree.h"
#include "auxlib.h"

#define YYEOF 0

extern FILE* tok_file;
extern FILE* yyin;
extern char* yytext; 
extern size_t yyleng; 
extern int yy_flex_debug;

int yylex();
int yylex_destroy();
int yyparse();
void yyerror (const char* message);
int yylval_token(int symbol);
struct lexer {
   static bool interactive;
   static location lloc;
   static size_t last_yyleng;
   static vector<string> filenames;
   static const string* filename (int filenr);
   static void newfilename (const string& filename);
   static void advance();
   static void newline();
   static void badchar (unsigned char bad);
   static void badtoken (char* lexeme);
   static void include();
};

struct parser {
   static astree* root;
   static const char* get_tname (int symbol);
};

#define YYSTYPE_IS_DECLARED
typedef astree* YYSTYPE;
#include "yyparse.h"

#endif

