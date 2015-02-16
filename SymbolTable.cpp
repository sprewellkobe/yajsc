/*--------------------------------------------------------------------------------------------
Yet Another JS Compiler, by kobe, 2006,9
Symbol Table
Map with PushDown Stack
--------------------------------------------------------------------------------------------*/
#include "SymbolTable.hpp"
#include "Heap.hpp"
//------------------------------------------------------------------------

TSymbolTable::TSymbolTable()
{
 InitializeTemp();
}
//------------------------------------------------------------------------

void TSymbolTable::InitializeTemp()
{
 string name=FUNCTEMP;//插入临时变量
 items.clear();
 vector<TItem> vec;
 TItem item;
 item.value=0;
 item.type=0;
 item.level=0;
 vec.push_back(item);
 //items.insert(items.begin(),pair<string,vector<TItem> >(name,vec));
 items[name]=vec;
 //cout<<"Initialized Temp"<<endl;
}
//------------------------------------------------------------------------

void TSymbolTable::ClearUp()
{
 items.clear();
 while(!funcitems_stack.empty())
	   funcitems_stack.pop();
 InitializeTemp();
}
//------------------------------------------------------------------------

bool TSymbolTable::IfSymbolExists(const string& name,map<string,vector<TItem> >::iterator& iter,int& which)
{
 if(funcitems_stack.empty())
   {
	which=-1;
    iter=items.find(name);
    if(iter==items.end())
       return false;
    return true;
   }
 map<string,vector<TItem> >& cmap=funcitems_stack.top();
 iter=cmap.find(name);
 if(iter!=cmap.end())
   {
	which=1;
	return true;
   }
 iter=items.find(name);
 if(iter!=items.end())
   {
	which=0;
	return true;
   }
 //cout<<name<<" not found"<<endl;
 return false;
}
//------------------------------------------------------------------------

bool TSymbolTable::AddSymbol(const string& name,const TItem& item)
{
 if(funcitems_stack.empty())
   {
	int w;
    map<string,vector<TItem> >::iterator iter;
    bool r=IfSymbolExists(name,iter,w);
    if(r==false)//符号名第一次出现
      {
	   vector<TItem> vec;
       vec.push_back(item);
	   items.insert(iter,pair<string,vector<TItem> >(name,vec));
	   return true;
      }
    else//符号名存在
      {
       vector<TItem>& vec=iter->second;
	   //新加入的符号层数必须大于原来层,否则编译报错(改进,相同也可以,js特殊语法)
	   /*if(item.level>vec.back().level)
	     {
	      vec.push_back(item);
	      return true;
         }
	   else if(item.level==vec.back().level)
		 {
		  TItem& it=vec.back();
		  it=item;
		  return true;
		 }*/
       //return false;
      if(item.level<=vec.back().level)
        {
         TItem vb=vec.back();
         while(vb.level>=item.level)
              {
               if(vb.type==TYPE::HEAP || vb.type==TYPE::DATE)
                  runtime_heap->DeleteHeapItem(vb.value);
               vec.pop_back();
               if(vec.empty())
                  break;
               vb=vec.back();
              }
         }
       vec.push_back(item);
       return true;
      }
    return false;
   }//end if funcitems_stack.empty()

 map<string,vector<TItem> >& cmap=funcitems_stack.top();
 map<string,vector<TItem> >::iterator iter;
 int w;
 bool r=IfSymbolExists(name,iter,w);
 if(r==false || w==0)//没找到或者在全局中发现
   {
	//cout<<name<<endl;
	vector<TItem> vec;
	vec.push_back(item);
	cmap.insert(cmap.begin(),pair<string,vector<TItem> >(name,vec));
	return true;
   }
 else if(w==1)//在当前cmap中发现
   {	
    vector<TItem>& vec=iter->second;
	/*if(item.level>vec.back().level)
	  {
	   vec.push_back(item);
	   return true;
	  }
	else if(item.level==vec.back().level)
	  {
	   TItem& it=vec.back();
	   it=item;
	   return true;
	  }*/
    if(item.level<=vec.back().level)
      {
       TItem vb=vec.back();
       while(vb.level>=item.level)
            {
             if(vb.type==TYPE::HEAP || vb.type==TYPE::DATE)
                runtime_heap->DeleteHeapItem(vb.value);
             vec.pop_back();
             if(vec.empty())
                break;
             vb=vec.back();
            }
      }
    vec.push_back(item);
    return true;
	//return false;
   }
 return false;
}
//------------------------------------------------------------------------

