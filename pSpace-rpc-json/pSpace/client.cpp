#include "client.h"


Persistent<FunctionTemplate> Client::constructTemplate_;
//PSHANDLE Client::handle_ = PSHANDLE_UNUSED;
//Client::Client()
//{
//	handle_ = PSHANDLE_UNUSED;
//}

void Client::init(Handle<Object> target)
{
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructTemplate_ = Persistent<FunctionTemplate>::New(t);
	constructTemplate_->InstanceTemplate()->SetInternalFieldCount(1);
	constructTemplate_->SetClassName(String::NewSymbol("Client"));

	NODE_SET_PROTOTYPE_METHOD(constructTemplate_,"connect",connect);
	NODE_SET_PROTOTYPE_METHOD(constructTemplate_,"isConnected",isConnected);

	target->Set(String::NewSymbol("Client"),constructTemplate_->GetFunction());

}

Handle<Value> Client::New(const Arguments& args)
{
	HandleScope scope;
	Client *client = new Client();
	 client->Wrap(args.This());
	return scope.Close(args.This());
}

PSHANDLE Client::getHandle()
{
	return handle_;
}
Handle<Value> Client::connectSync(const Arguments& args)
{
	HandleScope scope;
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
    uv_work_t* req = new uv_work_t();
	try {
		req->data = &baton;
		baton.client_->Ref();
		connectWork(req);
		baton.client_->Unref();
        delete req;
		///if (PSERR(nRet))
		if(baton.errString_)
		{
			Local<Object> errObj = Error::newObj();
			errObj->Set(String::New("code"),Number::New(baton.code_));
			errObj->Set(String::New("errString"),String::New(GBK2UTF8(baton.errString_->c_str()).c_str()));
			return scope.Close(errObj);	
		}
		
	} catch(PsException &ex) {
        delete req;
		return scope.Close(ThrowException(Exception::Error(String::New(GBK2UTF8(ex.what()).c_str()))));
	} catch (const std::exception& ex) {
		delete req;
		return scope.Close(ThrowException(Exception::Error(String::New(ex.what()))));
	}
	Handle<Object> ps = PspaceNode::constructorTemplate->GetFunction()->NewInstance();
	(node::ObjectWrap::Unwrap<PspaceNode>(ps))->setConnection(baton.handle_);
	client->handle_ = baton.handle_;
	return scope.Close(ps);

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
	HandleScope scope;
	ConnectBaton* baton = static_cast<ConnectBaton*>(req->data);
	baton->client_->Unref();

	Handle<Value> argv[2];
	if(baton->code_!=0) {
		///argv[0] = Exception::Error(String::New(GBK2UTF8((baton->errString_)->c_str())));
		argv[0] = String::New(GBK2UTF8(baton->errString_->c_str()).c_str());
		argv[1] = Undefined();
	} else {
		argv[0] = Undefined();
		Handle<Object> ps = PspaceNode::constructorTemplate->GetFunction()->NewInstance();
		(node::ObjectWrap::Unwrap<PspaceNode>(ps))->setConnection(baton->handle_);
		baton->client_->handle_ = baton->handle_;
		argv[1] = ps;
	}
	node::MakeCallback(Context::GetCurrent()->Global(), baton->callback_, 2, argv);
	delete baton;
    delete req;
}
Handle<Value> Client::connect(const Arguments& args)
{
	HandleScope scope;
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
			if (settings->Has(String::New("port")))
			{
				baton->port_ = settings->Get(String::New("port"))->ToInt32()->Value();
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
		return scope.Close(Undefined());
	}
	else{
		return connectSync(args);
	}
	
}
Handle<Value> Client::isConnected(const Arguments& args) 
{
	HandleScope scope;
	Client* client = ObjectWrap::Unwrap<Client>(args.This());
	PSAPIStatus nRet = PSRET_OK;
	PSBOOL bConnected = PSFALSE;
	//std::cout<<client->handle_<<std::endl;
	nRet = psAPI_Server_IsConnected(client->handle_,&bConnected);
	if (PSERR(nRet) )
	{
		return scope.Close(Boolean::New(false));

	}
	if (bConnected)
	{
		return scope.Close(Boolean::New(true));
	}
	else
	{

		return scope.Close(Boolean::New(false));
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




