#include "pSpaceNode.h"

Persistent<FunctionTemplate> PspaceNode::constructorTemplate;

void PspaceNode::init(Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructorTemplate = Persistent<FunctionTemplate>::New(t);
	constructorTemplate->InstanceTemplate()->SetInternalFieldCount(1);
	constructorTemplate->SetClassName(String::NewSymbol("PspaceNode"));

	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"isConnected",isConnected);
	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"close",close);
	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"realReadSyn",realReadSyn);
	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"realWriteSyn",realWriteSyn);
	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"read",read);
	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"write",write);
	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"subValueAsy",subValueAsy);
	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"sub",sub);
	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"add",add);
	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"del",del);
	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"setTagpropsAsy",setTagpropsAsy);

	target->Set(String::NewSymbol("PspaceNode"), constructorTemplate->GetFunction());
}

Handle<Value> PspaceNode::New(const Arguments& args) {
	HandleScope scope;
	PspaceNode *psNode = new PspaceNode();
	psNode->Wrap(args.This());
	return scope.Close(args.This());
}

PspaceNode::PspaceNode():hHanle_(PSHANDLE_UNUSED){}

PspaceNode::~PspaceNode()
{
	//关闭连接
	disConnect();
}

void PspaceNode::disConnect()
{
	PSAPIStatus nRet = PSRET_OK;
	try {
			nRet = psAPI_Server_Disconnect(this->hHanle_);
			if ( PSERR(nRet) )
			{
				std::cout<<psAPI_Commom_GetErrorDesc(nRet)<<std::endl;
				this->hHanle_ = PSHANDLE_UNUSED;
			}
			else
			{
				std::cout<<"连接"<<this->getHandle()<<":断开连接成功！"<<std::endl;
				this->hHanle_ = PSHANDLE_UNUSED;
			}		
	} catch (PsException &ex) {
	
		throw;
	}
}
	
Handle<Value> PspaceNode::close(const Arguments& args) 
{
	HandleScope scope;
	try {
		PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
		ps->disConnect();
		return scope.Close(Undefined());
	} catch (const exception& ex) {
		return scope.Close(ThrowException(Exception::Error(String::New(ex.what()))));
	}
}

PSHANDLE PspaceNode::getHandle()
{
	return hHanle_;
}

void PspaceNode::setConnection(PSHANDLE h)
{
	hHanle_ = h;
}

Handle<Value> PspaceNode::isConnected(const Arguments& args) 
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());

	if(ps->hHanle_==PSHANDLE_UNUSED) {
		return scope.Close(Boolean::New(false));
	} else {
		return scope.Close(Boolean::New(true));
	}
}

Handle<Value> PspaceNode::realReadSyn(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	RealReadBaton* rbaton;
	try {
		Handle<Object> robj = Object::New();
			rbaton = new RealReadBaton(ps,NULL);
			PS_DATA *pRealData = PSNULL;
			String::Utf8Value str(args[0]);
			const char * pstr = ToCString(str);
			std::vector<std::string> result=split(pstr,".");
		//	std::cout<<ps->hHanle_<<std::endl;
			rbaton->id = rbaton->getTagID(replace_all(result[0],"/","\\").c_str(),ps->hHanle_);
			//std::cout<<rbaton->id<<std::endl;
			if (rbaton->id==PSTAGID_UNUSED)
			{
				rbaton->code_ = -1;
				rbaton->errString = new std::string("tag not found!");
				robj->Set(String::New("code"),Number::New(-1));
				robj->Set(String::New("errString"),String::New("tag not found!"));
				return robj;
			}
			uv_work_t* req = new uv_work_t();
			req->data = rbaton;

			rbaton->psNode->Ref();
			realReadWork(req);
			rbaton->psNode->Unref();
			//如果失败
			if(rbaton->errString) {
				
				robj->Set(String::New("code"),Number::New(rbaton->code_));
				std:string *s =rbaton->errString;
				robj->Set(String::New("errString"),String::New(GBKToUtf8(s->c_str())));
				delete rbaton;
				return robj;
			}
			//  成功
			robj->Set(String::New("value"),Number::New(rbaton->db));
			robj->Set(String::New("quality"),Number::New(rbaton->quality));
			robj->Set(String::New("time"),String::New(PSTIME2STR(rbaton->time)));
			return robj;
			delete rbaton;
		} catch(PsException &ex) {
		return ThrowException(Exception::Error(String::New(ex.what())));
	}	
}
void PspaceNode::realReadWork(uv_work_t* req)
{
	RealReadBaton* rbton = static_cast<RealReadBaton*>(req->data);
	
	rbton->code_  = PSRET_OK;
	rbton->errString = NULL;
	try {
		if(rbton->psNode->hHanle_ == PSHANDLE_UNUSED) {
			throw PsException("Connection already closed");
		}else{
			PS_DATA *pRealData = PSNULL;
			PSAPIStatus nRet = psAPI_Real_Read(rbton->psNode->hHanle_, rbton->id, &pRealData);
			if (PSERR(nRet))
			{
				rbton->code_ = nRet;
				rbton->errString = new std::string(psAPI_Commom_GetErrorDesc(nRet));
			}
			rbton->db = PS_VARIANT2DOUBLE(pRealData->Value);
			rbton->time = pRealData->Time;
			rbton->quality = pRealData->Quality;
		}	
	}catch(PsException &ex) {
		rbton->errString = new string(ex.what());
	} catch (const exception& ex) {
		rbton->errString = new string(ex.what());
	} catch (...) {
		rbton->errString = new string("Unknown Error");
	}
}
void PspaceNode::afterRealRead(uv_work_t* req, int status)
{
	HandleScope scope;
	RealReadBaton* rbaton = static_cast<RealReadBaton*>(req->data);
	rbaton->psNode->Unref();
	try {
		if(rbaton->errString){
			Handle<Value> argv[2];
			argv[0] = Exception::Error(String::New(GBKToUtf8(rbaton->errString->c_str())));
			argv[1] = Undefined();
		}
		Handle<Value> argv[2];
		//结果设置
		argv[0] = Undefined();
		Handle<Object> robj = Object::New();
		robj->Set(String::New("value"),Number::New(rbaton->db));
		robj->Set(String::New("quality"),Number::New(rbaton->quality));
		robj->Set(String::New("time"),String::New(PSTIME2STR(rbaton->time)));
		argv[1] = robj;
		node::MakeCallback(Context::GetCurrent()->Global(), rbaton->callback, 2, argv);
	} catch(PsException &ex) {
		Handle<Value> argv[2];
		argv[0] = Exception::Error(String::New(ex.what()));
		argv[1] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), rbaton->callback, 2, argv);
	} catch(const exception &ex) {
		Handle<Value> argv[2];
		argv[0] = Exception::Error(String::New(GBKToUtf8(rbaton->errString->c_str())));
		argv[1] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), rbaton->callback, 2, argv);
	}
	delete rbaton;
}
Handle<Value> PspaceNode::realReadAsy(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	String::Utf8Value str(args[0]);
	const char * pstr = ToCString(str);
	std::vector<std::string> result=split(pstr,".");
	REQ_FUN_ARG(1, callback);
	RealReadBaton* rbaton;
	try {
		rbaton = new RealReadBaton(ps, &callback);
		rbaton->id = rbaton->getTagID(replace_all(result[0],"/","\\").c_str(),ps->hHanle_);
	} catch(PsException &ex) {
		return scope.Close(ThrowException(Exception::Error(String::New(ex.what()))));
	}
	uv_work_t* req = new uv_work_t();
	req->data = rbaton;
	uv_queue_work(uv_default_loop(), req, realReadWork, (uv_after_work_cb)afterRealRead);
	ps->Ref();
	return scope.Close(Undefined());

}

