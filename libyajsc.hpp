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
��̬��ӿں���

����
code: �����js����,��\0����,�������jsע���Լ�htmlע��
addtional_code: ���ӵ�js����,��\0����,�������òο�readme
buf: ��ŷ��ؽ���Ļ���ռ�,�����ַ�����\0����
buf_length: ����ռ�Ĵ�С,���׼�����ص��ַ�����С���ڵ��ڻ�������С,�򲻸ı�buf
url: ��ǰҳ���url��ַ,��http://gb3764.sme.cn/cms/redirect.jsp;jsessionid=76DEE4E313F365C403C07E80C999CB80

����ֵ
1��ʾ�ɹ�,0��ʾʧ��
*/
extern "C" int yajsc_compile_string(const char* code,const char* addtional_code,char* buf, unsigned int buf_length);

extern "C" int yajsc_compile_string_withlog(const char* code,const char* addtional_code,
                                            char* buf, unsigned int buf_length);

extern "C" int yajsc_compile_string_withurl(const char* code,const char* addtional_code,
                                            const char* url,char* buf,unsigned int buf_length);
//-------------------------------------------------------------------
#endif
