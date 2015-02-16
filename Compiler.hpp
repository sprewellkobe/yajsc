/*--------------------------------------------------------------------------------------------
Yet Another JS Compiler, by kobe, 2006,9
Compiler
Stack Push Machine
--------------------------------------------------------------------------------------------*/
#ifndef COMPILERHPP
#define COMPILERHPP
#include <string>
#include <vector>
#include <stack>
#include <map>
#include "Main.hpp"
#include "SymbolTable.hpp"
#include "Common.hpp"
#include "Heap.hpp"
#include "Error.hpp"
#include "Simulation.hpp"
using namespace std;
//-------------------------------------------------------------------

class TMyStack
{
 private:
  stack<int> fstack;
  int sum;
 public:
  TMyStack():sum(0){}
  void push(const int& v){fstack.push(v);sum+=v;}
  void pop()
  {
   int r=fstack.top();
   fstack.pop();
   sum-=r;
  }
  int GetSum(){return sum;}
};
//-------------------------------------------------------------------
/*
注意js语法支持函数定义嵌套,需要利用prescan建立函数名树
*/
 //--------------------------------------------------------
class FNTreeNode//function name tree node
{
 public:
  int addr;
  int tree_level;
  int code_level;
  FNTreeNode* parent;
  map<string,FNTreeNode> children;

 public:
  FNTreeNode():addr(-1),tree_level(-1),code_level(0),parent(NULL){}
  FNTreeNode(const int& p_addr,const int& p_level,const int& c_level,FNTreeNode* p_parent):
             addr(p_addr),tree_level(p_level),code_level(c_level),parent(p_parent){}

 public:
  
  void AddFunctionChild(const string& name,const int& addr,const int& codelevel,FNTreeNode **ftn)
  {
   map<string,FNTreeNode>::iterator iter=children.find(name);
   if(iter==children.end())
	 {
      FNTreeNode ftntemp(addr,tree_level+1,codelevel,this);
	  //cout<<"level是:"<<level+1<<endl;
	  iter=children.insert(iter,make_pair(name,ftntemp));
	  *ftn=&(iter->second);
	 }
   else//函数名重定义(不是重载),js支持特性
	 {
      (iter->second).addr=addr;
	  (iter->second).code_level=codelevel;
	  (iter->second).children.clear();
	  *ftn=(FNTreeNode*)(&(iter->second));
	 }
  }
 //----------------------------------------------
  //通过name获得子节点中的函数地址,codelevel为代码环境值
  int GetFunctionAddr(const string& name,FNTreeNode** ftn)
  {
   map<string,FNTreeNode>::iterator iter=children.find(name);
   if(iter==children.end())
     {
	  *ftn=NULL;
	  return -1;
	 }
   *ftn=&(iter->second);
   //codelevel=(iter->second).code_level;
   return (iter->second).addr;
  }
};//end class FNTreeNode
 //--------------------------------------------------------

class FNTree
{
public:
  FNTreeNode root;//根节点tree_level是-1
private:
  FNTreeNode* active_node;

public:
  FNTree()
  {
   active_node=&root;
  }

public:
 //----------------------------------------------
 
 FNTreeNode* AddFunction(const string& name,const int& addr,const int& codelevel)
 {
  FNTreeNode* node;
  active_node->AddFunctionChild(name,addr,codelevel,&node);
  return node;
 }
 //----------------------------------------------

 int GetFunctionAddrByName(const string& name,FNTreeNode** fnt)
 {
  FNTreeNode* cnode=active_node;
  while(true)
       {
	    int addr=cnode->GetFunctionAddr(name,fnt);
	    if(addr!=-1)
		   return addr;
	    if(cnode->parent!=NULL)
		   cnode=cnode->parent;
	    else
		   break;
	   }
  return -1;
 }
 //----------------------------------------------
 void ResetActiveNode()
 {
  active_node=&root;
 }

 void EnterActiveNode(FNTreeNode& node)
 {
  active_node=&node;
 }

 void QuitActiveNode()
 {
  if(active_node->parent!=NULL)
	 active_node=active_node->parent;
 }
};//end class FNTree
//---------------------------------------------------------------------------------------

class Compiler
{
private:
 //void ShowError(const int& errortype,const int& sp,const IntValue& iv,const string& name);
 bool do_command(const int& command,const int& env,int& sp,vector<int>& code,vector<int>& codeenv);
 bool PreScan(vector<int>& code,vector<int>& codeenv);
 void CheckRuntimeStackSize();//防止运行栈溢出

private:

 vector<IntValue> runtime_stack;//运行栈,寄存器栈,全局
 
 map<int,string>& symbol_table;//传来的语法分析器符号表,全局
 
 TSymbolTable runtime_table;//编译期符号表,对应函数环境

 stack<map<string,int> > labellist;//跳转表,对应函数环境

 //map<string,int> functionmap;//函数表(名到地址),全局
 FNTree functiontree;


 stack<IntValue> function_stack;//函数调用栈,全局

 stack<IntValue> with_stack;//with调用栈

 string preobjname;//在A.B.C时,记录最后一个C的name

//运行堆原则,负责运行时分配的数组和对象,全局
 THeap runtime_heap;

 TSimulation* sim;
//-------------------------------------------------------------------
private:
 bool IsID(const IntValue& number)//是否是ID
 {
  if(number.type==TYPE::ID)
	{
     if(number.value<split_number)
		return true;
	 else
		cout<<"Fatal ERROR!"<<endl;
	}
  return false;
 }//end IsID

