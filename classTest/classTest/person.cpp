#include <iostream>
#include <stdlib.h>
#include "person.h"
#include  "utils.h"

using namespace std;

Persistent<FunctionTemplate> Person::constructorTemplate;
Person::Person()
{
	int a = 0;
	age = a;
	strcpy(name," ");
}
void Person::init(Handle<Object> target)
{
	HandleScope scope;
	//定义一个函数模板
	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructorTemplate = Persistent<FunctionTemplate>::New(t);
	constructorTemplate->InstanceTemplate()->SetInternalFieldCount(1);
	constructorTemplate->SetClassName(String::NewSymbol("Person"));
	
	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"showPerson",showPerson);
	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"runcallback",runCallback);
	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"readread",readRead);
	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"importdata",import);

	target->Set(String::NewSymbol("Person"),constructorTemplate->GetFunction());
}
Handle<Value> Person::runCallback(const Arguments& args)
{
	HandleScope scope;
	//将传递过来的函数转换成C++的函数
	Local<Function> cb = Local<Function>::Cast(args[0]);
	//创建数组对象
	const unsigned argc = 2;
	Local<Value> argv[2] = {Local<Value>::New(String::New("how to do!")),
							Local<Value>::New(String::New("What a fucking!"))};
	cb->Call(Context::GetCurrent()->Global(),argc,argv);
	
	return scope.Close(Undefined());
}

//json格式数据导出测试
Handle<Value> Person::readRead(const Arguments& args)
{
	//HandleScope scope;
	//定义数组对象元素个数
	Handle<Array> array = Array::New(2);
	//创建数组对象
	Handle<Object> obj = Object::New(); 
	Handle<Object> obj1 = Object::New();
	//向数组对象中添加元素
	obj->Set(String::New("fistName"),String::New("guangfei"));
	obj->Set(String::New("lastName"),String::New("yin"));
	obj->Set(Int32::New(1),String::New("good"));
	
	obj1->Set(String::New("2"),String::New("bad"));
	obj1->Set(Int32::New(3),String::New("to bo or not"));
	

	//对像作为数组第0个位置元素
	array->Set(0,obj);
	//对象作为数组第1个位置元素
	array->Set(1,obj1);
	//返回数组对象给外部
	return array;
}
const char* ToCString(const String::Utf8Value& value)
{
	return *value ? *value: "string conversion failed";
}
//json数据导入测试
 Handle<Value> Person::import(const Arguments& args)
{
	//设置object 
	//Handle<Object> obj = Object::New();
	//args[0]->Set(String::NewSymbol("hostname"),Integer::New(1));
	REQ_OBJECT_ARG(0, settings);
	//查询是否有对应的key,并获取value
	if (settings->Has(String::New("hostname")))
	{
		Handle<Value> val = settings->Get(String::New("password"));
		Handle<String> str = val->ToString();
		char buf[64] = " ";
		memset(buf,0,sizeof(buf));
		str->WriteUtf8(buf);
		std::cout<<buf<<std::endl;
	}
	return Undefined();

}


//对外的New函数
Handle<Value> Person::New(const Arguments& args)
{
	HandleScope scope;
	Person *person = new Person();//实例化一个Person对象，用指针person指向
	person->Wrap(args.This()); //包裹person对象
	return scope.Close(args.This());//返回这个对象给外部使用
}
Handle<Value> Person::showPerson(const Arguments& args)
{
	HandleScope scope;
	std::cout<<"my age is:"<<endl;
	return scope.Close(Undefined());
	

}