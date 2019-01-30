%{
// $Id: parser.y,v 1.15 2018-04-06 15:14:40-07 - - $
// Dummy parser for scanner project.

#include <cassert>
#include "lyutils.h"
#include "astree.h"


%}

%debug
%defines
%error-verbose
%token-table
%verbose

%token TOK_VOID TOK_CHAR TOK_INT TOK_STRING TOK_BOOL
%token TOK_IF TOK_ELSE TOK_WHILE TOK_RETURN TOK_STRUCT
%token TOK_NULL TOK_NEW TOK_ARRAY TOK_ARROW
%token TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%token TOK_IDENT TOK_INTCON TOK_CHARCON TOK_STRINGCON

%token TOK_ROOT TOK_BLOCK TOK_CALL TOK_IFELSE TOK_INITDECL
%token TOK_POS TOK_NEG TOK_NEWARRAY TOK_TYPEID TOK_FIELD
%token TOK_PROTO TOK_PARAM TOK_INDEX TOK_FUNCTION
%token TOK_DECLID TOK_VARDECL TOK_NEWSTR TOK_CHR TOK_ORD


%right TOK_IF TOK_ELSE
%right '='
%left TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%left '+' '-'
%left '*' '/' '%'
%right TOK_POS TOK_NEG '!'
%nonassoc '('
%start start


%%


start       : program                  { parser::root = $1; }
            ;
program     : program structdef        { $$ = $1->adopt($2); }
            | program function         { $$ = $1->adopt($2); }
            | program statement        { $$ = $1->adopt($2); }
            | program globaldecl       { $$ = $1->adopt($2); }
            | program error '}'        { $$ = $1; }
            | program error ';'        { $$ = $1; }
            |                          { 
                                         parser::root = new astree
                                            (TOK_ROOT, {0,0,0},"");
                                         $$ = parser::root;
                                       }
            ;

structdef   : fielddecls '}'
                                       {
                                         destroy($2);
                                         $$ = $1;
                                       }
            ;
fielddecls  : fielddecls fielddecl     {
                                         $$ = $1->adopt($2);
                                       }
            | TOK_STRUCT TOK_IDENT '{' 
                                       {
                                         destroy($3);
                                         $2->adopt_sym
                                            (nullptr,TOK_TYPEID);
                                         $$ = $1->adopt($2); 
                                       }
            ;
fielddecl   : basetype TOK_ARRAY TOK_IDENT ';'
                                       {
                                         destroy($4);
                                         $3->adopt_sym
                                            (nullptr,TOK_FIELD);
                                         $$ = $2->adopt($1,$3);
                                       }
            | basetype TOK_IDENT ';'      {
                                         destroy($3);
                                         $2->adopt_sym
                                            (nullptr,TOK_FIELD);
                                         $$ = $1->adopt($2);
                                       }
            ; 

basetype    : TOK_VOID                 { $$ = $1; }
            | TOK_BOOL                 { $$ = $1; }
            | TOK_INT                  { $$ = $1; }
            | TOK_STRING               { $$ = $1; }
            | TOK_CHAR                 { $$ = $1;printf("char\n"); }
            | TOK_IDENT                { $$ = $1->adopt_sym
                                          (nullptr,TOK_TYPEID); 
                                       }
            ;

globaldecl  : identdecl '=' constant ';'
                                       {
                                         destroy($4);
                                         $2->adopt_sym($1,TOK_VARDECL);
                                         $$ = $2->adopt($3);
                                       }
            ;

function    : identdecl '(' ')' ';'    {
                                         destroy ($3,$4);
                                         $$ = new astree
                                            (TOK_PROTO,$1->lloc, "");
                                         $2->adopt_sym
                                            (nullptr,TOK_PARAM);
                                         $$->adopt($1,$2);
                                       }
            | identdecl '(' ')' fnbody {
                                         destroy($3);
                                         $2->adopt_sym
                                            (nullptr,TOK_PARAM);
                                         $$ = new astree (TOK_FUNCTION,
                                              $1->lloc, "");
                                         $$->adopt($1,$2);
                                         $$->adopt($4);
                                       }
            | identdecl params ')' ';'
                                       {
                                         destroy($3,$4);
                                         $$ = new astree(TOK_PROTO, 
                                              $1->lloc , "");
                                         $$->adopt($1,$2);
                                       }
            | identdecl params ')' fnbody
                                       {
                                         destroy($3);
                                         $$ = new astree(TOK_FUNCTION,
                                              $1->lloc, "");
                                         $$->adopt($1,$2);
                                         $$->adopt($4);
                                       }
            ;