 bool IsLocation(const string& str)//判断是否可疑,因为有这种,parent.frame2.location.href
 {
  if(str=="location")
	 return true;
  return false;
 }
//-------------------------------------------------------------------
 
 int GetIVType(const IntValue& iv)//编译期类型
 {
  return iv.type;
 }

 string GetNameByID(const int& number);//获得ID的名字
//-------------------------------------------------------------------
private://指令操作
 bool command_inttype(const int& command,const IntValue& c2,const int& env,int& sp);//单目
 bool command_vartype(const int& command,const IntValue& c2,const int& env,int& sp);// var
 
 bool command_ippiss(const int& command,const IntValue& c2,const int& env,int& sp);//单目
 bool command_ppissi(const int& command,const IntValue& c2,const int& env,int& sp);//单目
 
 bool command_negoppchar(const int& command,const IntValue& c2,const int& env,int& sp);//取反,单目
 
 //运算符 + - * / % 
 bool command_operation(const int& command,const IntValue& c1,const IntValue& c2,const int& env,int& sp);//双目
 
//位运算符 & | ^ >> << >>>
 bool command_bitchar(const int& command,const IntValue& c1,const IntValue& c2,const int& env,int& sp);

 //关系符
 bool command_relation(const int& command,const IntValue& c1,const IntValue& c2,const int& env,int& sp);
 
 bool command_set(const int& command,const IntValue& c1,const IntValue& c2,const int& env,int& sp,
	              vector<int>& code,vector<int>& codeenv);//双目
 // =,+=,-=,*=,/= %= <<= >>= >>>= &= |= ^=赋值,双目
 bool command_heapset(const int& command,const IntValue& c1,const IntValue& c2,const int& env,int& sp);

 //逻辑运算符,双目 && ||
 //bool command_logic(const int& command,const IntValue& c1,const IntValue& c2,const int& env,int& sp);

 //跳转指令,双目
 bool command_btbfchar(const int& command,const IntValue& c1,const IntValue& c2,const int& env,int& sp);
 //跳转指令,单目
 bool command_blchar(const int& command,const IntValue& c2,const int& env,int& sp);
 //label指令,单目
 bool command_label(const int& command,const IntValue& c2,const int& env,int& sp);
 //typeof指令,单目
 bool command_typeofchar(const int& command,const IntValue& c2,const int& env,int& sp);

//EA指令,单目
/*函数调用原则:
  运行栈结构,            函数名,参数1,参数2...,NUM,EA
                对象名,函数名,.,参数1,参数2...,NUM,EA
  分配单独的函数调用栈,独立于运行栈
  栈分配由caller负责
  栈清理由callee负责,退出时清理
  函数调用栈结构,    NUM(sp新地址),RET,NUM(运行栈地址,从该地址后(包含)的栈清空),BP,参数N,...参数1

  函数体终止符号EF
*/
bool command_eachar(const int& command,const IntValue& c2,const int& env,int& sp);
bool command_functionchar(const int& command,const IntValue& c2,const int& env,int& sp);
bool command_inichar(const int& command,const IntValue& c2,const int& env,int& sp);
bool command_return(const int& command,const IntValue& c2,const int& env,int& sp);

bool command_unfunctionchar(const int& command,const IntValue& c2,const int& env,int& sp);//匿名函数
bool command_unefchar(const int& command,const IntValue& c2,const int& env,int& sp);

//成员函数调用命令
bool command_eachar2(const int& command,const IntValue& c2,const int& env,int& sp);
bool function_reflection(IntValue& iv,const IntValue& object,const IntValue& func,
	                     vector<IntValue>& argumentslist,
	                     const int& env,const int sp,bool& isreturn);
bool command_dotchar(const int& command,const IntValue& c1,const IntValue& c2,const int& env,int& sp);
bool command_withchar(const int& command,const IntValue& c2,const int& env,int& sp);
bool command_quitwithchar(const int& command,const IntValue& c2,const int& env,int& sp);

//堆分配指令函数
bool command_newchar(const int& command,const int& env,int& sp);//特殊,多目
bool command_offsetchar(const int& command,const IntValue& c1,const IntValue& c2,const int& env,int& sp);

//遇到起URL跳转作用的函数
bool meet_jump_func(vector<IntValue>& argumentslist,const int& env,const int sp,bool isopen=false);

//-------------------------------------------------------------------
private:
 void enter_function();
 void quit_function();
 
 //treelevel为在functiontree里面的层数,0表示全局函数
 int GetFunctionAddr(const string& funcname,int& treelevel,int& codelevel);
 
private:
 /*
 保存跳转结果,格式为:
 JUMP URL/n
 POP URL/n
 DEAD URL\n
 */
 string jump_result;
 void AddOneResult(const string& type,const string& url)
 {
  if(url!=UNKNOWNSITE.substr(1))
     jump_result+=type+" "+url+"\n";
  else
	 jump_result+="UNKNOWNJMP "+url+"\n";
 }
 
private:
 map<int,int> jumpmap;
 bool AddToJumpmap(const int& addr);

private:
 void Initialize();
 int infunction;//控制sp跳过未执行函数
 int inunfunction;
 stack<int> endnewpos;//记录ENEW指令地址
 int funcenv;

private:

public:
 Compiler(map<int,string>& st):symbol_table(st),funcenv(0){};
 bool compile(vector<int>& code,vector<int>& codeenv);
 void PrintValueInStack();
 int GetStackSize();
 void PrintValueInTable();
 void PrintValueInHeap(bool order=false);
 void Close();
 string GetResult()
 {
  return jump_result;
 }
};
//-------------------------------------------------------------------
#endif
