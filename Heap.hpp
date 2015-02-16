//--------------------------------------------------------------------------------------------
//Runtime Heap, simulate java GC, by kobe
#ifndef HEAPHPP
#define HEAPHPP
//--------------------------------------------------------------------------------------------
#include <string>
#include <vector>
#include <stack>
#include <map>
#include "Common.hpp"
#include "Error.hpp"
using namespace std;
//--------------------------------------------------------------------------------------------
//���ж�
class THeap
{
 private:
  vector< vector<IntValue> > heaplist;
  stack<int> vacancy;//��¼�ͷŹ��Ŀ���λ��
  vector<int> rcount;
  vector<int> recursion_stack; //recode recursion stack
 private:
  void ClearUp();

 public:
  THeap();
  ~THeap();

  vector<IntValue>& GetNew(int& index);
  bool DeleteHeapItem(const int& addr,bool force=false);
  IntValue* GetHeapItem(const int& addr,const int& offset,const int& sp);
  int GetHeapSize();
  bool AddReferenceCount(const int& index);
 private:
  bool AddrExistedInRecursionStack(const int& addr);
 public:
  int GetItemLength(const int& addr);
  bool GetArray(const int& addr,vector<IntValue>& vec);//Array��toString()ר��
  bool ReverseArray(const int& addr);//Array��reverse()ר��
 public:
  void DisplayHeap(map<int,string>& symbol_table,bool order=false);//ȫ��ӡ,����,���ն��ڶ��������˳���ӡ
};
//--------------------------------------------------------------------------------------------
#endif
