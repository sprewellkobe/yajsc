//------------------------------------------------------------------------
//Provide the common value, common function, by kobe
#ifndef COMMONHPP
#define COMMONHPP
#include <time.h>
#include <sys/time.h>
#include <string>
#include <stdio.h>
#include <iostream>
#include <stack>
using namespace std;
//------------------------------------------------------------------------
//公用函数
string IntToStr(int count);
int MonthToInt(string month);

template<class T>
void ClearStack(stack<T>& mystack)
{
 while(!mystack.empty())
	   mystack.pop();
}
//------------------------------------------------------------------------
const int INT_HMIN=INT_MIN/2;
//------------------------------------------------------------------------
const int DEFAULT_ARRAY_SIZE=10;
//------------------------------------------------------------------------
const string UNKNOWNSITE="*unknown_site";
//------------------------------------------------------------------------
const string FUNCTEMP="*FTEMP";
//------------------------------------------------------------------------
const int STACK_SIZE_LIMIT=65536;
//------------------------------------------------------------------------
const int LOOP_MAX=8096;
//------------------------------------------------------------------------
const unsigned int MIN_URL_SIZE=11;//默认最少的url长度 http://a.cn
//------------------------------------------------------------------------

//解析URL,分离出host和path
// http://a.com/h.html#xxx&c=0 => host: a.com pathname:/h.html
bool ParseURL(const string& url,string& host,string& path);

//去除转义符
string TransformString(const string& str);
//------------------------------------------------------------------------
class TItem
{
 public:
  int value;//值
  int type;//对应类型
  int level;//环境层数
};
//------------------------------------------------------------------------

class TItem2 : public TItem //输出专用
{
 public:
  string name;
};
//------------------------------------------------------------------------

namespace TYPE
{
  //编译器类型
  const static int ID=0;
  const static int NUMBER=1;
  const static int STR=2;
  //还有HEAP和DATE,都在heap里
 
  //-------------------------------------------------------
  //运行期类型
  const static int VAR=3;
  const static int INT=4;
  const static int COMMAND=5;
  const static int HEAP=6;//new 专用,value为heaplist地址
  const static int STRING=7;
  const static int FLOAT=8;
  const static int DATE=9;
  const static int LOBJ=10;
  const static int COBJ=11;
  const static int OOBJ=12;
};
//------------------------------------------------------------------------

class POINT
{
 public:
  int x;
  int y;
  POINT():x(-1),y(-1){}
};
class IntValue
{
 public:
  int value;
  int type;
  POINT hp;//堆指针,对象在堆中专用
  //----------------------------------------------------------------------
  IntValue(const int& a,const bool& b):value(a)//为了兼容老函数IsNumber
  {
   if(b)
	 type=TYPE::NUMBER;
   else
	 type=TYPE::ID;
  };

  IntValue(int a=0):value(a),type(TYPE::NUMBER)
  {};

  bool HasHP() const
  {
   if(hp.x!=-1 && hp.y!=-1)
	  return true;
   return false;
  }

  void SetHPNull()
  {
   hp.x=-1;hp.y=-1;
  }
};
//-------------------------------------------------------------------------------------------------
struct timeval BeginTiming();
double EndTiming(const struct timeval& bt);
//-------------------------------------------------------------------------------------------------
#endif
