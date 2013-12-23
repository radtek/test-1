#include <v8.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include "getidbylongname.h"
#include "common.h"

using namespace std;
using namespace v8;


Persistent<Function> GetIDByLongName::constructor;
void GetIDByLongName::Init(Handle<Object> exports)
{
	Isolate* isolate = Isolate::GetCurrent();


	// Prepare constructor template
	Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
	tpl->SetClassName(String::NewSymbol("GetIDByLongName"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Prototype
	tpl->PrototypeTemplate()->Set(String::NewSymbol("getID"),
		FunctionTemplate::New(GetID)->GetFunction());
	constructor= Persistent<Function>::New(tpl->GetFunction());

	exports->Set(String::NewSymbol("GetIDByLongName"), constructor); //译者：将构造函数暴露出去


	 
}
Handle<Value> GetIDByLongName::NewInstance(const Arguments& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope;

	const unsigned argc = 1;
	Handle<Value> argv[argc] = { args[0] };
	Local<Object> instance = constructor->NewInstance(argc, argv);

	return scope.Close(instance);
}

Handle<Value> GetIDByLongName::New(const Arguments& args) 
{
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope;

	GetIDByLongName* obj = new GetIDByLongName();
	//obj->counter_ = args[0]->IsUndefined() ? 0 : args[0]->NumberValue();
	obj->Wrap(args.This());

	return args.This();
}
Handle<Value> GetIDByLongName::GetID(const Arguments& args)
{
	HandleScope scope;
	Server *s = new Server();
	PSAPIStatus nRet = PSRET_OK;
	PSUINT32 nTagID = PSTAGID_UNUSED;

	String::Utf8Value names(args[0]);
	const char * pszName = ToCString(names);
	//std::map<std::string, PSUINT32>::iterator iter =  m_TagName2ID.find(pszName);
	
	
	nRet = psAPI_Tag_GetIdByLongName(s->GetHandle(), (PSSTR)pszName, &nTagID);
	if (PSERR(nRet))
	{
		cout<<"GetIDByLongName err"<<psAPI_Commom_GetErrorDesc(nRet)<<endl;
	}
	
	return scope.Close(Number::New(nTagID));
}