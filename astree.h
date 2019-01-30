// $Id: astree.h,v 1.7 2016-10-06 16:13:39-07 - - $
// Chenxing Ji
// cji13
// Nelson Yeap
// nyeap

#ifndef __ASTREE_H__
#define __ASTREE_H__

#include <string>
#include <vector>
//#include <bitset>
//#include "auxlib.h"
//#include "symtable.h"
#include <unordered_map>
#include "attr_bitset.h"
using namespace std;

struct location {
   size_t filenr;
   size_t linenr;
   size_t offset;
};

struct astree {

   // Fields.
   int symbol;               // token code
   location lloc;            // source location
   location decl_lloc;       // symbol declared location 
   const string* lexinfo;    // pointer to lexical information
   vector<astree*> children; // children of this n-way node
   attr_bitset attribute;
   size_t block_num;
   size_t block_count;
   const string* struct_name; //parent struct name
   //symbol_table* struct_table;
   
   // Functions.
   astree (int symbol, const location&, const char* lexinfo);
   ~astree();
   astree* adopt (astree* child1, astree* child2 = nullptr);
   astree* adopt_sym (astree* child, int symbol);
   void dump_node (FILE*);
   void dump_tree (FILE*, int depth = 0);
   static void dump (FILE* outfile, astree* tree);
   static void print (FILE* outfile, astree* tree, int depth = 0);
};

void destroy (astree* tree1, astree* tree2 = nullptr);

void errllocprintf (const location&, const char* format, const char*);

#endif