void PspaceNode::realWriteWork(uv_work_t* req)
{
	RealReadBaton* rbton = static_cast<RealReadBaton*>(req->data);
	rbton->code_  = PSRET_OK;
	rbton->errString = NULL;
	try {
		if(rbton->psNode->hHanle_ == PSHANDLE_UNUSED) {
			throw PsException("Connection already closed");
		}else{

			PS_VARIANT realData;
			realData.DataType = PSDATATYPE_DOUBLE;
			realData.Double = rbton->db;

			PSAPIStatus nRet = psAPI_Real_Write(rbton->psNode->hHanle_, rbton->id, &realData,&(rbton->time), &(rbton->quality));
			if (PSERR(nRet))
			{
				rbton->code_ = nRet;
				rbton->errString = new std::string(psAPI_Commom_GetErrorDesc(nRet));
			}
			rbton->tagName = rbton->getTagName(rbton->id);
		}	
	}catch(PsException &ex) {
		rbton->errString = new string(ex.what());
	} catch (const exception& ex) {
		rbton->errString = new string(ex.what());
	} catch (...) {
		rbton->errString = new string("Unknown Error");
	}
}
Handle<Value> PspaceNode::realWriteSyn(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	RealReadBaton* rbaton;
	try { 
		Handle<Object> robj = Object::New();
		rbaton = new RealReadBaton(ps,NULL);
		String::Utf8Value str(args[0]);
		const char * pstr = ToCString(str);
		std::vector<std::string> result=split(pstr,".");
		
		if (args[1]->IsObject())
		{
			REQ_OBJECT_ARG(1, settings);
			Handle<Object> obj = args[1]->ToObject();
			if (args[1]->IsObject())
			{
				REQ_OBJECT_ARG(1, settings);
				Handle<Object> obj = args[1]->ToObject();
				if(obj->Has(v8::String::New("value")))
				{
					GET_NUMBER(settings, "value", rbaton->db);
				}
				if(obj->Has(v8::String::New("quality")))
				{
					GET_NUMBER(settings,"quality",rbaton->quality);
				}
				//时间戳
				if (obj->Has(v8::String::New("time"))) 
				{
					Handle<v8::Value> value = obj->Get(v8::String::New("time"));
					String::Utf8Value strTime(value);
					std::string str = std::string(*strTime);
					PS_TIME *d = new PS_TIME[sizeof(PS_TIME)];
					STR2PSTIME(d,str);
					rbaton->time = *d;
				}
			}else{
				rbaton->db = args[1]->ToNumber()->Value();
				if (args.Length() == 3)
				{
					rbaton->quality = args[2]->ToUint32()->Value();
				}else if (args.Length() ==4 )
				{
					String::Utf8Value strTime(args[3]);
					std::string str = std::string(*strTime);
					PS_TIME *d = new PS_TIME[sizeof(PS_TIME)];
					STR2PSTIME(d,str);
					rbaton->time = *d;
				}
			}
		}
		rbaton->id = rbaton->getTagID(replace_all(result[0],"/","\\").c_str(),ps->hHanle_);
		if (rbaton->id==PSTAGID_UNUSED)
		{
			rbaton->code_ = -1;
			rbaton->errString = new std::string("tag not found!");
			robj->Set(String::New("code"),Number::New(-1));
			robj->Set(String::New("errString"),String::New("tag not found!"));
			return robj;
		}
		uv_work_t* req = new uv_work_t();
		req->data = rbaton;
		rbaton->psNode->Ref();
		realWriteWork(req);
		rbaton->psNode->Unref();
		//如果失败
		if(rbaton->errString) {
			robj->Set(String::New("code"),Number::New(rbaton->code_));
			std:string *s =rbaton->errString;
			robj->Set(String::New("errString"),String::New(GBKToUtf8(s->c_str())));
			delete rbaton;
			return robj;
		}
		//成功返回测点长名
		return String::New(replace_all(result[0],"/","\\").c_str());
		delete rbaton;
	} catch(PsException &ex) {
		return ThrowException(Exception::Error(String::New(ex.what())));
	}	
}
void PspaceNode::afterRealWrite(uv_work_t* req, int status)
{
	HandleScope scope;
	RealReadBaton* rbaton = static_cast<RealReadBaton*>(req->data);
	rbaton->psNode->Unref();
	try {
		if(rbaton->errString){
			Handle<Value> argv[2];
			argv[0] = Exception::Error(String::New(GBKToUtf8(rbaton->errString->c_str())));
			argv[1] = Undefined();
		}
		Handle<Value> argv[2];
		//结果设置
		argv[0] = Undefined();
		argv[1] = String::New(rbaton->tagName.c_str());
		node::MakeCallback(Context::GetCurrent()->Global(), rbaton->callback, 2, argv);
	} catch(PsException &ex) {
		Handle<Value> argv[2];
		argv[0] = Exception::Error(String::New(ex.what()));
		argv[1] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), rbaton->callback, 2, argv);
	} catch(const exception &ex) {
		Handle<Value> argv[2];
		argv[0] = Exception::Error(String::New(GBKToUtf8(rbaton->errString->c_str())));
		argv[1] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), rbaton->callback, 2, argv);
	}
	delete rbaton;
}
Handle<Value> PspaceNode::realWriteAsy(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	String::Utf8Value str(args[0]);
	const char * pstr = ToCString(str);
	std::vector<std::string> result=split(pstr,".");
	
	RealReadBaton* rbaton;
	try {
		
		if (args[1]->IsObject())
		{
			REQ_FUN_ARG(2, callback);
			rbaton = new RealReadBaton(ps, &callback);
			REQ_OBJECT_ARG(1, settings);
			Handle<Object> obj = args[1]->ToObject();
			if(obj->Has(v8::String::New("value")))
			{
				GET_NUMBER(settings, "value", rbaton->db);
			}
			if(obj->Has(v8::String::New("quality")))
			{
				GET_NUMBER(settings,"quality",rbaton->quality);
			}
			//时间戳
			if (obj->Has(v8::String::New("time"))) 
			{
				Handle<v8::Value> value = obj->Get(v8::String::New("time"));
				String::Utf8Value strTime(value);
				std::string str = std::string(*strTime);
				PS_TIME *d = new PS_TIME[sizeof(PS_TIME)];
				STR2PSTIME(d,str);
				rbaton->time = *d;
			}
		}else{
				int k =0;
				for (int i=0;i<args.Length();i++)
				{
					if (args[i]->IsFunction())
					{
						k=i;
						break;
					}
				}
				REQ_FUN_ARG(k, callback);
				rbaton = new RealReadBaton(ps, &callback);
				rbaton->db = args[1]->ToNumber()->Value();
				if (args.Length() == 4)
				{
					rbaton->quality = args[2]->ToUint32()->Value();
				}else if (args.Length() ==5 )
				{
					String::Utf8Value strTime(args[3]);
					std::string str = std::string(*strTime);
					PS_TIME *d = new PS_TIME[sizeof(PS_TIME)];
					STR2PSTIME(d,str);
					rbaton->time = *d;
				}
		}
		
		rbaton->id = rbaton->getTagID(replace_all(result[0],"/","\\").c_str(),ps->hHanle_);
	} catch(PsException &ex) {
		return scope.Close(ThrowException(Exception::Error(String::New(ex.what()))));
	}
	uv_work_t* req = new uv_work_t();
	req->data = rbaton;
	uv_queue_work(uv_default_loop(), req, realWriteWork, (uv_after_work_cb)afterRealWrite);
	ps->Ref();
	return scope.Close(Undefined());

}

Handle<Value> PspaceNode::read(const Arguments& args)
{
	String::Utf8Value str(args[0]);
	const char * pstr = ToCString(str);
	std::vector<std::string> result=split(pstr,".");
	if(args.Length()==1 && result[1] == "pv" && !fun(args)){
		return realReadSyn(args);
	}else if(args.Length()==2 && result[1] == "pv" && fun(args)){
		return realReadAsy(args);
	}else if (args.Length()==2 && result[1] == "props" && !fun(args))
	{
		return getTagPropsSyn(args);
	}else if (args.Length()==3 && result[1] == "props" && fun(args))
	{
		return getTagpropsAsy(args);
	}
}
Handle<Value> PspaceNode::write(const Arguments& args)
 {
	 String::Utf8Value str(args[0]);
	 const char * pstr = ToCString(str);
	 std::vector<std::string> result=split(pstr,".");
	 if(result[1] == "pv" && !fun(args)){
		 return realWriteSyn(args);
	 }else if(result[1] == "pv" && fun(args)){
		 return realWriteAsy(args);
	 }else if (args.Length()==2 && result[1] == "props" && !fun(args))
	 {
		 return setTagPropsSyn(args);
	 }else if (args.Length()==3 && result[1] == "props" && fun(args))
	 {
		 return setTagpropsAsy(args);
	 }else if (result[1] == "his" && !fun(args))
	 {
		 return hisSyn(args);
	 }else if (result[1] == "his" &&  fun(args))
	 {
		 return hisAsy(args);
	 }
 }

void PspaceNode::subValueWork(uv_work_t* req)
{
	SubBaton* sbton = static_cast<SubBaton*>(req->data);
	sbton->code_  = PSRET_OK;
	sbton->errString = NULL;
	try {
		if(sbton->psNode->hHanle_ == PSHANDLE_UNUSED) {
			throw PsException("Connection already closed");
		}else{ 
			PS_DATA *pRealDataList = PSNULL;
			PSAPIStatus *pAPIErrors = PSNULL;
			PSUINT32 nSubscribeID = 0;
			static PSUINT32 nNewSubscribe = 0;
			  PSAPIStatus nRet = PSRET_OK;
			PSUINT32 n = 0;
			PSUINT32 tagID = sbton->id;
			nRet = psAPI_Real_NewSubscribeAndRead(sbton->psNode->hHanle_, 1, &tagID,
				sbton->Real_CallbackFunction, (PSVOID*)nNewSubscribe++, &nSubscribeID, &pRealDataList, &pAPIErrors);
			if (PSERR(nRet) && nRet != PSERR_FAIL_IN_BATCH)
			{
				sbton->code_ = nRet;
				sbton->errString = new std::string(psAPI_Commom_GetErrorDesc(nRet));
			} 
			if (nRet == PSERR_FAIL_IN_BATCH)
			{
				sbton->code_ = nRet;
				sbton->errString = new std::string(psAPI_Commom_GetErrorDesc(nRet));
				psAPI_Memory_FreeAndNull((PSVOID**)pAPIErrors);
			}
			sbton->subID = nSubscribeID;
			sbton->tagName = sbton->getTagName(sbton->id);
			psAPI_Memory_FreeDataList(&pRealDataList, 1);
		}	
	}catch(PsException &ex) {
		sbton->errString = new string(ex.what());
	} catch (const exception& ex) {
		sbton->errString = new string(ex.what());
	} catch (...) {
		sbton->errString = new string("Unknown Error");
	}

}

void PspaceNode::aftersubValue(uv_work_t* req, int status)
{
	HandleScope scope;
	SubBaton* sbaton = static_cast<SubBaton*>(req->data);
	sbaton->psNode->Unref();
	try {
		if(sbaton->errString){
			Handle<Value> argv[3];
			argv[0] = Exception::Error(String::New(GBKToUtf8(sbaton->errString->c_str())));
			argv[1] = Undefined();
			argv[2] = Undefined();
		}
		Handle<Value> argv[3];
		do
		{
			while(sbaton->callbackData_.size() > 0)
			{
				argv[0] = Undefined();
				argv[1] = String::New(sbaton->tagName.c_str());
				SubBaton::CallbackData callBackData = sbaton->callbackData_.front();
				sbaton->callbackData_.pop_front();
				//将数据转化为node的数据保存
				Local<Value> dbVal = Number::New(callBackData.db);
				Local<Value> quality = Uint32::New(callBackData.quality);
				Local<Value> time = String::New(callBackData.time);
				Handle<Object> subObj = Object::New();
				subObj->Set(String::New("value"),dbVal);
				subObj->Set(String::New("quality"),quality);
				subObj->Set(String::New("time"),time);
				argv[2] = subObj;
				//异步回调执行cb
				node::MakeCallback(Context::GetCurrent()->Global(), sbaton->callback, 3, argv);
			}
		}while(1);
		
	} catch(PsException &ex) {
		Handle<Value> argv[3];
		argv[0] = Exception::Error(String::New(ex.what()));
		argv[1] = Undefined();
		argv[2] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), sbaton->callback, 3, argv);
	} catch(const exception &ex) {
		Handle<Value> argv[3];
		argv[0] = Exception::Error(String::New(sbaton->errString->c_str()));
		argv[1] = Undefined();
		argv[1] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), sbaton->callback, 3, argv);
	}
	delete sbaton;
}

Handle<Value> PspaceNode::subValueAsy(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	String::Utf8Value str(args[0]);
	const char * pstr = ToCString(str);
	std::vector<std::string> result=split(pstr,".");
	REQ_FUN_ARG(1, callback);
	SubBaton* sbaton;
	try {
		sbaton = new SubBaton(ps, &callback);
		sbaton->id = sbaton->getTagID(replace_all(result[0],"/","\\").c_str(),ps->hHanle_);
	} catch(PsException &ex) {
		return scope.Close(ThrowException(Exception::Error(String::New(ex.what()))));
	}
	uv_work_t* req = new uv_work_t();
	req->data = sbaton;
	uv_queue_work(uv_default_loop(), req, subValueWork, (uv_after_work_cb)aftersubValue);
	ps->Ref();
	return scope.Close(Number::New(sbaton->subID));
}
Handle<Value> PspaceNode::sub(const Arguments& args)
{
	String::Utf8Value str(args[0]);
	const char * pstr = ToCString(str);
	std::vector<std::string> result=split(pstr,".");
	if(args.Length()==2 && result[1] == "real" && fun(args)){
		return subValueAsy(args);
	}else if (args.Length()==2 && result[1] == "props" && fun(args))
	{
		return subPropsAsy(args);
	}
	
}

