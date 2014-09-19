#ifndef GUARD_H_
#define GUARD_H_
#include "node.h"
#include "v8.h"
#include "node_object_wrap.h"
#include "GuardClient/GuardClient.h"
#include <dbghelp.h>
#pragma comment(lib, "DbgHelp")
using namespace node;
using namespace v8;
using namespace std;

#define REQ_FUN_ARG(I, VAR)                                                                          \
	if (args.Length() <= (I) || !args[I]->IsFunction())                                                \
	return ThrowException(Exception::TypeError(String::New("Argument " #I " must be a function")));  \
	Local<Function> VAR = Local<Function>::Cast(args[I]);

const char* ToCString(const String::Utf8Value& value);
 CGuardClient* g_pGuardClient = NULL;
//���߳�ID
DWORD	g_dwMainThreadID = 0;

uv_timer_t *timer = new uv_timer_t();

static bool isStop = false;
//���ڴ����յ�ֹͣʱ����
class CCallbackStop : ICallbackStop
{
private:
	virtual void OnStop()
	{
		if (g_dwMainThreadID != 0)
		{
			isStop = true;
		}
	}
};


class Guard:ObjectWrap{
public:
	Guard();
	~Guard(){}
	static Handle<Value> New(const Arguments& args);
	static void init(Handle<Object> target);
	
	static Handle<Value> BeginStart(const Arguments& args);

	//���ػ����̳�ʼ�����������
	//��ʼ������ʧ��ʱ ���ػ�����Ӧ��ֹͣ�Լ� �ػ����������Ӧ�Ĺ����ڴ�����
	static Handle<Value> StartError(const Arguments& args);

	//�ȴ� �ػ�������ֹͣ����
	static Handle<Value> Wait(const Arguments& args);

	//����ֹͣ���
	static Handle<Value> SetStop(const Arguments& args);

	

	static Handle<Value> Start(const Arguments& args);

	static Handle<Value> Stop(const Arguments& args);
	static Handle<Value> onStop(const Arguments& args);

	///
	static void work(uv_work_t* req);
	static void afterWork(uv_work_t* req, int status);
	////
	//static Handle<Value> quit(const Arguments& args);
private:
	
	 static CGuardClient guardClient;
	 static CCallbackStop callbackStop;
	 static void timer_cb(uv_timer_t *handle,int status);

private:
	static Persistent<FunctionTemplate> constructorTemplate;

};

#endif
