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
	1.类型判断并转换，提供的类型：
		bool,string,array,function,object
	2.依据关键字给对象赋值，包括类型
		string,number
	3.参数解析,功能：
		字符串分割，字符串替换
	4.判断该采用同步还是异步的方法
		参数中有函数返回true，没有函数返回false

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
/*返回分割后的字符串*/
std::vector<std::string> split(std::string str,std::string pattern);
/*替换指定的字符串*/
string&   replace_all(std::string& str,const string&  old_value,const  string&  new_value);     
/*将javascript字符串转换成c++可用的字符转*/
const char* ToCString(const String::Utf8Value& value);
//参数中含有函数，返回true,没有函数，返回false
bool fun(const Arguments& args);
//把PS_VARIANT所有有小数的数据作为double处理
double PS_VARIANT2DOUBLE(const PS_VARIANT &var);
//把所有int作为整数处理
int PS_VARIANT2INT(const PS_VARIANT &var);
//把所有uInt作为unsigned int处理
unsigned int PS_VARIANT2UINT(const PS_VARIANT &var);


//时间转换为字符串
char* PSTIME2STR(PS_TIME psTm);
//字符串转化为时间
void STR2PSTIME(PS_TIME* d, std::string s);

char* PSVARIANT2STRHELP(char* pszVariant, PS_VARIANT *pVariant);
//获取父节点
//string& getParentName(std::vector<std::string>& s);
//编码转换
char* GBKToUtf8(const char* strGBK);
std::string GBK2UTF8(const char* strGBK);
//编码转换
void UnicodeToUTF_8(char* pOut,wchar_t* pText);
void Gb2312ToUnicode(wchar_t* pOut,char *gbBuffer);

string UTF8ToGBK(const std::string& strUTF8);  

//根据关键字获取属性ID
PS_TAG_PROP_ENUM finType(map<std::string,PS_TAG_PROP_ENUM> &m,std::string key);

//根据关键字获取数据类型
enum __PS_DATATYPE_ENUM findDataType(map<std::string,PS_DATATYPE_ENUM> &m,std::string key);

PS_QUALITY_ENUM findQuality(map<std::string,PS_QUALITY_ENUM> &m,std::string key);

PS_HIS_AGGREGATE_ENUM findAggregate(map<std::string,PS_HIS_AGGREGATE_ENUM> &m,std::string key);

std::string ws2s(const std::wstring& ws);

int findFunction(const Arguments& args);

//小写转大写
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

//判断测点是否存在
bool isExit(PSHANDLE h,PSUINT32 tagId);


#endif