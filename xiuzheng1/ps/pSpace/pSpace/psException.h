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

class ErrObj:ObjectWrap{
public:
	ErrObj(){}
	~ErrObj(){}
	static Persistent<FunctionTemplate> constructorTemplate;
//private:
	int code_;
	std::string *errString_;
	
};

#endif
