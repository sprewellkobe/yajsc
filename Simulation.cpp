//--------------------------------------------------------------------------------------
//Javascript kernel object implement, by kobe
#include "Simulation.hpp"
#include <algorithm>
#include <cctype>
#include <math.h>
#include <time.h>
#include <boost/regex.hpp>
#include <functional>
using namespace boost;
//---------------------------------------------------------------------------------------

void GetRegexOption(const string& str,int& option,match_flag_type& mt)
{
 option=regbase::normal;
 mt=boost::match_default;
 if(str.find('i')!=string::npos)//includes i
    option|=regbase::icase;
 if(str.find('g')==string::npos)//not includes g
    mt|=boost::format_first_only;
}
//---------------------------------------------------------------------------------------
//辅助函数,分割regex的pattern  /ab*/g
bool SplitPattern(const string& os,string& s1,string& s2)
{
 s2="";
 s1="";
 int start=1;
 if(os[0]=='/')
	start=1;
 else
    start=0;
 int pos=start;
 while(pos<(int)(os.size()))
	  {
	   if(os[pos]=='/' && os[pos-1]!='\\')
		  break;
	   pos++;
	  }
 try
   {
    s1=os.substr(start,pos-start);	
   }
 catch(...)
   {
    s1="";
   }
 try
   {
    s2=os.substr(pos+1);	
    }
 catch(...)
    {
    s2="";
    }
 
 if(s2!="i" && s2!="g" && s2!="gi")
	s2="";
 return true;
}
//---------------------------------------------------------------------------------------

string TSimulation::GetNameByID(const int& number)
{
 map<int,string>::iterator iter=symbol_table.find(number);
 if(iter==symbol_table.end())
	return "";
 return iter->second;
}
//---------------------------------------------------------------------------------------

