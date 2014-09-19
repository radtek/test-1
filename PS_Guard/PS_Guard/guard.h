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
//主线程ID
DWORD	g_dwMainThreadID = 0;

uv_timer_t *timer = new uv_timer_t();

static bool isStop = false;
//用于处理收到停止时处理
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

	//被守护进程初始操作完成设置
	//初始化操作失败时 被守护进程应该停止自己 守护服务清楚响应的共享内存设置
	static Handle<Value> StartError(const Arguments& args);

	//等待 守护服务发送停止命令
	static Handle<Value> Wait(const Arguments& args);

	//设置停止完成
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
