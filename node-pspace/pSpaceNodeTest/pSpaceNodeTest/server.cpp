#include "psAPISDK.h"
#include <v8.h>
#include <node.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include "server.h"
#include "common.h"
using namespace std;
using namespace v8;//译者：使用v8命名空间，免去每次都写v8::


Server::Server() {};
Server::~Server() {};

PSHANDLE Server::hHandle_ = PSHANDLE_UNUSED;
Persistent<Function> Server::constructor;


void Server::Init(Handle<Object> exports)
{
	Isolate* isolate = Isolate::GetCurrent();

	// Prepare constructor template
	Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
	tpl->SetClassName(String::NewSymbol("Server"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);


	// Prototype
	tpl->PrototypeTemplate()->Set(String::NewSymbol("serverConnect"),
		FunctionTemplate::New(ServerConnect)->GetFunction());

	tpl->PrototypeTemplate()->Set(String::NewSymbol("serverdisconnect"),
		FunctionTemplate::New(ServerDisconnect)->GetFunction());

	 constructor = Persistent<Function>::New(tpl->GetFunction());

	exports->Set(String::NewSymbol("Server"), constructor); //译者：将构造函数暴露出去
}

Handle<Value> Server::New(const Arguments& args) 
{
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope;

	Server* obj = new Server();
	//obj->counter_ = args[0]->IsUndefined() ? 0 : args[0]->NumberValue();
	obj->Wrap(args.This());

	return args.This();
}

Handle<Value> Server::NewInstance(const Arguments& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope;

	const unsigned argc = 1;
	Handle<Value> argv[argc] = { args[0] };
	Local<Object> instance = constructor->NewInstance(argc, argv);

	return scope.Close(instance);
}
Handle<Value> Server::ServerConnect(const Arguments& args) 
{
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope;

	Server* obj = ObjectWrap::Unwrap<Server>(args.This());
	//obj->counter_ += 2;
	psAPI_Common_StartAPI();
	PSHANDLE hServer = PSHANDLE_UNUSED;

	String::Utf8Value str(args[0]);
	String::Utf8Value str1(args[1]);
	String::Utf8Value str2(args[2]);

	const char * pszServer = ToCString(str);
	const char * pszUserName = ToCString(str1);
	const char * pszPassword = ToCString(str2);

	PSAPIStatus nRet = psAPI_Server_Connect((PSSTR)pszServer, (PSSTR)pszUserName, (PSSTR)pszPassword, &hServer);
	if (PSERR(nRet))
	{
		cout<<"psAPI_Server_Connect 错误:"<<psAPI_Commom_GetErrorDesc(nRet)<<"\n";
	}
	else
	{
		cout<<"psAPI_Server_Connect 成功\n";
		hHandle_ = hServer;
		
	}
	return scope.Close(Number::New(nRet));
}

Handle<Value> Server::ServerDisconnect(const Arguments& args)
{
	HandleScope scope;
	PSAPIStatus nRet = PSRET_OK;
	nRet = psAPI_Server_Disconnect(hHandle_);
	psAPI_Common_StopAPI();
	return scope.Close(Number::New(nRet));
}

PSUINT32 Server::GetHandle()
{
	return this->hHandle_;
}