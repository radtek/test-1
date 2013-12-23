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
	//����һ������ģ��
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
	//�����ݹ����ĺ���ת����C++�ĺ���
	Local<Function> cb = Local<Function>::Cast(args[0]);
	//�����������
	const unsigned argc = 2;
	Local<Value> argv[2] = {Local<Value>::New(String::New("how to do!")),
							Local<Value>::New(String::New("What a fucking!"))};
	cb->Call(Context::GetCurrent()->Global(),argc,argv);
	
	return scope.Close(Undefined());
}

//json��ʽ���ݵ�������
Handle<Value> Person::readRead(const Arguments& args)
{
	//HandleScope scope;
	//�����������Ԫ�ظ���
	Handle<Array> array = Array::New(2);
	//�����������
	Handle<Object> obj = Object::New(); 
	Handle<Object> obj1 = Object::New();
	//��������������Ԫ��
	obj->Set(String::New("fistName"),String::New("guangfei"));
	obj->Set(String::New("lastName"),String::New("yin"));
	obj->Set(Int32::New(1),String::New("good"));
	
	obj1->Set(String::New("2"),String::New("bad"));
	obj1->Set(Int32::New(3),String::New("to bo or not"));
	

	//������Ϊ�����0��λ��Ԫ��
	array->Set(0,obj);
	//������Ϊ�����1��λ��Ԫ��
	array->Set(1,obj1);
	//�������������ⲿ
	return array;
}
const char* ToCString(const String::Utf8Value& value)
{
	return *value ? *value: "string conversion failed";
}
//json���ݵ������
 Handle<Value> Person::import(const Arguments& args)
{
	//����object 
	//Handle<Object> obj = Object::New();
	//args[0]->Set(String::NewSymbol("hostname"),Integer::New(1));
	REQ_OBJECT_ARG(0, settings);
	//��ѯ�Ƿ��ж�Ӧ��key,����ȡvalue
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


//�����New����
Handle<Value> Person::New(const Arguments& args)
{
	HandleScope scope;
	Person *person = new Person();//ʵ����һ��Person������ָ��personָ��
	person->Wrap(args.This()); //����person����
	return scope.Close(args.This());//�������������ⲿʹ��
}
Handle<Value> Person::showPerson(const Arguments& args)
{
	HandleScope scope;
	std::cout<<"my age is:"<<endl;
	return scope.Close(Undefined());
	

}