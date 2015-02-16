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
//运行堆
class THeap
{
 private:
  vector< vector<IntValue> > heaplist;
  stack<int> vacancy;//记录释放过的可用位置
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
  bool GetArray(const int& addr,vector<IntValue>& vec);//Array的toString()专用
  bool ReverseArray(const int& addr);//Array的reverse()专用
 public:
  void DisplayHeap(map<int,string>& symbol_table,bool order=false);//全打印,或者,按照堆内对象的引用顺序打印
};
//--------------------------------------------------------------------------------------------
#endif
