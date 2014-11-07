#ifndef UTILS_H_
#define UTILS_H_

#include <node.h>
#include <v8.h>
#include <string>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <iostream>
#include <map>
#include "psAPISDK.h"


using namespace std;
using namespace node;
using namespace v8;


/*
	1.�����жϲ�ת�����ṩ�����ͣ�
		bool,string,array,function,object
	2.���ݹؼ��ָ�����ֵ����������
		string,number
	3.��������,���ܣ�
		�ַ����ָ�ַ����滻
	4.�жϸò���ͬ�������첽�ķ���
		�������к�������true��û�к�������false

*/
#define REQ_BOOL_ARG(I, VAR)                                                                         \
	if (args.Length() <= (I) || !args[I]->IsBoolean())                                                 \
	return ThrowException(Exception::TypeError(String::New("Argument " #I " must be a bool")));      \
	bool VAR = args[I]->IsTrue();

#define REQ_STRING_ARG(I, VAR)                                                                       \
	if (args.Length() <= (I) || !args[I]->IsString())                                                  \
	return ThrowException(Exception::TypeError(String::New("Argument " #I " must be a string")));    \
	Local<String> VAR = Local<String>::Cast(args[I]);

#define REQ_ARRAY_ARG(I, VAR)                                                                        \
	if (args.Length() <= (I) || !args[I]->IsArray())                                                   \
	return ThrowException(Exception::TypeError(String::New("Argument " #I " must be an array")));    \
	Local<Array> VAR = Local<Array>::Cast(args[I]);

#define REQ_FUN_ARG(I, VAR)                                                                          \
	if (args.Length() <= (I) || !args[I]->IsFunction())                                                \
	return ThrowException(Exception::TypeError(String::New("Argument " #I " must be a function")));  \
	Local<Function> VAR = Local<Function>::Cast(args[I]);

#define REQ_OBJECT_ARG(I, VAR)                                                                       \
	if (args.Length() <= (I) || !args[I]->IsObject())                                                  \
	return ThrowException(Exception::TypeError(String::New("Argument " #I " must be an object")));   \
	Local<Object> VAR = Local<Object>::Cast(args[I]);

#define OBJ_GET_STRING(OBJ, KEY, VAR)                                                                \
  {                                                                                                  \
  Local<Value> __val = OBJ->Get(String::New(KEY));                                                 \
  if(__val->IsString()) {                                                                          \
  String::Utf8Value __utf8Val(__val);                                                          \
  VAR = *__utf8Val;                                                                             \
  }                                                                                                \
  }

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

#define GET_NUMBER(OBJ, KEY, VAR)                                                       \
{                                                                                                  \
	Local<Value> __val = OBJ->Get(String::New(KEY));                                                 \
	if(__val->IsNumber()) {                                                                          \
	VAR = __val->ToNumber()->Value();                                                              \
	}                                                                                                \
	else if(__val->IsString()) {                                                                     \
	return ThrowException(Exception::TypeError(String::New("Argument  must be a number")));    \
	}                                                                                       \
}

#define GET_INTER(OBJ, KEY, VAR)                                                       \
{                                                                                                  \
	Local<Value> __val = OBJ->Get(String::New(KEY));                                                 \
	if(__val->IsInt32()) {                                                                          \
	VAR = __val->ToInt32()->Value();                                                              \
	}                                                                                                \
	else if(__val->IsString()) {                                                                     \
	return ThrowException(Exception::TypeError(String::New("Argument  must be a number")));    \
}                                                                                       \
}

#define GET_UINTER(OBJ, KEY, VAR)                                                       \
{                                                                                                  \
	Local<Value> __val = OBJ->Get(String::New(KEY));                                                 \
	if(__val->IsUint32()) {                                                                          \
	VAR = __val->ToUint32()->Value();                                                             \
	}                                                                                                \
	else if(__val->IsString()) {                                                                     \
	return ThrowException(Exception::TypeError(String::New("Argument  must be a number")));    \
}                                                                                       \
}
#define GET_BOOL(OBJ, KEY, VAR)                                                       \
{                                                                                                  \
	Local<Value> __val = OBJ->Get(String::New(KEY));                                                 \
	if(__val->IsBoolean()) {                                                                          \
	VAR = __val->ToBoolean()->Value();                                                             \
	}                                                                                                \
	else if(__val->IsString()) {                                                                     \
	return ThrowException(Exception::TypeError(String::New("Argument  must be a number")));    \
}                                                                                       \
}
/*���طָ����ַ���*/
std::vector<std::string> split(std::string str,std::string pattern);
/*�滻ָ�����ַ���*/
string&   replace_all(std::string& str,const string&  old_value,const  string&  new_value);     
/*��javascript�ַ���ת����c++���õ��ַ�ת*/
const char* ToCString(const String::Utf8Value& value);
//�����к��к���������true,û�к���������false
bool fun(const Arguments& args);
//��PS_VARIANT������С����������Ϊdouble����
double PS_VARIANT2DOUBLE(const PS_VARIANT &var);
//������int��Ϊ��������
int PS_VARIANT2INT(const PS_VARIANT &var);
//������uInt��Ϊunsigned int����
unsigned int PS_VARIANT2UINT(const PS_VARIANT &var);


//ʱ��ת��Ϊ�ַ���
char* PSTIME2STR(PS_TIME psTm);
//�ַ���ת��Ϊʱ��
void STR2PSTIME(PS_TIME* d, std::string s);

char* PSVARIANT2STRHELP(char* pszVariant, PS_VARIANT *pVariant);
//��ȡ���ڵ�
//string& getParentName(std::vector<std::string>& s);
//����ת��
char* GBKToUtf8(const char* strGBK);
std::string GBK2UTF8(const char* strGBK);
//����ת��
void UnicodeToUTF_8(char* pOut,wchar_t* pText);
void Gb2312ToUnicode(wchar_t* pOut,char *gbBuffer);

string UTF8ToGBK(const std::string& strUTF8);  

//���ݹؼ��ֻ�ȡ����ID
PS_TAG_PROP_ENUM finType(map<std::string,PS_TAG_PROP_ENUM> &m,std::string key);

//���ݹؼ��ֻ�ȡ��������
enum __PS_DATATYPE_ENUM findDataType(map<std::string,PS_DATATYPE_ENUM> &m,std::string key);

PS_QUALITY_ENUM findQuality(map<std::string,PS_QUALITY_ENUM> &m,std::string key);

PS_HIS_AGGREGATE_ENUM findAggregate(map<std::string,PS_HIS_AGGREGATE_ENUM> &m,std::string key);

std::string ws2s(const std::wstring& ws);

int findFunction(const Arguments& args);

//Сдת��д
const char* str2Upper(const char *s);

//v8::Date to SYSTEMDATE
void v8date_to_systemtime(v8::Handle<Value> dateVal,LPSYSTEMTIME st);
//SYSTEMDATE to v8::Date
v8::Handle<Value> systemtime_to_v8date(LPSYSTEMTIME st);
//v8::Date to PS_TIME
void V8DATE2PSTIME(PS_TIME* d,v8::Handle<Value> dateVal);


//PS_TIME to v8::Date
Handle<Value> PSTIME2V8DATE(PS_TIME d);

Local<Object> getRealObj(PS_DATA *psData);

Local<Object> getPropObj(PS_TAG_PROP_LIST *var,Local<Array> arr);

Local<Object> getpropValue(PS_VARIANT *val,Local<Object> robj);

//�жϲ���Ƿ����
bool isExit(PSHANDLE h,PSUINT32 tagId);


#endif