identdecl   : basetype TOK_ARRAY TOK_IDENT
                                       {
                                         $3->adopt_sym
                                            (nullptr,TOK_DECLID);
                                         $$ = $2->adopt($1,$3);
                                       }
            | basetype TOK_IDENT       {
                                         $2->adopt_sym
                                            (nullptr,TOK_DECLID);
                                         $$ = $1->adopt($2);
                                       }
            ;

fnbody      : funcBody '}'             {
                                         destroy($2);
                                         $$ = $1;
                                       }
            ;


funcBody    : funcBody localdecl       {
                                         $$ = $1->adopt($2);
                                       }
            | funcBody statement       {
                                         $$ = $1->adopt($2);
                                       }
            | '{' statement            {
                                         $$ = $1->adopt_sym
                                                 ($2,TOK_BLOCK);
                                       }
            | '{' localdecl            {
                                         $$ = $1->adopt_sym
                                                 ($2,TOK_BLOCK);
                                       }
            | '{'                      { $$ = $1; }
            ;
localdecl   : identdecl '=' expr ';'   {
                                         destroy($4);
                                         $2->adopt_sym($1,TOK_VARDECL);
                                         $$ = $2->adopt($3);
                                       }
            ;

statement   : block                    { $$ = $1; }
            | while                    { $$ = $1; }
            | ifelse                   { $$ = $1; }
            | return                   { $$ = $1; }
            | function ';'             { 
                                         destroy($2);
                                         $$ = $1; 
                                       }
            | expr ';'                 {
                                         destroy($2);
                                         $$ = $1;
                                       }
            
            | ';'                      {
                                         destroy($1);
                                       }
            ;

block       : '{' '}'                  {
                                         destroy($2);
                                         $$ = $1->adopt_sym
                                                 (nullptr,TOK_BLOCK);
                                       }
            | blockBody '}'                  {
                                         destroy($2);
                                         $$ = $1->adopt_sym
                                                 (nullptr,TOK_BLOCK);
                                       }
            ;

blockBody   : '{' statement            {
                                         $$ = $1->adopt($2);
                                       }
            | blockBody statement      {
                                         $$ = $1->adopt($2);
                                       }                            
            ;
while       : TOK_WHILE '(' expr ')' statement
                                       {
                                         destroy($2,$4);
                                         $$ = $1->adopt($3,$5);
                                       }
            ;
ifelse      : TOK_IF '(' expr ')' statement
                                       {
                                         destroy($2,$4);
                                         $$ = $1->adopt($3,$5);
                                       }
            | TOK_IF '(' expr ')' statement TOK_ELSE statement
                                       {
                                         destroy ($2,$4);
                                         destroy ($6);
                                         $1->adopt($3,$5);
                                         $$ = $1->adopt($7);
                                       }
            ;
return      : TOK_RETURN ';'           {
                                         destroy($2);
                                         $$ = $1;
                                       }
            | TOK_RETURN expr ';'      {
                                         destroy($3);
                                         $$ = $1->adopt($2);
                                       }
            ;
