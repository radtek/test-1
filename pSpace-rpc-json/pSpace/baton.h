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

class PspaceNode;

class Baton{
public:
	Baton(){}
	virtual ~Baton(){}
	//�ɲ������ȡ���ID
	PSUINT32 getTagID(const char* pszTagName,PSHANDLE h);
	//�����ɲ�㳤����ȡ���ID
	PSUINT32 **getTagIDList(PSSTR *tagNames,PSHANDLE h,int tagCount);
	//�ɲ��id��ȡ�������
	std::string getTagName(PSUINT32 nTagID);
	//���������ƻ�ȡ����ID 
	PSUINT16 getPropID(const char* pszPropName,PSHANDLE h);
	//�����������ƻ�ȡ������������
	PS_TAG_PROP_INFO getPropInfo(const char* pszPropName,PSHANDLE h);
	//��ȡ���ڵ�
	PSUINT32 getParentID(std::vector<std::string> &v,PSHANDLE h);
	//���ݲ���������ƻ�ȡ�������
	PSUINT16 getTagType(const char* pszTagTypeName);
	//��ȡ��������(��Ҫ������ģ����������)
	PS_DATATYPE_ENUM getDataType(const char* key);
	//��ȡ������
	PS_QUALITY_ENUM getQuality(const char* key);
	//��ȡͳ�Ʒ���
	PS_HIS_AGGREGATE_ENUM getAggregate(const char* key);
	///���ݲ�㳤����ȡ���������������
	//PS_DATATYPE_ENUM getDataTypeByLongName(const char* tagName,PSHANDLE h);
private:
	friend class PspaceNode;
	static std::map<PSUINT32, std::string> id2TagName_;
	static std::map<std::string, PSUINT32> tagName2ID_;
};

class RealReadBaton: public Baton{
public:
	RealReadBaton(PspaceNode* psNode,v8::Handle<v8::Function>* callback);
	~RealReadBaton();
private:
	friend class PspaceNode;
	PspaceNode *psNode;
	v8::Persistent<v8::Function> callback;
	int code_;
	PSUINT32 id;
	std::string *errString;
	std::string tagName;
	PS_DATA *realData_;	
	PS_VARIANT varData_;
	PS_TIME *time_;
	PSUINT32 quality_;

};


class SubBaton:public Baton{
public:
	SubBaton(PspaceNode* psNode,v8::Handle<v8::Function>* callback);
	~SubBaton();
	//ʵʱ�������ݽṹ
	typedef struct CallbackData
	{
		PS_DATA date;
		PSUINT32 tagID;
        CallbackData(){};
        ~CallbackData()
        {
            if (date.Value.DataType ==PSDATATYPE_STRING)
            {
                delete [] date.Value.String.Data;
            }
        };

	}CallbackData;
	//�������ݽṹ
	typedef struct CallbackProp{
		PS_VARIANT propData;
		PSUINT32 tagID;
		PSUINT32 propID;
		PSUINT32 changeType;
        CallbackProp(){};
        ~CallbackProp()
        {
            if (propData.DataType ==PSDATATYPE_STRING)
            {
                delete [] propData.String.Data;
            }
        };
	}CallbackProp;
	//typedef struct CallbackOtherProp{
	//	PSUINT32 propCount;
	//	PSUINT32 changeType;
	//}CallbackOtherProp;
	list<CallbackData> getSubMapData(PSUINT32 subid);
	list<CallbackProp> getSubMapPropData(PSUINT32 subid);
	//CallbackOtherProp getOtherPropData(PSUINT32 subid);
private:
	friend class PspaceNode;
	v8::Persistent<v8::Function> callback;
	int code_;
	PSUINT32 id;
	PSUINT32 subID;
	std::string *errString;
	std::string tagName;
	uv_timer_t *timer;
	PSSTR  *tagName_;
	int  tagCount_;
	PspaceNode *psNode;
	PS_DATA *subData_;
	//��Ŷ��ĵ���ʵʱ����
	static std::map<PSUINT32,list<CallbackData>> mapData_;
	//static std::list<CallbackData> callbackData_;
	//��Ŷ��ĵ�������ֵ
	//static std::list<char*> callbackProps_; 
	//��Ŷ��ĵ�����������
	//static std::map<PSUINT32,CallbackOtherProp> propsOther_;
	//��Ŷ��ĵ�����������
	static std::map<PSUINT32,list<CallbackProp>> mapPropData_;
private:
	//����ʵʱֵ�Ļص�����
	 static PSVOID PSAPI Real_CallbackFunction(
		PSIN PSHANDLE hServer,
		PSIN PSUINT32 nSubscribeId,
		PSIN void *pUserPara,
		PSIN PSUINT32 nCount,
		PSIN PSUINT32 *pTagIds,
		PSIN PS_DATA *pRealDataList
		);
	