bool TSimulation::SimulateString(IntValue& iv,bool func,TItem* object,const string& ostr,const string& method,
                                 vector<IntValue>* al,const int& sp,const int& env,int strindex)
{
 iv.value=0;
 iv.type=TYPE::NUMBER;
 string str;
 if(object==NULL)
	str=ostr;
 else
	{
	str=GetNameByID(object->value);
	iv.value=object->value;
	}
 
 if(!func)//属性
   {
	if(method=="length")
	  iv.value=str.size()-1;//因为第一个字符为*,所以减1
	//else if(method=="prototype")
	  //;
   }
 else//方法
   {
	iv.type=TYPE::STR;
	iv.value=strindex;
	//-----------------------------------------------------------------------------------
	if(method=="toLowerCase")
	  {
       transform(str.begin(),str.end(),str.begin(),(int (*)(int))tolower); 
	   int ti=current_string_index++;
	   symbol_table[ti]=str;
	   iv.value=ti;
	  }
	else if(method=="toString")
	  {
	   ;       
	  }
	else if(method=="toUpperCase")
	  {
	   transform(str.begin(),str.end(),str.begin(),(int (*)(int))toupper); 
	   int ti=current_string_index++;
	   symbol_table[ti]=str;
	   iv.value=ti;
	  }
	//-----------------------------------------------------------------------------------
	else if(method=="substring" || method=="substr" || method=="slice")
	  {
	   if(al->empty())
		 {
		  TError::ShowError(Error_argumentsnumbernotmatch,sp-1,iv,method);
		  return false;
		 }
	   IntValue& ag1=(*al)[0];
	   IntValue ag2;
	   if(al->size()>1)
	      ag2=(*al)[1];
       else
		  {
		   ag2.value=str.size();
		   ag2.type=TYPE::NUMBER;
	      }
	   int t1=0;
	   int t2=0;
	   
	   if(ag1.type==TYPE::NUMBER)
		  t1=ag1.value;
	   else if(ag1.type==TYPE::ID)
		 {
		  string c1name=GetNameByID(ag1.value);
		  TItem* item2;
		  if( (runtime_table->GetItemByName(c1name,&item2))==false )
			{
			 TError::ShowError(Error_expectdefinition,sp-1,ag1,"");
			 return false;
			}
          if(item2->type==TYPE::INT)
			 t1=item2->value;
		  else
			{
			 cout<<"Error: type name error"<<endl;
			 return false;
			}
		 }
	   else if(ag1.type==TYPE::COBJ || ag1.type==TYPE::LOBJ)
		 {
		  t1=0;
		 }
	   else
		 {
		  cout<<"Error: type name error"<<endl;
		  return false;
		 }
	   
	   if(ag2.type==TYPE::NUMBER)
		  t2=ag2.value;
	   else if(ag2.type==TYPE::ID)
		 {
		  string c2name=GetNameByID(ag2.value);
		  TItem* item2;
		  if(runtime_table->GetItemByName(c2name,&item2)==false)
			{
			 TError::ShowError(Error_expectdefinition,sp-1,ag2,"");
			 return false;
			}
		  if(item2->type==TYPE::INT)
			 t2=item2->value;
		  else
			{
			 cout<<"Error: type name error"<<endl;
			 return false;
			}
		 }
	   else if(ag2.type==TYPE::COBJ || ag2.type==TYPE::LOBJ)
		 {
		  t2=t1;
		 }
	   else
		 {
		  cout<<"Error: type name error"<<endl;
		  return false;
		 }
	   str=str.substr(1);
	   //cout<<str<<" "<<t1<<" "<<t2<<endl;
	   if(method=="substring")
             {
              try
                 {
                 str=str.substr(t1,t2-t1);
                 }
              catch(...)
                 {
                 str="";
                 }
             }
	   else if(method=="substr") //substr
             {
              try
                 {
                 str=str.substr(t1,t2);
                 }
              catch(...)
                 {
                  str="";
                 } 
             }
	   else if(method=="slice")
		 {
		  if(t2>0)
            {
             try
                {   
			     str=str.substr(t1,t2-t1);
                }
             catch(...)
                {
                 str="";
                }
            }
		  else
            {
             try
                {
		         str=str.substr(t1,str.size()+t2-t1);
                }
             catch(...)
                {
                 str="";
                }
            }
		 }
	   str="*"+str;
       int ti=current_string_index++;
	   symbol_table[ti]=str;
	   iv.value=ti;
	  }//end if substring str
	//-----------------------------------------------------------------------------------
	else if(method=="split")
	  {
	   str=str.substr(1);
	   vector<string> vec;
	   string reg=" ";
	   if(al->empty()==false)//有参数
		 {
		  IntValue& ag1=(*al)[0];
		  if(ag1.type==TYPE::ID)
			{
			 string c2name=GetNameByID(ag1.value);
			 TItem* item2;
			 if(runtime_table->GetItemByName(c2name,&item2)==false)
			   {
			    TError::ShowError(Error_expectdefinition,sp-1,ag1,"");
			    return false;
			   }
			 if(item2->type==TYPE::STRING)
				reg=GetNameByID(item2->value).substr(1);
			 else
				{
				cout<<"Error: type name error"<<endl;
			    return false;
				}
			}
		  else if(ag1.type==TYPE::STR)
			{
			 reg=GetNameByID(ag1.value).substr(1);
			}
	      else if(ag1.type==TYPE::COBJ || ag1.type==TYPE::LOBJ)
			{
			 reg=" ";
			}
		  else
			{
			 cout<<"Error: type name error"<<endl;
			 return false;
			}
		
		  if(al->size()>1)//还有第二个参数
			{
		     int limit=100;
			 IntValue& ag2=(*al)[1];
			 if(ag2.type==TYPE::ID)
			   {
				string c2name=GetNameByID(ag2.value);
				TItem* item2;
				if(runtime_table->GetItemByName(c2name,&item2)==false)
				  {
				   TError::ShowError(Error_expectdefinition,sp-1,ag2,"");
			       return false;
				  }
				if(item2->type==TYPE::INT)
				   limit=item2->value;
				else
				  {
				   cout<<"Error: type name error"<<endl;
			       return false;
				  }
			   }
			 else if(ag2.type==TYPE::NUMBER)
			   limit=ag2.value;
               
             char* tempstr=strdupa(str.c_str());
             char* tempflag;
			 char* token = strtok_r(tempstr, reg.c_str(),&tempflag);
             while(token != NULL )
                  {
                   vec.push_back(token);
				   token = strtok_r( NULL, reg.c_str(),&tempflag);
                  }
			 if(int(vec.size())>limit)
			    vec.erase(vec.begin()+limit,vec.end());
             if(vec.empty())
                vec.push_back("");
			}
		  else//没有第二个参数
			{
             char* tempstr=strdupa(str.c_str());
             char* tempp;
             char* token = strtok_r(tempstr,reg.c_str(),&tempp);
             while(token != NULL )
                  {
                   vec.push_back(token);
				   token = strtok_r( NULL,reg.c_str(),&tempp);
                  }
		     if(vec.empty())
                vec.push_back("");
			}//
	     }//有参
	   else
		 {
		  reg=" ";
          char* tempp;
          char* tempstr=strdupa(str.c_str());
		  char* token = strtok_r(tempstr,reg.c_str(),&tempp);
          while(token != NULL )
               {
                vec.push_back(token);
				token = strtok_r(NULL,reg.c_str(),&tempp);
               }
          if(vec.empty())
             vec.push_back("");
		 }
       
	   int index;
       vector<IntValue>& cvec=runtime_heap->GetNew(index);
	   //cout<<"vec size:"<<vec.size()<<endl;
	   for(int i=0;i<int(vec.size());i++)
		  {
		    IntValue ivt(0);
			ivt.type=TYPE::STR;
			int ti=current_string_index++;
	        symbol_table[ti]="*"+vec[i];
	        ivt.value=ti;
		    cvec.push_back(ivt);
		  }
	   iv.type=TYPE::HEAP;
	   iv.value=index;
	  }//end if split
	//-----------------------------------------------------------------------------------
	else if(method=="search")//待实现,正则匹配
	  {
	   if(al->empty())
		 {
		  TError::ShowError(Error_argumentsnumbernotmatch,sp-1,iv,method);
		  return false;
		 }
	   iv.type=TYPE::NUMBER;
	   iv.value=-1;

	   str=str.substr(1);//原串
       IntValue& ag1=(*al)[0];
	   string regs="*";
	   if(ag1.type==TYPE::STR)
		  regs=GetNameByID(ag1.value);//参数是目标字符串
	   else if(ag1.type==TYPE::ID)
		  {
		  string name=GetNameByID(ag1.value);
		  TItem* item;
		  if(runtime_table->GetItemByName(name,&item))
			{
			 if(item->type==TYPE::STRING)
			    regs=GetNameByID(item->value);
			}
		  }//end TYPE::ID
	   regs=regs.substr(1);
	   string s1,s2;
	   if(SplitPattern(regs,s1,s2)==false)
		  s1="\\s";
       int option=regbase::normal;
       match_flag_type mt=match_default;
       GetRegexOption(s2,option,mt);
	   bool rv=0;
	   boost::match_results<std::string::const_iterator> what;
	   string::const_iterator start, end;
       start = str.begin();
       end = str.end();
	   if(!s1.empty())
		 {
          try
            {
	         regex expression(s1,option);

             rv=regex_search(start,end,what,expression,boost::match_default);
            }
          catch(...)
            {
             TError::ShowError(Error_regexerror,sp-1,iv,s1);
             cout<<"Regex Failed"<<endl;
            }
		 }
	   if(rv)
		  iv.value=what[0].first-start;
	   else
		  iv.value=-1;
	  }
	//-----------------------------------------------------------------------------------
	else if(method=="match")
	  {
	   if(al->empty())
		 {
		  TError::ShowError(Error_argumentsnumbernotmatch,sp-1,iv,method);
		  return false;
		 }
	   str=str.substr(1);//原串
       IntValue& ag1=(*al)[0];
	   string regs="*";
	   if(ag1.type==TYPE::STR)
		  regs=GetNameByID(ag1.value);//参数是匹配原则
	   else if(ag1.type==TYPE::ID)
		  {
		  string name=GetNameByID(ag1.value);
		  TItem* item;
		  if(runtime_table->GetItemByName(name,&item))
			{
			 if(item->type==TYPE::STRING)
			    regs=GetNameByID(item->value);
			}
		  }//end TYPE::ID
	   regs=regs.substr(1);
	   string s1,s2;
	   if(SplitPattern(regs,s1,s2)==false)
		  s1="\\s";
       if(s1.empty())
		  s1="\\s";
       int option=regbase::normal;
       match_flag_type mt=match_default;
       GetRegexOption(s2,option,mt);
       vector<string> vec;
	   string::const_iterator start, end;
       start = str.begin();
       end = str.end();
       match_results<string::const_iterator> what;
       //match_flag_type flags = match_default;
       try
         {
	      regex ex(s1,option);
          while(boost::regex_search(start, end, what, ex,mt))
               {
                vec.push_back(what.str());
                start = what[0].second;
                if(mt&boost::format_first_only)
                   break;
               }//end while
         }
       catch(...)
         {
          TError::ShowError(Error_regexerror,sp-1,iv,s1);
          cout<<"Regex Failed"<<endl;
         }
       if(vec.empty())
          vec.push_back("");
       int index;
       vector<IntValue>& cvec=runtime_heap->GetNew(index);
	   for(int i=0;i<int(vec.size());i++)
		  {
		   IntValue ivt(0);
		   ivt.type=TYPE::STR;
		   int ti=current_string_index++;
	       symbol_table[ti]="*"+vec[i];
	       ivt.value=ti;
		   cvec.push_back(ivt);
		  }
	   iv.type=TYPE::HEAP;
	   iv.value=index;
	  }
	//-----------------------------------------------------------------------------------
	else if(method=="replace")
	  {
	   if(al->size()!=2)
		 {
		  TError::ShowError(Error_argumentsnumbernotmatch,sp-1,iv,"replace");
		  return false;
		 }
	   str=str.substr(1);//原串
       IntValue& ag1=(*al)[0];
	   string regs="*";
	   if(ag1.type==TYPE::STR)
		  regs=GetNameByID(ag1.value);//参数是匹配原则
	   else if(ag1.type==TYPE::ID)
		  {
		  string name=GetNameByID(ag1.value);
		  TItem* item;
		  if(runtime_table->GetItemByName(name,&item))
			{
			 if(item->type==TYPE::STRING)
			    regs=GetNameByID(item->value);
			}
		  }//end TYPE::ID
	   regs=regs.substr(1);
	   string s1,s2;
	   if(SplitPattern(regs,s1,s2)==false)
		  s1="\\s";
       int option=regbase::normal;
       match_flag_type mt=match_default|format_first_only;
       GetRegexOption(s2,option,mt);

       IntValue& ag2=(*al)[1];
	   string ts="*";
	   if(ag2.type==TYPE::STR)
		  ts=GetNameByID(ag2.value);//参数是目标字符串
	   else if(ag2.type==TYPE::ID)
		  {
		  string name=GetNameByID(ag2.value);
		  TItem* item;
		  if(runtime_table->GetItemByName(name,&item))
			{
			 if(item->type==TYPE::STRING)
			    ts=GetNameByID(item->value);
			}
		  }//end TYPE::ID
	   ts=ts.substr(1);
       string newstr=str;
       try
          {
	      if(!s1.empty())
		    {
	         regex expression(s1,option);
	         newstr=regex_replace(str,expression,ts,mt);
		    }
          }
       catch(...)
          {
           TError::ShowError(Error_regexerror,sp-1,iv,s1);
           cout<<"Regex Failed"<<endl;
          }
	   iv.type=TYPE::STR;
	   int ti=current_string_index++;
	   symbol_table[ti]="*"+newstr;
	   iv.value=ti;
	  }
	//-----------------------------------------------------------------------------------
	else if(method=="lastIndexOf" || method=="indexOf")
	  {
	   if(al->empty())
		 {
		  TError::ShowError(Error_argumentsnumbernotmatch,sp-1,iv,method);
		  return false;
		 }
	   IntValue& ag1=(*al)[0];
       string ts;
       if(ag1.type==TYPE::STR)
		  ts=GetNameByID(ag1.value);
	   else if(ag1.type==TYPE::ID)
		 {
		  TItem* item;
		  if(runtime_table->GetItemByName(GetNameByID(ag1.value),&item)==false)
			{
			 TError::ShowError(Error_expectdefinition,sp-1,ag1,"");
		     return false;
			}
		  if(item->type==TYPE::STRING)
			 ts=GetNameByID(item->value);
		  else
			{
			 cout<<"Error: type name error"<<endl;
		     return false;
			}
		 }
	   else if(ag1.type==TYPE::COBJ || ag1.type==TYPE::LOBJ)
		 {
		  ts="* ";
		 }
	   else
		 {
		  cout<<"Error: type name error"<<endl;
		  return false;
		 }
	   
	   ts=ts.substr(1);//去*
	   iv.type=TYPE::NUMBER;
	   if(al->size()>1)//有第二个参数
		 {
		  IntValue& ag2=(*al)[1];
		  int pos=0;
          if(ag2.type==TYPE::NUMBER)
			{
			 pos=ag2.value;
			}
	      else if(ag2.type==TYPE::ID)
			{
			 TItem* item;
			 if(runtime_table->GetItemByName(GetNameByID(ag2.value),&item)==false)
			   {
				TError::ShowError(Error_expectdefinition,sp-1,ag2,"");
		        return false;
			   }
			 if(item->type==TYPE::INT)
			   pos=item->value;
             else
			   {
			   cout<<"Error: type name error"<<endl;
		       return false;
			   }
			}//end TYPE::ID
		  else if(ag2.type==TYPE::COBJ || ag2.type==TYPE::LOBJ)
			{
			 pos=0;
			}
          else
			{
			 cout<<"Error: type name error"<<endl;
		     return false;
			}
		  
		  if(method=="lastIndexOf")
			{
			 int r=str.find_last_of(ts,pos);
			 if(r==int(string::npos))
				iv.value=-1;
			 else
				iv.value=r;
			}
		  else//index of
			{
			 int r=str.find(ts,pos);
			 if(r==int(string::npos))
			    iv.value=-1;
			 else
				iv.value=r;
			}
		 }
	   else//没有第二个参数
		 {
		  if(method=="lastIndexOf")
			{
			 int r=str.find_last_of(ts);
			 if(r==int(string::npos))//没找到
			    iv.value=-1;
			 else
				iv.value=r;
			}
		  else// indexOf
			{
			 cout<<"find:"<<ts<<endl;
			 int r=str.find(ts);
			 if(r==int(string::npos))//没找到
			    iv.value=-1;
			 else
				iv.value=r;
			}
		 }//end else 没第二个参数
	  }
	//-----------------------------------------------------------------------------------
	else if(method=="concat")//连接两个字符串
	  {
	   if(al->empty())
		 {
		  TError::ShowError(Error_argumentsnumbernotmatch,sp-1,iv,method);
		  return false;
		 }
	   iv.type=TYPE::STR;
	   string ts;
	   IntValue& ag1=(*al)[0];
	   if(ag1.type==TYPE::STR)
		  ts=GetNameByID(ag1.value);
	   else if(ag1.type==TYPE::ID)
		 {
		  TItem* item;
		  if(runtime_table->GetItemByName(GetNameByID(ag1.value),&item)==false)
			{
			 TError::ShowError(Error_expectdefinition,sp-1,ag1,"");
		     return false;
			}
		  if(item->type==TYPE::STRING)
			 ts=GetNameByID(item->value);
		  else
			{
			 cout<<"Error: type name error"<<endl;
		     return false;
			}
		 }//end if ID
	   else if(ag1.type==TYPE::COBJ || ag1.type==TYPE::LOBJ)
		 {
		  ts="*";
		 }
	   else
		 {
		  cout<<"Error: type name error"<<endl;
		  return false;
		 }
	   ts=ts.substr(1);
	   str=str+ts;
       int ti=current_string_index++;
	   symbol_table[ti]=str;
	   iv.value=ti;
	  }//end concat
	//-----------------------------------------------------------------------------------
	else if(method=="charAt")
	  {
	   if(al->empty())
		 {
		  TError::ShowError(Error_argumentsnumbernotmatch,sp-1,iv,method);
		  return false;
		 }
	   iv.type=TYPE::STR;
	   int pos=0;
	   IntValue& ag1=(*al)[0];
	   if(ag1.type==TYPE::NUMBER)
		  pos=ag1.value;
	   else if(ag1.type==TYPE::ID)
		 {
		  TItem* item;
		  if(runtime_table->GetItemByName(GetNameByID(ag1.value),&item)==false)
			{
			 TError::ShowError(Error_expectdefinition,sp-1,ag1,"");
		     return false;
			}
		  if(item->type==TYPE::INT)
			{
			 pos=item->value;
			}
	      else
			{
			 cout<<"Error: type name error"<<endl;
		     return false;
			}
		 }//end TYPE::ID
	   else if(ag1.type==TYPE::COBJ || ag1.type==TYPE::LOBJ)
		 {
		  pos=0;
		 }
	   else
		 {
		  cout<<"Error: type name error"<<endl;
		  return false;
		 }
	   str=str.substr(1);
	   try
	     {
	      str=str.substr(pos,1);
	     }
	   catch(...)
	     {
	      str="";
	     }
       str="*"+str;
       int ti=current_string_index++;
	   symbol_table[ti]=str;
	   iv.value=ti;
	  }//end charAt
	//-----------------------------------------------------------------------------------
	//可能为RegExp的操作,因为视RegExp为string
	else if(method=="compile")
	  {
	   iv.type=TYPE::STR;
       string strt="/\\s/";
	   if(al->size()>=1)//单参
		 {
		  IntValue& ag1=(*al)[0];
		  if(ag1.type==TYPE::STR)
		    {
			 string name=GetNameByID(ag1.value).substr(1);
			 if(name.empty()==false)
			   {
			    strt="/"+name+"/";
			   }
		    }
		  else if(ag1.type==TYPE::ID)
		    {
			string name=GetNameByID(ag1.value);
			TItem* item;
			if(runtime_table->GetItemByName(name,&item))
			  {
			   if(item->type==TYPE::STRING)
				 {
				  strt="/"+GetNameByID(item->value).substr(1)+"/";
				 }
			  }
		   }//end TYPE::ID
		}//end 单参
	  if(al->size()>=2)//还有第二个参数
		{
		 IntValue& ag2=(*al)[1];
         string style="";
		 if(ag2.type==TYPE::STR)
		   {
			string name=GetNameByID(ag2.value).substr(1);
			if(name=="i"||name=="g"||name=="gi")
			   style=name;
		   }
		 else if(ag2.type==TYPE::ID)
		   {
			string name=GetNameByID(ag2.value);
			TItem* item;
			if(runtime_table->GetItemByName(name,&item))
			  {
               if(item->type==TYPE::STRING)
				 {
				  name=GetNameByID(item->value).substr(1);
				  if(name=="i" || name=="g" ||name=="gi")
					 style=name;
				 }
			  }
		   }//end TYPE::ID
		 strt=strt+style;
		}//end 第二个参数
	  strt="*"+strt;
	  int ti=current_string_index++;
	  symbol_table[ti]=strt;
	  iv.value=ti;
	  }//end compile
	//-----------------------------------------------------------------------------------
	else if(method=="exec")
	  {
       //cout<<"enter"<<endl;
	   if(al->empty())
		 {
		  TError::ShowError(Error_argumentsnumbernotmatch,sp-1,iv,method);
		  return false;
		 }
	   str=str.substr(1);//regex匹配原则
       cout<<str<<endl;
       IntValue& ag1=(*al)[0];
	   string target="*";
	   if(ag1.type==TYPE::STR)
		  target=GetNameByID(ag1.value);//参数是目标字符串
	   else if(ag1.type==TYPE::ID)
		  {
		  string name=GetNameByID(ag1.value);
		  TItem* item;
		  if(runtime_table->GetItemByName(name,&item))
			{
			 if(item->type==TYPE::STRING)
			    target=GetNameByID(item->value);
             else
                {
                 ;
                }
			}
		  }//end TYPE::ID
       else if(ag1.type==TYPE::COBJ || ag1.type==TYPE::LOBJ)
          ;
           
	   target=target.substr(1);
	   string s1="";
       string s2="";
       //match_flag_type flags = match_default;
	   if(SplitPattern(str,s1,s2)==false)
		  s1="\\s";
       if(s1.empty())
		  s1="\\s";
          
       int option=regbase::normal;
       match_flag_type mt=match_default;
       GetRegexOption(s2,option,mt);

	   vector<string> vec;
	   string::const_iterator start, end;
       start = target.begin();
       end = target.end();
       match_results<string::const_iterator> what;
       //match_flag_type flags = match_default;
       try
         {
	      regex ex(s1,option);
          while(boost::regex_search(start, end, what, ex,mt))
               {
                vec.push_back(what.str());
                start = what[0].second;
                if(mt&format_first_only)
                   break;
               }//end while
         }
       catch(...)
         {
          TError::ShowError(Error_regexerror,sp-1,iv,s1);
          cout<<"Regex Failed"<<endl;
         }
       if(vec.empty())
          vec.push_back("");
       int index;
       vector<IntValue>& cvec=runtime_heap->GetNew(index);
	   //cout<<"vec size:"<<vec.size()<<endl;
	   for(int i=0;i<int(vec.size());i++)
		  {
		   IntValue ivt(0);
		   ivt.type=TYPE::STR;
		   int ti=current_string_index++;
	       symbol_table[ti]="*"+vec[i];
	       ivt.value=ti;
		   cvec.push_back(ivt);
		  }
	   iv.type=TYPE::HEAP;
	   iv.value=index;
       //cout<<"leave"<<endl;
	  }
	//-----------------------------------------------------------------------------------
	else if(method=="test")//regex_match
	  {
	   if(al->empty())
		 {
		  TError::ShowError(Error_argumentsnumbernotmatch,sp-1,iv,method);
		  return false;
		 }
	   iv.type=TYPE::NUMBER;
	   str=str.substr(1);//regex匹配原则
       IntValue& ag1=(*al)[0];
	   string target="*";
	   if(ag1.type==TYPE::STR)
		  target=GetNameByID(ag1.value);//参数是目标字符串
	   else if(ag1.type==TYPE::ID)
		  {
		  string name=GetNameByID(ag1.value);
		  TItem* item;
		  if(runtime_table->GetItemByName(name,&item))
			{
			 if(item->type==TYPE::STRING)
			    target=GetNameByID(item->value);
			}
		  }//end TYPE::ID
	   string s1,s2;
	   if(SplitPattern(str,s1,s2)==false)
		  s1="\\s";
       int option=regbase::normal;
       match_flag_type mt=match_default;
       GetRegexOption(s2,option,mt);
	   bool res=0;
	   if(!s1.empty())
		 {
          try
            {
	         regex expression(s1,option);
	         res=regex_match(target.substr(1),expression);
            }
          catch(...)
            {
             TError::ShowError(Error_regexerror,sp-1,iv,s1);
             cout<<"Regex Fail"<<endl;
            }
		 }
	   if(res)
		  iv.value=1;
	   else
		  iv.value=0;
	  }//end test
	//-----------------------------------------------------------------------------------
	else//其余操作一律返回原字符串,包括了toString()
	  {
	  }
   }
 return true;
}
//---------------------------------------------------------------------------------------

