/*--------------------------------------------------------------------------------------------
Yet Another JS Compiler, by kobe, 2006,9
Compiler
Stack Push Machine
--------------------------------------------------------------------------------------------*/
#include <iostream>
#include <iomanip>
#include "Compiler.hpp"
#include <time.h>
#include <boost/regex.hpp>
using namespace std;
using namespace boost;
//---------------------------------------------------------------------------------------

void Compiler::Initialize()
{
 runtime_table.ClearUp();
 jump_result="";
 jumpmap.clear();
 functiontree.ResetActiveNode();
 ClearStack(endnewpos);
 map<string,int> labelmap;
 labellist.push(labelmap);//����ȫ����ת��
 infunction=0;
 inunfunction=0;
 //funcenv=0;//funcenv�������Ƶ��ú����Ļ���
 runtime_table.runtime_heap=(THeap *)(&runtime_heap);
 //���ű�ɾ������ʱ,��Ҫ����Ƿ��ڶ���,�����õ�heapָ��

 sim=new TSimulation(symbol_table,&runtime_table,&runtime_heap,jump_result);
}
//---------------------------------------------------------------------------------------

string Compiler::GetNameByID(const int& number)//���ID����ķ�������,����������������ջ
{
 map<int,string>::iterator iter=symbol_table.find(number);
 if(iter==symbol_table.end())
	return "";
 return iter->second;
}
//---------------------------------------------------------------------------------------
//��ɨ��һ��,�ҳ����е�FUNC��ʶ,����FUNCTIONTREE
bool Compiler::PreScan(vector<int>& code,vector<int>& codeenv)
{
 functiontree.ResetActiveNode();
 if(code.empty())
	return false;
 int sp=0;

 IntValue c2(0);
 
 int value=0;
 int codelevel=0;
 while(true)
	  {
	   value=code[sp];
	   codelevel=codeenv[sp];

	   if(value==functionchar)//FUNCTIONCHAR,��Ŀ
	     {
	      int preid=code[sp-1];
	      string name=GetNameByID(preid);
		  if(name.empty())
			{
			 TError::ShowError(Error_functionnameisnotid,sp,c2,name);
			 return false;
			}
		  /*
          map<string,int>::iterator iter=functionmap.find(name);
	      if(iter==functionmap.end())
	        {
	         functionmap.insert(iter,make_pair(name,sp+1));
	        }
	      else//�������ض���(��������),js֧������
	        {
	        //TError::ShowError(Error_functionsameenv,sp,c2,name);
	        //return false;
            iter->second=sp+1;
	        }
		 */
		 if(name=="window.onload")
		   {
			code.pop_back();
	        codeenv.pop_back();
	        code.push_back(preid);
	        codeenv.push_back(0);
	        code.push_back(0);
	        codeenv.push_back(0);
	        code.push_back(eachar);
	        codeenv.push_back(0);
            code.push_back(codeend);
	        codeenv.push_back(0);
		   }//end if name=="window.onload"
		 FNTreeNode* rn=functiontree.AddFunction(name,sp+1,codelevel);
         functiontree.EnterActiveNode(*rn);
         }//end if functionchar
	   else if(value==efchar)
		 {
		  functiontree.QuitActiveNode();
		 }
	   
	   sp++;
	   if(value==codeend)
		  break;
	  }//end while
 return true;
}
//---------------------------------------------------------------------------------------

bool Compiler::compile(vector<int>& code,vector<int>& codeenv)
{
 Initialize();
 bool finished=false;
 int sp=0;//��ǰָ���ַ
 int preenv=-99999;//ǰ��Ļ���,���ڲ�����������

 if(!PreScan(code,codeenv))
   {
	Close();
    return false;
   }
 cout<<"PreScan Successfully"<<endl;
 //int csize=code.size();
 while(true)
    {
	 if(sp<0 || sp>=int(code.size()))
	   {
		cout<<endl<<"Compiled Failed"<<endl;
		break;
	   }
	 int value=code[sp];
	 int cenv;
     //if(codeenv[sp]>=0)
	 cenv=codeenv[sp];
	 //else
		//cenv=codeenv[sp]-funcenv_stack.GetSum();
     //cout<<"SP:"<<sp<<" ";
     
	 if(infunction>0)//�������������Ķ���
       {
		if(value==efchar)
		  {
		   infunction--;
		  }
		else if(value==functionchar)
		  {
		   infunction++;
		  }
		sp++;
		continue;
       }
	 else if(inunfunction>0)
	   {
		if(value==unefchar)
		  {
		   if(do_command(value,cenv,sp,code,codeenv)==false)
			  break;
		  }
		else if(value==unfunctionchar)
		  {
		   if(do_command(value,cenv,sp,code,codeenv)==false)
			  break;
		  }
		sp++;
		continue;
	   }

     //��������	 
	 if(cenv<preenv && cenv>=0)//����ĳһ����,����ֲ�����,�ͷſռ�
	   {
		//runtime_table.DeleteLastLevelSymbol(cenv+1);//ע�����һ���������
		//cout<<"exit env: "<<cenv+1<<endl;
	   }
	 else if(sp>=1&&codeenv[sp-1]<0)
	   {
        int temp=-1*codeenv[sp-1];
		temp=cenv+1<temp?cenv+1:temp;
		//runtime_table.DeleteLastLevelSymbol(temp);//ע�����һ���������
		//cout<<"exit env: "<<temp<<endl;
	   }
	 preenv=cenv;
     //��������

	 if(value==codeend)//������ֹ
	   {
		finished=true;
		sp++;
        break;
	   }
     else if(value>=0)//������,�м��������в���������0,��Ϊ��NEGָ��
	   {
        IntValue iv(value,true);
        runtime_stack.push_back(iv);
		sp++;
	   }
     else//ָ�� or ID or STR
	   {
		if(cenv>=0)
		  {
           if(do_command(value,cenv,sp,code,codeenv)==false)
              break;
		  }
		else//�����˳�������ʶ,ʹ��ǰһ��������
		  {
		   if(do_command(value,-1*cenv,sp,code,codeenv)==false)
              break;
		  }
       }//end else
	 CheckRuntimeStackSize();
	}//end while true
 
 Close();
 if(finished==true)
   {
    cout<<endl<<"Compiled Successfully"<<endl;
	return true;
   }
 cout<<endl<<"Compiled Failed"<<endl;
 return false;
}
//-------------------------------------------------------------------

void Compiler::Close()
{
 if(sim!=NULL)
	delete sim;
 sim=NULL;
}
//-------------------------------------------------------------------

bool Compiler::do_command(const int& command,const int& env,int& sp,
                          vector<int>& code,vector<int>& codeenv)
{
 unsigned int len=runtime_stack.size();
 if(command>split_number)//��ָ��
   {
    //c1,c2Ϊ����������
    IntValue c2;
	if(!runtime_stack.empty())
	   c2=runtime_stack.back();//�Ĵ���
 
	//envΪ��ǰ��������
    
	//��Ŀ-----------------------------------------------------------
	if(command==vartype)//var����,��Ŀ
	  {
	   return command_vartype(command,c2,env,sp);
	  }
	else if(command==ipp || command==iss)//i++,i--
	  {
	   return command_ippiss(command,c2,env,sp);
	  }
    else if(command==ppi || command==ssi)//++i,--i ��Ŀ
	  {
	   return command_ppissi(command,c2,env,sp);
	  }
    else if(command==labelchar)//label ��Ŀ
	  {
	   return command_label(command,c2,env,sp);
	  }
    else if(command==blchar)//��תBL
	  {
	   return command_blchar(command,c2,env,sp);
	  }
	else if(command==eachar)//EA��Ŀ,��������
	  {
	   return command_eachar(command,c2,env,sp);
	  }
	else if(command==eachar2)
	  {
	   return command_eachar2(command,c2,env,sp);
	  }
    else if(command==functionchar)//FUNCTION��Ŀ
	  {
	   return command_functionchar(command,c2,env,sp);
	  }
	else if(command==unfunctionchar)//��������
	  {
	   return command_unfunctionchar(command,c2,env,sp);
	  }
	else if(command==unefchar)
	  {
	   return command_unefchar(command,c2,env,sp);
	  }
    else if(command==returnchar || command==efchar || command==return0char)// RETURN
	  {
	   return command_return(command,c2,env,sp);
	  }
	else if(command==inichar)// INI
	  {
	   return command_inichar(command,c2,env,sp);
	  }
    else if(command==negchar || command==oppchar)//ȡ��,��Ŀ - !
	  {
	   return command_negoppchar(command,c2,env,sp);
	  }
	else if(command==withchar)
	  {
	   return command_withchar(command,c2,env,sp);
	  }
	else if(command==quitwithchar)
	  {
	   return command_quitwithchar(command,c2,env,sp);
	  }
	else if(command==typeofchar)
	  {
	   return command_typeofchar(command,c2,env,sp);
	  }
    else if(command==endnewchar)// ָ���ջ,ר��
	  {
	   endnewpos.push(runtime_stack.size());
	   IntValue iv(0);
	   iv.value=endnewchar;
	   iv.type=TYPE::COMMAND;
	   runtime_stack.push_back(iv);
	   sp++;
	   return true;
	  }
	else if(command==trychar)
	  {
	   sp++;
	   return true;
	  }
	else if(command==quittrychar)
	  {
	   sp++;
	   return true;
	  }
	else if(command==catchchar)
	  {
	   sp++;
	   runtime_stack.pop_back();//catch(e)
	   int local_catch=1;
       while(true)
		    {
		     if(code[sp]==catchchar)
				local_catch++;
			 if(code[sp]==quitcatchchar)
				local_catch--;
			 sp++;
			 if(local_catch==0)
				break;
			}
	   return true;
	  }
	else if(command==quitcatchchar)
	  {
	   sp++;
	   return true;
	  }
	else if(command==finallychar)
	  {
	   sp++;
	   return true;
	  }
	else if(command==quitfinallychar)
	  {
	   sp++;
	   return true;
	  }
    //��Ŀ-----------------------------------------------------------

    if(runtime_stack.size()<2)
	  {
	   TError::ShowError(Error_internalerror,sp,c2,"");
	   return false;
	  }
	IntValue c1=runtime_stack[len-2];

    //˫Ŀ-----------------------------------------------------------
    // =,*=,-=,*=,/= %= <<= >>= >>>= &= |= ^=��ֵ,˫Ŀ
    if(command==setchar || command==pe || command==se ||
	   command==me || command==de || command==mode ||
	   command==lefte|| command==righte || command==rightee ||
	   command==ande || command==ore || command==ine)
	  {
	   return command_set(command,c1,c2,env,sp,code,codeenv);
	  }
    else if(command==pluschar || command==subchar || // +,-,*,/��
		    command==multichar || command== divchar || 
		    command==modchar)
	  {
       return command_operation(command,c1,c2,env,sp);
      }
    else if(command==leftchar || command==rightchar || command==rightrightchar ||//˫Ŀλ���� << >> >>> & | ^
		    command==andbchar || command==orbchar || command==inbchar)
	  {
	   return command_bitchar(command,c1,c2,env,sp);
	  }
	else if(command==gtchar || command==ltchar || command==gtechar || 
		   command==ltechar ||command==eqchar || command==noteqchar)//˫Ŀ > < >= <= == !=
	  {
	   return command_relation(command,c1,c2,env,sp);
	  }
    
    else if(command==btchar || command==bfchar || command==bf2char)//��תBT BF BF2,˫Ŀ
	  {
	   return command_btbfchar(command,c1,c2,env,sp);
	  }
	else if(command==offsetchar)// []
	  {
       //cout<<"enter offsetchar"<<endl;
       bool rv=command_offsetchar(command,c1,c2,env,sp);
       //cout<<"leave offsetchar"<<endl;
	   return rv;
	  }
	else if(command==dotchar)// .
	  {
	   return command_dotchar(command,c1,c2,env,sp);
	  }
    //˫Ŀ-----------------------------------------------------------
	
	else if(command==newchar)//��Ŀ
	  {
       bool rv=command_newchar(command,env,sp);
	   return rv;
	  }

    sp++;
	return true;
   }//end if ָ��
 else if(command>=INT_HMIN)//��ID
   {
    IntValue iv(command);
	iv.type=TYPE::ID;
    runtime_stack.push_back(iv);
	sp++;
   }
 else// command<INT_HMIN ��STR
   {
	IntValue iv(command);
	iv.type=TYPE::STR;
	runtime_stack.push_back(iv);
	sp++;
   }
 return true;
}
//-------------------------------------------------------------------

void Compiler::PrintValueInStack()
{
 for(int i=0;i<(int)runtime_stack.size();i++)
	{
	 switch(runtime_stack[i].type)
		   {
            case TYPE::NUMBER:
	             cout<<runtime_stack[i].value<<endl;
			     break;
	        case TYPE::ID:
		         cout<<GetNameByID(runtime_stack[i].value)<<endl;
			     break;
	        case TYPE::STR:
		         cout<<GetNameByID(runtime_stack[i].value)<<endl;
			     break;
	        case TYPE::HEAP:
		         cout<<runtime_stack[i].value<<endl;
			     break;
	        case TYPE::DATE:
		         cout<<runtime_stack[i].value<<endl;
			     break;
	        case TYPE::COBJ:
		         cout<<"COBJ"<<endl;
			     break;
	        case TYPE::LOBJ:
		         cout<<"LOBJ"<<endl;
			     break;
	        default:
		         cout<<"There is a unacceptable type in runtime stack"<<endl;
		         break;
		   }//end switch
	}//end for
}
//-------------------------------------------------------------------

int Compiler::GetStackSize()
{
 return runtime_stack.size();
}
//-------------------------------------------------------------------
void Compiler::PrintValueInTable()//��ʾ��������������
{
 vector<string> vec;
 vec.resize(20);
 vec[TYPE::VAR]="VAR";
 vec[TYPE::STRING]="STRING";
 vec[TYPE::INT]="INT";
 vec[TYPE::COMMAND]="COMMAND";
 vec[TYPE::HEAP]="HEAP";
 vec[TYPE::DATE]="DATE";
 vec[TYPE::COBJ]="COBJ";
 vec[TYPE::LOBJ]="LOBJ";
 cout<<endl<<"SymbolTable Size: "<<runtime_table.GetTableSize()<<endl;
 cout<<setw(16)<<"name"<<setw(16)<<"value"<<setw(8)<<"type"<<setw(8)<<"level"<<endl;
 vector<TItem2> res=runtime_table.GetOutput();
 for(int i=0;i<(int)res.size();i++)
	{
	 TItem2 v=res[i];
	 if(v.type!=TYPE::STRING)
	    cout<<setw(16)<<v.name<<setw(16)<<v.value
	    <<setw(8)<<vec[v.type]<<setw(8)<<v.level;
	 else
		cout<<setw(16)<<v.name<<setw(16)<<GetNameByID(v.value)
	    <<setw(8)<<vec[v.type]<<setw(8)<<v.level;
	 cout<<endl;
	}
}
//-------------------------------------------------------------------

