// Chenxing Ji
// cji13
// Nelson Yeap
// nyeap
#include <bitset>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include "astree.h"
#include "auxlib.h"
#include "symtable.h"
#include "attr_bitset.h"
#include "yyparse.h"
using namespace std;

void oil_traverse_root(FILE*,astree*);
void emit_oil(FILE*, astree*);
void emit_expr(FILE*, astree*);
