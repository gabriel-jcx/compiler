#ifndef __ATTR_BITSET_H__
#define __ATTR_BITSET_H__

#include <string>
#include <vector>
#include <bitset>
#include <string>
#include <iostream>
#include <unordered_map>
using namespace std;

enum class attr{
       VOID, INT, NULLX, STRING, STRUCT, ARRAY, FUNCTION, VARIABLE,
       FIELD, TYPEID, LVAL, LOCAL, PARAM, CONST, VREG, VADDR, CHAR, 
       BITSET_SIZE,
};

using attr_bitset = bitset<unsigned(attr::BITSET_SIZE)>;
const string attr_to_string (int i);
const string to_string_array(attr_bitset attributes);
#endif
