#include <node.h>
#include "server.h"
#include "getidbylongname.h"
#include "real.h"

using namespace v8;//���ߣ�ʹ��v8�����ռ䣬��ȥÿ�ζ�дv8::

Handle<Value> CreateObject(const Arguments& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope;
	return scope.Close(Server::NewInstance(args));
}

Handle<Value> CreateObject1(const Arguments& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope;
	return scope.Close(GetIDByLongName::NewInstance(args));
}

Handle<Value> CreateObject2(const Arguments& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope;
	return scope.Close(Real::NewInstance(args));
}
void InitAll(Handle<Object> exports,Handle<Object> module) 
{
	Server::Init(exports); 
	GetIDByLongName::Init(exports);
	Real::Init(exports);
	module->Set(String::NewSymbol("exports"),
		FunctionTemplate::New(CreateObject)->GetFunction());
}

NODE_MODULE(pSpaceNodeTest, InitAll)