bool TSymbolTable::AddTempSymbol(const TItem& item)//利用临时变量
{
 int w=0;
 string name=FUNCTEMP;
 map<string,vector<TItem> >::iterator iter;
 bool rv=IfSymbolExists(name,iter,w);
 if(rv==false)
    cout<<"FATAL ERROR: tmp lost"<<endl;
 vector<TItem>& vec=iter->second;
 TItem& myitem=vec.back();
 myitem.value=item.value;
 myitem.type=item.type;
 //myitem.str=item.str;
 return true;
}
//------------------------------------------------------------------------

bool TSymbolTable::DeleteOverLevelSymbolByName(int level,const string& name)
{
 if(funcitems_stack.empty())
   {
    if(level==0)
       return true;
    map<string,vector<TItem> >::iterator iter=items.find(name);
    if(iter!=items.end())
      {
       vector<TItem>& vec=iter->second;
       TItem item=vec.back();
       while(item.level>=level)
            {
             if(item.type==TYPE::HEAP || item.type==TYPE::DATE)
                runtime_heap->DeleteHeapItem(item.value);
             vec.pop_back();
             if(vec.empty())
                break;
             item=vec.back();
            }
       if(vec.empty())//需要在符号表中清除
          items.erase(iter);
      }//end iter!=items.end()
    return true;
   }//end if funcitems_stack.empty()
 map<string,vector<TItem> >& cmap=funcitems_stack.top();
 map<string,vector<TItem> >::iterator iter=cmap.find(name);
 if(iter!=cmap.end())
   {
    vector<TItem>& vec=iter->second;
    TItem item=vec.back();
    while(item.level>=level)
         {
          if(item.type==TYPE::HEAP || item.type==TYPE::DATE)
            {
             runtime_heap->DeleteHeapItem(item.value);
            }
          vec.pop_back();
          if(vec.empty())
             break;
          item=vec.back();
         }
    if(vec.empty())//需要在符号表中清除
       cmap.erase(iter);
   }
 return true;
}
//------------------------------------------------------------------------
bool TSymbolTable::DeleteLastLevelSymbol(int level)
{
 //cout<<"level:"<<level<<endl;
 if(funcitems_stack.empty())
   {
    if(level==0)
       return true;
    map<string,vector<TItem> >::iterator iter=items.begin();
    for(;iter!=items.end();)
       {
        vector<TItem>& vec=iter->second;
        TItem item=vec.back();
	    while(item.level>=level)
		     {
              if(item.type==TYPE::HEAP || item.type==TYPE::DATE)
			     runtime_heap->DeleteHeapItem(item.value);
		      vec.pop_back();
		      if(vec.empty())
			     break;
		      item=vec.back();
		     }
	    if(vec.empty())//需要在符号表中清除
           items.erase(iter++);
        else
		   ++iter;
	   }//end for
    return true;
   }
 
  map<string,vector<TItem> >& cmap=funcitems_stack.top();
  map<string,vector<TItem> >::iterator iter=cmap.begin();
  for(;iter!=cmap.end();)
     {
      vector<TItem>& vec=iter->second;
      TItem item=vec.back();
	  while(item.level>=level)
		   {
            if(item.type==TYPE::HEAP || item.type==TYPE::DATE)
			  {
			   runtime_heap->DeleteHeapItem(item.value);
			  }
		    vec.pop_back();
		    if(vec.empty())
			   break;
		    item=vec.back();
		   }
	  if(vec.empty())//需要在符号表中清除
         cmap.erase(iter++);
      else
		 ++iter;
	 }
 return true;
}
//------------------------------------------------------------------------

