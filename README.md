# yajsc
yet another javascript compiler

YAJSC README

整体架构:

URL --Crawler--> html --Parser1--> javascript source --Scanner--> pure javascript source --Parser2--> AST(Abstract Syntax Tree)

--Translation--> RPN code & symbol table --Precompile--> function name tree & RPN code & symbol table --Compiler--> final result

模块
说明
实现文件
Crawler
抓取器
GetScript.java
Parser1
自顶向下语法分析器,可以提出script块,以及各部tag属性
GetScript.java
Scanner
扫描器,去除代码中的注释(包括html注释),去除多余的换行符
Main.cpp
Parser2
自顶下向语法分析器,支持LL(k),带回朔
Main.cpp , boost::spirit
Translation
代码翻译及目标代码生成,逆波兰翻译,同时产生运行地址对应表
Main.cpp
Precompile
前扫描,生成函数名树,作用:1,支持函数的向后跳转; 2,区分全局函数和局部函数和嵌套函数

Compiler.cpp
Comiler
解释执行器,栈推结构
Compiler.cpp

代码结构:

Common.hpp Common.cpp
一些公用的函数和类

Error.hpp Error.cpp
用于错误显示的TError

Heap.hpp Heap.cpp
运行堆,结构为vector<vector>, 维护一个可用地址栈,根据引用计数将释放后的空间放入栈中,以达到空间重复利用的目的

SymbolTable.hpp SymbolTable.cpp
符号表,兼用局部运行栈的作用,结构为map<string,vector>,vector为下推栈; 当某一符号退出当前环境时,由符号表负责弹出符号; 当退出全局函 数体内部时,由符号表负责将所有变量声明销毁

Yajsc.hpp Yajsc.cpp
提供静态库接口

Simulation.hpp Simulation.cpp
内建javascript核心对象,实现javascript自带全局函数

Main.hpp Main.cpp
负责前扫描,内含javascript文法描述,根据语义动作建立地址映射和RPN代码

Compiler.hpp Compiler.cpp
解释RPN代码,通过运行栈执行代码

libyajsc.hpp libyajsc.cpp
提供动态库接口

类名作用表:

类名	作用
MyScanner	扫描器
gram_g	文法描述
DoubleJumpList	RPN翻译环境,包括回填地址链
FNTreeNode	函数名树节点
FNTree	函数名树
Compiler	编译执行器
TItem	符号表项
IntValue	中间代码项
TError	错误处理
THeap	运行期堆
TSymbolTable	运行期符号表
TSimulation	建立javascript核心对象及全局函数
JSIInterface	包装接口
Yajsc	静态库接口

javascript支持度:

全局函数支持表:

全支持(作用与js一样)
半支持(不影响页面效果,模拟js效果)
不支持(不影响页面效果,作用未实现)
parseInt
escape
taint
parseFloat
write
untaint
String
registerCFunction
Number
getOptionValue
redirect
getOptionValueCount
flush
callC
addClient
ssjs_getClientID
deleteResponseHeader
blob
addResponseHeader
unescape
debug
Function
ssjs_generateClientID
ssjs_getCGIVariable
isNaN
Array
核心对象支持表:

对象名
全支持
半支持
不支持
Math	abs, max, min, pow, floor, ceil	
E, LN10, LN2, LOG10E, LOG2E, PI, SQRT1_2, random, cos, sin, tan, acos, asin, atan, atan2, exp, sqrt, log, round

 
Object	toString	prototype	 
Array	length, join, push, pop, reverse	shift,concat,slice,splice,sort,unshift	 
Date	getDate, getDay, getMonth,getYear, getMinutes, getSeconds, getTime, getHours, setDate, setHours, setMinutes, setMonth, setSeconds, setYear, setTime	getFullYear, getTimezoneOffset, parse, setFullYear, toGMTString, toLocaleString	 
String	length, toLowerCase, toUpperCase, substring, substr, slice, split, search, match, replace, indexOf, lastindexOf, concat, charAt	 	 
RegExp	compile, exec, test	 	 
Number	MAX_VALUE, MIN_VALUE, NaN	POSITIVE_INFINITY, NEGATIVE_INFINITY	 
Boolean	 	 	 
 


JSI进程命令行调用格式:

./jsi.exe 目标文件名 URL(可选) -log(可选)

目标文件名: 目标js代码文件名, 同下面YAJSC调用接口说明里的filename
URL: 可选,javascript所在页面的URL,该URL帮助javascript在解析过程中分析host,hostname,pathname
     http://x.com/a.html#123 => host=x.com hostname=x.com pathname=/a.html 
-log: 可选,输出log文件在当前目录下,文件名为yajsc_log

例子:
./jsi.exe a.js http://x.com/a.html#123 -log
./jsi.exe a.js -log

