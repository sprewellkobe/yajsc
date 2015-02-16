#include "BinaryCode.hpp"
//-------------------------------------------------------------------------------------------------
bool BinaryCode::SaveToFile(string filename,const BinaryCode& bc)
{
 if( (filename.size()>=5&&filename.substr(filename.size()-bc_ext.size(),bc_ext.size())==bc_ext)==false )
    filename+=bc_ext;
 ofstream ofs(filename.c_str());
 boost::archive::binary_oarchive oa(ofs);
 try
   {
    oa<<bc;
   }
 catch(...)
   {
    ofs.close();
    return false;
   }
 ofs.close();
 cout<<"saved binarycode to "<<filename<<" successfully"<<endl; 
 return true; 
}
//-------------------------------------------------------------------------------------------------
bool BinaryCode::LoadFormFile(const string& filename,BinaryCode& bc)
{
 ifstream ifs(filename.c_str(),std::ios::binary);
 archive::binary_iarchive ia(ifs);
 try
   {
    ia>>bc;
   }
 catch(...)
   {
    ifs.close();
    return false;
   }
 ifs.close();
 cout<<"loaded binarycode from "<<filename<<" successfully"<<endl;
 return true;
}
//-------------------------------------------------------------------------------------------------
