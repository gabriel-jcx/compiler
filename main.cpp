// Chenxing Ji
// cji13
// Nelson Yeap
// nyeap

#include <string>
using namespace std;
#include <iostream>
#include <fstream>
#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>
#include <getopt.h>

#include "auxlib.h"
#include "string_set.h"
#include "lyutils.h"
#include "oiler.h"

string CPP = "/usr/bin/cpp -nostdinc";
constexpr size_t LINESIZE = 1024;

// Chomp the last character from a buffer if it is delim.
void chomp (char* string, char delim) {
   size_t len = strlen (string);
   if (len == 0) return;
   char* nlpos = string + len - 1;
   if (*nlpos == delim) *nlpos = '\0';
}
void generate_astree(string filename){
   string ast_filename = filename;
   size_t i = ast_filename.find_last_of('.');
   ast_filename.erase(i + 1, 2);
   ast_filename.append("ast");
   FILE* ast_out = fopen(ast_filename.c_str(), "w");
   astree::dump(ast_out, parser::root);
   fclose(ast_out);
}
void generate_symtable(string filename){
   string sym_filename = filename;
   size_t i = sym_filename.find_last_of('.');
   sym_filename.erase(i+1,2);
   sym_filename.append("sym");
   FILE* sym_out = fopen(sym_filename.c_str(), "w");
   traverse_root(sym_out,parser::root);
   fclose(sym_out);
}

void generate_oil(string filename) {
   string oil_filename = filename;
   size_t i = oil_filename.find_last_of('.');
   oil_filename.erase(i+1,2);
   oil_filename.append("oil");
   FILE* oil_out = fopen(oil_filename.c_str(), "w");
   fprintf(oil_out, "#define __OCLIB_C__\n");
   ifstream in_file (filename);
   string define;
   getline(in_file, define);
   while (!in_file.eof())
   {
      if (define[0] == '#')
      {
         fprintf(oil_out, "%s\n", define.c_str());
      }
      getline(in_file,define);
   }
   oil_traverse_root(oil_out, parser::root);
   fclose(oil_out);
}

void scan (string filename){
   string tok_filename = filename;
   size_t i = tok_filename.find_last_of('.');
   tok_filename.erase(i + 1, 2);
   tok_filename.append("tok");
   tok_file = fopen(tok_filename.c_str(), "w");
   if (tok_file == NULL){
      perror("Error open the file");
   }
   else{
      yyparse();
   }
   fclose(tok_file);
}

int main(int argc, char **argv){
   yy_flex_debug = 0;
   yydebug = 0;
   int op_flag;
   //The options for the commandline
   while((op_flag = getopt(argc,argv,"@:D:ly")) != -1){
      switch(op_flag){
         case '@':
            set_debugflags(optarg);
            break;
         case 'D':
            CPP += "-D ";
            CPP += optarg;
            DEBUGF('o', "Opt -D set with flag: %c", optarg);
            break;
         case 'l':
            yy_flex_debug = 1;
            DEBUGF('o', "Opt -l set");
            break;
         case 'y':
            yydebug = 1;
            DEBUGF('o', "Opt -y set");
            break;
         default:
            perror(
                "Usage: oc [-ly] [-@ flag ...] [-D string] program.oc\n"
                  );
            break;
      }
   }
   if(optind > argc){
      errprintf("Command Error\n");
      exit(1);
   }
   //basename get the filename from a path
   string filename = basename(argv[argc-1]);
   //if the filename does not end with .oc
   if (filename.find(".oc") == string::npos){
      fprintf(stderr,"The file '%s' does not end with .oc\n",
            filename.c_str());
      exit(1);
   }
   
   exec::execname = basename(argv[0]);
   char* file_name = argv[argc-1];
   string command = CPP + " " + file_name;
   yyin = popen(command.c_str(),"r"); // this line replace the last line
   if (yyin == NULL)
   {
      syserrprintf(command.c_str());
   } 
   else
   {
      if (yy_flex_debug){
         fprintf(stderr, "-- popen %s, fileno(yyin) = %d\n",
              command.c_str(), fileno(yyin));
      }
      lexer::newfilename(command.c_str());
      scan (filename); 
      int closeComm = pclose(yyin);
      eprint_status (command.c_str(), closeComm);
      if (closeComm != 0)
      {
         exit(1);
      }     
      string str_filename = filename;
      size_t i = str_filename.find_last_of('.');
      str_filename.erase(i + 1, 2);

      str_filename.append("str");

      FILE* out = fopen(str_filename.c_str(), "w");
      string_set::dump(out);
      fclose(out);
      generate_symtable(filename);
      generate_astree(filename); 
      generate_oil(filename);
   }
   return 0;
}
