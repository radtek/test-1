#include "psException.h"

Persistent<Function> Error::constructor_;

void Error::New(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Isolate* isolate = args.GetIsolate();

	if (args.IsConstructCall())
	{
		//Invoked as constructor:'new Error(...)'
		Error* pObj = new Error();
		pObj->Wrap(args.This());
		args.GetReturnValue().Set(args.This());
	}
	else
	{
		//Invoked as plain function 'Error(...)',turn into construct call

		Local<Value> argv[1] = { args[0] };
		Local<Context> context = isolate->GetCurrentContext();
		Local<Function> cons = Local<Function>::New(isolate, constructor_);
		Local<Object> result = cons->NewInstance(context, 1, argv).ToLocalChecked();
		args.GetReturnValue().Set(result);
	}
}

Local<Object> Error::newObj(const v8::FunctionCallbackInfo<v8::Value>& args) {
	
	Isolate* isolate = args.GetIsolate();
	Local<Value> argv[1] = { args[0] };
	Local<Function> cons = Local<Function>::New(isolate, constructor_);
	Local<Context> context = isolate->GetCurrentContext();
	Local<Object> instance = cons->NewInstance(context, 1, argv).ToLocalChecked();
	return instance;

}

void Error::init(Handle<Object> target)
{

	Isolate* isolate = target->GetIsolate();

	Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);

	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	tpl->SetClassName(String::NewFromUtf8(isolate, "Error"));

	constructor_.Reset(isolate, tpl->GetFunction());

	target->Set(String::NewFromUtf8(isolate, "Error"), tpl->GetFunction());


}