void Compiler::PrintValueInHeap(bool order)
{
 cout<<endl<<"Heap Size: "<<runtime_heap.GetHeapSize()<<endl;
 runtime_heap.DisplayHeap(symbol_table,order);
}
//-------------------------------------------------------------------
//��Ŀ, int����
bool Compiler::command_inttype(const int& command,const IntValue& c2,const int& env,int& sp)
{
 sp++;
 string name=symbol_table[c2.value];
 TItem temp;
 temp.value=0;//��int���͵ĳ�ʼֵ
 temp.type=TYPE::INT;//���ű��������
 temp.level=env;//����
 if(runtime_table.AddSymbol(name,temp))
	return true;
 else
    TError::ShowError(Error_symbolinsameenv,sp-1,c2,name);
 return false;
}
//-------------------------------------------------------------------

bool Compiler::command_vartype(const int& command,const IntValue& c2,const int& env,int& sp)
{
 sp++;
 string name=symbol_table[c2.value];
 TItem temp;
 temp.value=0;//var���͵ĳ�ʼֵ,�����string����ָ���ַ
 temp.type=TYPE::VAR;
 temp.level=env;
 if(runtime_table.AddSymbol(name,temp))
	return true;
 else
    TError::ShowError(Error_symbolinsameenv,sp-1,c2,name);
 return false;
}
//-------------------------------------------------------------------
//i++,i--
bool Compiler::command_ippiss(const int& command,const IntValue& c2,const int& env,int& sp)//��Ŀ
{
 sp++;

 string c2name=symbol_table[c2.value];
 TItem* item;
 if(runtime_table.GetItemByName(c2name,&item))
   {
    if(item->type==TYPE::VAR||item->type==TYPE::INT)//ֻ��VAR����ִ��IPP�Ȳ���
	  {
	   item->type=TYPE::INT;
	   IntValue iv(0);
       //iv.type=TYPE::NUMBER;
	   iv.value=item->value;
	   runtime_stack.pop_back();
	   runtime_stack.push_back(iv);
	   if(command==ipp)//i++
          item->value=item->value+1;
       else//i--
          item->value=item->value-1;
	  }
	else if(item->type==TYPE::COBJ||item->type==TYPE::LOBJ)
	  {
       IntValue iv(0);
       runtime_stack.pop_back();
	   runtime_stack.push_back(iv);
	  }
	else
	  {
	   TError::ShowError(Error_stringnotsupportoperation,sp-1,c2,c2name);
	   return false;
	  }
   }
 else//������δ֪��DOM����
   {
    //TError::ShowError(Error_expectdefinition,sp-1,c2,c2name);
	//return false;
    IntValue iv(0);
    runtime_stack.pop_back();
	runtime_stack.push_back(iv);
   }
 return true;
}
//-------------------------------------------------------------------
//++i,--i
bool Compiler::command_ppissi(const int& command,const IntValue& c2,const int& env,int& sp)//��Ŀ
{
 sp++;

 string c2name=symbol_table[c2.value];
 TItem* item;
 if(runtime_table.GetItemByName(c2name,&item))
   {
	if(item->type==TYPE::VAR||item->type==TYPE::INT)
	  {
	   if(command==ppi)//i++
          item->value=item->value+1;
       else//i--
          item->value=item->value-1;

       item->type=TYPE::INT;

	   IntValue iv(0);
       //iv.type=TYPE::NUMBER;
	   iv.value=item->value;
	   runtime_stack.pop_back();
	   runtime_stack.push_back(iv);
	  }
	else if(item->type==TYPE::COBJ||item->type==TYPE::LOBJ)
	  {
	   IntValue iv(0);
	   runtime_stack.pop_back();
	   runtime_stack.push_back(iv);
	  }
	else
	  {
	   TError::ShowError(Error_stringnotsupportoperation,sp-1,c2,c2name);
	   return false;
	  }
   }//end runtime_table
 else//�����δ֪��DOM����
   {
    //TError::ShowError(Error_expectdefinition,sp-1,c2,c2name);
	//return false;
	IntValue iv(0);
	runtime_stack.pop_back();
	runtime_stack.push_back(iv);
   }
 return true;
}
//-------------------------------------------------------------------

