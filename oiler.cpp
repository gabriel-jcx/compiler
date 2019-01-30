// Chenxing Ji
// cji13
// Nelson Yeap
// nyeap
#include "oiler.h"
#include <vector>
using namespace std;
vector<string> const_string_array;
int break_count = 1;
int i_reg_count = 1;
int a_reg_count = 1;
int c_reg_count = 1;
int p_reg_count = 1;
int const_string_count = 1;

void output_type(FILE* oil_out, astree* node){
   if(node->symbol == TOK_STRING){
      fprintf(oil_out, "char*");
   }else if(node->symbol == TOK_INT){
      fprintf(oil_out, "int");
   }else if(node->symbol == TOK_CHAR){
      fprintf(oil_out, "char");
   }
}
void emit_string_cons(FILE* oil_out, astree* node)
{
   for (auto child: node->children)
   {
      if (child->symbol == TOK_STRINGCON)
      {
         fprintf(oil_out, "char* s%d = %s;\n", c_reg_count++,
               child->lexinfo->c_str());
         const_string_array.push_back(*child->lexinfo);
      }
      emit_string_cons(oil_out, child);
   }
}

void emit_expr(FILE* oil_out, astree* node)
{ 
   // go one level down if child is symbol
   if (node->children[0]->symbol == '+' or
         node->children[0]->symbol == '-' or
         node->children[0]->symbol == '*' or
         node->children[0]->symbol == '/' or
         node->children[0]->symbol == '%')
   {
      emit_expr(oil_out, node->children[0]);
      fprintf(oil_out, " %s ", node->lexinfo->c_str());
   }
   else
   {
      //print intcon
      if (node->children[0]->symbol != TOK_IDENT)
      {
         fprintf(oil_out, "%s %s ", \
               node->children[0]->lexinfo->c_str(), \
               node->lexinfo->c_str());
      }
      //print ident
      else
      {
         fprintf(oil_out, "_%d_%s ", \
               static_cast<int>(node->block_count), \
               node->children[0]->lexinfo->c_str());
         fprintf(oil_out, "%s ", node->lexinfo->c_str());
      }
   }
   // go one level down if child1 is symbol
   if (node->children[1]->symbol == '+' or
         node->children[1]->symbol == '-' or
         node->children[1]->symbol == '*' or
         node->children[1]->symbol == '/' or
         node->children[1]->symbol == '%')
   {
      fprintf(oil_out, "_%d_%s;", \
            static_cast<int>(node->block_count), \
            node->lexinfo->c_str());

   }
   else
   {
      //print intcon
      if (node->children[1]->symbol != TOK_IDENT)
      {
         emit_oil(oil_out, node->children[1]);
         fprintf(oil_out, ";");
      }
      //print ident
      else
      {
         fprintf(oil_out, "_%d_%s; ", \
               static_cast<int>(node->block_count), \
               node->children[1]->lexinfo->c_str());
         fprintf(oil_out, ";");
      }
   }
   fprintf(oil_out,"\n");
}