bool TSimulation::SimulateNumber(IntValue& iv,bool func,TItem* object,const string& method,
                                 vector<IntValue>* al,const int& sp,const int& env)
{
 iv.type=TYPE::NUMBER;
 iv.value=0;
 if(!func)//属性
   {
	if(method=="length")
	  iv.value=GetNameByID(object->value).size()-1;//因为第一个字符为*,所以减1
	else if(method=="prototype")
	  ;
   }
 return true;
}
//---------------------------------------------------------------------------------------

bool TSimulation::SimulateStatic(IntValue& iv,bool func,const string& classname,IntValue* withobject,const string& method,
	                             vector<IntValue>* al,const int& sp,const int& env)
{
 iv.type=TYPE::NUMBER;
 iv.value=0;
 
 if(withobject)
   {
    if(func==false)
      {
       if(withobject->type==TYPE::LOBJ)
         {
          if(method=="host"||method=="hostname")
            {
             iv.value=host_string_index;
             iv.type=TYPE::STR;
             return true;
            }
          else if(method=="pathname")
            {
             iv.value=path_string_index;
             iv.type=TYPE::STR;
             return true;
            }
          else
            {
             iv.value=0;
             iv.type=TYPE::LOBJ;
            }
          return true;
         }
       if(withobject->type==TYPE::COBJ)
         {
          iv.type=TYPE::COBJ;
          iv.value=0;
          return true;
         }
      }//end if func==false
    else
      {
       if(withobject->type==TYPE::LOBJ)
         {
          if(method=="replace")
            {
             string url=UNKNOWNSITE;
             if(!al->empty())
               {
                IntValue& ag1=(*al)[0];
                if(ag1.type==TYPE::STR)
                   url=GetNameByID(ag1.value);
                else if(ag1.type==TYPE::ID)
	              {
	               string name=GetNameByID(ag1.value);
	               TItem* item;
	               if(runtime_table->GetItemByName(name,&item)==false)
	                 {
		              TError::ShowError(Error_expectdefinition,sp-1,ag1,name);
		              return false;
	                 }
	               if(item->type==TYPE::STRING)
		              url=GetNameByID(item->value);
	              }
               }
             cout<<"JUMP!!! "<<url.substr(1)<<endl;
	         AddOneResult("JUMP",url.substr(1));
             iv.type=TYPE::LOBJ;
             iv.value=0; 
             return true;
            }//end if method=="replace"
          iv.type=TYPE::LOBJ;
          iv.value=0;
          return true;
         }
       else if(withobject->type==TYPE::COBJ)
         {
          iv.type=TYPE::COBJ;
          iv.value=0;
          return true;
         }
      }//end else
   }
 //---------------------------------------------------------------------------------
 if(func==false)//属性
   {
	if(classname=="document" && method=="URL")
	  {
	   iv.value=url_string_index;
       iv.type=TYPE::STR;
       return true;
	  }
	else if(classname=="Number")
	  {
	   if(method=="MAX_VALUE"||method=="POSITIVE_INFINITY")
		  iv.value=INT_MAX;
	   else if(method=="MIN_VALUE"||method=="NEGATIVE_INFINITY")
		  iv.value=INT_MIN;
	   else if(method=="prototype")
		  ;
	   else if(method=="NaN")
		 {
		  iv.type=TYPE::STR;
		  iv.value=nanchar;
		 }
	   else
		 {
		  TError::ShowError(Error_classnothavethisattribute,sp-1,iv,method);
		  return false;
		 }
	  }//end Number
	//---------------------------------------------------------------
	else if(classname=="Math")
	  {
	   if(method=="E")
		  iv.value=3;//e的取整
	   else if(method=="LN10")
          iv.value=2;
	   else if(method=="LN2")
		  iv.value=1;
	   else if(method=="LOG10E")
		  iv.value=1;
	   else if(method=="LOG2E")
		  iv.value=1;
	   else if(method=="PI")
		  {
		  iv.value=3;
		  }
	   else if(method=="SQRT1_2")
		  iv.value=1;
	   else if(method=="SQRT2")
		  iv.value=1;
	   else
		 {
		  TError::ShowError(Error_classnothavethisattribute,sp-1,iv,method);
		  return false;
		 }
	  }//end Math
    else if(classname=="location")
      {
       if(method=="host" || method=="hostname")
         {
          iv.value=host_string_index;
          iv.type=TYPE::STR;
         }
       else if(method=="pathname")
         {
          iv.value=path_string_index;
          iv.type=TYPE::STR;
         }
       else
         {
          iv.value=0;
          iv.type=TYPE::LOBJ;
         }
      }
	else//未知类型
	  {
	   iv.value=0;
	   if(method=="location")
	      iv.type=TYPE::LOBJ;
	   else
		  iv.type=TYPE::COBJ;
	  }
   }//end 属性
 //---------------------------------------------------------------------------------
 else//函数
   {
	if(classname=="Math")
	  {
	   //------------------------------------------------------------
	   if(method=="abs")
		 {
		  if(al->size()<1)
			{
			 TError::ShowError(Error_argumentsnumbernotmatch,sp-1,iv,"abs");
			 return false;
			}
		  IntValue& ag1=(*al)[0];
		  if(ag1.type==TYPE::NUMBER)
			{
			 iv.value=abs(ag1.value);
			}
		  else if(ag1.type==TYPE::ID)
			{
			 TItem* item;
			 if(runtime_table->GetItemByName(GetNameByID(ag1.value),&item)==false)
			   {
				TError::ShowError(Error_expectdefinition,sp-1,ag1,GetNameByID(ag1.value));
				return false;
			   }
			 if(item->type==TYPE::INT)
			   {
				iv.value=abs(item->value);
			   }
			 else
			   {
				TError::ShowError(Error_argumentstypenotmatch,sp-1,ag1,"abs");
			    return false;
			   }
			}//end TYPE::ID
		  else if(ag1.type==TYPE::COBJ || ag1.type==TYPE::LOBJ)
			{
			 iv.value=1;
			}
		  else
			{
			 TError::ShowError(Error_argumentstypenotmatch,sp-1,ag1,"abs");
			 return false;
			}
		 }//end method abs
	  //-------------------------------------------------------------
	  else if(method=="max" || method=="min" | method=="pow")
		 {
		  if(al->size()!=2)
			{
			 TError::ShowError(Error_argumentsnumbernotmatch,sp-1,iv,method);
			 return false;
			}
		  float a,b;

		  IntValue& ag1=(*al)[0];
		  if(ag1.type==TYPE::NUMBER)
			{
			 a=ag1.value;
			}
		  else if(ag1.type==TYPE::ID)
			{
			 TItem* item;
			 if(runtime_table->GetItemByName(GetNameByID(ag1.value),&item)==false)
			   {
                TError::ShowError(Error_expectdefinition,sp-1,ag1,GetNameByID(ag1.value));
				return false;
			   }
			 if(item->type==TYPE::INT)
				a=item->value;
			 else
			   {
				TError::ShowError(Error_argumentstypenotmatch,sp-1,ag1,method);
			    return false;
			   }
			}//end TYPE::ID
		  else if(ag1.type==TYPE::COBJ || ag1.type==TYPE::LOBJ)
			{
			 a=0;
			}
		  else
			{
			 TError::ShowError(Error_argumentstypenotmatch,sp-1,ag1,method);
			 return false;
			}
		  
		  IntValue& ag2=(*al)[1];
		  if(ag2.type==TYPE::NUMBER)
			 b=ag2.value;
		  else if(ag2.type==TYPE::ID)
			{
			 TItem* item;
			 if(runtime_table->GetItemByName(GetNameByID(ag2.value),&item)==false)
			   {
                TError::ShowError(Error_expectdefinition,sp-1,ag2,GetNameByID(ag2.value));
				return false;
			   }
			 if(item->type==TYPE::INT)
				b=item->value;
			 else
			   {
				TError::ShowError(Error_argumentstypenotmatch,sp-1,ag2,method);
			    return false;
			   }
			}//end TYPE::ID
		  else if(ag2.type==TYPE::COBJ || ag2.type==TYPE::LOBJ)
			{
			 b=1;
			}
		  else
			{
			 TError::ShowError(Error_argumentstypenotmatch,sp-1,ag2,method);
			 return false;
			}
		  
		  if(method=="max")
			 iv.value=(int)(a>b?a:b);
		  else if(method=="min")
			 iv.value=(int)(a>b?b:a);
		  else if(method=="pow")
			 iv.value=(int)(pow(a,b));
		 }//end method max min
	  //-------------------------------------------------------------
	  else if(method=="random")
		 {
		  iv.value=1;
		 }
	  else if(method=="cos" || method=="sin" || method=="tan")
		 {
		  iv.value=1;
		 }
	  else if(method=="acos" || method=="asin" || method=="atan" || method=="atan2")
		 {
		  iv.value=1;
		 }
	  //-------------------------------------------------------------
	  else if(method=="exp" || method=="sqrt" || method=="log" || method=="round")
		 {
          if(al->size()!=1)
			{
			 TError::ShowError(Error_argumentsnumbernotmatch,sp-1,iv,method);
			 return false;
			}
		  float a;

		  IntValue& ag1=(*al)[0];
		  if(ag1.type==TYPE::NUMBER)
			{
			 a=ag1.value;
			}
		  else if(ag1.type==TYPE::ID)
			{
			 TItem* item;
			 if(runtime_table->GetItemByName(GetNameByID(ag1.value),&item)==false)
			   {
                TError::ShowError(Error_expectdefinition,sp-1,ag1,GetNameByID(ag1.value));
				return false;
			   }
			 if(item->type==TYPE::INT)
				a=item->value;
			 else
			   {
				TError::ShowError(Error_argumentstypenotmatch,sp-1,ag1,method);
			    return false;
			   }
			}//end if TYPE::ID
		  else if(ag1.type==TYPE::COBJ || ag1.type==TYPE::LOBJ)
			{
			 a=0;
			}
		  else
			{
			 TError::ShowError(Error_argumentstypenotmatch,sp-1,ag1,method);
			 return false;
			}
		  
		  if(method=="exp")
		     iv.value=(int)(exp(a));
		  else if(method=="sqrt")
			 iv.value=(int)(sqrt(a));
		  else if(method=="log")
			 iv.value=(int)(log(a));
		  else if(method=="round")
			 iv.value=(int)(a);
		 }
	  //-------------------------------------------------------------
	  else if(method=="floor" || method=="ceil")
		 {
		  int a=0;
		  if(al->size()<1)
			{
			 TError::ShowError(Error_argumentsnumbernotmatch,sp-1,iv,method);
			 return false;
			}
		  IntValue& ag1=(*al)[0];
		  if(ag1.type==TYPE::NUMBER)
			{
			 a=ag1.value;
			}
		  else if(ag1.type==TYPE::ID)
			{
			 TItem* item;
			 if(runtime_table->GetItemByName(GetNameByID(ag1.value),&item)==false)
			   {
                TError::ShowError(Error_expectdefinition,sp-1,ag1,GetNameByID(ag1.value));
				return false;
			   }
			 if(item->type==TYPE::INT)
				a=item->value;
			 else if(item->type==TYPE::COBJ || item->type==TYPE::LOBJ)
				a=0;
			 else
			   {
				TError::ShowError(Error_argumentstypenotmatch,sp-1,ag1,method);
			    return false;
			   }
			}//end TYPE::ID
		  else if(ag1.type==TYPE::COBJ || ag1.type==TYPE::LOBJ)
			{
			 a=0;
			}
		  else
			{
			 TError::ShowError(Error_argumentstypenotmatch,sp-1,ag1,method);
			 return false;
			}
		  if(method=="floor")
			 iv.value=a-1;
		  else
			 iv.value=a+1;
		 }//end floor ceil
	  else
		 {
		  TError::ShowError(Error_classnothavethismethod,sp-1,iv,method);
		  return false;
		 }
	  }//end Math
	else if(classname=="window" && 
		     (method=="alert" || method=="confirm" || method=="prompt" ||method=="setTimeout")
		   )
	     {;
		  if(method!="setTimeout")
			{
		     iv.value=1;
		     iv.type=TYPE::NUMBER;
		     cout<<"ALERT!!!"<<endl;
		     AddOneResult("ALERT","");
			}
		  else//setTimeout
			{
			 iv.value=1;
			 iv.type=TYPE::NUMBER;
			 if(al->size()>=2)
			   {
				string code="*";
				IntValue& ag1=(*al)[0];
				if(ag1.type==TYPE::STR)
				   code=GetNameByID(ag1.value);
				else if(ag1.type==TYPE::ID)
				   {
				   string name=GetNameByID(ag1.value);
				   TItem* item;
				   if(runtime_table->GetItemByName(name,&item))
					 {
					  if(item->type==TYPE::STRING)
						 code=GetNameByID(item->value);
					 }
				   }
				code=code.substr(1);
				if(code.find("location")!=string::npos)//存在location操作
				  {
				   cout<<"JUMP!!! "<<UNKNOWNSITE.substr(1)<<endl;
	               AddOneResult("JUMP",UNKNOWNSITE.substr(1));
				  }
			   }
			 else
			   {
				TError::ShowError(Error_argumentsnumbernotmatch,sp-1,iv,method);
				return false;
			   }
			}//end setTimeout
		 }//end window
    else//未知类型
	  {
	   iv.value=0;
	   if(method=="location" || classname=="location")
	      iv.type=TYPE::LOBJ;
	   else if(method=="open")
		  {
		  if( al->size()>=1)
			{
             string url=UNKNOWNSITE;
             IntValue& ag1=(*al)[0];
             if(ag1.type==TYPE::STR)
                url=GetNameByID(ag1.value);
             else if(ag1.type==TYPE::ID)
	            {
	            string name=GetNameByID(ag1.value);
	            TItem* item;
	            if(runtime_table->GetItemByName(name,&item))
			      {
	               if(item->type==TYPE::STRING)
		              url=GetNameByID(item->value);
			      }
	            }//end if ID
             else if(ag1.type==TYPE::NUMBER)
	            {
	            TError::ShowError(Error_argumentstypenotmatch,sp-1,ag1,method);
	            return false;
	            }
             cout<<"POP!!! "<<url.substr(1)<<endl;
		     AddOneResult("POP",url.substr(1));
		     iv.value=1;
		     iv.type=TYPE::NUMBER;
			}
		   else//open无参数
			{
			 string url=UNKNOWNSITE;
			 cout<<"POP!!! "<<url.substr(1)<<endl;
		     AddOneResult("POP",url.substr(1));
		     iv.type=TYPE::COBJ;
			}
		  }//end open
	   else
		  iv.type=TYPE::COBJ;
	  }//end 未知类型
   }
 return true;
}
//---------------------------------------------------------------------------------------