bool TSymbolTable::GetItemByName(const string& name,TItem** item)//为false时可以影响到判断是否是TYPE::STRING
{
 if(funcitems_stack.empty())
   {
    map<string,vector<TItem> >::iterator iter;
    iter=items.find(name);
    if(iter==items.end())
	  {
	   //cout<<"not find "<<name<<endl;
       return false;
	  }
    vector<TItem>& vec=iter->second;//下推栈
    *item=(TItem*) (&(vec.back()));
    return true;
   }

 map<string,vector<TItem> >& cmap=funcitems_stack.top();
 map<string,vector<TItem> >::iterator iter;
 iter=cmap.find(name);
 if(iter==cmap.end())
   {
    iter=items.find(name);
	if(iter==items.end())
	  {
	   //cout<<"not find "<<name<<endl;
	   return false;
	  }
	vector<TItem>& vec=iter->second;
    *item=(TItem*) (&(vec.back()));
    return true;
   }
 vector<TItem>& vec=iter->second;//下推栈
 *item=(TItem*) (&(vec.back()));
 return true;
}
//------------------------------------------------------------------------

void TSymbolTable::Print()
{
 map<string,vector<TItem> >::iterator iter=items.begin();
 for(;iter!=items.end();)
    {
     string name=iter->first;
	 vector<TItem>& vec=iter->second;
     for(int i=0;i<(int)vec.size();i++)
		{
		 TItem p=vec[i];
		 cout<<name<<":"<<p.value<<endl;
		}//end for i
    iter++;
	}//end for
}
//------------------------------------------------------------------------

vector<TItem2> TSymbolTable::GetOutput()
{
 vector<TItem2> res;
 map<string,vector<TItem> >::iterator iter=items.begin();
 for(;iter!=items.end();)
    {
     string name=iter->first;
	 vector<TItem>& vec=iter->second;
	 if(vec.empty())
	    cout<<"vec should not be empty"<<endl;
     for(int i=0;i<(int)vec.size();i++)
		{
		 TItem2 p;
		 p.name=name;
		 p.value=vec[i].value;
		 p.type=vec[i].type;
		 p.level=vec[i].level;
		 res.push_back(p);
		}//end for i
    iter++;
	}//end for
 return res;
}
//------------------------------------------------------------------------

void TSymbolTable::EnterFunc()
{
 map<string,vector<TItem> > tempmap;
 funcitems_stack.push(tempmap);
}
//------------------------------------------------------------------------

void TSymbolTable::QuitFunc()
{
 map<string,vector<TItem> >& cmap=funcitems_stack.top();

 map<string,vector<TItem> >::iterator iter=cmap.begin();
 for(;iter!=cmap.end();iter++)
    {
     vector<TItem>& vec=iter->second;
     for(int i=0;i<(int)(vec.size());i++)
	    {
         if(vec[i].type==TYPE::HEAP || vec[i].type==TYPE::DATE)
			runtime_heap->DeleteHeapItem(vec[i].value);
		}
	}

 funcitems_stack.pop();
}
//------------------------------------------------------------------------

int TSymbolTable::GetTableSize()
{
 int res=0;
 map<string,vector<TItem> >::iterator iter=items.begin();
 for(;iter!=items.end();iter++)
	{
	 vector<TItem>& vec=iter->second;
	 if(vec.empty())
	   cout<<"vec should not be empty"<<endl;
	 res+=vec.size();
	}
 if(!funcitems_stack.empty())
   {
	map<string,vector<TItem> >& cmap=funcitems_stack.top();
	map<string,vector<TItem> >::iterator iter=cmap.begin();
    for(;iter!=cmap.end();iter++)
	   {
		vector<TItem>& vec=iter->second;
		if(vec.empty())
		   cout<<"vec should not be empty"<<endl;
		res+=vec.size();
	   }
   }
 return res;
}
//------------------------------------------------------------------------