void PspaceNode::subPropsWork(uv_work_t* req)
{
	SubBaton* sbton = static_cast<SubBaton*>(req->data);
	sbton->code_  = PSRET_OK;
	sbton->errString = NULL;
	try {
		if(sbton->psNode->hHanle_ == PSHANDLE_UNUSED) {
			throw PsException("Connection already closed");
		}else{ 
			PSAPIStatus *pAPIErrors = PSNULL;
			PSUINT32 nSubscribeID = 0;
			static PSUINT32 nNewSubscribe = 0;
			PSAPIStatus nRet = PSRET_OK;
			PSUINT32 tagID = sbton->id;
			//std::cout<<"ID="<<sbton->id<<std::endl;
			nRet = psAPI_Tag_NewSubscribe(sbton->psNode->hHanle_, 1, &tagID,
				sbton->Tag_CallbackFunction, (PSVOID*)nNewSubscribe++, &nSubscribeID, &pAPIErrors);
			if (PSERR(nRet) && nRet != PSERR_FAIL_IN_BATCH)
			{
				sbton->code_ = nRet;
				sbton->errString = new std::string(psAPI_Commom_GetErrorDesc(nRet));
			} 
			if (nRet == PSERR_FAIL_IN_BATCH)
			{
				sbton->code_ = nRet;
				sbton->errString = new std::string(psAPI_Commom_GetErrorDesc(nRet));
				psAPI_Memory_FreeAndNull((PSVOID**)pAPIErrors);
			}
			sbton->tagName = sbton->getTagName(sbton->id);
			sbton->subID = nSubscribeID;
			psAPI_Memory_FreeAndNull((PSVOID**)pAPIErrors);
			
		}	
	}catch(PsException &ex) {
		sbton->errString = new string(ex.what());
	} catch (const exception& ex) {
		sbton->errString = new string(ex.what());
	} catch (...) {
		sbton->errString = new string("Unknown Error");
	}
}

void PspaceNode::aftersubProps(uv_work_t* req, int status)
{
	HandleScope scope;
	SubBaton* sbaton = static_cast<SubBaton*>(req->data);
	sbaton->psNode->Unref();
	try {
		if(sbaton->errString){
			Handle<Value> argv[3];
			argv[0] = Exception::Error(String::New(GBKToUtf8(sbaton->errString->c_str())));
			argv[1] = Undefined();
			argv[2] = Undefined();
		}
		Handle<Value> argv[3];
		//std::cout <<"subProps test size:"<<sbaton->callbackData_.size()<<std::endl;
		do
		{
			//std::cout <<"subProps test size:"<<sbaton->callbackData_.size()<<std::endl;
			while(sbaton->callbackProps_.size() > 0 && sbaton->propsOther_.size()>0)
			{
				//////////////////
				argv[0] = Undefined();
				argv[1] = String::New(sbaton->tagName.c_str());
				//std::cout<<"val1:"<<sbaton->callbackProps_.front()<<std::endl;
				Handle<Object> propObj = Object::New();
				propObj->Set(String::New("id"),Number::New(sbaton->propsOther_.front()));
				sbaton->propsOther_.pop_front();
				propObj->Set(String::New("type"),Number::New(sbaton->propsOther_.front()));
				sbaton->propsOther_.pop_front();
				propObj->Set(String::New("count"),Number::New(sbaton->propsOther_.front()));
				sbaton->propsOther_.pop_front();
				propObj->Set(String::New("value"),String::New(sbaton->callbackProps_.front()));
				argv[2] = propObj;
				sbaton->callbackProps_.pop_front();
				//异步回调执行cb
				node::MakeCallback(Context::GetCurrent()->Global(), sbaton->callback, 3, argv);

			}
		}while(1);

	} catch(PsException &ex) {
		Handle<Value> argv[3];
		argv[0] = Exception::Error(String::New(ex.what()));
		argv[1] = Undefined();
		argv[2] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), sbaton->callback, 3, argv);
	} catch(const exception &ex) {
		Handle<Value> argv[3];
		argv[0] = Exception::Error(String::New(sbaton->errString->c_str()));
		argv[1] = Undefined();
		argv[1] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), sbaton->callback, 3, argv);
	}
	delete sbaton;
}

Handle<Value> PspaceNode::subPropsAsy(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	String::Utf8Value str(args[0]);
	const char * pstr = ToCString(str);
	std::vector<std::string> result=split(pstr,".");
	REQ_FUN_ARG(1, callback);
	SubBaton* sbaton;
	try {
		sbaton = new SubBaton(ps, &callback);
		sbaton->id = sbaton->getTagID(replace_all(result[0],"/","\\").c_str(),ps->hHanle_);
	} catch(PsException &ex) {
		return scope.Close(ThrowException(Exception::Error(String::New(ex.what()))));
	}
	uv_work_t* req = new uv_work_t();
	req->data = sbaton;
	uv_queue_work(uv_default_loop(), req, subPropsWork, (uv_after_work_cb)aftersubProps);
	ps->Ref();
	return scope.Close(Number::New(sbaton->subID));
}

Handle<Value> PspaceNode::tagAddSyn(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	Tag* t;
	try { 
		Handle<Object> robj = Object::New();
		t = new Tag(ps,NULL);
		String::Utf8Value str(args[0]);
		const char * pstr = ToCString(str);
		std::vector<std::string> result=split(pstr,"/");
		int len = result.size();
		//要添加的测点名称
		t->tagName = result[len-1];
		///获取父节点
		for (int i=0;i<len-1;i++)
		{
			result[i] = "/"+result[i];

		}
		std::string com = "";
		for(int j =1;j<len-1;j++)
		{
			com+=result[j];
		}
		if (com=="")
		{
			t->parentTagId_ = PSTAGID_ROOT;
		}else
		{
			t->parentTagId_ = t->getTagID(replace_all(com,"/","\\").c_str(),t->psNode->hHanle_);
		}
		
		Handle<Object> obj = args[1]->ToObject();
		/*Handle<v8::Value> valID = obj->Get(v8::String::New("propID"));
		String::Utf8Value utf8_value1(valID);
		std::string strID = std::string(*utf8_value1);
		std::vector<std::string> resultID=split(strID,",");*/
		Handle<v8::Value> idObj = obj->Get(v8::String::New("propID"));
		static Array* valID = v8::Array::Cast(*idObj);
		//获取属性数量
		t->propCount_ = valID->Length();
		//std::cout<<t->propCount_<<std::endl;
		Handle<v8::Value> valDataType = obj->Get(v8::String::New("dataType"));
		/*String::Utf8Value utf8_value2(valDataType);
		std::string strDataType = std::string(*utf8_value2);
		std::vector<std::string> resultDataType=split(strDataType,",");*/
		static Array* dataType = v8::Array::Cast(*valDataType);

		Handle<v8::Value> values = obj->Get(v8::String::New("values"));
		static Array* arr = v8::Array::Cast(*values);
		//注意new的位置
		t->pPropIds_ = new PSUINT16[t->propCount_];
		t->pPropValues_ = new PS_VARIANT[t->propCount_];
		for (int i=0;i<t->propCount_;i++)
		{
			String::Utf8Value iStr(valID->Get(i));
			const char * idStr = ToCString(iStr);
			t->pPropIds_[i] = (PS_TAG_PROP_ENUM)finType(t->tag,idStr);

			String::Utf8Value dStr(dataType->Get(i));
			const char * dtp = ToCString(dStr);
			t->pPropValues_[i].DataType = (PS_DATATYPE_ENUM)findDataType(t->dataTypeMap_,dtp);
			/*if ((PS_DATATYPE_ENUM)findDataType(t->dataTypeMap_,resultDataType[i])==PSDATATYPE_STRING)
			{
				t->pPropValues_[i].String.Data = PSSTR(resultVal[0].c_str());
				t->pPropValues_[i].String.Length = strlen(t->pPropValues_[i].String.Data);
			}*/
			/*if (resultVal[i]=="PSTAGTYPE_NODE")
			{
				t->pPropValues_[i].UInt16 = PSTAGTYPE_NODE;
			}else if(resultVal[i]=="PSTAGTYPE_ALL")
			{
				t->pPropValues_[i].UInt16 = PSTAGTYPE_ALL;
			}*/
			switch(t->pPropValues_[i].DataType)
			{
				case PSDATATYPE_UINT16:
					{
						Local<Value> nu = arr->Get(i);
						PSUINT16 v = (PSUINT16)nu->Uint32Value();
						t->pPropValues_[i].UInt16 = v;
					}
					break;
				case  PSDATATYPE_BOOL:
					{
						Local<Value> bo = arr->Get(i);
						PS_DATATYPE_ENUM b = (PS_DATATYPE_ENUM)bo->BooleanValue();
						t->pPropValues_[i].Bool = b;
					}
					break;
				case PSDATATYPE_DOUBLE:
					{
						Local<Value> db = arr->Get(i);
						double d = db->NumberValue();
						t->pPropValues_[i].Double = d;
					}
					break;
				case PSDATATYPE_INT16:
					{
						Local<Value> int16 = arr->Get(i);
						PS_DATATYPE_ENUM i16 = (PS_DATATYPE_ENUM)int16->Int32Value();
						t->pPropValues_[i].Int16 = i16;
					}
					break;
				case PSDATATYPE_INT32:
					{
						Local<Value> int32 = arr->Get(i);
						PSINT32 i32 = (PSINT32)(int32->Int32Value());
						t->pPropValues_[i].Int32 = i32;
					}
					break;
				case PSDATATYPE_UINT32:
					{
						Local<Value> int32 = arr->Get(i);
						PSUINT32 i32 = (PSUINT32)int32->Uint32Value();
						t->pPropValues_[i].UInt32 = i32;
					}
					break;
				case PSDATATYPE_INT64:
					{
						Local<Value> int64 = arr->Get(i);
						PS_DATATYPE_ENUM i64 = (PS_DATATYPE_ENUM)int64->Int32Value();
						t->pPropValues_[i].Int64 = i64;
					}
					break;
				case PSDATATYPE_UINT64:
					{
						Local<Value> uint64 = arr->Get(i);
						PS_DATATYPE_ENUM u64 = (PS_DATATYPE_ENUM)uint64->Uint32Value();
						t->pPropValues_[i].UInt64 = u64;
					}
					break;
				case PSDATATYPE_INT8:
					{
						Local<Value> int8 = arr->Get(i);
						PS_DATATYPE_ENUM i8 = (PS_DATATYPE_ENUM)int8->Int32Value();
						t->pPropValues_[i].Int8 = i8;
					}
					break;
				case PSDATATYPE_UINT8:
					{
						Local<Value> uint8 = arr->Get(i);
						PS_DATATYPE_ENUM u8 = (PS_DATATYPE_ENUM)uint8->Uint32Value();
						t->pPropValues_[i].UInt8 = u8;
					}
					break;
				case PSDATATYPE_FLOAT:
					{
						Local<Value> fl = arr->Get(i);
						float f = (float)fl->NumberValue();
						t->pPropValues_[i].Float = f;
					}
					break;
				case  PSDATATYPE_WSTRING:
					{
						String::Utf8Value str1(arr->Get(i));
						const char * pstr1 = ToCString(str1);
						t->pPropValues_[i].String.Data = new char[strlen(pstr1)+1];
						strcpy(t->pPropValues_[i].String.Data,(PSSTR)UTF8ToGBK(pstr1).c_str());
						t->pPropValues_[i].String.Data[strlen(pstr1)] = 0;
						t->pPropValues_[i].String.Length = strlen(t->pPropValues_[i].String.Data);
					}
					break;
				case  PSDATATYPE_STRING:
					{
						String::Utf8Value str1(arr->Get(i));
						const char * pstr1 = ToCString(str1);
						t->pPropValues_[i].String.Data = new char[strlen(pstr1)+1];
						strcpy(t->pPropValues_[i].String.Data,(PSSTR)UTF8ToGBK(pstr1).c_str());
						t->pPropValues_[i].String.Data[strlen(pstr1)] = 0;
						t->pPropValues_[i].String.Length = strlen(t->pPropValues_[i].String.Data);
					}
					break;
				case PSDATATYPE_TIME:
					{
						String::Utf8Value str1(arr->Get(i));
						const char * pstr1 = ToCString(str1);
						PS_TIME *d = new PS_TIME();
						std::string s = pstr1;
						STR2PSTIME(d,s);
						t->pPropValues_[i].Time =*d;
					}
					break;
				default:
					{
						robj->Set(String::New("code"),Number::New(-1));
						std:string *s =t->errString;
						robj->Set(String::New("errString"),String::New(GBKToUtf8("数据类型为空或有误!")));
						delete t;
						return robj;

					}

			}
		}	
		uv_work_t* req = new uv_work_t();
		req->data = t;
		t->psNode->Ref();
		tagAddWork(req);
		t->psNode->Unref();
		//如果失败
		if(t->errString) {
			robj->Set(String::New("code"),Number::New(t->code_));
			string *s =t->errString;
			robj->Set(String::New("errString"),String::New(GBKToUtf8(s->c_str())));
			delete t;
			return robj;
		}
		//成功返回测点长名
		//std::cout<<t->tagName<<std::endl;
		return String::New(t->tagName.c_str());
		delete t;
	} catch(PsException &ex) {
		return ThrowException(Exception::Error(String::New(ex.what())));
	}		
}

