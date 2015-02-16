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
 map<string,vector<TItem> > items;//map���ű�,ÿ�����Ŷ�Ӧ����ջ�ṹ,ȫ��
 stack< map<string,vector<TItem> > > funcitems_stack;
 //bool infunc;//�ж��Ƿ��ں�����

public:
 bool GetItemByName(const string& name,TItem** item);

public:
 void Print();
 vector<TItem2> GetOutput();

//-------------------------------------------------------------------
//�ں����ڿ���
void EnterFunc();

 void QuitFunc();
//-------------------------------------------------------------------

private:
 void InitializeTemp();

public:
 THeap* runtime_heap;//��������ж�,���ڱ�ʶ�յĶѵ�ַ

public:
 TSymbolTable();
 void ClearUp();
 int GetTableSize();
 //void DisplayTable();

 bool IfSymbolExists(const string& name, map<string,vector<TItem> >::iterator& iter,int& which);
 //ĳһ�����Ƿ����,whichΪ0������ȫ�ַ���,1�����ڵ�ǰ����ջ��

 bool AddSymbol(const string& name,const TItem& item);
 bool AddTempSymbol(const TItem& item);//������ʱ����
 bool DeleteLastLevelSymbol(int level);//�˳�����,�ֲ������ͷ�
 bool DeleteOverLevelSymbolByName(int level,const string& name);
 //bool ChangeLastLevelSymbolType(const string& name,const unsigned int& type);//
 //bool ChangeLastLevelSymbolValue(const string& name,const unsigned int& value);
};
#endif
//------------------------------------------------------------------------------------------
