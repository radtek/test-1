#include "client.h"


Persistent<Function> Client::constructor_;
//PSHANDLE Client::handle_ = PSHANDLE_UNUSED;
//Client::Client()
//{
//	handle_ = PSHANDLE_UNUSED;
//}

void Client::init(Handle<Object> target)
{
	Isolate* isolate = target->GetIsolate();

	Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate,New);

	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	tpl->SetClassName(String::NewFromUtf8(isolate, "Client"));
	
	NODE_SET_PROTOTYPE_METHOD(tpl,"connect",connect);
	NODE_SET_PROTOTYPE_METHOD(tpl,"isConnected",isConnected);

	constructor_.Reset(isolate, tpl->GetFunction());

	target->Set(String::NewFromUtf8(isolate, "Client"),tpl->GetFunction());

}

void Client::New(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();

	if (args.IsConstructCall())
	{
		//Invoked as constructor:'new Client(...)'
		Client* pObj = new Client();
		pObj->Wrap(args.This());
		args.GetReturnValue().Set(args.This());
	}
	else
	{
		//Invoked as plain function 'Client(...)',turn into construct call

		Local<Value> argv[1] = { args[0] };
		Local<Context> context = isolate->GetCurrentContext();
		Local<Function> cons = Local<Function>::New(isolate, constructor_);
		Local<Object> result = cons->NewInstance(context, 1, argv).ToLocalChecked();
		args.GetReturnValue().Set(result);
	}

}

PSHANDLE Client::getHandle()
{
	return handle_;
}
void Client::connectSync(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();

	Client* client = ObjectWrap::Unwrap<Client>(args.This());
	ConnectBaton baton(client,NULL);
	
	if(args[0]->IsObject())
	{
		REQ_OBJECT_ARG(0, settings);
		OBJ_GET_STRING(settings, "hostname", baton.hostname_);
		OBJ_GET_STRING(settings, "user", baton.user_);
		OBJ_GET_STRING(settings, "password", baton.password_);
		OBJ_GET_NUMBER(settings, "port", baton.port_, 8889);
	}else if (args[0]->IsString() && args[1]->IsString() && args[2]->IsString())
	{
		String::Utf8Value host(args[0]);
		baton.hostname_ = ToCString(host);

		String::Utf8Value use(args[1]);
		baton.user_ = ToCString(use);

		String::Utf8Value pass(args[2]);
		baton.password_ = ToCString(pass);
		if (args.Length()==4)
		{
			baton.port_ = args[3]->ToUint32()->Value();
		}
	}
    uv_work_t req;
	try {
		req.data = &baton;
		baton.client_->Ref();
		connectWork(&req);
		baton.client_->Unref();
        //delete req;
		///if (PSERR(nRet))
		if(baton.errString_)
		{
			Local<Object> errObj = Error::newObj(args);
			errObj->Set(String::NewFromUtf8(isolate,"code"),Number::New(isolate,baton.code_));
			errObj->Set(String::NewFromUtf8(isolate,"errString"),String::NewFromUtf8(isolate, GBK2UTF8(baton.errString_->c_str()).c_str()));
			args.GetReturnValue().Set(errObj);
			return;
		}
		
	} catch(PsException &ex) {
        //delete req;
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, ex.what())));
		return;
	} catch (const std::exception& ex) {
		//delete req;
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, ex.what())));
		return;
	}

	//Local<Function> cons = Local<Function>::New(isolate, PspaceNode::constructor_);
	//Local<Object> ps = cons->NewInstance();
	//(node::ObjectWrap::Unwrap<PspaceNode>(ps))->setConnection(baton.handle_);
	//client->handle_ = baton.handle_;
	//args.GetReturnValue().Set(ps);
	//PspaceNode::NewInstance(args);

	Local<Value> argv[1] = { args[0] };
	Local<Context> context = isolate->GetCurrentContext();
	Local<Function> cons = Local<Function>::New(isolate, PspaceNode::constructor_);
	Local<Object> ps = cons->NewInstance(context, 1, argv).ToLocalChecked();
	PspaceNode* psObj = node::ObjectWrap::Unwrap<PspaceNode>(ps);
	if (psObj)
	{
		psObj->setConnection(baton.handle_);
	}	
	client->handle_ = baton.handle_;

	args.GetReturnValue().Set(ps);

	return;
}

void Client::connectWork(uv_work_t* req)
{
	ConnectBaton* baton = static_cast<ConnectBaton*>(req->data);
	
	baton->code_ = 0;
	try {
		psAPI_Common_StartAPI();
		PSHANDLE hServer = PSHANDLE_UNUSED;
		PSAPIStatus nRet = psAPI_Server_Connect( (PSSTR)(baton->hostname_.c_str()), 
												 (PSSTR)(baton->user_.c_str()), 
												 (PSSTR)(baton->password_.c_str()), &hServer);
		if (PSERR(nRet))
		{
			baton->code_ = nRet;
			baton->errString_ = new std::string(psAPI_Commom_GetErrorDesc(nRet));
		}
		else
		{
			baton->handle_ = hServer;
		}	
	} catch(PsException &ex) {
		baton->errString_ = new std::string(ex.what());
	}
}

