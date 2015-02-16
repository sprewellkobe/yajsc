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
ע��js�﷨֧�ֺ�������Ƕ��,��Ҫ����prescan������������
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
	  //cout<<"level��:"<<level+1<<endl;
	  iter=children.insert(iter,make_pair(name,ftntemp));
	  *ftn=&(iter->second);
	 }
   else//�������ض���(��������),js֧������
	 {
      (iter->second).addr=addr;
	  (iter->second).code_level=codelevel;
	  (iter->second).children.clear();
	  *ftn=(FNTreeNode*)(&(iter->second));
	 }
  }
 //----------------------------------------------
  //ͨ��name����ӽڵ��еĺ�����ַ,codelevelΪ���뻷��ֵ
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
  FNTreeNode root;//���ڵ�tree_level��-1
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
 void CheckRuntimeStackSize();//��ֹ����ջ���

private:

 vector<IntValue> runtime_stack;//����ջ,�Ĵ���ջ,ȫ��
 
 map<int,string>& symbol_table;//�������﷨���������ű�,ȫ��
 
 TSymbolTable runtime_table;//�����ڷ��ű�,��Ӧ��������

 stack<map<string,int> > labellist;//��ת��,��Ӧ��������

 //map<string,int> functionmap;//������(������ַ),ȫ��
 FNTree functiontree;


 stack<IntValue> function_stack;//��������ջ,ȫ��

 stack<IntValue> with_stack;//with����ջ

 string preobjname;//��A.B.Cʱ,��¼���һ��C��name

//���ж�ԭ��,��������ʱ���������Ͷ���,ȫ��
 THeap runtime_heap;

 TSimulation* sim;
//-------------------------------------------------------------------
private:
 bool IsID(const IntValue& number)//�Ƿ���ID
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

 bool IsLocation(const string& str)//�ж��Ƿ����,��Ϊ������,parent.frame2.location.href
 {
  if(str=="location")
	 return true;
  return false;
 }
//-------------------------------------------------------------------
 
 int GetIVType(const IntValue& iv)//����������
 {
  return iv.type;
 }

 string GetNameByID(const int& number);//���ID������
//-------------------------------------------------------------------
private://ָ�����
 bool command_inttype(const int& command,const IntValue& c2,const int& env,int& sp);//��Ŀ
 bool command_vartype(const int& command,const IntValue& c2,const int& env,int& sp);// var
 
 bool command_ippiss(const int& command,const IntValue& c2,const int& env,int& sp);//��Ŀ
 bool command_ppissi(const int& command,const IntValue& c2,const int& env,int& sp);//��Ŀ
 
 bool command_negoppchar(const int& command,const IntValue& c2,const int& env,int& sp);//ȡ��,��Ŀ
 
 //����� + - * / % 
 bool command_operation(const int& command,const IntValue& c1,const IntValue& c2,const int& env,int& sp);//˫Ŀ
 
//λ����� & | ^ >> << >>>
 bool command_bitchar(const int& command,const IntValue& c1,const IntValue& c2,const int& env,int& sp);

 //��ϵ��
 bool command_relation(const int& command,const IntValue& c1,const IntValue& c2,const int& env,int& sp);
 
 bool command_set(const int& command,const IntValue& c1,const IntValue& c2,const int& env,int& sp,
	              vector<int>& code,vector<int>& codeenv);//˫Ŀ
 // =,+=,-=,*=,/= %= <<= >>= >>>= &= |= ^=��ֵ,˫Ŀ
 bool command_heapset(const int& command,const IntValue& c1,const IntValue& c2,const int& env,int& sp);

 //�߼������,˫Ŀ && ||
 //bool command_logic(const int& command,const IntValue& c1,const IntValue& c2,const int& env,int& sp);

 //��תָ��,˫Ŀ
 bool command_btbfchar(const int& command,const IntValue& c1,const IntValue& c2,const int& env,int& sp);
 //��תָ��,��Ŀ
 bool command_blchar(const int& command,const IntValue& c2,const int& env,int& sp);
 //labelָ��,��Ŀ
 bool command_label(const int& command,const IntValue& c2,const int& env,int& sp);
 //typeofָ��,��Ŀ
 bool command_typeofchar(const int& command,const IntValue& c2,const int& env,int& sp);

//EAָ��,��Ŀ
/*��������ԭ��:
  ����ջ�ṹ,            ������,����1,����2...,NUM,EA
                ������,������,.,����1,����2...,NUM,EA
  ���䵥���ĺ�������ջ,����������ջ
  ջ������caller����
  ջ������callee����,�˳�ʱ����
  ��������ջ�ṹ,    NUM(sp�µ�ַ),RET,NUM(����ջ��ַ,�Ӹõ�ַ��(����)��ջ���),BP,����N,...����1

  ��������ֹ����EF
*/
bool command_eachar(const int& command,const IntValue& c2,const int& env,int& sp);
bool command_functionchar(const int& command,const IntValue& c2,const int& env,int& sp);
bool command_inichar(const int& command,const IntValue& c2,const int& env,int& sp);
bool command_return(const int& command,const IntValue& c2,const int& env,int& sp);

bool command_unfunctionchar(const int& command,const IntValue& c2,const int& env,int& sp);//��������
bool command_unefchar(const int& command,const IntValue& c2,const int& env,int& sp);

//��Ա������������
bool command_eachar2(const int& command,const IntValue& c2,const int& env,int& sp);
bool function_reflection(IntValue& iv,const IntValue& object,const IntValue& func,
	                     vector<IntValue>& argumentslist,
	                     const int& env,const int sp,bool& isreturn);
bool command_dotchar(const int& command,const IntValue& c1,const IntValue& c2,const int& env,int& sp);
bool command_withchar(const int& command,const IntValue& c2,const int& env,int& sp);
bool command_quitwithchar(const int& command,const IntValue& c2,const int& env,int& sp);

//�ѷ���ָ���
bool command_newchar(const int& command,const int& env,int& sp);//����,��Ŀ
bool command_offsetchar(const int& command,const IntValue& c1,const IntValue& c2,const int& env,int& sp);

//������URL��ת���õĺ���
bool meet_jump_func(vector<IntValue>& argumentslist,const int& env,const int sp,bool isopen=false);

//-------------------------------------------------------------------
private:
 void enter_function();
 void quit_function();
 
 //treelevelΪ��functiontree����Ĳ���,0��ʾȫ�ֺ���
 int GetFunctionAddr(const string& funcname,int& treelevel,int& codelevel);
 
private:
 /*
 ������ת���,��ʽΪ:
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
 int infunction;//����sp����δִ�к���
 int inunfunction;
 stack<int> endnewpos;//��¼ENEWָ���ַ
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