Handle<Value> PspaceNode::tagAddAsy(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	REQ_FUN_ARG(2, callback);
	Tag* t;
	try {
		t = new Tag(ps, &callback);
		String::Utf8Value str(args[0]);
		const char * pstr = ToCString(str);
		std::vector<std::string> result=split(pstr,"/");
		int len = result.size();
		//要添加的测点名称
		t->tagName = result[len-1];
		///获取父节点
		for (int i=0;i<len-1;i++)
		{
			result[i] = "/"+result[i];

		}
		std::string com = "";
		for(int j =1;j<len-1;j++)
		{
			com+=result[j];
		}
		if (com=="")
		{
			t->parentTagId_ = PSTAGID_ROOT;
		}else
		{
			t->parentTagId_ = t->getTagID(replace_all(com,"/","\\").c_str(),t->psNode->hHanle_);
		}
		//////////////////////////////////////////////////////////////////////////
		Handle<Object> obj = args[1]->ToObject();
		Handle<v8::Value> idObj = obj->Get(v8::String::New("propID"));
		static Array* valID = v8::Array::Cast(*idObj);
		//获取属性数量
		t->propCount_ = valID->Length();
		Handle<v8::Value> valDataType = obj->Get(v8::String::New("dataType"));
		static Array* dataType = v8::Array::Cast(*valDataType);

		Handle<v8::Value> values = obj->Get(v8::String::New("values"));
		static Array* arr = v8::Array::Cast(*values);
		//注意new的位置
		t->pPropIds_ = new PSUINT16[t->propCount_];
		t->pPropValues_ = new PS_VARIANT[t->propCount_];
		for (int i=0;i<t->propCount_;i++)
		{
			String::Utf8Value iStr(valID->Get(i));
			const char * idStr = ToCString(iStr);
			t->pPropIds_[i] = (PS_TAG_PROP_ENUM)finType(t->tag,idStr);

			String::Utf8Value dStr(dataType->Get(i));
			const char * dtp = ToCString(dStr);
			t->pPropValues_[i].DataType = (PS_DATATYPE_ENUM)findDataType(t->dataTypeMap_,dtp);
			switch(t->pPropValues_[i].DataType)
			{
				case PSDATATYPE_UINT16:
					{
						Local<Value> nu = arr->Get(i);
						PSUINT16 v = (PSUINT16)nu->Uint32Value();
						t->pPropValues_[i].UInt16 = v;
					}
					break;
				case  PSDATATYPE_BOOL:
					{
						Local<Value> bo = arr->Get(i);
						bool b = (bool)bo->BooleanValue();
						t->pPropValues_[i].Bool = b;
					}
					break;
				case PSDATATYPE_DOUBLE:
					{
						Local<Value> db = arr->Get(i);
						double d = db->NumberValue();
						t->pPropValues_[i].Double = d;
					}
					break;
				case PSDATATYPE_INT16:
					{
						Local<Value> int16 = arr->Get(i);
						PS_DATATYPE_ENUM i16 = (PS_DATATYPE_ENUM)int16->Int32Value();
						t->pPropValues_[i].Int16 = i16;
					}
					break;
				case PSDATATYPE_INT32:
					{
						Local<Value> int32 = arr->Get(i);
						PSINT32 i32 = (PSINT32)int32->Int32Value();
						t->pPropValues_[i].Int32 = i32;
					}
					break;
				case PSDATATYPE_UINT32:
					{
						Local<Value> int32 = arr->Get(i);
						PSUINT32 i32 = (PSUINT32)int32->Uint32Value();
						t->pPropValues_[i].UInt32 = i32;
					}
					break;
				case PSDATATYPE_INT64:
					{
						Local<Value> int64 = arr->Get(i);
						PS_DATATYPE_ENUM i64 = (PS_DATATYPE_ENUM)int64->Int32Value();
						t->pPropValues_[i].Int64 = i64;
					}
					break;
				case PSDATATYPE_UINT64:
					{
						Local<Value> uint64 = arr->Get(i);
						PS_DATATYPE_ENUM u64 = (PS_DATATYPE_ENUM)uint64->Uint32Value();
						t->pPropValues_[i].UInt64 = u64;
					}
					break;
				case PSDATATYPE_INT8:
					{
						Local<Value> int8 = arr->Get(i);
						PS_DATATYPE_ENUM i8 = (PS_DATATYPE_ENUM)int8->Int32Value();
						t->pPropValues_[i].Int8 = i8;
					}
					break;
				case PSDATATYPE_UINT8:
					{
						Local<Value> uint8 = arr->Get(i);
						PS_DATATYPE_ENUM u8 = (PS_DATATYPE_ENUM)uint8->Uint32Value();
						t->pPropValues_[i].UInt8 = u8;
					}
					break;
				case PSDATATYPE_FLOAT:
					{
						Local<Value> fl = arr->Get(i);
						float f = (float)fl->NumberValue();
						t->pPropValues_[i].Float = f;
					}
					break;
				case  PSDATATYPE_WSTRING:
					{
						String::Utf8Value str1(arr->Get(i));
						const char * pstr1 = ToCString(str1);
						//注意指针的作用域与释放
						t->pPropValues_[i].String.Data = new char[strlen(pstr1)+1];
						strcpy(t->pPropValues_[i].String.Data,(PSSTR)UTF8ToGBK(pstr1).c_str());
						t->pPropValues_[i].String.Data[strlen(pstr1)] = 0;
						t->pPropValues_[i].String.Length = strlen(t->pPropValues_[i].String.Data);
					}
					break;
				case  PSDATATYPE_STRING:
					{
						String::Utf8Value str1(arr->Get(i));
						const char * pstr1 = ToCString(str1);
						//注意指针的作用域与释放
						t->pPropValues_[i].String.Data = new char[strlen(pstr1)+1];
						strcpy(t->pPropValues_[i].String.Data,(PSSTR)UTF8ToGBK(pstr1).c_str());
						t->pPropValues_[i].String.Data[strlen(pstr1)] = 0;
						t->pPropValues_[i].String.Length = strlen(t->pPropValues_[i].String.Data);
					}
					break;
				case PSDATATYPE_TIME:
					{
						String::Utf8Value str1(arr->Get(i));
						const char * pstr1 = ToCString(str1);
						PS_TIME *d = new PS_TIME();
						std::string s = pstr1;
						STR2PSTIME(d,s);
						t->pPropValues_[i].Time =*d;
					}
					break;
				default:
					{
						t->code_ = -1;
						t->errString = new std::string("数据类型为空或有误");
					}

			}
		}
		
	} catch(PsException &ex) {
		return scope.Close(ThrowException(Exception::Error(String::New(ex.what()))));
	}
	uv_work_t* req = new uv_work_t();
	req->data = t;
	uv_queue_work(uv_default_loop(), req, tagAddWork, (uv_after_work_cb)afterTagAdd);
	ps->Ref();
	return scope.Close(Undefined());
}

void PspaceNode::tagAddWork(uv_work_t* req)
{
	Tag* t = static_cast<Tag*>(req->data);
	t->code_  = PSRET_OK;
	t->errString = NULL;
	PSAPIStatus nRet  = PSRET_OK;
	try {
		if(t->psNode->hHanle_ == PSHANDLE_UNUSED) {
			throw PsException("Connection already closed");
		}else{
			PSUINT32 tagID = t->id;
			nRet = psAPI_Tag_Add(t->psNode->hHanle_,
				t->parentTagId_, 
				t->propCount_,
				t->pPropIds_, 
				t->pPropValues_,
				&tagID);
			if (PSERR(nRet))
			{
				t->code_ = nRet;
				t->errString = new std::string(psAPI_Commom_GetErrorDesc(nRet));
			}
			t->id = tagID;
			
			//std::cout<<"i am work!"<<std::endl;
		}	
	}catch(PsException &ex) {
		t->errString = new string(ex.what());
	} catch (const exception& ex) {
		t->errString = new string(ex.what());
	} catch (...) {
		t->errString = new string("Unknown Error");
	}
}

void PspaceNode::afterTagAdd(uv_work_t* req, int status)
{
	HandleScope scope;
	Tag* t = static_cast<Tag*>(req->data);
	t->psNode->Unref();
	try {
		
		if(t->errString){
			//std::cout<<"err:"<<(t->errString->c_str())<<std::endl;
			Handle<Value> argv[3];
			argv[0] = Exception::Error(String::New(GBKToUtf8(t->errString->c_str())));
			argv[1] = Undefined();
			argv[2] = Undefined();
			node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 3, argv);
		}else{
			Handle<Value> argv[3];
			//结果设置
			argv[0] = Undefined();
			argv[1] = String::New(t->tagName.c_str());
			argv[2] = Number::New(t->id);
			node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 3, argv);
		}
		
	} catch(PsException &ex) {
		Handle<Value> argv[3];
		argv[0] = Exception::Error(String::New(ex.what()));
		argv[1] = Undefined();
		argv[3] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 3, argv);
	} catch(const exception &ex) {
		Handle<Value> argv[3];
		argv[0] = Exception::Error(String::New(t->errString->c_str()));
		argv[1] = Undefined();
		argv[2] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 3, argv);
	}
	delete t;
}

Handle<Value> PspaceNode::add(const Arguments& args)
{
	String::Utf8Value str(args[0]);
	if(!fun(args)){
		return tagAddSyn(args);
	}else{
		return tagAddAsy(args);
	}
}


