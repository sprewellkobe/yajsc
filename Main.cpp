/*--------------------------------------------------------------------------------------------
Yet Another JS Compiler, by kobe, 2006,9
syntax LL(1) by boost::spirit
Target Code, RPN
--------------------------------------------------------------------------------------------*/
#define BOOST_SPIRIT_DUMP_PARSETREE_AS_XML

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <boost/regex.hpp>
#include <boost/spirit/utility/regex.hpp>

#include <boost/spirit/core.hpp>
#include <boost/spirit/tree/parse_tree.hpp>
#ifdef BOOST_SPIRIT_DUMP_PARSETREE_AS_XML
#include <boost/spirit/tree/tree_to_xml.hpp>
#include <boost/spirit/dynamic/if.hpp>
#endif

#include <stack>
#include <vector>
#include "Common.hpp"
#include "Main.hpp"
#include "Compiler.hpp"
#include "Error.hpp"
#include <sys/time.h>
#include <time.h>
#include <iomanip>
#include "BinaryCode.hpp"
using namespace std;
using namespace boost;
using namespace boost::spirit;
//--------------------------------------------------------------------------------------------

const int EXIT_REASON_MATCHFAIL=0;
const int EXIT_REASON_COMPILEFAIL=1;
const int EXIT_REASON_SUCCESS=2;

//语言环境类型
const int FORWHILE=-11;// FOR和WHILE循环
const int DOWHILE=-12;//DO-WHILE循环
const int IF=-13;//IF语句
const int SEL=-14;// A>B?C:D
const int SWITCH=-15;// switch语句
const int LOGICALAND=-16;
const int LOGICALOR=-17;

//---------------------------------------------------------------------------------------------
class DoubleJumpList
{
public:
 vector<unsigned int> BFList;//假跳转链表
 vector<unsigned int> BLList;//无条件跳转链表
 vector<unsigned int> BTList;//真条件跳转
 vector<int> tempcode;//缓冲区,为了回填地址 for循环专用
 vector<unsigned int> continuelist;//回填continue,for循环专用

public:
 int envtype;//当前环境类型

 int startpos;//for前缀码起始地址
 bool isforin;//是不是forin语句
public:
 DoubleJumpList(const int& env):startpos(-1),isforin(false)
 {envtype=env;}
};
//--------------------------------------------------------------------------------------------
//扫描器
class MyScanner//扫描器,去除注释,检查引号匹配
{
 private:
  bool inscomment;// 在//注释里
  //bool inscomment2;// 在<!-- 轮播对联 结束 -->注释里
  bool inmcomment;// 在/* */注释里
  int dquote;// 双引号数
  int squote;// 单引号数
 public:
  MyScanner():inscomment(false),inmcomment(false),dquote(0),squote(0){}

  string GetSourceFromFilename(const string& filename);
  string GetSourceFromStream(istream& ifs);
  string GetSourceFromString(const string& str);

  bool IsNewLineChar(const char& ch)
  {return (ch==10 || ch==13);}

  void TrimString(string& str);
 private:
  void ClearUp();
};
//-------------------------------------------------------------------

void MyScanner::TrimString(string& str)
{
 while(str.empty()==false && (str[0]==' ' || str[0]==10 || str[0]==13))
	    str.erase(str.begin());
 while(str.empty()==false && (*(str.end()-1))==' ')
		str.erase(str.end()-1);
}
//-------------------------------------------------------------------

void MyScanner::ClearUp()
{
 inscomment=false;inmcomment=false;
 dquote=0;squote=0;
}
//-------------------------------------------------------------------

string MyScanner::GetSourceFromString(const string& str)
{
 istringstream iss(str);
 return GetSourceFromStream(iss);
}
//-------------------------------------------------------------------

string MyScanner::GetSourceFromStream(istream& ifs)
{
 ClearUp();
 string result="";
 unsigned char prechar=0;
 unsigned char preprechar=0;
 while(true)
	  {
	   char c;
	   if(!ifs.get(c))
		   break;
	   c=(unsigned char)c;
	   switch(c)
		     {
		      case '\r'://windows
				   {
			       //if(inmcomment==false)
					 {
			         // inscomment=false;
				     // result+=c;
					 }
				   break;
				   }
              case '\n':
				   {
				   if(inmcomment==false)
					 {
				      inscomment=false;
					  //inscomment2=false;
					  if(*(result.end()-1)!='\n')
				         result+=c;
					 }
			       break;
				   }
              case '"':
				   {
				   if(inscomment || inmcomment )
				      break;
				   if(prechar!='\\')//起作用
				     {
					  if(dquote>0)//在双引号中
						 dquote--;
					  else
						 dquote++;
					 }
				   result+=c;
                   break;
				   }
              case '\'':
				   {
				   if(inscomment || inmcomment)
				      break;
			       if(prechar!='\\')//起作用
				     {
					  if(squote>0)
                        squote--;
					  else
                        squote++;
					 }
                   result+=c;
				   break;
				   }
              case '/':// //
				   {
                   
				   if(prechar=='/' && squote==0 && dquote==0 && 
				      inscomment==false && inmcomment==false)
				     {
				      inscomment=true;
			          result.erase(result.size()-1,1);//删除最后一个/
					 }
                   else if(prechar=='*' && squote==0 && dquote==0 &&//在多行注释内
                       inscomment==false && inmcomment==true)
				     {
					  inmcomment=false;
					 }
                   else if(inmcomment==false && inscomment==false)
					 result+=c;
                   break;
				   }
			  case '*':// /*
				   {
				   if(prechar=='/' && squote==0 && dquote==0 &&
				      inscomment==false)
				     {
				      inmcomment=true;
                      result.erase(result.size()-1,1);//删除最后一个/
					 }
                    else if(inmcomment==false && inscomment==false)
					  result+=c;
					break;
				   }
			  case '}':
				   {
				   if(inscomment==false && inmcomment==false && squote==0 && dquote==0)
					 {
					  if(*(result.end()-1)!='\n')
				         result+="\n";
					 }
				   if(inscomment==false && inmcomment==false)
					  result+=c;
				   break;
				   }
			  case 0xBB://全角;变半角
				   {
				    if(prechar==0xA3 && inscomment==false && inmcomment==false &&
					   squote==0 && dquote==0)
					  {
					   result.erase(result.size()-1,1);
					   result+=";";
					  }
					else if(inscomment==false && inmcomment==false)
					  result+=c;
					break;
				   }
			  case '-':// 处理<!--
				   {
				   if(inscomment==false && inmcomment==false)
					 {
					  if(squote==0 &&dquote==0)
						{
						 if(result.size()>=3 && result.substr(result.size()-3)=="<!-")
						   {
							inscomment=true;
							result.erase(result.size()-3,3);
						   }
						 else
						   result+=c;
						}
					  else
						result+=c;
					 }
				   break;
				   }
			  case '>':// 处理-->
				   {
				   if(prechar=='-'&& preprechar=='-' && squote==0 && dquote==0 &&
					  inscomment==false && inmcomment==false)
					 {
                      result.erase(result.size()-2,2);
					 }
				   else if(inscomment==false && inmcomment==false)
				      result+=c;
				   break;
				   }
			  default:
				   {
				   if(inscomment==false && inmcomment==false)
				      result+=c;
			       break;
				   }
			 }//end switch c
       //if(inscomment==false && inmcomment==false)
	   preprechar=prechar;
       if(prechar=='\\' && c=='/')//NOTICE: // bug found 27,Apr,2007
          prechar=' ';
       else
          prechar=c;
	  }//end while
 if(dquote==0 && squote==0)
   {
	TrimString(result);
	if(result[result.size()-1]!=13)
	   result=result+"\n";
    return result;
   }

TrimString(result);
if(result[result.size()-1]!=13)
   result=result+"\n";
 return result;
 //cout<<"Scanner Error: quote not match!"<<endl;
 //return "";
}
//-------------------------------------------------------------------

string MyScanner::GetSourceFromFilename(const string& filename)
{
 ifstream ifs(filename.c_str());
 string result=GetSourceFromStream(ifs);
 ifs.close();
 return result;
}
//--------------------------------------------------------------------------------------------
/*
单目指令符
+ -(减和负号) * / ++ -- BL
>= <= == < > != 

双目指令符
= += -= *= /= BF
*/

/*
编译期地址分配原则:
[0,INT_MAX] : 操作数 正数正常显示,负数以NEG显示
split_number : -10000;
codeend : -10000

(spilt_number,0) : 指令(current_command_index),从spilt_number+1开始递增(包含)

[INT_HMIN,split_number) : 符号ID
其中:
  temp_id_index=spilt_number-1;
  current_id_index 从spilt_number-2开始递减(包含)

[INT_MIN,INT_HMIN) : 字符串地址
  current_string_index 从INT_MIN+3开始递增(包含)
*/

int current_command_index=split_number+1;//指令

int temp_id_index=split_number-1;//函数临时变量
int array_id_index=split_number-2;//Array字符串

int current_id_index=split_number-3;//符号ID

//int empty_string_index=INT_MIN;
//host_string_index=INT_MIN+1;
//path_string_index=INT_MIN+2;
//url_string_index=INT_MIN+3;
int current_string_index=INT_MIN+4;//递增

int bfchar=current_command_index++;//BF
int bf2char=current_command_index++;//BF2
int blchar=current_command_index++;//BL
int btchar=current_command_index++;//BT
int labelchar=current_command_index++;//LABEL
int functionchar=current_command_index++;//FUNCTION
int unfunctionchar=current_command_index++;//匿名FUNCTION
int inichar=current_command_index++;//INI
int efchar=current_command_index++;//EF END FUNCTION
int unefchar=current_command_index++;//EF 匿名FUNCTION
int newchar=current_command_index++;//NEW
int endnewchar=current_command_index++;//END NEW
int offsetchar=current_command_index++;// []
//运算符
int pluschar=current_command_index++;//+
int subchar=current_command_index++;//-
int multichar=current_command_index++;//*
int divchar=current_command_index++;// /
int negchar=current_command_index++;//单目运算符,取反 -
int oppchar=current_command_index++;//单目运算符,取反 !
int modchar=current_command_index++;// %
//逻辑运算符
int andchar=current_command_index++;// &&
int orchar=current_command_index++;// ||
//关系符
int gtechar=current_command_index++;//>=
int ltechar=current_command_index++;//<=
int eqchar=current_command_index++;//==
int noteqchar=current_command_index++;//!=

