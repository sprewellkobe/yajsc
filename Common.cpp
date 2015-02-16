#include "Common.hpp"
#include <algorithm>
#include <cctype>
//------------------------------------------------------------------------

string IntToStr(int count)
{
 string res="";
 char a[42];
 if(a) 
   sprintf(a,"%d",count);
 else
	return res;
 res=a;
 return res;
}
//------------------------------------------------------------------------

bool  ParseURL(const string& url,string& host,string& path)
{
 host="";
 path="";
 if(url.size()<MIN_URL_SIZE || url.substr(0,7)!="http://")//MIN_URL_SIZE==7+4
    return false;
 string pureurl=url.substr(7);

 string::size_type pos;
 pos=pureurl.find('/');
 if(pos==string::npos)
    host=pureurl;
 else
    {
    host=pureurl.substr(0,pos);
    string lefturl=pureurl.substr(pos)+"#";

    int i=0;
    for(;;i++)
       {
        bool needbreak=false;
        switch(lefturl[i])
              {
               case '#':
               case '?':
               case '*':
               case '&':
			   case ';':
                    needbreak=true;
                    break;
               default:
                    break;
              }
        if(needbreak)
           break;
       }//end for(;;i++)
     path=lefturl.substr(0,i);
    }//end else
 return true;
}
//------------------------------------------------------------------------

string TransformString(const string& str)
{
 string newstr="";
 if(!str.empty())
   {
    unsigned int pos=0;
    char prechar='\0';
    while(pos<str.size())
         {
          if(str[pos]=='\\' && prechar=='\\')
             ;
          else
            newstr=newstr+str[pos];
          prechar=str[pos];
          pos++;
         }
   }
 return newstr;
}
//------------------------------------------------------------------------

int MonthToInt(string month)
{
 transform(month.begin(),month.end(),month.begin(),(int (*)(int))tolower);
 if(month=="january")
	return 0;
 else if(month=="february")
	return 1;
 else if(month=="march")
	return 2;
 else if(month=="april")
	return 3;
 else if(month=="may")
	return 4;
 else if(month=="june")
	return 5;
 else if(month=="july")
	return 6;
 else if(month=="august")
	return 7;
 else if(month=="september")
	return 8;
 else if(month=="october")
    return 9;
 else if(month=="november")
	return 10;
 else if(month=="december")
	return 11;
 return -1;
}
//------------------------------------------------------------------------
struct timeval BeginTiming()
{
 struct timeval bt;
 gettimeofday(&bt,NULL);
 return bt;
}
//-------------------------------------------------------------------------------------------------

double EndTiming(const struct timeval& bt)
{
 struct timeval et;
 double timeuse=0;
 gettimeofday(&et,NULL);
 timeuse=et.tv_sec-bt.tv_sec+((double)(et.tv_usec-bt.tv_usec)/1000000);
 return (double)(timeuse);
}
//-------------------------------------------------------------------------------------------------

