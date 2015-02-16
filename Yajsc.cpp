#include "Yajsc.hpp"
//-----------------------------------------------------------------------------

string Yajsc::Compile(const string& filename,string addtional_code)
{
 JSIInterface jsi(filename);
 if(!addtional_code.empty())
	jsi.SetAddtionalCode(addtional_code);
 jsi.Prepare();
 if(jsi.Scan())
    return jsi.Compile();
 return "";
}
//-----------------------------------------------------------------------------

string Compile(istream& ifs,string addtional_code)
{
 JSIInterface jsi(&ifs);
 if(!addtional_code.empty())
	jsi.SetAddtionalCode(addtional_code);
 jsi.Prepare();
 if(jsi.Scan())
    return jsi.Compile();
 return "";
}
//-----------------------------------------------------------------------------
