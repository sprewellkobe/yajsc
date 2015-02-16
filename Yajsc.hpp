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
����˵��:
filename�Ǵ����js������ļ���
istream�Ǵ����js������ֽ���
{
 Ҫ��:
      1, ��������javascript����(��Сд������!)
      2, ���԰���ע��(ע��������Scanner�����ȥ),js����ע�͸�ʽͬjava����
	  3, ���԰���htmlע��,����<!--��-->,������ע���ַ�������Scan���̱��˵�,��������־�м�Ĵ��뱣��

}
addtional_code�Ǵ����Ҫ������͵Ĵ���
{
 Ҫ��:
      1, ���ַ�����ָ��<script></script>��ǩ��,������Ҫ���͵Ĵ���,�Ƚ���Ҫ��Ӧ���ǵ�page onload�е���f(),���԰�f()��
	     ��addtional_code;
		 ����: onload="javascript::f()" => addtional_code="f()"
	  
	  2, addtional_code�������Ϊ���ӳ����ִ�е�,������ڵ�,�ô��벻����Scanner,���Ծ��Բ��ܰ����κ�ע��!!!
	  
	  3, addtional_codeĬ��Ϊ��,��ֵ������onload�¼��ĵ��÷�������
}
*/
/*
����ֵ˵��:
����string��ӳ��javascript���������
{
 ����:
      1, JUMP ����javascript��ת,�������漸�����:
	     a, ����locationϵ�ж��������ֵ,(��νϵ�ж���,ָDOM����location��������ӽڵ�)
		 b, ����locationϵ�ж������replace����(reload����)
		 c, ����jsȫ�ֺ���redirect()
	 
	  2, POP ����javascript��������,�������漸�����:
	     a, ����windowϵ�ж������open����
		 b, ����δ֪�������open����,�Ҳ���Ϊ�ַ���
	 
	  3, BAD ������javascript�ж�������,�������漸�����:
	     a, ��ת��ͬһ�ֽڴ����ַ����8096��,�ж�Ϊ����ѭ��
		 b, ������ͬһ��ִַ��alert����32��,�ж�Ϊ���ҳ��(δʵ��)
 
 ��ʽ:
JUMP URL\n
POP URL\n
BAD\n

 ˵��:
 1, ����URLδ����Ե�ַ�����Ե�ַ��ת��
 2, ���ڿ��Ե���ת,��location=δ֪����,URL��Ϊunknownsite
 3, δ֪�������Ӵ��DOM��������δ����ʵ�ֵĶ���
}
{
����:

��1:
"
 var url="/index.jsp"
 with(location)
     {
	  replace(url)
	 }
 window.open("http://www.ad.com.cn/ad.htm")
"
���:
JUMP /index.jsp
POP http://www.ad.com.cn/ad.htm

��2:
"
for(;;)
   {
	if(1==0)
	   break;
   }
"
 ���:
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