int gtchar=current_command_index++;//>
int ltchar=current_command_index++;//<
//单目赋值符
int ipp=current_command_index++;//i++
int ppi=current_command_index++;//++i
int iss=current_command_index++;//i--
int ssi=current_command_index++;//--i
//双目赋值符
int setchar=current_command_index++;//=
int pe=current_command_index++;// +=
int se=current_command_index++;// -=
int me=current_command_index++;// *=
int de=current_command_index++;// /=
int mode=current_command_index++;// %=
int lefte=current_command_index++;// <<=
int righte=current_command_index++;// >>=
int rightee=current_command_index++;// >>>=
int ande=current_command_index++;// &=
int ore=current_command_index++;// |=
int ine=current_command_index++;// ^=
//双目位运算符
int leftchar=current_command_index++;// <<
int rightchar=current_command_index++;// >>
int rightrightchar=current_command_index++;// >>>
int andbchar=current_command_index++;// &
int orbchar=current_command_index++;// |
int inbchar=current_command_index++;// ^
//函数控制指令
int eachar=current_command_index++;// EA 调全局函数
int eachar2=current_command_index++;// EA2 调成员函数
int retchar=current_command_index++;// RET
int bpchar=current_command_index++;// BP
int returnchar=current_command_index++;// RETURN
int return0char=current_command_index++;// RETURN0
//类型符,单目
int inttype=current_command_index++;// int
int stringtype=current_command_index++;// string
int vartype=current_command_index++;// var

//类名
int arraytype=current_command_index;// Array
//特殊符
int commachar=current_command_index++;// ,
int dotchar=current_command_index++;// ,
int withchar=current_command_index++;// with
int quitwithchar=current_command_index++;// quit with
int nanchar=current_command_index++;// NaN

int trychar=current_command_index++;
int quittrychar=current_command_index++;
int catchchar=current_command_index++;
int quitcatchchar=current_command_index++;
int finallychar=current_command_index++;
int quitfinallychar=current_command_index++;

int typeofchar=current_command_index++;
//---------------------------------------------------------------------------------------------

vector<int> code;//代码
vector<int> codeenv;//代码作用域

bool proxy=false;//放入缓存
stack<int> member_function;//区别取属性和member_function


vector<DoubleJumpList> jumplist;//跳转表
int current_level=global;//当前环境层数

stack<string> setopstack;//赋值符栈

map<int,string> symbol_table;//符号表

int inforwhile=0;//是否在循环语句中,检查break,contine语句使
int bclabel;//记录break和continue的跳转标号
stack<int> arguments_number;//跟踪参数数目
bool parse_flag=true;

//----------------------------------------------------------------------------------------------

void Initialize(const string& host,const string& path,const string& url)
{
 symbol_table.clear();
 //指令
 symbol_table[blchar]="CJL";//单目指令
 symbol_table[bfchar]="CJF";
 
 symbol_table[bf2char]="CJF2";
 //特殊的BF指令,执行完从栈中只推一个操作数,保留一个操作数,以备后面的判断使,switch专用
 
 symbol_table[btchar]="CJT";

 symbol_table[labelchar]="CLAB";
 symbol_table[functionchar]="CFUNC";
 symbol_table[unfunctionchar]="CUNF";
 symbol_table[inichar]="CINI";
 symbol_table[efchar]="CEF";
 symbol_table[unefchar]="CUNEF";
 symbol_table[newchar]="CNEW";
 symbol_table[endnewchar]="CENEW";
 symbol_table[offsetchar]="COFS";

 symbol_table[gtechar]=">=";//比较,双目
 symbol_table[ltechar]="<=";
 symbol_table[eqchar]="==";
 symbol_table[noteqchar]="!=";

 symbol_table[gtchar]=">";
 symbol_table[ltchar]="<";
 symbol_table[negchar]="NEG";//单目指令,取反
 symbol_table[oppchar]="!";
 symbol_table[pluschar]="+";
 symbol_table[subchar]="-";
 symbol_table[multichar]="*";
 symbol_table[divchar]="/";
 symbol_table[modchar]="%";
 symbol_table[andchar]="&&";
 symbol_table[orchar]="||";
 //类型
 symbol_table[codeend]="CEND";
 symbol_table[inttype]="CINT";
 symbol_table[stringtype]="CSTR";
 symbol_table[vartype]="CVAR";
 //核心类名
 symbol_table[arraytype]="ARRAY";
 //单目赋值
 symbol_table[ipp]="CIPP";
 symbol_table[ppi]="CPPI";
 symbol_table[iss]="CISS";
 symbol_table[ssi]="CSSI";

 //位运算符
 symbol_table[leftchar]="<<";
 symbol_table[rightchar]=">>";
 symbol_table[rightrightchar]=">>>";
 symbol_table[andbchar]="&";
 symbol_table[orbchar]="|";
 symbol_table[inbchar]="^";

 //赋值
 symbol_table[setchar]="=";//赋值
 symbol_table[pe]="+=";
 symbol_table[se]="-=";
 symbol_table[me]="*=";
 symbol_table[de]="/=";
 symbol_table[mode]="%=";
 symbol_table[ande]="&=";
 symbol_table[ore]="|=";
 symbol_table[ine]="^=";
 symbol_table[lefte]="<<=";
 symbol_table[righte]=">>=";
 symbol_table[rightee]=">>>=";

 //函数控制指令
 symbol_table[eachar]="CEA";
 symbol_table[eachar2]="CEA2";
 symbol_table[retchar]="CRET";
 symbol_table[bpchar]="CBP";
 symbol_table[returnchar]="CRET";
 symbol_table[return0char]="CRET0";

//特殊符号
 symbol_table[commachar]=",";
 symbol_table[dotchar]=".";
 symbol_table[withchar]="CWT";
 symbol_table[quitwithchar]="CQW";
 symbol_table[temp_id_index]=FUNCTEMP;
 symbol_table[nanchar]="NaN";

 symbol_table[trychar]="CTRY";
 symbol_table[quittrychar]="CQTRY";
 symbol_table[catchchar]="CCATCH";
 symbol_table[quitcatchchar]="CQCATCH";
 symbol_table[finallychar]="CFINA";
 symbol_table[quitfinallychar]="CQFINA";
 
 symbol_table[typeofchar]="CTYPEOF";

 //空串
 symbol_table[empty_string_index]="*";

 symbol_table[host_string_index]="*"+host;
 symbol_table[path_string_index]="*"+path;
 symbol_table[url_string_index]="*"+url;

 //Array
 symbol_table[array_id_index]="Array";

 //int id_start_pos=current_command_index;
 proxy=false;
 code.clear();
 codeenv.clear();
 proxy=false;
 ClearStack(member_function);
 jumplist.clear();
 current_level=global;
 ClearStack(setopstack);
 
 inforwhile=0;
 bclabel=0;
 parse_flag=true;
 ClearStack(arguments_number);
}
//----------------------------------------------------------------------------------------------

void MarkQuitEnv()
{
 codeenv.back()=-1 * codeenv.back();
}
//---------------------------------------------------------------------------------------------

void OutputCharCode(char c)
{
 if(proxy==false)
   {
    code.push_back(c);
	codeenv.push_back(current_level);
   }
 else
    jumplist.back().tempcode.push_back(c);
}
//--------------------------------------------------------------------------------------------
//unsigned int, unsigned real, hex, oct 都转换为整型,注意小于1的实数变为1,避免除0错误
void OutputIntCode(char const* first, char const* last)
{
 string str(first,last);
 unsigned int temp;
 char* stopstr;
 if(str[0]=='0'&&str.size()>1)//8进制或者16进制
   {
	if(str[1]=='X' || str[1]=='x')//HEX
	  {
	   temp=strtol(str.c_str(),&stopstr,16);
	   
	  }
	else//OCT
	  {
	   temp=strtol(str.c_str(),&stopstr,8);
	  }
   }
 else//十进制的实数或者整数
   {
    if(str.find(".")==string::npos)//整数
	  {
	   temp=atoi(str.c_str());
	  }
	else//实数
	  {
	   float f=(float)(strtod(str.c_str(),&stopstr));
	   if(f>1.0)
		  temp=(int)f;
	   else
		  temp=1;
	  }
   }
 if(proxy==false)
   {
    code.push_back(temp);
	codeenv.push_back(current_level);
   }
 else
    jumplist.back().tempcode.push_back(temp);
}
//--------------------------------------------------------------------------------------------

