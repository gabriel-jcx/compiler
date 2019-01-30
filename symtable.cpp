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

symbol_table global_identifiers_table;
symbol_table type_name_table;//symbol table for all the types

int next_block = 0;
bool inside_func = false;
int local_counter = 0;
int param_counter = 0;
int field_counter = 0;
int in_struct = 0;
int in_func = 0;
int curr_block_count = 0;

string curr_func_type;
string literal_string = "string";
string literal_int = "int";
string literal_char = "char";
string literal_array = "array";
string literal_test = "test";
string literal_nullx = "nullx";
string literal_struct = "struct";
string literal_void = "void";
string literal_not_implemented = "not_implement_type";

const string* return_symbol_type(symbol* node)
{
   if(node->attribute[static_cast<unsigned>(attr::STRUCT)]){
      return node->struct_name;
   }
   if(node->attribute[static_cast<unsigned>(attr::STRING)]){
      return &literal_string;
   }
   if(node->attribute[static_cast<unsigned>(attr::INT)]){
      return &literal_int;
   }
   if(node->attribute[static_cast<unsigned>(attr::CHAR)]){
      return &literal_char;
   }
   return &literal_not_implemented;
}

void attr_on_arrow(symbol* symbol, astree* node)
{
   if (*return_symbol_type(symbol) == "int"){
      node->attribute.set(static_cast<unsigned>(attr::INT));  
   }else if(*return_symbol_type(symbol) == "string"){
      node->attribute.set(static_cast<unsigned>(attr::STRING));
   }else if(*return_symbol_type(symbol) == "char"){
      node->attribute.set(static_cast<unsigned>(attr::CHAR));
   }
}


void set_type_attr_on_operator(astree *node)
{
   if (*return_type(node->children[0]) == "int"){
      node->attribute.set(static_cast<unsigned>(attr::INT));  
   }else if(*return_type(node->children[0]) == "string"){
      node->attribute.set(static_cast<unsigned>(attr::STRING));
   }else if(*return_type(node->children[0]) == "char"){
      node->attribute.set(static_cast<unsigned>(attr::CHAR));
   }else if(*return_type(node->children[0]) == "void"){
      node->attribute.set(static_cast<unsigned>(attr::VOID));
   }
}   
bool int_checker(astree* node){
   if (node->children.size() == 1)
   {
      if(node->children[0]->attribute[static_cast<unsigned>(attr::INT)])
      {
         return true;
      }
      else
      {
         return false;
      } 
   }
   if(node->children[0]->attribute[static_cast<unsigned>(attr::INT)]and
      node->children[1]->attribute[static_cast<unsigned>(attr::INT)]){
      return true;
   }
   else{
      return false;
   }
}

const string* return_type(astree* node){
   if(node->attribute[static_cast<unsigned>(attr::STRUCT)]){
      return node->struct_name;
   }
   if(node->attribute[static_cast<unsigned>(attr::STRING)]){
      return &literal_string;
   }
   if(node->attribute[static_cast<unsigned>(attr::INT)]){
      return &literal_int;
   }
   if(node->attribute[static_cast<unsigned>(attr::CHAR)]){
      return &literal_char;
   }
   if(node->attribute[static_cast<unsigned>(attr::NULLX)]){
      return &literal_nullx;
   }
   if(node->attribute[static_cast<unsigned>(attr::VOID)]){
      return &literal_void;
   }
   fprintf(stderr,"The type is no implemented: %s\n", node->lexinfo->c_str());
   return &literal_not_implemented;
}
bool type_check(astree* node1, astree* node2)
{
   
   const string* type1 = return_type(node1);
   const string* type2 = return_type(node2);
   if (*type1 == *type2)
   {
      return true;
   }  
   return false;
}

