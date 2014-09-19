#include "guard.h"


Persistent<FunctionTemplate> Guard::constructorTemplate;

CGuardClient Guard::guardClient;
CCallbackStop Guard::callbackStop;


const char* ToCString(const String::Utf8Value& value)
{
	return *value ? *value: "string conversion failed";
}
Guard::Guard()
{
	g_pGuardClient = &guardClient;
	g_dwMainThreadID = GetCurrentThreadId();
}
void Guard::init(Handle<Object> target) {
	HandleScope scope;
	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructorTemplate = Persistent<FunctionTemplate>::New(t);
	constructorTemplate->InstanceTemplate()->SetInternalFieldCount(1);
	constructorTemplate->SetClassName(String::NewSymbol("Guard"));

	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"beginStart",BeginStart);
	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"startError",StartError);
	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"wait",Wait);
	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"setStop",SetStop);
	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"start",Start);
	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"stop",Stop);
	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"onStop",onStop);
	target->Set(String::NewSymbol("Guard"), constructorTemplate->GetFunction());
}

Handle<Value> Guard::New(const Arguments& args)
{
	HandleScope scope;
	Guard *guard = new Guard();
	guard->Wrap(args.This());
	return scope.Close(args.This());
}


Handle<Value> Guard::BeginStart(const Arguments& args)
{
	bool b = guardClient.BeginStart((ICallbackStop*)&callbackStop);
	return Boolean::New(b);
}

Handle<Value> Guard::onStop(const Arguments& args)
{
	REQ_FUN_ARG(0,callback);
	int r;
	timer->data = &callback;
	r = uv_timer_init(uv_default_loop(),timer);  
	assert(r == 0);  
	assert(!uv_is_active((uv_handle_t *)timer));  
	assert(!uv_is_closing((uv_handle_t *)>timer));   
	r = uv_timer_start(timer,timer_cb, 0, 100);
	r = uv_run(uv_default_loop(), UV_RUN_DEFAULT);  
}

void Guard::timer_cb(uv_timer_t *handle,int status)
{
	v8::Persistent<v8::Function> *fun1 = static_cast<v8::Persistent<v8::Function>*>(handle->data);
	if (isStop)
	{
		Handle<Value> argv[2];
		argv[0] = Undefined();
		argv[1] = Uint32::New(1);
		isStop = false;
		//发送消息
		::PostThreadMessage(g_dwMainThreadID, WM_QUIT, 0, 0);
		MSG msg;
		while (::GetMessage(&msg, NULL, 0, 0))
		{
			//printf("message:%d wParam:%d lParam:%d\n", msg.message, msg.wParam, msg.lParam);
		}
		uv_timer_stop(timer);
		node::MakeCallback(Context::GetCurrent()->Global(),*fun1, 2, argv);
	}else{
		Handle<Value> argv[2];
		argv[0] = String::New("no stop");
		argv[1] = Undefined();
		//回调执行cb
		node::MakeCallback(Context::GetCurrent()->Global(),*fun1, 2, argv);
	}
}


Handle<Value> Guard::StartError(const Arguments& args)
{
	guardClient.StartError();
	return Boolean::New(true);
}

//等待 守护服务发送停止命令
 Handle<Value> Guard::Wait(const Arguments& args)
 {
	 guardClient.Wait();
	 return Boolean::New(true);
 }

//设置停止完成
Handle<Value> Guard::SetStop(const Arguments& args)
{
	guardClient.SetStop();
	return Boolean::New(true);
}


Handle<Value> Guard::Start(const Arguments& args)
{
	String::Utf8Value str(args[0]);
	const char * pstr = ToCString(str);
	bool ret = guardClient.Start(pstr);
	return Boolean::New(ret);
}

Handle<Value> Guard::Stop(const Arguments& args)
{
	String::Utf8Value str(args[0]);
	const char * pstr = ToCString(str);
	bool ret = guardClient.Stop(pstr);
	return Boolean::New(ret);
}

extern "C" {
	static void init(Handle<Object> target) {
		Guard::init(target);
	}

	NODE_MODULE(PS_Guard, init);
}

