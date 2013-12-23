#ifndef _util_h_
#define _util_h_

#include <string>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>

//�жϲ����Ƿ���bool����,������ǣ����쳣����
#define REQ_BOOL_ARG(I, VAR)                                                                         \
	if (args.Length() <= (I) || !args[I]->IsBoolean())                                                 \
	return ThrowException(Exception::TypeError(String::New("Argument " #I " must be a bool")));      \
	bool VAR = args[I]->IsTrue();
//�жϲ����Ƿ���string����
#define REQ_STRING_ARG(I, VAR)                                                                       \
	if (args.Length() <= (I) || !args[I]->IsString())                                                  \
	return ThrowException(Exception::TypeError(String::New("Argument " #I " must be a string")));    \
	Local<String> VAR = Local<String>::Cast(args[I]);
//�жϲ����Ƿ�����������
#define REQ_ARRAY_ARG(I, VAR)                                                                        \
	if (args.Length() <= (I) || !args[I]->IsArray())                                                   \
	return ThrowException(Exception::TypeError(String::New("Argument " #I " must be an array")));    \
	Local<Array> VAR = Local<Array>::Cast(args[I]);
//�жϲ����Ƿ��Ǻ�������
#define REQ_FUN_ARG(I, VAR)                                                                          \
	if (args.Length() <= (I) || !args[I]->IsFunction())                                                \
	return ThrowException(Exception::TypeError(String::New("Argument " #I " must be a function")));  \
	Local<Function> VAR = Local<Function>::Cast(args[I]);
//�жϲ����Ƿ��Ƕ������Ͳ�������ת��ΪLocal<Object>
#define REQ_OBJECT_ARG(I, VAR)                                                                       \
	if (args.Length() <= (I) || !args[I]->IsObject())                                                  \
	return ThrowException(Exception::TypeError(String::New("Argument " #I " must be an object")));   \
	Local<Object> VAR = Local<Object>::Cast(args[I]);
//�������ĳһ�ַ���������ֵvar�Ǳ�����key��������ֵ�Ĺؼ���
#define OBJ_GET_STRING(OBJ, KEY, VAR)                                                                \
  {                                                                                                  \
  Local<Value> __val = OBJ->Get(String::New(KEY));                                                 \
  if(__val->IsString()) {                                                                          \
  String::Utf8Value __utf8Val(__val);                                                          \
  VAR = *__utf8Val;                                                                             \
  }                                                                                                \
  }

//�������ĳһ��ֵ���͸�ֵ
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
