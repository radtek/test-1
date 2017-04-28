#ifndef PSPACENODE_H_
#define PSPACENODE_H_

#include <node.h>
#include <v8.h>
#include <node_object_wrap.h>
#include <iostream>
#include "baton.h"
#include "utils.h"
using namespace node;
using namespace v8;

class PspaceNode:public ObjectWrap{
private:
	PSHANDLE hHanle_;
public:
	PspaceNode();
	~PspaceNode();
	static void init(Handle<Object> target);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void read(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void write(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void query(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void realReadSyn(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void realReadAsy(const v8::FunctionCallbackInfo<v8::Value>& args);
	uv_timer_t **getTimer(PSUINT32 subid);
	void delMapTimer(PSUINT32 subid);
private:
	static void realReadWork(uv_work_t* req);
	static void afterRealRead(uv_work_t* req, int status);
public:
	static void realWriteSyn(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void realWriteAsy(const v8::FunctionCallbackInfo<v8::Value>& args);
	
private:
	static void realWriteWork(uv_work_t* req);
	static void afterRealWrite(uv_work_t* req, int status);
	void disConnect();
public:
	void setConnection(PSHANDLE environment);
	static void close(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void isConnected(const v8::FunctionCallbackInfo<v8::Value>& args);
	
	PSHANDLE getHandle();
	
public:
	static void sub(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void subValueAsy(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void subPropsAsy(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void delSubAll(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void delSub(const v8::FunctionCallbackInfo<v8::Value>& args);
private:
	static void subValueWork(uv_work_t* req);
	static void aftersubValue(uv_work_t* req, int status);
	static void subPropsWork(uv_work_t* req);
	static void aftersubProps(uv_work_t* req, int status);
	//static void timer_cb(uv_timer_t *handle,int status);
	static void delSubAllWork(uv_work_t* req);
	static void afterDelSub(uv_work_t* req, int status);
	static void delSubWork(uv_work_t* req);
	
	
public:
	static void tagAddSyn(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void tagAddAsy(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void add(const v8::FunctionCallbackInfo<v8::Value>& args);
private:
	static void saveHis(PSHANDLE h,PSUINT32 id);
	static void tagAddWork(uv_work_t* req);
	static void afterTagAdd(uv_work_t* req, int status);
public:
	static void tagDelSyn(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void tagDelAsy(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void del(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void setTagPropsSyn(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void setTagpropsAsy(const v8::FunctionCallbackInfo<v8::Value>& args);
private:
	static void tagDelWork(uv_work_t* req);
	static void afterTagDel(uv_work_t* req, int status);
	static void setTagpropsWork(uv_work_t* req);
	static void afterTagsetTagprops(uv_work_t* req, int status);
public:
	static void GetTagsInfo(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void getTagPropsSyn(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void getTagpropsAsy(const v8::FunctionCallbackInfo<v8::Value>& args);
	//.description方式获取单个属性
	static void getSignalPropsSyn(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void getSignalpropsAsy(const v8::FunctionCallbackInfo<v8::Value>& args);
private:
	static void getTagpropsWork(uv_work_t* req);
	static void aftergetTagprops(uv_work_t* req, int status);
	static void aftergetSignalTagprops(uv_work_t* req, int status);
public:
	static void hisSyn(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void hisAsy(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void readHisAtTimeSyn(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void readHisAtTimeAsy(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void readHisRawSyn(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void readHisRawAsy(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void readHisProcessSyn(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void readHisProcessAsy(const v8::FunctionCallbackInfo<v8::Value>& args);
private:
	static void hisInsertWork(uv_work_t* req);
	static void afterHis(uv_work_t* req, int status);
	static void hisInserReplaceWork(uv_work_t* req);
	static void hisReplaceWork(uv_work_t* req);
	static void readHisAtTimeWork(uv_work_t* req);
	static void afterReadHisAtTime(uv_work_t* req,int status);
	static void readHisRawWork(uv_work_t* req);
	static void afterReadHisRaw(uv_work_t* req,int status);
	static void readHisProcessWork(uv_work_t* req);
	//static void afterReadHisProcess(uv_work_t* req,int status);
public:
	static void alarmRealSyn(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void alarmRealAsy(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void hisAlarmSyn(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void hisAlarmAsy(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void ackAlarmSyn(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void ackAlarmAsy(const v8::FunctionCallbackInfo<v8::Value>& args);
private:
	static void alarmWork(uv_work_t* req);
	static void afterAlarmReal(uv_work_t* req,int status);
	static void hisAlarmWork(uv_work_t* req);
	static void ackAlarmWork(uv_work_t* req);
	static void afterAckAlarm(uv_work_t* req,int status);

private:
	static void batchReadWork(uv_work_t* req);
	static void batchWriteWork(uv_work_t* req);
	static void afterBatchRead(uv_work_t* req, int status);
	static void afterBatchWrite(uv_work_t* req, int status);
public:
	static void batRealReadSyn(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void batRealReadAsy(const v8::FunctionCallbackInfo<v8::Value>& args);

	static void batRealWriteSyn(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void batRealWriteAsy(const v8::FunctionCallbackInfo<v8::Value>& args);

	static void querySyn(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void queryAsy(const v8::FunctionCallbackInfo<v8::Value>& args);
private:
	static void queryWork(uv_work_t* req);
	static void afterQueryWork(uv_work_t* req);

public:
	static Persistent<Function> constructor_;
};

class SubChange:ObjectWrap{
public:
	SubChange(){}
	~SubChange(){}
	//static v8::Local<v8::Object> newObj();
	static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void init(Handle<Object> target);

public:
	static Persistent<Function> constructor_;
};
#endif