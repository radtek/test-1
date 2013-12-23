#ifndef _util_h_
#define _util_h_

#include <string>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>

//判断参数是否是bool类型,如果不是，做异常处理
#define REQ_BOOL_ARG(I, VAR)                                                                         \
	if (args.Length() <= (I) || !args[I]->IsBoolean())                                                 \
	return ThrowException(Exception::TypeError(String::New("Argument " #I " must be a bool")));      \
	bool VAR = args[I]->IsTrue();
//判断参数是否是string类型
#define REQ_STRING_ARG(I, VAR)                                                                       \
	if (args.Length() <= (I) || !args[I]->IsString())                                                  \
	return ThrowException(Exception::TypeError(String::New("Argument " #I " must be a string")));    \
	Local<String> VAR = Local<String>::Cast(args[I]);
//判断参数是否是数组类型
#define REQ_ARRAY_ARG(I, VAR)                                                                        \
	if (args.Length() <= (I) || !args[I]->IsArray())                                                   \
	return ThrowException(Exception::TypeError(String::New("Argument " #I " must be an array")));    \
	Local<Array> VAR = Local<Array>::Cast(args[I]);
//判断参数是否是函数类型
#define REQ_FUN_ARG(I, VAR)                                                                          \
	if (args.Length() <= (I) || !args[I]->IsFunction())                                                \
	return ThrowException(Exception::TypeError(String::New("Argument " #I " must be a function")));  \
	Local<Function> VAR = Local<Function>::Cast(args[I]);
//判断参数是否是对象类型并将对象转化为Local<Object>
#define REQ_OBJECT_ARG(I, VAR)                                                                       \
	if (args.Length() <= (I) || !args[I]->IsObject())                                                  \
	return ThrowException(Exception::TypeError(String::New("Argument " #I " must be an object")));   \
	Local<Object> VAR = Local<Object>::Cast(args[I]);
//给对象的某一字符串变量赋值var是变量，key是用来赋值的关键字
#define OBJ_GET_STRING(OBJ, KEY, VAR)                                                                \
  {                                                                                                  \
  Local<Value> __val = OBJ->Get(String::New(KEY));                                                 \
  if(__val->IsString()) {                                                                          \
  String::Utf8Value __utf8Val(__val);                                                          \
  VAR = *__utf8Val;                                                                             \
  }                                                                                                \
  }

//给对象的某一数值类型赋值
#define OBJ_GET_NUMBER(OBJ, KEY, VAR, DEFAULT)                                                       \
  {                                                                                                  \
  Local<Value> __val = OBJ->Get(String::New(KEY));                                                 \
  if(__val->IsNumber()) {                                                                          \
  VAR = __val->ToNumber()->Value();                                                              \
  }                                                                                                \
	else if(__val->IsString()) {                                                                     \
	String::Utf8Value __utf8Value(__val);                                                          \
	VAR = atoi(*__utf8Value);                                                                       \
	} else {                                                                                         \
	VAR = DEFAULT;                                                                                 \
	}                                                                                                \
  }


#endif
