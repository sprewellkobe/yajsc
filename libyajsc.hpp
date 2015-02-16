//-------------------------------------------------------------------
//Provide the interface to CU, by kobe
#ifndef LIBYAJSCHPP
#define LIBYAJSCHPP
#include "Main.hpp"
#include "Common.hpp"
#include <string>
#include <iostream>
using namespace std;
//-------------------------------------------------------------------
typedef struct
{
 unsigned int totalnum;
 unsigned int matchfailnum;
 unsigned int compilefailnum;
 unsigned int successnum;
} YAJSC_RECORD;

YAJSC_RECORD yr;
/*
动态库接口函数

参数
code: 输入的js代码,以\0结束,允许包括js注释以及html注释
addtional_code: 附加的js代码,以\0结束,具体作用参考readme
buf: 存放返回结果的缓冲空间,返回字符串以\0结束
buf_length: 缓冲空间的大小,如果准备返回的字符串大小大于等于缓冲区大小,则不改变buf
url: 当前页面的url地址,如http://gb3764.sme.cn/cms/redirect.jsp;jsessionid=76DEE4E313F365C403C07E80C999CB80

返回值
1表示成功,0表示失败
*/
extern "C" int yajsc_compile_string(const char* code,const char* addtional_code,char* buf, unsigned int buf_length);

extern "C" int yajsc_compile_string_withlog(const char* code,const char* addtional_code,
                                            char* buf, unsigned int buf_length);

extern "C" int yajsc_compile_string_withurl(const char* code,const char* addtional_code,
                                            const char* url,char* buf,unsigned int buf_length);
//-------------------------------------------------------------------
#endif
