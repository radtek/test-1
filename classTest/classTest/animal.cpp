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
	//����һ������ģ��
	Local<FunctionTemplate> t = FunctionTemplate::New(New);

	constructorTemplate = Persistent<FunctionTemplate>::New(t);
	constructorTemplate->InstanceTemplate()->SetInternalFieldCount(1);
	constructorTemplate->SetClassName(String::NewSymbol("Animal"));

	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"showAnimal",showAnimal);

	target->Set(String::NewSymbol("Animal"),constructorTemplate->GetFunction());



}
//�����New����
Handle<Value> Animal::New(const Arguments& args)
{
	HandleScope scope;
	Animal *animal = new Animal();//ʵ����һ��Animal������ָ��Animalָ��
	animal->Wrap(args.This()); //����Animal����
	return scope.Close(args.This());//�������������ⲿʹ��
}
Handle<Value> Animal::showAnimal(const Arguments& args)
{
	HandleScope scope;
	std::cout<<"my age is animal:"<<endl;
	return scope.Close(Undefined());


}