bool Compiler::command_label(const int& command,const IntValue& c2,const int& env,int& sp)
{
 sp++;
 string c2name=symbol_table[c2.value];
 map<string,int>& labelmap=labellist.top();
 string labelname=c2name;
 int addr=sp;
 map<string,int>::iterator iter=labelmap.find(labelname);
 if(iter==labelmap.end())// �״γ���label
   {
	labelmap.insert(iter,make_pair(labelname,addr));
   }
 else//ͬһ��������,label�ظ�
   {
	TError::ShowError(Error_labelinsameenv,sp-1,c2,c2name);
	return false;
   }
 runtime_stack.pop_back();
 return true;
}
//-------------------------------------------------------------------
//˫Ŀ +,-,*,/,%
bool Compiler::command_operation(const int& command,const IntValue& c1,const IntValue& c2,
               const int& env,int& sp)
{
 /*if(command==pluschar)
   {
	TItem* it;
	if(runtime_table.GetItemByName(GetNameByID(c2.value),&it))
	  {
	   if(IsID(c1)==false)
	      cout<<c1.value<<"  "<<it->value<<endl;
	   else
		 {
		  cout<<GetNameByID(c1.value)<<"  "<<it->value<<endl;
		 }
	  }
	else
	  cout<<"nothing "<<GetNameByID(c2.value)<<endl;
   }*/
 sp++;//ָ��ǰ��
 int t1,t2;//�������Ĵ���
 string ts1,ts2;//�����ַ���+
 ts1="";ts2="";
 string c1name,c2name;

if(IsID(c1))
   {
    c1name=GetNameByID(c1.value);
	TItem* item;
	if(runtime_table.GetItemByName(c1name,&item))
	  {
       t1=item->value;
	   if(item->type==TYPE::STRING)
		  ts1=GetNameByID(t1);
	   else if(item->type==TYPE::COBJ/* || item->type==TYPE::LOBJ*/)//����1�����ܶ���
		  {
		   IntValue iv;
		   iv.type=item->type;
	       iv.value=0;
	       runtime_stack.pop_back();
	       runtime_stack.pop_back();
	       runtime_stack.push_back(iv);
	       return true;
		  }
       else if(item->type==TYPE::LOBJ)
          {
           ts1=GetNameByID(url_string_index);//changed by kobe, 2007,6,4
          }
	  }
	else if(!with_stack.empty())//����Ϊwith���������
	  {
	   IntValue iiv;
       IntValue wo=with_stack.top();
       if( sim->SimulateStatic(iiv,false,GetNameByID(with_stack.top().value),&wo,c1name,NULL,sp,env)==false )
		 {
		  TError::ShowError(Error_expectdefinition,sp-1,c1,c1name);
		  return false;
		 }
	   t1=iiv.value;
       if(iiv.type==TYPE::STR)
          ts1=GetNameByID(t1);
	   //�����̬��Ա�������string,��Ľ�
	  }
	else if(command!=pluschar)//�Ľ�,�����δ֪��DOM����
	  {
       //TError::ShowError(Error_stringnotsupportoperation,sp-1,c1,c1name);
	   //return false;
       IntValue iv;
	   if(IsLocation(c1name))
	      iv.type=TYPE::LOBJ;
	   else
		  iv.type=TYPE::COBJ;
	   iv.value=0;
	   runtime_stack.pop_back();
	   runtime_stack.pop_back();
	   runtime_stack.push_back(iv);
	   return true;
	  }
    else
	  ts1=c1name;
   }//end IsID
 else if(c1.type==TYPE::NUMBER)
   t1=c1.value;
 else if(c1.type==TYPE::STR)
   ts1=GetNameByID(c1.value);//���Ľ�,��Ϊ���ܻ���HEAP����
 else if(c1.type==TYPE::COBJ /*||c1.type==TYPE::LOBJ*/ )//������1�����ܶ���
   {
	IntValue iv;
	iv.type=c1.type;
	iv.value=0;
	runtime_stack.pop_back();
	runtime_stack.pop_back();
	runtime_stack.push_back(iv);
	return true;
   }
 else if(c1.type==TYPE::LOBJ)
   {
    ts1=GetNameByID(url_string_index);
   }
 else
   {
    TError::ShowError(Error_unknowntype,sp-1,c1,"");
    return false;
   }
 //----------------------------------------------
 if(IsID(c2))
   {
    //cout<<"XXXXX"<<endl;
    c2name=GetNameByID(c2.value);
	TItem* item;
	if(runtime_table.GetItemByName(c2name,&item))
	  {	   
       t2=item->value;
	   if(item->type==TYPE::STRING)
		  ts2=GetNameByID(t2);
	   else if(item->type==TYPE::COBJ /*|| item->type==TYPE::LOBJ*/)//����1�����ܶ���
		  {
		   IntValue iv;
		   iv.type=item->type;
	       iv.value=0;
	       runtime_stack.pop_back();
	       runtime_stack.pop_back();
	       runtime_stack.push_back(iv);
	       return true;
		  }
      else if(item->type==TYPE::LOBJ)
          {
           ts2=GetNameByID(url_string_index);//changed by kobe,2007,6,4
          }
	  }
	else if(with_stack.empty()==false)//����Ϊwith���������
	  {
	  
	   IntValue iiv;
       IntValue wo=with_stack.top();
       if( sim->SimulateStatic(iiv,false,GetNameByID(with_stack.top().value),&wo,c2name,NULL,sp,env)==false )
		 {
		  TError::ShowError(Error_expectdefinition,sp-1,c2,c2name);
		  return false;
		 }
	   t2=iiv.value;
	   if(iiv.type==TYPE::STR)
          ts2=GetNameByID(t2);
	   //�����̬��Ա�������string,��Ľ�
	  }
	else if(command!=pluschar)//�����δ֪��DOM����
	  {
       //TError::ShowError(Error_stringnotsupportoperation,sp-1,c2,c2name);
	   //return false;
	   IntValue iv;
	   if(IsLocation(c2name))
	      iv.type=TYPE::LOBJ;
	   else
		  iv.type=TYPE::COBJ;
	   iv.value=0;
	   runtime_stack.pop_back();
	   runtime_stack.pop_back();
	   runtime_stack.push_back(iv);
	   return true;
	  }
	else
      {
      //cout<<c2name<<endl;
	   ts2="*"+c2name;
      }
   }//end IsID
 else if(c2.type==TYPE::NUMBER)
   t2=c2.value;
 else if(c2.type==TYPE::STR)
   ts2=GetNameByID(c2.value);
 else if(c2.type==TYPE::COBJ/* || c2.type==TYPE::LOBJ*/)//changed by kobe, 2007.6.4
  {
   IntValue iv;
   iv.type=c2.type;
   iv.value=0;
   runtime_stack.pop_back();
   runtime_stack.pop_back();
   runtime_stack.push_back(iv);
   return true;
  }
 else if(c2.type==TYPE::LOBJ)
  {
   ts2=GetNameByID(url_string_index);
  }
 else
  {
   TError::ShowError(Error_unknowntype,sp-1,c2,"");
   return false;
  }

 if(ts1.empty() && ts2.empty())//��ȫ��������
   {
    IntValue iv(0);
    //iv.type=TYPE::NUMBER;
    if(command==pluschar)
	   iv.value=t1+t2;
    else if(command==subchar)
	   iv.value=t1-t2;
    else if(command==multichar)
	   iv.value=t1*t2;
    else if(command==divchar)
      {
	   if(t2==0)
	     {
	      TError::ShowError(Error_dividebyzero,sp-1,c2,c1name);
		  //�ݴ�
		  iv.value=t1;
	      //return false;
	     }
	   else
	      iv.value=t1/t2;
      }
    else if(command==modchar)
      {
	   if(t2==0)
		 {
		  TError::ShowError(Error_dividebyzero,sp-1,c2,c2name);
		  //�ݴ�
		  iv.value=t1%2;
		  //return false;
		 }
	   else
	      iv.value=t1%t2;
      }
	runtime_stack.pop_back();
    runtime_stack.pop_back();
    runtime_stack.push_back(iv);
    return true;
   }
 else if( (ts1.empty()==false || ts2.empty()==false) && command==pluschar)//�ַ���,����Ϊ+��
   {
	if(ts1.empty()==false && ts2.empty()==false)//�����ַ���+
	  {
	   int temp=current_string_index++;
       symbol_table[temp]=ts1+ts2.substr(1);//��Ϊ����*��ͷ,����ȥ��һ��*
       IntValue iv(0);
	   iv.type=TYPE::STR;
	   iv.value=temp;
	   runtime_stack.pop_back();
       runtime_stack.pop_back();
       runtime_stack.push_back(iv);
       return true;
	  }
    else//��1 or 2����������,��toString��ת��
	  {
	   if(ts1.empty())
	      ts1="*"+IntToStr(t1);
	   else
		  ts2="*"+IntToStr(t2);
	   int temp=current_string_index++;
	   symbol_table[temp]=ts1+ts2.substr(1);
	   IntValue iv(0);
	   iv.type=TYPE::STR;
	   iv.value=temp;
	   runtime_stack.pop_back();
	   runtime_stack.pop_back();
	   runtime_stack.push_back(iv);
	   return true;
	  }
   }
 if(!ts1.empty())
    TError::ShowError(Error_stringnotsupportoperation,sp-1,c1,c1name);
 else
	TError::ShowError(Error_stringnotsupportoperation,sp-1,c2,c2name);
 return false;
}
//-------------------------------------------------------------------
//λ�����
bool Compiler::command_bitchar(const int& command,const IntValue& c1,const IntValue& c2,const int& env,int& sp)
{
 sp++;//ָ��ǰ��
 int t1=0;
 int t2=0;

 string c1name;
 if(IsID(c1))
   {
    c1name=GetNameByID(c1.value);
	TItem* item;
	if(runtime_table.GetItemByName(c1name,&item))
	  {
       if(item->type==TYPE::INT)
	      t1=item->value;
	   else if(item->type==TYPE::COBJ || item->type==TYPE::LOBJ)
          {
          IntValue iv;
          iv.type=item->type;
          iv.value=0;
          runtime_stack.pop_back();
          runtime_stack.pop_back();
          runtime_stack.push_back(iv);
          return true;
          }
	   else 
		  {
		  TError::ShowError(Error_bitoperationneedrighttype,sp-1,c1,c1name);
		  return false;
		  }
	  }
	else if(!with_stack.empty())//����Ϊwith���������
	  {
	   IntValue iiv;
       IntValue wo=with_stack.top();
       if(sim->SimulateStatic(iiv,false,GetNameByID(with_stack.top().value),&wo,c1name,NULL,sp,env)==false )
		 {
		  TError::ShowError(Error_expectdefinition,sp-1,c1,c1name);
		  return false;
		 }
       if(iiv.type!=TYPE::STR)
          t1=iiv.value;
       else
          {
          cout<<"bit op is a string"<<endl;
          return false;
          }
	   //�����̬��Ա�������string,��Ľ�
	  }
    else//�����δ֪��DOM����
	  {
	   //TError::ShowError(Error_expectdefinition,sp-1,c1,c1name);
	   //return false;
	    IntValue iv;
        iv.type=TYPE::COBJ;
        iv.value=0;
        runtime_stack.pop_back();
        runtime_stack.pop_back();
        runtime_stack.push_back(iv);
        return true;
	  }
   }//end if IsID
 else if(c1.type==TYPE::NUMBER)
   t1=c1.value;
 else if(c1.type==TYPE::COBJ || c1.type==TYPE::LOBJ)
   {
	IntValue iv;
    iv.type=c1.type;
    iv.value=0;
    runtime_stack.pop_back();
    runtime_stack.pop_back();
    runtime_stack.push_back(iv);
    return true;
   }
 else
   {
	TError::ShowError(Error_bitoperationneedrighttype,sp-1,c1,c1name);
	return false;
   }
 //----------------------------------------------
 string c2name;
 if(IsID(c2))
   {
    c2name=GetNameByID(c2.value);
	TItem* item;
	if(runtime_table.GetItemByName(c2name,&item))
	  {
      if(item->type==TYPE::INT)
		 t2=item->value;
	   else if(item->type==TYPE::COBJ ||item->type==TYPE::LOBJ)
		 {
		  IntValue iv;
          iv.type=item->type;
          iv.value=0;
          runtime_stack.pop_back();
          runtime_stack.pop_back();
          runtime_stack.push_back(iv);
          return true;
		 }
	   else
		 {
		  TError::ShowError(Error_bitoperationneedrighttype,sp-1,c2,c2name);
		  return false;
		 }
	  }
	else if(!with_stack.empty())//����Ϊwith���������
	  {
	   IntValue iiv;
       IntValue wo=with_stack.top();
       if( sim->SimulateStatic(iiv,false,GetNameByID(with_stack.top().value),&wo,c2name,NULL,sp,env)==false )
		 {
		  TError::ShowError(Error_expectdefinition,sp-1,c2,c2name);
		  return false;
		 }
       if(iiv.type!=TYPE::STR)
	      t2=iiv.value;
       else
          {
          cout<<"bit op is a string"<<endl;
          return false;
          }
	   //�����̬��Ա�������string,��Ľ�
	  }
	else//�����δ֪��DOM����
	  {
	   //TError::ShowError(Error_expectdefinition,sp-1,c2,c2name);
	   //return false;
	    IntValue iv;
        iv.type=TYPE::COBJ;
        iv.value=0;
        runtime_stack.pop_back();
        runtime_stack.pop_back();
        runtime_stack.push_back(iv);
        return true;
	  }
   }//end if IsID
 else if(c2.type==TYPE::NUMBER)
   t2=c2.value;
 else if(c2.type==TYPE::COBJ || c2.type==TYPE::LOBJ)
  {
   IntValue iv;
   iv.type=c2.type;
   iv.value=0;
   runtime_stack.pop_back();
   runtime_stack.pop_back();
   runtime_stack.push_back(iv);
   return true;
  }
 //else if(c2.type==TYPE::LOBJ)
  //{
  //}
 else
  {
   TError::ShowError(Error_bitoperationneedrighttype,sp-1,c2,c2name);
   return false;
  }
 
 IntValue iv(0);
 if(command==leftchar)// <<
    iv.value=t1<<t2;
 else if(command==rightchar)// >>
    iv.value=t1>>t2;
 else if(command==rightrightchar)// >>> ���Ľ�
    iv.value=t1>>t2;
 else if(command==andbchar)// &
	iv.value=t1&t2;
 else if(command==orbchar)// |
	iv.value=t1|t2;
 else if(command==inbchar)// ^
	iv.value=t1^t2;
 
 runtime_stack.pop_back();
 runtime_stack.pop_back();
 runtime_stack.push_back(iv);
 return true;
}
//--------------------------------------------------------------------------------------------
//c1�Ƕ���Ԫ��
bool Compiler::command_heapset(const int& command,const IntValue& c1,const IntValue& c2,const int& env,int& sp)
{
 sp++;
 int t2;//�ڶ���������
 int t2type=TYPE::NUMBER;
 string ts2;
 string c2name;
 int gettype=GetIVType(c2);
 if(gettype==TYPE::ID)
   {
	c2name=GetNameByID(c2.value);
    TItem* item2;
	if(runtime_table.GetItemByName(c2name,&item2))//�Ǳ���
	  { 
	   t2=item2->value;
	   t2type=item2->type;
	   //if(item2->type==TYPE::INT)
          //t2type=TYPE::NUMBER;
	   if(t2type==TYPE::STRING)
		 {
		  ts2=GetNameByID(t2);
		  t2type=TYPE::STR;
		 }
	   else if(t2type==TYPE::INT)//�Ľ�,���b=new Array(2);a[2]=b
		 {
		  t2type=TYPE::NUMBER;
		 }
	  }
	else if(!with_stack.empty())//����Ϊwith���������
	  {
	   IntValue iiv;
       IntValue wo=with_stack.top();
       if( sim->SimulateStatic(iiv,false,GetNameByID(with_stack.top().value),&wo,c2name,NULL,sp,env)==false )
		 {
		  TError::ShowError(Error_expectdefinition,sp-1,c2,c2name);
		  return false;
		 }
	   t2=iiv.value;
       if(iiv.type==TYPE::STR)
          ts2=GetNameByID(t2);
	   //�����̬��Ա�������string,��Ľ�
	  }
	else//�����δ֪��DOM����
	  {
       //TError::ShowError(Error_expectdefinition,sp-1,c2,c2name);
	   //return false;
	   IntValue iv;
	   if(IsLocation(c2name))
          iv.type=TYPE::LOBJ;
	   else
		  iv.type=TYPE::COBJ;
       iv.value=0;
       runtime_stack.pop_back();
       runtime_stack.pop_back();
       runtime_stack.push_back(iv);
       return true;
	  }
   }//end IsID
 else if(gettype==TYPE::STR)
   {
	t2=c2.value;
	t2type=TYPE::STR;
	ts2=GetNameByID(t2);
   }
 else if(gettype==TYPE::NUMBER)
   {
   t2=c2.value;
   t2type=TYPE::NUMBER;
   }
 else if(gettype==TYPE::HEAP)
   {
	t2=c2.value;
	t2type=TYPE::HEAP;
   }
 else if(gettype==TYPE::DATE)
   {
	t2=c2.value;
	t2type=TYPE::DATE;
   }
 else if(gettype==TYPE::COBJ || TYPE::LOBJ)
   {
	t2=c2.value;
	t2type=gettype;
   }
 else
   {
	cout<<"Fatal Error4"<<endl;
	return false;
   }

if(t2type==TYPE::COBJ || t2type==TYPE::LOBJ)
  {
   //(*(c1.hp)).value=0;
   //(*(c1.hp)).type=t2type;
   IntValue* hitem=runtime_heap.GetHeapItem(c1.hp.x,c1.hp.y,sp-1);
   if(hitem==NULL)
	 {
	  cout<<"heap access error"<<endl;
	  return false;
	 }
   hitem->value=0;
   hitem->type=t2type;

   runtime_stack.pop_back();
   IntValue iv(1);
   runtime_stack.push_back(iv);
   return true;
  }
IntValue* hitem=runtime_heap.GetHeapItem(c1.hp.x,c1.hp.y,sp-1);
if(hitem==NULL)
  {
   cout<<"heap access error"<<endl;
   return false;
  }
if(command==setchar)// =
  {
   hitem->value=t2;
  }
else if(command==se)//-=
  {
  //(*(c1.hp)).value-=t2;
   hitem->value-=t2;
  }
else if(command==me)//*=
  {
  //(*(c1.hp)).value*=t2;
   hitem->value*=t2;
  }
else if(command==de)// /=
  {
   if(t2==0)
	 {
	  TError::ShowError(Error_dividebyzero,sp-1,c2,"");
	  //�ݴ�
	  //return false;
	 }
   else
      hitem->value/=t2;
  }
else if(command==mode)// %=
  {
   if(t2==0)
	 {
	  TError::ShowError(Error_dividebyzero,sp-1,c2,"");
	  hitem->value%=2;
	  //return false;
	 }
   else
      hitem->value%=t2;
  }
else if(command==lefte)// <<=
  {
   hitem->value<<=t2;
  }
else if(command==righte)// >>=
  {
   hitem->value>>=t2;
  }
else if(command==rightee)// >>>=
  {
   //(*(c1.hp)).value>>=t2;//���Ľ�
   hitem->value>>=t2;
  }
else if(command==ande)// &=
  {
   hitem->value&=t2;
  }
else if(command==ore)// |=
  {
   hitem->value|=t2;
  }
else if(command==ine)// ^=
  {
   hitem->value^=t2;
  }

hitem->type=t2type;

if(command==pe)//+=
  {
   if( hitem->type ==TYPE::STR)
	 {
	  string ts1=GetNameByID( hitem->value );
	  if(ts2.empty())
		 ts2="*"+IntToStr(t2);
	  int temp=current_string_index++;
	  symbol_table[temp]=ts1+ts2.substr(1);
      //(*(c1.hp)).value=temp;
	  //(*(c1.hp)).type=TYPE::STR;
	  hitem->value=temp;
	  hitem->type=TYPE::STR;
	 }
   else if(ts2.empty())
	 {
      //(*(c1.hp)).value+=t2;
      //(*(c1.hp)).type=t2type;
	  hitem->value+=t2;
	  hitem->type=t2type;
	 }
   else
	 {
	  TError::ShowError(Error_stringnotsupportoperation,sp-1,c1,GetNameByID(c1.value));
	  return false;
	 }
  }

 runtime_stack.pop_back();
 IntValue iv(1);
 //iv.type=TYPE::NUMBER;
 runtime_stack.push_back(iv);
 return true;
}
//--------------------------------------------------------------------------------------------
//����=,+=,-=,*=,/=
bool Compiler::command_set(const int& command,const IntValue& c1,const IntValue& c2,const int& env,int& sp,
                           vector<int>& code,vector<int>& codeenv)
{
 //IntValue cc1=c1;
 if(c1.HasHP())//c1�Ƕ��ڶ���,����heapset
   {
    return command_heapset(command,c1,c2,env,sp);
   }
 sp++;
 if(c1.type==TYPE::NUMBER)//c1������ID
   {
    TError::ShowError(Error_setvaluetonumber,sp-1,c1,"");
	PrintValueInStack();
	return false;
   }
 
 //����window.onload=f�������
 if(c1.type==TYPE::COBJ && c1.value==-999999 && command==setchar && c2.type==TYPE::ID)
   {
   string name=GetNameByID(c2.value);
   int temp1,temp2;
   int newp=GetFunctionAddr(name,temp1,temp2);
   if(newp!=-1)
	 {
	  code.pop_back();
	  codeenv.pop_back();
	  code.push_back(c2.value);
	  codeenv.push_back(0);
	  code.push_back(0);
	  codeenv.push_back(0);
	  code.push_back(eachar);
	  codeenv.push_back(0);
      code.push_back(codeend);
	  codeenv.push_back(0);
	 }
   IntValue iv(1);
   runtime_stack.pop_back();
   runtime_stack.pop_back();
   runtime_stack.push_back(iv);
   return true;
   }
 
 string c1name=GetNameByID(c1.value);

 TItem* item1;
 if(runtime_table.GetItemByName(c1name,&item1)==false)//��δ����
   {
    if(command==setchar)//��ֵ,��������������
	  {
       TItem temp;
	   temp.type=TYPE::VAR;
	   temp.value=0;
	   temp.level=env;
	   runtime_table.AddSymbol(c1name,temp);//��̨����,javascript����
	   runtime_table.GetItemByName(c1name,&item1);
	  }
    else
	  {
	   TError::ShowError(Error_expectdefinition,sp-1,c1,c1name);
	   return false;
	  }
   }
 //----------------------------------------------
 int t2;//�ڶ���������
 int t2type;
 string ts2;

 int gettype=GetIVType(c2);
 switch(gettype)
	   {
        case TYPE::ID:
             {
	         string c2name=GetNameByID(c2.value);
             TItem* item2;
	         if(runtime_table.GetItemByName(c2name,&item2))//�Ǳ���
	           { 
	            t2=item2->value;
	            t2type=item2->type;
	            if(t2type==TYPE::STRING)
	               ts2=GetNameByID(t2);
	           }
	         else if(!with_stack.empty())//����Ϊwith���������
	           {
	            IntValue iiv;
                IntValue wo=with_stack.top();
                if(sim->SimulateStatic(iiv,false,GetNameByID(with_stack.top().value),&wo,c2name,NULL,sp,env)==false )
		          {
		           TError::ShowError(Error_expectdefinition,sp-1,c2,c2name);
		           return false;
		          }
	            t2=iiv.value;
	            t2type=iiv.type;
                if(t2type==TYPE::STR)
                  {
                   t2type=TYPE::STRING;
                   ts2=GetNameByID(t2);
                  }
	            //�����̬��Ա�������string,��Ľ�
	           }
	         else//�����δ֪��DOM����
	           {
	           t2=0;
	           if(IsLocation(c2name))
	              t2type=TYPE::LOBJ;
	           else
		          t2type=TYPE::COBJ;
	           }
			 break;
             }//end TYPE::ID
        case TYPE::STR:
             {
	         t2=c2.value;
	         t2type=TYPE::STRING;
	         ts2=GetNameByID(t2);
			 break;
             }
        case TYPE::NUMBER:
             {
             t2=c2.value;
             t2type=TYPE::INT;
			 break;
             }
        case TYPE::HEAP://a=new Array(3);
             {
             t2=c2.value;
	         t2type=TYPE::HEAP;
			 break;
             }
        case TYPE::DATE:
             {
             t2=c2.value;
	         t2type=TYPE::DATE;
			 break;
             }
        case TYPE::COBJ:
		case TYPE::LOBJ:
             {
	         t2=c2.value;
	         t2type=gettype;
			 break;
             }
        default:
             {
             cout<<"Fatal Error5! "<<gettype<<" "<<sp-1<<endl;
	         return false;
             }
	   }//end switch
//-----------------------------------------------

//���=�����LOBJ,��׽
if(item1->type==TYPE::LOBJ || c1name=="location" || c1.type==TYPE::LOBJ)
  {
   if(ts2.empty())
     {
	  ts2=UNKNOWNSITE;
      if(t2type==TYPE::LOBJ)
         ts2=GetNameByID(url_string_index);
     }
   cout<<"JUMP!!! "<<ts2.substr(1)<<endl;
   AddOneResult("JUMP",ts2.substr(1));

   IntValue iv(1);
   runtime_stack.pop_back();
   runtime_stack.pop_back();
   runtime_stack.push_back(iv);
   return true;
  }

/*if(item1->type==TYPE::COBJ && item1->value==-999999)
  {
   IntValue iv(1);
   runtime_stack.pop_back();
   runtime_stack.pop_back();
   runtime_stack.push_back(iv);
   return true;
  }*/
//�������ֵΪ���ܶ���
if(t2type==TYPE::COBJ || t2type==TYPE::LOBJ)
  {
   item1->value=0;
   item1->type=t2type;
   IntValue iv(1);
   runtime_stack.pop_back();
   runtime_stack.pop_back();
   runtime_stack.push_back(iv);
   return true;
  }

if(command==setchar)// =
  {
   bool selfset=false;
   if(item1->value==t2 && t2type==TYPE::HEAP && item1->type==TYPE::HEAP)//�����Ը�ֵ
	  selfset=true;
   if(item1->value==t2 && t2type==TYPE::DATE && item1->type==TYPE::DATE)
	  selfset=true;
   if(item1->type==TYPE::HEAP || item1->type==TYPE::DATE)//�����Ը�ֵ
	 {
	  if(!selfset)
		 runtime_heap.DeleteHeapItem(item1->value);
	 }
   if(ts2.empty())
	 {
	  item1->value=t2;
      item1->type=t2type;
	 }
   else
	 {
	  item1->value=t2;
	  item1->type=TYPE::STRING;
	 }
   if( (t2type==TYPE::HEAP||t2type==TYPE::DATE )&& !selfset)//���ü���,�����õ�HEAP���ڶ���,�������ü���
      runtime_heap.AddReferenceCount(t2);
  }

else if(command==pe)// +=  ���Ľ�
   {
	if(item1->type==TYPE::STRING)
	  {
	   string ts1=GetNameByID(item1->value);
	   if(ts2.empty())
		  ts2="*"+IntToStr(t2);
	   int temp=current_string_index++;
	   symbol_table[temp]=ts1+ts2.substr(1);
	   item1->value=temp;
	   item1->type=TYPE::STRING;
      }
	else if(item1->type==TYPE::HEAP || item1->type==TYPE::DATE)
	  {
	   TError::ShowError(Error_objectnotsupportoperation,sp-1,c1,c1name);
       return false;
	  }
	else if(ts2.empty())
	  {
	   item1->value+=t2;
	   item1->type=t2type;
	  }
	else
	 {
	  TError::ShowError(Error_stringnotsupportoperation,sp-1,c1,c1name);
	  return false;
	 }
   }
//-------------------------------------------------------------------
else if(t2type==TYPE::STRING)
   {
    TError::ShowError(Error_stringnotsupportoperation,sp-1,c1,c1name);
    return false;
   }
else if(command==se)// -=
   {
    item1->value-=t2;
	item1->type=t2type;
   }
else if(command==me)// *=
   {
    item1->value*=t2;
	item1->type=t2type;
   }
else if(command==de)// /=
   {
    if(t2!=0)
	  {
       item1->value/=t2;
	   item1->type=t2type;
	  }
	else
	  {
	   TError::ShowError(Error_dividebyzero,sp-1,c2,"");
	   item1->type=t2type;
	   //return false;
	  }
   }
else if(command==mode)// %=
   {
	if(t2==0)
	  {
	   TError::ShowError(Error_dividebyzero,sp-1,c2,"");
	   item1->value%=2;
	   item1->type=t2type;
	   //return false;
	  }
	else
	  {
	   item1->value%=t2;
	   item1->type=t2type;
	  }
   }
else if(command==lefte)// <<=
   {
	item1->value<<=t2;
	item1->type=t2type;
   }
else if(command==righte)// >>=
   {
	item1->value>>=t2;
	item1->type=t2type;
   }
else if(command==rightee)// >>>=
   {
	item1->value>>=t2;//���Ľ�
	item1->type=t2type;
   }
else if(command==ande)// &=
   {
	item1->value&=t2;
	item1->type=t2type;
   }
else if(command==ore)// |=
   {
	item1->value|=t2;
	item1->type=t2type;
   }
else if(command==ine)// ^=
   {
	item1->value^=t2;
    item1->type=t2type;
   }

IntValue iv(1);
//if(item1->type==TYPE::STRING)
   //iv.type=TYPE::VAR;
//else
//iv.type=TYPE::NUMBER;

runtime_stack.pop_back();
runtime_stack.pop_back();
runtime_stack.push_back(iv);//��ֵ�൱��1,if(i=1)....
//if(c1name=="temp")
   //cout<<"temp type:"<<item1->type<<endl;
return true;
}
//-------------------------------------------------------------------

