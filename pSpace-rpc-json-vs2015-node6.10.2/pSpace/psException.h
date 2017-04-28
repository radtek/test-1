#ifndef PSEXCEPTION_H_
#define PSEXCEPTION_H_
#include <node.h>
#include <v8.h>
#include "node_object_wrap.h"
#include <exception>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include "utils.h"
using namespace node;
using namespace v8;
using namespace std;
class PsException:public exception{
private:
	char msg_[70];
public:
	PsException(){
		sprintf(msg_,"%s","���������쳣��");
	}
	PsException(const char *msg){
		sprintf(msg_,"���������쳣:%s ",msg);
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
	static Local<Object> newObj(const v8::FunctionCallbackInfo<v8::Value>& args);
	static  void New(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void init(Handle<Object> target);
private:
	static Persistent<Function> constructor_;
};

#endif