Handle<Value> PspaceNode::tagDelSyn(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	Tag* t;
	try { 
		Handle<Object> robj = Object::New();
		t = new Tag(ps,NULL);
		String::Utf8Value str(args[0]);
		const char * pstr = ToCString(str);
		std::string strName = pstr;
		t->id = t->getTagID(replace_all(strName,"/","\\").c_str(),ps->hHanle_);
		t->tagName = t->getTagName(t->id);
		
		uv_work_t* req = new uv_work_t();
		req->data = t;
		t->psNode->Ref();
		tagDelWork(req);
		t->psNode->Unref();
		//如果失败
		if(t->errString) {
			robj->Set(String::New("code"),Number::New(t->code_));
			std:string *s =t->errString;
			std::cout<<s->c_str()<<std::endl;
			robj->Set(String::New("errString"),String::New(GBKToUtf8(s->c_str())));
			delete t;
			return robj;
		}
		//成功返回测点长名
		//std::cout<<t->tagName<<std::endl;
		return String::New(t->tagName.c_str());
		delete t;
	} catch(PsException &ex) {
		return ThrowException(Exception::Error(String::New(ex.what())));
	}	
}
Handle<Value> PspaceNode::tagDelAsy(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	REQ_FUN_ARG(1, callback);
	Tag* t;
	try {
		t = new Tag(ps, &callback);
		String::Utf8Value str(args[0]);
		const char * pstr = ToCString(str);
		std::string strName = pstr;
		t->id = t->getTagID(replace_all(strName,"/","\\").c_str(),ps->hHanle_);
		t->tagName = t->getTagName(t->id);
	} catch(PsException &ex) {
		return scope.Close(ThrowException(Exception::Error(String::New(ex.what()))));
	}
	uv_work_t* req = new uv_work_t();
	req->data = t;
	uv_queue_work(uv_default_loop(), req, tagDelWork, (uv_after_work_cb)afterTagDel);
	ps->Ref();
	return scope.Close(Undefined());
}

void PspaceNode::tagDelWork(uv_work_t* req)
{
	Tag* t = static_cast<Tag*>(req->data);
	t->code_  = PSRET_OK;
	t->errString = NULL;
	PSAPIStatus nRet  = PSRET_OK;
	try {
		if(t->psNode->hHanle_ == PSHANDLE_UNUSED) {
			throw PsException("Connection already closed");
		}else{
			PSUINT32 tagID = t->id;
			nRet = psAPI_Tag_Delete(t->psNode->hHanle_, t->id);
			if (PSERR(nRet))
			{
				t->code_ = nRet;
				t->errString = new std::string(psAPI_Commom_GetErrorDesc(nRet));
			}
		}	
	}catch(PsException &ex) {
		t->errString = new string(ex.what());
	} catch (const exception& ex) {
		t->errString = new string(ex.what());
	} catch (...) {
		t->errString = new string("Unknown Error");
	}
}

void PspaceNode::afterTagDel(uv_work_t* req, int status)
{
	HandleScope scope;
	Tag* t = static_cast<Tag*>(req->data);
	t->psNode->Unref();
	try {
		if(t->errString){
			Handle<Value> argv[2];
			argv[0] = Exception::Error(String::New(GBKToUtf8(t->errString->c_str())));
			argv[1] = Undefined();
		}
		Handle<Value> argv[2];
		//结果设置
		argv[0] = Undefined();
		argv[1] = String::New(t->tagName.c_str());
		node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 2, argv);
	} catch(PsException &ex) {
		Handle<Value> argv[2];
		argv[0] = Exception::Error(String::New(ex.what()));
		argv[1] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 2, argv);
	} catch(const exception &ex) {
		Handle<Value> argv[2];
		argv[0] = Exception::Error(String::New(GBKToUtf8(t->errString->c_str())));
		argv[1] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 3, argv);
	}
	delete t;

}

Handle<Value> PspaceNode::del(const Arguments& args)
{
	String::Utf8Value str(args[0]);
	if(!fun(args)){
		return tagDelSyn(args);
	}else{
		return tagDelAsy(args);
	}
}

Handle<Value> PspaceNode::setTagPropsSyn(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	Tag* t;
	try { 
		Handle<Object> robj = Object::New();
		t = new Tag(ps,NULL);
		String::Utf8Value str(args[0]);
		const char * pstr = ToCString(str);
		std::vector<std::string> result=split(pstr,".");
		//	std::cout<<ps->hHanle_<<std::endl;
		t->id = t->getTagID(replace_all(result[0],"/","\\").c_str(),ps->hHanle_);
		t->tagName = t->getTagName(t->id);
		Handle<Object> obj = args[1]->ToObject();
		Handle<v8::Value> idObj = obj->Get(v8::String::New("propID"));
		static Array* valID = v8::Array::Cast(*idObj);
		//获取属性数量
		t->propCount_ = valID->Length();
		//std::cout<<t->propCount_<<std::endl;
		Handle<v8::Value> valDataType = obj->Get(v8::String::New("dataType"));
		
		static Array* dataType = v8::Array::Cast(*valDataType);

		Handle<v8::Value> values = obj->Get(v8::String::New("values"));
		static Array* arr = v8::Array::Cast(*values);
		//注意new的位置
		t->pPropIds_ = new PSUINT16[t->propCount_];
		t->pPropValues_ = new PS_VARIANT[t->propCount_];
		for (int i=0;i<t->propCount_;i++)
		{
			String::Utf8Value iStr(valID->Get(i));
			const char * idStr = ToCString(iStr);
			t->pPropIds_[i] = (PS_TAG_PROP_ENUM)finType(t->tag,idStr);

			String::Utf8Value dStr(dataType->Get(i));
			const char * dtp = ToCString(dStr);
			t->pPropValues_[i].DataType = (PS_DATATYPE_ENUM)findDataType(t->dataTypeMap_,dtp);
			switch(t->pPropValues_[i].DataType)
			{
				case PSDATATYPE_UINT16:
					{
						Local<Value> nu = arr->Get(i);
						PSUINT16 v = (PSUINT16)nu->Uint32Value();
						t->pPropValues_[i].UInt16 = v;
					}
					break;
				case  PSDATATYPE_BOOL:
					{
						Local<Value> bo = arr->Get(i);
						PS_DATATYPE_ENUM b = (PS_DATATYPE_ENUM)bo->BooleanValue();
						t->pPropValues_[i].Bool = b;
					}
					break;
				case PSDATATYPE_DOUBLE:
					{
						Local<Value> db = arr->Get(i);
						double d = db->NumberValue();
						t->pPropValues_[i].Double = d;
					}
					break;
				case PSDATATYPE_INT16:
					{
						Local<Value> int16 = arr->Get(i);
						PS_DATATYPE_ENUM i16 = (PS_DATATYPE_ENUM)int16->Int32Value();
						t->pPropValues_[i].Int16 = i16;
					}
					break;
				case PSDATATYPE_TIME:
					{
						String::Utf8Value str1(arr->Get(i));
						const char * pstr1 = ToCString(str1);
						PS_TIME *d = new PS_TIME();
						std::string s = pstr1;
						STR2PSTIME(d,s);
						t->pPropValues_[i].Time =*d;
					}
					break;
				case PSDATATYPE_INT32:
					{
						Local<Value> int32 = arr->Get(i);
						PS_DATATYPE_ENUM i32 = (PS_DATATYPE_ENUM)int32->Int32Value();
						t->pPropValues_[i].Int32 = i32;
					}
					break;
				case PSDATATYPE_UINT32:
					{
						Local<Value> int32 = arr->Get(i);
						PS_DATATYPE_ENUM i32 = (PS_DATATYPE_ENUM)int32->Uint32Value();
						t->pPropValues_[i].UInt32 = i32;
					}
					break;
				case PSDATATYPE_INT64:
					{
						Local<Value> int64 = arr->Get(i);
						PS_DATATYPE_ENUM i64 = (PS_DATATYPE_ENUM)int64->Int32Value();
						t->pPropValues_[i].Int64 = i64;
					}
					break;
				case PSDATATYPE_UINT64:
					{
						Local<Value> uint64 = arr->Get(i);
						PS_DATATYPE_ENUM u64 = (PS_DATATYPE_ENUM)uint64->Uint32Value();
						t->pPropValues_[i].UInt64 = u64;
					}
					break;
				case PSDATATYPE_INT8:
					{
						Local<Value> int8 = arr->Get(i);
						PS_DATATYPE_ENUM i8 = (PS_DATATYPE_ENUM)int8->Int32Value();
						t->pPropValues_[i].Int8 = i8;
					}
					break;
				case PSDATATYPE_UINT8:
					{
						Local<Value> uint8 = arr->Get(i);
						PS_DATATYPE_ENUM u8 = (PS_DATATYPE_ENUM)uint8->Uint32Value();
						t->pPropValues_[i].UInt8 = u8;
					}
					break;
				case PSDATATYPE_FLOAT:
					{
						Local<Value> fl = arr->Get(i);
						float f = (float)fl->NumberValue();
						t->pPropValues_[i].Float = f;
					}
					break;
				case  PSDATATYPE_WSTRING:
					{
						String::Utf8Value str(arr->Get(i));
						const char * pstr = ToCString(str);
						t->pPropValues_[i].String.Data = PSSTR(pstr);
						t->pPropValues_[i].String.Length = strlen(t->pPropValues_[i].String.Data);
					}
					break;
				case  PSDATATYPE_STRING:
					{
						String::Utf8Value str1(arr->Get(i));
						const char * pstr1 = ToCString(str1);
						t->pPropValues_[i].String.Data = new char[strlen(pstr1)+1];
						strcpy(t->pPropValues_[i].String.Data,(PSSTR)UTF8ToGBK(pstr1).c_str());
						t->pPropValues_[i].String.Data[strlen(pstr1)] = 0;
						t->pPropValues_[i].String.Length = strlen(t->pPropValues_[i].String.Data);
					}
					break;
				default:
					{
						robj->Set(String::New("code"),Number::New(-1));
						std:string *s =t->errString;
						robj->Set(String::New("errString"),String::New(GBKToUtf8("数据类型为空或有误!")));
						delete t;
						return robj;
					}
			}
		}	
		uv_work_t* req = new uv_work_t();
		req->data = t;
		t->psNode->Ref();
		setTagpropsWork(req);
		t->psNode->Unref();
		//如果失败
		if(t->errString) {
			robj->Set(String::New("code"),Number::New(t->code_));
			string *s =t->errString;
			robj->Set(String::New("errString"),String::New(GBKToUtf8(s->c_str())));
			delete t;
			return robj;
		}
		//成功返回测点长名
		//std::cout<<t->tagName<<std::endl;
		return String::New(t->tagName.c_str());
		delete t;
	} catch(PsException &ex) {
		return ThrowException(Exception::Error(String::New(ex.what())));
	}		
}
Handle<Value> PspaceNode::setTagpropsAsy(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	String::Utf8Value str(args[0]);
	const char * pstr = ToCString(str);
	std::vector<std::string> result=split(pstr,".");
	REQ_FUN_ARG(2, callback);
	Tag* t;
	try {
		t = new Tag(ps, &callback);
		t->id = t->getTagID(replace_all(result[0],"/","\\").c_str(),ps->hHanle_);
		//////////////////////////////////////////////////////////////////////////
		Handle<Object> obj = args[1]->ToObject();
		Handle<v8::Value> idObj = obj->Get(v8::String::New("propID"));
		static Array* valID = v8::Array::Cast(*idObj);
		//获取属性数量
		t->propCount_ = valID->Length();
		Handle<v8::Value> valDataType = obj->Get(v8::String::New("dataType"));
		static Array* dataType = v8::Array::Cast(*valDataType);

		Handle<v8::Value> values = obj->Get(v8::String::New("values"));
		static Array* arr = v8::Array::Cast(*values);
		//注意new的位置
		t->pPropIds_ = new PSUINT16[t->propCount_];
		t->pPropValues_ = new PS_VARIANT[t->propCount_];
		for (int i=0;i<t->propCount_;i++)
		{
			String::Utf8Value iStr(valID->Get(i));
			const char * idStr = ToCString(iStr);
			t->pPropIds_[i] = (PS_TAG_PROP_ENUM)finType(t->tag,idStr);
			String::Utf8Value dStr(dataType->Get(i));
			const char * dtp = ToCString(dStr);
			t->pPropValues_[i].DataType = (PS_DATATYPE_ENUM)findDataType(t->dataTypeMap_,dtp);
			switch(t->pPropValues_[i].DataType)
			{
			case PSDATATYPE_UINT16:
				{
					Local<Value> nu = arr->Get(i);
					PSUINT16 v = (PSUINT16)nu->Uint32Value();
					t->pPropValues_[i].UInt16 = v;
				}
				break;
			case  PSDATATYPE_BOOL:
				{
					Local<Value> bo = arr->Get(i);
					PS_DATATYPE_ENUM b = (PS_DATATYPE_ENUM)bo->BooleanValue();
					t->pPropValues_[i].Bool = b;
				}
				break;
			case PSDATATYPE_DOUBLE:
				{
					Local<Value> db = arr->Get(i);
					double d = db->NumberValue();
					t->pPropValues_[i].Double = d;
				}
				break;
			case PSDATATYPE_INT16:
				{
					Local<Value> int16 = arr->Get(i);
					PS_DATATYPE_ENUM i16 = (PS_DATATYPE_ENUM)int16->Int32Value();
					t->pPropValues_[i].Int16 = i16;
				}
				break;
			case PSDATATYPE_INT32:
				{
					Local<Value> int32 = arr->Get(i);
					PS_DATATYPE_ENUM i32 = (PS_DATATYPE_ENUM)int32->Int32Value();
					t->pPropValues_[i].Int32 = i32;
				}
				break;
			case PSDATATYPE_UINT32:
				{
					Local<Value> int32 = arr->Get(i);
					PS_DATATYPE_ENUM i32 = (PS_DATATYPE_ENUM)int32->Uint32Value();
					t->pPropValues_[i].UInt32 = i32;
				}
				break;
			case PSDATATYPE_INT64:
				{
					Local<Value> int64 = arr->Get(i);
					PS_DATATYPE_ENUM i64 = (PS_DATATYPE_ENUM)int64->Int32Value();
					t->pPropValues_[i].Int64 = i64;
				}
				break;
			case PSDATATYPE_UINT64:
				{
					Local<Value> uint64 = arr->Get(i);
					PS_DATATYPE_ENUM u64 = (PS_DATATYPE_ENUM)uint64->Uint32Value();
					t->pPropValues_[i].UInt64 = u64;
				}
				break;
			case PSDATATYPE_INT8:
				{
					Local<Value> int8 = arr->Get(i);
					PS_DATATYPE_ENUM i8 = (PS_DATATYPE_ENUM)int8->Int32Value();
					t->pPropValues_[i].Int8 = i8;
				}
				break;
			case PSDATATYPE_UINT8:
				{
					Local<Value> uint8 = arr->Get(i);
					PS_DATATYPE_ENUM u8 = (PS_DATATYPE_ENUM)uint8->Uint32Value();
					t->pPropValues_[i].UInt8 = u8;
				}
				break;
			case PSDATATYPE_FLOAT:
				{
					Local<Value> fl = arr->Get(i);
					float f = (float)fl->NumberValue();
					t->pPropValues_[i].Float = f;
				}
				break;
			case  PSDATATYPE_WSTRING:
				{
					String::Utf8Value str(arr->Get(i));
					const char * pstr = ToCString(str);
					t->pPropValues_[i].String.Data = PSSTR(pstr);
					t->pPropValues_[i].String.Length = strlen(t->pPropValues_[i].String.Data);
				}
				break;
			case  PSDATATYPE_STRING:
				{
					String::Utf8Value str1(arr->Get(i));
					const char * pstr1 = ToCString(str1);
					//注意指针的作用域与释放
					t->pPropValues_[i].String.Data = new char[strlen(pstr1)+1];
					strcpy(t->pPropValues_[i].String.Data,(PSSTR)UTF8ToGBK(pstr1).c_str());
					t->pPropValues_[i].String.Data[strlen(pstr1)] = 0;
					t->pPropValues_[i].String.Length = strlen(t->pPropValues_[i].String.Data);
				}
				break;
			case PSDATATYPE_TIME:
				{
					String::Utf8Value str1(arr->Get(i));
					const char * pstr1 = ToCString(str1);
					PS_TIME *d = new PS_TIME();
					std::string s = pstr1;
					STR2PSTIME(d,s);
					t->pPropValues_[i].Time =*d;
				}
				break;
			default:
				{
					t->code_ = -1;
					t->errString = new std::string("数据类型为空或有误");
				}
			}
		}

	} catch(PsException &ex) {
		return scope.Close(ThrowException(Exception::Error(String::New(ex.what()))));
	}
	uv_work_t* req = new uv_work_t();
	req->data = t;
	uv_queue_work(uv_default_loop(), req, setTagpropsWork, (uv_after_work_cb)afterTagsetTagprops);
	ps->Ref();
	return scope.Close(Undefined());
}
void PspaceNode::setTagpropsWork(uv_work_t* req)
{
	Tag* t = static_cast<Tag*>(req->data);
	t->code_  = PSRET_OK;
	t->errString = NULL;
	PSAPIStatus nRet  = PSRET_OK;
	try {
		if(t->psNode->hHanle_ == PSHANDLE_UNUSED) {
			throw PsException("Connection already closed");
		}else{
			nRet = psAPI_Tag_SetTagProps(t->psNode->hHanle_,
				t->id,t->propCount_, 
				t->pPropIds_, 
				t->pPropValues_);

			if (PSERR(nRet))
			{
				t->code_ = nRet;
				t->errString = new std::string(psAPI_Commom_GetErrorDesc(nRet));
			}
		}	
	}catch(PsException &ex) {
		t->errString = new string(ex.what());
	} catch (const exception& ex) {
		t->errString = new string(ex.what());
	} catch (...) {
		t->errString = new string("Unknown Error");
	}

}
void PspaceNode::afterTagsetTagprops(uv_work_t* req, int status)
{
	HandleScope scope;
	Tag* ts = static_cast<Tag*>(req->data);
	ts->psNode->Unref();
	try {
		if(ts->errString){
			Handle<Value> argv[3];
			argv[0] = Exception::Error(String::New(GBKToUtf8(ts->errString->c_str())));
			argv[1] = Undefined();
			argv[2] = Undefined();
			node::MakeCallback(Context::GetCurrent()->Global(), ts->callback, 3, argv);
		}else{
			Handle<Value> argv[3];
			//结果设置
			argv[0] = Undefined();
			argv[1] = String::New(ts->tagName.c_str());
			argv[2] = Number::New(ts->id);
			node::MakeCallback(Context::GetCurrent()->Global(), ts->callback, 3, argv);
		}
	} catch(PsException &ex) {
		Handle<Value> argv[3];
		argv[0] = Exception::Error(String::New(ex.what()));
		argv[1] = Undefined();
		argv[3] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), ts->callback, 3, argv);
	} catch(const exception &ex) {
		Handle<Value> argv[3];
		argv[0] = Exception::Error(String::New(ts->errString->c_str()));
		argv[1] = Undefined();
		argv[2] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), ts->callback, 3, argv);
	}
	delete ts;
	scope.Close(Undefined());
}