bool Compiler::command_relation(const int& command,const IntValue& c1,
     const IntValue& c2,const int& env,int& sp)
{
 sp++;//ָ��ǰ��
 int t1,t2;
 string ts1,ts2;
 int gettype=GetIVType(c1);
 if(gettype==TYPE::ID)
   {
    string c1name=GetNameByID(c1.value);
	TItem* item;
	if(runtime_table.GetItemByName(c1name,&item))
	  {
       t1=item->value;
	   if(item->type==TYPE::STRING)
		  ts1=GetNameByID(t1);
	   else if(item->type==TYPE::COBJ || item->type==TYPE::LOBJ)//�������ܶ���
		  {
		   IntValue iv(0);
		   runtime_stack.pop_back();
           runtime_stack.pop_back();
           runtime_stack.push_back(iv);
           return true;
		  }
	  }
	else if(!with_stack.empty())//����Ϊwith���������
	  {
	   IntValue iiv;
       IntValue wo=with_stack.top();
       if( sim->SimulateStatic(iiv,false,GetNameByID(with_stack.top().value),&wo,c1name,NULL,sp,env)==false )
		 {
		  TError::ShowError(Error_expectdefinition,sp-1,c1,c1name);
		  return false;
		 }
	   t1=iiv.value;
    
       if(iiv.value==TYPE::STR)
          ts2=GetNameByID(t1);
	   //�����̬��Ա�������string,��Ľ�
	  }
	else//��һ����������δ֪��DOM����
	  {
       //TError::ShowError(Error_expectdefinition,sp-1,c1,c1name);
	   //return false;
	   IntValue iv(0);
	   runtime_stack.pop_back();
       runtime_stack.pop_back();
       runtime_stack.push_back(iv);
       return true;
	  }
   }//end IsID
 else if(gettype==TYPE::STR)
   {
	ts1=GetNameByID(c1.value);
   }
 else if(gettype==TYPE::NUMBER)
   {
    t1=c1.value;
   }
 else if(gettype==TYPE::COBJ || gettype==TYPE::LOBJ)
   {
	IntValue iv(0);
	runtime_stack.pop_back();
    runtime_stack.pop_back();
    runtime_stack.push_back(iv);
    return true;
   }
 else
   {
	cout<<sp<<"Fatal Error"<<c1.type<<endl;
	return false;
   }
 //----------------------------------------------
 gettype=GetIVType(c2);
 if(gettype==TYPE::ID)
   {
    string c2name=GetNameByID(c2.value);
	TItem* item;
	if(runtime_table.GetItemByName(c2name,&item))
	  {
       t2=item->value;
	   if(item->type==TYPE::STRING)
		  ts2=GetNameByID(t2);
	   else if(item->type==TYPE::COBJ || item->type==TYPE::LOBJ)
		  {
		  IntValue iv(0);
		  runtime_stack.pop_back();
          runtime_stack.pop_back();
          runtime_stack.push_back(iv);
          return true;
		  }
	  }
	else if(!with_stack.empty())//����Ϊwith���������
	  {
	   IntValue iiv;
       IntValue wo=with_stack.top();
       if( sim->SimulateStatic(iiv,false,GetNameByID(with_stack.top().value),&wo,c2name,NULL,sp,env)==false )
		 {
		  TError::ShowError(Error_expectdefinition,sp-1,c2,c2name);
		  return false;
		 }
	   t2=iiv.value;
       if(iiv.type==TYPE::STR)
          ts2=GetNameByID(t2);
	   //�����̬��Ա�������string,��Ľ�
	  }
	else//���������2��δ֪��DOM����
	  {
       //TError::ShowError(Error_expectdefinition,sp-1,c2,c2name);
	   //return false;
	   IntValue iv(0);
	   runtime_stack.pop_back();
       runtime_stack.pop_back();
       runtime_stack.push_back(iv);
       return true;
	  }
   }//end is id
 else if(gettype==TYPE::STR)
   {
	ts2=GetNameByID(c2.value);
   }
 else if(gettype==TYPE::NUMBER)
   t2=c2.value;
 else if(gettype==TYPE::COBJ || gettype==TYPE::LOBJ)
   {
	IntValue iv(0);
    runtime_stack.pop_back();
    runtime_stack.pop_back();
    runtime_stack.push_back(iv);
    return true;
   }
 else
   {
	cout<<"Fatal Error7"<<endl;
	return false;
   }

 IntValue iv(0);
 //iv.type=TYPE::NUMBER;
 if(command==gtchar)// >
   {
	if(ts1.empty()==false && ts2.empty()==false)//�ַ����Ƚ�
	   iv.value=(ts1>ts2?1:0);
	else
	   iv.value=(t1>t2?1:0);
   }
 else if(command==ltchar)// <
   {
	if(ts1.empty()==false && ts2.empty()==false)
	  iv.value=(ts1<ts2?1:0);
	else
      iv.value=(t1<t2?1:0);
   }
 else if(command==gtechar)// >=
   {
	if(ts1.empty()==false && ts2.empty()==false)
	   iv.value=(ts1>=ts2?1:0);
	else
       iv.value=(t1>=t2?1:0);
   }
 else if(command==ltechar)// <=
   {
	if(ts1.empty()==false && ts2.empty()==false)
	   iv.value=(ts1<=ts2?1:0);
	else
       iv.value=(t1<=t2?1:0);
   }
 else if(command==eqchar)// ==
   {
	if(ts1.empty()==false && ts2.empty()==false)
	   iv.value=(ts1==ts2?1:0);
	else
	  {
	   iv.value=(t1==t2?1:0);
	  }
   }
 else if(command==noteqchar)// !=
   {
	if(ts1.empty()==false && ts2.empty()==false)
	   iv.value=(ts1!=ts2?1:0);
	else
	iv.value=(t1!=t2?1:0);
   }
 
 runtime_stack.pop_back();
 runtime_stack.pop_back();
 runtime_stack.push_back(iv);
 return true;
}
//-------------------------------------------------------------------

bool Compiler::command_negoppchar(const int& command,const IntValue& c2,const int& env,int& sp)//- ! ��Ŀ
{
 sp++;
 int t;
 int gettype=GetIVType(c2);
 if(gettype==TYPE::ID)
   {
	string name=GetNameByID(c2.value);
	TItem* item;
	if(runtime_table.GetItemByName(name,&item))
	  {
       t=item->value;
	   if(item->type==TYPE::STRING)//�Ľ�Ϊ�ж�String�Ƿ�Ϊ��
		 {
		  //TError::ShowError(Error_stringnotsupportoperation,sp-1,c2,name);
		  //return false;
		  string str=GetNameByID(item->value);
          IntValue iv(0);
		  if(str.size()<=1)
			 iv.value=1;
          runtime_stack.pop_back();
          runtime_stack.push_back(iv);
          return true;
		 }
	   else if(item->type==TYPE::COBJ || item->type==TYPE::LOBJ)
		 {
		  IntValue iv(0);
          runtime_stack.pop_back();
          runtime_stack.push_back(iv);
          return true;
		 }
	  }
	else if(!with_stack.empty())//����Ϊwith���������
	  {
	   IntValue iiv;
       IntValue wo=with_stack.top();
       if( sim->SimulateStatic(iiv,false,GetNameByID(with_stack.top().value),&wo,name,NULL,sp,env)==false )
		 {
		  TError::ShowError(Error_expectdefinition,sp-1,c2,name);
		  return false;
		 }
	   t=iiv.value;
       if(iiv.type==TYPE::STR)
         {
          IntValue iv(0);
          string str=GetNameByID(iiv.value);
          if(str.size()<=1)
             iv.value=1;
          runtime_stack.pop_back();
          runtime_stack.push_back(iv);
          return true;
         }
	   //�����̬��Ա�������string,��Ľ�
	  }
    else//�����δ֪��DOM��������
	  {
	   //TError::ShowError(Error_expectdefinition,sp-1,c2,name);
	   //return false;
	   IntValue iv(0);
       runtime_stack.pop_back();
       runtime_stack.push_back(iv);
       return true;
	  }
   }//end IsID
 else if(gettype==TYPE::STR)//��Ϊ�ж�String�Ƿ�Ϊ��
   {
	//TError::ShowError(Error_stringnotsupportoperation,sp-1,c2,GetNameByID(c2.value));
	//return false;
	IntValue iv(0);
	string str=GetNameByID(c2.value);
	if(str.size()<=1)
	   iv.value=1;
	runtime_stack.pop_back();
    runtime_stack.push_back(iv);
	return true;
   }
 else if(gettype==TYPE::NUMBER)
   t=c2.value;
 else if(gettype==TYPE::COBJ || gettype==TYPE::LOBJ)
   {
    IntValue iv(0);
    runtime_stack.pop_back();
    runtime_stack.push_back(iv);
    return true;
   }
 else
   {
	cout<<"Fatal Error8"<<endl;
	return false;
   }

 IntValue iv(0);
 //iv.type=TYPE::NUMBER;
 if(command==negchar)// -
    iv.value=-1*t;
 else if(command==oppchar)// !
	iv.value= (t!=0)?0:1;
 runtime_stack.pop_back();
 runtime_stack.push_back(iv);
 return true;
}
//-------------------------------------------------------------------

bool Compiler::command_btbfchar(const int& command,const IntValue& c1,
     const IntValue& c2,const int& env,int& sp)//˫Ŀ
{
 int t1=0;
 int t2=sp+1;
 int gettype=GetIVType(c1);
 if(gettype==TYPE::ID)
   {
    string c1name=GetNameByID(c1.value);
	TItem* item;
	if(runtime_table.GetItemByName(c1name,&item))
       t1=item->value;
	else//����ж�δ֪��DOM����
	  {
       //TError::ShowError(Error_expectdefinition,sp-1,c1,c1name);
	   //return false;
	   t1=0;
	  }
   }
 else if(gettype==TYPE::STR)
   {
	t1=1;
   }
 else if(gettype==TYPE::NUMBER)
   t1=c1.value;
 else//������һ�ɷ��� 1
   t1=1;
 
 string c2name;
 gettype=GetIVType(c2);
 if(gettype==TYPE::ID)
   {
    c2name=GetNameByID(c2.value);
	TItem* item;
	if(runtime_table.GetItemByName(c2name,&item))
       t2=item->value;
	else
	  {
       TError::ShowError(Error_expectdefinition,sp-1,c2,c2name);
	   return false;
	  }
   }
 else if(gettype==TYPE::NUMBER)
   t2=c2.value;
 else
   {
	TError::ShowError(Error_internalerror,sp-1,c2,c2name);
	return false;
   }

 if(command!=bf2char)
   {
    if(runtime_stack.size()>1)
      {
       runtime_stack.pop_back();
       runtime_stack.pop_back();
      }
   else
     {
      TError::ShowError(Error_internalerror,sp-1,c2,c2name);
      return false;
     }
   }
 else//BF2ֻ��ջһ��,switchר��(����)
   {
    if(runtime_stack.empty()==false)
       runtime_stack.pop_back();
    else
       {
       TError::ShowError(Error_internalerror,sp-1,c2,c2name);
       return false;
       }
   }

 if(command==bfchar || command==bf2char)
   {
    if(!t1)
      {
       sp=t2;//������ת
       if(AddToJumpmap(t2)==false)
	  return false;
      }
    else
      sp++;
   }
 else//btchar
   {
    if(t1!=0)
          {
           sp=t2;
           if(AddToJumpmap(t2)==false)
              return false;
          }
        else
           sp++;
   }
 return true;
}
//-------------------------------------------------------------------