expr        : expr '=' expr            {
                                          $$ = $2->adopt($1,$3);
                                       }

            | expr '*' expr            {
                                          $$ = $2->adopt($1,$3);
                                       }

            | expr '/' expr            {
                                          $$ = $2->adopt($1,$3);
                                       }

            | expr '+' expr            {
                                          $$ = $2->adopt($1,$3);
                                       }

            | expr '-' expr            {
                                          $$ = $2->adopt($1,$3);
                                       }

            | expr TOK_EQ expr         {
                                          $$ = $2->adopt($1,$3);
                                       }

            | expr TOK_NE expr         {
                                          $$ = $2->adopt($1,$3);
                                       }

            | expr TOK_LT expr         {
                                          $$ = $2->adopt($1,$3);
                                       }

            | expr TOK_LE expr         {
                                          $$ = $2->adopt($1,$3);
                                       }

            | expr TOK_GT expr         {
                                          $$ = $2->adopt($1,$3);
                                       }

            | expr TOK_GE expr         {
                                          $$ = $2->adopt($1,$3);
                                       }

            | UNOP expr                {
                                         $$ = $1->adopt($2);
                                       }
            | allocation               {
                                         $$ = $1;
                                       }
            | call                     {
                                         $$ = $1;
                                       }
            | '(' expr ')'             {
                                         destroy($1,$3);
                                         $$ = $2;
                                       }
            | variable                 {
                                         $$ = $1;
                                       }
            | constant                 {
                                         $$ = $1;
                                       }
            | '(' ')'                  {
                                         destroy($1,$2);
                                         $$ = nullptr;
                                       }
            ;

allocation  : TOK_NEW TOK_IDENT '(' ')'
                                       {
                                         destroy($3,$4);
                                         $2->adopt_sym
                                            (nullptr,TOK_TYPEID);
                                         $$ = $1->adopt($2);
                                       }
            | TOK_NEW basetype '[' expr ']'
                                       {
                                         destroy($3,$5);
                                         $1->adopt_sym
                                            (nullptr,TOK_NEWARRAY);
                                         $$ = $1->adopt($2,$4);
                                       }
            | TOK_NEW TOK_STRING '(' expr ')'  
                                       {
                                         destroy($2,$3);
                                         destroy($5);
                                         $1->adopt_sym
                                            (nullptr,TOK_NEWSTR);
                                         $$= $1->adopt($4);
                                       }
            | TOK_NEW TOK_IDENT        {
                                         $2->adopt_sym
                                            (nullptr,TOK_TYPEID);
                                         $$ = $1->adopt($2);
                                       }
            ;
call        : TOK_IDENT exprs ')' 
                                       {
                                         astree* temp = new astree
                                             (TOK_CALL, $2->lloc,"("); 
                                         destroy($3);
                                         $$ = temp->adopt($1,$2);
                                       }
            ;

exprs       : '(' expr                 {
                                         $$ = $2;
                                       }
            | exprs ',' expr           {
                                         destroy($2);
                                         $$ = $1->adopt($3);
                                       }
            | '('                      { $$ = $1; }
            ;
variable    : TOK_IDENT                { $$ = $1; }
            | expr TOK_ARROW TOK_IDENT {
                                         $3->adopt_sym
                                            (nullptr,TOK_FIELD);
                                         $$ = $2->adopt($1,$3);
                                       }
            | expr '[' expr ']'        {
                                         destroy($4);
                                         $2->adopt_sym
                                            (nullptr,TOK_INDEX);
                                         $$ = $2->adopt($1,$3);
                                       }
            ;
constant    : TOK_INTCON               { $$ = $1; }
            | TOK_STRINGCON            { $$ = $1; }
            | TOK_CHARCON              { $$ = $1; }
            | TOK_NULL                 { $$ = $1; }
            ;

UNOP        : TOK_POS                  { $$ = $1; }
            | '-'                      { 
                                         $$ = $1->adopt_sym
                                            (nullptr,TOK_NEG); 
                                       }
            | '!'                      { $$ = $1; }
            | TOK_NEW                  { $$ = $1; }
            | TOK_ORD                  { $$ = $1; }
            | TOK_CHR                  { $$ = $1; }
                        
            ;

params      : '(' identdecl            {
                                         $$ = $1->adopt_sym
                                                 ($2, TOK_PARAM);
                                       }
            | params ',' identdecl     {
                                         destroy($2);
                                         $$ = $1->adopt($3);
                                       }
            ;               


%%


const char *parser::get_tname (int symbol) {
   return yytname [YYTRANSLATE (symbol)];
}

bool is_defined_token (int symbol) {
   return YYTRANSLATE (symbol) > YYUNDEFTOK;
}