void sym_dump(FILE* outfile, astree* node){
   if((node->attribute[static_cast<unsigned>(attr::STRUCT)] or
         node->attribute[static_cast<unsigned>(attr::FUNCTION)])and
         !node->attribute[static_cast<unsigned>(attr::FIELD)] and
         !node->attribute[static_cast<unsigned>(attr::LOCAL)] and
         !node->attribute[static_cast<unsigned>(attr::PARAM)]){
      fprintf(outfile, "\n");
   }
   else {
      for(int i = 0; i < next_block; ++i){
         fprintf(outfile, "   ");//indent acccordingly:
      }
   }
   fprintf(outfile, "%s: (%zu.%zu.%zu) ", node->lexinfo->c_str(), 
         node->lloc.filenr, node->lloc.linenr, node->lloc.offset
         );
   if(!node->attribute[static_cast<unsigned>(attr::FIELD)]){
      fprintf(outfile,"{%zd} ", node->block_num);
   }
   if(node->attribute[static_cast<unsigned>(attr::STRUCT)]){
     fprintf(outfile,"struct %s ", node->struct_name->c_str());
   }
   fprintf(outfile,"%s",to_string_array(node->attribute).c_str());
   if (node->attribute[static_cast<unsigned>(attr::LOCAL)]){
      fprintf(outfile,"%d", local_counter);
   } else if(node->attribute[static_cast<unsigned>(attr::PARAM)]){
      fprintf(outfile,"%d", param_counter);
   } else if(node->attribute[static_cast<unsigned>(attr::FIELD)]){
      fprintf(outfile,"%d", field_counter);
   }
   fprintf(outfile,"\n");
}

symbol* astree_to_symbol(astree*node)
{
   if (node == nullptr)
   {
      return nullptr;
   }
   symbol* symbol_temp = new symbol();
   symbol_temp->attribute = node->attribute;
   symbol_temp->lloc = node->lloc;
   symbol_temp->block_num = node->block_num;
   symbol_temp->struct_name = node->struct_name;
   symbol_temp->params = nullptr;
   symbol_temp->fields = nullptr;
   return symbol_temp;
}