bool Compiler::command_blchar(const int& command,const IntValue& c2,const int& env,int& sp)//��Ŀ
{
 int t;
 string name;
 int gettype=GetIVType(c2);
 if(gettype==TYPE::ID)
   {
    name=GetNameByID(c2.value);//���ܸĽ�
	TItem* item;
	if(runtime_table.GetItemByName(name,&item))//���ű���
       t=item->value;
	else
	  {
	   map<string,int>& labelmap=labellist.top();
	   map<string,int>::iterator iter=labelmap.find(name);
	   if(iter==labelmap.end())
		 {
          TError::ShowError(Error_expectdefinition,sp-1,c2,name);
	      return false;
		 }
	   else//�ҵ�
		 {
		  t=(*iter).second;
		 }
	  }//end else
   }//end IsID
 else if(gettype==TYPE::NUMBER)
   t=c2.value;
 else
   {
	TError::ShowError(Error_internalerror,sp-1,c2,name);
	return false;
   }
 runtime_stack.pop_back();
 sp=t;//��ת
 if(AddToJumpmap(t)==false)
	return false;
 return true;
}
//-------------------------------------------------------------------

bool Compiler::AddToJumpmap(const int& addr)
{
 map<int,int>::iterator iter;
 iter=jumpmap.find(addr);
 if(iter==jumpmap.end())//δ�ҵ�,�����µ�
   jumpmap.insert(iter,make_pair(addr,1));
 else//�ҵ�
   {
   iter->second++;
   if(iter->second>LOOP_MAX)
	 {
	  cout<<"Bad Javascript Code!!!"<<endl;
	  jump_result+="BAD\n";
	  return false;
	 }
   }
 return true;
}
//-------------------------------------------------------------------

int Compiler::GetFunctionAddr(const string& funcname,int& treelevel,int& codelevel)
{
 /*map<string,int>::iterator iter=functionmap.find(funcname);
 if(iter==functionmap.end())
    return -1;
 return iter->second;*/
 FNTreeNode *fnt;
 int addr=functiontree.GetFunctionAddrByName(funcname,&fnt);
 if(addr!=-1)//���ڸú���
   {
	functiontree.EnterActiveNode(*fnt);
    treelevel=fnt->tree_level;
	codelevel=fnt->code_level;
    //cout<<"��������:"<<level;
   }
 return addr;
}
//-------------------------------------------------------------------

bool Compiler::command_eachar(const int& command,const IntValue& c2,const int& env,int& sp)//��Ŀ
{
 int t;
 if(IsID(c2))
   {
	TError::ShowError(Error_argumentsnumbernotmatch,sp-1,c2,"");
	return false;
   }
 t=c2.value;//������Ŀ
 int ret=sp+1;//�����ķ��ص�ַ
 
 runtime_stack.pop_back();//ȥ����EA NUM
 vector<IntValue> argumentslist;// ������
 argumentslist.reserve(t);

 for(int i=0;i<t;i++)
	{
     IntValue iv=runtime_stack.back();
	    if(IsID(iv))//������ʱ,ΪNUMBER��Ҫ���Ʋ���
	      {
		   string name=GetNameByID(iv.value);
           TItem* item;
	       if(runtime_table.GetItemByName(name,&item))
		     {
			  switch(item->type)
				    {
		             case TYPE::INT://����������ʱ
			              {
			              iv.type=TYPE::NUMBER;
			              iv.value=item->value;
						  break;
			              }
		             case TYPE::STRING://������STRINGʱ
			              {
			              iv.type=TYPE::STR;
			              iv.value=item->value;
						  break;
			              }
		             case TYPE::HEAP:
			              {
			              iv.type=TYPE::HEAP;
			              iv.value=item->value;
						  break;
			              }
			         case TYPE::DATE:
				          {
				          iv.type=TYPE::DATE;
				          iv.value=item->value;
						  break;
				          }
			         case TYPE::COBJ:
					 case TYPE::LOBJ:
					 case TYPE::VAR:
				          {
				          iv.type=item->type;
				          iv.value=0;
						  break;
				          }
		             default:
			              {
			              cout<<"Fatal Error! "<<name<<endl;
			              return false;
			              }
					}//end switch
		     }//end if GetItemByName
		   else if(!with_stack.empty())//����Ϊwith���������
	         {
	          IntValue iiv;
              IntValue wo=with_stack.top();
              if(sim->SimulateStatic(iiv,false,GetNameByID(with_stack.top().value),&wo,name,NULL,sp,env)==false )
		        {
		         TError::ShowError(Error_expectdefinition,sp-1,iv,name);
		         return false;
		        }
	          iv.value=iiv.value;
			  iv.type=iiv.type;
	          //�����̬��Ա�������string,��Ľ�
	         }
	       else//����Ĳ�������һ��δ֪��DOM����
		     {
		      //TError::ShowError(Error_expectdefinition,sp-1,c2,name);
		      //return false;
			  iv.value=0;
			  if(IsLocation(name))
				 iv.type=TYPE::LOBJ;
			  else
				 iv.type=TYPE::COBJ;
		     }
		  }//end if IsID

	 argumentslist.insert(argumentslist.begin(),iv);
	 runtime_stack.pop_back();
	}//end for i

 //cout<<"���� "<<argumentslist[1].value<<" "<<argumentslist[2].value<<endl;
 IntValue func=runtime_stack.back();//������
 
 runtime_stack.pop_back();

 if(IsID(func)==false)
   {
	TError::ShowError(Error_functionnameisnotid,sp-1,func,"");
	return false;
   }
 int bp=runtime_stack.size();//������������ʼ��ַ
 if(func.value!=dotchar)// ����һ�㺯��
   {
	string name=GetNameByID(func.value);//������
	//-------------------------------------------
    int gfn_pos=-1;
	if((gfn_pos=sim->IsGlobalFunction(name))>=0)//������ȫ�ֺ���
	  {
	   sp++;
	   IntValue iv;
	   bool isreturn=true;
	   if(sim->SimulateGlobalFunction(gfn_pos,iv,name,&argumentslist,sp,env,isreturn)==false)
		  return false;
	   if(isreturn)
          runtime_stack.push_back(iv);
	   return true;
	  }//end global function
	//-------------------------------------------
	int tlevel=0;
	int clevel=0;
	int newsp=GetFunctionAddr(name,tlevel,clevel);
	if( (tlevel!=0||clevel!=0) && newsp!=-1)//�Ǻ����еĺ���
	  {
       /*
	   ���Ľ�,��֧�ֺ���Ƕ�׵���
	   ��Ҫ������̬��
	   */
	   //sp=newsp;
	   sp++;
	   IntValue iv(0);
	   iv.type=TYPE::COBJ;
	   runtime_stack.push_back(iv);
	   return true;
	  }
    //-------------------------------------------
	if(newsp==-1)//����ȫ�ֺ���
	  {
	   if(with_stack.empty())//�Ľ�,�п�����δ֪��ȫ�ֺ���
		 {
	      //TError::ShowError(Error_coundfindfuncname,sp,c2,name);
	      //return false;
		  sp++;
		  IntValue iv(0);
		  iv.type=TYPE::COBJ;
		  runtime_stack.push_back(iv);
		  return true;
		 }
	   //��with��,˵���ǵ��ó�Ա����
	   sp++;
	   IntValue iv;
	   bool isreturn;
	   if(function_reflection(iv,with_stack.top(),func,argumentslist,env,sp,isreturn)==false)
		 {
		  return false;
		 }
	   if(isreturn)
	      runtime_stack.push_back(iv);
	   return true;
	  }
    //-------------------------------------------
	IntValue iv0(ret,true);//NUM
	function_stack.push(iv0);
	IntValue iv1(retchar,false);// RET
	function_stack.push(iv1);
	IntValue iv2(bp,true);//NUM
	function_stack.push(iv2);
	IntValue iv3(bpchar,false);// BP
	function_stack.push(iv3);
	for(int i=argumentslist.size()-1;i>=0;i--)//�������������ջ
	   {
		function_stack.push(argumentslist[i]);
	   }
	sp=newsp;//SP��ת
	enter_function();
	return true;
   }//end if !=dotchar
 else
   {
	TError::ShowError(Error_fatalerror,sp-1,c2,"");
	return false;
   }
 return true;
}
//-------------------------------------------------------------------

bool Compiler::command_eachar2(const int& command,const IntValue& c2,const int& env,int& sp)//��Ŀ
{
 sp++;
 int t;
 if(IsID(c2))
   {
	TError::ShowError(Error_argumentsnumbernotmatch,sp-1,c2,"");
	return false;
   }
 t=c2.value;//������Ŀ
 runtime_stack.pop_back();//ȥ����EA NUM
 vector<IntValue> argumentslist;// ������
 argumentslist.reserve(t);
 
 for(int i=0;i<t;i++)
	{
     IntValue iv=runtime_stack.back();
	 if(IsID(iv))//������ʱ,ΪNUMBER��Ҫ���Ʋ���
	   {
		string name=GetNameByID(iv.value);
        TItem* item;
	    if(runtime_table.GetItemByName(name,&item))
		  {
		   if(item->type==TYPE::INT)//����������ʱ
			 {
			  iv.type=TYPE::NUMBER;
			  iv.value=item->value;
			 }
		   else if(item->type==TYPE::STRING)//������STRINGʱ
			 {
			  iv.type=TYPE::STR;
			  iv.value=item->value;
			 }
		   else if(item->type==TYPE::HEAP)
			 {
			  iv.type=TYPE::HEAP;
			  iv.value=item->value;
			 }
		   else if(item->type==TYPE::DATE)
			 {
			  iv.type=TYPE::DATE;
			  iv.value=item->value;
			 }
		   else if(item->type==TYPE::COBJ || item->type==TYPE::LOBJ)
			 {
              iv.type=item->type;
			  iv.value=0;
			 }
		   else//���Ľ�,������ε�ʱ���DOM������
			 {
			  cout<<"Fatal Error "<<item->type<<endl;
			  return false;
			 }
		  }
		else if(!with_stack.empty())//����Ϊwith���������
	      {
	       IntValue iiv;
           IntValue wo=with_stack.top();
           if(sim->SimulateStatic(iiv,false,GetNameByID(with_stack.top().value),&wo,name,NULL,sp,env)==false )
		     {
		      TError::ShowError(Error_expectdefinition,sp-1,iv,name);
		      return false;
		     }
	       iv.value=iiv.value;
		   iv.type=iiv.type;
	       //�����̬��Ա�������string,��Ľ�
	      }
	    else//�����δ֪��DOM����
		  {
		   //TError::ShowError(Error_expectdefinition,sp-1,c2,name);
		   //return false;
		   iv.value=0;
		   if(IsLocation(name))
			  iv.type=TYPE::LOBJ;
		   else
			  iv.type=TYPE::COBJ;
		  }
	   }//end if IsID
	 argumentslist.insert(argumentslist.begin(),iv);
	 runtime_stack.pop_back();
	}//end for i
 //ȡ������
 IntValue func=runtime_stack.back();//������
 runtime_stack.pop_back();
 //ȡ������
 IntValue object=runtime_stack.back();//������
 runtime_stack.pop_back();
 
 if(IsID(func)==false)
   {
	TError::ShowError(Error_functionnameisnotid,sp-1,func,"");
	return false;
   }
 //����IDҲ����,����s.toString().toUpperCase();
 /*if(IsID(object)==false)
   {
	TError::ShowError(Error_objectnameisnotid,sp-1,object,"");
	return false;
   }
 */
 
 IntValue result;
 bool isreturn;
 if(function_reflection(result,object,func,argumentslist,env,sp-1,isreturn)==false)
   {
    return false;
   }
 if(isreturn)
    runtime_stack.push_back(result);
 return true;
}
//-------------------------------------------------------------------