Handle<Value> PspaceNode::getTagPropsSyn(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	Tag* t;
	try { 
		Handle<Object> robj = Object::New();
		t = new Tag(ps,NULL);
		String::Utf8Value str(args[0]);
		const char * pstr = ToCString(str);
		std::vector<std::string> result=split(pstr,".");
		//	std::cout<<ps->hHanle_<<std::endl;
		t->id = t->getTagID(replace_all(result[0],"/","\\").c_str(),ps->hHanle_);
		t->tagName = t->getTagName(t->id);
		Handle<Object> obj = args[1]->ToObject();
		Handle<v8::Value> idObj = obj->Get(v8::String::New("propID"));
		static Array* valID = v8::Array::Cast(*idObj);
		//获取属性数量
		t->propCount_ = valID->Length();
		//std::cout<<t->propCount_<<std::endl;
		Handle<v8::Value> valDataType = obj->Get(v8::String::New("dataType"));
		static Array* dataType = v8::Array::Cast(*valDataType);
		//注意new的位置
		t->pPropIds_ = new PSUINT16[t->propCount_];
		t->pPropValues_ = PSNULL;
		for (int i=0;i<t->propCount_;i++)
		{
			String::Utf8Value iStr(valID->Get(i));
			const char * idStr = ToCString(iStr);
			t->pPropIds_[i] = (PS_TAG_PROP_ENUM)finType(t->tag,idStr);
		}	
		uv_work_t* req = new uv_work_t();
		req->data = t;
		t->psNode->Ref();
		getTagpropsWork(req);
		t->psNode->Unref();
		//如果失败
		if(t->errString) {
			robj->Set(String::New("code"),Number::New(t->code_));
			string *s =t->errString;
			robj->Set(String::New("errString"),String::New(GBKToUtf8(s->c_str())));
			delete t;
			return robj;
		}
		//成功
		Handle<Object> valObj = Object::New();
		for(int i=0;i<t->propCount_;i++)
		{
			switch(t->pPropValues_[i].DataType)
			{
			case PSDATATYPE_UINT16:
				{
					assert(t->pPropValues_[i].DataType == PSDATATYPE_UINT16);
					valObj->Set(i,Number::New(t->pPropValues_[i].UInt16));
				}
				break;
			case  PSDATATYPE_BOOL:
				{
					assert(t->pPropValues_[i].DataType == PSDATATYPE_BOOL);
					valObj->Set(i,Boolean::New(t->pPropValues_[i].Bool));
				}
				break;
			case PSDATATYPE_DOUBLE:
				{
					assert(t->pPropValues_[i].DataType == PSDATATYPE_DOUBLE);
					valObj->Set(i,Number::New(t->pPropValues_[i].Double));
				}
				break;
			case PSDATATYPE_INT16:
				{
					assert(t->pPropValues_[i].DataType == PSDATATYPE_INT16);
					valObj->Set(i,Number::New(t->pPropValues_[i].Int16));
				}
				break;
			case PSDATATYPE_TIME:
				{
					assert(t->pPropValues_[i].DataType == PSDATATYPE_TIME);
					valObj->Set(i,String::New(PSTIME2STR(t->pPropValues_[i].Time)));
				}
				break;
			case PSDATATYPE_INT32:
				{
					assert(t->pPropValues_[i].DataType == PSDATATYPE_INT32);
					valObj->Set(i,Number::New(t->pPropValues_[i].Int32));
				}
				break;
			case PSDATATYPE_UINT32:
				{
					assert(t->pPropValues_[i].DataType == PSDATATYPE_UINT32);
					valObj->Set(i,Number::New(t->pPropValues_[i].UInt32));
				}
				break;
			case PSDATATYPE_INT64:
				{
					assert(t->pPropValues_[i].DataType == PSDATATYPE_INT64);
					valObj->Set(i,Number::New(t->pPropValues_[i].Int64));
				}
				break;
			case PSDATATYPE_UINT64:
				{
					assert(t->pPropValues_[i].DataType == PSDATATYPE_UINT64);
					valObj->Set(i,Number::New(t->pPropValues_[i].UInt64));
				}
				break;
			case PSDATATYPE_INT8:
				{
					assert(t->pPropValues_[i].DataType == PSDATATYPE_INT8);
					valObj->Set(i,Number::New(t->pPropValues_[i].Int8));
				}
				break;
			case PSDATATYPE_UINT8:
				{
					assert(t->pPropValues_[i].DataType == PSDATATYPE_UINT8);
					valObj->Set(i,Number::New(t->pPropValues_[i].UInt8));
				}
				break;
			case PSDATATYPE_FLOAT:
				{
					assert(t->pPropValues_[i].DataType == PSDATATYPE_FLOAT);
					valObj->Set(i,Number::New(t->pPropValues_[i].Float));
				}
				break;
			case  PSDATATYPE_WSTRING:
				{
					assert(t->pPropValues_[i].DataType == PSDATATYPE_WSTRING);
					assert(t->pPropValues_[i].String.Data != PSNULL);
					valObj->Set(i,String::New((GBKToUtf8(t->pPropValues_[i].String.Data))));
				}
				break;
			case  PSDATATYPE_STRING:
				{
					assert(t->pPropValues_[i].DataType == PSDATATYPE_STRING);
					assert(t->pPropValues_[i].String.Data != PSNULL);
					valObj->Set(i,String::New((GBKToUtf8(t->pPropValues_[i].String.Data))));
				}
				break;
			default:
				{
					robj->Set(String::New("code"),Number::New(-1));
					std:string *s =t->errString;
					robj->Set(String::New("errString"),String::New(GBKToUtf8("数据类型为空或有误!")));
					delete t;
					return robj;
				}
		  }
		}
		return valObj;
		psAPI_Memory_FreeValueList(&(t->pPropValues_), t->propCount_);
		delete t;
	} catch(PsException &ex) {
		return ThrowException(Exception::Error(String::New(ex.what())));
	}		
}
Handle<Value> PspaceNode::getTagpropsAsy(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	String::Utf8Value str(args[0]);
	const char * pstr = ToCString(str);
	std::vector<std::string> result=split(pstr,".");
	REQ_FUN_ARG(2, callback);
	Tag* t;
	try {
		t = new Tag(ps, &callback);
		String::Utf8Value str(args[0]);
		const char * pstr = ToCString(str);
		std::vector<std::string> result=split(pstr,".");
		//	std::cout<<ps->hHanle_<<std::endl;
		t->id = t->getTagID(replace_all(result[0],"/","\\").c_str(),ps->hHanle_);
		t->tagName = t->getTagName(t->id);
		Handle<Object> obj = args[1]->ToObject();
		Handle<v8::Value> idObj = obj->Get(v8::String::New("propID"));
		static Array* valID = v8::Array::Cast(*idObj);
		//获取属性数量
		t->propCount_ = valID->Length();
		//std::cout<<t->propCount_<<std::endl;
		Handle<v8::Value> valDataType = obj->Get(v8::String::New("dataType"));
		static Array* dataType = v8::Array::Cast(*valDataType);
		//注意new的位置
		t->pPropIds_ = new PSUINT16[t->propCount_];
		t->pPropValues_ = PSNULL;
		for (int i=0;i<t->propCount_;i++)
		{
			String::Utf8Value iStr(valID->Get(i));
			const char * idStr = ToCString(iStr);
			t->pPropIds_[i] = (PS_TAG_PROP_ENUM)finType(t->tag,idStr);
		}	
	} catch(PsException &ex) {
		return scope.Close(ThrowException(Exception::Error(String::New(ex.what()))));
	}
	uv_work_t* req = new uv_work_t();
	req->data = t;
	uv_queue_work(uv_default_loop(), req, getTagpropsWork, (uv_after_work_cb)aftergetTagprops);
	ps->Ref();
	return scope.Close(Undefined());
}
void PspaceNode::getTagpropsWork(uv_work_t* req)
{
	Tag* t = static_cast<Tag*>(req->data);
	t->code_  = PSRET_OK;
	t->errString = NULL;
	PSAPIStatus nRet  = PSRET_OK;
	try {
		if(t->psNode->hHanle_ == PSHANDLE_UNUSED) {
			throw PsException("Connection already closed");
		}else{
			nRet = psAPI_Tag_GetTagProps(t->psNode->hHanle_,
				t->id, t->propCount_, t->pPropIds_, &(t->pPropValues_));
			if (PSERR(nRet))
			{
				t->code_ = nRet;
				t->errString = new std::string(psAPI_Commom_GetErrorDesc(nRet));
			}
			

		}	
	}catch(PsException &ex) {
		t->errString = new string(ex.what());
	} catch (const exception& ex) {
		t->errString = new string(ex.what());
	} catch (...) {
		t->errString = new string("Unknown Error");
	}

}
void PspaceNode::aftergetTagprops(uv_work_t* req, int status)
{
	HandleScope scope;
	Tag* t = static_cast<Tag*>(req->data);
	t->psNode->Unref();
	try {
		if(t->errString){
			//std::cout<<"err:"<<(t->errString->c_str())<<std::endl;
			Handle<Value> argv[3];
			argv[0] = Exception::Error(String::New(GBKToUtf8(t->errString->c_str())));
			argv[1] = Undefined();
			argv[2] = Undefined();
			node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 3, argv);
		}else{
			Handle<Value> argv[3];
			//结果设置
			argv[0] = Undefined();
			argv[1] = String::New(t->tagName.c_str());
			Handle<Object> valObj = Object::New();
			for(int i=0;i<t->propCount_;i++)
			{
				switch(t->pPropValues_[i].DataType)
				{
					case PSDATATYPE_UINT16:
						{
							assert(t->pPropValues_[i].DataType == PSDATATYPE_UINT16);
							valObj->Set(i,Number::New(t->pPropValues_[i].UInt16));
						}
						break;
					case  PSDATATYPE_BOOL:
						{
							assert(t->pPropValues_[i].DataType == PSDATATYPE_BOOL);
							valObj->Set(i,Boolean::New(t->pPropValues_[i].Bool));
						}
						break;
					case PSDATATYPE_DOUBLE:
						{
							assert(t->pPropValues_[i].DataType == PSDATATYPE_DOUBLE);
							valObj->Set(i,Number::New(t->pPropValues_[i].Double));
						}
						break;
					case PSDATATYPE_INT16:
						{
							assert(t->pPropValues_[i].DataType == PSDATATYPE_INT16);
							valObj->Set(i,Number::New(t->pPropValues_[i].Int16));
						}
						break;
					case PSDATATYPE_TIME:
						{
							assert(t->pPropValues_[i].DataType == PSDATATYPE_TIME);
							valObj->Set(i,String::New(PSTIME2STR(t->pPropValues_[i].Time)));
						}
						break;
					case PSDATATYPE_INT32:
						{
							assert(t->pPropValues_[i].DataType == PSDATATYPE_INT32);
							valObj->Set(i,Number::New(t->pPropValues_[i].Int32));
						}
						break;
					case PSDATATYPE_UINT32:
						{
							assert(t->pPropValues_[i].DataType == PSDATATYPE_UINT32);
							valObj->Set(i,Number::New(t->pPropValues_[i].UInt32));
						}
						break;
					case PSDATATYPE_INT64:
						{
							assert(t->pPropValues_[i].DataType == PSDATATYPE_INT64);
							valObj->Set(i,Number::New(t->pPropValues_[i].Int64));
						}
						break;
					case PSDATATYPE_UINT64:
						{
							assert(t->pPropValues_[i].DataType == PSDATATYPE_UINT64);
							valObj->Set(i,Number::New(t->pPropValues_[i].UInt64));
						}
						break;
					case PSDATATYPE_INT8:
						{
							assert(t->pPropValues_[i].DataType == PSDATATYPE_INT8);
							valObj->Set(i,Number::New(t->pPropValues_[i].Int8));
						}
						break;
					case PSDATATYPE_UINT8:
						{
							assert(t->pPropValues_[i].DataType == PSDATATYPE_UINT8);
							valObj->Set(i,Number::New(t->pPropValues_[i].UInt8));
						}
						break;
					case PSDATATYPE_FLOAT:
						{
							assert(t->pPropValues_[i].DataType == PSDATATYPE_FLOAT);
							valObj->Set(i,Number::New(t->pPropValues_[i].Float));
						}
						break;
					case  PSDATATYPE_WSTRING:
						{
							assert(t->pPropValues_[i].DataType == PSDATATYPE_WSTRING);
							assert(t->pPropValues_[i].String.Data != PSNULL);
							valObj->Set(i,String::New((GBKToUtf8(t->pPropValues_[i].String.Data))));
						}
						break;
					case  PSDATATYPE_STRING:
						{
							assert(t->pPropValues_[i].DataType == PSDATATYPE_STRING);
							assert(t->pPropValues_[i].String.Data != PSNULL);
							valObj->Set(i,String::New((GBKToUtf8(t->pPropValues_[i].String.Data))));
						}
						break;
					default:
						{
							argv[0] = String::New(GBKToUtf8("数据类型为空或不存在!"));
						}
					}
			}
			argv[2] = valObj;
			node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 3, argv);
			psAPI_Memory_FreeValueList(&(t->pPropValues_), t->propCount_);
		}

	} catch(PsException &ex) {
		Handle<Value> argv[3];
		argv[0] = Exception::Error(String::New(ex.what()));
		argv[1] = Undefined();
		argv[3] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 3, argv);
	} catch(const exception &ex) {
		Handle<Value> argv[3];
		argv[0] = Exception::Error(String::New(t->errString->c_str()));
		argv[1] = Undefined();
		argv[2] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 3, argv);
	}
	delete t;
	scope.Close(Undefined());
}