bool TSimulation::SimulateInt(IntValue& iv,bool func,TItem* object,const IntValue& oiv,const string& method,
                                  vector<IntValue>* al,const int& sp,const int& env)
{
 iv.type=TYPE::NUMBER;
 iv.value=0;
 
 if(!func)//属性
   {
    TError::ShowError(Error_integernothaveanyattribute,sp-1,iv,"");
	return false;
   }
 else//方法
   {
	if(object)
	   iv.value=object->value;
	else
	   iv.value=oiv.value;
	iv.type=TYPE::STR;
	if(method=="toString")
	  {
	   string ns="*"+IntToStr(iv.value);
       int ti=current_string_index++;
	   symbol_table[ti]=ns;
	   iv.value=ti;
	  }
	else//改进,当遇到未知的NUMBER
	  {
	   iv.type=TYPE::COBJ;
	  }
   }
 return true;
}
//---------------------------------------------------------------------------------------

bool TSimulation::SimulateArray(IntValue& iv,bool func,TItem* object,const IntValue& oiv,const string& method,
                                vector<IntValue>* al,const int& sp,const int& env,bool& isreturn)
{
 iv.type=TYPE::NUMBER;
 iv.value=0;
 isreturn=true;

 int object_value=0;
 if(object==NULL)
    object_value=oiv.value;
 else
    object_value=object->value;

 
 if(!func)//属性
   {
	if(method=="length")
      {
	   iv.value=runtime_heap->GetItemLength(object_value);
       if(iv.value==-1)
         {
          cout<<"Array get length error"<<endl;
          return false;
         }
      }
	else if(method=="prototype")
	  ;
   }
 //----------------------------------------------------------------------------
 else//方法
   {
	iv.type=TYPE::NUMBER;
	iv.value=0;
	//--------------------------------------------------------------------
	if(method=="toString" || method=="join")
	  {
	   vector<IntValue> cvec;
	   if(runtime_heap->GetArray(object_value,cvec)==false)
		 {
		  TError::ShowError(Error_arrayindexexceed,sp-1,iv,"");
		  return false;
		 }
	   string str;
       string sep=",";
       if(method=="join" && !(al->empty()))
         {
          if((*al)[0].type==TYPE::NUMBER)
             sep=IntToStr((*al)[0].value);
          else if((*al)[0].type==TYPE::STR)
             sep=GetNameByID((*al)[0].value).substr(1);
         }
	   int i=0;
	   for(;i<int(cvec.size()-1);i++)
	      {
	       if(cvec[i].type==TYPE::NUMBER)
	          str+=IntToStr(cvec[i].value)+sep;
	       else if(cvec[i].type==TYPE::STR)
		      str+=GetNameByID(cvec[i].value).substr(1)+sep;
		   else
			  str+=sep;
	      }
	   if(cvec[i].type==TYPE::NUMBER)
		   str+=IntToStr(cvec[i].value);
	   else if(cvec[i].type==TYPE::STR)
		   str+=GetNameByID(cvec[i].value).substr(1);
	   
	   int ti=current_string_index++;
	   symbol_table[ti]="*"+str;
	   iv.type=TYPE::STR; //
	   iv.value=ti;
	  }
	//--------------------------------------------------------------------
	else if(method=="push" || method=="pop")
	  {
	   vector<IntValue> cvec;
	   if(runtime_heap->GetArray(object_value,cvec)==false)
		 {
		  TError::ShowError(Error_arrayindexexceed,sp-1,iv,"");
		  return false;
		 }
	   if(method=="push")
		 {
		  if(al->empty())
		     {
		      TError::ShowError(Error_argumentsnumbernotmatch,sp-1,iv,"");
		      return false;
		     }
		  for(int i=0;i<int(al->size());i++)
			 {
			  cvec.push_back((*al)[i]);
              iv=(*al)[i];
             }
		 }
	   else// pop
		 {
		  iv=cvec.back();
		  cvec.pop_back();
		 }
	  }
	//--------------------------------------------------------------------
	else if(method=="reverse")
	  {
	   bool br=runtime_heap->ReverseArray(object_value);
       if(!br)
         {
          cout<<"reverse Array error"<<endl;
          return false;
         }
	   iv.value=1;
	   iv.type=TYPE::NUMBER;
	  }
	else if(method=="shift")
	  {
	  }
	else if(method=="concat")
	  {
	  }
	else if(method=="slice")
	  {
	  }
	else if(method=="splice")
	  {
	  }
	else if(method=="sort")
	  {
	  }
	else if(method=="unshift")
	  {
	  }
	else
	  {
	   TError::ShowError(Error_classnothavethismethod,sp-1,iv,method);
	   return false;
	  }
    //--------------------------------------------------------------------
   }
 return true;
}
//---------------------------------------------------------------------------------------

