#ifndef PSPACE_H_
#define PSPACE_H_

#include <v8.h>
#include <node.h>
#include "psAPISDK.h"
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include <list>

using namespace v8;
//字符编码转换
void ConvertUtf8ToGBK(std::string& strUtf8);
//格式异常处理
std::string formatException(v8::TryCatch* try_catch);

//主要功能类
class pSpace : public node::ObjectWrap
{
public:
	pSpace()
	{
		std::cout<<"pSpace::pSpace()\n";
		subscribeID_ = -1;
	}
	~pSpace()
	{
		std::cout<<"pSpace::~pSpace()\n";
	}

	//回调函数
	void OnRealData(PSUINT32 nTagID, double db);
	//Handle<Value> OnRealData(const Arguments& args);
public:
	//实时订阅
	static Handle<Value> RealSubscribe(const Arguments& args);
	//实时读取
	static Handle<Value> RealRead(const Arguments& args);
	//实时写数据
	static Handle<Value> RealWrite(const Arguments& args);

	static Handle<Value> Log(const Arguments& args);
	//////////
	//static Handle<Value> OnRealData(const Arguments& args);
	////////////////////////////////////////////////
	//把订阅到的数据返回
	//static Handle<Value> SubscribeData(const Arguments& args);
	

public:
	////////////////////////////////////////////////////////////////////
	static void Init();
	static v8::Handle<v8::Value> NewInstance(const v8::Arguments& args);

	//启动数据库
	static  Handle<Value> StartpSpace(const Arguments& args);
	//停止数据库
	static  Handle<Value> StoppSpace(const Arguments& args);

private:

	static v8::Persistent<v8::Function> constructor;
	static v8::Handle<v8::Value> New(const v8::Arguments& args);
	//添加
	static v8::Handle<v8::Value> GetID(const v8::Arguments& args);
	PSUINT32 GetTagID(const char* pszTagName);
	std::string GetTagName(PSUINT32 nTagID);
	///static std::string GetTagName(PSUINT32 nTagID);
	bool RealSubscribe_i(const char* pszTagName);
	
	bool RealWrite_i(const char* pszTagName, double db);
	bool RealRead_i(const char* pszTagName, double& db,PSUINT32& quality,PS_TIME& time);
	
	static PSVOID PSAPI Real_CallbackFunction(
		PSIN PSHANDLE hServer,
		PSIN PSUINT32 nSubscribeId,
		PSIN PSVOID *pUserPara,
		PSIN PSUINT32 nCount,
		PSIN PSUINT32 *pTagIds,
		PSIN PS_DATA *pRealDataList
		);
	
	//把PS_VARIANT 转换为DOUBLE字符串
	static double PS_VARIANT2DOUBLE(const PS_VARIANT &var);


private:
	static PSHANDLE	hHandle_;
	std::map<PSUINT32, std::string> m_ID2TagName;
	std::map<std::string, PSUINT32> m_TagName2ID;

	Persistent<Object>	jsObj_;
	Persistent<Object>	contextObj_;

	Persistent<Function> OnStart;
	Persistent<Function> OnData;
	Persistent<Function> OnStop;

	PSUINT32 subscribeID_;
public:
	static std::vector<pSpace*> pSpace_;
	typedef struct __CallbackData
	{
		pSpace* p;
		PSUINT32 nTagID;
		double db;
	}CallbackData;
	static std::list<CallbackData> s_CallbackData;
};



#endif