Handle<Value> PspaceNode::hisSyn(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	TagHis* t;
	try { 
		Handle<Object> robj = Object::New();
		t = new TagHis(ps,NULL);
		String::Utf8Value str(args[0]);
		const char * pstr = ToCString(str);
		std::vector<std::string> result=split(pstr,".");
		t->id = t->getTagID(replace_all(result[0],"/","\\").c_str(),ps->hHanle_);
		t->tagCount_ = 1;
		t->pTimeStamps_ = new PS_TIME[t->tagCount_];
		t->pQualities_ = new PSUINT32[t->tagCount_];
		t->pDataValues_ = new PS_VARIANT[t->tagCount_];
		t->tagIds_ = new PSUINT32[t->tagCount_];
		
		if (args[1]->IsObject())
		{
			REQ_OBJECT_ARG(1, settings);
			for (int i=0;i<t->tagCount_;i++)
			{
				t->pDataValues_[i].DataType = PSDATATYPE_DOUBLE;
				GET_NUMBER(settings, "value", t->pDataValues_[i].Double);
				std::string str = "";
				OBJ_GET_STRING(settings, "time",str);
				PS_TIME *d = new PS_TIME[sizeof(PS_TIME)];
				STR2PSTIME(d,str);
				t->pTimeStamps_[i]= *d;
				std::string str1 = "";
				OBJ_GET_STRING(settings, "quality",str1);
				t->pQualities_[i] = (PS_QUALITY_ENUM)findQuality(t->qualityMap_,str1);
				t->tagIds_[i] = t->id;
			}
		}else if (args[1]->IsNumber())
		{
			for (int i=0;i<t->tagCount_;i++)
			{	
				t->tagIds_[i] = t->id;
				t->pDataValues_[i].DataType = PSDATATYPE_DOUBLE;
				t->pDataValues_[i].Double = args[1]->ToNumber()->Value();
				String::Utf8Value str(args[2]);
				const char * pstr1 = ToCString(str);
				t->pQualities_[i] = (PS_QUALITY_ENUM)findQuality(t->qualityMap_,pstr1);
				String::Utf8Value str2(args[3]);
				const char * pstr2 = ToCString(str2);
				PS_TIME *d = new PS_TIME[sizeof(PS_TIME)];
				std::string ssTime = pstr2;
				STR2PSTIME(d,ssTime);
				t->pTimeStamps_[i] = *d;
			}
		}else{
			t->code_ = -1;
			t->errString = new std::string("参数错误!");
			robj->Set(String::New("code"),Number::New(-1));
			robj->Set(String::New("errString"),String::New(GBKToUtf8("参数错误!")));
			return robj;
		}
		if (t->id==PSTAGID_UNUSED)
		{
			t->code_ = -1;
			t->errString = new std::string("tag not found!");
			robj->Set(String::New("code"),Number::New(-1));
			robj->Set(String::New("errString"),String::New("tag not found!"));
			return robj;
		}
		uv_work_t* req = new uv_work_t();
		req->data = t;
		t->psNode->Ref();

		if (result[2] == "insertReplace")
		{
			hisInserReplaceWork(req);
		}
		if (result[2] == "replace")
		{
			hisReplaceWork(req);
		}
		if (result[2] == "insert")
		{
			hisInsertWork(req);
		}
		t->psNode->Unref();
		//如果失败
		if(t->errString) {
			robj->Set(String::New("code"),Number::New(t->code_));
			std:string *s =t->errString;
			robj->Set(String::New("errString"),String::New(GBKToUtf8(s->c_str())));
			delete t;
			return robj;
		}
		//成功返回测点长名
		return String::New(replace_all(result[0],"/","\\").c_str());
		delete t;
	} catch(PsException &ex) {
		return ThrowException(Exception::Error(String::New(ex.what())));
	}	
}
Handle<Value> PspaceNode::hisAsy(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	String::Utf8Value str(args[0]);
	const char * pstr = ToCString(str);
	std::vector<std::string> result=split(pstr,".");
	TagHis* t;
	try {
		if (args.Length()==3)
		{
			REQ_FUN_ARG(2, callback);
			t = new TagHis(ps, &callback);
		}else
		{
			REQ_FUN_ARG(4, callback);
			t = new TagHis(ps, &callback);
		}
		t->tagName = result[0];
		t->id = t->getTagID(replace_all(result[0],"/","\\").c_str(),ps->hHanle_);
		t->tagCount_ = 1;
		t->pTimeStamps_ = new PS_TIME[t->tagCount_];
		t->pQualities_ = new PSUINT32[t->tagCount_];
		t->pDataValues_ = new PS_VARIANT[t->tagCount_];
		t->tagIds_ = new PSUINT32[t->tagCount_];
		if (args[1]->IsObject())
		{
			REQ_OBJECT_ARG(1, settings);
			for (int i=0;i<t->tagCount_;i++)
			{
				t->pDataValues_[i].DataType = PSDATATYPE_DOUBLE;
				GET_NUMBER(settings, "value", t->pDataValues_[i].Double);
				std::string str = "";
				OBJ_GET_STRING(settings, "time",str);
				PS_TIME *d = new PS_TIME[sizeof(PS_TIME)];
				STR2PSTIME(d,str);
				t->pTimeStamps_[i]= *d;
				std::string str1 = "";
				OBJ_GET_STRING(settings, "quality",str1);
				t->pQualities_[i] = (PS_QUALITY_ENUM)findQuality(t->qualityMap_,str1);
				t->tagIds_[i] = t->id;
			}
		}else if (args[1]->IsNumber())
		{
			for (int i=0;i<t->tagCount_;i++)
			{	
				t->tagIds_[i] = t->id;
				t->pDataValues_[i].DataType = PSDATATYPE_DOUBLE;
				t->pDataValues_[i].Double = args[1]->ToNumber()->Value();
				String::Utf8Value str(args[2]);
				const char * pstr1 = ToCString(str);
				t->pQualities_[i] = (PS_QUALITY_ENUM)findQuality(t->qualityMap_,pstr1);
				String::Utf8Value str2(args[3]);
				const char * pstr2 = ToCString(str2);
				PS_TIME *d = new PS_TIME[sizeof(PS_TIME)];
				std::string ssTime = pstr2;
				STR2PSTIME(d,ssTime);
				t->pTimeStamps_[i] = *d;
			}
		}
		
	} catch(PsException &ex) {
		return scope.Close(ThrowException(Exception::Error(String::New(ex.what()))));
	}
	uv_work_t* req = new uv_work_t();
	req->data = t;
	if (result[2] == "insertReplace")
	{
		uv_queue_work(uv_default_loop(), req, hisInserReplaceWork, (uv_after_work_cb)afterHis);
	}
	if (result[2] == "replace")
	{
		uv_queue_work(uv_default_loop(), req, hisReplaceWork, (uv_after_work_cb)afterHis);
	}
	if (result[2] == "insert")
	{
		uv_queue_work(uv_default_loop(), req, hisInsertWork, (uv_after_work_cb)afterHis);
	}
	ps->Ref();
	return scope.Close(Undefined());
}
void PspaceNode::hisInsertWork(uv_work_t* req)
{
	TagHis* t = static_cast<TagHis*>(req->data);
	t->code_  = PSRET_OK;
	t->errString = NULL;
	PSAPIStatus nRet  = PSRET_OK;
	try {
		if(t->psNode->hHanle_ == PSHANDLE_UNUSED) {
			throw PsException("Connection already closed");
		}else{
			PSAPIStatus *pAPIErrors = PSNULL;
			nRet = psAPI_His_Insert(t->psNode->hHanle_, t->tagCount_, t->tagIds_,
				t->pDataValues_, t->pTimeStamps_, t->pQualities_, &pAPIErrors);
			
			if (PSERR(nRet) && nRet != PSERR_FAIL_IN_BATCH)
			{
				t->code_ = nRet;
				t->errString = new std::string(psAPI_Commom_GetErrorDesc(nRet));
			}else if (nRet == PSERR_FAIL_IN_BATCH)
			{
				t->code_ = nRet;
				for(int i=0;i<t->tagCount_;i++)
				{
					t->errString = new std::string(psAPI_Commom_GetErrorDesc((pAPIErrors[i])));
				}	
			}
			psAPI_Memory_FreeAndNull((PSVOID**)&pAPIErrors);
		}	
	}catch(PsException &ex) {
		t->errString = new string(ex.what());
	} catch (const exception& ex) {
		t->errString = new string(ex.what());
	} catch (...) {
		t->errString = new string("Unknown Error");
	}
}
void PspaceNode::afterHis(uv_work_t* req, int status)
{
	HandleScope scope;
	TagHis* t = static_cast<TagHis*>(req->data);
	t->psNode->Unref();
	try {
		if(t->errString){
			Handle<Value> argv[3];
			argv[0] = Exception::Error(String::New(GBKToUtf8(t->errString->c_str())));
			argv[1] = Undefined();
			argv[2] = Undefined();
			node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 3, argv);
		}else{
			Handle<Value> argv[3];
			//结果设置
			argv[0] = Undefined();
			argv[1] = String::New(t->tagName.c_str());
			argv[2] = Number::New(t->id);
			
			node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 3, argv);
		}

	} catch(PsException &ex) {
		Handle<Value> argv[3];
		argv[0] = Exception::Error(String::New(ex.what()));
		argv[1] = Undefined();
		argv[3] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 3, argv);
	} catch(const exception &ex) {
		Handle<Value> argv[3];
		argv[0] = Exception::Error(String::New(t->errString->c_str()));
		argv[1] = Undefined();
		argv[2] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 3, argv);
	}
	delete t;
	scope.Close(Undefined());
}