void Client::afterConnect(uv_work_t* req, int status)
{
	Isolate* isolate = Isolate::GetCurrent();
	v8::HandleScope handleScope(isolate);
	ConnectBaton* baton = static_cast<ConnectBaton*>(req->data);
	baton->client_->Unref();

	Handle<Value> argv[2];
	if(baton->code_!=0) {
		///argv[0] = Exception::Error(String::New(GBK2UTF8((baton->errString_)->c_str())));
		argv[0] = String::NewFromUtf8(isolate,GBK2UTF8(baton->errString_->c_str()).c_str());
		argv[1] = Undefined(isolate);
	} else {
		argv[0] = Undefined(isolate);
		
		Local<Function> cons = Local<Function>::New(isolate, PspaceNode::constructor_);
		Local<Object> ps = cons->NewInstance();
		(node::ObjectWrap::Unwrap<PspaceNode>(ps))->setConnection(baton->handle_);
		argv[1] = ps;
	}
	//node::MakeCallback(isolate, isolate->GetCurrentContext()->Global(), baton->callback_, 2, argv);

	//Local<Function> cb = Local<Function>::Cast(args[0]);
	//const unsigned argc = 1;
	//Local<Value> argv[argc] = { String::NewFromUtf8(isolate, "hello world") };
	//cb->Call(Null(isolate), argc, argv);

	v8::Local<v8::Function> callback = v8::Local<v8::Function>::New(isolate, baton->callback_);
	callback->Call(isolate->GetCurrentContext()->Global(), 2, argv);
	delete baton;
    delete req;
}
void Client::connect(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	if (fun(args)){
		//“Ï≤Ω
		Client* client = ObjectWrap::Unwrap<Client>(args.This());
		ConnectBaton* baton;
		if(args[0]->IsObject())
		{
			REQ_FUN_ARG(1, callback);
			baton = new ConnectBaton(client, &callback);
			REQ_OBJECT_ARG(0, settings);
			OBJ_GET_STRING(settings, "hostname", baton->hostname_);
			OBJ_GET_STRING(settings, "user", baton->user_);
			OBJ_GET_STRING(settings, "password", baton->password_);
			//OBJ_GET_NUMBER(settings, "port", baton->port_, );
			if (settings->Has(String::NewFromUtf8(isolate,"port")))
			{
				baton->port_ = settings->Get(String::NewFromUtf8(isolate,"port"))->ToInt32()->Value();
			}
		}else if (args[0]->IsString() && args[1]->IsString() && args[2]->IsString())
		{
				int k=0;
				for (int i=0;i<args.Length();i++)
				{
					if (args[i]->IsFunction())
					{
						k=i;
						break;
					}
				}
				REQ_FUN_ARG(k, callback);
				baton = new ConnectBaton(client,&callback);
				String::Utf8Value host(args[0]);
				baton->hostname_ =  ToCString(host);
				
				String::Utf8Value use(args[1]);
				baton->user_ = ToCString(use);

				String::Utf8Value pass(args[2]);
				baton->password_ =ToCString(pass);
				if (args.Length()==5)
				{
					baton->port_ = args[3]->ToUint32()->Value();
				}
		}
		//client->Ref();
        baton->client_->Ref();
		uv_work_t* req = new uv_work_t();
		req->data = baton;
		uv_queue_work(uv_default_loop(), req, connectWork, (uv_after_work_cb)afterConnect);
		args.GetReturnValue().Set(Undefined(isolate));
		return /*scope.Close(Undefined())*/;
	}
	else{
		return connectSync(args);
	}
	
}
void Client::isConnected(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	Client* client = ObjectWrap::Unwrap<Client>(args.This());
	PSAPIStatus nRet = PSRET_OK;
	PSBOOL bConnected = PSFALSE;
	//std::cout<<client->handle_<<std::endl;
	nRet = psAPI_Server_IsConnected(client->handle_,&bConnected);
	if (PSERR(nRet) )
	{
		args.GetReturnValue().Set(Boolean::New(isolate,false));
		return;

	}
	if (bConnected)
	{
		args.GetReturnValue().Set(Boolean::New(isolate, true));
		return;
	}
	else
	{
		args.GetReturnValue().Set(Boolean::New(isolate, false));
		return;		
	}

}

extern "C" {
	static void init(Handle<Object> target) {
		Client::init(target);
		PspaceNode::init(target);
		Error::init(target);
	}

	NODE_MODULE(pSpace, init);
}




