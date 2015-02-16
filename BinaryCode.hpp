#ifndef BINARYCODE
#define BINARYCODE
//-------------------------------------------------------------------------------------------------
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
//-------------------------------------------------------------------------------------------------
using namespace std;
using namespace boost;
const string bc_ext=".jso";
//-------------------------------------------------------------------------------------------------
class BinaryCode
{
 public:
  vector<int> code;
  vector<int> codeenv;
  map<int,string> symbol_table;
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar,const unsigned int version)//boost序列化支持
  {
   ar & code;
   ar & codeenv;
   ar & symbol_table;
  }
 public:
  BinaryCode(const vector<int>& c,const vector<int>& ce,const map<int,string>& st):code(c),codeenv(ce),symbol_table(st)
  {}
  BinaryCode()
  {}
 public:
  static bool SaveToFile(string filename,const BinaryCode& bc);
  static bool LoadFormFile(const string& filename,BinaryCode& bc);
};
//-------------------------------------------------------------------------------------------------
#endif
