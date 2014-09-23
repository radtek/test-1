#include "guard.h"
//Ö÷Ïß³ÌID
//DWORD	g_dwMainThreadID  = GetCurrentThreadId();
uv_timer_t *timer = new uv_timer_t();
bool isStop = false;

class CCallbackStop : ICallbackStop
{
private:
	virtual void OnStop()
	{
		isStop = true;
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
	//v8::Persistent<v8::Function> *callback = static_cast<v8::Persistent<v8::Function>*>(handle->data);
	Baton *b = static_cast<Baton*>(handle->data);
	v8::Persistent<v8::Function> *callback = (v8::Persistent<v8::Function> *)&(b->cb);
	if (isStop)
	{
		Handle<Value> argv[2];
		argv[0] = Undefined();
		argv[1] = Uint32::New(1);
		b->isStop = false;
		uv_timer_stop(timer);
		guardClient.SetStop();
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
	assert(!uv_is_closing((uv_handle_t *)timer));   
	r = uv_timer_start(timer,timer_cb, 0, 100);
	r = uv_run(uv_default_loop(), UV_RUN_DEFAULT); 
	

}

Handle<Value> start(const Arguments& args) {
	HandleScope scope;
	guardClient.BeginStart((ICallbackStop*)&callbackStop);
	return scope.Close(Undefined());
}
Handle<Value> onStop(const Arguments& args) {
	HandleScope scope;
	REQ_FUN_ARG(0,callback);
	uv_work_t* req = new uv_work_t();
	baton.cb = callback;
	//req->data = &callback;
	req->data = &baton;
	uv_queue_work(uv_default_loop(), req, work, (uv_after_work_cb)afterWork);
	//uv_run(uv_default_loop(), UV_RUN_NOWAIT);
	return scope.Close(Undefined());
	
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
	//exports->Set(String::NewSymbol("setStop"),
	//	FunctionTemplate::New(setStop)->GetFunction());
	exports->Set(String::NewSymbol("onStop"),
		FunctionTemplate::New(onStop)->GetFunction());
}
NODE_MODULE(PS_Guard, init)