bool Compiler::function_reflection(IntValue& iv,const IntValue& object,const IntValue& func,
                                   vector<IntValue>& argumentslist,const int& env,const int sp,
								   bool& isreturn)
{
 string objectname,classname,funcname;
 isreturn=true;
 objectname=GetNameByID(object.value);
 if( objectname.empty())
   {
	if( object.type==TYPE::ID )
	  {
	   TError::ShowError(Error_objectnameisnotid,sp,object,"");
	   return false;
	  }
	if(object.type==TYPE::STR)
	  {
	   iv.type=TYPE::COBJ;
	   iv.value=0;
	   return true;
	  }
   }
 if(objectname=="Math" || objectname=="window")//����Math�ľ�̬�ĺ���
   {
	if(sim->SimulateStatic(iv,true,objectname,NULL,GetNameByID(func.value),&argumentslist,sp,env))
	   return true;
	return false;
   }
 TItem* object_item=NULL;
 if(object.type!=TYPE::ID || runtime_table.GetItemByName(objectname,&object_item)==false)
	object_item=NULL;

 //if(object_item==NULL)
    //cout<<"object_item is null"<<endl;
 if(object_item)
   {
    switch(object_item->type)
	   {
		case TYPE::STRING:
		    {
			 if(sim->SimulateString(iv,true,object_item,objectname,GetNameByID(func.value),
			    &argumentslist,sp,env,object_item->value)==false)
			    return false;
			 break;
			}
		case TYPE::HEAP:
		    {
			 bool ir=true;
			 if(sim->SimulateArray(iv,true,object_item,object,GetNameByID(func.value),
			    &argumentslist,sp,env,ir)==false)
			    return false;
			 isreturn=ir;
			 break;
			}
		case TYPE::DATE:
		    {
			 if(sim->SimulateDate(iv,true,object_item,object,GetNameByID(func.value),
			    &argumentslist,sp,env)==false)
				return false;
			 break;
			}
		case TYPE::INT:
		    {
			 if(sim->SimulateInt(iv,true,object_item,object,GetNameByID(func.value),
			    &argumentslist,sp,env)==false)
			    return false;
			 break;
			}
		case TYPE::COBJ:
		    {
			 funcname=GetNameByID(func.value);
             iv.value=0;
             iv.type=TYPE::COBJ;
	         if(funcname=="open")
	           {
		        if(meet_jump_func(argumentslist,sp,env,true)==false)
		           return false;
		        isreturn=true;
	           }
			 break;
			}
		case TYPE::LOBJ:
		    {
			 funcname=GetNameByID(func.value);
             iv.value=0;
             iv.type=TYPE::LOBJ;
             if(funcname=="replace")
	           {
		        if(meet_jump_func(argumentslist,sp,env)==false)
		           return false;
		        isreturn=true;
	           }
			 else if(funcname=="reload")
				isreturn=false;
		     break;
			}
		default:
			 break;
	   }
   }//end if object_item!=NULL
 else if(object.type==TYPE::ID)
	{
     funcname=GetNameByID(func.value);
	 if(IsLocation(objectname))
	   {
        iv.value=0;
        iv.type=TYPE::LOBJ;
        if(funcname=="replace")
          {
		   if(meet_jump_func(argumentslist,sp,env)==false)
		      return false;
          }
	   }
	 else
	   {
	    if(funcname=="open")
		  {
		   if(meet_jump_func(argumentslist,sp,env,true)==false)
			  return false;
		   iv.type=TYPE::COBJ;
		   iv.value=0;
		  }
		else
		  {
		   iv.value=0;
		   iv.type=TYPE::COBJ;
		  }
	   }//end else
	}//end else if object.type==TYPE::ID
 else if(object.type==TYPE::STR)
	{
     if(sim->SimulateString(iv,true,NULL,objectname,GetNameByID(func.value),
		                    &argumentslist,sp,env,object.value)==false)
		return false;
	}
 else if(object.type==TYPE::NUMBER)
	{
	 if(sim->SimulateInt(iv,true,NULL,object,GetNameByID(func.value),&argumentslist,sp,env)==false)
	    return false;
	}
 else if(object.type==TYPE::HEAP)
	{
     bool ir=true;
	 if(sim->SimulateArray(iv,true,NULL,object,GetNameByID(func.value),&argumentslist,sp,env,ir)==false)
	    return false;
	 isreturn=ir;
	}
 else if(object.type==TYPE::DATE)
	{
	 if(sim->SimulateDate(iv,true,NULL,object,GetNameByID(func.value),&argumentslist,sp,env)==false)
	    return false;
	}
 else if(object.type==TYPE::COBJ)//���Ľ�
	{
     funcname=GetNameByID(func.value);
     iv.value=0;
	 if(funcname=="open")
	   {
		if(meet_jump_func(argumentslist,sp,env,true)==false)
		   return false;
		isreturn=true;
		iv.type=TYPE::COBJ;
	   }
	 else
	    iv.type=TYPE::COBJ;
	}
 else if(object.type==TYPE::LOBJ)
	{
	 funcname=GetNameByID(func.value);
     if(funcname=="replace")
	   {
		if(meet_jump_func(argumentslist,sp,env))
		  {
		   isreturn=true;
		   iv.type=TYPE::LOBJ;
           iv.value=0;
		  }
		else
		   return false;
	   }
	 //else if(funcname=="reload")
	   //isreturn=false;
	 //else
	 iv.type=TYPE::LOBJ;
	}
 else
	{
	 cout<<"error "<<objectname<<endl;
	 return false;
	}
 return true;
}
//-------------------------------------------------------------------

bool Compiler::meet_jump_func(vector<IntValue>& argumentslist,const int& env,const int sp,bool isopen)
{
 if(argumentslist.empty())
   {
	return true;
   }
 string url=UNKNOWNSITE;
 IntValue& ag1=argumentslist[0];
 if(ag1.type==TYPE::STR)
    url=GetNameByID(ag1.value);
 else if(ag1.type==TYPE::ID)
	{
	 string name=GetNameByID(ag1.value);
	 TItem* item;
	 if(runtime_table.GetItemByName(name,&item)==false)
	   {
		TError::ShowError(Error_expectdefinition,sp-1,ag1,name);
		return false;
	   }
	 if(item->type==TYPE::STRING)
		url=GetNameByID(item->value);
	}//end if ID
 else if(ag1.type==TYPE::NUMBER)
	{
	 TError::ShowError(Error_argumentstypenotmatch,sp-1,ag1,"");
	 return false;
	}
 if(!isopen)
   {
    cout<<"JUMP!!! "<<url.substr(1)<<endl;
	AddOneResult("JUMP",url.substr(1));
   }
 else
   {
	cout<<"POP!!! "<<url.substr(1)<<endl;
	AddOneResult("POP",url.substr(1));
   }
 return true;
}
//--------------------------------------------------------------------------------------------

bool Compiler::command_dotchar(const int& command,const IntValue& c1,const IntValue& c2,const int& env,int& sp)
{
 sp++;
 IntValue iv(0);
 if(c1.type==TYPE::NUMBER)
   {
	//TError::ShowError(Error_objectnameisnotid,sp-1,c1,"");
	/*
	columns02[i].chance;
	��iԽ��ʱ,����NUMBER
	*/
	//return false;
	iv.type=TYPE::COBJ;
	runtime_stack.pop_back();
    runtime_stack.pop_back();
    runtime_stack.push_back(iv);
    return true;
   }
 
 if(c2.type==TYPE::NUMBER)
   {
	TError::ShowError(Error_functionnameisnotid,sp-1,c2,"");
	return false;
   }
 
 string objectname=GetNameByID(c1.value);
 string attrname=GetNameByID(c2.value);

 TItem* item1;
 if(objectname=="window" && attrname=="onload")
   {
    iv.type=TYPE::COBJ;
    iv.value=-999999;
    runtime_stack.pop_back();
    runtime_stack.pop_back();
    runtime_stack.push_back(iv);
    return true;
   }
 if(objectname=="Number" || objectname=="Math" || objectname=="window" || 
	objectname=="location" || objectname=="document")//���þ�̬����
   {
	item1=NULL;
	if( sim->SimulateStatic(iv,false,objectname,NULL,attrname,NULL,sp,env)==false )
	   return false;
	goto finish;
   }
 
 if(c1.type!=TYPE::ID || runtime_table.GetItemByName(objectname,&item1)==false)//�Ľ�
   {
	item1=NULL;
   }

 if(item1)//�Ǿ�̬��Ա
   {
    switch(item1->type)
	      {
           case TYPE::HEAP:
		       {
			    bool ir=true;
				if(sim->SimulateArray(iv,false,item1,c1,attrname,NULL,sp,env,ir)==false)
				   return false;
		        break;
			   }
		   case TYPE::STRING:
		       {
			    if(sim->SimulateString(iv,false,item1,objectname,attrname,NULL,sp,env,item1->value)==false)
				   return false;
			    break;
			   }
		   case TYPE::INT:
			   {
			    TError::ShowError(Error_integernothaveanyattribute,sp,iv,"");
			    return false;
			   }
		   case TYPE::COBJ:
			   {
			    iv.type=TYPE::COBJ;
				break;
			   }
		   case TYPE::LOBJ:
			   {
                iv.value=0;
                if(attrname=="host" || attrname=="hostname")
                  {
                   iv.type=TYPE::STR;
                   iv.value=host_string_index;
                  }
                else if(attrname=="pathname")
                  {
                   iv.type=TYPE::STR;
                   iv.value=path_string_index;
                  }
                else
			       iv.type=TYPE::LOBJ;
				break;
			   }
		   default://�Ľ�,�ݴ�
			   iv.type=TYPE::COBJ;
		       break;
          }
   }//end if item1
 else if(c1.type==TYPE::STR)
    {
     if(sim->SimulateString(iv,false,item1,objectname,attrname,NULL,sp,env,c1.value)==false)
		return false;
	}
 else if(c1.type==TYPE::HEAP)
    {
     bool ir=true;
     if(sim->SimulateArray(iv,false,item1,c1,attrname,NULL,sp,env,ir)==false)
        return false;
    }
 else if(c1.type==TYPE::ID)//������DOM����
	{
	 iv.value=0;
	 if(IsLocation(objectname))
	   {
        if(attrname=="host" || attrname=="hostname")
          {
           iv.type=TYPE::STR;
           iv.value=host_string_index;
          }
        else if(attrname=="pathname")
          {
           iv.type=TYPE::STR;
           iv.value=path_string_index;
          }
        else
		  iv.type=TYPE::LOBJ;
       }
     else if(IsLocation(attrname))
       {
        iv.type=TYPE::LOBJ;
       }
	 else 
	   {
		iv.type=TYPE::COBJ;
	   }
	}//end if TYPE::ID
 else if(c1.type==TYPE::LOBJ)
	{
     if(attrname=="host" || attrname=="hostname")
       {
        iv.type=TYPE::STR;
        iv.value=host_string_index;
       }
     else if(attrname=="pathname")
       {
        iv.type=TYPE::STR;
        iv.value=path_string_index;
       }
     else
       {
	    iv.value=0;
	    iv.type=TYPE::LOBJ;
       }
	}
 else if(c1.type==TYPE::COBJ)
	{
	 iv.value=0;
	 if(IsLocation(attrname))
		iv.type=TYPE::LOBJ;
	 else
		iv.type=TYPE::COBJ;
	}
 else
	{
	 TError::ShowError(Error_unknowntype,sp-1,c1,"");
	 return false;
	}
 preobjname=attrname;//��¼���һ��name
finish:
 runtime_stack.pop_back();
 runtime_stack.pop_back();
 runtime_stack.push_back(iv);
 return true;
}
//--------------------------------------------------------------------------------------------

void Compiler::enter_function()//��������ջ,�·��ű���ջ
{
 map<string,int> labelmap;
 labellist.push(labelmap);
 runtime_table.EnterFunc();
}
//--------------------------------------------------------------------------------------------

void Compiler::quit_function()//�������ջ,�ɷ��ű��ջ
{
 labellist.pop();
 runtime_table.QuitFunc();
}
//--------------------------------------------------------------------------------------------

bool Compiler::command_functionchar(const int& command,const IntValue& c2,const int& env,int& sp)//��Ŀ
{
 sp++;
 /*if(IsID(c2))
   {
	string name=GetNameByID(c2.value);
	int addr=sp;
    map<string,int>::iterator iter=functionmap.find(name);
	if(iter==functionmap.end())
	  {
	   functionmap.insert(iter,make_pair(name,addr));
	  }
	else
	  {
	   TError::ShowError(Error_functionsameenv,sp-1,c2,name);
	   return false;
	  }
   }
 else
   {
    TError::ShowError(Error_functionnameisnotid,sp-1,c2,"");
	return false;
   }*/
 runtime_stack.pop_back();
 infunction++;
 return true;
}

//--------------------------------------------------------------------------------------------

bool Compiler::command_unfunctionchar(const int& command,const IntValue& c2,const int& env,int& sp)
{
 sp++;
 inunfunction++;
 return true;
}
//--------------------------------------------------------------------------------------------

bool Compiler::command_unefchar(const int& command,const IntValue& c2,const int& env,int& sp)
{
 sp++;
 inunfunction--;
 if(inunfunction==0)
   {
	IntValue iv(0);
	iv.type=TYPE::COBJ;
	runtime_stack.push_back(iv);
   }
 return true;
}
//--------------------------------------------------------------------------------------------

bool Compiler::command_inichar(const int& command,const IntValue& c2,const int& env,int& sp)
{
 //cout<<"INE env:"<<env<<endl;
 //������,ע�⴫��ʱ��ĸ���
 sp++;
 if(IsID(c2)==false)
   {
    TError::ShowError(Error_setvaluetonumber,sp-1,c2,"");
	return false;
   }
 string c1name=GetNameByID(c2.value);
 
 TItem* item1;//��ǰ���������������
 TItem temp;
 temp.type=TYPE::VAR;
 temp.value=0;
 temp.level=env;
 runtime_table.AddSymbol(c1name,temp);//��̨����,javascript����
 runtime_table.GetItemByName(c1name,&item1);

 IntValue c3=function_stack.top();
 function_stack.pop();

 TItem* item2;
 //----------------------------------------------
 /*
 js�﷨�Ĳ��淶��,���õĲ�����Ŀ���ԱȺ���ʵ����Ŀ��
 ��ô����ĺ���ʵ������ΪTYPE::VAR
 */
 if(c3.value==bpchar)
   {
	//TError::ShowError(Error_argumentsnumbernotmatch,sp-1,c2,"");
	//return false;
	function_stack.push(c3);//��bpchar�Ż�
	item1->value=0;
	item1->type=TYPE::VAR;
    runtime_stack.pop_back();
	return true;
   }
 //----------------------------------------------
 int t2;
 int t2type=TYPE::VAR;
 int gettype=GetIVType(c3);

 switch(gettype)
	   {
	    case TYPE::ID:
		     {
	         string c2name=GetNameByID(c3.value);
	         if(runtime_table.GetItemByName(c2name,&item2))//�Ǳ���
	           { 
	            t2=item2->value;
	            t2type=item2->type;
	           }
	         else//�Ľ�,����������δ֪��DOM����
	           {
                //TError::ShowError(Error_expectdefinition,sp-1,c3,c2name);
	            //return false;
				t2=0;
				if(IsLocation(c2name))
				   t2type=TYPE::LOBJ;
				else
				   t2type=TYPE::COBJ;
	           }
			 break;
             }
        case TYPE::STR:
             {
             t2=c3.value;
	         t2type=TYPE::STRING;
			 break;
             }
        case TYPE::NUMBER:
             {
             t2=c3.value;
             t2type=TYPE::INT;
			 break;
             }
        case TYPE::HEAP:
             {
	         t2=c3.value;
	         t2type=TYPE::HEAP;
			 break;
             }
        case TYPE::DATE:
             {
	         t2=c3.value;
	         t2type=TYPE::DATE;
			 break;
             }
        case TYPE::COBJ:
             {
              t2=0;
              t2type=TYPE::COBJ;
              break;
             }
		case TYPE::LOBJ:
             {
              t2=0;
              t2type=TYPE::LOBJ;
              break;
             }
		case TYPE::VAR:
		     {
		     t2=0;
			 t2type=gettype;
			 break;
			 }
       default:
             {
	         cout<<"Fatal Error13"<<gettype<<endl;
	         return false;
             }
	  }//end switch
 
 item1->value=t2;
 item1->type=t2type;
 if(t2type==TYPE::HEAP || t2type==TYPE::DATE)//���ü���,�����õ�HEAP���ڶ���,�������ü���
    runtime_heap.AddReferenceCount(t2);

 runtime_stack.pop_back();
 return true;
}
//--------------------------------------------------------------------------------------------

