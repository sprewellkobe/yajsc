/*--------------------------------------------------------------------------------------------
Yet Another JS Compiler, by kobe, 2006,9
global token,command
--------------------------------------------------------------------------------------------*/
#ifndef BTESTHPP
#define BTESTHPP
#include <map>
#include <string>
using namespace std;
//------------------------------------------------------------------------
/*
单目指令符
+ -(减和负号) * / ++ -- BL
>= <= == < > != 

双目指令符
= += -= *= /= BF
*/
extern int bfchar;//BF
extern int bf2char;//特殊的BF指令,修改为和BF同意义了
extern int blchar;//BL
extern int btchar;//BT
extern int labelchar;//LABEL
extern int functionchar;//FUNCTION
extern int unfunctionchar;//匿名FUNCTION对象

extern int inichar;//INITIALIZE
extern int efchar;//EF END FUNC
extern int unefchar;//匿名FUNCTION对象终止

extern int newchar;//NEW
extern int endnewchar;//END NEW
extern int offsetchar;//偏移指令,用做数组的[X]

//运算符
extern int pluschar;//+
extern int subchar;//-
extern int multichar;//*
extern int divchar;// /
extern int negchar;//单目运算符,取反 -
extern int oppchar;//单目运算符,取反 !
extern int modchar;//双目,取余数 %
//双目位运算符
extern int leftchar;// <<
extern int rightchar;// >>
extern int rightrightchar;// >>>
extern int andbchar;// &
extern int orbchar;// |
extern int inbchar;// ^
//逻辑运算符
extern int andchar;// &&
extern int orchar;// ||
//关系符
extern int gtechar;//>=
extern int ltechar;//<=
extern int eqchar;//==
extern int noteqchar;//!=

extern int gtchar;//>
extern int ltchar;//<
//单目赋值符
extern int ipp;//i++
extern int ppi;//++i
extern int iss;//i--
extern int ssi;//--i
//双目赋值符
extern int setchar;//=
extern int pe;// +=
extern int se;// -=
extern int me;// *=
extern int de;// /=
extern int mode;// %=
extern int lefte;// <<=
extern int righte;// >>=
extern int rightee;// >>>=
extern int ande;// &=
extern int ore;// |=
extern int ine;// ^=
//函数控制指令
extern int eachar;//单目,获得参数个数,用于全局函数
extern int eachar2;//双目,获得参数个数,用于成员函数
extern int retchar;//单目,返回sp地址
extern int bpchar;//栈基址,运行栈清理末地址
extern int returnchar;//单目
extern int return0char;//直接返回指令
//特殊符
extern int commachar;// ,
extern int dotchar;// .
extern int withchar;//with
extern int quitwithchar;//with终止
extern int nanchar;// NaN

extern int trychar;
extern int quittrychar;
extern int catchchar;
extern int quitcatchchar;
extern int finallychar;
extern int quitfinallychar;

extern int typeofchar;// typeof

//变量类型符,单目 item使
extern int inttype;
extern int stringtype;
extern int vartype;

//类名,多目
extern int arraytype;
//------------------------------------------------------------------------
extern int id_start_pos;//ID的分配起始值

extern int current_string_index;//递增

extern int current_command_index;//指令

extern int current_id_index;// ID

extern int temp_id_index;//临时变量，函数return时用
extern int array_id_index;

const int empty_string_index=INT_MIN;//空串
const int host_string_index=INT_MIN+1;//location对象的host和hostname
const int path_string_index=INT_MIN+2;//location对象的pathname
const int url_string_index=INT_MIN+3;//document对象的URL
//终止符
const int codeend=-10000;//终止符
//------------------------------------------------------------------------
const int split_number=-10000;//指令和ID的分解线

const int global=0;//初始化环境层数
//------------------------------------------------------------------------

class JSIInterface
{
 private:
   string filename;
   string jscode;
   istream* is;
   string original_code;
   //MyScanner myscanner;
 private:
   string addtional_code;
   string parsing_url;
   string host;
   string path;
 public:
   JSIInterface(const string& file_name):filename(file_name),is(NULL),original_code(""),host(""),path(""){};
   JSIInterface(istream* p_is):filename(""),is(p_is),original_code(""),host(""),path(""){};
   
   //int type is used to avoid construction function override
   JSIInterface(const string& ocode,int type):filename(""),is(NULL),original_code(ocode),host(""),path(""){}
   //---------------------------------------------------------------------
   void Prepare();
   bool Scan();
   string Compile(bool withlog=false);
   void SetAddtionalCode(const string& str);
   void SetURL(const string& str);
};
//------------------------------------------------------------------------
#endif