	//������Զ��ĵĻص�����
	static PSVOID PSAPI Tag_CallbackFunction( PSIN PSHANDLE hServer,
		PSIN PSUINT32 nSubscribeId,
		PSIN PSVOID *pUserPara,
		PSIN PSUINT32 nTagId,
		PSIN PSUINT32 nChangeType,
		PSIN PSUINT32 nPropCount,
		PSIN PSUINT16 *pPropIds,
		PSIN PS_VARIANT *pPropValues);
	static void timer_cb(uv_timer_t *handle,int status);

	static void time_propCb(uv_timer_t *handle,int status);
};

class DelSubBaton:public Baton{
public:
	DelSubBaton(PspaceNode* psNode,v8::Handle<v8::Function>* callback);
	~DelSubBaton();
private:
	friend class PspaceNode;
	v8::Persistent<v8::Function> callback;
	int code_;
	int tagCount_;
	PSSTR  *tagName_;
	PSUINT32 subID;
	PspaceNode *psNode;
	std::string *errString;
};



class Tag:public Baton{
public:
	Tag(PspaceNode* psNode,v8::Handle<v8::Function>* callback);
	~Tag();
private:
	friend class PspaceNode;
	v8::Persistent<v8::Function> callback;
	//static std::map<std::string,PS_TAG_PROP_ENUM> tag;
	//static std::map<std::string,PS_DATATYPE_ENUM> dataTypeMap_;
	int code_;
	PSUINT32 id;
	PSUINT32 parentTagId_;
	PSUINT32 propCount_;
	char **propArr;
	
	PSUINT16 *pPropIds_;
	PS_VARIANT *pPropValues_;
	std::string *errString;
	std::string tagName;
	PspaceNode *psNode;
	char *rName_;
};

class TagHis:public Baton{
public:
	TagHis(PspaceNode* psNode,v8::Handle<v8::Function>* callback);
	~TagHis();
private:
	friend class PspaceNode;
	v8::Persistent<v8::Function> callback;
	//static std::map<std::string,PS_QUALITY_ENUM> qualityMap_;
	//static std::map<std::string,PS_HIS_AGGREGATE_ENUM> aggregates_;
	int code_;
	PSUINT32 id;
	PSUINT32 tagCount_;
	PSBOOL bBounds_;
	PS_TIME startTime_;
	PS_TIME endTime_;
	PSUINT32 nNumTimeStamps_;
	PS_TIME resampleInterval_; 
	PS_VARIANT *pDataValues_;
	PS_HIS_DATA *pHisDataList_;
	PSUINT32 *tagIds_;
	PSUINT32 *pAggregates_;
	PS_TIME *pTimeStamps_;
	PSUINT32 *pQualities_;
	int  *dataStatus_;
	int timeStatusLen_;
	std::string *errString;
	std::string tagName;
	PspaceNode *psNode;
};

class Alarm:public Baton{
public:
	Alarm(PspaceNode* psNode,v8::Handle<v8::Function>* callback);
	~Alarm();
private:
	friend class PspaceNode;
	v8::Persistent<v8::Function> callback;
	int code_;
	PSUINT32 id;
	PS_TIME startTime_;
	PS_TIME endTime_;
	PS_ALARM_FILTER_FIELD filterField_;
	PS_ALARM_FILTER filter_ ;
	PSUINT32 alarmCount_;
	PS_ALARM *alarms_;
	PSUINT32 ackUserId_;
	char ackUserName_[74];
	PS_TIME ackTime_;
	PSUINT32*  alarmIds_; 
	PSUINT32 ackCount_;
	std::string *errString;
	std::string tagName;
	PspaceNode *psNode;
};

class BatchBaton: public Baton{
public:
	BatchBaton(PspaceNode* psNode,v8::Handle<v8::Function>* callback);
	virtual ~BatchBaton();
private:
	friend class PspaceNode;
	PspaceNode *psNode;
	v8::Persistent<v8::Function> callback;
	int code_;
	int tagCount_;
	//PSUINT32 *tagID_;
	PSSTR  *tagName_;
	PS_VARIANT *dataValues_;
	PS_TIME *timeStamps_;
	PSUINT32 **qualitys_;
	std::string *errString;
	PS_DATA *realData_;	
};

class TagQuery: public Baton{
public:
	TagQuery(PspaceNode* psNode,v8::Handle<v8::Function>* callback);
	virtual ~TagQuery();
private:
	friend class PspaceNode;
	PspaceNode *psNode_;
	v8::Persistent<v8::Function> callback_;
	int code_;
	std::string *errString_;
	PSUINT32 tagID_;
	PS_TAG_QUERY_FILTER *filter_;
	PSUINT16 *propIds_;
	PSUINT16 *fieldPropids_;
	PS_VARIANT *fieldPropValues_;
	int propCount_;
	PSUINT32 fieldPropCnt_;
	PSUINT32 tagCount_;
	PS_TAG_PROP_LIST *tagPropValues_; 
	char **propArr_;

};

#endif