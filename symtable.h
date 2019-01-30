// Chenxing Ji
// cji13
// Nelson Yeap
// nyeap
#include <vector>
#include <unordered_map>
//#include "astree.h"
#include <bitset>
#include "attr_bitset.h"
#include "astree.h"
using namespace std;
struct symbol 
{
   //public:
   //   symbol();
   attr_bitset attribute;
   const string* struct_name;
   location lloc;
   size_t block_num;
   size_t ident_count;
   vector<symbol*> *params;
   unordered_map<string*, symbol*> *fields;
};
typedef symbol sym;
using symbol_table = unordered_map<string*, symbol*>;
using symbol_entry = symbol_table::value_type;
typedef struct symbol symbol;
void set_type_attr_on_operator(astree *node);
const string* return_type(astree* node);
void traverse_root(FILE*, astree* node);
void DFS(FILE* out, astree* node, symbol_table* local_table);
void assign_sym(FILE* out, astree* node, symbol_table* local_table);
void sym_dump(FILE* outfile, astree* node);
