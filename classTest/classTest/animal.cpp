#include <iostream>
#include "animal.h"

using namespace std;

Persistent<FunctionTemplate> Animal::constructorTemplate;
Animal::Animal()
{
	int a = 0;
	age = a;
	strcpy(name," ");
}
void Animal::init(Handle<Object> target)
{
	HandleScope scope;
	//定义一个函数模板
	Local<FunctionTemplate> t = FunctionTemplate::New(New);

	constructorTemplate = Persistent<FunctionTemplate>::New(t);
	constructorTemplate->InstanceTemplate()->SetInternalFieldCount(1);
	constructorTemplate->SetClassName(String::NewSymbol("Animal"));

	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"showAnimal",showAnimal);

	target->Set(String::NewSymbol("Animal"),constructorTemplate->GetFunction());



}
//对外的New函数
Handle<Value> Animal::New(const Arguments& args)
{
	HandleScope scope;
	Animal *animal = new Animal();//实例化一个Animal对象，用指针Animal指向
	animal->Wrap(args.This()); //包裹Animal对象
	return scope.Close(args.This());//返回这个对象给外部使用
}
Handle<Value> Animal::showAnimal(const Arguments& args)
{
	HandleScope scope;
	std::cout<<"my age is animal:"<<endl;
	return scope.Close(Undefined());


}