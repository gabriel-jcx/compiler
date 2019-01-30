#include <unordered_map>
#include <bitset>
#include <string>
#include <iostream>
#include "attr_bitset.h"
using namespace std;
const string attr_to_string (int i) {

   static const unordered_map<int,string> hash {
      {0       , "void "       },
      {1       , "int "        },
      {2       , "null "       },
      {3       , "string "     },
      {4       , ""            },
      {5       , "array "      },
      {6       , "function "   },
      {7       , "variable "   },
      {8       , "field "      },
      {9       , "typeid "     },
      {10      , "lval "       },
      {11      , "local "      },
      {12      , "param "      },
      {13      , "const "      },
      {14      , "vreg "       },
      {15      , "vaddr "      },
      {16      , "char "       },
      {17      , "bitset_size "}
   };
   auto str = hash.find (i);
   if (str == hash.end()) throw invalid_argument (__PRETTY_FUNCTION__);
   return str->second;
   //return store;
}
const string to_string_array(attr_bitset attributes){
   string store;
   //string temp = attributes.to_string();
   for (size_t i = 0; i < attributes.size(); ++i)
   {
      if(attributes[i]){
         store.append(attr_to_string(i));
      }
   }
   return store;
}