void assign_sym(FILE* out, astree* node, symbol_table* local_table){
   if (node == nullptr)
   {
      return;
   }
   node->block_count = curr_block_count;
   node->block_num = next_block;
   switch (node->symbol)
   {
      case TOK_STRUCT:
      {
         ++curr_block_count;
         in_struct = 1;
         node->children[0]->block_count = curr_block_count;
         node->children[0]->block_num = next_block;
         node->children[0]->attribute.set(
               static_cast<int>(attr::STRUCT));
         node->children[0]->struct_name = node->children[0]->lexinfo;
         symbol* symbol = astree_to_symbol(node->children[0]);
         type_name_table.insert(symbol_entry(
                  (string*)node->children[0]->lexinfo, symbol));
         symbol_table* field_table = new symbol_table();
         symbol->fields = field_table;
         sym_dump(out,node->children[0]);
         for (auto child:node->children)
         {
            child->block_count = curr_block_count;
            child->block_num = next_block;
            if (child == node->children[0])
            {
               continue;
            }
            if(child->symbol == TOK_ARRAY){
               child->children[1]->block_count = curr_block_count;
               child->children[1]->block_num = next_block;
               child->children[1]->attribute.set(
                  static_cast<int>(attr::FIELD));
               DFS(out, child, field_table);
               auto grandchild = astree_to_symbol(child->children[1]);
               field_table->insert(symbol_entry((string*)
                  child->children[1]->lexinfo,grandchild));
               ++field_counter;
            }else{
               child->children[0]->attribute.set(
                  static_cast<int>(attr::FIELD));
               DFS(out,child, field_table);
               auto grandchild = astree_to_symbol(child->children[0]);
               field_table->insert(symbol_entry(
                  (string*)child->children[0]->lexinfo, grandchild));
               sym_dump(out,child->children[0]);
               ++field_counter;
            }
         }
         in_struct = 0;
      }
         break;
      case TOK_TYPEID:
      {
         auto finder = type_name_table.find((string*)node->lexinfo);
         if (finder == type_name_table.end())
         {
            symbol* symbol = astree_to_symbol(node);
            type_name_table.insert(symbol_entry(
                     (string*)node->lexinfo,symbol));
         }
         else
         {
            node->decl_lloc = finder->second->lloc;
         }
         if (node->children[0] != nullptr)
         { 
            node->children[0]->struct_name = finder->first;
            node->children[0]->attribute.set(
                  static_cast<unsigned>(attr::STRUCT));
         }
         if (!(in_struct || in_func))
         {
            node->children[0]->attribute.set(
                  static_cast<unsigned>(attr::VARIABLE));
            node->children[0]->attribute.set(
                  static_cast<unsigned>(attr::LVAL));
            node->children[0]->attribute.set(
                  static_cast<unsigned>(attr::LOCAL));
            node->children[0]->block_num = next_block;
            sym_dump(out,node->children[0]);
         }
      }
      break;
      case TOK_IDENT:
      {
         if (local_table == nullptr)
         {
            break;
         }

         auto finder = local_table->find((string*)node->lexinfo);
         if (finder == local_table->end())
         {
            finder = global_identifiers_table.find(
                  (string*)node->lexinfo);
         }
         if (finder == global_identifiers_table.end())
         {
            fprintf(stderr,"ident \"%s\" not found in global table\n", 
                  node->lexinfo->c_str());
         }
         else
         {
            node->struct_name = finder->second->struct_name;
            node->attribute = finder->second->attribute;
            node->decl_lloc = finder->second->lloc;
         }
      }
         break;
      case TOK_FUNCTION:
      {
         ++curr_block_count;
         next_block = 0;
         param_counter = 0;
         local_counter = 0;
         in_func = 1;
         node->block_num = 0;
         node->children[0]->children[0]->block_count = curr_block_count;
         node->children[0]->children[0]->block_num = next_block;
         ++next_block;
         curr_func_type = *node->children[0]->lexinfo;
         symbol_table* new_local_table = new symbol_table();
         auto finder = global_identifiers_table.find(
               (string*)node->children[0]->children[0]->lexinfo);
         node->children[1]->block_count = curr_block_count;
         node->children[1]->block_num = next_block;
         if (finder != global_identifiers_table.end())
         {
            break;
            // error out cause already a fucntion with that name
         }
         else
         {
            assign_sym(out, node->children[0], new_local_table);
            node->children[0]->children[0]->attribute.set
               (static_cast<unsigned>(attr::FUNCTION));
            sym_dump(out, node->children[0]->children[0]);
            symbol* func_name = astree_to_symbol(
                  node->children[0]->children[0]);
            global_identifiers_table.insert(symbol_entry(
                     (string*)node->children[0]->children[0]->lexinfo,
                     func_name));
            for (auto child:node->children[1]->children){
               child->children[0]->block_num = next_block;
               assign_sym(out,child, new_local_table);
               child->block_count = curr_block_count;
               child->block_num = next_block;
               if (child->symbol == TOK_ARRAY)
               {
                  child->children[1]->block_count = curr_block_count;
                  child->children[1]->block_num = next_block;
                  child->children[1]->attribute.set
                     (static_cast<unsigned>(attr::PARAM));
                  child->children[1]->attribute.set
                     (static_cast<unsigned>(attr::VARIABLE));
                  child->children[1]->attribute.set
                     (static_cast<unsigned>(attr::LVAL));
                  sym_dump(out,child->children[1]);
                  symbol* symbol = astree_to_symbol(child->children[1]);
                  new_local_table->insert(symbol_entry(
                       (string*)child->children[1]->lexinfo,symbol));
               }
               else
               {
                  child->children[0]->block_count = curr_block_count;
                  child->children[0]->block_num = next_block;
                  child->children[0]->attribute.set
                     (static_cast<unsigned>(attr::PARAM));
                  child->children[0]->attribute.set
                     (static_cast<unsigned>(attr::VARIABLE));
                  child->children[0]->attribute.set
                     (static_cast<unsigned>(attr::LVAL));
                  sym_dump(out,child->children[0]);
                  symbol* symbol = astree_to_symbol(child->children[0]);
                  new_local_table->insert(symbol_entry(
                       (string*)child->children[0]->lexinfo,symbol));
               }
               ++param_counter;
            }
            in_func = 0;
            if (node->children.size() < 3)
            {
               break;
            }
            for (auto child:node->children[2]->children)
            {
               assign_sym(out, child, new_local_table);
            }
         }
         next_block = 0;
      }
      break;
      case TOK_BLOCK:
      {
         ++next_block;
         for (auto child:node->children)
         {
            assign_sym(out, child, local_table);
         }
         --next_block; 
      }
         break;
      case TOK_VARDECL:
      {
         for (auto child:node->children)
         {
            assign_sym(out,child,local_table);
         }
         if(node->children[0]->symbol == TOK_ARRAY)
         { 
            node->children[0]->children[1]->attribute.set(
               static_cast<unsigned>(attr::VARIABLE));
            node->children[0]->children[1]->attribute.set(
               static_cast<unsigned>(attr::LVAL));
            node->children[0]->children[1]->block_num = next_block;
            node->children[0]->children[1]->block_count = 
               curr_block_count;

            symbol* symbol = astree_to_symbol(
                  node->children[0]->children[1]);
            
            local_table->insert(symbol_entry(
               (string*)node->children[0]->children[1]->lexinfo,
               symbol));
            sym_dump(out,node->children[0]->children[1]); 
         }else{
            node->children[0]->children[0]->attribute.set(
               static_cast<unsigned>(attr::VARIABLE));
            node->children[0]->children[0]->attribute.set(
               static_cast<unsigned>(attr::LVAL));
            
            node->children[0]->children[0]->block_count = 
               curr_block_count;
            
            node->children[0]->children[0]->block_num = next_block;
            symbol* symbol = astree_to_symbol(
               node->children[0]->children[0]);
            local_table->insert(symbol_entry((string*)
               node->children[0]->children[0]->lexinfo,symbol));
            sym_dump(out,node->children[0]->children[0]);
         }
      }
         break;
      case TOK_VOID:
      {
         if(node->children.size() == 0){
            break; 
         }
         node->children[0]->attribute.set(
               static_cast<int>(attr::VOID));
      }
         break;
      case TOK_BOOL: 
      {
         auto finder = type_name_table.find((string*)node->lexinfo);
         if (finder == type_name_table.end())
         {
            symbol* symbol = astree_to_symbol(node);
            node->decl_lloc = node->lloc;
            type_name_table.insert(symbol_entry
                  ((string*)node->lexinfo,symbol));
         }
      }
         break;
      case TOK_CHAR:
      {
         auto finder = type_name_table.find((string*)node->lexinfo);
         if (finder == type_name_table.end())
         {
            symbol* symbol = astree_to_symbol(node);
            type_name_table.insert(symbol_entry
                  ((string*)node->lexinfo,symbol));
         }
         break;
      }
      case TOK_INT:
      {
         if (in_struct||in_func|| next_block == 0)
         {
            auto finder = type_name_table.find((string*)node->lexinfo);
            if (finder == type_name_table.end())
            {
               symbol* symbol = astree_to_symbol(node);
               type_name_table.insert(symbol_entry
                     ((string*)node->lexinfo,symbol));
            }
            if(node->children.size() == 0){
               break;
            }
            node->children[0]->attribute.set(
                  static_cast<int>(attr::INT));
         }
         else 
         {
            if (node->children.size() == 0)
            {
               break;
            }
            node->children[0]->block_num = next_block;
            auto finder = local_table->find(
                  (string*)node->children[0]->lexinfo);
            node->children[0]->attribute.set(
                  static_cast<int>(attr::INT));
            node->children[0]->attribute.set(
                  static_cast<int>(attr::LOCAL));
            node->children[0]->attribute.set(
                  static_cast<int>(attr::VARIABLE));
            node->children[0]->attribute.set(
                  static_cast<int>(attr::LVAL));
            if (finder == local_table->end())
            {
               symbol* symbol = astree_to_symbol(node->children[0]);
               local_table->insert(symbol_entry(
                        (string*)node->children[0]->lexinfo,symbol));
            }
            sym_dump(out, node->children[0]);
            ++local_counter;
         }
      }
         break;
      case TOK_STRING:
      {
         if (in_struct||in_func || next_block == 0)
         {
            auto finder = type_name_table.find((string*)node->lexinfo);
            if (finder == type_name_table.end())
            {
               symbol* symbol = astree_to_symbol(node);
               type_name_table.insert(symbol_entry(
                        (string*)node->lexinfo,symbol));
            }
            if(node->children.size() == 0){
               break;
            }
            node->children[0]->attribute.set(
                  static_cast<int>(attr::STRING));
         }
         else 
         {
            if(node->children.size() == 0){
               break;
            }
            node->children[0]->block_num = next_block;
            auto finder = local_table->find((string*)node->lexinfo);
            if (finder == local_table->end())
            {
               symbol* symbol = astree_to_symbol(node);
               local_table->insert(symbol_entry(
                        (string*)node->lexinfo,symbol));
            }
            node->children[0]->attribute.set(
                  static_cast<int>(attr::LOCAL));
            node->children[0]->attribute.set(
                  static_cast<int>(attr::VARIABLE));
            next_block = 0;
            node->children[0]->attribute.set(
                  static_cast<int>(attr::LVAL));
            sym_dump(out, node->children[0]);
            ++local_counter;
         }
         node->children[0]->attribute.set(
               static_cast<int>(attr::STRING));
         break;
      }
      case TOK_IF:
      {
         for (auto child:node->children)
         {
            assign_sym(out, child, local_table);
         }
      }
         break;
      case TOK_NULL:
      {
         node->attribute.set(static_cast<int>(attr::NULLX));
         node->attribute.set(static_cast<int>(attr::CONST));
      }
         break;
      case TOK_NEW:
      {
         node->attribute.set(static_cast<unsigned>(attr::VREG));
      }
         break;
      case TOK_EQ:
      {
         for (auto child:node->children)
         {
            assign_sym(out,child,local_table); 
         }
         if (!type_check(node->children[0],node->children[1]))
         {
            fprintf(stderr,"ERROR TYPES DONT CHECK ON %s = %s\n",
                  node->children[0]->lexinfo->c_str(),
                  node->children[1]->lexinfo->c_str());
         }
         set_type_attr_on_operator(node);
      }   
         break;
      case TOK_NE:
         {
            for (auto child:node->children)
            {
               assign_sym(out,child,local_table); 
            }
            if (!type_check(node->children[0],node->children[1]))
            {
               fprintf(stderr,"ERROR TYPES DONT CHECK ON %s = %s\n",
                     node->children[0]->lexinfo->c_str(),
                     node->children[1]->lexinfo->c_str());
            }
            node->attribute.set(static_cast<unsigned>(attr::INT));
            node->attribute.set(static_cast<unsigned>(attr::VREG));

         }
         break;
      case TOK_LT:
         {
            for (auto child:node->children)
            {
               assign_sym(out,child,local_table); 
            }
            if (!type_check(node->children[0],node->children[1]))
            {
            fprintf(stderr,"ERROR TYPES DONT CHECK ON %s = %s\n",
                  node->children[0]->lexinfo->c_str(),
                  node->children[1]->lexinfo->c_str());
            }
            node->attribute.set(static_cast<unsigned>(attr::INT));
            node->attribute.set(static_cast<unsigned>(attr::VREG));
         }
         break;
      case TOK_LE:
         {
            for (auto child:node->children)
            {
               assign_sym(out,child,local_table); 
            }
            if (!type_check(node->children[0],node->children[1]))
            {
            fprintf(stderr,"ERROR TYPES DONT CHECK ON %s = %s\n",
                  node->children[0]->lexinfo->c_str(),
                  node->children[1]->lexinfo->c_str());
            }
            node->attribute.set(static_cast<unsigned>(attr::INT));
            node->attribute.set(static_cast<unsigned>(attr::VREG));
         }
         break;
      case TOK_GT:
         {
            for (auto child:node->children)
            {
               assign_sym(out,child,local_table); 
            }
            if (!type_check(node->children[0],node->children[1]))
            {
            fprintf(stderr,"ERROR TYPES DONT CHECK ON %s = %s\n",
                  node->children[0]->lexinfo->c_str(),
                  node->children[1]->lexinfo->c_str());
            }
            node->attribute.set(static_cast<unsigned>(attr::INT));
            node->attribute.set(static_cast<unsigned>(attr::VREG));
         }
         break;
      case TOK_GE:
         {
            for (auto child:node->children)
            {
               assign_sym(out,child,local_table); 
            }
            if (!type_check(node->children[0],node->children[1]))
            {
            fprintf(stderr,"ERROR TYPES DONT CHECK ON %s = %s\n",
                  node->children[0]->lexinfo->c_str(),
                  node->children[1]->lexinfo->c_str());
            }
            node->attribute.set(static_cast<unsigned>(attr::INT));
            node->attribute.set(static_cast<unsigned>(attr::VREG));
         }
         break;
      case '=':
         {
            for (auto child:node->children)
            {
               assign_sym(out,child,local_table); 
            }
            if(node->children[0]->symbol == TOK_ARRAY or
                  node->children[0]->symbol == TOK_INDEX){
            }
            else if(node->children[1]->symbol == TOK_NEWARRAY or
                  node->children[1]->symbol == TOK_ARRAY){

            }else{
               if (!type_check(node->children[0],node->children[1]))
               {
                  fprintf(stderr,"ERROR TYPES DONT CHECK ON %s = %s\n",
                        node->children[0]->lexinfo->c_str(),
                        node->children[1]->lexinfo->c_str());
               }
            }
           
         }
         break;
      case '+':
         {
            for (auto child:node->children)
            {
               assign_sym(out,child,local_table); 
            }
            if(!int_checker(node))
            {
               fprintf(stderr,"ERROR TYPES DONT CHECK ON %s = %s\n",
                     node->children[0]->lexinfo->c_str(),
                     node->children[1]->lexinfo->c_str());
            }
            set_type_attr_on_operator(node);
            node->attribute.set(static_cast<unsigned>(attr::VREG));
         }
         break;
      case '-':
         {
            for (auto child:node->children)
            {
               assign_sym(out,child,local_table); 
            }
            if(!int_checker(node))
            {
            fprintf(stderr,"ERROR TYPES DONT CHECK ON %s = %s\n",
                  node->children[0]->lexinfo->c_str(),
                  node->children[1]->lexinfo->c_str());
            }
            set_type_attr_on_operator(node);
            node->attribute.set(static_cast<unsigned>(attr::VREG));
         }
         break;
      case '*':
         {
            for (auto child:node->children)
            {
               assign_sym(out,child,local_table); 
            }
            if(!int_checker(node))
            {
            fprintf(stderr,"ERROR TYPES DONT CHECK ON %s = %s\n",
                  node->children[0]->lexinfo->c_str(),
                  node->children[1]->lexinfo->c_str());
            }
            set_type_attr_on_operator(node);
            node->attribute.set(static_cast<unsigned>(attr::VREG));
            fprintf(out, "\n");
         }
         break;
      case '/':
         {
            for (auto child:node->children)
            {
               assign_sym(out,child,local_table); 
            }
            if(!int_checker(node))
            {
              fprintf(stderr,"ERROR TYPES DONT CHECK ON %s = %s\n",
                    node->children[0]->lexinfo->c_str(),
                    node->children[1]->lexinfo->c_str());
            }
            set_type_attr_on_operator(node);
            node->attribute.set(static_cast<unsigned>(attr::VREG));
         }
         break;
      case '%':
         {
            for (auto child:node->children)
            {
               assign_sym(out,child,local_table); 
            }
            if (!int_checker(node))
            {
               fprintf(stderr,"ERROR TYPES DONT CHECK ON %s = %s\n",
                     node->children[0]->lexinfo->c_str(),
                     node->children[1]->lexinfo->c_str());
            }
            set_type_attr_on_operator(node);
            node->attribute.set(static_cast<unsigned>(attr::VREG));
         }
         break;
      case TOK_POS:
         {
            for (auto child:node->children)
            {
               assign_sym(out,child,local_table); 
            }
            if(!int_checker(node))
            {
               fprintf(stderr,"ERROR RIGHT OPERAND IS NOT AN INTEGER");
            }
            node->attribute.set(static_cast<unsigned>(attr::INT));
            node->attribute.set(static_cast<unsigned>(attr::VREG));
         }
         break;
      case TOK_NEG:
         {
            for (auto child:node->children)
            {
               assign_sym(out,child,local_table); 
            }
            if (!int_checker(node))
            {
               fprintf(stderr,"ERROR RIGHT OPERAND IS NOT AN INTEGER");
            } 
            node->attribute.set(static_cast<unsigned>(attr::INT));
            node->attribute.set(static_cast<unsigned>(attr::VREG));
         }
         break;
      case TOK_ARROW:
         {
            for (auto child:node->children)
            {
               assign_sym(out,child,local_table);
            }
            auto finder = type_name_table.find(
                  (string*)node->children[0]->struct_name);
            if (finder != type_name_table.end())
            {
               auto field_finder = finder->second->fields->find(
                     (string*)node->children[1]->lexinfo);
               if (field_finder == finder->second->fields->end())
               {
                  fprintf(stderr,
                        "ERROR RIGHT OPERAND NOT A FIELD IN STRUCT\n");
               }
               else
               {
                  attr_on_arrow(field_finder->second, node);
                  node->attribute.set(
                        static_cast<unsigned>(attr::VADDR));
                  node->attribute.set(
                        static_cast<unsigned>(attr::LVAL));
               }
            }
            else 
            {
               fprintf(stderr,"ERROR LEFT OPERAND IS NOT A STRUCT\n");
            }
         }
         break;
       case TOK_CALL:
         {
            for (auto child:node->children)
            {
               assign_sym(out,child,local_table);
            }
            set_type_attr_on_operator(node);
         }
         break;
      case TOK_INTCON:
         {
            node->attribute.set(static_cast<int>(attr::INT));
            node->attribute.set(static_cast<int>(attr::CONST)); 
         }
         break;
      case TOK_CHARCON:
         {
            node->attribute.set(static_cast<int>(attr::CHAR));
            node->attribute.set(static_cast<int>(attr::CONST)); 
         }
         break;
      case TOK_STRINGCON:
         {
            node->attribute.set(static_cast<int>(attr::STRING));
            node->attribute.set(static_cast<int>(attr::CONST)); 
         }
         break;
      case TOK_RETURN:
         for (auto child:node->children)
         {
            assign_sym(out, child,local_table);
         }
         if(curr_func_type == "void" and node->children.size() == 0){
            break;
         }
         if(node->children.size() == 0){
            fprintf(stderr, "ERROR non-void function ");
            fprintf(stderr, "type need to have a object to return\n");
            break;
         }
         if (!(*return_type(node->children[0])==curr_func_type ))
         {
           fprintf(stderr,"ERROR TYPES DONT CHECK ON %s = %s\n",
                 node->children[0]->lexinfo->c_str(),curr_func_type.c_str());
         }
         break;
      case TOK_FIELD:
         {
            node->attribute.set(static_cast<int>(attr::VADDR));
            node->attribute.set(static_cast<int>(attr::LVAL));

         }
         break;
      case TOK_WHILE:
         {
            for (auto child: node->children)
            {
               assign_sym(out,child,local_table);
            }
         }
         break;
      case TOK_PROTO:
         {
            next_block = 0;
            node->children[0]->children[0]->attribute.set(
                  static_cast<int>(attr::FUNCTION));
            node->children[0]->children[0]->block_num = next_block;
            next_block++;
            if (node->children[0]->symbol == TOK_INT){
               node->children[0]->children[0]->attribute.set(
                     static_cast<unsigned>(attr::INT));  
            }else if(node->children[0]->symbol == TOK_STRING){
               node->children[0]->children[0]->attribute.set(
                     static_cast<unsigned>(attr::STRING));
            }else if(node->children[0]->symbol == TOK_CHAR){
               node->children[0]->children[0]->attribute.set(
                     static_cast<unsigned>(attr::CHAR));
            }else if(node->children[0]->symbol == TOK_VOID){
               node->children[0]->children[0]->attribute.set(
                     static_cast<unsigned>(attr::VOID));
            }
            global_identifiers_table.insert(symbol_entry(
               (string*)node->children[0]->children[0]->lexinfo,
               astree_to_symbol(node->children[0]->children[0])));
            next_block--;
            next_block = 0; 
            sym_dump(out,node->children[0]->children[0]);
         }
         break;
      case TOK_INDEX:
         {
            for (auto child:node->children)
            {
               assign_sym(out,child,local_table);
            }
            if (*return_type(node->children[0]) == "int"){
               node->attribute.set(static_cast<unsigned>(attr::INT));  
            }else if(*return_type(node->children[0]) == "string"){
               node->attribute.set(static_cast<unsigned>(attr::CHAR));
            }else if(*return_type(node->children[0]) == "CHAR"){
               node->attribute.set(static_cast<unsigned>(attr::CHAR));
            }
           
         }
         break;
      case TOK_ARRAY:
         {
            for (auto child:node->children)
            {
               assign_sym(out,child,local_table);
            }
            if (node->children[0]->symbol == TOK_INT){
               node->children[1]->attribute.set(
                     static_cast<unsigned>(attr::INT));  
            }else if(node->children[0]->symbol == TOK_STRING){
               node->children[1]->attribute.set(
                     static_cast<unsigned>(attr::STRING));
            }else if(node->children[0]->symbol == TOK_CHAR){
               node->children[1]->attribute.set(
                     static_cast<unsigned>(attr::CHAR));
            }

            node->children[1]->attribute.set(
                  static_cast<int>(attr::ARRAY));
         }
         break;
      case TOK_NEWARRAY:
         {
            for (auto child:node->children)
            {
               assign_sym(out,child,local_table);
            }
         }
         break;
   }

}

