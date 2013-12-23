#include <v8.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include "real.h"
#include "common.h"

using namespace std;
using namespace v8;


Persistent<Function> Real::constructor;

void Real::Init(Handle<Object> exports)
{
	Isolate* isolate = Isolate::GetCurrent();


	// Prepare constructor template
	Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
	tpl->SetClassName(String::NewSymbol("Real"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Prototype
	tpl->PrototypeTemplate()->Set(String::NewSymbol("realRead"),
		FunctionTemplate::New(RealRead)->GetFunction());

	tpl->PrototypeTemplate()->Set(String::NewSymbol("realWrite"),
		FunctionTemplate::New(RealWrite)->GetFunction());

	constructor = Persistent<Function>::New(tpl->GetFunction());

	exports->Set(String::NewSymbol("Real"), constructor); //译者：将构造函数暴露出去
}

Handle<Value> Real::NewInstance(const Arguments& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope;

	const unsigned argc = 1;
	Handle<Value> argv[argc] = { args[0] };
	Local<Object> instance = constructor->NewInstance(argc, argv);

	return scope.Close(instance);
}
PSUINT32 Real::GetTagID(const char* pszTagName)
{
	Server *s = new Server();
	std::map<std::string, PSUINT32>::iterator iter = m_TagName2ID.find(pszTagName);
	if (iter != m_TagName2ID.end())
	{
		return iter->second;
	}
	PSUINT32 nTagID = PSTAGID_UNUSED;
	PSAPIStatus nRet = psAPI_Tag_GetIdByLongName(s->GetHandle(), (PSSTR)pszTagName, &nTagID);
	if (PSERR(nRet))
	{
		return PSTAGID_UNUSED;
	}
	m_TagName2ID[pszTagName] = nTagID;
	m_ID2TagName[nTagID] = pszTagName;
	return nTagID;
}
Handle<Value> Real::New(const Arguments& args) 
{
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope;

	Real* obj = new Real();
	//obj->counter_ = args[0]->IsUndefined() ? 0 : args[0]->NumberValue();
	obj->Wrap(args.This());

	return args.This();
}
bool Real::RealRead_i(const char* pszTagName, double& db)
{
	Server *s = new Server();
	PS_DATA *pRealData = PSNULL;
	PSUINT32 nTagID = GetTagID(pszTagName);
	if (nTagID==PSTAGID_UNUSED)
	{
		return false;
	}
	PSAPIStatus nRet = psAPI_Real_Read(s->GetHandle(), nTagID, &pRealData);
	if (PSERR(nRet))
	{
		return false;
	}
	db = PS_VARIANT2DOUBLE(pRealData->Value);
	psAPI_Memory_FreeDataList(&pRealData, 1);
	return true;
}

Handle<Value> Real::RealRead(const Arguments& args)
{
	if (!args[0]->IsString())
	{
		cout<<"pSpaceJS::RealRead error\n";
		return Boolean::New(false);
	}
	Real *r = new Real();
	String::Utf8Value str(args[0]);
	String::Utf8Value names(args[0]);
	const char * pszName = ToCString(names);
	double db = 0;
	bool bl = r->RealRead_i(pszName, db);
	if (bl)
	{
		return Number::New(db);
	}
	return Undefined();
}

bool Real::RealWrite_i(const char* pszTagName, double db)
{
	Server *s  = new Server();
	PS_VARIANT realData;
	realData.DataType = PSDATATYPE_DOUBLE;
	realData.Double = db;

	PSUINT32 nTagID = GetTagID(pszTagName);
	if (nTagID==PSTAGID_UNUSED)
	{
		return false;
	}

	PSAPIStatus nRet = psAPI_Real_Write(s->GetHandle(), nTagID, &realData, PSNULL, PSNULL);
	if (PSERR(nRet))
	{
		return false;
	}
	return true;
}
Handle<Value> Real::RealWrite(const Arguments& args)
{
	if (args.Length() != 2 || !args[0]->IsString() || !args[1]->IsNumber())
	{
		cout<<"REAL::RealWrite error\n";
		return Boolean::New(false);
	}
	String::Utf8Value names(args[0]);
	const char * pszName = ToCString(names);
	Real *r = new Real();
	bool bl = r->RealWrite_i(pszName, args[1]->ToNumber()->Value());
	return Boolean::New(bl);
}