log格式:
总共处理过的js文件数量(int) 处理成功的js文件数量(int) 语法编译阶段发生错误的js文件数量(int) 代码执行阶段发生错误的js文件数量(int)

例子:
90 83 2 5
总共处理过90个js代码文件,其中83个被成功处理,2个文件存在着错误(或不支持)的js语法,5个文件编译后执行期出现错误

 

 

YAJSC调用接口说明:

class Yajsc
{
public:
Yajsc(){};
string Compile(const string& filename,string addtional_code="");
string Compile(istream& ifs,string addtional_code="");
};

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



JSI动态库文件调用格式:

文件名 libyajsc.so(libyajsc.hpp libyajsc.cpp)

调用函数:
extern "C" int yajsc_compile_string(const char* code,const char* addtional_code,char* buf, unsigned int buf_length);

extern "C" int yajsc_compile_string_withlog(const char* code,const char* addtional_code,
char* buf, unsigned int buf_length);

extern "C" int yajsc_compile_string_withurl(const char* code,const char* addtional_code,
const char* url,char* buf,unsigned int buf_length);

说明:
{
1, code是传入的javascript代码(以\0结束),要求同上面调用格式中的filename文件对应的内容的要求(可以包含注释等)

2, addtional_code是额外解释的代码(以\0结束),要求同上面调用格式中的addtional_code的要求. 如果没有addtionalcode,可将其设为NULL

3, buf为返回的结果字符串的存放空间,返回的字符串以\0结束,buf_length为空间大小,如果需要返回的结果长度大于等于存放空间大小,则不对buf做任何变化,且返回0

4, url为被解释js所在页面的URL(以\0结束).
}



YAJSC返回值说明:

返回string反映该javascript所起的作用
{
作用:
1, JUMP 表明javascript跳转,包含下面几种情况:
   a, 对于location系列对象进行左赋值,PS:所谓系列对象,指DOM树中location点和它的子节点
   b, 对于location系列对象调用replace操作(reload除外)
   c, 调用js全局函数redirect()
   d, 实现window对象的onload函数,在此函数中,调用上面四种情况

2, UNKNOWNJMP 表明javascript跳转,但是不确定其跳转的URL:
   a, 跳转到未知的对象,情况包括1里面的各种情况
   b, 调用window对象函数setTimeout(),其中第一个参数中含有location字符串,PS:对于此类情况,url一律是"unknown_site"
   c, 注意此处是UNKNOWNJMP,不是JUMP

3, POP 表明javascript弹出窗口(window),包含下面几种情况:
   a, 对于window系列对象调用open操作
   b, 对于未知对象调用open操作,且参数为字符串

4, BAD 表明该javascript有恶意作用,包含下面几种情况:
   a, 跳转到同一字节代码地址大于8096次,判断为无限循环
   b, 连续在同一地址执行alert大于32次,判断为恶搞页面(未实现)

5, ALERT 表明该javascript启动时有弹出对话框(DialogBox),包含以下几种情况:
   a, 调用window对象的alert
   b, 调用window对象的confirm
   c, 调用window对象的prompt


格式:
JUMP空格URL\n
UNKNOWNJMP空格URL\n
POP空格URL\n
BAD\n
ALERT空格\n

说明:
1, 所有URL未经相对地址到绝对地址的转换
2, 对于可以的跳转,如location=未知对象,URL设为unknownsite
3, 未知对象是指庞大的DOM树中所有未加以实现的对象
}

{
例子:

----------------------------------------------------------------------------------
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

----------------------------------------------------------------------------------
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

----------------------------------------------------------------------------------
如3:
"
alert("password error");
"
输出:
ALERT

----------------------------------------------------------------------------------
如4:
"
setTimeout("document.location.href='news/index.asp'")
" 
输出:
UNKNOWNJMP

----------------------------------------------------------------------------------
如5:
"
window.onload=f
function f()
{location="xxx.com"} 
" 
输出:
JUMP xxx.com

-----------------------------------------------------------------------------------
如6:
"
var c=0; 
function window.onload()
{
location="xxx.com"
}
"
输出:
JUMP xxx.com

----------------------------------------------------------------------------------
如7:(URL: http://gb3764.sme.cn/cms/redirect.jsp;jsessionid=76DEE4E313F365C403C07E80C999CB80)
"
host = window.location.host;
var re, href,my;
re = new RegExp("\\.","i");
var replace="\.my\.";
href=host.replace(re,replace);
location.href="http://"+href;
" 
输出:
JUMP http://gb3764.my.sme.cn

----------------------------------------------------------------------------------
}

Frist edited @ Oct 23 2006, Updated @ Apr 24 2007, by kobe

 
