#include <iostream>
#include <algorithm>
#include <iomanip>
#include "Heap.hpp"
//--------------------------------------------------------------------------------------------

THeap::THeap()
{
 ClearUp();
}
//--------------------------------------------------------------------------------------------

THeap::~THeap()
{
 ClearUp();
}
//--------------------------------------------------------------------------------------------

void THeap::ClearUp()
{
 heaplist.clear();
 recursion_stack.clear();
}
//--------------------------------------------------------------------------------------------

bool THeap::DeleteHeapItem(const int& addr,bool force)
{
 if(addr>=int(heaplist.size()))//����������
    return false;
 rcount[addr]--;
 if(force==false && rcount[addr]>0)//���ü���
	return true;
 
 recursion_stack.push_back(addr);
 vector<IntValue>& cvec=heaplist[addr];
 vector<IntValue>::iterator iter=cvec.begin();
 for(;iter!=cvec.end();iter++)
	{
	 IntValue& iv=*iter;
	 if(iv.type==TYPE::HEAP && AddrExistedInRecursionStack(iv.value)==false )//avoid unlimited recursion
		DeleteHeapItem(iv.value,true);
	}
 cvec.clear();
 recursion_stack.pop_back();
 vacancy.push(addr);//�������ջ
 return true;
}
//--------------------------------------------------------------------------------------------

bool THeap::AddrExistedInRecursionStack(const int& addr)
{
 vector<int>::iterator vi=find(recursion_stack.begin(),recursion_stack.end(),addr);
 if(vi==recursion_stack.end())
    return false;
 return true;
}
//--------------------------------------------------------------------------------------------

int THeap::GetItemLength(const int& addr)
{
 if(int(heaplist.size())<=addr || addr<0)
	return -1;
 vector<IntValue>& cvec=heaplist[addr];
 return cvec.size();
}
//--------------------------------------------------------------------------------------------

bool THeap::GetArray(const int& addr,vector<IntValue>& vec)
{
 if(int(heaplist.size())<=addr || heaplist.empty() || addr<0)
	return false;
 vec=heaplist[addr];
 return true;
}
//--------------------------------------------------------------------------------------------

bool THeap::ReverseArray(const int& addr)
{
 if(int(heaplist.size())<=addr || addr<0)
	return false;
 vector<IntValue>& vec=heaplist[addr];
 reverse(vec.begin(),vec.end());
 return true;
}
//--------------------------------------------------------------------------------------------

bool THeap::AddReferenceCount(const int& index)
{
 rcount[index]++;
 return true;
}
//--------------------------------------------------------------------------------------------

vector<IntValue>& THeap::GetNew(int& index)
{
 
 if(!vacancy.empty())//���п��öѵ�ַ
   {
    index=vacancy.top();
	vacancy.pop();
	rcount[index]=0;
	return heaplist[index];
   }
 index=heaplist.size();
 
 vector<IntValue> temp;
 heaplist.push_back(temp);
 rcount.push_back(0);
 
 return heaplist.back();
}
//--------------------------------------------------------------------------------------------

int THeap::GetHeapSize()
{
 int c=0;
 for(int i=0;i<int(heaplist.size());i++)
	 c+=heaplist[i].size();
 return c;
}
//--------------------------------------------------------------------------------------------

IntValue* THeap::GetHeapItem(const int& addr,const int& offset,const int& sp)//��ֹ����������
{
 int myaddr=addr;
 if(addr<0 || addr>=int(heaplist.size()) || offset<0 || offset>65535)
   {
	IntValue temp;
	TError::ShowWarning(Warning_arrayaccesserror,sp-1,temp,"");
    return NULL;
   }
 vector<IntValue>& vec=heaplist[myaddr];
 if(offset>=int(vec.size()))//�������Խ��
   {
	IntValue temp(0);
	int c=vec.size();
	while(c++<offset+1)
	      vec.push_back(temp);
   }
 return (IntValue*)(&(vec[offset]));
}
//--------------------------------------------------------------------------------------------

void THeap::DisplayHeap(map<int,string>& symbol_table,bool order)
{
 if(order==false)
   {
    int c=0;
    for(int i=0;i<int(heaplist.size());i++)
	   {
	    for(int j=0;j<int(heaplist[i].size());j++)
	       {
			if(heaplist[i][j].type==TYPE::STR)
			   cout<<setw(8)<<symbol_table[ heaplist[i][j].value ]<<" ";
			else
	           cout<<setw(8)<<heaplist[i][j].value<<" ";
		    c++;
		    if(c%8==0)
		      {cout<<endl;c=0;}
		   }
	   }
   }//end order==false
 else //���Ľ�
   {
    for(int i=0;i<int(heaplist.size());i++)
	   {
		cout<<"REFERENCE COUNT: "<<rcount[i]<<endl;
		int c=0;
	    for(int j=0;j<int(heaplist[i].size());j++)
	       {
            if(heaplist[i][j].type==TYPE::STR)
			   cout<<setw(8)<<symbol_table[ heaplist[i][j].value ]<<" ";
			else
	           cout<<setw(8)<<heaplist[i][j].value<<" "<<heaplist[i][j].type<<" ";
		    c++;
		    if(c%8==0)
		      {cout<<endl;}
		   }
	    cout<<endl<<endl;
	   }
   }
}
//--------------------------------------------------------------------------------------------