bool TSimulation::SimulateDate(IntValue& iv,bool func,TItem* object,const IntValue& oiv,const string& method,
	              vector<IntValue>* al,const int& sp,const int& env)
{
 iv.type=TYPE::NUMBER;
 iv.value=0;
 int ov;
 if(object==NULL)
	ov=oiv.value;
 else
	ov=object->value;

 if(!func)//属性
   {
	if(method=="prototype")
	  ;
   }
 //----------------------------------------------------------------------------
 else//方法
   {
	iv.type=TYPE::NUMBER;
	iv.value=0;
	vector<IntValue> cvec;
	if(runtime_heap->GetArray(ov,cvec)==false)
	  {
	   TError::ShowError(Error_arrayindexexceed,sp-1,iv,"");
	   return false;
      }
	//-------------------------------------------------------------------------
	if(method=="getDate")
	  {
       if(cvec.size()>=3)
	      iv.value=cvec[2].value;
      }
	else if(method=="getDay")
	  {
       if(cvec.size()>=2)
	      iv.value=1;
	  }
	else if(method=="getHours")
	  {
       if(cvec.size()>=4)
	      iv.value=cvec[3].value;
	  }
	else if(method=="getMinutes")
	  {
       if(cvec.size()>=5)
	      iv.value=cvec[4].value;
	  }
	else if(method=="getMonth")
	  {
       if(cvec.size()>=2)
	      iv.value=cvec[1].value;
	  }
	else if(method=="getSeconds")
	  {
       if(cvec.size()>=6)
	      iv.value=cvec[5].value;
	  }
	else if(method=="getTime")
	  {
	   tm tms;
       memset(&tms,0,sizeof(tm));
       if(cvec.size()>=6)
         {
	      tms.tm_year=cvec[0].value-1900;
	      tms.tm_mon=cvec[1].value-1;
	      tms.tm_mday=cvec[2].value;
	      tms.tm_hour=cvec[3].value;
	      tms.tm_min=cvec[4].value;
	      tms.tm_sec=cvec[5].value;
         }
       else
         {
          tms.tm_year=2006-1900;
          tms.tm_mon=3;
          tms.tm_mday=1;
          tms.tm_hour=1;
          tms.tm_min=1;
          tms.tm_sec=1;
         }
	   iv.value=mktime(&tms);
	  }
	else if(method=="getTimezoneOffset")
	  {
	   iv.value=0;
	  }
	else if(method=="getYear" || method=="getFullYear")
	  {
       if(cvec.size()>=1)
	      iv.value=cvec[0].value;
	  }
	else if(method=="parse")
	  {
	   iv.value=0;
	  }
	else if(method=="setDate" || method=="setHours"  || method=="setMinutes" ||
		    method=="setMonth"|| method=="setSeconds"||method=="setYear" || method=="setFullYear")
	  {
	   if(al->size()<1)
		 {
		  TError::ShowError(Error_argumentsnumbernotmatch,sp-1,iv,method);
		  return false;
		 }
	   int a;
	   IntValue& ag1=(*al)[0];
	   if(ag1.type==TYPE::NUMBER)
		  a=ag1.value;
	   else if(ag1.type==TYPE::ID)
		  {
		   TItem* item;
		   if(runtime_table->GetItemByName(GetNameByID(ag1.value),&item)==false)
			 {
              TError::ShowError(Error_expectdefinition,sp-1,ag1,GetNameByID(ag1.value));
			  return false;
			 }
		   if(item->type==TYPE::INT)
			  a=item->value;
		   else
			 {
			  TError::ShowError(Error_argumentstypenotmatch,sp-1,ag1,method);
			  return false;
			 }
		  }//end TYPE::ID
	   else if(ag1.type==TYPE::COBJ || ag1.type==TYPE::LOBJ)
		  {
		   a=0;
		  }
	   else
	      {
		   TError::ShowError(Error_argumentstypenotmatch,sp-1,ag1,method);
		   return false;
		  }
	   int off;
	   if(method=="setYear" || method=="setFullYear")
		  off=0;
	   else if(method=="setMonth")
		  off=1;
	   else if(method=="setDate")
		  off=2;
	   else if(method=="setHours")
		  off=3;
	   else if(method=="setMinutes")
		  off=4;
	   else
		  off=5;
	   IntValue* iv=runtime_heap->GetHeapItem(ov,off,sp);
	   if(iv==NULL)
		 {
		  cout<<"heap access error"<<endl;
		  return false;
		 }
	   iv->value=a;
	   iv->type=TYPE::NUMBER;
	  }
	//---------------------------------------------------------------
	else if(method=="setTime")
	  {
	   time_t now;
	   now = time(0);
	   tm *tnow = localtime(&now);
	   //隐式转换
	   IntValue* iv0=runtime_heap->GetHeapItem(ov,0,sp);
	   if(iv0==NULL)
		 {
		  cout<<"heap access error"<<endl;
		  return false;
		 }
       iv0->value=1900+tnow->tm_year;//年
	   IntValue* iv1=runtime_heap->GetHeapItem(ov,1,sp);
	   if(iv1==NULL)
		 {
		  cout<<"heap access error"<<endl;
		  return false;
		 }
	   iv1->value=tnow->tm_mon+1;//月
	   IntValue* iv2=runtime_heap->GetHeapItem(ov,2,sp);
	   if(iv2==NULL)
		 {
		  cout<<"heap access error"<<endl;
		  return false;
		 }
	   iv2->value=tnow->tm_mday;//日
	   IntValue* iv3=runtime_heap->GetHeapItem(ov,3,sp);
	   if(iv3==NULL)
		 {
		  cout<<"heap access error"<<endl;
		  return false;
		 }
	   iv3->value=tnow->tm_hour;//小时
	   IntValue* iv4=runtime_heap->GetHeapItem(ov,4,sp);
	   if(iv4==NULL)
		 {
		  cout<<"heap access error"<<endl;
		  return false;
		 }
	   iv4->value=tnow->tm_min;//分
	   IntValue* iv5=runtime_heap->GetHeapItem(ov,5,sp);
	   if(iv5==NULL)
		 {
		  cout<<"heap access error"<<endl;
		  return false;
		 }
	   iv5->value=tnow->tm_sec;//秒
	  }
	//---------------------------------------------------------------
	else if(method=="toGMTString" || method=="toLocaleString" || method=="toString")
	  {
	   /*vector<IntValue> cvec;
	   if(runtime_heap->GetArray(object->value,cvec)==false)
		 {
		  TError::ShowError(Error_arrayindexexceed,sp-1,iv,"");
		  return false;
		 }*/
	   string str;
	   int i=0;
	   for(;i<int(cvec.size()-1);i++)
	      {
	       if(cvec[i].type==TYPE::NUMBER)
	          str+=IntToStr(cvec[i].value)+",";
	       else if(cvec[i].type==TYPE::STR)
		      str+=GetNameByID(cvec[i].value)+",";
		   else
			  str+=",";
	      }
	   if(cvec[i].type==TYPE::NUMBER)
		   str+=IntToStr(cvec[i].value);
	   else if(cvec[i].type==TYPE::STR)
		   str+=GetNameByID(cvec[i].value);
	   str="*"+str;
	   int ti=current_string_index++;
	   symbol_table[ti]=str;
	   iv.type=TYPE::STR;
	   iv.value=ti;
	  }
	else if(method=="UTC")
	  {
	   iv.value=0;
	  }
	else
	  {
	   TError::ShowWarning(Warning_compilernotrealizethismethod,sp-1,iv,method);
	   iv.value=0;
	  }
   }
 return true;
}
//--------------------------------------------------------------------------------------------

