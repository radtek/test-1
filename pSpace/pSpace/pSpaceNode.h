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
	static Handle<Value> New(const Arguments& args);
	static Handle<Value> read(const Arguments& args);
	static Handle<Value> write(const Arguments& args);
	static Handle<Value> query(const Arguments& args);
	static Handle<Value> realReadSyn(const Arguments& args);
	static Handle<Value> realReadAsy(const Arguments& args);
	uv_timer_t **getTimer(PSUINT32 subid);
	void delMapTimer(PSUINT32 subid);
private:
	static void realReadWork(uv_work_t* req);
	static void afterRealRead(uv_work_t* req, int status);
public:
	static Handle<Value> realWriteSyn(const Arguments& args);
	static Handle<Value> realWriteAsy(const Arguments& args);
	
private:
	static void realWriteWork(uv_work_t* req);
	static void afterRealWrite(uv_work_t* req, int status);
	void disConnect();
public:
	void setConnection(PSHANDLE environment);
	static Handle<Value> close(const Arguments& args);
	//static Handle<Value> isConnected(const Arguments& args);
	
	PSHANDLE getHandle();
	static Persistent<FunctionTemplate> constructorTemplate;
public:
	static Handle<Value> sub(const Arguments& args);
	static Handle<Value> subValueAsy(const Arguments& args);
	static Handle<Value> subPropsAsy(const Arguments& args);
	static Handle<Value> delSubAll(const Arguments& args);
	static Handle<Value> delSub(const Arguments& args);
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
	static Handle<Value> tagAddSyn(const Arguments& args);
	static Handle<Value> tagAddAsy(const Arguments& args);
	static Handle<Value> add(const Arguments& args);
private:
	static void saveHis(PSHANDLE h,PSUINT32 id);
	static void tagAddWork(uv_work_t* req);
	static void afterTagAdd(uv_work_t* req, int status);
public:
	static Handle<Value> tagDelSyn(const Arguments& args);
	static Handle<Value> tagDelAsy(const Arguments& args);
	static Handle<Value> del(const Arguments& args);
	static Handle<Value> setTagPropsSyn(const Arguments& args);
	static Handle<Value> setTagpropsAsy(const Arguments& args);
private:
	static void tagDelWork(uv_work_t* req);
	static void afterTagDel(uv_work_t* req, int status);
	static void setTagpropsWork(uv_work_t* req);
	static void afterTagsetTagprops(uv_work_t* req, int status);
public:
	static Handle<Value> getTagPropsSyn(const Arguments& args);
	static Handle<Value> getTagpropsAsy(const Arguments& args);
	//.description方式获取单个属性
	static Handle<Value> getSignalPropsSyn(const Arguments& args);
	static Handle<Value> getSignalpropsAsy(const Arguments& args);
private:
	static void getTagpropsWork(uv_work_t* req);
	static void aftergetTagprops(uv_work_t* req, int status);
	static void aftergetSignalTagprops(uv_work_t* req, int status);
public:
	static Handle<Value> hisSyn(const Arguments& args);
	static Handle<Value> hisAsy(const Arguments& args);
	static Handle<Value> readHisAtTimeSyn(const Arguments& args);
	static Handle<Value> readHisAtTimeAsy(const Arguments& args);
	static Handle<Value> readHisRawSyn(const Arguments& args);
	static Handle<Value> readHisRawAsy(const Arguments& args);
	static Handle<Value> readHisProcessSyn(const Arguments& args);
	static Handle<Value> readHisProcessAsy(const Arguments& args);
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
	static Handle<Value> alarmRealSyn(const Arguments& args);
	static Handle<Value> alarmRealAsy(const Arguments& args);
	static Handle<Value> hisAlarmSyn(const Arguments& args);
	static Handle<Value> hisAlarmAsy(const Arguments& args);
	static Handle<Value> ackAlarmSyn(const Arguments& args);
	static Handle<Value> ackAlarmAsy(const Arguments& args);
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
	static Handle<Value> batRealReadSyn(const Arguments& args);
	static Handle<Value> batRealReadAsy(const Arguments& args);

	static Handle<Value> batRealWriteSyn(const Arguments& args);
	static Handle<Value> batRealWriteAsy(const Arguments& args);

	static Handle<Value> querySyn(const Arguments& args);
	static Handle<Value> queryAsy(const Arguments& args);
private:
	static void queryWork(uv_work_t* req);
	static void afterQueryWork(uv_work_t* req);
};

class SubChange:ObjectWrap{
public:
	SubChange(){}
	~SubChange(){}
	static v8::Local<v8::Object> newObj();
	static Handle<Value> New(const Arguments& args);
	static void init(Handle<Object> target);
};
#endif