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
��Ŀָ���
+ -(���͸���) * / ++ -- BL
>= <= == < > != 

˫Ŀָ���
= += -= *= /= BF
*/
extern int bfchar;//BF
extern int bf2char;//�����BFָ��,�޸�Ϊ��BFͬ������
extern int blchar;//BL
extern int btchar;//BT
extern int labelchar;//LABEL
extern int functionchar;//FUNCTION
extern int unfunctionchar;//����FUNCTION����

extern int inichar;//INITIALIZE
extern int efchar;//EF END FUNC
extern int unefchar;//����FUNCTION������ֹ

extern int newchar;//NEW
extern int endnewchar;//END NEW
extern int offsetchar;//ƫ��ָ��,���������[X]

//�����
extern int pluschar;//+
extern int subchar;//-
extern int multichar;//*
extern int divchar;// /
extern int negchar;//��Ŀ�����,ȡ�� -
extern int oppchar;//��Ŀ�����,ȡ�� !
extern int modchar;//˫Ŀ,ȡ���� %
//˫Ŀλ�����
extern int leftchar;// <<
extern int rightchar;// >>
extern int rightrightchar;// >>>
extern int andbchar;// &
extern int orbchar;// |
extern int inbchar;// ^
//�߼������
extern int andchar;// &&
extern int orchar;// ||
//��ϵ��
extern int gtechar;//>=
extern int ltechar;//<=
extern int eqchar;//==
extern int noteqchar;//!=

extern int gtchar;//>
extern int ltchar;//<
//��Ŀ��ֵ��
extern int ipp;//i++
extern int ppi;//++i
extern int iss;//i--
extern int ssi;//--i
//˫Ŀ��ֵ��
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
//��������ָ��
extern int eachar;//��Ŀ,��ò�������,����ȫ�ֺ���
extern int eachar2;//˫Ŀ,��ò�������,���ڳ�Ա����
extern int retchar;//��Ŀ,����sp��ַ
extern int bpchar;//ջ��ַ,����ջ����ĩ��ַ
extern int returnchar;//��Ŀ
extern int return0char;//ֱ�ӷ���ָ��
//�����
extern int commachar;// ,
extern int dotchar;// .
extern int withchar;//with
extern int quitwithchar;//with��ֹ
extern int nanchar;// NaN

extern int trychar;
extern int quittrychar;
extern int catchchar;
extern int quitcatchchar;
extern int finallychar;
extern int quitfinallychar;

extern int typeofchar;// typeof

//�������ͷ�,��Ŀ itemʹ
extern int inttype;
extern int stringtype;
extern int vartype;

//����,��Ŀ
extern int arraytype;
//------------------------------------------------------------------------
extern int id_start_pos;//ID�ķ�����ʼֵ

extern int current_string_index;//����

extern int current_command_index;//ָ��

extern int current_id_index;// ID

extern int temp_id_index;//��ʱ����������returnʱ��
extern int array_id_index;

const int empty_string_index=INT_MIN;//�մ�
const int host_string_index=INT_MIN+1;//location�����host��hostname
const int path_string_index=INT_MIN+2;//location�����pathname
const int url_string_index=INT_MIN+3;//document�����URL
//��ֹ��
const int codeend=-10000;//��ֹ��
//------------------------------------------------------------------------
const int split_number=-10000;//ָ���ID�ķֽ���

const int global=0;//��ʼ����������
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