bool Compiler::command_return(const int& command,const IntValue& c2,const int& env,int& sp)
{
 if(function_stack.empty())
   {
	TError::ShowError(Error_stackfail,sp-1,c2,"");
	return false;
   }
 IntValue bp=function_stack.top();
 IntValue c22=c2;
 function_stack.pop();
 //----------------------------------------------
 /*
 js�﷨�Ĳ��淶��,���ϸ��麯������
 ���ô���Ĳ����������ԱȺ���ԭ�Ͷ�
 */
 while(bp.value!=bpchar)
   {
	if(function_stack.empty())
	  {
	   TError::ShowError(Error_stackfail,sp-1,c22,"");
	   PrintValueInStack();
	   return false;
	  }
	bp=function_stack.top();
	function_stack.pop();
   }
 //----------------------------------------------
 if(function_stack.empty())
   {
	TError::ShowError(Error_stackfail,sp-1,c2,"");
	return false;
   }
 bp=function_stack.top();
 function_stack.pop();
 int addr=bp.value;
 runtime_stack.erase(runtime_stack.begin()+addr,runtime_stack.end());//��ջ
 
 if(command==returnchar)
   {
	if(c2.type==TYPE::NUMBER) //ֵ����,ֱ�ӽ�ջ
	  {
	   runtime_stack.push_back(c22);//return ��������ջ
	  }
	else if(c2.type==TYPE::ID)
	  {
	   int temp=temp_id_index;
       
	   string c2name=GetNameByID(c22.value);
	   TItem* item1;
	   
       if(runtime_table.GetItemByName(c2name,&item1))
         {
          TItem tempt;
	      tempt.type=item1->type;
	      tempt.value=item1->value;
	      tempt.level=0;
	      runtime_table.AddTempSymbol(tempt);

          IntValue iv(0);
	      iv.type=TYPE::ID;
	      iv.value=temp;
          runtime_stack.push_back(iv);
		 }
	   else if(!with_stack.empty())//����Ϊwith���������
	     {
	      IntValue iiv;
          IntValue wo=with_stack.top();
          if(sim->SimulateStatic(iiv,false,GetNameByID(with_stack.top().value),&wo,c2name,NULL,sp,env)==false )
		    {
		     TError::ShowError(Error_expectdefinition,sp-1,c22,c2name);
		     return false;
		    }
	      if(iiv.type==TYPE::NUMBER)
			 runtime_stack.push_back(iiv);
		  else if(iiv.type==TYPE::STR)//���Ľ�,��������
			{
			 TItem tempt;
	         tempt.type=TYPE::STRING;
	         tempt.value=iiv.value;
	         tempt.level=0;
	         runtime_table.AddTempSymbol(tempt);

			 IntValue iv(0);
	         iv.type=TYPE::ID;
	         iv.value=temp_id_index;
	         runtime_stack.push_back(iv);
            }
          else
            {
             runtime_stack.push_back(iiv);
            }
		 }//�����̬��Ա�������string,��Ľ�
	  else//��δ����(�Ľ�,������ص���δ֪��DOM����)
		{
		 //TError::ShowError(Error_expectdefinition,sp-1,c22,c2name);
		 //return false;
		 TItem tempt;
		 if(IsLocation(c2name))
	        tempt.type=TYPE::LOBJ;
		 else
			tempt.type=TYPE::COBJ;
	     tempt.value=0;
	     tempt.level=0;
	     runtime_table.AddTempSymbol(tempt);
         IntValue iv(0);
	     iv.type=TYPE::ID;
	     iv.value=temp_id_index;
	     runtime_stack.push_back(iv);
		}
	  }//end IsID
    else if(c2.type==TYPE::STR || c2.type==TYPE::HEAP||c2.type==TYPE::DATE)// return new Array(3)
	  {
	   //string c2name=GetNameByID(c22.value);
	   TItem tempt;
	   if(c2.type==TYPE::STR)
	      tempt.type=TYPE::STRING;
	   else if(c2.type==TYPE::HEAP)
		  tempt.type=TYPE::HEAP;
       else
		  tempt.type=TYPE::DATE;
	   tempt.value=c22.value;
	   tempt.level=0;
	   runtime_table.AddTempSymbol(tempt);

	   IntValue iv(0);
	   iv.type=TYPE::ID;
	   iv.value=temp_id_index;
	   runtime_stack.push_back(iv);
	  }
	else if(c2.type==TYPE::COBJ || c2.type==TYPE::LOBJ)
	  {
	   TItem tempt;
	   tempt.type=c2.type;
	   tempt.value=0;
	   tempt.level=0;
	   runtime_table.AddTempSymbol(tempt);

	   IntValue iv(0);
	   iv.type=TYPE::ID;
	   iv.value=temp_id_index;
	   runtime_stack.push_back(iv);
	  }
	else
	  {
	   cout<<"Fatal Error14"<<endl;
	   return false;
	  }
   }//== returnchar
 else if(command==efchar)//���Ľ�,�����޷���ֵʱ�費��Ҫ����ֵ
   {
	IntValue iv(0);
	iv.type=TYPE::COBJ;
	runtime_stack.push_back(iv);
   }
 if(function_stack.empty())
   {
	TError::ShowError(Error_stackfail,sp-1,c2,"");
	return false;
   }
 IntValue ret=function_stack.top();
 function_stack.pop();
 if(ret.value!=retchar)
   {
	TError::ShowError(Error_stackfail,sp-1,c22,"");
	PrintValueInStack();
	return false;
   }
 if(function_stack.empty())
   {
	TError::ShowError(Error_stackfail,sp-1,c2,"");
	return false;
   }
 ret=function_stack.top();//sp�µ�ַ
 function_stack.pop();
 sp=ret.value;
 //PrintValueInStack();
 quit_function();
 functiontree.QuitActiveNode();
 return true;
}
//--------------------------------------------------------------------------------------------

bool Compiler::command_newchar(const int& command,const int& env,int& sp)
{
 sp++;
 vector<IntValue> temp;
 for(int i=endnewpos.top();i<(int)(runtime_stack.size());i++)//�˴����Ƕ��new��bug
	 temp.push_back(runtime_stack[i]);
 runtime_stack.erase(runtime_stack.begin()+endnewpos.top(),runtime_stack.end());//��������
 if(temp[0].value!=endnewchar)//temp[0]����ENEW
   {
	TError::ShowError(Error_internalerror,sp-1,temp[0],"");
	return false;
   }

if(IsID(temp[1]))//temp[1]����ID��ӦClassname
  {
   string classname=GetNameByID(temp[1].value);
   //------------------------------------------------------------------------------------
   if(classname=="Array")
	 {
      //cout<<"enter Array"<<endl;
	  int index;
	  vector<IntValue>& vec=runtime_heap.GetNew(index);
	  if(temp.size()>2)//�в�Array���캯��
		{
		 if(temp.size()==3)//�����Ҳ���Ϊ����ID��ֵΪ��,�ε������Ƿ��������С
		   {
			if(temp[2].type==TYPE::NUMBER)
			  {
               if(temp[2].value<0)
                 {
                  TError::ShowError(Error_arraysizebelowzero,sp-1,temp[2],"");
                  return false;
                 }
			   vec.resize(temp[2].value);
			  }
			//---------------------------------------------
			else if(temp[2].type==TYPE::ID)
			   {
			    string name=GetNameByID(temp[2].value);
				TItem* item;
				if(runtime_table.GetItemByName(name,&item))
				  {
					if(item->type==TYPE::INT)
                      {
                       if(item->value<0)
                         {
                          TError::ShowError(Error_arraysizebelowzero,sp-1,temp[2],"");
                          return false;
                         }
					   vec.resize(item->value);
                      }
					else if(item->type==TYPE::STRING)
					  {
					   IntValue iv(item->value);
					   iv.type=TYPE::STR;
                       vec.push_back(iv);
					  }
					else//����չ
					  {
					   /*IntValue iv(item->value);
					   iv.type=item->type;
					   vec.push_back(temp[2]);*/
					   vec.resize(DEFAULT_ARRAY_SIZE);
					  }
				  }//end GetItemByName
				else if(!with_stack.empty())//����Ϊwith���������
	              {
	               IntValue iiv;
                   IntValue wo=with_stack.top();
                   if(sim->SimulateStatic(iiv,false,GetNameByID(with_stack.top().value),&wo,name,NULL,sp,env)==false )
		             {
		              TError::ShowError(Error_expectdefinition,sp-1,temp[2],name);
		              return false;
		             }
                   if(iiv.type==TYPE::STR)
                     {
                      //string s=GetNameByID(iiv.value);
                      vec.push_back(iiv);
                     }
                   else
	                  vec.resize(iiv.value);
	               //�����̬��Ա�������string,��Ľ�
	              }
				else//�Ľ�,���������δ֪��DOM����
				  {
				   //TError::ShowError(Error_expectdefinition,sp-1,temp[2],name);
				   //return false;
				   vec.resize(DEFAULT_ARRAY_SIZE);
				  }
			   }//end ID
			//---------------------------------------------
			else if(temp[2].type==TYPE::STR)//����,����Ϊһ���ַ���
			   {
                vec.push_back(temp[2]);
			   }
			else if(temp[2].type==TYPE::COBJ || temp[2].type==TYPE::LOBJ)
			   {
				vec.resize(DEFAULT_ARRAY_SIZE);
			   }
			else
			   {
				cout<<"string in []"<<endl;
				return false;
			   }
		   }//end ����
		 else//���,ʾΪһά����,��ÿ������Ϊ�����ʼ��ֵ
		   {
			for(int i=2;i<int(temp.size());i++)//������,����ID
			   {
                if(temp[i].type==TYPE::ID)
				  {
				   string name=GetNameByID(temp[i].value);
				   TItem* item;
				   if(runtime_table.GetItemByName(name,&item))
					 {
					  IntValue iv(item->value);
					  if(item->type==TYPE::INT)
						 vec.push_back(iv);
					  else if(item->type==TYPE::STRING)
						{
						 iv.type=TYPE::STR;
						 vec.push_back(iv);
						}
					  else//����չ
						{
						 /*iv.type=item->type;
						 vec.push_back(iv);*/
						 vec.resize(DEFAULT_ARRAY_SIZE);
						 break;
						}
					 }//end GetItemByName
				   else if(!with_stack.empty())//����Ϊwith���������
	                 {
	                  IntValue iiv;
                      IntValue wo=with_stack.top();
                      if(sim->SimulateStatic(iiv,false,GetNameByID(with_stack.top().value),&wo,name,NULL,sp,env)
						  ==false )
		                {
		                 TError::ShowError(Error_expectdefinition,sp-1,temp[i],name);
		                 return false;
		                }
	                  vec.push_back(iiv);
	                  //�����̬��Ա�������string,��Ľ�
	                 }
				   else//�Ľ�,������������һ��δ֪��DOM����
					 {
					  //TError::ShowError(Error_expectdefinition,sp-1,temp[i],name);
				      //return false;
					  IntValue iv(0);
					  vec.push_back(iv);
					 }
				  }//end if ID
				else
				   vec.push_back(temp[i]);
			   }
		   }//end else ���
		}//end �в�
	  else//�޲�Array���캯��
		{
		 vec.resize(DEFAULT_ARRAY_SIZE);//�շ��������ռ�
		}
	  IntValue iv(0);
	  iv.value=index;
	  iv.type=TYPE::HEAP;
	  runtime_stack.push_back(iv);

	  endnewpos.pop();
      //cout<<"leave Array "<<sp<<endl;
	  return true;
	 }//end if Array
   //------------------------------------------------------------------------------------
   else if(classname=="String")
	 {
	  IntValue iv(0);
	  iv.type=TYPE::STR;
	  if(temp.size()==3)//����
		{
		 if(temp[2].type==TYPE::STR)
		   {
			iv.value=temp[2].value;
		   }
		 else if(temp[2].type==TYPE::COBJ || temp[2].type==TYPE::LOBJ)
		   {
			iv.value=empty_string_index;
		   }
		 else
		   {
			TError::ShowError(Error_constructoragruments,sp-1,temp[1],"String");
		    return false;
		   }
		}//end if temp.size()==3
      else if(temp.size()<3)//�޲�
		{
		 //int ti=current_string_index++;
	     //symbol_table[ti]="*";
	     iv.value=empty_string_index;
		}
	  else//���캯����������
		{
		 TError::ShowError(Error_constructoragruments,sp-1,temp[1],"String");
		 return false;
		}
	  runtime_stack.push_back(iv);
	 }//end if String
   //------------------------------------------------------------------------------------
   else if(classname=="Number")
	 {
	  IntValue iv(0);
	  iv.type=TYPE::NUMBER;
	  runtime_stack.push_back(iv);
	 }
   else if(classname=="Function")
	 {
	  IntValue iv(0);
	  iv.type=TYPE::COBJ;
      iv.value=0;
	  runtime_stack.push_back(iv);
	 }
   else if(classname=="Boolean")
	 {
	  IntValue iv(0);
	  int rv=0;
	  if(temp.size()<3)//�޲�
	     iv.value=0;
	  else if(temp.size()==3)//����
		{
		 if(temp[2].type==TYPE::NUMBER)
		   {
			if(temp[2].value==0)
			   rv=0;
			else
			   rv=1;
		   }
		 else if(temp[2].type==TYPE::STR)
		   {
			string str=GetNameByID(temp[2].value);
			if(str=="*")
			   rv=0;
			else
			   rv=1;
		   }
		 else if(temp[2].type==TYPE::COBJ || temp[2].type==TYPE::LOBJ)
		   {
			rv=0;
		   }
		 else
		   {
			TError::ShowError(Error_constructoragruments,sp-1,temp[2],"Boolean");
			return false;
		   }
		}//end ����
	  else
		{
		 TError::ShowError(Error_argumentsnumbernotmatch,sp-1,temp[1],"Boolean");
		 return false;
		}
	  iv.value=rv;
	  runtime_stack.push_back(iv);
	 }
   //------------------------------------------------------------------------------------
   else if(classname=="Date")//Date���캯��
	 {
	  int index;
	  vector<IntValue>& vec=runtime_heap.GetNew(index);
	  if(temp.size()==2)//�޲�
		{
		 time_t now;
		 now = time(0);
		 tm *tnow = localtime(&now);
		 //��ʽת��
         vec.push_back(1900+tnow->tm_year);//��
		 vec.push_back(tnow->tm_mon+1);//��
		 vec.push_back(tnow->tm_mday);//��
		 vec.push_back(tnow->tm_hour);//Сʱ
		 vec.push_back(tnow->tm_min);//��
		 vec.push_back(tnow->tm_sec);//��
		}
      else if(temp.size()==3)//����,������String
		{
		 string str;
		 if(temp[2].type==TYPE::STR || temp[2].type==TYPE::ID)
		   {
			if(temp[2].type==TYPE::STR)
			   str=GetNameByID(temp[2].value);
            else
			  {
			   str=GetNameByID(temp[2].value);
			   TItem* item;
			   if(runtime_table.GetItemByName(str,&item)==false)
			     {
				  //�Ľ�,������캯�������δ֪��DOM����
			      //TError::ShowError(Error_expectdefinition,sp-1,temp[2],str);
			      //return false;
				  str="March 16, 1981 16:16:16";
			     }
			   if(item->type==TYPE::STRING)
			     {
			      str=GetNameByID(item->value);
			     }
		       else
		         {
			      TError::ShowError(Error_constructoragruments,sp-1,temp[2],"Date");
			      return false;
		         }
			  }
			regex reg("\\s+|,|:",regbase::normal);
			vector<string> rv;
			regex_split(back_inserter(rv),str,reg);
			while(rv.size()<6)
				  rv.push_back("");
			if(rv.size()>6)
			   rv.erase(rv.begin(),rv.begin()+6);
			int time_infor[6];
			time_infor[1]=MonthToInt(rv[0]);//��
			if(time_infor[1]==-1)
			  {
			   TError::ShowError(Error_constructoragruments,sp-1,temp[2],"Date");
			   return false;
			  }
			time_infor[2]=atoi(rv[1].c_str());//��
			time_infor[0]=atoi(rv[2].c_str());//��
			time_infor[3]=atoi(rv[3].c_str());//Сʱ
			time_infor[4]=atoi(rv[4].c_str());//��
			time_infor[5]=atoi(rv[5].c_str());//��
			for(int i=0;i<6;i++)
			    vec.push_back(time_infor[i]);//��ʽת��
		   }//end if TYPE::STR TYPE::ID
		 else if(temp[2].type==TYPE::COBJ || temp[2].type==TYPE::LOBJ)
		   {
			time_t now;
		    now = time(0);
		    tm *tnow = localtime(&now);
		    //��ʽת��
            vec.push_back(1900+tnow->tm_year);//��
		    vec.push_back(tnow->tm_mon+1);//��
		    vec.push_back(tnow->tm_mday);//��
		    vec.push_back(tnow->tm_hour);//Сʱ
		    vec.push_back(tnow->tm_min);//��
		    vec.push_back(tnow->tm_sec);//��
		   }
		 else
		   {
			TError::ShowError(Error_constructoragruments,sp-1,temp[2],"Date");
			return false;
		   }
		}//end ����
	  else if(temp.size()==5)//����
		{
		 int time_infor[3];
		 for(int i=0;i<3;i++)
			{
			 if(temp[i].type==TYPE::NUMBER)
			   {
				time_infor[i]=temp[i].value;
			   }
			 else if(temp[i].type==TYPE::ID)
			   {
				string name=GetNameByID(temp[i].value);
				TItem* item;
				if(runtime_table.GetItemByName(name,&item))
				  {
					if(item->type==TYPE::INT)
					  time_infor[i]=item->value;
					else
					  time_infor[i]=0;
                  }
				else//�����������һ��δ֪��DOM����
				  {
				   //TError::ShowError(Error_expectdefinition,sp-1,temp[i],name);
			       //return false;
				   time_infor[i]=0;
				  }
			   }
			 else
			   time_infor[i]=0;
			 vec.push_back(time_infor[i]);//��ʽת��
			}//end for
		 for(int i=0;i<3;i++)
			 vec.push_back(0);
		}//end ����
	  else if(temp.size()==8)//����
		{
		 int time_infor[6];
		 for(int i=0;i<6;i++)
			{
			 if(temp[i].type==TYPE::NUMBER)
			   {
				time_infor[i]=temp[i].value;
			   }
			 else if(temp[i].type==TYPE::ID)
			   {
				string name=GetNameByID(temp[i].value);
				TItem* item;
				if(runtime_table.GetItemByName(name,&item))
				  {
					if(item->type==TYPE::INT)
					  time_infor[i]=item->value;
					else
					  time_infor[i]=0;
                  }
				else//�Ľ�,���6�������и���δ֪��DOM����
				  {
				   //TError::ShowError(Error_expectdefinition,sp-1,temp[i],name);
			       //return false;
				   time_infor[i]=0;
				  }
			   }
			 else
			   time_infor[i]=0;
			 vec.push_back(time_infor[i]);//��ʽת��
			}//end for
		}//end ����
      else//��������
		{
		 TError::ShowError(Error_constructoragruments,sp-1,temp[2],"Date");
		 return false;
		}
	  IntValue iv(0);
	  iv.value=index;
	  iv.type=TYPE::DATE;
	  runtime_stack.push_back(iv);
	 }
   //------------------------------------------------------------------------------------
   else if(classname=="Object")
	 {
	  IntValue iv(0);
	  iv.type=TYPE::COBJ;
	  runtime_stack.push_back(iv);
	 }
   //------------------------------------------------------------------------------------
   else if(classname=="RegExp")
	 {
	  string str="/\\s/";
	  if(temp.size()>=3)//����
		{
		 if(temp[2].type==TYPE::STR)
		   {
			string name=GetNameByID(temp[2].value).substr(1);
			if(name.empty()==false)
			  {
               string newname=TransformString(name);
			   str="/"+newname+"/";
			  }
		   }
		 else if(temp[2].type==TYPE::ID)
		   {
			string name=GetNameByID(temp[2].value);
			TItem* item;
			if(runtime_table.GetItemByName(name,&item))
			  {
			   if(item->type==TYPE::STRING)
				 {
				  str="/"+GetNameByID(item->value).substr(1)+"/";
				 }
			  }
		   }//end TYPE::ID
		}//end ����
	  if(temp.size()==4)//���еڶ�������
		{
         string style="";
		 if(temp[3].type==TYPE::STR)
		   {
			string name=GetNameByID(temp[3].value).substr(1);
			if(name=="i"||name=="g"||name=="gi")
			   style=name;
		   }
		 else if(temp[3].type==TYPE::ID)
		   {
			string name=GetNameByID(temp[3].value);
			TItem* item;
			if(runtime_table.GetItemByName(name,&item))
			  {
               if(item->type==TYPE::STRING)
				 {
				  name=GetNameByID(item->value).substr(1);
				  if(name=="i" || name=="g" ||name=="gi")
					 style=name;
				 }
			  }
		   }//end TYPE::ID
		 str=str+style;
		}//end �ڶ�������
	  str="*"+str;
	  IntValue iv(0);
	  iv.type=TYPE::STR;
	  int ti=current_string_index++;
	  symbol_table[ti]=str;
	  iv.value=ti;
	  runtime_stack.push_back(iv);
	 }//end RgeExp
   //------------------------------------------------------------------------------------
   else//�Ľ�,δ֪����,������ĳ��function��prototype
	 {
	  //TError::ShowError(Error_needrightclassname,sp-1,temp[1],classname);
      //return false;
	  IntValue iv(0);
	  iv.type=TYPE::COBJ;
	  runtime_stack.push_back(iv);
	 }
  }//end if IsID
else
  {
   TError::ShowError(Error_needrightclassname,sp-1,temp[0],"");
   return false;
  }
 endnewpos.pop();
 
 return true;
}
//--------------------------------------------------------------------------------------------

