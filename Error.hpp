//-------------------------------------------------------------------------------------------------
//Provide Error Handler, by kobe
#ifndef ERRORHPP
#define ERRORHPP
#include <string>
#include <iostream>
#include "Common.hpp"
using namespace std;
//-------------------------------------------------------------------------------------------------
//Runtime Error
 const static int Error_dividebyzero=0;
 const static int Error_expectdefinition=1;
 const static int Error_labelinsameenv=2;
 const static int Error_symbolinsameenv=3;
 const static int Error_functionsameenv=4;
 const static int Error_coundfindfuncname=5;
 const static int Error_objectnotsupportoperation=6;
 const static int Error_arrayindexexceed=7;
 const static int Error_arraysizebelowzero=8;
//Syntax Error
 const static int Error_setvaluetonumber=100;
 const static int Error_argumentsnumbernotmatch=101;
 const static int Error_argumentstypenotmatch=102;
 const static int Error_functionnameisnotid=103;
 const static int Error_stringnotsupportoperation=104;
 const static int Error_needrightclassname=105;
 const static int Error_takenumberasclassname=106;
 const static int Error_stringcomparenumber=107;
 const static int Error_objectnameisnotid=108;
 const static int Error_constructoragruments=109;
 const static int Error_bitoperationneedrighttype=110;
 const static int Error_integernothaveanyattribute=111;
 const static int Error_classnothavethismethod=112;
 const static int Error_classnothavethisattribute=113;

//Compiler Error
 const static int Error_internalerror=900;
 const static int Error_stackfail=901;
 const static int Error_withstackfail=902;
 const static int Error_fatalerror=903;
 const static int Error_unknowntype=904;
 const static int Error_regexerror=905;

 //Parser Error
 const static int Error_breaknotiniteration=1000;
 const static int Error_jumplistfail=1001;
 const static int Error_pfatalerror=1002;
 //Warning
 const static int Warning_arrayaccesserror=1500;
 const static int Warning_compilernotrealizethismethod=1501;
//-------------------------------------------------------------------------------------------------
 
class TError
 {
  public:
   static void ShowError(const int& errortype,const int& sp,const IntValue& iv,const string& name);
   static void ShowWarning(const int& warningtype,const int& sp,const IntValue& iv,const string& name);
   static void ParsingError(const int& errortype,const string& str);
 };
//-------------------------------------------------------------------------------------------------
#endif
