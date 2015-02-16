#ifndef SYMBOLTABLEHPP
#define SYMBOLTABLEHPP
//------------------------------------------------------------------------------------------
#include <string>
#include <map>
#include <vector>
#include <stack>
#include <iostream>
#include "Common.hpp"
#include "Error.hpp"
using namespace std;
//------------------------------------------------------------------------------------------
class THeap;
class TSymbolTable
{
private:
 map<string,vector<TItem> > items;//map符号表,每个符号对应下推栈结构,全局
 stack< map<string,vector<TItem> > > funcitems_stack;
 //bool infunc;//判断是否在函数内

public:
 bool GetItemByName(const string& name,TItem** item);

public:
 void Print();
 vector<TItem2> GetOutput();

//-------------------------------------------------------------------
//在函数内开关
void EnterFunc();

 void QuitFunc();
//-------------------------------------------------------------------

private:
 void InitializeTemp();

public:
 THeap* runtime_heap;//传入的运行堆,用于标识空的堆地址

public:
 TSymbolTable();
 void ClearUp();
 int GetTableSize();
 //void DisplayTable();

 bool IfSymbolExists(const string& name, map<string,vector<TItem> >::iterator& iter,int& which);
 //某一符号是否存在,which为0表明在全局发现,1表明在当前函数栈里

 bool AddSymbol(const string& name,const TItem& item);
 bool AddTempSymbol(const TItem& item);//利用临时变量
 bool DeleteLastLevelSymbol(int level);//退出环境,局部变量释放
 bool DeleteOverLevelSymbolByName(int level,const string& name);
 //bool ChangeLastLevelSymbolType(const string& name,const unsigned int& type);//
 //bool ChangeLastLevelSymbolValue(const string& name,const unsigned int& value);
};
#endif
//------------------------------------------------------------------------------------------
