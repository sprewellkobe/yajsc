#include "Error.hpp"
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void TError::ShowError(const int& errortype,const int& sp,const IntValue& iv,const string& name)
{
 string errort;
 if(errortype<100)
   {
	errort="Runtime Error: ";
	switch(errortype)
	      {
		   case Error_dividebyzero:
			    cout<<errort<<"divided by zero @[LN"<<sp<<"]"<<endl;
		        break;
		   case Error_expectdefinition:
			    cout<<errort<<name<<" expect definition @[LN"<<sp<<"]"<<endl;
		        break;
		   case Error_labelinsameenv:
			    cout<<errort<<"label "<<name<<" already existed in same level @[LN"<<sp<<"]"<<endl;
		        break;
		   case Error_symbolinsameenv:
			    cout<<errort<<"symbol "<<name<<" already existed in same level @[LN"<<sp<<"]"<<endl;
		        break;
		   case Error_coundfindfuncname:
		        cout<<errort<<"could not find function "<<name<<" @[LN"<<sp<<"]"<<endl;
				break;
		   case Error_objectnotsupportoperation:
			    cout<<errort<<"object could not support some operation"<<name<<" @[LN"<<sp<<"]"<<endl;
		        break;
		   case Error_arrayindexexceed:
			    cout<<errort<<"array index exceeded @[LN"<<sp<<"]"<<endl;
		        break;
           case Error_arraysizebelowzero:
                cout<<errort<<"array size below zero @[LN"<<sp<<"]"<<endl;
                break;
		  }//end switch
   }
 else if(errortype<900)
   {
	errort="Syntax Error: ";
    switch(errortype)
	      {
		   case Error_setvaluetonumber:
			    cout<<errort<<"set value to number "<<iv.value<<" @[LN"<<sp<<"]"<<endl;
		        break;
		   case Error_argumentsnumbernotmatch:
			    cout<<errort<<"arguments number not same as function declaration @[LN"<<sp<<"]"<<endl;
		        break;
		   case Error_argumentstypenotmatch:
			    cout<<errort<<"arguments type not same as function declaration @[LN"<<sp<<"]"<<endl;
		        break;
	       case Error_functionnameisnotid:
			    cout<<errort<<"function name is not a valid id @[LN"<<sp<<"]"<<endl;
		        break;
		   case Error_stringnotsupportoperation:
			    cout<<errort<<"string not support some operation @[LN"<<sp<<"]"<<endl;
		        break;
		   case Error_needrightclassname:
			    cout<<errort<<"need a right class name @[LN"<<sp<<"]"<<endl;
			    break;
		   case Error_takenumberasclassname:
			    cout<<errort<<"take number as class name @[LN"<<sp<<"]"<<endl;
		        break;
		   case Error_stringcomparenumber:
			    cout<<errort<<"string compare with number @[LN"<<sp<<"]"<<endl;
		   case Error_objectnameisnotid:
			    cout<<errort<<"object name is not a valid id @[LN"<<sp<<"]"<<endl;
		        break;
		   case Error_constructoragruments:
			    cout<<errort<<"problem with construtor's agruments @[LN"<<sp<<"]"<<endl;
		        break;
		   case Error_bitoperationneedrighttype:
			    cout<<errort<<"bit operation need a right type id @[LN"<<sp<<"]"<<endl;
		   case Error_integernothaveanyattribute:
			    cout<<errort<<"integer donot have any attribute @[LN"<<sp<<"]"<<endl;
		        break;
		   case Error_classnothavethismethod:
			    cout<<errort<<"object not support "<<name<<" method @[LN"<<sp<<"]"<<endl;
		        break;
		   case Error_classnothavethisattribute:
			    cout<<errort<<"object not support "<<name<<" attribute @[LN"<<sp<<"]"<<endl;
		        break;
		   case Error_functionsameenv:
			    cout<<errort<<"same function names "<<name<<" existed @[LN"<<sp<<"]"<<endl;
			    break;
		  }//end switch
   }
 else if(errortype<1000)
   {
	errort="Compiler Error: ";
	switch(errortype)
	      {
		   case Error_internalerror:
			    cout<<errort<<"internal error @[LN"<<sp<<"]"<<endl;
		        break;
           case Error_stackfail:
			    cout<<errort<<"stack fail @[LN"<<sp<<"]"<<endl;
		        break;
		   case Error_withstackfail:
			    cout<<errort<<"with stack fail @[LN"<<sp<<"]"<<endl;
			    break;
		   case Error_fatalerror:
			    cout<<errort<<"fatal error @[LN"<<sp<<"]"<<endl;
		        break;
		   case Error_unknowntype:
			    cout<<errort<<"meet a unknown type in stack @[LN"<<sp<<"]"<<endl;
		        break;
           case Error_regexerror:
                cout<<errort<<"regex "<<name<<" construction failed @[LN"<<sp<<"]"<<endl;
                break;
		  }//end switch
   }
}
//-------------------------------------------------------------------------------------------------

void TError::ParsingError(const int& errortype,const string& str)
{
 string errort="Parser Error: ";
 switch(errortype)
       {
        case Error_breaknotiniteration:
             cout<<errort<<"break not in iteration statement(for whiledo dowhile) @"<<str<<endl;
		     break;
		case Error_jumplistfail:
			 cout<<errort<<"jumplist fail @"<<str<<endl;
		     break;
	    case Error_pfatalerror:
			 cout<<errort<<"fatal error"<<str<<endl;
		     break;
       }
}
//-------------------------------------------------------------------------------------------------

void TError::ShowWarning(const int& warningtype,const int& sp,const IntValue& iv,const string& name)
{
 string warningt="Runtime Warning: ";
 switch(warningtype)
	   {
	    case Warning_arrayaccesserror:
			 cout<<warningtype<<"invalid array index cause access error @LN"<<sp<<"]"<<endl;
		     break;
	    case Warning_compilernotrealizethismethod:
			 cout<<warningtype<<"compiler not realize "<<name<<" method yet @LN"<<sp<<"]"<<endl;
		     break;
	   }
}
//-------------------------------------------------------------------------------------------------
