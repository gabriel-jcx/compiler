// $Id: astree.cpp,v 1.9 2017-10-04 15:59:50-07 - - $
// Chenxing Ji
// cji13
// Nelson Yeap
// nyeap

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "attr_bitset.h"
#include "astree.h"
#include "string_set.h"
#include "lyutils.h"
#include "symtable.h"

astree::astree (int symbol_, const location& lloc_, const char* info) {
   symbol = symbol_;
   lloc = lloc_;
   lexinfo = string_set::intern (info);
   // vector defaults to empty -- no children
}

astree::~astree() {
   while (not children.empty()) {
      astree* child = children.back();
      children.pop_back();
      delete child;
   }
   if (yydebug) {
      fprintf (stderr, "Deleting astree (");
      astree::dump (stderr, this);
      fprintf (stderr, ")\n");
   }
}

astree* astree::adopt (astree* child1, astree* child2) {
   if (child1 != nullptr) children.push_back (child1);
   if (child2 != nullptr) children.push_back (child2);
   return this;
}

astree* astree::adopt_sym (astree* child, int symbol_) {
   symbol = symbol_;
   return adopt (child);
}

void astree::dump_node (FILE* outfile) {
   char *tok_name = strdup(parser::get_tname(symbol));
   if (strstr(tok_name, "TOK_") == tok_name)
      tok_name += 4;
   //printf("%s\n", to_string_array(attribute).c_str());
   //fprintf(outfile,"{%zu}", block_count);
   fprintf(outfile, "%s \"%s\" (%zu.%zu.%zu) {%zu} ",
         tok_name, lexinfo->c_str(),lloc.filenr, lloc.linenr,
         lloc.offset, block_num);
   if(attribute[static_cast<unsigned>(attr::STRUCT)]){
      //fprintf(outfile,"struct ");
      fprintf(outfile, "struct \"%s\" ", struct_name->c_str());
   }
   fprintf(outfile, "%s", to_string_array(attribute).c_str());
   //fprintf(
   if(decl_lloc.linenr == 0){
      fprintf(outfile,"\n");
   }else{
      fprintf(outfile, " (%zd.%zd.%zd)\n", decl_lloc.filenr,
            decl_lloc.linenr, decl_lloc.offset);
   }
}

void astree::dump_tree (FILE* outfile, int depth) {
   for(int i = 0; i < depth; i++){
      if(depth == 0)
         break;
      else{
         fprintf(outfile, "%s", "|  ");
      }
   }
   dump_node (outfile);
   //fprintf (outfile, "\n");
   for (astree* child: children) child->dump_tree (outfile, depth + 1);
   fflush (nullptr);
}

void astree::dump (FILE* outfile, astree* tree) {
   if (tree == nullptr) fprintf (outfile, "nullptr");
                   else tree->dump_tree (outfile, 0);
}


void astree::print (FILE* outfile, astree* tree, int depth) {
   fprintf (outfile, "; %*s", depth * 3, "");
   fprintf (outfile, "%s \"%s\" (%zd.%zd.%zd) {%zu} %s",
            parser::get_tname (tree->symbol), tree->lexinfo->c_str(),
            tree->lloc.filenr, tree->lloc.linenr, tree->lloc.offset,
            tree->block_num, to_string_array(tree->attribute).c_str());
   if(tree->decl_lloc.linenr == 0){
      //printf("Not a declared ident yet\n");
      fprintf(outfile, "\n");
   }else{
      //printf("Existed symbol\n");
      fprintf(outfile, " (%zd.%zd.%zd)\n", tree->decl_lloc.filenr, \
            tree->decl_lloc.linenr, tree->decl_lloc.offset);
   }
   for (astree* child: tree->children) {
      astree::print (outfile, child, depth + 1);
   }
}

void destroy (astree* tree1, astree* tree2) {
   if (tree1 != nullptr) {
      while(tree1->children.size() != 0){
         tree1->children.pop_back();
         destroy(tree1);
      }
      delete tree1;
   }
   if (tree2 != nullptr) {
      while(tree2->children.size() != 0){
         tree2->children.pop_back();
         destroy(tree2);
      }
      delete tree2;
   }
}

void errllocprintf (const location& lloc, const char* format,
                    const char* arg) {
   static char buffer[0x1000];
   assert (sizeof buffer > strlen (format) + strlen (arg));
   snprintf (buffer, sizeof buffer, format, arg);
   errprintf ("%s:%zd.%zd: %s", 
              lexer::filename (lloc.filenr), lloc.linenr, lloc.offset,
              buffer);
}


