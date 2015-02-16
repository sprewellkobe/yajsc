//---------------------------------------------------------------------------------------
//Simulate Javascript kernel object, by kobe
#ifndef SIMULATIONHPP
#define SIMULATIONHPP
//---------------------------------------------------------------------------------------
#include <string>
#include <vector>
#include "Common.hpp"
#include "Error.hpp"
#include "Heap.hpp"
#include "SymbolTable.hpp"
#include "Main.hpp"
#include <fstream>
using namespace std;

//---------------------------------------------------------------------------------------
const int gfn_size=31;

const string gfn[gfn_size]=//js所有全局函数
{
"Array",//0
"Function",//1
"Number",//2
"String",//3
"addClient",//4
"addResponseHeader",//5
"alert",//6
"blob",//7
"callC",//8
"confirm",//9
"debug",//10
"deleteResponseHeader",//11
"escape",//12
"eval",//13
"flush",//14
"getOptionValue",//15
"getOptionValueCount",//16
"isNaN",//17
"parseFloat",//18
"parseInt",//19
"prompt",//20
"redirect",//21
"registerCFunction",//22
"setTimeout",//23
"ssjs_generateClientID",//24
"ssjs_getCGIVariable",//25
"ssjs_getClientID",//26
"taint",//27
"unescape",//28
"untaint",//29
"write",//30
};//this array must be in order
//---------------------------------------------------------------------------------------

class TSimulation
{
 private:
  map<int,string>& symbol_table;
  TSymbolTable* runtime_table;
  THeap* runtime_heap;
  vector<string> gfn_vec;
  string& jump_result;
  
  string ustr;
 private:
  void AddOneResult(const string& type,const string& url)
  {
   if(url!=ustr)
      jump_result+=type+" "+url+"\n";
   else
	  jump_result+="UNKNOWNJMP "+url+"\n";
  }
 public:
  TSimulation(map<int,string>& st,TSymbolTable* rst,THeap* rhp,string& str):
	         symbol_table(st),runtime_table(rst),runtime_heap(rhp),jump_result(str)
  {
   for(int i=0;i<gfn_size;i++)
	   gfn_vec.push_back(gfn[i]);
   ustr=UNKNOWNSITE.substr(1);
   /*sort(gfn_vec.begin(),gfn_vec.end());
   ofstream ofs("gfn");
   vector<string>::iterator vi=gfn_vec.begin();
   for(;vi!=gfn_vec.end();vi++)
      ofs<<*vi<<endl;
   ofs.close();*/
  }

  ~TSimulation()
  {
   //gfn_vec.clear();
   runtime_heap=NULL;
   runtime_table=NULL;
  }
 public:
  //模拟String类
  bool SimulateString(IntValue& iv,bool func,TItem* object,const string& ostr,const string& method,
	                  vector<IntValue>* al,const int& sp,const int& env,int strindex=0);
  
  //模拟Number类
  bool SimulateNumber(IntValue& iv,bool func,TItem* object,const string& method,
	                  vector<IntValue>* al,const int& sp,const int& env);

  //模拟静态成员
  bool SimulateStatic(IntValue& iv,bool func,const string& classname,IntValue* withobject,const string& method,
	                  vector<IntValue>* al,const int& sp,const int& env);
  
  //模拟数操作,比如toString()
  bool SimulateInt(IntValue& iv,bool func,TItem* object,const IntValue& oiv,const string& method,
	                   vector<IntValue>* al,const int& sp,const int& env);

  //模拟Array类
  bool SimulateArray(IntValue& iv,bool func,TItem* object,const IntValue& oiv,const string& method,
	                 vector<IntValue>* al,const int& sp,const int& env,bool& isreturn);
  
  //模拟Date类
  bool SimulateDate(IntValue& iv,bool func,TItem* object,const IntValue& oiv,const string& method,
	                 vector<IntValue>* al,const int& sp,const int& env);

  //判断是否为全局函数
  int IsGlobalFunction(const string& funcname);

  //模拟全局函数
  bool SimulateGlobalFunction(int function_index,IntValue& iv,const string& method,
	                          vector<IntValue>* al,const int& sp,const int& env,bool& isreturn);

  string GetNameByID(const int& number);
};

//---------------------------------------------------------------------------------------
#endif
