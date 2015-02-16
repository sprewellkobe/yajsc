#include "libyajsc.hpp"
//-------------------------------------------------------------------

int yajsc_compile_string(const char* code,const char* addtional_code,char* buf, unsigned int buf_length)
{
 if(code==NULL || buf==NULL)
	return 0;
 string s=code;
 JSIInterface jsi(s,1);
 string as;
 if(addtional_code==NULL)
    as="";
 else
    as=addtional_code;
 if(!as.empty())
	jsi.SetAddtionalCode(as);
 jsi.Prepare();
 if(jsi.Scan())
   {
    string str=jsi.Compile();
	if(str.size()+1<buf_length)
	  {
	   strncpy(buf,str.c_str(),str.size()+1);
	   buf[str.size()]=0;
	  }
	else
	   return 0;
   }
 return 1;
}
//-------------------------------------------------------------------

int yajsc_compile_string_withurl(const char* code,const char* addtional_code,
                                  const char* url,char* buf,unsigned int buf_length)
{
 if(code==NULL || buf==NULL)
    return 0;
 string s=code;
 JSIInterface jsi(s,1);
 string as;
 if(addtional_code==NULL)
    as="";
 else
    as=addtional_code;
 string urlstr;
 if(url==NULL)
    urlstr="";
 else
    urlstr=url;
 
 if(!as.empty())
    jsi.SetAddtionalCode(as);
 if(!urlstr.empty())
    jsi.SetURL(urlstr);

 jsi.Prepare();
 if(jsi.Scan())
   {
    string str=jsi.Compile();
    if(str.size()+1<buf_length)
      {
       strncpy(buf,str.c_str(),str.size()+1);
       buf[str.size()]=0;
      }
    else
       return 0;
   }
 return 1;
}
//-------------------------------------------------------------------

int yajsc_compile_string_withlog(const char* code,const char* addtional_code,char* buf, unsigned int buf_length)
{
 if(code==NULL || buf==NULL)
	return 0;
 string s=code;
 JSIInterface jsi(s,1);
 string as;
 if(addtional_code==NULL)
    as="";
 else
    as=addtional_code;
 if(!as.empty())
	jsi.SetAddtionalCode(as);
 jsi.Prepare();
 if(jsi.Scan())
   {
    string str=jsi.Compile(true);
	if(str.size()+1<buf_length)
	  {
	   strncpy(buf,str.c_str(),str.size()+1);
	   buf[str.size()]=0;
	  }
	else
	   return 0;
   }
 return 1;
}
//-------------------------------------------------------------------
