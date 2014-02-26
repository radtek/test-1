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
	static Handle<Value> realReadSyn(const Arguments& args);
	static Handle<Value> realReadAsy(const Arguments& args);
	static void realReadWork(uv_work_t* req);
	static void afterRealRead(uv_work_t* req, int status);

	static Handle<Value> realWriteSyn(const Arguments& args);
	static Handle<Value> realWriteAsy(const Arguments& args);
	static void realWriteWork(uv_work_t* req);
	static void afterRealWrite(uv_work_t* req, int status);

	void setConnection(PSHANDLE environment);
	 static Handle<Value> close(const Arguments& args);
	static Handle<Value> isConnected(const Arguments& args);
	 void disConnect();
	PSHANDLE getHandle();
	static Persistent<FunctionTemplate> constructorTemplate;
public:
	static Handle<Value> sub(const Arguments& args);
	static void subValueWork(uv_work_t* req);
	static void aftersubValue(uv_work_t* req, int status);
	static Handle<Value> subValueAsy(const Arguments& args);

	static void subPropsWork(uv_work_t* req);
	static void aftersubProps(uv_work_t* req, int status);
	static Handle<Value> subPropsAsy(const Arguments& args);
public:
	static Handle<Value> tagAddSyn(const Arguments& args);
	static Handle<Value> tagAddAsy(const Arguments& args);
	static void tagAddWork(uv_work_t* req);
	static void afterTagAdd(uv_work_t* req, int status);
	static Handle<Value> add(const Arguments& args);

	static Handle<Value> tagDelSyn(const Arguments& args);
	static Handle<Value> tagDelAsy(const Arguments& args);
	static void tagDelWork(uv_work_t* req);
	static void afterTagDel(uv_work_t* req, int status);
	static Handle<Value> del(const Arguments& args);

	static Handle<Value> setTagPropsSyn(const Arguments& args);
	static Handle<Value> setTagpropsAsy(const Arguments& args);
	static void setTagpropsWork(uv_work_t* req);
	static void afterTagsetTagprops(uv_work_t* req, int status);

	static Handle<Value> getTagPropsSyn(const Arguments& args);
	static Handle<Value> getTagpropsAsy(const Arguments& args);
	static void getTagpropsWork(uv_work_t* req);
	static void aftergetTagprops(uv_work_t* req, int status);

	static Handle<Value> hisSyn(const Arguments& args);
	static Handle<Value> hisAsy(const Arguments& args);
	static void hisInsertWork(uv_work_t* req);
	static void afterHis(uv_work_t* req, int status);

	/*static Handle<Value> hisInsertReplaceSyn(const Arguments& args);
	static Handle<Value> hisInsertReplaceAsy(const Arguments& args);*/
	static void hisInserReplaceWork(uv_work_t* req);
	/*static void afterHisInsertReplace(uv_work_t* req, int status);*/
	static void hisReplaceWork(uv_work_t* req);

	static Handle<Value> readHisAtTimeSyn(const Arguments& args);
	static Handle<Value> readHisAtTimeAsy(const Arguments& args);
	static void readHisAtTimeWork(uv_work_t* req);
	static void afterReadHisAtTime(uv_work_t* req,int status);

	static Handle<Value> readHisRawSyn(const Arguments& args);
	static Handle<Value> readHisRawAsy(const Arguments& args);
	static void readHisRawWork(uv_work_t* req);
	static void afterReadHisRaw(uv_work_t* req,int status);

	static Handle<Value> readHisProcessSyn(const Arguments& args);
	static Handle<Value> readHisProcessAsy(const Arguments& args);
	static void readHisProcessWork(uv_work_t* req);
	//static void afterReadHisProcess(uv_work_t* req,int status);

	static Handle<Value> alarmRealSyn(const Arguments& args);
	static Handle<Value> alarmRealAsy(const Arguments& args);
	static void alarmWork(uv_work_t* req);
	static void afterAlarmReal(uv_work_t* req,int status);

	static Handle<Value> hisAlarmSyn(const Arguments& args);
	static Handle<Value> hisAlarmAsy(const Arguments& args);
	static void hisAlarmWork(uv_work_t* req);

	static Handle<Value> ackAlarmSyn(const Arguments& args);
	static Handle<Value> ackAlarmAsy(const Arguments& args);
	static void ackAlarmWork(uv_work_t* req);
	static void afterAckAlarm(uv_work_t* req,int status);
	
};

#endif