int TSimulation::IsGlobalFunction(const string& funcname)
{
 //return binary_search(&(gfn[0]),&(gfn[gfn_size-1]),funcname);
 pair<vector<string>::iterator,vector<string>::iterator> res;
 res=equal_range(gfn_vec.begin(),gfn_vec.end(),funcname);
 if(res.first!=res.second)//找到了
    return res.first-gfn_vec.begin();
 return -1;
}
//--------------------------------------------------------------------------------------------

bool TSimulation::SimulateGlobalFunction(int function_index,IntValue& iv,const string& method,
	                        vector<IntValue>* al,const int& sp,const int& env,bool& isreturn)
{
 iv.type=TYPE::NUMBER;
 iv.value=0;
 isreturn=true;

 //----------------------------------------------------------------------------
 //function_index对应
 switch(function_index)
 {
    //if(/*method=="parseInt" || method=="parseFloat"*/
    //function_index==18 || function_index==19)
   case 18:
   case 19:
   {
	if(al->empty())
	  {
	   TError::ShowError(Error_argumentsnumbernotmatch,sp-1,iv,method);
	   return false;
	  }
    string str;
	IntValue& ag1=(*al)[0];
	if(ag1.type==TYPE::STR)
	   str=GetNameByID(ag1.value);
	else if(ag1.type==TYPE::ID)
	  {
	   TItem* item;
	   if(runtime_table->GetItemByName(GetNameByID(ag1.value),&item)==false)
		{
         TError::ShowError(Error_expectdefinition,sp-1,ag1,GetNameByID(ag1.value));
		 return false;
		}
	   if(item->type==TYPE::STRING)
		  str=GetNameByID(item->value);
	   else
	    {
		 TError::ShowError(Error_argumentstypenotmatch,sp-1,ag1,method);
		 return false;
		}
	  }//end TYPE::ID
	else if(ag1.type==TYPE::COBJ || ag1.type==TYPE::LOBJ)
	  {
	   str="*0";
	  }
	else if(ag1.type==TYPE::NUMBER)
	  {
	   iv.value=ag1.value;
	   return true;
	  }
    else
	  {
	   TError::ShowError(Error_argumentstypenotmatch,sp-1,ag1,method);
	   return false;
	  }
    
	char* stopstring;
	str=str.substr(1);
	if(function_index==19)
	  {
	   if(al->size()==2)
		 {
		  int base=10;
		  IntValue& ag2=(*al)[1];
		  if(ag2.type==TYPE::NUMBER)
			 base=ag2.value;
		  else if(ag2.type==TYPE::ID)
			{
			  TItem* item;
	          if(runtime_table->GetItemByName(GetNameByID(ag2.value),&item)==false)
		        {
                 TError::ShowError(Error_expectdefinition,sp-1,ag2,GetNameByID(ag2.value));
		         return false;
		        }
	          if(item->type==TYPE::INT)
		         base=item->value;
		    }
		  iv.value=strtol(str.c_str(),&stopstring,base);
		 }
	   else
		 {
		  iv.value=strtol(str.c_str(),&stopstring,10);
		 }
	  }
	else//parseFloat
	  {
	   iv.value=strtol(str.c_str(), &stopstring,10);
	  }
	break;
   }//end if parseInt
 //----------------------------------------------------------------------------
 case 3: //|| /*method=="escape"*/function_index==12)//强制转换
 case 12:
   {
	if(al->empty())
	  {
	   TError::ShowError(Error_argumentsnumbernotmatch,sp-1,iv,method);
	   return false;
	  }
	iv.type=TYPE::STR;
    string str;
	IntValue& ag1=(*al)[0];
	if(ag1.type==TYPE::STR)
	   str=GetNameByID(ag1.value).substr(1);
	else if(ag1.type==TYPE::INT)
	   str=IntToStr(ag1.value);
	else if(ag1.type==TYPE::ID)
	   {
	   TItem* item;
	   if(runtime_table->GetItemByName(GetNameByID(ag1.value),&item)==false)
		 {
          TError::ShowError(Error_expectdefinition,sp-1,ag1,GetNameByID(ag1.value));
		  return false;
		 }
	   if(item->type==TYPE::INT)
		  str=IntToStr(item->value);
	   else if(item->type==TYPE::STRING)
		  str=GetNameByID(item->value).substr(1);
	   else
		  str="kobe";
	   }//end TYPE::ID
	else if(ag1.type==TYPE::NUMBER)
	   {
	    str=IntToStr(ag1.value);
	   }
	else
	   str="kobe";
    str="*"+str;
    int ti=current_string_index++;
	symbol_table[ti]=str;
	iv.value=ti;
	break;
   }
 //----------------------------------------------------------------------------
 //else if(/*method=="Number"*/function_index==2)//强制转换
 case 2:  
   {
	if(al->empty())
	  {
	   TError::ShowError(Error_argumentsnumbernotmatch,sp-1,iv,method);
	   return false;
	  }
	iv.type=TYPE::NUMBER;
	int rv;
    IntValue& ag1=(*al)[0];
	if(ag1.type==TYPE::NUMBER)
	   rv=ag1.value;
	else if(ag1.type==TYPE::STR)
	   {
	   string temp=GetNameByID(ag1.value);
	   rv=atoi(temp.c_str());
	   }
	else if(ag1.type==TYPE::ID)
	   {
	   TItem* item;
	   if(runtime_table->GetItemByName(GetNameByID(ag1.value),&item)==false)
		 {
		  TError::ShowError(Error_expectdefinition,sp-1,ag1,GetNameByID(ag1.value));
		  return false;
		 }
	   if(item->type==TYPE::INT)
		  rv=item->value;
	   else if(item->type==TYPE::STRING)
		  rv=atoi(GetNameByID(item->value).c_str());
	   else
		  rv=1;
	   }//end TYPE::ID
	else
	   rv=1;
	break;
   }
 //-------------------------------------------------------------------------------------------
 //else if(/*method=="write"*/function_index==30)
 case 30:
   {
	//写入log
	break;
   }
 //-------------------------------------------------------------------------------------------
 //else if(/*method=="alert" || method=="confirm" || method=="prompt"*/function_index==6
	     //||function_index==9 || function_index==20)
 case 6:
 case 9:
 case 20:
   {
	iv.value=1;
	iv.type=TYPE::NUMBER;
	cout<<"ALERT!!!"<<endl;
    AddOneResult("ALERT","");
	break;
   }
 //-------------------------------------------------------------------------------------------
 //else if(/*method=="setTimeout"*/function_index==23)
 case 23:
    {
	iv.value=1;
	iv.type=TYPE::NUMBER;
	if(al->size()>=2)
	  {
	   string code="*";
	   IntValue& ag1=(*al)[0];
	   if(ag1.type==TYPE::STR)
		  code=GetNameByID(ag1.value);
	   else if(ag1.type==TYPE::ID)
		  {
		   string name=GetNameByID(ag1.value);
		   TItem* item;
		   if(runtime_table->GetItemByName(name,&item))
			 {
			  if(item->type==TYPE::STRING)
				 code=GetNameByID(item->value);
			 }
		  }
	   code=code.substr(1);
	   if(code.find("location")!=string::npos)//存在location操作
		  {
		  cout<<"JUMP!!! "<<UNKNOWNSITE.substr(1)<<endl;
	      AddOneResult("JUMP",UNKNOWNSITE.substr(1));
		  }
	   }
	else
	   {
		TError::ShowError(Error_argumentsnumbernotmatch,sp-1,iv,"setTimeout");
		return false;
	   }
	break;
    }
 //-------------------------------------------------------------------------------------------
 //else if(/*method=="redirect"*/function_index==21)//重要,起跳转作用
 case 21:
   {
	if(al->empty())
	  {
	   TError::ShowError(Error_argumentsnumbernotmatch,sp-1,iv,method);
	   return false;
	  }
    string str;
	IntValue& ag1=(*al)[0];
	if(ag1.type==TYPE::STR)
	   str=GetNameByID(ag1.value);
	else if(ag1.type==TYPE::ID)
	  {
	   TItem* item;
	   if(runtime_table->GetItemByName(GetNameByID(ag1.value),&item)==false)
		{
         TError::ShowError(Error_expectdefinition,sp-1,ag1,GetNameByID(ag1.value));
		 return false;
		}
	   if(item->type==TYPE::STRING)
		  str=GetNameByID(item->value);
	   else
	    {
		 TError::ShowError(Error_argumentstypenotmatch,sp-1,ag1,method);
		 return false;
		}
	  }//end TYPE::ID
	else if(ag1.type==TYPE::COBJ || ag1.type==TYPE::LOBJ)
	  {
	   str=UNKNOWNSITE;
	  }
    else
	  {
	   TError::ShowError(Error_argumentstypenotmatch,sp-1,ag1,method);
	   return false;
	  }
	cout<<"JUMP!!! "<<str.substr(1)<<endl;
	AddOneResult("JUMP",str.substr(1));
	break;
   }
 //-------------------------------------------------------------------------------------------
 //else if(/*method=="flush" || method=="registerCFunction" || method=="deleteResponseHeader" ||
 //	     method=="addClient" || method=="addResponseHeader" || method=="debug"*/
 //		 function_index==14 || function_index==22 || function_index==11 || function_index==4 
 //		 || function_index==5 || function_index==10)
 case 14:
 case 22:
 case 11:
 case 4:
 case 5:
 case 10:
   {
   isreturn=false;
   break;
   }
 //else if(/*method=="getOptionValue" || method=="getOptionValueCount"*/
 //       function_index==15 || function_index==16)
 case 15:
 case 16:
   {
	iv.value=0;
	break;
   }
 //else if(/*method=="ssjs_generateClientID" || method=="ssjs_getCGIVariable" || method=="callC" ||
 //     method=="eval"*/
 // function_index==24 || function_index==25 || function_index==8 || function_index==13)
 case 24:
 case 25:
 case 8:
 case 13:
   {
	iv.value=0;
	break;
   }
 //else if(/*method=="ssjs_getClientID"*/function_index==26)//分配ID
 case 26:
   {
	int id=rand();
	iv.value=id;
	break;
   }
 //else if(/*method=="blob"*/function_index==7 || function_index==27 || function_index==29 )
 case 7:
 case 27:
 case 29:
   {
    break;
   }
 //---------------------------------------------------------------------------------
 //else if(/*method=="isNaN"*/function_index==17)
 case 17:
   {
	if(al->empty())
	  {
	   TError::ShowError(Error_argumentsnumbernotmatch,sp-1,iv,method);
	   return false;
	  }
	int rv=0;
	IntValue& ag1=(*al)[0];
	if(ag1.type==TYPE::NUMBER)
	   rv=ag1.value;
	else if(ag1.type==TYPE::ID)
	   {
	    TItem* item;
	    if(runtime_table->GetItemByName(GetNameByID(ag1.value),&item)==false)
		  {
           TError::ShowError(Error_expectdefinition,sp-1,ag1,GetNameByID(ag1.value));
		   return false;
		  }
	    if(item->type==TYPE::INT)
		   rv=item->value;
		else
		   rv=0;
	    }
    else
	    rv=0;
    iv.value=(rv==0?0:1);
	break;
   }//end isNaN
 //---------------------------------------------------------------------------------
 /*else if(method=="taint")
   ;
 else if(method=="untaint")
   ;*/
 //else if(/*method=="unescape"*/function_index==28)
 case 28:
   {
	iv.type=TYPE::STR;
	string str="*&";
    int ti=current_string_index++;
	symbol_table[ti]=str;
	iv.value=ti;
	break;
   }
 //else if(/*method=="Function"*/function_index==1)
 case 1:
   {
	iv.type=TYPE::COBJ;
	iv.value=0;
	break;
   }
 //----------------------------------------------------------------------------
 //else if(/*method=="Array"*/function_index==0)
 case 0:
   {
	int index;
	vector<IntValue>& vec=runtime_heap->GetNew(index);
	if(al->size()>0)//有参Array构造函数
	  {
	   if(al->size()==1)//单参且参数为数或ID的值为数,参的意义是分配数组大小
		 {
		  if((*al)[0].type==TYPE::NUMBER)
			 vec.resize((*al)[0].value);
		  //---------------------------------------------
		  else if((*al)[0].type==TYPE::ID)
			{
			 string name=GetNameByID((*al)[0].value);
			 TItem* item;
			 if(runtime_table->GetItemByName(name,&item))
			   {
			    if(item->type==TYPE::INT)
				   vec.resize(item->value);
				else if(item->type==TYPE::STRING)
				   {
				   IntValue iv(item->value);
				   iv.type=TYPE::STR;
                   vec.push_back(iv);
				   }
				else//待扩展
				   {
				   vec.resize(DEFAULT_ARRAY_SIZE);
				   }
			   }//end GetItemByName
			else if((*al)[0].type==TYPE::STR)//单参,参数为一个字符串
			   {
                vec.push_back((*al)[0]);
			   }
			else if((*al)[0].type==TYPE::COBJ || (*al)[0].type==TYPE::LOBJ)
			   {
				vec.resize(DEFAULT_ARRAY_SIZE);
			   }
			else
			   {
				vec.resize(DEFAULT_ARRAY_SIZE);
			   }
		   }//end 单参
	   else//多参,示为一维数组,且每个参数为数组初始化值
		   {
			for(int i=0;i<int(al->size());i++)//有问题,考虑ID
			   {
                if((*al)[i].type==TYPE::ID)
				  {
				   string name=GetNameByID((*al)[i].value);
				   TItem* item;
				   if(runtime_table->GetItemByName(name,&item))
					 {
					  IntValue iv(item->value);
					  if(item->type==TYPE::INT)
						 vec.push_back(iv);
					  else if(item->type==TYPE::STRING)
						{
						 iv.type=TYPE::STR;
						 vec.push_back(iv);
						}
					  else//待扩展
						{
						 vec.resize(DEFAULT_ARRAY_SIZE);
						 break;
						}
					 }//end GetItemByName
				   else//改进,如果多参里面有一个未知的DOM变量
					 {
					  IntValue iv(0);
					  vec.push_back(iv);
					 }
				  }//end if ID
				else
				   vec.push_back((*al)[i]);
			   }
		   }//end else 多参
		}//end 有参
	else//无参Array构造函数
		{
		 vec.resize(DEFAULT_ARRAY_SIZE);//空分配的数组空间
		}
	iv.value=index;
	iv.type=TYPE::HEAP;
	}//end if Array
	break;
   }
 default:
   {
	iv.type=TYPE::COBJ;
	iv.value=0;
	break;
   }
 }
 return true;
}
//--------------------------------------------------------------------------------------------
