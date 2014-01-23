#ifndef BATON_H_
#define BATON_H_

#include <node.h>
#include <v8.h>
#include <string.h>
#include <iostream>
#include <string>
#include  <map>
#include <list>
#include <vector>
#include "psAPISDK.h"
#include "psException.h"

class Baton{
public:
	Baton(){}
	~Baton(){}
	//由测点名获取测点ID
	PSUINT32 getTagID(const char* pszTagName,PSHANDLE h);
	//由测点id获取测点名字
	std::string getTagName(PSUINT32 nTagID);
	//实时数据数据结构
	typedef struct CallbackData
	{
		double db;
		PSUINT32 quality;
		std::string *time;
	}CallbackData;
	//存放订阅到的数据
	static std::list<CallbackData> callbackData_;
private:
	std::map<PSUINT32, std::string> id2TagName_;
	std::map<std::string, PSUINT32> tagName2ID_;
};
class PspaceNode;
class RealReadBaton: public Baton{
public:
	RealReadBaton(PspaceNode* psNode,v8::Handle<v8::Function>* callback);
	~RealReadBaton();
	PspaceNode *psNode;
	v8::Persistent<v8::Function> callback;
	int code_;
	PSUINT32 id;
	std::string *errString;
	std::string tagName;
	double db;
	PS_TIME time;
	PSUINT32 quality;	
};

class SubBaton:public Baton{
public:
	SubBaton(PspaceNode* psNode,v8::Handle<v8::Function>* callback);
	v8::Persistent<v8::Function> callback;
	~SubBaton(){}
	int code_;
	PSUINT32 id;
	PSUINT32 subID;
	std::string *errString;
	std::string tagName;
	PspaceNode *psNode;
	//实时数据数据结构
	typedef struct CallbackData
	{
		double db;
		PSUINT32 quality;
		char *time;;
	}CallbackData;
	//存放订阅到的实时数据
	static std::list<CallbackData> callbackData_;
	//存放订阅到的属性值
	static std::list<char*> callbackProps_; 
	//存放订阅到的其他属性
	static std::list<PSUINT32> propsOther_;
public:
	//订阅实时值的回调函数
	static PSVOID PSAPI Real_CallbackFunction(
		PSIN PSHANDLE hServer,
		PSIN PSUINT32 nSubscribeId,
		PSIN PSVOID *pUserPara,
		PSIN PSUINT32 nCount,
		PSIN PSUINT32 *pTagIds,
		PSIN PS_DATA *pRealDataList
		);
	//测点属性订阅的回调函数
	static PSVOID PSAPI Tag_CallbackFunction( PSIN PSHANDLE hServer,
		PSIN PSUINT32 nSubscribeId,
		PSIN PSVOID *pUserPara,
		PSIN PSUINT32 nTagId,
		PSIN PSUINT32 nChangeType,
		PSIN PSUINT32 nPropCount,
		PSIN PSUINT16 *pPropIds,
		PSIN PS_VARIANT *pPropValues);
};

class Tag:public Baton{
public:
	Tag(PspaceNode* psNode,v8::Handle<v8::Function>* callback);
	~Tag();
	v8::Persistent<v8::Function> callback;
	static std::map<std::string,PS_TAG_PROP_ENUM> tag;
	static std::map<std::string,PS_DATATYPE_ENUM> dataTypeMap_;
	int code_;
	PSUINT32 id;
	PSUINT32 parentTagId_;
	PSUINT32 propCount_;
	
	PSUINT16 *pPropIds_;
	PS_VARIANT *pPropValues_;
	std::string *errString;
	std::string tagName;
	PspaceNode *psNode;
};

class TagHis:public Baton{
public:
	TagHis(PspaceNode* psNode,v8::Handle<v8::Function>* callback);
	~TagHis();
	v8::Persistent<v8::Function> callback;
	static std::map<std::string,PS_QUALITY_ENUM> qualityMap_;
	int code_;
	PSUINT32 id;
	PSUINT32 tagCount_;

	PS_VARIANT *pDataValues_;
	PSUINT32 *tagIds_;

	PS_TIME *pTimeStamps_;
	PSUINT32 *pQualities_;
	std::string *errString;
	std::string tagName;
	PspaceNode *psNode;
};

#endif