bool Compiler::command_offsetchar(const int& command,const IntValue& c1,const IntValue& c2,const int& env,int& sp)
{
 sp++;

 int gettype=GetIVType(c1);
 if(gettype!=TYPE::ID && gettype!=TYPE::COBJ && 
	gettype!=TYPE::LOBJ && c1.HasHP()==false)
   {
	TError::ShowError(Error_takenumberasclassname,sp-1,c1,"");
	return false;
   }
 
 int heapaddr;
 int offset;
 if(c1.HasHP())//���Ľ�,���hpָ������Ƿ�ΪHEAP
   {
	IntValue *ivp=runtime_heap.GetHeapItem(c1.hp.x,c1.hp.y,sp-1);
	if(ivp==NULL)
	  {
	   cout<<"heap access error"<<endl;
	   return false;
	  }
	heapaddr=ivp->value;
   }
 else// hp==NULL
   {
    if(c1.type==TYPE::HEAP)//c1��ʾ���е�ַ
      {
	   heapaddr=c1.value;
      }
	else if(c1.type==TYPE::COBJ || c1.type==TYPE::LOBJ)//A[B]���AΪ���ܶ���,�����һ�����ܶ���ѹջ
	  {
	   IntValue iv;
	   iv.value=0;
	   iv.type=c1.type;
       runtime_stack.pop_back();
       runtime_stack.pop_back();
       runtime_stack.push_back(iv);
	   return true;
	  }
    else//���ܲ��ᱻִ��
      {
       string c1name=GetNameByID(c1.value);
       TItem* item1;
       if(runtime_table.GetItemByName(c1name,&item1))//���ű���
         {
		  if(item1->type==TYPE::HEAP)
	         heapaddr=item1->value;
		  else//�����COBJ����LOBJ
			 heapaddr=0;
         }//end if
	   else if(!with_stack.empty())//����Ϊwith���������
	     {
	      IntValue iiv;
          IntValue wo=with_stack.top();
          if(sim->SimulateStatic(iiv,false,GetNameByID(with_stack.top().value),&wo,c1name,NULL,sp,env)==false )
		    {
		     TError::ShowError(Error_expectdefinition,sp-1,c1,c1name);
		     return false;
		    }
          if(iiv.type!=TYPE::STR)
	         heapaddr=iiv.value;
          else
            {
             cout<<"headaddr is a string"<<endl;
             return false;
            }
	      //�����̬��Ա�������string,��Ľ�
	     }
       else//�Ľ�
         {
	      //TError::ShowError(Error_expectdefinition,sp-1,c1,c1name);
	      //return false;
		  //����ID
		  heapaddr=0;
		  IntValue ivtemp(0);
          ivtemp.type=TYPE::COBJ;
          runtime_stack.pop_back();
          runtime_stack.pop_back();
          runtime_stack.push_back(ivtemp);
		  return true;
         }
      }
   }//end else
 

 TItem* item2;
 if(IsID(c2))
   {
	string c2name=GetNameByID(c2.value);
    if(runtime_table.GetItemByName(c2name,&item2))
	  {
	   if(item2->type==TYPE::STRING)
		  offset=0;
	   else
	      offset=item2->value;//���Ľ�,�����ͼ��
	   if(item2->type==TYPE::COBJ || item2->type==TYPE::LOBJ)
		  offset=0;
	  }
	else if(!with_stack.empty())//����Ϊwith���������
	  {
	   IntValue iiv;
       IntValue wo=with_stack.top();
       if(sim->SimulateStatic(iiv,false,GetNameByID(with_stack.top().value),&wo,c2name,NULL,sp,env)==false )
		 {
		  TError::ShowError(Error_expectdefinition,sp-1,c2,c2name);
		  return false;
		 }
       if(iiv.type!=TYPE::STR)
	      offset=iiv.value;
       else
          {
           cout<<"offset is a string"<<endl;
           return false;
          }
	   //�����̬��Ա�������string,��Ľ�
	  }
	else//�Ľ�
	  {
	   //TError::ShowError(Error_expectdefinition,sp-1,c2,c2name);
	   //return false;
	   offset=0;
	  }
   }
 else if(GetIVType(c2)==TYPE::NUMBER)
   {
	offset=c2.value;
   }
 else if(GetIVType(c2)==TYPE::COBJ || GetIVType(c2)==TYPE::LOBJ)
   {
	offset=0;
   }
 else if(GetIVType(c2)==TYPE::STR)//���Ľ�
   {
	offset=0;
   }
 else
   {
	TError::ShowError(Error_classnothavethismethod,sp-1,c2,"[]");
	return false;
   }

 //����heapaddr��offset
 IntValue* iv=runtime_heap.GetHeapItem(heapaddr,offset,sp);
 if(iv==NULL)
   {
	cout<<"heap access error"<<endl;
	return false;
   }
 IntValue ivtemp(0);
 ivtemp.value=iv->value;
 ivtemp.type=iv->type;
 //ivtemp.hp=(IntValue *)(&iv);
 ivtemp.hp.x=heapaddr;
 ivtemp.hp.y=offset;

 runtime_stack.pop_back();
 runtime_stack.pop_back();
 runtime_stack.push_back(ivtemp);

 return true;
}
//--------------------------------------------------------------------------------------------

bool Compiler::command_withchar(const int& command,const IntValue& c2,const int& env,int& sp)
{
 sp++;
 IntValue iv=c2;
 runtime_stack.pop_back();
 with_stack.push(iv);
 return true;
}
//--------------------------------------------------------------------------------------------

bool Compiler::command_quitwithchar(const int& command,const IntValue& c2,const int& env,int& sp)
{
 sp++;
 if(with_stack.empty())
   {
	TError::ShowError(Error_withstackfail,sp-1,c2,"");
	return false;
   }
 with_stack.pop();
 return true;
}
//--------------------------------------------------------------------------------------------

bool Compiler::command_typeofchar(const int& command,const IntValue& c2,const int& env,int& sp)
{
 sp++;
 string us="undefined";
 string tname;
 switch(c2.type)
	   {
	    case TYPE::ID:
		     {
			 TItem* item;
			 string c2name=GetNameByID(c2.value);
             if(runtime_table.GetItemByName(c2name,&item))
			   {
				switch(item->type)
				      {
					   case TYPE::INT:
						    {
						     tname="number";
							 break;
							}
					   case TYPE::STRING:
						    {
						     tname="string";
							 break;
							}
					   case TYPE::VAR:
						    {
						     tname=us;
							 break;
							}
					   case TYPE::HEAP:
						    {
						     tname="object";
							 break;
							}
					   case TYPE::DATE:
						    {
						     tname="object";
							 break;
							}
					   case TYPE::LOBJ:
						    {
						     tname=us;
							 break;
							}
					   case TYPE::COBJ:
						    {
						     tname=us;
							 break;
							}
					   default:
						    tname=us;
					        break;
					  }
			   }//end GetItemByName
			 else
			   tname=us;
		     break;
			 }
		case TYPE::NUMBER:
		     {
			  tname="number";
			  break;
			 }
		case TYPE::STR:
		     {
			  tname="string";
			  break;
			 }
		case TYPE::VAR:
		     {
			  tname=us;
			  break;
			 }
		case TYPE::HEAP:
		     {
			  tname="object";
			  break;
			 }
		case TYPE::DATE:
		     {
			  tname="object";
			  break;
			 }
		case TYPE::LOBJ:
		     {
			  tname=us;
			  break;
			 }
		case TYPE::COBJ:
		     {
			  tname=us;
			  break;
			 }
		default:
		     tname=us;
		     break;
	   }//end switch
  IntValue iv(0);
  iv.type=TYPE::STR;
  tname="*"+tname;
  int ti=current_string_index++;
  symbol_table[ti]=tname;
  iv.value=ti;
  
  runtime_stack.pop_back();
  runtime_stack.push_back(iv);
  return true;
}
//--------------------------------------------------------------------------------------------

void Compiler::CheckRuntimeStackSize()
{
 if(runtime_stack.size()>2*STACK_SIZE_LIMIT && endnewpos.empty() )
   {
	runtime_stack.erase(runtime_stack.begin(),runtime_stack.begin()+STACK_SIZE_LIMIT);
   }
}
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