void emit_oil(FILE* oil_out, astree* node)
{
   if (node == nullptr)
   {
      return;
   }
   switch(node->symbol)
   {
      case TOK_NULL:
         {
            fprintf(oil_out, "0");
         }
         break;
      case TOK_FUNCTION:
         {

            fprintf(oil_out, "%s %s (",
                  node->children[0]->lexinfo->c_str(), 
                  node->children[0]->children[0]->lexinfo->c_str());
            // print out function names and params
            if(node->children[1]->children.size() == 0){
               fprintf(oil_out,"void)\n");
            }

            for (auto child:node->children[1]->children)
            {
               if(child->symbol == TOK_ARRAY)
               {
                  fprintf(oil_out, "\n        ");
                  output_type(oil_out, child->children[0]);
                  fprintf(oil_out, "* _%d_%s",
                     static_cast<int>(child->children[1]->block_count),
                     child->children[1]->lexinfo->c_str());
                  if(child != node->children[1]->children.back())
                  {
                     fprintf(oil_out, ",");
                  }else
                  {
                     fprintf(oil_out, ")\n");
                  }
                  continue;
               }
               if (child != node->children[1]->children.back())
               {
                  fprintf(oil_out, "\n        %s _%d_%s,", 
                        child->lexinfo->c_str(), 
                        static_cast<int>(
                           child->children[0]->block_count),
                        child->children[0]->lexinfo->c_str());

               }
               else
               {
                  fprintf(oil_out, "\n        %s _%d_%s)\n", 
                        child->lexinfo->c_str(), 
                        static_cast<int>(
                           child->children[0]->block_count), 
                        child->children[0]->lexinfo->c_str());
               }
            }
            fprintf(oil_out, "{\n");
            // print out blocks in funcitons if there is one
            if (node->children.size() < 3)
            {
               fprintf(oil_out, "}\n");
               return;
            }
            else
            {
               emit_oil(oil_out, node->children[2]);
            }
            fprintf(oil_out,"}\n"); 
         }
         break;

      case TOK_STRUCT:
         {

            fprintf(oil_out, "%s %s {\n", \
                  node->lexinfo->c_str(), 
                  node->children[0]->lexinfo->c_str());
            for (auto child:node->children)
            {
               if (child == node->children[0])
               {
                  continue;
               }
               if (child->symbol == TOK_ARRAY)
               {
                  fprintf(oil_out, "        %s %s %s;\n", \
                        child->children[0]->lexinfo->c_str(), 
                        child->lexinfo->c_str(),
                        child->children[1]->lexinfo->c_str());
               }
               else
               {
                  fprintf(oil_out, "        %s %s;\n", \
                        child->lexinfo->c_str(), \
                        child->children[0]->lexinfo->c_str()); 
               }
            }
            fprintf(oil_out, "};\n");

         }
         break;

      case TOK_VARDECL:
         {
            if (node->children[1]->symbol == TOK_STRINGCON)
            {
               fprintf(oil_out, "        char* ");
               emit_oil(oil_out, node->children[0]->children[0]);
               fprintf(oil_out, "= s%d;\n",
                     const_string_count);
               
               break;
            }
            if (node->children[1]->symbol == TOK_NEWARRAY)
            {
               emit_oil(oil_out, node->children[1]);
            }
            if (node->children[0]->children[0]->
                  attribute[static_cast<int>(attr::STRUCT)])
            {
               fprintf(oil_out, "        ");
               fprintf(oil_out, "struct %s* p%d = ",
                     node->children[0]->lexinfo->c_str(),
                     p_reg_count);
               fprintf(oil_out, "xcalloc(1, sizeof(struct %s));\n",
                     node->children[0]->lexinfo->c_str());
               return;
            }
            if (node->children[0]->symbol == TOK_ARRAY){
               //This following logic may can go to TOK_ARRAY 
               //and use emit_oil
               fprintf(oil_out, "        ");
               output_type(oil_out, node->children[0]->children[0]);
               fprintf(oil_out, "* ");
               emit_oil(oil_out, node->children[0]->children[1]);
            }else{
               fprintf(oil_out, "        %s _%d_%s ",
                  node->children[0]->lexinfo->c_str(),
                  static_cast<int>(
                     node->children[0]->children[0]->block_count),
                  node->children[0]->children[0]->lexinfo->c_str());
            }
            if (node->children[1]->symbol == TOK_INTCON or 
                  node->children[1]->symbol == TOK_STRINGCON or 
                  node->children[1]->symbol == TOK_CHARCON)
            {
               fprintf(oil_out, "= %s;", \
                     node->children[1]->lexinfo->c_str());
            }
            else
            {
               fprintf(oil_out, "= ");
               if (node->children[1]->symbol != TOK_NEWARRAY)
               {
                  emit_oil(oil_out, node->children[1]);
               }else{
                  fprintf(oil_out, "p%d;",p_reg_count++);
               }
            }
            fprintf(oil_out, "\n");
         }
         break;
      case TOK_NEWARRAY:
         {
            if (node->children[0]->symbol == TOK_INT)
            {
               fprintf(oil_out, 
                     "        int* p%d = xcalloc(%s, sizeof (int));\n",
                     p_reg_count,
                     node->children[1]->lexinfo->c_str());
            }
            if (node->children[0]->symbol == TOK_CHAR)
            {
               fprintf(oil_out, 
                     "        char* p%d = xcalloc(%s, sizeof(char));\n",
                     p_reg_count,
                     node->children[1]->lexinfo->c_str());
            }
            if (node->children[0]->symbol == TOK_STRING)
            {
               fprintf(oil_out, "        char** p%d = ",
                     p_reg_count);
               fprintf(oil_out, "xcalloc(%s, sizeof (char*));\n",
                     node->children[1]->lexinfo->c_str());
            }

         }
         break;
      case TOK_STRINGCON:
         {
            fprintf(oil_out, "s%d", const_string_count);
            ++const_string_count;
         }
         break;
      case TOK_INTCON:
         {
            fprintf(oil_out, "%s", node->lexinfo->c_str());
         }
         break;
      case TOK_CHARCON:
         {
            fprintf(oil_out, "%s", node->lexinfo->c_str());
         }
         break;
      case TOK_RETURN:
         {
            // print IDENT 
            fprintf(oil_out, "        %s",node->lexinfo->c_str());
            if (node->children.size() == 0)
            {
               break;
            }
            if(node->children[0]->symbol == TOK_IDENT)
            {

               fprintf(oil_out, " _%d_%s;", \
                     static_cast<int>(node->block_count), \
                     node->children[0]->lexinfo->c_str());
            }
            // print FOOCON
            else if (node->children[0]->symbol != TOK_IDENT)
            {
               fprintf(oil_out, " %s;", \
                     node->children[0]->lexinfo->c_str());
            }
            fprintf(oil_out,"\n");
         }
         break;
      case TOK_BLOCK:
         {
            for (auto child: node->children)
            {
               emit_oil(oil_out, child);
            }
         }
         break;
      case TOK_IF:
         {
            // if node->children[0] is not an operand i.e. '+','='
            emit_oil(oil_out, node->children[0]);
            fprintf(oil_out,"        if (!b%d) goto else_%d_%d_%d;\n",
                  break_count++,static_cast<int>(node->lloc.filenr),
                  static_cast<int>(node->lloc.linenr),
                  static_cast<int>(node->lloc.offset));
            emit_oil(oil_out, node->children[1]);
            fprintf(oil_out, "        goto fi_%d_%d_%d;\n",
                  static_cast<int>(node->lloc.filenr),
                  static_cast<int>(node->lloc.linenr),
                  static_cast<int>(node->lloc.offset));
            fprintf(oil_out, "else_%d_%d_%d:;\n",
                  static_cast<int>(node->lloc.filenr),
                  static_cast<int>(node->lloc.linenr),
                  static_cast<int>(node->lloc.offset));
            emit_oil(oil_out, node->children[2]);
            fprintf(oil_out, "fi_%d_%d_%d:;\n",
                  static_cast<int>(node->lloc.filenr),
                  static_cast<int>(node->lloc.linenr),
                  static_cast<int>(node->lloc.offset));
         }
         break;
      case '!':
         {
            emit_oil(oil_out, node->children[0]);
            ++break_count;
            fprintf(oil_out, "        char b%d = b%d;\n",
                  break_count,break_count);

         }
         break;
      case '=':
         {
            if (node->children[1]->attribute[
                  static_cast<int>(attr::VREG)])
            {
               fprintf(oil_out, "        int i%d = ", i_reg_count);
               emit_oil(oil_out, node->children[1]);
               fprintf(oil_out, "        ");
               emit_oil(oil_out, node->children[0]);
               fprintf(oil_out, " = i%d", i_reg_count++);
            }else{
               fprintf(oil_out, "        ");
               emit_oil(oil_out, node->children[0]);
               fprintf(oil_out, " = ");
               emit_oil(oil_out, node->children[1]);

            }
            fprintf(oil_out, ";\n");
         }
         break;

      case TOK_ARROW:
         {
            fprintf(oil_out, "p%d%s%s",
                  p_reg_count,
                  node->lexinfo->c_str(),
                  node->children[1]->lexinfo->c_str());
         }
         break;

      case '+':
         {
            if(node->children[0]->attribute[
                  static_cast<unsigned>(attr::VREG)]){
               emit_oil(oil_out, node->children[0]);
               fprintf(oil_out, "        ");
               fprintf(oil_out, "int i%d", ++i_reg_count);
               fprintf(oil_out, " = ");
               fprintf(oil_out, "%s + i%d;\n", 
                     node->children[1]->lexinfo->c_str(),
                     i_reg_count-1);
            }else{
               emit_expr(oil_out, node);
            }
         }
         break;

      case '-':
         {
            if(node->children[1]->attribute[
                  static_cast<unsigned>(attr::VREG)]){
               emit_oil(oil_out, node->children[1]);
            }else{
               emit_expr(oil_out, node);
            }
         }
         break;

      case '*':
         {
            if(node->children[1]->attribute[
                  static_cast<unsigned>(attr::VREG)]){
               emit_oil(oil_out, node->children[1]);
               fprintf(oil_out, "        ");
               fprintf(oil_out, "int i%d", ++i_reg_count);
               fprintf(oil_out, " = ");
               fprintf(oil_out, "%s * i%d;\n", 
                     node->children[0]->lexinfo->c_str(),
                     i_reg_count-1);
            }
            emit_expr(oil_out, node);
         }
         break;

      case '/':
         {
            emit_expr(oil_out, node);
            if(node->children[1]->attribute[
                  static_cast<unsigned>(attr::VREG)]){
               emit_oil(oil_out, node->children[1]);
            }
         }
         break;

      case '%':
         {     
            if(node->children[1]->attribute[
                  static_cast<unsigned>(attr::VREG)]){
               emit_oil(oil_out, node->children[1]);
               fprintf(oil_out, "        ");
               ++i_reg_count;
               fprintf(oil_out, "int i%d", i_reg_count);
               fprintf(oil_out, " = ");
               i_reg_count -= 1;
               fprintf(oil_out, "%s  i%d;\n", 
                     node->children[0]->lexinfo->c_str(),
                     i_reg_count);
               i_reg_count += 1;
            }
            emit_expr(oil_out, node);
         }
         break;
      case TOK_DECLID:
         {
            fprintf(oil_out, "_%d_%s",
                  static_cast<int>(node->block_count),
                  node->lexinfo->c_str());
         }
         break;

      case TOK_IDENT:
         {
            fprintf(oil_out, "_%d_%s", \
                  static_cast<int>(node->block_count), \
                  node->lexinfo->c_str());
         }
         break;
      case TOK_WHILE:
         {
            fprintf(oil_out,"while_%d_%d_%d:;\n", \
                  static_cast<int>(node->lloc.filenr), \
                  static_cast<int>(node->lloc.linenr), \
                  static_cast<int>(node->lloc.offset));
            emit_oil(oil_out, node->children[0]);
            fprintf(oil_out,"        if (!b%d) goto break_%d_%d_%d;\n",
                  break_count++,static_cast<int>(node->lloc.filenr),
                  static_cast<int>(node->lloc.linenr),
                  static_cast<int>(node->lloc.offset));
            emit_oil(oil_out, node->children[1]);
            fprintf(oil_out, "        goto while_%d_%d_%d;\n",
                  static_cast<int>(node->lloc.filenr),
                  static_cast<int>(node->lloc.linenr),
                  static_cast<int>(node->lloc.offset));
            fprintf(oil_out, "break_%d_%d_%d:\n",
                  static_cast<int>(node->lloc.filenr),
                  static_cast<int>(node->lloc.linenr),
                  static_cast<int>(node->lloc.offset));


         }
         break;

      case TOK_GT:
         {
            fprintf(oil_out, "        char b%d = ", break_count);
            
            emit_expr(oil_out, node);
         }
         break;

      case TOK_LT:
         {
            fprintf(oil_out, "        char b%d = ", break_count);
            emit_expr(oil_out, node);
         }
         break;   

      case TOK_EQ:
         {
            fprintf(oil_out, "        ");
            fprintf(oil_out, "char b%d", break_count);
            fprintf(oil_out, " = ");
            emit_expr(oil_out, node);
         }
         break;

      case TOK_GE:
         {
            fprintf(oil_out, "        char b%d = ", break_count);
            emit_expr(oil_out, node);
         }
         break;

      case TOK_LE:
         {
            fprintf(oil_out, "        char b%d = ", break_count); 
            emit_expr(oil_out, node);
         }
         break;
      case TOK_INDEX:
         {
            emit_oil(oil_out, node->children[0]);
            fprintf(oil_out,"[");
            emit_oil(oil_out, node->children[1]);
            fprintf(oil_out,"]");
         }
         break;
      case TOK_CALL:
         {
            if(node->children[1]->symbol == TOK_CALL){
               emit_oil(oil_out, node->children[1]);
               fprintf(oil_out, "        %s (",
                     node->children[0]->lexinfo->c_str());
               if (*return_type(node->children[1]) == "int"){
                  fprintf(oil_out, "i%d", i_reg_count-1);
               }else if(*return_type(node->children[1]) == "string"){
                  fprintf(oil_out, "c%d",c_reg_count-1);
               }else if(*return_type(node->children[1]) == "char"){
                  fprintf(oil_out, "c%d", c_reg_count-1);
               }else if(*return_type(node->children[1]) == "void"){
               } 
            }
            else{
               if (*return_type(node->children[0]) == "int"){
                  fprintf(oil_out, "        int i%d = ", 
                        i_reg_count++);
               }else if(*return_type(node->children[0]) == "string"){
                  fprintf(oil_out, "        char s%d = ", 
                        c_reg_count++);
               }else if(*return_type(node->children[0]) == "char"){
                  fprintf(oil_out, "        char i%d = ", 
                        c_reg_count++);
               }else if(*return_type(node->children[0]) == "void"){
                  fprintf(oil_out, "        ");
               } 

               fprintf(oil_out, "%s (",
                     node->children[0]->lexinfo->c_str());
              
            }
            if(node->children[1]->symbol != TOK_CALL){
               emit_oil(oil_out, node->children[1]);
            }
            fprintf(oil_out, ");\n");

            
         }
         break;

      case TOK_NEW:
         {

         }
         break;
   }

}

void oil_traverse_root(FILE* oil_out, astree* root)
{
   emit_string_cons(oil_out, root);
   for (auto child:root->children)
   {
      emit_oil(oil_out, child);
   }
}
