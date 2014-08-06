#ifndef PSEXCEPTION_H_
#define PSEXCEPTION_H_
#include <node.h>
#include <v8.h>
#include <exception>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

using namespace node;
using namespace v8;
using namespace std;
class PsException:public exception{
private:
	char msg_[70];
public:
	PsException(){
		sprintf(msg_,"%s","服务器端异常！");
	}
	PsException(const char *msg){
		sprintf(msg_,"服务器端异常:%s ",msg);
	}
	const char *what()const throw()
	{
		return msg_;
	}
};

class Error:ObjectWrap{
public:
	Error(){}
	~Error(){}
	static v8::Local<v8::Object> newObj();
	static Handle<Value> New(const Arguments& args);
	static void init(Handle<Object> target);
};

#endif
