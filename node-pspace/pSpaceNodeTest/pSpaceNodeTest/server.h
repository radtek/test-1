#ifndef SERVER_H_
#define SERVER_H_

#include "psAPISDK.h"
#include <node.h> //译者：将内部node类库包含进来


class Server : public node::ObjectWrap {
public:
	Server();
	~Server();
	static v8::Handle<v8::Value> NewInstance(const v8::Arguments& args);
	static void Init(v8::Handle<v8::Object> exports);
	PSUINT32 GetHandle();

private:
	
	static v8::Persistent<v8::Function> constructor;
	static v8::Handle<v8::Value> New(const v8::Arguments& args); //译者：定义静态方法New
	static v8::Handle<v8::Value> ServerConnect(const v8::Arguments& args); //译者：定义静态方法PlusOne
	static v8::Handle<v8::Value> ServerDisconnect(const v8::Arguments& args); 
	
	//double counter_;
	static PSHANDLE	hHandle_;
};

#endif

