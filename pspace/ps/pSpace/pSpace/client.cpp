#include "client.h"


Persistent<FunctionTemplate> Client::constructTemplate_;
PSHANDLE Client::handle_ = PSHANDLE_UNUSED;
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

void Client::setHandle(PSHANDLE h)
{
	handle_ = h;
}
Handle<Value> Client::connectSync(const Arguments& args)
{
	HandleScope scope;
	Client* client = ObjectWrap::Unwrap<Client>(args.This());
	ConnectBaton baton(client, client->handle_, NULL);
	
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
		const char * hostName = ToCString(host);
		baton.hostname_ = hostName;

		String::Utf8Value use(args[1]);
		const char * user = ToCString(use);
		baton.user_ = user;

		String::Utf8Value pass(args[2]);
		const char * password = ToCString(pass);
		baton.password_ = password;
		if (args.Length()==4)
		{
			baton.port_ = args[3]->ToUint32()->Value();
		}
	}

	try {
		psAPI_Common_StartAPI();
		PSHANDLE hServer = PSHANDLE_UNUSED;
		PSAPIStatus nRet = psAPI_Server_Connect(PSSTR(baton.hostname_.c_str()), 
												(PSSTR)(baton.user_.c_str()), 
												(PSSTR)(baton.password_.c_str()), &hServer);
		if (PSERR(nRet))
		{
			
			Handle<Object> errObj = Object::New();
			errObj->Set(String::New("code"),Number::New(nRet));
			//目前问题不支持中文字符串
			errObj->Set(String::New("errString"),String::New(GBKToUtf8(psAPI_Commom_GetErrorDesc(nRet))));
			return scope.Close(errObj);
		}
		else
		{
			
			setHandle(hServer);
			baton.client_->handle_ = hServer;
			baton.handle_ = hServer;
		}	
	} catch(PsException &ex) {
		
		strcpy(baton.errString_,ex.what());
		return scope.Close(ThrowException(Exception::Error(String::New(GBKToUtf8(baton.errString_)))));
	} catch (const std::exception& ex) {
		
		return scope.Close(ThrowException(Exception::Error(String::New(ex.what()))));
	}
	
	Handle<Object> ps = PspaceNode::constructorTemplate->GetFunction()->NewInstance();
	(node::ObjectWrap::Unwrap<PspaceNode>(ps))->setConnection(baton.client_->handle_);
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
			strcpy(baton->errString_,psAPI_Commom_GetErrorDesc(nRet));
		}
		else
		{
			setHandle(hServer);
			baton->client_->handle_ = hServer;
			baton->handle_ = hServer;
		}	
	} catch(PsException &ex) {
		strcpy(baton->errString_,ex.what());
	}
}

void Client::afterConnect(uv_work_t* req, int status)
{
	HandleScope scope;
	ConnectBaton* baton = static_cast<ConnectBaton*>(req->data);
	baton->client_->Unref();

	Handle<Value> argv[2];
	if(baton->code_!=0) {
		argv[0] = Exception::Error(String::New(GBKToUtf8(baton->errString_)));
		argv[1] = Undefined();
	} else {
		argv[0] = Undefined();
		Handle<Object> ps = PspaceNode::constructorTemplate->GetFunction()->NewInstance();
		(node::ObjectWrap::Unwrap<PspaceNode>(ps))->setConnection(baton->client_->handle_);
		argv[1] = ps;
	}
	node::MakeCallback(Context::GetCurrent()->Global(), baton->callback_, 2, argv);
	delete baton;
}
Handle<Value> Client::connect(const Arguments& args)
{
	HandleScope scope;
	if (fun(args)){
		//异步
		Client* client = ObjectWrap::Unwrap<Client>(args.This());
		ConnectBaton* baton;
		if(args[0]->IsObject())
		{
			REQ_FUN_ARG(1, callback);
			baton = new ConnectBaton(client, client->handle_, &callback);
			REQ_OBJECT_ARG(0, settings);
			OBJ_GET_STRING(settings, "hostname", baton->hostname_);
			OBJ_GET_STRING(settings, "user", baton->user_);
			OBJ_GET_STRING(settings, "password", baton->password_);
			OBJ_GET_NUMBER(settings, "port", baton->port_, 8889);
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
				baton = new ConnectBaton(client, client->handle_, &callback);
				String::Utf8Value host(args[0]);
				const char * hostName = ToCString(host);
				baton->hostname_ = hostName;
				
				String::Utf8Value use(args[1]);
				const char * user = ToCString(use);
				baton->user_ = user;

				String::Utf8Value pass(args[2]);
				const char * password = ToCString(pass);
				baton->password_ = password;
				if (args.Length()==5)
				{
					baton->port_ = args[3]->ToUint32()->Value();
				}
		}
		client->Ref();
		uv_work_t* req = new uv_work_t();
		req->data = baton;
		uv_queue_work(uv_default_loop(), req, connectWork, (uv_after_work_cb)afterConnect);
		return scope.Close(Undefined());
	}
	else{
		return connectSync(args);
	}
	
}

extern "C" {
	static void init(Handle<Object> target) {
		Client::init(target);
		PspaceNode::init(target);
	}

	NODE_MODULE(pSpace, init);
}




