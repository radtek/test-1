#include "psException.h"
static Persistent<Function> constructorTemplate;

Handle<Value> Error::New(const Arguments& args)
{
	HandleScope scope;
	Error *t = new Error();
	t->Wrap(args.This());
	return scope.Close(args.This());
}

Local<Object> Error::newObj() {
	HandleScope scope;
	Local<Object> obj = constructorTemplate->NewInstance();
	return scope.Close(obj);
}

void Error::init(Handle<Object> target)
{
	HandleScope scope;
	//定义一个函数模板
	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	//constructorTemplate = Persistent<FunctionTemplate>::New(t);
	t->InstanceTemplate()->SetInternalFieldCount(1);

	t->SetClassName(String::NewSymbol("Error"));

	constructorTemplate= Persistent<Function>::New(t->GetFunction());
	target->Set(String::NewSymbol("Err"),t->GetFunction());

	target->Set(String::NewSymbol("Error"), constructorTemplate);


}