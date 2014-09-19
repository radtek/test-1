#include "guard.h"
//主线程ID
DWORD	g_dwMainThreadID  = GetCurrentThreadId();
uv_timer_t *timer = new uv_timer_t();
bool isStop = false;
class CCallbackStop : ICallbackStop
{
private:
	virtual void OnStop()
	{
		if (g_dwMainThreadID != 0)
		{
			isStop = true;
			//::PostThreadMessage(g_dwMainThreadID, WM_QUIT, 0, 0);
		}
	}
};

CCallbackStop callbackStop;
CGuardClient guardClient;
CGuardClient* g_pGuardClient = &guardClient;
void work(uv_work_t* req)
{

}
void timer_cb(uv_timer_t *handle,int status)
{
	v8::Persistent<v8::Function> *callback = static_cast<v8::Persistent<v8::Function>*>(handle->data);
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
		node::MakeCallback(Context::GetCurrent()->Global(),*callback, 2, argv);
	}else{
		Handle<Value> argv[2];
		argv[0] = String::New("no stop");
		argv[1] = Undefined();
		//回调执行cb
		node::MakeCallback(Context::GetCurrent()->Global(),*callback, 2, argv);
	}
}

void afterWork(uv_work_t* req, int status)
{
	int r;
	timer->data = req->data;
	r = uv_timer_init(uv_default_loop(),timer);  
	assert(r == 0);  
	assert(!uv_is_active((uv_handle_t *)timer));  
	assert(!uv_is_closing((uv_handle_t *)>timer));   
	r = uv_timer_start(timer,timer_cb, 0, 100);
	r = uv_run(uv_default_loop(), UV_RUN_DEFAULT); 

}

Handle<Value> start(const Arguments& args) {
	HandleScope scope;
	guardClient.BeginStart((ICallbackStop*)&callbackStop);
	return scope.Close(Undefined());
}
Handle<Value> onStop(const Arguments& args) {

	REQ_FUN_ARG(0,callback);
	uv_loop_t *loop = uv_default_loop();
	uv_work_t req;
	req.data = &callback;
	uv_queue_work(loop, &req, work, afterWork);
	uv_run(loop, UV_RUN_DEFAULT);
	
	
}
Handle<Value> startError(const Arguments& args) {
	HandleScope scope;
	guardClient.StartError();
	return scope.Close(Undefined());
}

Handle<Value> setStop(const Arguments& args) {
	HandleScope scope;
	guardClient.SetStop();
	return scope.Close(Undefined());
}

void init(Handle<Object> exports) {
	exports->Set(String::NewSymbol("start"),
		FunctionTemplate::New(start)->GetFunction());
	exports->Set(String::NewSymbol("startError"),
		FunctionTemplate::New(startError)->GetFunction());
	exports->Set(String::NewSymbol("setStop"),
		FunctionTemplate::New(setStop)->GetFunction());
	exports->Set(String::NewSymbol("onStop"),
		FunctionTemplate::New(onStop)->GetFunction());
}
NODE_MODULE(PS_Guard, init)