void OutputIntType(char const* first, char const* last)//int指令
{
 if(proxy==false)
   {
    code.push_back(inttype);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(inttype);
}

void OutputVarType(char const* first, char const* last)//var指令
{
 if(proxy==false)
   {
    code.push_back(vartype);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(vartype);
}

void OutputStringType(char const* first, char const* last)//String指令
{
 if(proxy==false)
   {
    code.push_back(stringtype);
	codeenv.push_back(current_level);
   }
 else
    jumplist.back().tempcode.push_back(stringtype);
}
//--------------------------------------------------------------------------------------------
//i++,i--,++i,--i
void OutputSSI(char const* first, char const* last)//--i指令
{
 if(proxy==false)
   {
    code.push_back(ssi);
	codeenv.push_back(current_level);
   }
 else
    jumplist.back().tempcode.push_back(ssi);
}

void OutputISS(char const* first, char const* last)//i--指令
{
 if(proxy==false)
   {
    code.push_back(iss);
	codeenv.push_back(current_level);
   }
 else
    jumplist.back().tempcode.push_back(iss);
}

void OutputPPI(char const* first, char const* last)//++i指令
{
 if(proxy==false)
   {
    code.push_back(ppi);
    codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(ppi);
}

void OutputIPP(char const* first, char const* last)//i++指令
{
 if(proxy==false)
   {
    code.push_back(ipp);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(ipp);
}

void OutputDotchar(char const* first, char const* last)//i++指令
{
 if(proxy==false)
   {
    code.push_back(dotchar);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(dotchar);
}
//----------------------------------------------------------------------------------------------

void OutputSymbolCode(char const* first, char const* last)//ID标识
{
 string str(first,last);
 int temp=current_id_index--;
 symbol_table[temp]=str;
 if(proxy==false)
   {
    code.push_back(temp);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(temp);
}
//--------------------------------------------------------------------------------------------

void OutputStringCode(char const* first, char const* last)//string
{
 string str(first+1,last-1);
 str="*"+str;//以*开头区别string和变量名
 int temp=current_string_index++;
 symbol_table[temp]=str;
 if(proxy==false)
   {
    code.push_back(temp);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(temp);
}

//--------------------------------------------------------------------------------------------

void OutputPatternCode(char const* first, char const* last)//pattern
{
 string str(first,last);
 str="*"+str;
 int temp=current_string_index++;
 symbol_table[temp]=str;
 if(proxy==false)
   {
    code.push_back(temp);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(temp);
}
//--------------------------------------------------------------------------------------------

void do_record_label(char const* first, char const* last)
{
 string str(first,last);
 int temp=current_id_index--;//从-100000递减
 symbol_table[temp]=str;
 bclabel=temp;
}

//----------------------------------------------------------------------------------------------

void do_set(char const*, char const*)//声明语句里的=指令
{
 if(proxy==false)
   {
    code.push_back(setchar);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(setchar);
}
//----------------------------------------------------

void do_enter_setop(char const* first,char const* last)//栈控制赋值栈,
{
 string str(first,last);
 setopstack.push(str);
}
//赋值语句里面的各种操作 = += *= -= /=
//----------------------------------------------------

void do_setop(char const*, char const*)//各种赋值指令
{
 string currentop=setopstack.top();
 int tempchar;
 if(currentop=="=")
    tempchar=setchar;
 else if(currentop=="+=")
    tempchar=pe;
 else if(currentop=="-=")
    tempchar=se;
 else if(currentop=="*=")
	tempchar=me;
 else if(currentop=="/=")
	tempchar=de;
 else if(currentop=="%=")
	tempchar=mode;
 else if(currentop=="<<=")
	tempchar=lefte;
 else if(currentop==">>=")
	tempchar=righte;
 else if(currentop==">>>=")
	tempchar=rightee;
 else if(currentop=="&=")
	tempchar=ande;
 else if(currentop=="|=")
	tempchar=ore;
 else if(currentop=="^=")
	tempchar=ine;
 else
    cout<<"internal error"<<endl;
 if(proxy==false)
   {
    code.push_back(tempchar);
	codeenv.push_back(current_level);
   }
 else
    jumplist.back().tempcode.push_back(tempchar);
 setopstack.pop();
}
//-----------------------------------------------------------------------------------------------

void do_true(char const*, char const*)
{
 if(proxy==false)
   {
    code.push_back(1);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(1);
}

void do_false(char const*,char const*)
{
 if(proxy==false)
   {
    code.push_back(0);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(0);
}

void do_null(char const*,char const*)
{
 if(proxy==false)
   {
    code.push_back(0);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(0);
}
//--------------------------------------------------------------------------------------------

void do_add(char const*, char const*)//+指令
{
 if(proxy==false)
   {
    code.push_back(pluschar);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(pluschar);
}

void do_sub(char const*, char const*)//-,减指令
{
 if(proxy==false)
   {
    code.push_back(subchar);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(subchar);
}

void do_multi(char const*,char const*)//*指令
{
 if(proxy==false)
   {
    code.push_back(multichar);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(multichar);
}

void do_div(char const*,char const*)// /指令
{
 if(proxy==false)
   {
    code.push_back(divchar);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(divchar);
};

void do_mod(char const*,char const*)// %指令
{
 if(proxy==false)
   {
	code.push_back(modchar);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(modchar);
}
//------------------------------------------------------------------------

void do_left(char const*,char const*)// <<指令
{
 if(proxy==false)
   {
	code.push_back(leftchar);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(leftchar);
}

void do_right(char const*,char const*)// >>指令
{
 if(proxy==false)
   {
	code.push_back(rightchar);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(rightchar);
}

void do_rightright(char const*,char const*)// >>>指令
{
 if(proxy==false)
   {
	code.push_back(rightrightchar);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(rightrightchar);
}

void do_andbchar(char const*,char const*)// &指令
{
 if(proxy==false)
   {
	code.push_back(andbchar);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(andbchar);
}

void do_orbchar(char const*,char const*)// |指令
{
 if(proxy==false)
   {
	code.push_back(orbchar);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(orbchar);
}

void do_inbchar(char const*,char const*)// ^指令
{
 if(proxy==false)
   {
	code.push_back(inbchar);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(inbchar);
}
//------------------------------------------------------------------------

void do_commachar(char const*,char const*)// ^指令
{
 if(proxy==false)
   {
	code.push_back(commachar);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(commachar);
}
//------------------------------------------------------------------------
void do_equal(char const*,char const*)// ==指令
{
 if(proxy==false)
   {
    code.push_back(eqchar);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(eqchar);
}

void do_gt(char const*,char const*)// >指令
{
 if(proxy==false)
   {
    code.push_back(gtchar);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(gtchar);
}

void do_lt(char const*,char const*)// <指令
{
 if(proxy==false)
   {
    code.push_back(ltchar);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(ltchar);
}

void do_gte(char const*,char const*)// >=指令
{
 if(proxy==false)
   {
    code.push_back(gtechar);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(gtechar);
}

void do_lte(char const*,char const*)// <=指令
{
 if(proxy==false)
   {
    code.push_back(ltechar);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(ltechar);
}

void do_notequal(char const*,char const*)// !=指令
{
 if(proxy==false)
   {
    code.push_back(noteqchar);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(noteqchar);
}

void do_neg(char const*,char const*)// -取反指令
{
 if(proxy==false)
   {
    code.push_back(negchar);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(negchar);
}

void do_opp(char const*,char const*)// !取反指令
{
 if(proxy==false)
   {
    code.push_back(oppchar);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(oppchar);
}
//-----------------------------------------------------------------------------------------------

void do_andchar(char const*,char const*)
{
 if(proxy==false)
   {
    code.push_back(andchar);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(andchar);
}

void do_orchar(char const*,char const*)
{
 if(proxy==false)
   {
    code.push_back(orchar);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(orchar);
}
//-----------------------------------------------------------------------------------------------

void set_proxy(char const*,char const*)//进入缓冲
{
 proxy=true;
}

void reset_proxy(char const*,char const*)//退出缓冲
{
 proxy=false;
}
//-----------------------------------------------------------------------------------------------

void do_bl(char const*,char const*)//BL指令
{
 DoubleJumpList& djl=jumplist.back();
 unsigned int len=code.size();
 djl.BLList.push_back(len);//地址
 code.push_back(len+2);
 code.push_back(blchar);
 codeenv.push_back(current_level);
 codeenv.push_back(current_level);
};

void do_bf(char const*,char const*)//BF指令
{
 DoubleJumpList& djl=jumplist.back();
 unsigned int len=code.size();
 djl.BFList.push_back(len);//地址
 code.push_back(len+2);
 code.push_back(bfchar);
 codeenv.push_back(current_level);
 codeenv.push_back(current_level);
};

void do_bf2(char const*,char const*)//BF2指令,switch专用
{
 DoubleJumpList& djl=jumplist.back();
 for(int i=0;i<(int)djl.tempcode.size();i++)
    {
	 code.push_back(djl.tempcode[i]);
	 codeenv.push_back(current_level);
	}
 //djl.tempcode.clear();
 code.push_back(eqchar);//写等号
 codeenv.push_back(current_level);
 //DoubleJumpList& djl=jumplist.back();
 unsigned int len=code.size();
 djl.BFList.push_back(len);//地址
 code.push_back(len+2);
 code.push_back(bfchar);//修改为bf, bf2无意义了
 codeenv.push_back(current_level);
 codeenv.push_back(current_level);
}

void do_bt(char const*,char const*)//BT指令
{
 DoubleJumpList& djl=jumplist.back();
 unsigned int len=code.size();
 djl.BTList.push_back(len);//地址
 code.push_back(len+2);
 code.push_back(btchar);
 codeenv.push_back(current_level);
 codeenv.push_back(current_level);
}

void do_label(char const* first,char const* last)
{
 code.push_back(bclabel);
 code.push_back(labelchar);
 codeenv.push_back(current_level);
 codeenv.push_back(current_level);
}

void do_label_continue(char const* first,char const* last)//continue label BL
{
 code.push_back(blchar);
 codeenv.push_back(current_level);
}

void do_label_break(char const* first,char const* last)//break label BL
{
 code.push_back(blchar);
 codeenv.push_back(current_level);
}
//----------------------------------------------------------------------

void do_empty_bl(char const*,char const*)//BL指令,延迟执行
{
 DoubleJumpList& djl=jumplist.back();
 unsigned int len=code.size();
 djl.BLList.push_back(len);
}
//-----------------------------------------------------------------------------------------------

void do_enter_for(char const*,char const*)//进入for或者while循环
{
 current_level++;
 inforwhile++;
 DoubleJumpList djl(FORWHILE);
 djl.startpos=code.size();
 jumplist.push_back(djl);
}
//-----------------------------------------------------------------------------------------------

void set_forin(char const*,char const*)//表示当前环境为for in语句
{
 DoubleJumpList& djl=jumplist.back();
 djl.isforin=true;
}
//-----------------------------------------------------------------------------------------------

void do_enter_for_statement(char const*,char const*)//进入内环境
{
 current_level++;
}

void do_quit_for_statement(char const*,char const*)
{
 current_level--;
}

void do_enter_if_statement(char const*,char const*)
{
 current_level++;
}

void do_quit_if_statement(char const*,char const*)
{
 current_level--;
}

void do_enter_while_statement(char const*,char const*)
{
 current_level++;
}

void do_quit_while_statement(char const*,char const*)
{
 current_level--;
}
//-----------------------------------------------------------------------------------------------

void backpatch_bl(char const*,char const*)//回填BL
{
 if(jumplist.empty())
   {
	parse_flag=false;
    TError::ParsingError(Error_jumplistfail,"");
	return;
   }
 DoubleJumpList& djl=jumplist.back();
 for(int i=0;i<(int)djl.BLList.size();i++)
	{
     code[djl.BLList[i] ]=code.size();
	}
}
//-----------------------------------------------------------------------------------------------

void backpatch_proxy(DoubleJumpList& djl)//回填for循环里的i++
{
 for(int i=0;i<(int)djl.tempcode.size();i++)
    {
	 code.push_back(djl.tempcode[i]);
	 codeenv.push_back(current_level);
	}
 djl.tempcode.clear();
}
//-----------------------------------------------------------------------------------------------

void backpatch_continuelist(DoubleJumpList& djl)
{
 for(int i=0;i<(int)djl.continuelist.size();i++)
	 code[djl.continuelist[i]]=code.size();
}

void backpatch_dowhile_continuelist(char const*,char const*)
{
 if(jumplist.empty())
   {
	parse_flag=false;
    TError::ParsingError(Error_jumplistfail,"");
	return;
   }
 DoubleJumpList& djl=jumplist.back();
 backpatch_continuelist(djl);
}
//-----------------------------------------------------------------------------------------------

void do_existed_bl(char const*,char const*)//立即执行被延迟执行的BL
{
 if(jumplist.empty())
   {
	parse_flag=false;
    TError::ParsingError(Error_jumplistfail,"");
	return;
   }
 DoubleJumpList& djl=jumplist.back();
 backpatch_continuelist(djl);//回填contine地址
 backpatch_proxy(djl);//for循环回填i++,while-do循环不执行
 unsigned int existed_value=0;
 if(!djl.BLList.empty())//如果是for in,就会导致BLList为空
    existed_value=djl.BLList.back();
 code.push_back(existed_value);
 code.push_back(blchar);
 codeenv.push_back(current_level);
 codeenv.push_back(current_level);
}
//------------------------------------------------------------------------------------------------

void backpatch_bf(char const*,char const*)//回填BF
{
 if(jumplist.empty())
	{
	 parse_flag=false;
	 TError::ParsingError(Error_jumplistfail,"");
	 return;
	}
 DoubleJumpList& djl=jumplist.back();
 for(int i=0;i<(int)djl.BFList.size();i++)
	{
     code[djl.BFList[i] ]=code.size();
	}
}

void backpatch_bf_pop(char const*,char const*)//回填上一个BF,填完POP,switch语句专用
{
 if(jumplist.empty())
	{
	 parse_flag=false;
	 TError::ParsingError(Error_jumplistfail,"");
	 return;
	}
 DoubleJumpList& djl=jumplist.back();
 if(djl.BFList.empty())
   {
	parse_flag=false;
	TError::ParsingError(Error_jumplistfail,"");
	return;
   }
 unsigned int addr=djl.BFList.back();
 code[addr]=code.size();
 djl.BFList.pop_back();
}
//------------------------------------------------------------------------------------------------

void backpatch_bf_quit_for(char const*,char const*)//退出for,加入退栈的功能
{
 if(jumplist.empty())
	{
     parse_flag=false;
	 TError::ParsingError(Error_jumplistfail,"");
	 return;
	}
 current_level--;
 DoubleJumpList& djl=jumplist.back();
 if(djl.isforin)//原来是for in语句,跳过处理
   {
	code.erase(code.begin()+djl.startpos,code.end());
	codeenv.erase(codeenv.begin()+djl.startpos,codeenv.end());
	jumplist.pop_back();
    inforwhile--;
	return;
   }
 for(int i=0;i<(int)djl.BFList.size();i++)
	{
     code[djl.BFList[i] ]=code.size();
	}
 jumplist.pop_back();
 inforwhile--;
 MarkQuitEnv();
}
//------------------------------------------------------------------------------------------------

void backpatch_bf_quit_whiledo(char const*,char const*)//退出whiledo,加入退栈的功能
{
 if(jumplist.empty())
	{
	 parse_flag=false;
	 TError::ParsingError(Error_jumplistfail,"");
	 return;
	}
 current_level--;
 DoubleJumpList& djl=jumplist.back();
 for(int i=0;i<(int)djl.BFList.size();i++)
	{
     code[djl.BFList[i] ]=code.size();
	}
 jumplist.pop_back();
 inforwhile--;
 MarkQuitEnv();
}
//------------------------------------------------------------------------------------------------
//控制if语句环境
void do_enter_if(char const*,char const*)
{
 current_level++;
 DoubleJumpList djl(IF);
 jumplist.push_back(djl);
}

void do_quit_if(char const*,char const*)
{
 current_level--;
 jumplist.pop_back();
 MarkQuitEnv();
}

//---------------------------------------------------------------------------------------------
//控制选择语句环境
void do_entersel_bf(char const* first,char const* last)
{
 DoubleJumpList djl(SEL);
 jumplist.push_back(djl);
 do_bf(first,last);
}

void backpatch_bl_quitsel(char const* first,char const* last)
{
 backpatch_bl(first,last);
 jumplist.pop_back();
 MarkQuitEnv();
}
//--------------------------------------------------------------------------------------------
// && 逻辑与语句
void do_enter_logicaland(char const* first,char const* last)
{
 DoubleJumpList djl(LOGICALAND);
 jumplist.push_back(djl);
}
//--------------------------------------------------------------------------------------------

void backpatch_bf_bl(char const* first,char const* last)
{
 do_bl(first,last);
 if(jumplist.empty())
   {
	parse_flag=false;
    TError::ParsingError(Error_jumplistfail,"");
	return;
   }
 DoubleJumpList& djl=jumplist.back();
 for(int i=0;i<(int)djl.BFList.size();i++)
	{
     code[djl.BFList[i] ]=code.size();
	}
 if(proxy==false)
   {
    code.push_back(0);//如果有BF跳转,则补结果0
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(0);
 backpatch_bl(first,last);
}

void do_quit_logicaland(char const* first,char const* last)
{
 jumplist.pop_back();
}

// || 逻辑与语句
void do_enter_logicalor(char const* first,char const* last)
{
 DoubleJumpList djl(LOGICALOR);
 jumplist.push_back(djl);
}

void backpatch_bt_bl(char const* first,char const* last)
{
 do_bl(first,last);
 if(jumplist.empty())
   {
	parse_flag=false;
    TError::ParsingError(Error_jumplistfail,"");
	return;
   }
 DoubleJumpList& djl=jumplist.back();
 for(int i=0;i<(int)djl.BTList.size();i++)
	{
     code[djl.BTList[i] ]=code.size();
	}
 if(proxy==false)
   {
    code.push_back(1);//如果有BT跳转,则补结果1
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(1);
 backpatch_bl(first,last);
}

void do_quit_logicalor(char const* first,char const* last)
{
 jumplist.pop_back();
}
//--------------------------------------------------------------------------------------------

void do_for_bf(DoubleJumpList& djl)//BL指令,for whiledo语句用
{
 /*int pos=jumplist.size()-1;
 while(jumplist[pos].envtype!=FORWHILE&&pos>=0)
	   pos--;
 DoubleJumpList& djl=jumplist[pos];*/
 unsigned int len=code.size();
 djl.BFList.push_back(len);//地址
 code.push_back(len+2);
 code.push_back(blchar);
 codeenv.push_back(current_level);
 codeenv.push_back(current_level);
};

void do_dowhile_bl(DoubleJumpList& djl)
{
 unsigned int len=code.size();
 djl.BLList.push_back(len);//地址
 code.push_back(len+2);
 code.push_back(blchar);
 codeenv.push_back(current_level);
 codeenv.push_back(current_level);
}
//---------------------------------------------------------------------------------------

void do_break(char const* first,char const* last)//break跳转指令
{
 if(inforwhile>0)
	{
     int pos=jumplist.size()-1;
     while(jumplist[pos].envtype!=FORWHILE && jumplist[pos].envtype!=DOWHILE &&
		   jumplist[pos].envtype!=SWITCH  && pos>=0 )
	       pos--;
	 if(jumplist[pos].envtype==FORWHILE)//for和whiledo循环中的break
	   {
	    DoubleJumpList& djl=jumplist[pos];
        do_for_bf(djl);//for和whiledo循环加入BF跳表
	   }
	 else if(jumplist[pos].envtype==DOWHILE)//dowhile循环中的break
	   {
		DoubleJumpList& djl=jumplist[pos];
		do_dowhile_bl(djl);//dowhile循环加入BL跳表
	   }
	 else if(jumplist[pos].envtype==SWITCH)//switch,加入BL跳表
	   {
		DoubleJumpList& djl=jumplist[pos];
		unsigned int len=code.size();
        djl.BLList.push_back(len);//地址
        code.push_back(len+2);
        code.push_back(blchar);
        codeenv.push_back(current_level);
        codeenv.push_back(current_level);
	   }
	 else
	   {
		//cout<<"Syntax Error: break not in iteration statement(for whiledo dowhile)"<<endl;
		TError::ParsingError(Error_breaknotiniteration,"break");
	   }
	}
 else
	TError::ParsingError(Error_breaknotiniteration,"break");
}
//---------------------------------------------------------------------------------------

void do_continue(char const*,char const*)//continue跳转指令,BL
{
 int pos=jumplist.size()-1;
 while(jumplist[pos].envtype!=FORWHILE && jumplist[pos].envtype!=DOWHILE &&pos>=0)
	   pos--;
 if(jumplist[pos].envtype!=FORWHILE && jumplist[pos].envtype!=DOWHILE)
   {
    cout<<"Syntax Error: continue not in iteration statement(for whiledo dowhile)"<<endl;
   }
 DoubleJumpList& djl=jumplist[pos];
 unsigned int len=code.size();
 djl.continuelist.push_back(len);//地址,把需要跳转的指令地址放入continuelist
 code.push_back(len+2);
 code.push_back(blchar);
 codeenv.push_back(current_level);
 codeenv.push_back(current_level);
};
//-------------------------------------------------------------------------------------------------

void do_empty_bt()//预写BT跳转地址
{
 if(jumplist.empty())
   {
	parse_flag=false;
    TError::ParsingError(Error_jumplistfail,"");
	return;
   }
 DoubleJumpList& djl=jumplist.back();
 unsigned int len=code.size();
 djl.BTList.push_back(len);
}

void do_enter_dowhile(char const*,char const*)//进入do-while循环,预先填写BT
{
 current_level++;
 inforwhile++;
 DoubleJumpList djl(DOWHILE);
 jumplist.push_back(djl);
 do_empty_bt();
}
//-------------------------------------------------------------------------------------------------

void do_existed_bt(char const*,char const*)//do(BT){} while continue ()BL
{
 if(jumplist.empty())
   {
	parse_flag=false;
    TError::ParsingError(Error_jumplistfail,"");
	return;
   }
 DoubleJumpList& djl=jumplist.back();
 int existed_value=djl.BTList.back();
 code.push_back(existed_value);
 code.push_back(btchar);
 codeenv.push_back(current_level);
 codeenv.push_back(current_level);
}
//-------------------------------------------------------------------------------------------------

void do_quit_dowhile(char const*,char const*)
{
if(jumplist.empty())
	{
	 parse_flag=false;
	 TError::ParsingError(Error_jumplistfail,"");
	 return;
	}
 current_level--;
 DoubleJumpList& djl=jumplist.back();
 for(int i=0;i<(int)djl.BLList.size();i++)//回填BL地址
	{
     code[djl.BLList[i] ]=code.size();
	}
 jumplist.pop_back();
 inforwhile--;
 MarkQuitEnv();
}
//-------------------------------------------------------------------------------------------------

void do_enter_switch(char const* first,char const* last)
{
 current_level++;
 inforwhile++;
 DoubleJumpList djl(DOWHILE);
 jumplist.push_back(djl);
 set_proxy(first,last);
}

void do_quit_switch(char const* first,char const* last)//退出switch的同时,回填switch里的BL跳转
{
 backpatch_bl(first,last);
 if(jumplist.empty())
   {
	parse_flag=false;
    TError::ParsingError(Error_jumplistfail,"");
	return;
   }
 jumplist.back().tempcode.clear();
 jumplist.pop_back();
 current_level--;
 inforwhile--;
 MarkQuitEnv();
}

void do_enter_casedefault(char const*,char const*)
{
 current_level++;
}

void do_quit_casedefault(char const*,char const*)
{
 current_level--;
}
//---------------------------------------------------------------------------------------------
//控制参数从左到右记录
void do_enter_argumentslist(char const*,char const*)
{
 arguments_number.push(0);
}

void do_quit_argumentslist(char const*,char const*)
{
 if(proxy==false)
   {
	code.push_back(arguments_number.top());
    code.push_back(eachar);
	codeenv.push_back(current_level);
	codeenv.push_back(current_level);
   }
 else
   {
	if(jumplist.empty())
	  {
	   parse_flag=false;
       TError::ParsingError(Error_jumplistfail,"");
	   return;
	  }
	jumplist.back().tempcode.push_back(arguments_number.top());
	jumplist.back().tempcode.push_back(eachar);
   }
 arguments_number.pop();
}

void do_argument(char const*,char const*)
{
 int& cn=arguments_number.top();
 cn++;
}

int FindLastFunctionID(int startpos)//寻找前一个函数名ID,do_call_function专用,注意必须从EFCHAR开始
{
 int count=0;
 while(startpos>=0)
	  {
	   if(code[startpos]==efchar)
		  count++;
	   else if(code[startpos]==functionchar)
		  count--;
	   if(count==0)
		  break;
	   startpos--;
	  }//end while
 return code[startpos-1];
}

void do_call_function(char const* first,char const* last)// (function(){})() 语法专用,有quit_argumentlist的功能
{
 //int an=arguments_number.top();
 int pos=code.size()-1;
 while(code[pos]!=efchar)
	  {
	   pos--;
	   if(pos<0)
		 {
		  IntValue temp;
		  TError::ShowError(Error_pfatalerror,0,temp,string(first,last));
		  return;
		 }
	  }
 pos++;//efchar后的第一个位置
 int fid=FindLastFunctionID(pos-1);

 code.insert(code.begin()+pos,fid);
 codeenv.insert(codeenv.begin()+pos,current_level);
 code.push_back(arguments_number.top());
 code.push_back(eachar);
 codeenv.push_back(current_level);
 codeenv.push_back(current_level);
 arguments_number.pop();
}
//---------------------------------------------------------------------------------------------
//function集合
void do_efchar(char const* first,char const* last)
{
 if(proxy==false)
   {
	code.push_back(efchar);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(efchar);
}

void do_enter_function(char const*,char const*)
{
 current_level++;
}

void do_quit_function(char const* first,char const* last)
{
 do_efchar(first,last);
 current_level--;
 MarkQuitEnv();
}

void do_return(char const*,char const*)//有值返回
{
 if(proxy==false)
   {
	code.push_back(returnchar);
	codeenv.push_back(current_level);
   }
 else
   {
	jumplist.back().tempcode.push_back(returnchar);
   }
}

void do_return0(char const*,char const*)//无值返回
{
 if(proxy==false)
   {
	code.push_back(return0char);
	codeenv.push_back(current_level);
   }
 else
   {
	jumplist.back().tempcode.push_back(return0char);
   }
}

void do_argument_initialize(char const*,char const*)//结合函数调用栈
{
 if(proxy==false)
   {
    code.push_back(inichar);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(inichar);
}

void do_functionchar(char const*,char const*)//function标识
{
 if(proxy==false)
   {
	code.push_back(functionchar);
	codeenv.push_back(current_level);
   }
 else
   jumplist.back().tempcode.push_back(functionchar);
}
//--------------------------------------------------------------------------------------------

void do_newchar(char const*,char const*)
{
 if(proxy==false)
   {
    code.push_back(newchar);
    codeenv.push_back(current_level);
   }
 else
    jumplist.back().tempcode.push_back(newchar);
}

void do_endnewchar(char const*,char const*)
{
 if(proxy==false)
   {
    code.push_back(endnewchar);
    codeenv.push_back(current_level);
   }
 else
   jumplist.back().tempcode.push_back(endnewchar);
}

void do_endnewchar_array(char const* first,char const* last)
{
 do_endnewchar(first,last);
 int temp=array_id_index;
 if(proxy==false)
   {
    code.push_back(temp);
	codeenv.push_back(current_level);
   }
 else
	jumplist.back().tempcode.push_back(temp);
}
//--------------------------------------------------------------------------------------------

void do_offsetchar(char const*,char const*)
{
 if(proxy==false)
   {
	code.push_back(offsetchar);
	codeenv.push_back(current_level);
   }
 else
   jumplist.back().tempcode.push_back(offsetchar);
}
//--------------------------------------------------------------------------------------------

void do_with(char const*,char const*)
{
 code.push_back(withchar);
 codeenv.push_back(current_level);
}

void do_quit_with(char const*,char const*)
{
 code.push_back(quitwithchar);
 codeenv.push_back(current_level);
}
//--------------------------------------------------------------------------------------------

void do_try(char const*,char const*)
{
 code.push_back(trychar);
 codeenv.push_back(current_level);
 current_level++;
}

void do_quit_try(char const*,char const*)
{
 current_level--;
 code.push_back(quittrychar);
 codeenv.push_back(current_level);
}
//--------------------------------------------------------------------------------------------

void do_catch(char const*,char const*)
{
 code.push_back(catchchar);
 codeenv.push_back(current_level);
 current_level++;
}

void do_quit_catch(char const*,char const*)
{
 current_level--;
 code.push_back(quitcatchchar);
 codeenv.push_back(current_level);
}
//--------------------------------------------------------------------------------------------

void do_finally(char const*,char const*)
{
 code.push_back(finallychar);
 codeenv.push_back(current_level);
 current_level++;
}

void do_quit_finally(char const*,char const*)
{
 current_level--;
 code.push_back(quitfinallychar);
 codeenv.push_back(current_level);
}
//--------------------------------------------------------------------------------------------

void do_typeof(char const*,char const*)
{
 if(proxy==false)
   {
	code.push_back(typeofchar);
	codeenv.push_back(current_level);
   }
 else
   jumplist.back().tempcode.push_back(typeofchar);
}
//--------------------------------------------------------------------------------------------

void is_member_function(char const* first,char const* last)//注意pop一个,避免重复
{
 member_function.pop();
 member_function.push(1);
 do_enter_argumentslist(first,last);
}

void not_member_function(char const* first,char const* last)
{
 member_function.push(0);
}

void reset_member_function(char const* first,char const* last)
{
 member_function.pop();
}

void do_pe2(char const* first,char const* last)
{
 int& mc=member_function.top();
 if(mc==0)//调用属性
   {
	if(proxy==false)
      {
       code.push_back(dotchar);
	   codeenv.push_back(current_level);
      }
    else
	   jumplist.back().tempcode.push_back(dotchar);
   }
 else//调用成员函数
   {
	if(proxy==false)
      {
	   code.push_back(arguments_number.top());
	   code.push_back(eachar2);
	   codeenv.push_back(current_level);
	   codeenv.push_back(current_level);
      }
    else
      {
	   jumplist.back().tempcode.push_back(arguments_number.top());
	   jumplist.back().tempcode.push_back(eachar2);
      }
    arguments_number.pop();
   }
 reset_member_function(first,last);
}
//--------------------------------------------------------------------------------------------

void do_enter_unfunction(char const* first,char const* last)
{
 current_level++;
 if(proxy==false)
   {
	code.push_back(unfunctionchar);
	codeenv.push_back(current_level);
   }
 else
   jumplist.back().tempcode.push_back(unfunctionchar);
}

void do_quit_unfunction(char const* first,char const* last)
{
 current_level--;
 if(proxy==false)
   {
	code.push_back(unefchar);
	codeenv.push_back(current_level);
   }
 else
   jumplist.back().tempcode.push_back(unefchar);
}
//--------------------------------------------------------------------------------------------

void do_emptyexpr(char const* first,char const* last)//待改进,存在var a={}
{
 if(proxy==false)
   {
    code.push_back(0);
	codeenv.push_back(current_level);
   }
 else
    jumplist.back().tempcode.push_back(0);
}
//--------------------------------------------------------------------------------------------
typedef char const* iterator_t;
typedef tree_match<iterator_t> parse_tree_match_t;
typedef parse_tree_match_t::const_tree_iterator iter_t;
typedef pt_match_policy<iterator_t> match_policy_t;
typedef scanner_policies<iteration_policy, match_policy_t, action_policy> scanner_policy_t;
typedef scanner<iterator_t, scanner_policy_t> scanner_t;
typedef rule<scanner_t> rule_t;
//--------------------------------------------------------------------------------------------

struct gram_g:grammar<gram_g> 
{ 
 template <class Scan> 
 struct definition 
  {
   rule<Scan> ifexpr; //if语句
   rule<Scan> forexpr;//for循环语句
   rule<Scan> whiledoexpr;//while-do循环
   rule<Scan> dowhileexpr;//do-while循环
   rule<Scan> switchexpr;//switch语句

   rule<Scan> jumpexpr;//控制语句, goto continue break;
   rule<Scan> continueexpr;//continue语句
   rule<Scan> breakexpr;//break语句
   //rule<Scan> gotoexpr;//goto语句
   rule<Scan> labelexpr;//label语句
   rule<Scan> constant_expr;//常数句
   rule<Scan> caseexpr;//case语句
   rule<Scan> defaultexpr;//default语句

   rule<Scan> integer; //整数
   rule<Scan> keywords;//屏蔽词
   rule<Scan> uinteger;//无符号整数
   rule<Scan> trueexpr;//真true
   rule<Scan> falseexpr;//假true
   rule<Scan> nullexpr;//空null,看作0
   rule<Scan> type;//类型
   rule<Scan> condition; //条件
   rule<Scan> statement; //语句
   rule<Scan> statementlist;//语句串
   rule<Scan> compoundstatement;//复杂语句
   rule<Scan> setvalueexpr; //赋值语句
   rule<Scan> idexpr; //标识符

   rule<Scan> nopexpr;//空语句
   rule<Scan> thisexpr;//this
   rule<Scan> expr;//表达式

   rule<Scan> stringexpr;//字符串
   rule<Scan> declarexpr;//声明语句
   //rule<Scan> ippiss;//++ --
   //rule<Scan> ppissi;//++i --i

   rule<Scan> assignment_operator;//赋值
   rule<Scan> primary_expr;//基本符

   rule<Scan> postfix_expr;//后缀号 [] 全局函数
   rule<Scan> postfix_expr2;// . 成员函数

   rule<Scan> unary_expr;//单目符号
   rule<Scan> multiplicative_expr;//乘除号 * /
   rule<Scan> additive_expr;//加减号 + -
   rule<Scan> shift_expr;// 移位运算符
   rule<Scan> relational_expr;
   rule<Scan> equality_expr;//等
   rule<Scan> and_expr;//位与
   rule<Scan> exclusive_or_expr;//位异或
   rule<Scan> inclusive_or_expr;//位或|
   rule<Scan> logical_and_expr;//逻辑与 &&
   rule<Scan> logical_or_expr;//逻辑或 ||
   rule<Scan> conditional_expr;//条件
   rule<Scan> assignment_expr;//赋值
   
   rule<Scan> argument_expr_list;//参数列
   rule<Scan> arrayexpr;

   rule<Scan> returnexpr;//return
   rule<Scan> functionexpr;//function
   rule<Scan> functionexpr2;//匿名function
   rule<Scan> functionexpr3;// 直接执行的function

   rule<Scan> withexpr;
   rule<Scan> importexpr;
   rule<Scan> exportexpr;
   rule<Scan> newexpr;
   rule<Scan> aaexpr;// Array里面的参数
   rule<Scan> skip;//跳过的文法,因为js里面可以以\n为终止符
   rule<Scan> testexpr;//保留
   rule<Scan> classnameexpr;//类名
   rule<Scan> typeofexpr;//typeof内置操作符

   rule<Scan> tryexpr;
   rule<Scan> catchexpr;
   rule<Scan> finallyexpr;

   rule<Scan> patternexpr;
   rule<Scan> functionid;
   rule<Scan> emptyexpr;//空字符串

   rule<Scan> htmlcommentexpr;//HTML代码注释,js兼容,特殊

   rule<Scan> const &start() const 
   {
    //return patternexpr;
    return statementlist;
   }
   //---------------------------------------------------------------------------------------------------------
   
   definition(const gram_g &) 
   { 
	skip=*(/*eol_p | blank_p*/space_p );//js的换行符可当做句终止符
    /*
    keywords=  str_p("var") | str_p("String") | str_p("if") 
		     | str_p("for") | str_p("while") |str_p("goto") 
		     | str_p("true") | str_p("false") | str_p("continue")
		     | str_p("break") | str_p("function") |str_p("return") 
		     | str_p("new") | str_p("delete") | str_p("this") 
		     | str_p("typeof") | str_p("Function") | str_p("Math")
		     | str_p("void") | str_p("do") | str_p("Date") 
		     | str_p("Number") | str_p("Boolean") | str_p("Object") 
		     | str_p("RegExp") | str_p("export") | str_p("import") 
		     | str_p("default")| str_p("case") | str_p("switch")
		     | str_p("Array") | str_p("with") | str_p("null");
	*/ 
    patternexpr=( 
		        lexeme_d[
		                 token_node_d[
		                               (
                                         ch_p('/')>>
		                                  *(
		                                      ( anychar_p-ch_p('/')-ch_p('\\') )
		                                      |
		                                      ( ch_p('\\')>>anychar_p )
		                                   )>>
		                                ch_p('/')
                                       )>>
		                                !(
		                                    ch_p('i') 
		                                    | 
		                                    ( ch_p('g')>> !(ch_p('i')) )
		                                 )
		                            ]
		                ])[&OutputPatternCode];
    
	uinteger=( 
		        lexeme_d[
		                  token_node_d[
		                                 (
											if_p(ch_p('0'))[
												            !(
	                                                            if_p( ch_p('X') | ch_p('x') )[hex_p]
												                .
												                else_p[
		                                                                if_p(ch_p('.'))[uint_p]
		                                                                .
		                                                                else_p[oct_p]
		                                                              ]
		                                                     )
												           ]
											.
											else_p[ strict_ureal_p | uint_p ]
		                                  )
									   ]
		               ] 
		     )[&OutputIntCode];

    trueexpr=(str_p("true"))[&do_true];

	falseexpr=str_p("false")[&do_false];

	nullexpr=str_p("null")[&do_null];

	thisexpr=str_p("this");

	continueexpr=str_p("continue")>>
		                            (
		                              (
		                                (str_p(";"))|eol_p
		                              )[&do_continue] 
		                              | 
		                              (idexpr>>((ch_p(';'))|eol_p))[&do_label_continue]
		                            );
	
	breakexpr=str_p("break")>>
		                      (
		                         ( 
		                           (str_p(";"))|eol_p
		                         )[&do_break]
		                         |
		                         (idexpr>>(ch_p(';')|eol_p))[&do_label_break]
		                      );
    
	returnexpr=(
		         (
		           (str_p("return")>>(ch_p(';')|eol_p))[&do_return0]
		         )
		         | 
	             (
		           (lexeme_d[str_p("return")>>space_p]>>skip>>expr>>((ch_p(';'))|eol_p))[&do_return]
		         )
		       );

	//javascript没有goto
	//gotoexpr=str_p("goto ")>>idexpr>>ch_p(';');  
	
	labelexpr=(
                 ( lexeme_d[ token_node_d[
		                                  ( 
		                                    (+(alpha_p | ch_p('_')) >>*(ch_p('_')|alpha_p|digit_p))
		                                    -
		                                    (
		                                       str_p("var") | str_p("String") | str_p("if") 
		                                     | str_p("for") | str_p("while") |str_p("goto") 
		                                     | str_p("true") | str_p("false") | str_p("continue")
		                                     | str_p("break") | str_p("function") |str_p("return") 
		                                     | str_p("new") | str_p("delete") | str_p("this") 
		                                     | str_p("typeof") | str_p("Function") | str_p("Math")
		                                     | str_p("void") | str_p("do") | str_p("Date") 
		                                     | str_p("Number") | str_p("Boolean") | str_p("Object") 
		                                     | str_p("RegExp") | str_p("export") | str_p("import") 
		                                     | str_p("default")| str_p("case") | str_p("switch")
		                                     | str_p("Array") | str_p("with") | str_p("null")
		                                     | str_p("try") | str_p("catch")
		                                    ) 
		                                 )
		                                ] 
		                   ] 
		         )[&do_record_label]>>skip>>ch_p(':')
		      )[&do_label];

	classnameexpr=lexeme_d[ 
		                    token_node_d[ 
		                                   +(alpha_p | ch_p('-'))
		                                   >>*( ch_p('_')|alpha_p|digit_p ) 
		                                ] 
		                  ];
	
	jumpexpr=continueexpr | breakexpr ;
	
    stringexpr=(lexeme_d[
		               token_node_d[
		                               (ch_p('"')>>
		                                *(
		                                    ( anychar_p-ch_p('"')-ch_p('\\') )
		                                    |
		                                    ( ch_p('\\')>>anychar_p )
		                                 )
		                                >>ch_p('"'))
		                             |
		                               (ch_p('\'')>>
		                                *(
		                                    ( anychar_p-ch_p('\'')-ch_p('\\') )
		                                    |
		                                    ( ch_p('\\')>>anychar_p )
		                                 )
		                                >>ch_p('\''))
		                           ]
		               ]
		       )[&OutputStringCode];

    idexpr=( lexeme_d[ token_node_d[
		                            ( 
		                               ( 
		                                  +(alpha_p | ch_p('_') | ch_p('$') )
		                                  >>*(ch_p('_')|alpha_p|digit_p|ch_p('$'))
		                               )
		                               -
		                               (
		                                str_p("var") | /*str_p("String") |*/ str_p("if") 
		                              | str_p("for") | str_p("while") |str_p("goto") 
		                              | str_p("true") | str_p("false") | str_p("continue")
		                              | str_p("break") | str_p("function") |str_p("return") 
		                              | str_p("new") | str_p("delete") | str_p("this") 
		                              | str_p("typeof") /*| str_p("Function")*/ 
		                              | str_p("void") | str_p("do") | str_p("Date") 
		                              /*| str_p("Number") | str_p("Boolean") | str_p("Object") 
		                              | str_p("RegExp")*/ | str_p("export") | str_p("import") 
		                              | str_p("default")| str_p("case") | str_p("switch")
		                              /*| str_p("Array")*/ | str_p("with") | str_p("null")
		                              | str_p("try") | str_p("catch")
		                               ) 
		                            )
		                           ] 
		             ] 
		   )[&OutputSymbolCode];
    
	functionid=( lexeme_d[ token_node_d[
		                            ( 
		                               ( 
		                                  +(alpha_p | ch_p('_') | ch_p('$') | ch_p('.'))
		                                  >>*(ch_p('_')|alpha_p|digit_p|ch_p('$')|ch_p('.'))
		                               )
		                               -
		                               (
		                                str_p("var") | /*str_p("String") |*/ str_p("if") 
		                              | str_p("for") | str_p("while") |str_p("goto") 
		                              | str_p("true") | str_p("false") | str_p("continue")
		                              | str_p("break") | str_p("function") |str_p("return") 
		                              | str_p("new") | str_p("delete") | str_p("this") 
		                              | str_p("typeof") /*| str_p("Function")*/ 
		                              | str_p("void") | str_p("do") | str_p("Date") 
		                              /*| str_p("Number") | str_p("Boolean") | str_p("Object") 
		                              | str_p("RegExp")*/ | str_p("export") | str_p("import") 
		                              | str_p("default")| str_p("case") | str_p("switch")
		                              /*| str_p("Array")*/ | str_p("with") | str_p("null")
		                              | str_p("try") | str_p("catch")
		                               ) 
		                            )
		                           ] 
		             ] 
		   )[&OutputSymbolCode];
	/*ippiss=(str_p("++"))[&OutputIPP] | (str_p("--"))[&OutputISS] ;
	  ppissi= (str_p("++")>>skip>>idexpr)[&OutputPPI] |
		      (str_p("--")>>skip>>idexpr)[&OutputSSI];*/

//---------------------------------------------------------------------------------------

    assignment_operator=(
		                  ch_p('=')|str_p("*=")|str_p("/=")|str_p("%=")|
		                  str_p("+=")|str_p("-=")|str_p("<<=")|str_p(">>=")|
		                  str_p(">>>=")|str_p("&=")|str_p("^=")|str_p("|=")
		                )[&do_enter_setop];

	typeofexpr= (
		            str_p("typeof")
		            >>skip>>expr
		        )[&do_typeof];
    
	primary_expr=
		           idexpr
		         | uinteger
		         | stringexpr
		         | patternexpr
		         | trueexpr 
		         | falseexpr
		         | nullexpr
		         | thisexpr
		         | newexpr
		         | typeofexpr
		         | arrayexpr
		         | (
		             ch_p('(')>>skip>>
		             expr
		             >>skip>>ch_p(')')
		           );//右递推
    

	arrayexpr=(
		        (ch_p('[')>>skip)[&do_endnewchar_array]>>
		                !(
		                  expr>>skip>>
		                  *(ch_p(',')>>skip>>expr>>skip)
		                 )
		        >>ch_p(']')
		      )[&do_newchar];

	postfix_expr= (
		           primary_expr >>
		                           !(
								      (str_p("("))[&do_enter_argumentslist]
		                              >>skip>>!argument_expr_list>>skip>>
		                              (str_p(")"))[&do_quit_argumentslist]
								   )
		                        >> 
		                           *(
		                               (
		                                 ch_p('[')>>skip>>expr>>skip>>ch_p(']')
		                               )[&do_offsetchar]
		                               |//区别成员函数和全局函数
		                               (
		                                 ch_p('.')>>skip>>idexpr[&not_member_function]>>
		                                 !(
		                                    (str_p("("))[&is_member_function]
		                                    >>skip>>
		                                    !argument_expr_list>>skip>>ch_p(')')
		                                  )
		                               )[&do_pe2]
		                            )
		         );

	
    postfix_expr2=postfix_expr>>
		                        *(
                                  (str_p("++"))[&OutputIPP]
		                          |
		                          (str_p("--"))[&OutputISS]
	                             );
    
	//函数调用参数列
	argument_expr_list=(
		                  (assignment_expr | functionexpr2)[&do_argument]
		                  |
	                      (
		                     (ch_p('{')>>skip)>>
		                     assignment_expr[&do_argument]>>!(
		                                                  skip>>ch_p(':')>>skip>>assignment_expr[&do_argument]
		                                                  )
		                     >>
		                     *(
		                        skip>>ch_p(',')>>skip>>
		                        assignment_expr[&do_argument]>>!(
		                                                     skip>>ch_p(':')>>skip>>assignment_expr[&do_argument]
		                                                        )
		                      )
		                     >>skip>>ch_p('}')
		                  )
                       )
		                >>
		                *(
		                    skip>>ch_p(',')>>skip>>
		                    (
		                       (assignment_expr | functionexpr2)[&do_argument]
		                       |
	                           (
		                          (ch_p('{')>>skip)>>
		                          assignment_expr[&do_argument]>>!(
		                                                  skip>>ch_p(':')>>skip>>assignment_expr[&do_argument]
		                                                  )
		                          >>
		                         *(
		                           skip>>ch_p(',')>>skip>>
		                           assignment_expr[&do_argument]>>!(
		                                                     skip>>ch_p(':')>>skip>>assignment_expr[&do_argument]
		                                                     )
		                          )
		                         >>skip>>ch_p('}')
		                       )
		                    )
		                 );

	unary_expr= postfix_expr2 
		       |
		        (str_p("++")>>skip>>unary_expr)[&OutputPPI]
		       |
                (str_p("--")>>skip>>unary_expr[&OutputSSI])
		       |
		        (ch_p('+')>>skip>>unary_expr)
		       |
		        (ch_p('-')>>skip>>unary_expr)[&do_neg]
		       |
		        (ch_p('!')>>skip>>unary_expr)[&do_opp]
		       ;

	multiplicative_expr=unary_expr >> *(
		                                 skip>>
		                                 (
		                                    ((ch_p('*')>>skip>>unary_expr)[&do_multi])
		                                    |
		                                    ((ch_p('/')>>skip>>unary_expr)[&do_div])
		                                    |
		                                    ((ch_p('%')>>skip>>unary_expr)[&do_mod])
		                                 )
		                               );

	additive_expr=multiplicative_expr>>*(
		                                  skip>>
		                                  (
		                                    ((ch_p('+')>>skip>>multiplicative_expr)[&do_add])
		                                    |
		                                    ((ch_p('-')>>skip>>multiplicative_expr)[&do_sub])
		                                  )
		                                );

	shift_expr=additive_expr>>*(
		                         skip>>
		                         (
		                            ((str_p("<<")>>skip>>additive_expr)[&do_left])
		                            |
		                            ((str_p(">>")>>skip>>additive_expr)[&do_right])
		                            |
		                            ((str_p(">>>")>>skip>>additive_expr)[&do_rightright])
		                         )
		                       );

	relational_expr=shift_expr>>*(
		                           skip>>
		                           (
		                             ((ch_p('>')>>skip>>shift_expr)[&do_gt])
		                             |
		                             ((ch_p('<')>>skip>>shift_expr)[&do_lt])
		                             |
		                             ((str_p(">=")>>skip>>shift_expr)[&do_gte])
		                             |
		                             ((str_p("<=")>>skip>>shift_expr)[&do_lte])
		                           )
		                         );

	equality_expr=relational_expr>>*(
		                             skip>>
		                             (
		                                ((str_p("==")>>skip>>relational_expr)[&do_equal])
		                                |
		                                ((str_p("!=")>>skip>>relational_expr)[&do_notequal])
		                             )
		                            );

	and_expr=equality_expr >>*(
		                       skip>>(ch_p('&')>>skip>>equality_expr)[&do_andbchar]
		                      );

	exclusive_or_expr=and_expr >>*(
		                            skip>>(ch_p('^')>>skip>>and_expr)[&do_inbchar]
		                          );

	inclusive_or_expr=exclusive_or_expr >>*(
		                                     skip>>(ch_p('|')>>skip>>exclusive_or_expr)[&do_orbchar]
		                                   );
    
	//对于&&进行跳转翻译,以模拟优化编译的效果
	logical_and_expr=inclusive_or_expr[&do_enter_logicaland] >> 
		                                  (
		                                   !(
	                                          (
		                                       +(
		                                          skip>>(str_p("&&"))[&do_bf]>>skip>>inclusive_or_expr
		                                        )
		                                      )[&backpatch_bf_bl]
		                                    )
		                                  )[&do_quit_logicaland];
    
	//对于||进行跳转翻译,以模拟优化编译的效果
	logical_or_expr=logical_and_expr[&do_enter_logicalor] >> 
		                                 (
		                                   !(
		                                      (
		                                        +(
	                                              skip>>(str_p("||"))[&do_bt]>>skip>>logical_and_expr
		                                         )
		                                      )[&backpatch_bt_bl]
		                                    )
		                                 )[&do_quit_logicalor];

	conditional_expr=logical_or_expr >>!(
		                                  skip>>(str_p("?"))[&do_entersel_bf]
		                                  >>skip>>
		                                  assignment_expr[&do_bl]
		                                  >>skip>> 
		                                  str_p(":")[&backpatch_bf]
		                                  >>skip>>
		                                  assignment_expr[&backpatch_bl_quitsel] 
		                                );

	constant_expr=conditional_expr;
	
	emptyexpr=(ch_p('{')>>skip>>ch_p('}'))[&do_emptyexpr];
	assignment_expr= conditional_expr>>
		                                *(
		                                   (
		                                     skip>>
	                                         assignment_operator
		                                     >>skip>>
		                                     (conditional_expr | functionexpr2 | emptyexpr)
		                                   )[&do_setop]
		                                 );
	
	//声明语句后可选紧跟赋初值
	//javascript中声明语句和赋值混为一体,只有var
	declarexpr=( 
		         (
		          ( 
		             lexeme_d[str_p("var")>>space_p]
		              >>skip>>idexpr
		          )[&OutputVarType] 
		          >> 
		            !(( ch_p('=')>>skip>> (conditional_expr | functionexpr2 | emptyexpr) )[&do_set]) 
		          >>
		            *( 
		                (ch_p(',')>>skip>>idexpr)[&OutputVarType] 
		                >> 
		                !((ch_p('=')>>skip>>conditional_expr)[&do_set])
		             )
		         )
			   );
    
	//new里面的构造参数列,注意new可能存在关联数组
	aaexpr=constant_expr >> !(ch_p(':')>>skip>>constant_expr)
		   >>
		   *(
		       skip>>ch_p(',')>>skip>> constant_expr>> !(ch_p(':')>>skip>>constant_expr)
		    );

    newexpr=( 
		      ((lexeme_d[str_p("new")>>space_p])[&do_endnewchar])
		      >>skip>>
		      (classnameexpr[&OutputSymbolCode])
		      >>
		      !(
		          ch_p('(') >>skip>> !aaexpr >>skip>> ch_p(')')
		       )
		    )[&do_newchar];

	expr=assignment_expr>>*(
		                    skip>>(ch_p(",")>>skip>>assignment_expr)[&do_commachar]
		                   );

//---------------------------------------------------------------------------------------

    nopexpr=(ch_p(';'))|eol_p;

    statement=
		      (expr>>((ch_p(';'))|eol_p))
			  |
		      ifexpr 
		      | 
		      forexpr 
		      | 
		      (declarexpr>>((ch_p(';'))|eol_p)) 
		      | 
		      jumpexpr
		      |
		      labelexpr
		      |
		      whiledoexpr 
		      | 
		      dowhileexpr
		      |
		      switchexpr
		      |
		      returnexpr
		      |
		      functionexpr3
		      |
		      withexpr
		      |
		      tryexpr
		      |
		      importexpr
		      |
		      exportexpr
		      | 
		      nopexpr
		      |
		      compoundstatement;

	statementlist=+(statement);

	compoundstatement=ch_p('{')>>skip>> !(statementlist) >>skip>>ch_p('}') ;
    
	ifexpr= ( 
		      ( 
		        (
	               (
		               (str_p("if"))[&do_enter_if] 
		               >>skip>>
		               ch_p('(')
		               >>skip>>
		               (
		                expr>>skip>>ch_p(')')
		               )[&do_bf] 
		           )[&do_enter_if_statement] 
		           >>skip>>
		           statement[&do_quit_if_statement]
		        )[&do_bl]
		      )[&backpatch_bf] 
		      >>skip>>
		      !( 
		          (
		             str_p("else")[&do_enter_if_statement] >>skip>> statement[&do_quit_if_statement] 
		          )[&backpatch_bl] 
		       )
		    )[&do_quit_if];
    
	//BF跳到for循环末尾,BL在每次循环后跳到statement开始处
    forexpr=( str_p("for")[&do_enter_for] >>skip>>
		       ch_p('(')
		       >>skip>>
               (
		         (
		             ( 
		               (declarexpr>>skip>>ch_p(';')) //for里面的int i=0,然后预设BL地址
		               | 
		               (expr>>skip>>ch_p(';'))
				       |
				       ch_p(';')
		             )[&do_empty_bl]
		             >>skip>>!(expr[&do_bf])
			         >>(skip>>ch_p(';'))[&set_proxy]//开启proxy,处理i++
		             >>skip>>
				     !(expr)
				 )//A;B;C
			   |
			     (
				  (declarexpr | expr)
				   >>skip>>
				   (str_p("in"))[&set_forin]
				   >>skip>>
				   postfix_expr
				 )//A in B
               )
			   >>skip>>
			  ((str_p(")")[&do_enter_for_statement])[&reset_proxy]

		      >>skip>>statement[&do_quit_for_statement]
			  )[&do_existed_bl]
	        )[&backpatch_bf_quit_for];
	//-----------------------------------------------------

	whiledoexpr=(
		           (str_p("while"))[&do_enter_for]
		           >>skip>>
		           (str_p("("))[&do_empty_bl]>>skip>>
		           expr[&do_bf]>>skip>>
		           (
		             str_p(")")[&do_enter_while_statement]
		             >>skip>>
		             statement[&do_quit_while_statement]
		           )[&do_existed_bl]
		        )[&backpatch_bf_quit_whiledo];

    //while语句特殊,需要BT跳转,break用BL跳转,continue用BT跳转
	dowhileexpr=(
		          (str_p("do"))[&do_enter_dowhile]
		        )[&do_enter_while_statement]
		        >>skip>>
		        statement[&do_quit_while_statement]
		        >>skip>>str_p("while")>>skip>>
		        (str_p("("))[&backpatch_dowhile_continuelist]
		        >>skip>>
		        expr[&do_existed_bt]>>(skip>>ch_p(')'))[&do_quit_dowhile];
    
    caseexpr=(lexeme_d[str_p("case")>>space_p])
		     >>skip>>constant_expr[&do_bf2]
		     >>skip>>
		    (
		      (str_p(":"))[&do_enter_casedefault]
		      >>skip>>
		      (!statementlist)[&backpatch_bf_pop]
		    )[&do_quit_casedefault];

	defaultexpr=str_p("default")
		        >>skip>>
		        (
		          (str_p(":"))[&do_enter_casedefault]
		          >>skip>>
		          (!statementlist)
		        )[&do_quit_casedefault];

    switchexpr=(str_p("switch"))[&do_enter_switch]
		       >>skip>>ch_p('(')>>skip>>expr[&reset_proxy]>>skip>>ch_p(')')
		       >>skip>>
	             (
		            (
	                  ch_p('{')>>
		              +(
		                  skip>>(caseexpr|defaultexpr)
		               )
		              >>skip>>ch_p('}')
		            )
		            |
		            (
		              caseexpr|defaultexpr
		            )
	             )[&do_quit_switch];

   functionexpr=
	              (
	               lexeme_d[str_p("function")>>space_p]>>skip>>functionid[&do_functionchar]
	              )[&do_enter_function]
	              >>skip>>ch_p('(')>>skip>>
	              !(
	                 idexpr[&do_argument_initialize]
	                 >>skip>>*(ch_p(',')>>skip>>
	                 idexpr[&do_argument_initialize])
	               )
	              >>skip>>ch_p(')')>>skip>>
	              (
	                ch_p('{') >>skip>> !statementlist >>skip>> ch_p('}')
	              )[&do_quit_function];

   //匿名函数
   functionexpr2=(str_p("function"))[&do_enter_unfunction]>>skip>>ch_p('(')>>skip>>
	             !(
	                idexpr[&do_argument_initialize]
	                >>skip>>*(ch_p(',')>>skip>>
	                idexpr[&do_argument_initialize])
	             )
	            >>skip>>ch_p(')')>>skip>>
	            (
	              ch_p('{') >>skip>> !statementlist >>skip>> ch_p('}')
	            )[&do_quit_unfunction];
   
   //直接执行的function
   functionexpr3= functionexpr | 
	              ( 
	                ch_p('(')>>skip>>functionexpr>>skip>>ch_p(')')
	                >>!(
	                     (
	                       (str_p("("))[&do_enter_argumentslist]>>skip>> 
	                       !argument_expr_list >>skip>>
	                       ch_p(')')
	                     )[&do_call_function]
	                   )
	              );
	              

   withexpr=( 
	           (
	              str_p("with")>>skip>>ch_p('(')>>skip>>postfix_expr>>skip>>ch_p(')')
	           )[&do_with]>>skip>>statement
	        )[&do_quit_with];

   catchexpr=(
	           str_p("catch") >>skip>> 
	           ch_p('(') >>skip>> idexpr >>skip>> ch_p(')')
	         )[&do_catch]
	         >>skip>> compoundstatement[&do_quit_catch];
   
   finallyexpr=(str_p("finally"))[&do_finally]>>skip>>
	           compoundstatement[&do_quit_finally];
   
   tryexpr=(str_p("try"))[&do_try] >>skip>> compoundstatement[&do_quit_try] 
	       >>skip>>catchexpr>> !(skip>>finallyexpr);
   

/*Netscape 4 支持*/
   importexpr=lexeme_d[str_p("import")>>space_p]>>skip>>postfix_expr>>*(ch_p(',')>>skip>>postfix_expr)>>((ch_p(';'))|eol_p);

   exportexpr=lexeme_d[str_p("export")>>space_p]>>skip>>postfix_expr>>*(ch_p(',')>>skip>>postfix_expr)>>((ch_p(';'))|eol_p);
/*Netscape 4 支持*/

   }//end definition
  }; 
}; //end gram_g
//------------------------------------------------------------------------------------------------------

void PrintCode(bool tofile=false)
{
 if(tofile==false)
   {
    for(int i=0;i<(int)code.size();i++)
    {
	 int index=code[i];
	 map<int,string>::iterator mi;
	 mi=symbol_table.find(index);
	 cout<<"LN"<<setw(4)<<i<<": ";
	 if(mi==symbol_table.end())
        cout<<uppercase<<hex<<setw(8)<<index<<" "<<dec<<setw(10)<<index<<"  "<<codeenv[i]<<endl;
	 else
        cout<<uppercase<<hex<<setw(8)<<index<<" "<<dec<<setw(10)<<mi->second<<"  "<<codeenv[i]<<endl;
    }
     cout<<"---------------------------------------------------------------------"<<endl;
   }
 else
   {
    ofstream ofs("result.txt");
    for(int i=0;i<(int)code.size();i++)
       {
	    int index=code[i];
	    map<int,string>::iterator mi;
	    mi=symbol_table.find(index);
	    ofs<<"LN "<<i<<": ";
	    if(mi==symbol_table.end())
           ofs<<index<<endl;
	    else
           ofs<<mi->second<<endl;
       }
    ofs<<endl;
    ofs.close();
   }
}
//------------------------------------------------------------------------------------------------------

/*
LOG文件格式
总共的处理数 成功的处理数 语法分析出错的处理数 编译出错的处理数
*/
void WriteLog(const int exit_reason,const string& jump_string)
{
 FILE *logfile=NULL;
 logfile=fopen("yajsc_log","r");
 int totalnum=0;
 int successnum=0;
 int matchfailnum=0;
 int compilefailnum=0;
 if(logfile)//已经存在
   {
	char temp[512];
	int rl=fread(temp,sizeof(char),512,logfile);
	temp[rl]=0;
	string line=temp;
    int p1=line.find(" ");
	totalnum=atoi(line.substr(0,p1).c_str());
	int p2=line.find(" ",p1+1);
	successnum=atoi(line.substr(p1+1,p2-p1-1).c_str());
	int p3=line.find(" ",p2+1);
	matchfailnum=atoi(line.substr(p2+1,p3-p2-1).c_str());
	compilefailnum=atoi(line.substr(p3+1).c_str());
	fclose(logfile);
   }
 ofstream ofs("yajsc_log");
 switch(exit_reason)
	   {
	    case EXIT_REASON_SUCCESS:
			 ofs<<totalnum+1<<" "<<successnum+1<<" "<<matchfailnum<<" "<<compilefailnum<<endl;
			 break;
		case EXIT_REASON_MATCHFAIL:
			 ofs<<totalnum+1<<" "<<successnum<<" "<<matchfailnum+1<<" "<<compilefailnum<<endl;
			 break;
		case EXIT_REASON_COMPILEFAIL:
			 ofs<<totalnum+1<<" "<<successnum<<" "<<matchfailnum<<" "<<compilefailnum+1<<endl;
			 break;
	   }
 ofs.close();
}
//------------------------------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
 int exit_reason=EXIT_REASON_SUCCESS;
 if(argc<2)
   {
	cout<<"Wrong argument number!"<<endl;
	return 0;
   }
 struct timeval st;

 string argv1=argv[1];
 if(argv1.size()>bc_ext.size() && argv1.substr(argv1.size()-bc_ext.size(),bc_ext.size())==bc_ext )//is object file
   {
    BinaryCode bc;
    BinaryCode::LoadFormFile(argv1,bc);
    code=bc.code;
    codeenv=bc.codeenv;
    symbol_table=bc.symbol_table;
   }
 else
   {
 string host="";
 string path="";
 string url="";
 if(argc==3)
   {
    string param3=string(argv[2]);
	if(param3!="-log")
	  {
	   url=param3;
       ParseURL(url,host,path);
	  }
   }
 Initialize(host,path,url);
 gram_g g;
 MyScanner scanner;
 BOOST_SPIRIT_DEBUG_NODE(g);
 
 //Scanner
 st=BeginTiming();
 string s=scanner.GetSourceFromFilename(argv1);
 cout<<"---------------------------------------------------------------------"<<endl;
 cout<<"Scanner Result(time cost "<<EndTiming(st)<<" sec):"<<endl<<endl<<s<<endl;
 cout<<"---------------------------------------------------------------------"<<endl;
 
 //Scanner
 
 //------Parser---------------------------------------------------------------------
 const char* first=s.c_str();
 gettimeofday(&st,NULL);
 tree_parse_info<> info=pt_parse(first,g,blank_p);
   
 code.push_back(codeend);
 codeenv.push_back(global);
 st=BeginTiming();
 //cout<<"Codeenv Size:"<<codeenv.size()<<" Code Size:"<<code.size()<<endl<<endl;
 if(info.full && parse_flag)
	{  
	 PrintCode();
	 cout<<"Parsed Successfully(time cost "<<EndTiming(st)<<" sec)"<<endl;
	 //tree_to_xml(cout,info.trees,first);
     BinaryCode bc(code,codeenv,symbol_table);
     BinaryCode::SaveToFile(argv1,bc);
	} 
 else
    {
	 cout<<"Match Failed @"<<endl;
     exit_reason=EXIT_REASON_MATCHFAIL;
	 //cout<<info.stop<<endl;
	}
 }//end else object file
 //------Parser---------------------------------------------------------------------

 //------Compiler-------------------------------------------------------------------
 string result_string;
 if(exit_reason!=EXIT_REASON_MATCHFAIL)
   {
    st=BeginTiming();
    Compiler c(symbol_table);
    bool crv=c.compile(code,codeenv);
    if(crv)
	   exit_reason=EXIT_REASON_SUCCESS;
    else
	   exit_reason=EXIT_REASON_COMPILEFAIL;
    cout<<"(time cost "<<EndTiming(st)<<" sec)"<<endl;
    cout<<"---------------------------------------------------------------------"<<endl;
    //------Compiler-------------------------------------------------------------------
    
    //c.PrintValueInStack();
    //cout<<"Runtime Stack Size: "<<c.GetStackSize()<<endl;
    c.PrintValueInHeap();
    cout<<endl;
    //c.PrintValueInTable();
    cout<<"---------------------------------------------------------------------"<<endl;
    
    code.clear();
    codeenv.clear();
    string nfn=argv1+".jump";
    result_string=c.GetResult();
    ofstream ofst(nfn.c_str());
    ofst<<result_string;
    ofst.close();
   }
 
 if(string(argv[argc-1])=="-log")
   {
	WriteLog(exit_reason,result_string);
   }
 return 0;
}
//------------------------------------------------------------------------------------------------------

//接口类
void JSIInterface::Prepare()
{
 Initialize(host,path,parsing_url);
}
//------------------------------------------------------------------------------------------------------

bool JSIInterface::Scan()
{
 MyScanner myscanner;
 if(!filename.empty())
    jscode=myscanner.GetSourceFromFilename(filename);
 else if(is!=NULL)
	jscode=myscanner.GetSourceFromStream(*is);
 else if(!original_code.empty())
	jscode=myscanner.GetSourceFromString(original_code);
 else
	return false;
 jscode=addtional_code+"\n"+jscode;
 if(jscode.empty())
	return false;
 return true;
}
//------------------------------------------------------------------------------------------------------

string JSIInterface::Compile(bool withlog)
{
 int exit_reason=EXIT_REASON_SUCCESS;
 string result="";
 gram_g g;
 
 BOOST_SPIRIT_DEBUG_NODE(g);
 const char* first=jscode.c_str();
 
 tree_parse_info<> info=pt_parse(first,g,blank_p);
 if(info.full && parse_flag)//语法分析成功
   {
	code.push_back(codeend);
    codeenv.push_back(global);
	Compiler c(symbol_table);

    bool rv=c.compile(code,codeenv);
	if(rv)
	  {
	   exit_reason=EXIT_REASON_SUCCESS;
	   result=c.GetResult();
	  }
	else
	  {
	   exit_reason=EXIT_REASON_COMPILEFAIL;
	  }
   }//end info.full
 else
   {
	exit_reason=EXIT_REASON_MATCHFAIL;
   }
 
 if(withlog)//写LOG
   {
   }
 return result;
}
//------------------------------------------------------------------------------------------------------

void JSIInterface::SetAddtionalCode(const string& str)
{
 addtional_code=str;
}
//------------------------------------------------------------------------------------------------------

void JSIInterface::SetURL(const string& str)
{
 parsing_url=str;
 ParseURL(parsing_url,host,path);
}
//------------------------------------------------------------------------------------------------------
