#ifndef GETIDBYLONGNAME_H_
#define GETIDBYLONGNAME_H_

#include <node.h>
#include <vector>
#include <map>
#include <list>
#include <sstream>
#include "server.h"
#include "psAPISDK.h"

using namespace std;
class GetIDByLongName : public node::ObjectWrap {
public:
	GetIDByLongName(){}
	~GetIDByLongName(){}
	static void Init(v8::Handle<v8::Object> exports);
	static v8::Handle<v8::Value> NewInstance(const v8::Arguments& args);

private:
	

	static v8::Handle<v8::Value> GetID(const v8::Arguments& args);
	static v8::Handle<v8::Value> New(const v8::Arguments& args); 
	static v8::Persistent<v8::Function> constructor;
	 
	
};

#endif