void PspaceNode::hisInserReplaceWork(uv_work_t* req)
{
	TagHis* t = static_cast<TagHis*>(req->data);
	t->code_  = PSRET_OK;
	t->errString = NULL;
	PSAPIStatus nRet  = PSRET_OK;
	try {
		if(t->psNode->hHanle_ == PSHANDLE_UNUSED) {
			throw PsException("Connection already closed");
		}else{
			PSAPIStatus *pAPIErrors = PSNULL;
			nRet = psAPI_His_InsertReplace(t->psNode->hHanle_, t->tagCount_, t->tagIds_,
				t->pDataValues_, t->pTimeStamps_, t->pQualities_, &pAPIErrors);

			if (PSERR(nRet) && nRet != PSERR_FAIL_IN_BATCH)
			{
				t->code_ = nRet;
				t->errString = new std::string(psAPI_Commom_GetErrorDesc(nRet));
			}else if (nRet == PSERR_FAIL_IN_BATCH)
			{
				t->code_ = nRet;
				for(int i=0;i<t->tagCount_;i++)
				{
					t->errString = new std::string(psAPI_Commom_GetErrorDesc((pAPIErrors[i])));
				}	
			}
			psAPI_Memory_FreeAndNull((PSVOID**)&pAPIErrors);
		}	
	}catch(PsException &ex) {
		t->errString = new string(ex.what());
	} catch (const exception& ex) {
		t->errString = new string(ex.what());
	} catch (...) {
		t->errString = new string("Unknown Error");
	}

}

void PspaceNode::hisReplaceWork(uv_work_t* req)
{
	TagHis* t = static_cast<TagHis*>(req->data);
	t->code_  = PSRET_OK;
	t->errString = NULL;
	PSAPIStatus nRet  = PSRET_OK;
	try {
		if(t->psNode->hHanle_ == PSHANDLE_UNUSED) {
			throw PsException("Connection already closed");
		}else{
			PSAPIStatus *pAPIErrors = PSNULL;
			nRet = psAPI_His_Replace(t->psNode->hHanle_, t->tagCount_, t->tagIds_,
				t->pDataValues_, t->pTimeStamps_, t->pQualities_, &pAPIErrors);

			if (PSERR(nRet) && nRet != PSERR_FAIL_IN_BATCH)
			{
				t->code_ = nRet;
				t->errString = new std::string(psAPI_Commom_GetErrorDesc(nRet));
			}else if (nRet == PSERR_FAIL_IN_BATCH)
			{
				t->code_ = nRet;
				for(int i=0;i<t->tagCount_;i++)
				{
					t->errString = new std::string(psAPI_Commom_GetErrorDesc((pAPIErrors[i])));
				}	
			}
			psAPI_Memory_FreeAndNull((PSVOID**)&pAPIErrors);
		}	
	}catch(PsException &ex) {
		t->errString = new string(ex.what());
	} catch (const exception& ex) {
		t->errString = new string(ex.what());
	} catch (...) {
		t->errString = new string("Unknown Error");
	}

}