#ifndef REAL_H_
#define REAL_H_

#include <node.h>
#include "psAPISDK.h"
#include "getidbylongname.h"

using namespace std;
class Server;
class Real : public node::ObjectWrap{
public:
	Real(){}
	~Real(){}
	
	PSUINT32 GetTagID(const char* pszTagName);
	static void Init(v8::Handle<v8::Object> exports);
	static v8::Handle<v8::Value> NewInstance(const v8::Arguments& args);
	
	
private:
	std::map<PSUINT32, std::string> m_ID2TagName;
	std::map<std::string, PSUINT32> m_TagName2ID;
	bool RealWrite_i(const char* pszTagName, double db);
	bool RealRead_i(const char* pszTagName, double& db);
	static v8::Handle<v8::Value> New(const v8::Arguments& args);
	static v8::Persistent<v8::Function> constructor;
	static v8::Handle<v8::Value> RealRead(const v8::Arguments& args);
	static v8::Handle<v8::Value> RealWrite(const v8::Arguments& args);
};



#endif