void traverse_root(FILE* out, astree* root)
{
   int temp = 0;
   for (auto child: root->children)
   {
      next_block = 0;
      if(child->symbol == TOK_VARDECL){
         fprintf(out, "\n");
         for (auto grandchild:child->children)
         {
            assign_sym(out,grandchild,nullptr);
         }
         if(child->children[0]->symbol == TOK_ARRAY)
         { 
            child->children[0]->children[1]->attribute.set(
               static_cast<unsigned>(attr::VARIABLE));
            child->children[0]->children[1]->block_num = next_block;
            symbol* symbol = astree_to_symbol(
                  child->children[0]->children[1]);
            
            global_identifiers_table.insert(symbol_entry(
               (string*)child->children[0]->children[1]->lexinfo,
               symbol));
            
            sym_dump(out,child->children[0]->children[1]); 
         }else{
            child->children[0]->children[1]->attribute.set(
               static_cast<unsigned>(attr::VARIABLE));
            child->children[0]->children[1]->block_num = next_block;
            symbol* symbol = astree_to_symbol(
               child->children[0]->children[0]);
            global_identifiers_table.insert(symbol_entry((string*)
               child->children[0]->children[0]->lexinfo,symbol));
            sym_dump(out,child->children[0]->children[0]);
         }
      }else{
         assign_sym(out, child, nullptr);
      }
      temp ++;
   }
}
void DFS(FILE* out, astree* node, symbol_table* local_table) { 
   if (node->symbol == TOK_FUNCTION)
   {
      
   }
   else if (node->symbol == TOK_STRUCT)
   {
      
   }
   for (auto child: node->children)
   {
      assign_sym(out, node, local_table);
      DFS(out, child, local_table);
   }
}
