//----------------------------------------------------------------------------
//Provide the static interface to CU, by kobe
#ifndef YAJSCHPP
#define YAJSCHPP
#include "Main.hpp"
#include "Common.hpp"
#include <string>
#include <iostream>
using namespace std;
//-----------------------------------------------------------------------------
/*
调用说明:
filename是存放着js代码的文件名
istream是存放着js代码的字节流
{
 要求:
      1, 包含的是javascript代码(大小写不区分!)
      2, 可以包含注释(注释内容由Scanner负责除去),js包含注释格式同java语言
	  3, 可以包含html注释,包括<!--和-->,这两种注释字符串会在Scan过程被滤掉,而两个标志中间的代码保留

}
addtional_code是存放需要额外解释的代码
{
 要求:
      1, 该字符串是指除<script></script>标签外,额外需要解释的代码,比较重要的应用是当page onload中调用f(),可以把f()当
	     做addtional_code;
		 例子: onload="javascript::f()" => addtional_code="f()"
	  
	  2, addtional_code可以理解为增加程序的执行点,或者入口点,该代码不经过Scanner,所以绝对不能包含任何注释!!!
	  
	  3, addtional_code默认为空,其值建议由onload事件的调用方法决定
}
*/
/*
返回值说明:
返回string反映该javascript所起的作用
{
 作用:
      1, JUMP 表明javascript跳转,包含下面几种情况:
	     a, 对于location系列对象进行左赋值,(所谓系列对象,指DOM树中location点和它的子节点)
		 b, 对于location系列对象调用replace操作(reload除外)
		 c, 调用js全局函数redirect()
	 
	  2, POP 表明javascript弹出窗口,包含下面几种情况:
	     a, 对于window系列对象调用open操作
		 b, 对于未知对象调用open操作,且参数为字符串
	 
	  3, BAD 表明该javascript有恶意作用,包含下面几种情况:
	     a, 跳转到同一字节代码地址大于8096次,判断为无限循环
		 b, 连续在同一地址执行alert大于32次,判断为恶搞页面(未实现)
 
 格式:
JUMP URL\n
POP URL\n
BAD\n

 说明:
 1, 所有URL未经相对地址到绝对地址的转换
 2, 对于可以的跳转,如location=未知对象,URL设为unknownsite
 3, 未知对象是庞大的DOM树中所有未加以实现的对象
}
{
例子:

如1:
"
 var url="/index.jsp"
 with(location)
     {
	  replace(url)
	 }
 window.open("http://www.ad.com.cn/ad.htm")
"
输出:
JUMP /index.jsp
POP http://www.ad.com.cn/ad.htm

如2:
"
for(;;)
   {
	if(1==0)
	   break;
   }
"
 输出:
 BAD
}
*/
class Yajsc
{
 public:
  Yajsc(){};
  string Compile(const string& filename,string addtional_code="");
  string Compile(istream& ifs,string addtional_code="");
};
//-----------------------------------------------------------------------------
#endif
