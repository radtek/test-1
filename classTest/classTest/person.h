#include <node.h>
#include <v8.h>
#include "node_object_wrap.h"

using namespace node;
using namespace v8;

class Person: ObjectWrap {
private:
	int age;
	char name[64];
public:
	Person();
	~Person(){}
	static void init(Handle<Object> target);
	static Handle<Value> New(const Arguments& args);
	static Handle<Value> showPerson(const Arguments& args);
	static Handle<Value> runCallback(const Arguments& args);
	static Handle<Value> readRead(const Arguments& args);
	static Handle<Value> import(const Arguments& args);
	//获取构造函数，需要自己释放
	static Persistent<FunctionTemplate> constructorTemplate;
	
};