#include "pSpaceNode.h"
static Persistent<Function> constructorTemplate;
//#include <boost/date_time/posix_time/posix_time.hpp>

Persistent<FunctionTemplate> PspaceNode::constructorTemplate;
std::map<PSUINT32,uv_timer_t*> timerMap;
uv_timer_t** PspaceNode::getTimer(PSUINT32 subid)
{
	std::map<PSUINT32,uv_timer_t*>::iterator iter = timerMap.find(subid);
	if (iter !=timerMap.end())
	{
		return &(iter->second);
	}
	return NULL;
}
void PspaceNode::delMapTimer(PSUINT32 subid)
{
	std::map<PSUINT32,uv_timer_t*>::iterator iter = timerMap.find(subid);
	if (iter !=timerMap.end())
	{
		timerMap.erase(iter);
	}
}
void PspaceNode::init(Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructorTemplate = Persistent<FunctionTemplate>::New(t);
	constructorTemplate->InstanceTemplate()->SetInternalFieldCount(1);
	constructorTemplate->SetClassName(String::NewSymbol("PspaceNode"));

	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"close",close);
    NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"isConnected",isConnected);
	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"realReadSyn",realReadSyn);
	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"read",read);
	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"write",write);
	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"subValueAsy",subValueAsy);
	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"sub",sub);
	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"add",add);
	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"del",del);
	NODE_SET_PROTOTYPE_METHOD(constructorTemplate,"query",query);
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
	//disConnect();
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
				//std::cout<<"连接"<<this->getHandle()<<":断开连接成功！"<<std::endl;
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

Handle<Value> PspaceNode::isConnected(const Arguments& args)
{
    HandleScope scope;
    try {
        PSAPIStatus nRet = PSRET_OK;
        PSBOOL bConnected = PSFALSE;
        PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
        nRet = psAPI_Server_IsConnected(ps->hHanle_,&bConnected);
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

Handle<Value> PspaceNode::realReadSyn(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	RealReadBaton* rbaton;
    uv_work_t* req;
	try {
		    Handle<Object> robj = Object::New();
			rbaton = new RealReadBaton(ps,NULL);
			PS_DATA *pRealData = PSNULL;
			String::Utf8Value str(args[0]);
			const char * pstr = ToCString(str);
			std::vector<std::string> result=split(pstr,".");
			rbaton->id = rbaton->getTagID(UTF8ToGBK(replace_all(result[0],"/","\\").c_str()).c_str(),ps->hHanle_);
			if (rbaton->id==PSTAGID_UNUSED)
			{
				Local<Object> errObj = Error::newObj();
				rbaton->code_ = -1;
				rbaton->errString = new std::string("tag not found!");
				
				errObj->Set(String::New("code"),Number::New(-1));
				errObj->Set(String::New("errString"),String::New("tag not found!"));
				return errObj;
			}
			req = new uv_work_t();
			req->data = rbaton;

			rbaton->psNode->Ref();
			realReadWork(req);
			rbaton->psNode->Unref();
			//如果失败
			if(rbaton->errString) {
				Local<Object> errObj = Error::newObj();
				errObj->Set(String::New("code"),Number::New(rbaton->code_));
				std:string *s =rbaton->errString;
				errObj->Set(String::New("errString"),String::New(GBK2UTF8(s->c_str()).c_str()));
				delete rbaton;
                delete req;
				return errObj;
			}
			//成功
            Local<Object> resObj = getRealObj(rbaton->realData_);
            delete req;
            delete rbaton;
			return resObj;
		} catch(PsException &ex) {
            delete rbaton;
            delete req;
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
			PSAPIStatus nRet = psAPI_Real_Read(rbton->psNode->hHanle_, rbton->id, &(rbton->realData_));
			if (PSERR(nRet))
			{
				rbton->code_ = nRet;
				rbton->errString = new std::string(psAPI_Commom_GetErrorDesc(nRet));
			}
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
			Handle<Value> argv[3];
			argv[0] = Exception::Error(String::New(GBK2UTF8(rbaton->errString->c_str()).c_str()));
			argv[1] = Undefined();
			argv[2] = Undefined();
			node::MakeCallback(Context::GetCurrent()->Global(), rbaton->callback, 3, argv);
		}else{
			
			Handle<Value> argv[3];
			//结果设置
			argv[0] = Undefined();
			argv[1] = String::New(rbaton->tagName.c_str());
			argv[2] = getRealObj(rbaton->realData_);
			node::MakeCallback(Context::GetCurrent()->Global(), rbaton->callback, 3, argv);
		}
	} catch(PsException &ex) {
		Handle<Value> argv[3];
		argv[0] = Exception::Error(String::New(ex.what()));
		argv[1] = Undefined();
		argv[2] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), rbaton->callback, 3, argv);
	} catch(const exception &ex) {
		Handle<Value> argv[3];
		argv[0] = Exception::Error(String::New(GBK2UTF8(rbaton->errString->c_str()).c_str()));
		argv[1] = Undefined();
		argv[2] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), rbaton->callback, 3, argv);
	}
    if (rbaton != NULL)
    {
        delete rbaton;
        rbaton = NULL;
    }
    if (req != NULL)
    {
        delete req;
        req = NULL;
    }
}
Handle<Value> PspaceNode::realReadAsy(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	String::Utf8Value str(args[0]);
	const char * pstr = ToCString(str);
	std::vector<std::string> result=split(pstr,".");
	REQ_FUN_ARG(1, callback);
	RealReadBaton* rbaton = NULL;
    uv_work_t* req = NULL;
	try {
		rbaton = new RealReadBaton(ps, &callback);
		rbaton->tagName = result[0];
		rbaton->id = rbaton->getTagID(UTF8ToGBK(replace_all(result[0],"/","\\").c_str()).c_str(),ps->hHanle_);
	    req = new uv_work_t();
	    req->data = rbaton;
	    uv_queue_work(uv_default_loop(), req, realReadWork, (uv_after_work_cb)afterRealRead);
	    ps->Ref();
	    return scope.Close(Undefined());
    } catch(PsException &ex) {
        if (rbaton != NULL)
        {
            delete rbaton;
            rbaton = NULL;
        }
        if (req != NULL)
        {
            delete req;
            req = NULL;
        }
        return scope.Close(ThrowException(Exception::Error(String::New(ex.what()))));
    }

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
			
			PSAPIStatus nRet = psAPI_Real_Write(rbton->psNode->hHanle_, rbton->id, &(rbton->varData_),(rbton->time_), &(rbton->quality_));
			if (PSERR(nRet))
			{
				
				rbton->code_ = nRet;
				rbton->errString = new std::string(psAPI_Commom_GetErrorDesc(nRet));
				
			}
			//rbton->tagName = rbton->getTagName(rbton->id);
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
	RealReadBaton* rbaton = NULL;
    uv_work_t* req = NULL;
	try { 
		Handle<Object> robj = Object::New();
		rbaton = new RealReadBaton(ps,NULL);
		String::Utf8Value str(args[0]);
		const char * pstr = ToCString(str);
		std::vector<std::string> result=split(pstr,".");
		std::string strName = result[0];
		if (args[1]->IsObject())
		{
			REQ_OBJECT_ARG(1, settings);
			if(settings->Has(v8::String::New("value")))
			{
				Local<Value> obj = settings->Get(String::New("value"));
				if (obj->IsInt32())
				{
					rbaton->varData_.DataType = PSDATATYPE_INT32;
					GET_INTER(settings, "value", rbaton->varData_.Int32);
				}else if (obj->IsBoolean())
				{
					rbaton->varData_.DataType = PSDATATYPE_BOOL;
					rbaton->varData_.Bool =obj->ToBoolean()->Value();
				}else if (obj->IsNumber())
				{
					
					rbaton->varData_.DataType = PSDATATYPE_DOUBLE;
					GET_NUMBER(settings, "value", rbaton->varData_.Double);
					
				}else if (obj->IsString())
				{
					rbaton->varData_.DataType = PSDATATYPE_STRING;
					String::Utf8Value tmpStr(settings->Get(String::New("value")));
					const char *pstr = ToCString(tmpStr);
					rbaton->varData_.String.Data = new char[strlen(pstr)+1];
					strcpy(rbaton->varData_.String.Data,pstr);
					rbaton->varData_.String.Data[strlen(pstr)]=0;
					rbaton->varData_.String.Length = strlen(rbaton->varData_.String.Data);
				}	
				
			}
			if(settings->Has(v8::String::New("quality")))
			{
                Local<Value> qObj = settings->Get(String::New("quality"));
                if (qObj->IsInt32())
                {
                    GET_INTER(settings, "quality", rbaton->quality_);
                }
                else if (qObj->IsString())
                {
                    String::Utf8Value str(settings->Get(String::New("quality")));
                    const char * pstr1 = ToCString(str);
                    rbaton->quality_ = (PS_QUALITY_ENUM)rbaton->getQuality(pstr1);
                }
			}
			//时间戳
			if (settings->Has(v8::String::New("time"))) 
			{
				rbaton->time_ = new PS_TIME;
				Local<v8::Value> val = settings->Get(v8::String::New("time"));
				v8::Local<v8::Date> date = v8::Local<v8::Date>::Cast(val);
				V8DATE2PSTIME(rbaton->time_,date);
			}
		}else{
			Local<Value> valObj = args[1];
			if (valObj->IsInt32())
			{
				rbaton->varData_.DataType = PSDATATYPE_INT32;
				rbaton->varData_.Int32 = valObj->ToInt32()->Value();
			}else if (valObj->IsBoolean())
			{
				rbaton->varData_.DataType = PSDATATYPE_BOOL;
				rbaton->varData_.Bool = valObj->ToBoolean()->Value(); 
			}else if (valObj->IsNumber())
			{
				rbaton->varData_.DataType = PSDATATYPE_DOUBLE;
				rbaton->varData_.Double = valObj->ToNumber()->Value();
			}else if (valObj->IsString())
			{
				rbaton->varData_.DataType = PSDATATYPE_STRING;
				String::Utf8Value tmpStr(valObj);
				const char *pstr = ToCString(tmpStr);
				rbaton->varData_.String.Data = new char[strlen(pstr)+1];
				strcpy(rbaton->varData_.String.Data,pstr);
				rbaton->varData_.String.Data[strlen(pstr)]=0;
				rbaton->varData_.String.Length = strlen(rbaton->varData_.String.Data);
			}
			if (!args[2]->IsNull() && args[2]->IsString())
			{
				String::Utf8Value str(args[2]);
				const char * pstr1 = ToCString(str);
				rbaton->quality_ = (PS_QUALITY_ENUM)rbaton->getQuality(pstr1);
			}else if (!args[3]->IsNull() && args[3]->IsDate())
			{
				rbaton->time_ = new PS_TIME;
				v8::Local<v8::Date> date = v8::Local<v8::Date>::Cast(args[3]);
				V8DATE2PSTIME(rbaton->time_,date);
			}
		}
		rbaton->id = rbaton->getTagID(UTF8ToGBK(replace_all(result[0],"/","\\").c_str()).c_str(),ps->hHanle_);
		if (rbaton->id==PSTAGID_UNUSED)
		{
			rbaton->code_ = -1;
			rbaton->errString = new std::string("tag not found!");
			Local<Object> errObj = Error::newObj();
			errObj->Set(String::New("code"),Number::New(-1));
			errObj->Set(String::New("errString"),String::New("tag not found!"));
			return errObj;
		}
		
		req = new uv_work_t();
		req->data = rbaton;
		rbaton->psNode->Ref();
		realWriteWork(req);
		rbaton->psNode->Unref();
		//如果失败
		if(rbaton->errString) {
			Local<Object> errObj = Error::newObj();
			errObj->Set(String::New("code"),Number::New(rbaton->code_));
			errObj->Set(String::New("errString"),String::New(GBK2UTF8(rbaton->errString->c_str()).c_str()));
            FREE_MEMORY(rbaton);
            FREE_MEMORY(req);
			return errObj;
		}
		//成功返回测点长名
        FREE_MEMORY(rbaton);
        FREE_MEMORY(req);
		return String::New(strName.c_str());
	} catch(PsException &ex) {
        FREE_MEMORY(rbaton);
        FREE_MEMORY(req);
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
			argv[0] = Exception::Error(String::New(GBK2UTF8(rbaton->errString->c_str()).c_str()));
			argv[1] = Undefined();
			node::MakeCallback(Context::GetCurrent()->Global(), rbaton->callback, 2, argv);
		}else{
			Handle<Value> argv[2];
			//结果设置
			argv[0] = Undefined();
			argv[1] = String::New(rbaton->tagName.c_str());
			node::MakeCallback(Context::GetCurrent()->Global(), rbaton->callback, 2, argv);
		}
	} catch(PsException &ex) {
		Handle<Value> argv[2];
		argv[0] = Exception::Error(String::New(ex.what()));
		argv[1] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), rbaton->callback, 2, argv);
	} catch(const exception &ex) {
		Handle<Value> argv[2];
		argv[0] = Exception::Error(String::New(GBK2UTF8(rbaton->errString->c_str()).c_str()));
		argv[1] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), rbaton->callback, 2, argv);
	}
    FREE_MEMORY(rbaton);
    FREE_MEMORY(req);
	
}
Handle<Value> PspaceNode::realWriteAsy(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	String::Utf8Value str(args[0]);
	const char * pstr = ToCString(str);
	std::vector<std::string> result=split(pstr,".");
	
	RealReadBaton* rbaton = NULL;
	try {
		REQ_FUN_ARG(args.Length()-1, callback);
		rbaton = new RealReadBaton(ps, &callback);
		rbaton->tagName = result[0];
		if (args[1]->IsObject())
		{
			
			REQ_OBJECT_ARG(1, settings);
			if(settings->Has(v8::String::New("value")))
			{
				Local<Value> obj = settings->Get(String::New("value"));
				if (obj->IsInt32())
				{
					rbaton->varData_.DataType = PSDATATYPE_INT32;
					GET_INTER(settings, "value", rbaton->varData_.Int32);
				}else if (obj->IsBoolean())
				{
					rbaton->varData_.DataType = PSDATATYPE_BOOL;
					rbaton->varData_.Bool =obj->ToBoolean()->Value();
				}else if (obj->IsNumber())
				{
					rbaton->varData_.DataType = PSDATATYPE_DOUBLE;
					GET_NUMBER(settings, "value", rbaton->varData_.Double);

				}else if (obj->IsString())
				{
					rbaton->varData_.DataType = PSDATATYPE_STRING;
					String::Utf8Value tmpStr(settings->Get(String::New("value")));
					const char *pstr = ToCString(tmpStr);
					rbaton->varData_.String.Data = new char[strlen(pstr)+1];
					strcpy(rbaton->varData_.String.Data,pstr);
					rbaton->varData_.String.Data[strlen(pstr)]=0;
					rbaton->varData_.String.Length = strlen(rbaton->varData_.String.Data);
				}	

			}
			
			if(settings->Has(v8::String::New("quality")))
			{
                Local<Value> qObj = settings->Get(String::New("quality"));
                if (qObj->IsInt32())
                {
                    GET_INTER(settings, "quality", rbaton->quality_);
                }
                else if (qObj->IsString())
                {
                    String::Utf8Value str(settings->Get(String::New("quality")));
                    const char * pstr1 = ToCString(str);
                    rbaton->quality_ = (PS_QUALITY_ENUM)rbaton->getQuality(pstr1);
                }
			}
			//时间戳
			if (settings->Has(v8::String::New("time"))) 
			{
				rbaton->time_ = new PS_TIME;
				Local<v8::Value> value = settings->Get(v8::String::New("time"));
				v8::Local<v8::Date> date = v8::Local<v8::Date>::Cast(value);
				V8DATE2PSTIME(rbaton->time_,date);
			}
		
		}else{
			
			Local<Value> valObj = args[1];
			if (valObj->IsInt32())
			{
				rbaton->varData_.DataType = PSDATATYPE_INT32;
				rbaton->varData_.Int32 = valObj->ToInt32()->Value();
			}else if (valObj->IsBoolean())
			{
				rbaton->varData_.DataType = PSDATATYPE_BOOL;
				rbaton->varData_.Bool = valObj->ToBoolean()->Value(); 
			}else if (valObj->IsNumber())
			{
				
				rbaton->varData_.DataType = PSDATATYPE_DOUBLE;
				rbaton->varData_.Double = valObj->ToNumber()->Value();
				
			}else if (valObj->IsString())
			{
				rbaton->varData_.DataType = PSDATATYPE_STRING;
				String::Utf8Value tmpStr(valObj);
				const char *pstr = ToCString(tmpStr);
				rbaton->varData_.String.Data = new char[strlen(pstr)+1];
				strcpy(rbaton->varData_.String.Data,pstr);
				rbaton->varData_.String.Data[strlen(pstr)]=0;
				rbaton->varData_.String.Length = strlen(rbaton->varData_.String.Data);
			}
			if (!args[2]->IsNull()&& args[2]->IsString())
			{
				String::Utf8Value str(args[2]);
				const char * pstr1 = ToCString(str);
				rbaton->quality_ = (PS_QUALITY_ENUM)rbaton->getQuality(pstr1);
			}else if (!args[3]->IsNull()&& args[3]->IsDate())
			{
				rbaton->time_ = new PS_TIME;
				v8::Local<v8::Date> date = v8::Local<v8::Date>::Cast(args[3]);
				V8DATE2PSTIME(rbaton->time_,date);
			}	
		}
		rbaton->id = rbaton->getTagID(UTF8ToGBK(replace_all(result[0],"/","\\").c_str()).c_str(),ps->hHanle_);
	} catch(PsException &ex) {
		return scope.Close(ThrowException(Exception::Error(String::New(ex.what()))));
	}
    uv_work_t* req = NULL;
	req = new uv_work_t();
	req->data = rbaton;
	
	uv_queue_work(uv_default_loop(), req, realWriteWork, (uv_after_work_cb)afterRealWrite);
	ps->Ref();
	return scope.Close(Undefined());

}
Handle<Value> PspaceNode::query(const Arguments& args)
{
	if (!fun(args))
	{
		return querySyn(args);
	}
	if (fun(args))
	{
		return queryAsy(args);
	}
}
Handle<Value> PspaceNode::read(const Arguments& args)
{
	if (args[0]->IsArray())
	{
		REQ_ARRAY_ARG(0,arr);
		String::Utf8Value str(arr->Get(0));
		const char * pstr = ToCString(str);
		std::vector<std::string> result=split(pstr,".");
		if (result.size()!=2)
		{
			if (!fun(args))
			{
				Local<Object> errObj = Error::newObj();
				errObj->Set(String::New("code"),Number::New(-1));
				errObj->Set(String::New("errString"),String::New(GBK2UTF8("参数错误，请检查!").c_str()));
				return errObj;
			}
			if (fun(args))
			{
				Handle<Value> argv[2];
				v8::Persistent<v8::Function> callback;
				argv[0] = String::New(GBK2UTF8("参数错误，请检查!").c_str());
				node::MakeCallback(Context::GetCurrent()->Global(), callback, 2, argv);
			}
		}
		if(result.at(1) == "pv" && !fun(args))
			return batRealReadSyn(args);
		else if (result.at(1) == "pv" && fun(args))
		{
			return batRealReadAsy(args);
		}
	}else{
		String::Utf8Value str(args[0]);
		const char * pstr = ToCString(str);
		std::vector<std::string> result=split(pstr,".");
		int len = result.size();
		if (len<2 || len>3)
		{
			if (!fun(args))
			{
				Local<Object> errObj = Error::newObj();
				errObj->Set(String::New("code"),Number::New(-1));
				errObj->Set(String::New("errString"),String::New(GBK2UTF8("参数错误，请检查!").c_str()));
				return errObj;
			}
			if (fun(args))
			{
				Handle<Value> argv[2];
				Local<Function> fun = Local<Function>::Cast(args[args.Length()-1]);
				v8::Persistent<v8::Function> callback = Persistent<Function>::New(fun);
				argv[0] = String::New(GBK2UTF8("参数错误，请检查!").c_str());
				argv[1] = Undefined();
				node::MakeCallback(Context::GetCurrent()->Global(), callback, 2, argv);
			}
		}
		if(result.at(1) == "pv" && !fun(args)){
			return realReadSyn(args);
		}else if(result.at(1) == "pv" && fun(args)){
			return realReadAsy(args);
		}else if (result.at(1) == "props" && !fun(args))
		{
			return getTagPropsSyn(args);
		}else if (result.at(1) == "props" && fun(args))
		{
			return getTagpropsAsy(args);
		}else if (result.at(1) == "his" && result.at(2) == "atTime" && !fun(args))
		{
			return readHisAtTimeSyn(args);
		}else if (result.at(1) == "his" && result.at(2) == "atTime" && fun(args))
		{
			return readHisAtTimeAsy(args);
		}else if (result.at(1) == "his" && result.at(2) == "raw" && !fun(args))
		{
			return readHisRawSyn(args);
		}else if (result.at(1)== "his" && result.at(2) == "raw" && fun(args))
		{
			return readHisRawAsy(args);
		}else if (result.at(1) == "his" && result.at(2) == "processed" && !fun(args))
		{
			return readHisProcessSyn(args);
		}else if (result.at(1) == "his" && result.at(2) == "processed" && fun(args))
		{
			return readHisProcessAsy(args);
		}else if (result.at(1) == "alarm" && result.at(2) == "real" && !fun(args))
		{
			return alarmRealSyn(args);
		}else if (result.at(1) == "alarm" && result.at(2) == "real" && fun(args))
		{
			return alarmRealAsy(args);
		}else if (result.at(1) == "alarm" && result.at(2) == "his" && !fun(args))
		{
			return hisAlarmSyn(args);
		}else if (result.at(1) == "alarm" && result.at(2) == "his" && fun(args))
		{
			return hisAlarmAsy(args);
		}else if (result.at(1)== "alarm" && result.at(2) == "ack" && !fun(args))
		{
			return ackAlarmSyn(args);
		}else if (result.at(1) == "alarm" && result.at(2) == "ack" && fun(args))
		{
			return ackAlarmAsy(args);
		}else if (result.size()==2 && !fun(args))
		{
			return getSignalPropsSyn(args);
		}else if(result.size()==2 && fun(args)){
			return getSignalpropsAsy(args);
		}else{
			if (!fun(args))
			{
				Local<Object> errObj = Error::newObj();
				errObj->Set(String::New("code"),Number::New(-1));
				errObj->Set(String::New("errString"),String::New(GBK2UTF8("参数错误，请检查!").c_str()));
				return errObj;
			}
			if (fun(args))
			{
				Handle<Value> argv[2];
				Local<Function> fun = Local<Function>::Cast(args[args.Length()-1]);
				v8::Persistent<v8::Function> callback = Persistent<Function>::New(fun);
				argv[0] = String::New(GBK2UTF8("参数错误，请检查!").c_str());
				argv[1] = Undefined();
				node::MakeCallback(Context::GetCurrent()->Global(), callback, 2, argv);
			}
		}
	}
}



Handle<Value> PspaceNode::write(const Arguments& args)
{
	if(args[0]->IsString()){
	
		String::Utf8Value str(args[0]);
		const char * pstr = ToCString(str);
		std::vector<std::string> result=split(pstr,".");
		int len = result.size();
		if (len<2 || len>3)
		{
			if (!fun(args))
			{
				Local<Object> errObj = Error::newObj();
				errObj->Set(String::New("code"),Number::New(-1));
				errObj->Set(String::New("errString"),String::New(GBK2UTF8("参数错误，请检查!").c_str()));
				return errObj;
			}
			if (fun(args))
			{
				Handle<Value> argv[2];
				Local<Function> fun = Local<Function>::Cast(args[args.Length()-1]);
				v8::Persistent<v8::Function> callback = Persistent<Function>::New(fun);
				argv[0] = String::New(GBK2UTF8("参数错误，请检查!").c_str());
				argv[1] = Undefined();
				node::MakeCallback(Context::GetCurrent()->Global(), callback, 2, argv);
			}
		}
		if(result[1] == "pv" && !fun(args)){
			return realWriteSyn(args);
		}else if(result[1] == "pv" && fun(args)){
			return realWriteAsy(args);
		}else if (result[1] == "props" && !fun(args))
		{
			return setTagPropsSyn(args);
		}else if (result[1] == "props" && fun(args))
		{
			return setTagpropsAsy(args);
		}else if (result[1] == "his" && !fun(args))
		{
			return hisSyn(args);
		}else if (result[1] == "his" &&  fun(args))
		{
			return hisAsy(args);
		}
	}else if(args[0]->IsObject() && !fun(args)){
		return batRealWriteSyn(args);
	}else if (args[0]->IsObject() && fun(args))
	{
			
		return batRealWriteAsy(args);
	}else{
		if (!fun(args))
		{
			Local<Object> errObj = Error::newObj();
			errObj->Set(String::New("code"),Number::New(-1));
			errObj->Set(String::New("errString"),String::New(GBK2UTF8("参数错误，请检查!").c_str()));
			return errObj;
		}
		if (fun(args))
		{
			Handle<Value> argv[2];
			Local<Function> fun = Local<Function>::Cast(args[args.Length()-1]);
			v8::Persistent<v8::Function> callback = Persistent<Function>::New(fun);
			argv[0] = String::New(GBK2UTF8("参数错误，请检查!").c_str());
			argv[1] = Undefined();
			node::MakeCallback(Context::GetCurrent()->Global(), callback, 2, argv);
		}
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
			PSAPIStatus *pAPIErrors = PSNULL;
			PSUINT32 nSubscribeID = 0;
			//static PSUINT32 nNewSubscribe = 0;
			PSAPIStatus nRet = PSRET_OK;
			//PSUINT32 **tagID = sbton->getTagIDList(sbton->tagName_,sbton->psNode->hHanle_,sbton->tagCount_);
			PSUINT32 *tagIDs = *(sbton->getTagIDList(sbton->tagName_,sbton->psNode->hHanle_,sbton->tagCount_));
			
			if (tagIDs==NULL)
			{
				sbton->code_ = -1;
				sbton->errString = new std::string("有测点不存在或参数错误!");
			}else{
				nRet = psAPI_Real_NewSubscribeAndRead(sbton->psNode->hHanle_, sbton->tagCount_, 
					tagIDs,sbton->Real_CallbackFunction, (PSVOID*)0, &nSubscribeID, &(sbton->subData_), &pAPIErrors);
				if (PSERR(nRet) && nRet != PSERR_FAIL_IN_BATCH)
				{
					sbton->code_ = nRet;
					sbton->errString = new std::string(psAPI_Commom_GetErrorDesc(nRet));
					psAPI_Memory_FreeAndNull((PSVOID**)&tagIDs);
				} 
				if (nRet == PSERR_FAIL_IN_BATCH)
				{
					sbton->code_ = nRet;
					sbton->errString = new std::string(psAPI_Commom_GetErrorDesc(nRet));
					psAPI_Memory_FreeAndNull((PSVOID**)&pAPIErrors);
					psAPI_Memory_FreeAndNull((PSVOID**)&tagIDs);
				}
				sbton->subID = nSubscribeID;
				sbton->tagName = sbton->getTagName(sbton->id);
				psAPI_Memory_FreeAndNull((PSVOID**)&tagIDs);
				
			}
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
			Handle<Value> argv[4];
			argv[0] = Exception::Error(String::New(GBK2UTF8(sbaton->errString->c_str()).c_str()));
			argv[1] = Undefined();
			argv[2] = Undefined();
			argv[3] = Undefined();
			node::MakeCallback(Context::GetCurrent()->Global(),sbaton->callback, 4, argv);
		}else{
			Handle<Value> argv[4];
			Local<Array> arrObj1 = Array::New(sbaton->tagCount_);
			argv[0] = Undefined();
			argv[1] = Uint32::New(sbaton->subID);
			for (int i=0;i<sbaton->tagCount_;i++)
			{
				Local<Object> tmpObj = Object::New();
				std::string tmpStr= GBK2UTF8(sbaton->tagName_[i]); 
				tmpObj->Set(String::New("name"),String::New(replace_all(tmpStr,"\\","/").c_str()));
				tmpObj->Set(String::New("value"),getRealObj(sbaton->subData_+i));
				arrObj1->Set(i,tmpObj);
			}
			
			argv[2] = arrObj1;
			argv[3] = Undefined();
			node::MakeCallback(Context::GetCurrent()->Global(),sbaton->callback, 4, argv);
			
			int r;
			timerMap[sbaton->subID] = sbaton->timer;
			sbaton->timer->data = sbaton;
			r = uv_timer_init(uv_default_loop(), sbaton->timer);  
			assert(r == 0);  
			assert(!uv_is_active((uv_handle_t *) sbaton->timer));  
			assert(!uv_is_closing((uv_handle_t *) sbaton->timer));   
			//r = uv_timer_start(sbaton->timer, timer_cb, 0, 500); 
			r = uv_timer_start(sbaton->timer, sbaton->timer_cb, 0, 100);
			r = uv_run(uv_default_loop(), UV_RUN_DEFAULT);  
		}
	} catch(PsException &ex) {
		Handle<Value> argv[4];
		argv[0] = Exception::Error(String::New(ex.what()));
		argv[1] = Undefined();
		argv[2] = Undefined();
		argv[3] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), sbaton->callback, 4, argv);
	} catch(const exception &ex) {
		Handle<Value> argv[4];
		argv[0] = Exception::Error(String::New(sbaton->errString->c_str()));
		argv[1] = Undefined();
		argv[2] = Undefined();
		argv[3] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), sbaton->callback, 4, argv);
	}
	scope.Close(Undefined());
	FREE_MEMORY(sbaton);
    FREE_MEMORY(req);
	
}
/*
void PspaceNode::timer_cb(uv_timer_t *handle,int status)
{
	
	Handle<Value> argv[4];
	SubBaton* sbaton = static_cast<SubBaton*>(handle->data);
	Local<Array> arrObj1 = Array::New(sbaton->tagCount_);
	if(sbaton->callbackData_.size() >=1)
	{
		int len = sbaton->callbackData_.size();
		Local<Array> arrObj = Array::New(len);
		for (int i=0;i<len;i++)
		{
			Local<Object> tmpObj = Object::New();
			SubBaton::CallbackData callBackData = sbaton->callbackData_.front();
			sbaton->callbackData_.pop_front();
			tmpObj->Set(String::New("name"),String::New(replace_all(sbaton->getTagName(callBackData.tagID),"\\","/").c_str()));
			tmpObj->Set(String::New("value"),getRealObj(&callBackData.date));
			arrObj->Set(i,tmpObj);
		}
		argv[0] = Undefined();
		argv[1] = Uint32::New(sbaton->subID);
		argv[2] =  Undefined();
		argv[3] = arrObj;
		//异步回调执行cb
		node::MakeCallback(Context::GetCurrent()->Global(), sbaton->callback, 4, argv);
	}
	
}
*/
void PspaceNode::delSubAllWork(uv_work_t* req)
{
	DelSubBaton* delBaton = static_cast<DelSubBaton*>(req->data);
	delBaton->code_  = PSRET_OK;
	delBaton->errString = NULL;
	try {
		if(delBaton->psNode->hHanle_ == PSHANDLE_UNUSED) {
			throw PsException("Connection already closed");
		}else{ 
			PSAPIStatus nRet = PSRET_OK;
			if (delBaton->psNode->getTimer(delBaton->subID)!=NULL)
			{
				uv_timer_stop(*(delBaton->psNode->getTimer(delBaton->subID)));
				delBaton->psNode->delMapTimer(delBaton->subID);
			}
			nRet = psAPI_Real_DelSubscribeAll(delBaton->psNode->hHanle_,delBaton->subID);
			if (PSERR(nRet))
			{
				delBaton->code_ = nRet;
				delBaton->errString = new std::string(psAPI_Commom_GetErrorDesc(nRet));
			}

		}	
	}catch(PsException &ex) {
		delBaton->errString = new string(ex.what());
	} catch (const exception& ex) {
		delBaton->errString = new string(ex.what());
	} catch (...) {
		delBaton->errString = new string("Unknown Error");
	}
}

void PspaceNode::afterDelSub(uv_work_t* req, int status)
{

}

Handle<Value> PspaceNode::delSubAll(const Arguments& args)
{
	
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	DelSubBaton* baton = NULL;
    uv_work_t* req = NULL;
	try {
		Handle<Object> robj = Object::New();
		baton = new DelSubBaton(ps,NULL);
		baton->subID = args[0]->ToUint32()->Value(); 
		req = new uv_work_t();
		req->data = baton;
		
		baton->psNode->Ref();
		delSubAllWork(req);
		
		baton->psNode->Unref();
		//如果失败
		if(baton->errString) {
			Local<Object> errObj = Error::newObj();
			errObj->Set(String::New("code"),Number::New(baton->code_));
			errObj->Set(String::New("errString"),String::New(GBK2UTF8(baton->errString->c_str()).c_str()));
            FREE_MEMORY(baton);
            FREE_MEMORY(req);
			return errObj;
		}
		//成功
        FREE_MEMORY(baton);
        FREE_MEMORY(req);
		return scope.Close(Boolean::New(true));
	} catch(PsException &ex) {
		return ThrowException(Exception::Error(String::New(ex.what())));
	}	
}

Handle<Value> PspaceNode::delSub(const Arguments& args)
{
	
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	
	DelSubBaton* bat = NULL;
    uv_work_t* req = NULL;
	try {
		Handle<Object> robj = Object::New();
		bat = new DelSubBaton(ps,NULL);
		REQ_ARRAY_ARG(1,tagArr);
		bat->tagCount_ = tagArr->Length();
		bat->tagName_ = new PSSTR[bat->tagCount_];
		bat->subID = args[0]->ToUint32()->Value();
		
		for (int i=0;i<bat->tagCount_;i++)
		{
			String::Utf8Value iStr(tagArr->Get(i));
			const char* IDStr = ToCString(iStr);
			//std::vector<std::string> result=split(IDStr,".");
			std::string restlt = IDStr;
			const char* str = replace_all(restlt,"/","\\").c_str();
			bat->tagName_[i] = new char[strlen(str)+1];
			strcpy(bat->tagName_[i],str);
			bat->tagName_[i][strlen(bat->tagName_[i])] = 0;
			
		}
		req = new uv_work_t();
		req->data = bat;
		bat->psNode->Ref();
		delSubWork(req);
		bat->psNode->Unref();
		//如果失败
		if(bat->errString) {
			Local<Object> errObj = Error::newObj();
			errObj->Set(String::New("code"),Number::New(bat->code_));
			errObj->Set(String::New("errString"),String::New(GBK2UTF8(bat->errString->c_str()).c_str()));
			FREE_MEMORY(bat);
            FREE_MEMORY(req);
			return errObj;
		}
		//  成功
		FREE_MEMORY(bat);
        FREE_MEMORY(req);
		return scope.Close(Boolean::New(true));
	} catch(PsException &ex) {
		return ThrowException(Exception::Error(String::New(ex.what())));
	}	
	
}

void PspaceNode::delSubWork(uv_work_t* req)
{
	DelSubBaton* delBaton = static_cast<DelSubBaton*>(req->data);
	delBaton->code_  = PSRET_OK;
	delBaton->errString = NULL;
	try {
		if(delBaton->psNode->hHanle_ == PSHANDLE_UNUSED) {
			throw PsException("Connection already closed");
		}else{ 
			PSAPIStatus *pAPIErrors = PSNULL;
			PSAPIStatus nRet = PSRET_OK;
			if (delBaton->psNode->getTimer(delBaton->subID)!=NULL)
			{
				uv_timer_stop(*(delBaton->psNode->getTimer(delBaton->subID)));
				delBaton->psNode->delMapTimer(delBaton->subID);
			}
			PSUINT32 *tagIDs = *(delBaton->getTagIDList(delBaton->tagName_,delBaton->psNode->hHanle_,delBaton->tagCount_));
			nRet = psAPI_Real_DelSubscribe(delBaton->psNode->hHanle_, delBaton->subID, 
				delBaton->tagCount_,tagIDs, &pAPIErrors);
			if (PSERR(nRet))
			{
				delBaton->code_ = nRet;
				delBaton->errString = new std::string(psAPI_Commom_GetErrorDesc(nRet));
				psAPI_Memory_FreeAndNull((PSVOID**)&tagIDs);
			}
			if (nRet == PSERR_FAIL_IN_BATCH)
			{
				for (int n = 0; n < delBaton->tagCount_; n++)
				{
					delBaton->code_ = n;
					delBaton->errString = new std::string(psAPI_Commom_GetErrorDesc(n));
					
				}
				psAPI_Memory_FreeAndNull((PSVOID**)&pAPIErrors);
				psAPI_Memory_FreeAndNull((PSVOID**)&tagIDs);
			}
			psAPI_Memory_FreeAndNull((PSVOID**)&tagIDs);
		}	
	}catch(PsException &ex) {
		delBaton->errString = new string(ex.what());
	} catch (const exception& ex) {
		delBaton->errString = new string(ex.what());
	} catch (...) {
		delBaton->errString = new string("Unknown Error");
	}
}

Handle<Value> PspaceNode::subValueAsy(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	REQ_ARRAY_ARG(0,tagArr);
	REQ_FUN_ARG(1, callback);
	SubBaton* sbaton = NULL;
	try {
		sbaton = new SubBaton(ps, &callback);
		sbaton->tagCount_ = tagArr->Length();
		sbaton->tagName_ = new PSSTR[sbaton->tagCount_];
		for (int i=0;i<sbaton->tagCount_;i++)
		{
			String::Utf8Value iStr(tagArr->Get(i));
			const char* IDStr = ToCString(iStr);
			//std::vector<std::string> result=split(IDStr,".");
			std::string restlt = UTF8ToGBK(IDStr);
			const char* str = replace_all(restlt,"/","\\").c_str();
			sbaton->tagName_[i] = new char[strlen(str)+1];
			strcpy(sbaton->tagName_[i],str);
			sbaton->tagName_[i][strlen(sbaton->tagName_[i])] = 0;
		}
	} catch(PsException &ex) {
		return scope.Close(ThrowException(Exception::Error(String::New(ex.what()))));
	}
	uv_work_t* req = NULL;
    req = new uv_work_t();
	req->data = sbaton;
	uv_queue_work(uv_default_loop(), req, subValueWork, (uv_after_work_cb)aftersubValue);
	ps->Ref();
	return scope.Close(Undefined());
}

Handle<Value> PspaceNode::sub(const Arguments& args)
{
	if(args[0]->IsArray() && fun(args)){
		REQ_ARRAY_ARG(0,tagArr);
		String::Utf8Value iStr(tagArr->Get(0));
		const char* IDStr = ToCString(iStr);
		std::vector<std::string> result=split(IDStr,".");
		if (result.size()<2)
		{
			return subValueAsy(args);
		}else if(result.size()==2){
			return subPropsAsy(args);
		}	
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
			PSUINT32 **tagID = sbton->getTagIDList(sbton->tagName_,sbton->psNode->hHanle_,sbton->tagCount_);
			if (tagID==NULL)
			{
				sbton->code_ = -1;
				sbton->errString = new std::string("测点不存在或参数错误!");
			}else{

					nRet = psAPI_Tag_NewSubscribe(sbton->psNode->hHanle_,sbton->tagCount_, *tagID,
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
						psAPI_Memory_FreeAndNull((PSVOID**)&pAPIErrors);
					}
					sbton->subID = nSubscribeID;
                    psAPI_Memory_FreeAndNull((PSVOID**)tagID);
			}
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
			argv[0] = Exception::Error(String::New(GBK2UTF8(sbaton->errString->c_str()).c_str()));
			argv[1] = Undefined();
			argv[2] = Undefined();
		}else{
			int r;
			timerMap[sbaton->subID] = sbaton->timer;
			sbaton->timer->data = sbaton;
			r = uv_timer_init(uv_default_loop(), sbaton->timer);  
			assert(r == 0);  
			assert(!uv_is_active((uv_handle_t *) sbaton->timer));  
			assert(!uv_is_closing((uv_handle_t *) sbaton->timer));   
			//r = uv_timer_start(sbaton->timer, timer_cb, 0, 500); 
			r = uv_timer_start(sbaton->timer, sbaton->time_propCb, 0, 100);
			
			r = uv_run(uv_default_loop(), UV_RUN_DEFAULT); 
		}
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
    FREE_MEMORY(sbaton);
    FREE_MEMORY(req);
}

Handle<Value> PspaceNode::subPropsAsy(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	//String::Utf8Value str(args[0]);
	REQ_ARRAY_ARG(0,tagArr);
	REQ_FUN_ARG(1, callback);
	
	SubBaton* sbaton = NULL;
	try {
		sbaton = new SubBaton(ps, &callback);
		sbaton->tagCount_ = tagArr->Length();
		//bat->tagID_ = new PSUINT32[tagArr->Length()];
		sbaton->tagName_ = new PSSTR[sbaton->tagCount_];
		for (int i=0;i<sbaton->tagCount_;i++)
		{
			String::Utf8Value iStr(tagArr->Get(i));
			const char* IDStr = ToCString(iStr);
			std::vector<std::string> result=split(UTF8ToGBK(IDStr),".");
			
			const char* str = replace_all(result[0],"/","\\").c_str();
			sbaton->tagName_[i] = new char[strlen(str)+1];
			strcpy(sbaton->tagName_[i],str);
			sbaton->tagName_[i][strlen(sbaton->tagName_[i])] = 0;
		}
	} catch(PsException &ex) {
		return scope.Close(ThrowException(Exception::Error(String::New(ex.what()))));
	}
	uv_work_t* req = NULL;
    req = new uv_work_t();
	req->data = sbaton;
	uv_queue_work(uv_default_loop(), req, subPropsWork, (uv_after_work_cb)aftersubProps);
	ps->Ref();
	return scope.Close(Number::New(sbaton->subID));
}

Handle<Value> PspaceNode::tagAddSyn(const Arguments& args)
{
	//std::cout<<"asfdasfas"<<std::endl;
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	Tag* t = NULL;
	try { 
		t = new Tag(ps,NULL);
		String::Utf8Value str(args[0]);
		const char * pstr = ToCString(str);
		std::vector<std::string> result=split(pstr,"/");
		int len = result.size();
		//要添加的测点名称
        if (len>=1)
        {
            t->tagName = result[len-1];
        }
		//获取父节点
		t->parentTagId_ = t->getParentID(result,t->psNode->hHanle_);
		REQ_OBJECT_ARG(1,propObj);
		//获取对象所有的key
		Handle<Array> propKeys = propObj->GetOwnPropertyNames();
		//key中含有name,属性数量减少一个
		for (int i=0;i<propKeys->Length();i++)
		{
			String::Utf8Value str(propKeys->Get(i));
			const char * pstr = ToCString(str);
			if (strstr(str2Upper(pstr),"NAME"))
			{
				t->propCount_ = propKeys->Length();
				propObj->Delete(String::New(pstr));
				propKeys = propObj->GetOwnPropertyNames();
				break;
			}else{
				t->propCount_ = propKeys->Length()+1;
				break;
			}
		}
		//创建数组
		t->pPropIds_ = new PSUINT16[t->propCount_];
		t->pPropValues_ = new PS_VARIANT[t->propCount_];
		//默认忽略NAME key
		t->pPropIds_[t->propCount_-1] = PS_TAG_PROP_NAME;
		t->pPropValues_[t->propCount_-1].DataType = PSDATATYPE_STRING;
		t->pPropValues_[t->propCount_-1].String.Data = new char[strlen((t->tagName).c_str())+1];
		strcpy(t->pPropValues_[t->propCount_-1].String.Data,(PSSTR)UTF8ToGBK(t->tagName).c_str());
		t->pPropValues_[t->propCount_-1].String.Data[strlen((t->tagName).c_str())] = 0;
		t->pPropValues_[t->propCount_-1].String.Length = strlen(t->pPropValues_[t->propCount_-1].String.Data);
		
		for (int i=0;i<t->propCount_-1;i++)
		{
			String::Utf8Value str(propKeys->Get(i));
			const char * pstr = ToCString(str);
			if (strstr(str2Upper(pstr),"NAME"))
			{
				propObj->Delete(String::New(pstr));
				Local<Object> errObj = Error::newObj();
				errObj->Set(String::New("code"),Number::New(-1));
				std::string *s =t->errString;
				errObj->Set(String::New("errString"),String::New(GBK2UTF8("测点长名不需要提供了!").c_str()));
				delete t;
				return errObj;
			}
			if (strstr(str2Upper(pstr),"TAGTYPE"))
			{
				t->pPropIds_[i] = PS_TAG_PROP_TAGTYPE;
				t->pPropValues_[i].DataType=PSDATATYPE_UINT16;
				Local<Value> propVal = propObj->Get(String::New(pstr));
				String::Utf8Value propStr(propVal);
				const char * strVal = ToCString(propStr);
				t->pPropValues_[i].UInt16 = t->getTagType(strVal);
				//std::cout<<"tagType:"<< t->getTagType(strVal)<<std::endl;
			}else
			{
				t->pPropIds_[i] = t->getPropID(pstr,t->psNode->hHanle_);
				t->pPropValues_[i].DataType = t->getPropInfo(pstr,t->psNode->hHanle_).DataType;
				switch(t->pPropValues_[i].DataType)
				{
				case PSDATATYPE_UINT16:
					{
						if (t->pPropIds_[i] == PS_TAG_PROP_DATATYPE)
						{
							String::Utf8Value str1(propObj->Get(String::New(pstr)));
							const char * pstr1 = ToCString(str1);
							t->pPropValues_[i].UInt16 = t->getDataType(pstr1);
						}else
						{
							t->pPropValues_[i].UInt16 = (PSUINT16)propObj->Get(String::New(pstr))->Uint32Value();
						}	
					}
					break;
				case  PSDATATYPE_BOOL:
					{
						if (t->pPropIds_[i] == PS_TAG_PROP_DATATYPE)
						{
							String::Utf8Value str1(propObj->Get(String::New(pstr)));
							const char * pstr1 = ToCString(str1);
							t->pPropValues_[i].Bool = t->getDataType(pstr1);
						}else
						{
							Local<Value> bo = propObj->Get(String::New(pstr));
							PS_DATATYPE_ENUM b = (PS_DATATYPE_ENUM)bo->BooleanValue();
							t->pPropValues_[i].Bool = b;
						}
					}
					break;
				case PSDATATYPE_DOUBLE:
					{
						if (t->pPropIds_[i] == PS_TAG_PROP_DATATYPE)
						{
							String::Utf8Value str1(propObj->Get(String::New(pstr)));
							const char * pstr1 = ToCString(str1);
							t->pPropValues_[i].Double = t->getDataType(pstr1);
						}else
						{
							Local<Value> db = propObj->Get(String::New(pstr));
							double d = db->NumberValue();
							t->pPropValues_[i].Double = d;
						}
					}
					break;
				case PSDATATYPE_INT16:
					{
						if (t->pPropIds_[i] == PS_TAG_PROP_DATATYPE)
						{
							String::Utf8Value str1(propObj->Get(String::New(pstr)));
							const char * pstr1 = ToCString(str1);
							t->pPropValues_[i].Int16 = t->getDataType(pstr1);
						}else
						{
							Local<Value> int16 = propObj->Get(String::New(pstr));
							PS_DATATYPE_ENUM i16 = (PS_DATATYPE_ENUM)int16->Int32Value();
							t->pPropValues_[i].Int16 = i16;
						}
					}
					break;
				case PSDATATYPE_INT32:
					{
						if (t->pPropIds_[i] == PS_TAG_PROP_DATATYPE)
						{
							String::Utf8Value str1(propObj->Get(String::New(pstr)));
							const char * pstr1 = ToCString(str1);
							t->pPropValues_[i].Int32 = t->getDataType(pstr1);
						}else
						{
							Local<Value> int32 = propObj->Get(String::New(pstr));
							PSINT32 i32 = (PSINT32)(int32->Int32Value());
							t->pPropValues_[i].Int32 = i32;
						}
					}
					break;
				case PSDATATYPE_UINT32:
					{
						if (t->pPropIds_[i] == PS_TAG_PROP_DATATYPE)
						{
							String::Utf8Value str1(propObj->Get(String::New(pstr)));
							const char * pstr1 = ToCString(str1);
							t->pPropValues_[i].UInt32 = t->getDataType(pstr1);
						}else
						{
							Local<Value> int32 = propObj->Get(String::New(pstr));
							PSUINT32 i32 = (PSUINT32)int32->Uint32Value();
							t->pPropValues_[i].UInt32 = i32;
						}
					}
					break;
				case PSDATATYPE_INT64:
					{
						if (t->pPropIds_[i] == PS_TAG_PROP_DATATYPE)
						{
							String::Utf8Value str1(propObj->Get(String::New(pstr)));
							const char * pstr1 = ToCString(str1);
							t->pPropValues_[i].Int64 = t->getDataType(pstr1);
						}else
						{
							Local<Value> int64 = propObj->Get(String::New(pstr));
							PS_DATATYPE_ENUM i64 = (PS_DATATYPE_ENUM)int64->Int32Value();
							t->pPropValues_[i].Int64 = i64;
						}
					}
					break;
				case PSDATATYPE_UINT64:
					{
						if (t->pPropIds_[i] == PS_TAG_PROP_DATATYPE)
						{
							String::Utf8Value str1(propObj->Get(String::New(pstr)));
							const char * pstr1 = ToCString(str1);
							t->pPropValues_[i].UInt64 = t->getDataType(pstr1);
						}else
						{
							Local<Value> uint64 = propObj->Get(String::New(pstr));
							PS_DATATYPE_ENUM u64 = (PS_DATATYPE_ENUM)uint64->Uint32Value();
							t->pPropValues_[i].UInt64 = u64;
						}

					}
					break;
				case PSDATATYPE_INT8:
					{
						if (t->pPropIds_[i] == PS_TAG_PROP_DATATYPE)
						{
							String::Utf8Value str1(propObj->Get(String::New(pstr)));
							const char * pstr1 = ToCString(str1);
							t->pPropValues_[i].Int8 = t->getDataType(pstr1);
						}else
						{
							Local<Value> int8 = propObj->Get(String::New(pstr));
							PS_DATATYPE_ENUM i8 = (PS_DATATYPE_ENUM)int8->Int32Value();
							t->pPropValues_[i].Int8 = i8;
						}

					}
					break;
				case PSDATATYPE_UINT8:
					{
						if (t->pPropIds_[i] == PS_TAG_PROP_DATATYPE)
						{
							String::Utf8Value str1(propObj->Get(String::New(pstr)));
							const char * pstr1 = ToCString(str1);
							t->pPropValues_[i].UInt8 = t->getDataType(pstr1);
						}else
						{
							Local<Value> uint8 = propObj->Get(String::New(pstr));
							PS_DATATYPE_ENUM u8 = (PS_DATATYPE_ENUM)uint8->Uint32Value();
							t->pPropValues_[i].UInt8 = u8;
						}	
					}
					break;
				case PSDATATYPE_FLOAT:
					{
						if (t->pPropIds_[i] == PS_TAG_PROP_DATATYPE)
						{
							String::Utf8Value str1(propObj->Get(String::New(pstr)));
							const char * pstr1 = ToCString(str1);
							t->pPropValues_[i].Float = t->getDataType(pstr1);
						}else
						{
							Local<Value> fl = propObj->Get(String::New(pstr));
							float f = (float)fl->NumberValue();
							t->pPropValues_[i].Float = f;
						}	
					}
					break;
				case  PSDATATYPE_WSTRING:
					{
						String::Utf8Value str1(propObj->Get(String::New(pstr)));
						const char * pstr1 = ToCString(str1);
						t->pPropValues_[i].String.Data = new char[strlen(pstr1)+1];
						strcpy(t->pPropValues_[i].String.Data,(PSSTR)UTF8ToGBK(pstr1).c_str());
						t->pPropValues_[i].String.Data[strlen(pstr1)] = 0;
						t->pPropValues_[i].String.Length = strlen(t->pPropValues_[i].String.Data);
					}
					break;
				case  PSDATATYPE_STRING:
					{
						String::Utf8Value str1(propObj->Get(String::New(pstr)));
						const char * pstr1 = ToCString(str1);
						t->pPropValues_[i].String.Data = new char[strlen(pstr1)+1];
						strcpy(t->pPropValues_[i].String.Data,(PSSTR)UTF8ToGBK(pstr1).c_str());
						t->pPropValues_[i].String.Data[strlen(pstr1)] = 0;
						t->pPropValues_[i].String.Length = strlen(t->pPropValues_[i].String.Data);
					}
					break;
				case PSDATATYPE_TIME:
					{
						/*String::Utf8Value str1(propObj->Get(String::New(pstr)));
						const char * pstr1 = ToCString(str1);
						PS_TIME *d = new PS_TIME();
						std::string s = pstr1;
						STR2PSTIME(d,s);*/
						V8DATE2PSTIME(&t->pPropValues_[i].Time,propObj->Get(String::New(pstr)));
						//t->pPropValues_[i].Time =*d;
					}
					break;
				default:
					{
						Local<Object> errObj = Error::newObj();
						errObj->Set(String::New("code"),Number::New(-1));
						std::string *s =t->errString;
						errObj->Set(String::New("errString"),String::New(GBK2UTF8("数据类型为空或有误!").c_str()));
						delete t;
						return errObj;
					}
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
			Local<Object> errObj = Error::newObj();
			errObj->Set(String::New("code"),Number::New(t->code_));
			string *s =t->errString;
			errObj->Set(String::New("errString"),String::New(GBK2UTF8(s->c_str()).c_str()));
			delete t;
			return errObj;
		}

		saveHis(t->psNode->hHanle_,t->id);
		
		delete t;
		return String::New(pstr);
	} catch(PsException &ex) {
		return ThrowException(Exception::Error(String::New(ex.what())));
	}		
}

Handle<Value> PspaceNode::tagAddAsy(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	REQ_FUN_ARG(findFunction(args), callback);
	Tag* t;
	try {
		t = new Tag(ps, &callback);
		String::Utf8Value str(args[0]);
		const char * pstr = ToCString(str);
		t->rName_ = new char[strlen(pstr)+1];
		strcpy(t->rName_,pstr);
		t->rName_[strlen(t->rName_)] = 0;
		std::vector<std::string> result=split(pstr,"/");
		int len = result.size();
		//要添加的测点名称
		t->tagName = result[len-1];
		///获取父节点
		t->parentTagId_ = t->getParentID(result,t->psNode->hHanle_);
		REQ_OBJECT_ARG(1,propObj);
		//获取对象所有的key
		Handle<Array> propKeys = propObj->GetOwnPropertyNames();
		//key中含有name,属性数量减少一个
		for (int i=0;i<propKeys->Length();i++)
		{
			String::Utf8Value str(propKeys->Get(i));
			const char * pstr = ToCString(str);
			if (strstr(str2Upper(pstr),"NAME"))
			{
				t->propCount_ = propKeys->Length();
				propObj->Delete(String::New(pstr));
				propKeys = propObj->GetOwnPropertyNames();
				break;
			}else{
				t->propCount_ = propKeys->Length()+1;

				break;
			}
		}
		//创建数组
		t->pPropIds_ = new PSUINT16[t->propCount_];
		t->pPropValues_ = new PS_VARIANT[t->propCount_];
		//默认忽略NAME key
		t->pPropIds_[t->propCount_-1] = PS_TAG_PROP_NAME;
		t->pPropValues_[t->propCount_-1].DataType = PSDATATYPE_STRING;
		t->pPropValues_[t->propCount_-1].String.Data = new char[strlen((t->tagName).c_str())+1];
		strcpy(t->pPropValues_[t->propCount_-1].String.Data,(PSSTR)UTF8ToGBK(t->tagName).c_str());
		t->pPropValues_[t->propCount_-1].String.Data[strlen((t->tagName).c_str())] = 0;
		t->pPropValues_[t->propCount_-1].String.Length = strlen(t->pPropValues_[t->propCount_-1].String.Data);

		for (int i=0;i<t->propCount_-1;i++)
		{
			String::Utf8Value str(propKeys->Get(i));
			const char * pstr = ToCString(str);
			if (strstr(str2Upper(pstr),"NAME"))
			{
				t->code_ = -1;
				t->errString = new std::string("测点长名重复，不需要再次提供!");
			}
			if (strstr(str2Upper(pstr),"TAGTYPE"))
			{
				t->pPropIds_[i] = PS_TAG_PROP_TAGTYPE;
				t->pPropValues_[i].DataType=PSDATATYPE_UINT16;
				Local<Value> propVal = propObj->Get(String::New(pstr));
				String::Utf8Value propStr(propVal);
				const char * strVal = ToCString(propStr);
				t->pPropValues_[i].UInt16 = t->getTagType(strVal);
				//std::cout<<"tagType:"<< t->getTagType(strVal)<<std::endl;

			}else
			{
				t->pPropIds_[i] = t->getPropID(pstr,t->psNode->hHanle_);
				t->pPropValues_[i].DataType = t->getPropInfo(pstr,t->psNode->hHanle_).DataType;
				
				switch(t->pPropValues_[i].DataType)
				{
				case PSDATATYPE_UINT16:
					{
						if (t->pPropIds_[i] == PS_TAG_PROP_DATATYPE)
						{
							String::Utf8Value str1(propObj->Get(String::New(pstr)));
							const char * pstr1 = ToCString(str1);
							t->pPropValues_[i].UInt16 = t->getDataType(pstr1);
						}else
						{
							t->pPropValues_[i].UInt16 = (PSUINT16)propObj->Get(String::New(pstr))->Uint32Value();
						}	
					}
					break;
				case  PSDATATYPE_BOOL:
					{
						if (t->pPropIds_[i] == PS_TAG_PROP_DATATYPE)
						{
							String::Utf8Value str1(propObj->Get(String::New(pstr)));
							const char * pstr1 = ToCString(str1);
							t->pPropValues_[i].Bool = t->getDataType(pstr1);
						}else
						{
							Local<Value> bo = propObj->Get(String::New(pstr));
							PS_DATATYPE_ENUM b = (PS_DATATYPE_ENUM)bo->BooleanValue();
							t->pPropValues_[i].Bool = b;
						}

					}
					break;
				case PSDATATYPE_DOUBLE:
					{
						if (t->pPropIds_[i] == PS_TAG_PROP_DATATYPE)
						{
							String::Utf8Value str1(propObj->Get(String::New(pstr)));
							const char * pstr1 = ToCString(str1);
							t->pPropValues_[i].Double = t->getDataType(pstr1);
						}else
						{
							Local<Value> db = propObj->Get(String::New(pstr));
							double d = db->NumberValue();
							t->pPropValues_[i].Double = d;
						}

					}
					break;
				case PSDATATYPE_INT16:
					{
						if (t->pPropIds_[i] == PS_TAG_PROP_DATATYPE)
						{
							String::Utf8Value str1(propObj->Get(String::New(pstr)));
							const char * pstr1 = ToCString(str1);
							t->pPropValues_[i].Int16 = t->getDataType(pstr1);
						}else
						{
							Local<Value> int16 = propObj->Get(String::New(pstr));
							PS_DATATYPE_ENUM i16 = (PS_DATATYPE_ENUM)int16->Int32Value();
							t->pPropValues_[i].Int16 = i16;
						}

					}
					break;
				case PSDATATYPE_INT32:
					{
						if (t->pPropIds_[i] == PS_TAG_PROP_DATATYPE)
						{
							String::Utf8Value str1(propObj->Get(String::New(pstr)));
							const char * pstr1 = ToCString(str1);
							t->pPropValues_[i].Int32 = t->getDataType(pstr1);
						}else
						{
							Local<Value> int32 = propObj->Get(String::New(pstr));
							PSINT32 i32 = (PSINT32)(int32->Int32Value());
							t->pPropValues_[i].Int32 = i32;
						}
					}
					break;
				case PSDATATYPE_UINT32:
					{
						if (t->pPropIds_[i] == PS_TAG_PROP_DATATYPE)
						{
							String::Utf8Value str1(propObj->Get(String::New(pstr)));
							const char * pstr1 = ToCString(str1);
							t->pPropValues_[i].UInt32 = t->getDataType(pstr1);
						}else
						{
							Local<Value> int32 = propObj->Get(String::New(pstr));
							PSUINT32 i32 = (PSUINT32)int32->Uint32Value();
							t->pPropValues_[i].UInt32 = i32;
						}

					}
					break;
				case PSDATATYPE_INT64:
					{
						if (t->pPropIds_[i] == PS_TAG_PROP_DATATYPE)
						{
							String::Utf8Value str1(propObj->Get(String::New(pstr)));
							const char * pstr1 = ToCString(str1);
							t->pPropValues_[i].Int64 = t->getDataType(pstr1);
						}else
						{
							Local<Value> int64 = propObj->Get(String::New(pstr));
							PS_DATATYPE_ENUM i64 = (PS_DATATYPE_ENUM)int64->Int32Value();
							t->pPropValues_[i].Int64 = i64;
						}

					}
					break;
				case PSDATATYPE_UINT64:
					{
						if (t->pPropIds_[i] == PS_TAG_PROP_DATATYPE)
						{
							String::Utf8Value str1(propObj->Get(String::New(pstr)));
							const char * pstr1 = ToCString(str1);
							t->pPropValues_[i].UInt64 = t->getDataType(pstr1);
						}else
						{
							Local<Value> uint64 = propObj->Get(String::New(pstr));
							PS_DATATYPE_ENUM u64 = (PS_DATATYPE_ENUM)uint64->Uint32Value();
							t->pPropValues_[i].UInt64 = u64;
						}

					}
					break;
				case PSDATATYPE_INT8:
					{
						if (t->pPropIds_[i] == PS_TAG_PROP_DATATYPE)
						{
							String::Utf8Value str1(propObj->Get(String::New(pstr)));
							const char * pstr1 = ToCString(str1);
							t->pPropValues_[i].Int8 = t->getDataType(pstr1);
						}else
						{
							Local<Value> int8 = propObj->Get(String::New(pstr));
							PS_DATATYPE_ENUM i8 = (PS_DATATYPE_ENUM)int8->Int32Value();
							t->pPropValues_[i].Int8 = i8;
						}

					}
					break;
				case PSDATATYPE_UINT8:
					{
						if (t->pPropIds_[i] == PS_TAG_PROP_DATATYPE)
						{
							String::Utf8Value str1(propObj->Get(String::New(pstr)));
							const char * pstr1 = ToCString(str1);
							t->pPropValues_[i].UInt8 = t->getDataType(pstr1);
						}else
						{
							Local<Value> uint8 = propObj->Get(String::New(pstr));
							PS_DATATYPE_ENUM u8 = (PS_DATATYPE_ENUM)uint8->Uint32Value();
							t->pPropValues_[i].UInt8 = u8;
						}	
					}
					break;
				case PSDATATYPE_FLOAT:
					{
						if (t->pPropIds_[i] == PS_TAG_PROP_DATATYPE)
						{
							String::Utf8Value str1(propObj->Get(String::New(pstr)));
							const char * pstr1 = ToCString(str1);
							t->pPropValues_[i].Float = t->getDataType(pstr1);
						}else
						{
							Local<Value> fl = propObj->Get(String::New(pstr));
							float f = (float)fl->NumberValue();
							t->pPropValues_[i].Float = f;
						}	
					}
					break;
				case  PSDATATYPE_WSTRING:
					{
						String::Utf8Value str1(propObj->Get(String::New(pstr)));
						const char * pstr1 = ToCString(str1);
						t->pPropValues_[i].String.Data = new char[strlen(pstr1)+1];
						strcpy(t->pPropValues_[i].String.Data,(PSSTR)UTF8ToGBK(pstr1).c_str());
						t->pPropValues_[i].String.Data[strlen(pstr1)] = 0;
						t->pPropValues_[i].String.Length = strlen(t->pPropValues_[i].String.Data);
					}
					break;
				case  PSDATATYPE_STRING:
					{
						String::Utf8Value str1(propObj->Get(String::New(pstr)));
						const char * pstr1 = ToCString(str1);
						t->pPropValues_[i].String.Data = new char[strlen(pstr1)+1];
						strcpy(t->pPropValues_[i].String.Data,(PSSTR)UTF8ToGBK(pstr1).c_str());
						t->pPropValues_[i].String.Data[strlen(pstr1)] = 0;
						t->pPropValues_[i].String.Length = strlen(t->pPropValues_[i].String.Data);
					}
					break;
				case PSDATATYPE_TIME:
					{
						/*String::Utf8Value str1(propObj->Get(String::New(pstr)));
						const char * pstr1 = ToCString(str1);
						PS_TIME *d = new PS_TIME();
						std::string s = pstr1;
						STR2PSTIME(d,s);
						t->pPropValues_[i].Time =*d;*/
						V8DATE2PSTIME(&t->pPropValues_[i].Time,propObj->Get(String::New(pstr)));
					}
					break;
				default:
					{
						t->code_ =-1;
						t->errString = new std::string("数据类型或参数错误！");
					}
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
			argv[0] = Exception::Error(String::New(GBK2UTF8(t->errString->c_str()).c_str()));
			argv[1] = Undefined();
			argv[2] = Undefined();
			node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 3, argv);
		}else{
			Handle<Value> argv[3];
			saveHis(t->psNode->hHanle_,t->id);
			//结果设置
			argv[0] = Undefined();
			argv[1] = String::New(t->rName_);
			argv[2] = Number::New(t->id);
			delete t->rName_;
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

void PspaceNode::saveHis(PSHANDLE h,PSUINT32 id)
{
	PSUINT16 pPropIds[1] = {PS_TAG_PROP_HIS_ISSAVE};
	PS_VARIANT pPropValues[1] = {0};
	pPropValues[0].DataType = PSDATATYPE_BOOL;
	pPropValues[0].Bool = true;
	PSAPIStatus nRet = PSRET_OK;
	nRet = psAPI_Tag_SetTagProps(h, id, 1, pPropIds, pPropValues);
	if (PSERR(nRet))
	{
		//std::cout<< psAPI_Commom_GetErrorDesc(nRet)<<std::endl;
	}
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
			Local<Object> errObj = Error::newObj();
			errObj->Set(String::New("code"),Number::New(t->code_));
			std:string *s =t->errString;
			//std::cout<<s->c_str()<<std::endl;
			errObj->Set(String::New("errString"),String::New(GBK2UTF8(s->c_str()).c_str()));
			delete t;
			return errObj;
		}
		//成功返回测点长名
		//std::cout<<t->tagName<<std::endl;
		return String::New(strName.c_str());
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
		t->tagName = strName;
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
			argv[0] = Exception::Error(String::New(GBK2UTF8(t->errString->c_str()).c_str()));
			argv[1] = Undefined();
		}else{
			Handle<Value> argv[2];
			//结果设置
			argv[0] = Undefined();
			argv[1] = String::New(t->tagName.c_str());
			node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 2, argv);
		}
	} catch(PsException &ex) {
		Handle<Value> argv[2];
		argv[0] = Exception::Error(String::New(ex.what()));
		argv[1] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 2, argv);
	} catch(const exception &ex) {
		Handle<Value> argv[2];
		argv[0] = Exception::Error(String::New(GBK2UTF8(t->errString->c_str()).c_str()));
		argv[1] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 3, argv);
	}
	delete t;
}

Handle<Value> PspaceNode::del(const Arguments& args)
{
	if (args[0]->IsUint32()&& args.Length()==1)
	{
		return delSubAll(args);
	}else if (args[1]->IsArray() && args[0]->IsUint32())
	{
		return delSub(args);
	}else if(!fun(args) && args[0]->IsString()){
		return tagDelSyn(args);
	}else if (fun(args) && args[0]->IsString())
	{
		return tagDelAsy(args);
	}else{
		if (!fun(args))
		{
			Local<Object> errObj = Error::newObj();
			errObj->Set(String::New("code"),Number::New(-1));
			errObj->Set(String::New("errString"),String::New(GBK2UTF8("参数错误，请检查!").c_str()));
			return errObj;
		}
		if (fun(args))
		{
			Handle<Value> argv[2];
			Local<Function> fun = Local<Function>::Cast(args[args.Length()-1]);
			v8::Persistent<v8::Function> callback = Persistent<Function>::New(fun);
			argv[0] = String::New(GBK2UTF8("参数错误，请检查!").c_str());
			argv[1] = Undefined();
			node::MakeCallback(Context::GetCurrent()->Global(), callback, 2, argv);
		}
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
		t->id = t->getTagID(UTF8ToGBK(replace_all(result[0],"/","\\").c_str()).c_str(),ps->hHanle_);
		t->tagName = result[0];
		REQ_OBJECT_ARG(1,propObj);
		Handle<Array> propKeys = propObj->GetOwnPropertyNames();
		t->propCount_ = propKeys->Length();
		t->pPropIds_ = new PSUINT16[t->propCount_];
		t->pPropValues_ = new PS_VARIANT[t->propCount_];
		
		for (int i=0;i<t->propCount_;i++)
		{
			String::Utf8Value str1(propKeys->Get(i));
			const char * propStr = ToCString(str1);
			t->pPropIds_[i] = t->getPropID(propStr,t->psNode->hHanle_);
			t->pPropValues_[i].DataType = t->getPropInfo(propStr,t->psNode->hHanle_).DataType;
			//std::cout<<t->getPropInfo(propStr,t->psNode->hHanle_).Desc<<std::endl;
			switch(t->pPropValues_[i].DataType)
			{
			case PSDATATYPE_UINT16:
				{
					
					t->pPropValues_[i].UInt16 = (PSUINT16)propObj->Get(String::New(propStr))->Uint32Value();
				}
				break;
			case  PSDATATYPE_BOOL:
				{
					Local<Value> bo = propObj->Get(String::New(propStr));
					PS_DATATYPE_ENUM b = (PS_DATATYPE_ENUM)bo->BooleanValue();
					t->pPropValues_[i].Bool = b;
				}
				break;
			case PSDATATYPE_DOUBLE:
				{
					Local<Value> db = propObj->Get(String::New(propStr));
					double d = db->NumberValue();
					t->pPropValues_[i].Double = d;
				}
				break;
			case PSDATATYPE_INT16:
				{
					Local<Value> int16 =propObj->Get(String::New(propStr));
					PS_DATATYPE_ENUM i16 = (PS_DATATYPE_ENUM)int16->Int32Value();
					t->pPropValues_[i].Int16 = i16;
				}
				break;
			case PSDATATYPE_TIME:
				{
					String::Utf8Value str1(propObj->Get(String::New(propStr)));
					const char * pstr1 = ToCString(str1);
					PS_TIME *d = new PS_TIME();
					std::string s = pstr1;
					STR2PSTIME(d,s);
					t->pPropValues_[i].Time =*d;
				}
				break;
			case PSDATATYPE_INT32:
				{
					Local<Value> int32 = propObj->Get(String::New(propStr));
					PS_DATATYPE_ENUM i32 = (PS_DATATYPE_ENUM)int32->Int32Value();
					t->pPropValues_[i].Int32 = i32;
				}
				break;
			case PSDATATYPE_UINT32:
				{
					Local<Value> int32 = propObj->Get(String::New(propStr));
					PS_DATATYPE_ENUM i32 = (PS_DATATYPE_ENUM)int32->Uint32Value();
					t->pPropValues_[i].UInt32 = i32;
				}
				break;
			case PSDATATYPE_INT64:
				{
					Local<Value> int64 = propObj->Get(String::New(propStr));
					PS_DATATYPE_ENUM i64 = (PS_DATATYPE_ENUM)int64->Int32Value();
					t->pPropValues_[i].Int64 = i64;
				}
				break;
			case PSDATATYPE_UINT64:
				{
					Local<Value> uint64 = propObj->Get(String::New(propStr));
					PS_DATATYPE_ENUM u64 = (PS_DATATYPE_ENUM)uint64->Uint32Value();
					t->pPropValues_[i].UInt64 = u64;
				}
				break;
			case PSDATATYPE_INT8:
				{
					Local<Value> int8 = propObj->Get(String::New(propStr));
					PS_DATATYPE_ENUM i8 = (PS_DATATYPE_ENUM)int8->Int32Value();
					t->pPropValues_[i].Int8 = i8;
				}
				break;
			case PSDATATYPE_UINT8:
				{
					Local<Value> uint8 = propObj->Get(String::New(propStr));
					PS_DATATYPE_ENUM u8 = (PS_DATATYPE_ENUM)uint8->Uint32Value();
					t->pPropValues_[i].UInt8 = u8;
				}
				break;
			case PSDATATYPE_FLOAT:
				{
					Local<Value> fl = propObj->Get(String::New(propStr));
					float f = (float)fl->NumberValue();
					t->pPropValues_[i].Float = f;
				}
				break;
			case  PSDATATYPE_WSTRING:
				{
					String::Utf8Value str1(propObj->Get(String::New(propStr)));
					const char * pstr1 = ToCString(str1);
					t->pPropValues_[i].String.Data = new char[strlen(pstr1)+1];
					strcpy(t->pPropValues_[i].String.Data,(PSSTR)UTF8ToGBK(pstr1).c_str());
					t->pPropValues_[i].String.Data[strlen(pstr1)] = 0;
					t->pPropValues_[i].String.Length = strlen(t->pPropValues_[i].String.Data);
				}
				break;
			case  PSDATATYPE_STRING:
				{
					String::Utf8Value str1(propObj->Get(String::New(propStr)));
					const char * pstr1 = ToCString(str1);
					t->pPropValues_[i].String.Data = new char[strlen(pstr1)+1];
					strcpy(t->pPropValues_[i].String.Data,(PSSTR)UTF8ToGBK(pstr1).c_str());
					t->pPropValues_[i].String.Data[strlen(pstr1)] = 0;
					t->pPropValues_[i].String.Length = strlen(t->pPropValues_[i].String.Data);
				}
				break;
			default:
				{
					Local<Object> errObj = Error::newObj();
					errObj->Set(String::New("code"),Number::New(-1));
					std:string *s =t->errString;
					errObj->Set(String::New("errString"),String::New(GBK2UTF8("数据类型为空或有误!").c_str()));
					delete t;
					return errObj;
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
			Local<Object> errObj = Error::newObj();
			errObj->Set(String::New("code"),Number::New(t->code_));
			string *s =t->errString;
			errObj->Set(String::New("errString"),String::New(GBK2UTF8(s->c_str()).c_str()));
			delete t;
			return errObj;
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
	REQ_FUN_ARG(args.Length()-1, callback);
	Tag* t;
	try {
		t = new Tag(ps, &callback);
		t->tagName = result[0];
		t->id = t->getTagID(UTF8ToGBK(replace_all(result[0],"/","\\").c_str()).c_str(),ps->hHanle_);
		REQ_OBJECT_ARG(1,propObj);
		Handle<Array> propKeys = propObj->GetOwnPropertyNames();
		t->propCount_ = propKeys->Length();
		t->pPropIds_ = new PSUINT16[t->propCount_];
		t->pPropValues_ = new PS_VARIANT[t->propCount_];
		
		for (int i=0;i<t->propCount_;i++)
		{
			String::Utf8Value str1(propKeys->Get(i));
			const char * propStr = ToCString(str1);
			t->pPropIds_[i] = t->getPropID(propStr,t->psNode->hHanle_);
			t->pPropValues_[i].DataType = t->getPropInfo(propStr,t->psNode->hHanle_).DataType;
			switch(t->pPropValues_[i].DataType)
			{
			case PSDATATYPE_UINT16:
				{

					t->pPropValues_[i].UInt16 = (PSUINT16)propObj->Get(String::New(propStr))->Uint32Value();
				}
				break;
			case  PSDATATYPE_BOOL:
				{
					Local<Value> bo = propObj->Get(String::New(propStr));
					PS_DATATYPE_ENUM b = (PS_DATATYPE_ENUM)bo->BooleanValue();
					t->pPropValues_[i].Bool = b;
				}
				break;
			case PSDATATYPE_DOUBLE:
				{
					Local<Value> db = propObj->Get(String::New(propStr));
					double d = db->NumberValue();
					t->pPropValues_[i].Double = d;
				}
				break;
			case PSDATATYPE_INT16:
				{
					Local<Value> int16 =propObj->Get(String::New(propStr));
					PS_DATATYPE_ENUM i16 = (PS_DATATYPE_ENUM)int16->Int32Value();
					t->pPropValues_[i].Int16 = i16;
				}
				break;
			case PSDATATYPE_TIME:
				{
					String::Utf8Value str1(propObj->Get(String::New(propStr)));
					const char * pstr1 = ToCString(str1);
					PS_TIME *d = new PS_TIME();
					std::string s = pstr1;
					STR2PSTIME(d,s);
					t->pPropValues_[i].Time =*d;
				}
				break;
			case PSDATATYPE_INT32:
				{
					Local<Value> int32 = propObj->Get(String::New(propStr));
					PS_DATATYPE_ENUM i32 = (PS_DATATYPE_ENUM)int32->Int32Value();
					t->pPropValues_[i].Int32 = i32;
				}
				break;
			case PSDATATYPE_UINT32:
				{
					Local<Value> int32 = propObj->Get(String::New(propStr));
					PS_DATATYPE_ENUM i32 = (PS_DATATYPE_ENUM)int32->Uint32Value();
					t->pPropValues_[i].UInt32 = i32;
				}
				break;
			case PSDATATYPE_INT64:
				{
					Local<Value> int64 = propObj->Get(String::New(propStr));
					PS_DATATYPE_ENUM i64 = (PS_DATATYPE_ENUM)int64->Int32Value();
					t->pPropValues_[i].Int64 = i64;
				}
				break;
			case PSDATATYPE_UINT64:
				{
					Local<Value> uint64 = propObj->Get(String::New(propStr));
					PS_DATATYPE_ENUM u64 = (PS_DATATYPE_ENUM)uint64->Uint32Value();
					t->pPropValues_[i].UInt64 = u64;
				}
				break;
			case PSDATATYPE_INT8:
				{
					Local<Value> int8 = propObj->Get(String::New(propStr));
					PS_DATATYPE_ENUM i8 = (PS_DATATYPE_ENUM)int8->Int32Value();
					t->pPropValues_[i].Int8 = i8;
				}
				break;
			case PSDATATYPE_UINT8:
				{
					Local<Value> uint8 = propObj->Get(String::New(propStr));
					PS_DATATYPE_ENUM u8 = (PS_DATATYPE_ENUM)uint8->Uint32Value();
					t->pPropValues_[i].UInt8 = u8;
				}
				break;
			case PSDATATYPE_FLOAT:
				{
					Local<Value> fl = propObj->Get(String::New(propStr));
					float f = (float)fl->NumberValue();
					t->pPropValues_[i].Float = f;
				}
				break;
			case  PSDATATYPE_WSTRING:
				{
					String::Utf8Value str1(propObj->Get(String::New(propStr)));
					const char * pstr1 = ToCString(str1);
					t->pPropValues_[i].String.Data = new char[strlen(pstr1)+1];
					strcpy(t->pPropValues_[i].String.Data,(PSSTR)UTF8ToGBK(pstr1).c_str());
					t->pPropValues_[i].String.Data[strlen(pstr1)] = 0;
					t->pPropValues_[i].String.Length = strlen(t->pPropValues_[i].String.Data);
				}
				break;
			case  PSDATATYPE_STRING:
				{
					String::Utf8Value str1(propObj->Get(String::New(propStr)));
					const char * pstr1 = ToCString(str1);
					t->pPropValues_[i].String.Data = new char[strlen(pstr1)+1];
					strcpy(t->pPropValues_[i].String.Data,(PSSTR)UTF8ToGBK(pstr1).c_str());
					t->pPropValues_[i].String.Data[strlen(pstr1)] = 0;
					t->pPropValues_[i].String.Length = strlen(t->pPropValues_[i].String.Data);
				}
				break;
			default:
				{
					t->code_ = -1;
					t->errString = new std::string("数据类型为空或错误!");
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
			argv[0] = Exception::Error(String::New(GBK2UTF8(ts->errString->c_str()).c_str()));
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
		t->id = t->getTagID(UTF8ToGBK(replace_all(result[0],"/","\\").c_str()).c_str(),ps->hHanle_);
		t->tagName = result[0];
		REQ_ARRAY_ARG(1,propID);
		t->propCount_ = propID->Length();
		t->pPropIds_ = new PSUINT16[t->propCount_];
		t->pPropValues_ = PSNULL;
		for (int i=0;i<t->propCount_;i++)
		{
			String::Utf8Value iStr(propID->Get(i));
			const char * idStr = ToCString(iStr);
			t->pPropIds_[i] = t->getPropID(idStr,t->psNode->hHanle_);
		}
		uv_work_t* req = new uv_work_t();
		req->data = t;
		t->psNode->Ref();
		getTagpropsWork(req);
		t->psNode->Unref();
		//如果失败
		if(t->errString) {
			Local<Object> errObj = Error::newObj();
			errObj->Set(String::New("code"),Number::New(t->code_));
			string *s =t->errString;
			errObj->Set(String::New("errString"),String::New(GBK2UTF8(s->c_str()).c_str()));
			delete t;
			return errObj;
		}
		//成功
		Handle<Object> valObj = Object::New();
		for(int i=0;i<t->propCount_;i++)
		{
			String::Utf8Value iStr(propID->Get(i));
			const char * idStr = ToCString(iStr);
			switch(t->pPropValues_[i].DataType)
			{
			case PSDATATYPE_UINT16:
				{
					assert(t->pPropValues_[i].DataType == PSDATATYPE_UINT16);
					valObj->Set(String::New(idStr),Number::New(t->pPropValues_[i].UInt16));
				}
				break;
			case  PSDATATYPE_BOOL:
				{
					assert(t->pPropValues_[i].DataType == PSDATATYPE_BOOL);
					valObj->Set(String::New(idStr),Boolean::New(t->pPropValues_[i].Bool));
				}
				break;
			case PSDATATYPE_DOUBLE:
				{
					assert(t->pPropValues_[i].DataType == PSDATATYPE_DOUBLE);
					valObj->Set(String::New(idStr),Number::New(t->pPropValues_[i].Double));
				}
				break;
			case PSDATATYPE_INT16:
				{
					assert(t->pPropValues_[i].DataType == PSDATATYPE_INT16);
					valObj->Set(String::New(idStr),Number::New(t->pPropValues_[i].Int16));
				}
				break;
			case PSDATATYPE_TIME:
				{
					assert(t->pPropValues_[i].DataType == PSDATATYPE_TIME);
					valObj->Set(String::New(idStr),String::New(PSTIME2STR(t->pPropValues_[i].Time)));
				}
				break;
			case PSDATATYPE_INT32:
				{
					assert(t->pPropValues_[i].DataType == PSDATATYPE_INT32);
					valObj->Set(String::New(idStr),Number::New(t->pPropValues_[i].Int32));
				}
				break;
			case PSDATATYPE_UINT32:
				{
					assert(t->pPropValues_[i].DataType == PSDATATYPE_UINT32);
					valObj->Set(String::New(idStr),Number::New(t->pPropValues_[i].UInt32));
				}
				break;
			case PSDATATYPE_INT64:
				{
					assert(t->pPropValues_[i].DataType == PSDATATYPE_INT64);
					valObj->Set(String::New(idStr),Number::New(t->pPropValues_[i].Int64));
				}
				break;
			case PSDATATYPE_UINT64:
				{
					assert(t->pPropValues_[i].DataType == PSDATATYPE_UINT64);
					valObj->Set(String::New(idStr),Number::New(t->pPropValues_[i].UInt64));
				}
				break;
			case PSDATATYPE_INT8:
				{
					assert(t->pPropValues_[i].DataType == PSDATATYPE_INT8);
					valObj->Set(String::New(idStr),Number::New(t->pPropValues_[i].Int8));
				}
				break;
			case PSDATATYPE_UINT8:
				{
					assert(t->pPropValues_[i].DataType == PSDATATYPE_UINT8);
					valObj->Set(String::New(idStr),Number::New(t->pPropValues_[i].UInt8));
				}
				break;
			case PSDATATYPE_FLOAT:
				{
					assert(t->pPropValues_[i].DataType == PSDATATYPE_FLOAT);
					valObj->Set(String::New(idStr),Number::New(t->pPropValues_[i].Float));
				}
				break;
			case  PSDATATYPE_WSTRING:
				{
					assert(t->pPropValues_[i].DataType == PSDATATYPE_WSTRING);
					assert(t->pPropValues_[i].String.Data != PSNULL);
					valObj->Set(String::New(idStr),String::New((GBK2UTF8(t->pPropValues_[i].String.Data).c_str())));
				}
				break;
			case  PSDATATYPE_STRING:
				{
					assert(t->pPropValues_[i].DataType == PSDATATYPE_STRING);
					assert(t->pPropValues_[i].String.Data != PSNULL);
					valObj->Set(String::New(idStr),String::New((GBK2UTF8(t->pPropValues_[i].String.Data)).c_str()));
				}
				break;
			default:
				{
					Local<Object> errObj = Error::newObj();
					errObj->Set(String::New("code"),Number::New(-1));
					std:string *s =t->errString;
					errObj->Set(String::New("errString"),String::New(GBK2UTF8("数据类型为空或有误!").c_str()));
					delete t;
					return errObj;
				}
		  }
		}
		psAPI_Memory_FreeValueList(&(t->pPropValues_), t->propCount_);
		delete t;
		return valObj;
// 		psAPI_Memory_FreeValueList(&(t->pPropValues_), t->propCount_);
// 		delete t;
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
	REQ_FUN_ARG(args.Length()-1, callback);
	Tag* t;
	try {
		t = new Tag(ps, &callback);
		String::Utf8Value str(args[0]);
		const char * pstr = ToCString(str);
		std::vector<std::string> result=split(pstr,".");
		//	std::cout<<ps->hHanle_<<std::endl;
		t->id = t->getTagID(UTF8ToGBK(replace_all(result[0],"/","\\").c_str()).c_str(),ps->hHanle_);
		t->tagName = result[0];
		REQ_ARRAY_ARG(1,propID);
		t->propCount_ = propID->Length();
		t->pPropIds_ = new PSUINT16[t->propCount_];
		t->propArr = new char *[t->propCount_];
		t->pPropValues_ = PSNULL;
		for (int i=0;i<t->propCount_;i++)
		{
			String::Utf8Value iStr(propID->Get(i));
			const char * idStr = ToCString(iStr);
			t->propArr[i] = new char[strlen(idStr)+1];
			strcpy(t->propArr[i],idStr);
			t->propArr[i][strlen(t->propArr[i])]=0;
			t->pPropIds_[i] = t->getPropID(idStr,t->psNode->hHanle_);
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
Handle<Value> PspaceNode::getSignalPropsSyn(const Arguments& args)
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
		t->id = t->getTagID(UTF8ToGBK(replace_all(result[0],"/","\\").c_str()).c_str(),ps->hHanle_);
		t->tagName = result[0];
		t->propCount_ = 1;
		t->pPropIds_ = new PSUINT16[1];
		t->pPropIds_[0] = t->getPropID(result[1].c_str(),t->psNode->hHanle_);
		if (t->pPropIds_[0] == PSPROPID_UNUSED)
		{
			Local<Object> errObj = Error::newObj();
			errObj->Set(String::New("code"),Number::New(-1));
			string *s =t->errString;
			errObj->Set(String::New("errString"),String::New("参数错误，请检查!"));
			delete t;
			return errObj;
		}
		uv_work_t* req = new uv_work_t();
		req->data = t;
		t->psNode->Ref();
		getTagpropsWork(req);
		t->psNode->Unref();
		//如果失败
		if(t->errString) {
			Local<Object> errObj = Error::newObj();
			errObj->Set(String::New("code"),Number::New(t->code_));
			string *s =t->errString;
			errObj->Set(String::New("errString"),String::New(GBK2UTF8(s->c_str()).c_str()));
			delete t;
			return errObj;
		}
		Local<Object> valObj = Object::New();
			switch(t->pPropValues_[0].DataType)
			{
			case PSDATATYPE_UINT16:
				{
					assert(t->pPropValues_[0].DataType == PSDATATYPE_UINT16);
					valObj->Set(String::New(result[1].c_str()),Number::New(t->pPropValues_[0].UInt16));
				}
				break;
			case  PSDATATYPE_BOOL:
				{
					assert(t->pPropValues_[0].DataType == PSDATATYPE_BOOL);
					valObj->Set(String::New(result[1].c_str()),Boolean::New(t->pPropValues_[0].Bool));
				}
				break;
			case PSDATATYPE_DOUBLE:
				{
					assert(t->pPropValues_[0].DataType == PSDATATYPE_DOUBLE);
					valObj->Set(String::New(result[1].c_str()),Number::New(t->pPropValues_[0].Double));
				}
				break;
			case PSDATATYPE_INT16:
				{
					assert(t->pPropValues_[0].DataType == PSDATATYPE_INT16);
					valObj->Set(String::New(result[1].c_str()),Number::New(t->pPropValues_[0].Int16));
				}
				break;
			case PSDATATYPE_TIME:
				{
					assert(t->pPropValues_[0].DataType == PSDATATYPE_TIME);
					valObj->Set(String::New(result[1].c_str()),String::New(PSTIME2STR(t->pPropValues_[0].Time)));
				}
				break;
			case PSDATATYPE_INT32:
				{
					assert(t->pPropValues_[0].DataType == PSDATATYPE_INT32);
					valObj->Set(String::New(result[1].c_str()),Number::New(t->pPropValues_[0].Int32));
				}
				break;
			case PSDATATYPE_UINT32:
				{
					assert(t->pPropValues_[0].DataType == PSDATATYPE_UINT32);
					valObj->Set(String::New(result[1].c_str()),Number::New(t->pPropValues_[0].UInt32));
				}
				break;
			case PSDATATYPE_INT64:
				{
					assert(t->pPropValues_[0].DataType == PSDATATYPE_INT64);
					valObj->Set(String::New(result[1].c_str()),Number::New(t->pPropValues_[0].Int64));
				}
				break;
			case PSDATATYPE_UINT64:
				{
					assert(t->pPropValues_[0].DataType == PSDATATYPE_UINT64);
					valObj->Set(String::New(result[1].c_str()),Number::New(t->pPropValues_[0].UInt64));
				}
				break;
			case PSDATATYPE_INT8:
				{
					assert(t->pPropValues_[0].DataType == PSDATATYPE_INT8);
					valObj->Set(String::New(result[1].c_str()),Number::New(t->pPropValues_[0].Int8));
				}
				break;
			case PSDATATYPE_UINT8:
				{
					assert(t->pPropValues_[0].DataType == PSDATATYPE_UINT8);
					valObj->Set(String::New(result[1].c_str()),Number::New(t->pPropValues_[0].UInt8));
				}
				break;
			case PSDATATYPE_FLOAT:
				{
					assert(t->pPropValues_[0].DataType == PSDATATYPE_FLOAT);
					valObj->Set(String::New(result[1].c_str()),Number::New(t->pPropValues_[0].Float));
				}
				break;
			case  PSDATATYPE_WSTRING:
				{
					assert(t->pPropValues_[0].DataType == PSDATATYPE_WSTRING);
					assert(t->pPropValues_[0].String.Data != PSNULL);
					valObj->Set(String::New(result[1].c_str()),String::New((GBK2UTF8(t->pPropValues_[0].String.Data).c_str())));
				}
				break;
			case  PSDATATYPE_STRING:
				{
					assert(t->pPropValues_[0].DataType == PSDATATYPE_STRING);
					assert(t->pPropValues_[0].String.Data != PSNULL);
					valObj->Set(String::New(result[1].c_str()),String::New((GBK2UTF8(t->pPropValues_[0].String.Data).c_str())));
				}
				break;
			default:
				{
					Local<Object> errObj = Error::newObj();
					errObj->Set(String::New("code"),Number::New(-1));
					std:string *s =t->errString;
					errObj->Set(String::New("errString"),String::New(GBK2UTF8("数据类型为空或有误!").c_str()));
					delete t;
					return errObj;
				}
			}
		psAPI_Memory_FreeValueList(&(t->pPropValues_), t->propCount_);
		delete t;
		return valObj;
// 		psAPI_Memory_FreeValueList(&(t->pPropValues_), t->propCount_);
// 		delete t;
	} catch(PsException &ex) {
		return ThrowException(Exception::Error(String::New(ex.what())));
	}		
}
Handle<Value> PspaceNode::getSignalpropsAsy(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	String::Utf8Value str(args[0]);
	const char * pstr = ToCString(str);
	std::vector<std::string> result=split(pstr,".");
	REQ_FUN_ARG(args.Length()-1, callback);
	Tag* t;
	try {
		t = new Tag(ps, &callback);
		String::Utf8Value str(args[0]);
		const char * pstr = ToCString(str);
		std::vector<std::string> result=split(pstr,".");
		//	std::cout<<ps->hHanle_<<std::endl;
		t->id = t->getTagID(UTF8ToGBK(replace_all(result[0],"/","\\").c_str()).c_str(),ps->hHanle_);
		t->tagName = result[0];
		t->propCount_ = 1;
		t->pPropIds_ = new PSUINT16[1];
		t->propArr = new char *[1];
		t->pPropValues_ = PSNULL;
		t->pPropIds_[0] = t->getPropID(result[1].c_str(),t->psNode->hHanle_);
		t->propArr = new char *[1];
		t->propArr[0] = new char[strlen(result[1].c_str())+1];
		strcpy(t->propArr[0],result[1].c_str());
		t->propArr[0][strlen(t->propArr[0])]=0;
		
		if (t->pPropIds_[0] == PSPROPID_UNUSED)
		{
			Handle<Value> argv[2];
			argv[0] = Exception::Error(String::New(GBK2UTF8("参数错误，请检查!").c_str()));
			argv[1] = Undefined();
			node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 2, argv);
		}
	} catch(PsException &ex) {
		return scope.Close(ThrowException(Exception::Error(String::New(ex.what()))));
	}
	uv_work_t* req = new uv_work_t();
	req->data = t;
	uv_queue_work(uv_default_loop(), req, getTagpropsWork, (uv_after_work_cb)aftergetSignalTagprops);
	ps->Ref();
	return scope.Close(Undefined());

}
void PspaceNode::aftergetSignalTagprops(uv_work_t* req, int status)
{
	HandleScope scope;
	Tag* t = static_cast<Tag*>(req->data);
	t->psNode->Unref();
	try {
		if(t->errString){
			Handle<Value> argv[2];
			argv[0] = Exception::Error(String::New(GBK2UTF8(t->errString->c_str()).c_str()));
			argv[1] = Undefined();
			node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 2, argv);
		}else{
			Handle<Value> argv[2];
			//结果设置
			argv[0] = Undefined();
			Handle<Object> valObj = Object::New();
			switch(t->pPropValues_[0].DataType)
			{
			case PSDATATYPE_UINT16:
				{
					assert(t->pPropValues_[0].DataType == PSDATATYPE_UINT16);
					valObj->Set(String::New(t->propArr[0]),Number::New(t->pPropValues_[0].UInt16));
				}
				break;
			case  PSDATATYPE_BOOL:
				{
					assert(t->pPropValues_[0].DataType == PSDATATYPE_BOOL);
					valObj->Set(String::New(t->propArr[0]),Boolean::New(t->pPropValues_[0].Bool));
				}
				break;
			case PSDATATYPE_DOUBLE:
				{
					assert(t->pPropValues_[0].DataType == PSDATATYPE_DOUBLE);
					valObj->Set(String::New(t->propArr[0]),Number::New(t->pPropValues_[0].Double));
				}
				break;
			case PSDATATYPE_INT16:
				{
					assert(t->pPropValues_[0].DataType == PSDATATYPE_INT16);
					valObj->Set(String::New(t->propArr[0]),Number::New(t->pPropValues_[0].Int16));
				}
				break;
			case PSDATATYPE_TIME:
				{
					assert(t->pPropValues_[0].DataType == PSDATATYPE_TIME);
					valObj->Set(String::New(t->propArr[0]),String::New(PSTIME2STR(t->pPropValues_[0].Time)));
				}
				break;
			case PSDATATYPE_INT32:
				{
					assert(t->pPropValues_[0].DataType == PSDATATYPE_INT32);
					valObj->Set(String::New(t->propArr[0]),Number::New(t->pPropValues_[0].Int32));
				}
				break;
			case PSDATATYPE_UINT32:
				{
					assert(t->pPropValues_[0].DataType == PSDATATYPE_UINT32);
					valObj->Set(String::New(t->propArr[0]),Number::New(t->pPropValues_[0].UInt32));
				}
				break;
			case PSDATATYPE_INT64:
				{
					assert(t->pPropValues_[0].DataType == PSDATATYPE_INT64);
					valObj->Set(String::New(t->propArr[0]),Number::New(t->pPropValues_[0].Int64));
				}
				break;
			case PSDATATYPE_UINT64:
				{
					assert(t->pPropValues_[0].DataType == PSDATATYPE_UINT64);
					valObj->Set(String::New(t->propArr[0]),Number::New(t->pPropValues_[0].UInt64));
				}
				break;
			case PSDATATYPE_INT8:
				{
					assert(t->pPropValues_[0].DataType == PSDATATYPE_INT8);
					valObj->Set(String::New(t->propArr[0]),Number::New(t->pPropValues_[0].Int8));
				}
				break;
			case PSDATATYPE_UINT8:
				{
					assert(t->pPropValues_[0].DataType == PSDATATYPE_UINT8);
					valObj->Set(String::New(t->propArr[0]),Number::New(t->pPropValues_[0].UInt8));
				}
				break;
			case PSDATATYPE_FLOAT:
				{
					assert(t->pPropValues_[0].DataType == PSDATATYPE_FLOAT);
					valObj->Set(String::New(t->propArr[0]),Number::New(t->pPropValues_[0].Float));
				}
				break;
			case  PSDATATYPE_WSTRING:
				{
					assert(t->pPropValues_[0].DataType == PSDATATYPE_WSTRING);
					assert(t->pPropValues_[0].String.Data != PSNULL);
					valObj->Set(String::New(t->propArr[0]),String::New((GBK2UTF8(t->pPropValues_[0].String.Data).c_str())));
				}
				break;
			case  PSDATATYPE_STRING:
				{
					assert(t->pPropValues_[0].DataType == PSDATATYPE_STRING);
					assert(t->pPropValues_[0].String.Data != PSNULL);
					valObj->Set(String::New(t->propArr[0]),String::New((GBK2UTF8(t->pPropValues_[0].String.Data).c_str())));
				}
				break;
			default:
				{
					Handle<Value> argv[2];
					argv[0] = Exception::Error(String::New(GBK2UTF8("数据类型为空或有错误!").c_str()));
					argv[1] = Undefined();
					node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 2, argv);
				}
			}
			argv[1] = valObj;
			node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 2, argv);
			psAPI_Memory_FreeValueList(&(t->pPropValues_), t->propCount_);
		}

	} catch(PsException &ex) {
		Handle<Value> argv[2];
		argv[0] = Exception::Error(String::New(ex.what()));
		argv[1] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 2, argv);
	} catch(const exception &ex) {
		Handle<Value> argv[2];
		argv[0] = Exception::Error(String::New(t->errString->c_str()));
		argv[1] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 2, argv);
	}
	delete t;
	scope.Close(Undefined());

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
			Handle<Value> argv[3];
			argv[0] = Exception::Error(String::New(GBK2UTF8(t->errString->c_str()).c_str()));
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
							valObj->Set(String::New(t->propArr[i]),Number::New(t->pPropValues_[i].UInt16));
						}
						break;
					case  PSDATATYPE_BOOL:
						{
							assert(t->pPropValues_[i].DataType == PSDATATYPE_BOOL);
							valObj->Set(String::New(t->propArr[i]),Boolean::New(t->pPropValues_[i].Bool));
						}
						break;
					case PSDATATYPE_DOUBLE:
						{
							assert(t->pPropValues_[i].DataType == PSDATATYPE_DOUBLE);
							valObj->Set(String::New(t->propArr[i]),Number::New(t->pPropValues_[i].Double));
						}
						break;
					case PSDATATYPE_INT16:
						{
							assert(t->pPropValues_[i].DataType == PSDATATYPE_INT16);
							valObj->Set(String::New(t->propArr[i]),Number::New(t->pPropValues_[i].Int16));
						}
						break;
					case PSDATATYPE_TIME:
						{
							assert(t->pPropValues_[i].DataType == PSDATATYPE_TIME);
							valObj->Set(String::New(t->propArr[i]),String::New(PSTIME2STR(t->pPropValues_[i].Time)));
						}
						break;
					case PSDATATYPE_INT32:
						{
							assert(t->pPropValues_[i].DataType == PSDATATYPE_INT32);
							valObj->Set(String::New(t->propArr[i]),Number::New(t->pPropValues_[i].Int32));
						}
						break;
					case PSDATATYPE_UINT32:
						{
							assert(t->pPropValues_[i].DataType == PSDATATYPE_UINT32);
							valObj->Set(String::New(t->propArr[i]),Number::New(t->pPropValues_[i].UInt32));
						}
						break;
					case PSDATATYPE_INT64:
						{
							assert(t->pPropValues_[i].DataType == PSDATATYPE_INT64);
							valObj->Set(String::New(t->propArr[i]),Number::New(t->pPropValues_[i].Int64));
						}
						break;
					case PSDATATYPE_UINT64:
						{
							assert(t->pPropValues_[i].DataType == PSDATATYPE_UINT64);
							valObj->Set(String::New(t->propArr[i]),Number::New(t->pPropValues_[i].UInt64));
						}
						break;
					case PSDATATYPE_INT8:
						{
							assert(t->pPropValues_[i].DataType == PSDATATYPE_INT8);
							valObj->Set(String::New(t->propArr[i]),Number::New(t->pPropValues_[i].Int8));
						}
						break;
					case PSDATATYPE_UINT8:
						{
							assert(t->pPropValues_[i].DataType == PSDATATYPE_UINT8);
							valObj->Set(String::New(t->propArr[i]),Number::New(t->pPropValues_[i].UInt8));
						}
						break;
					case PSDATATYPE_FLOAT:
						{
							assert(t->pPropValues_[i].DataType == PSDATATYPE_FLOAT);
							valObj->Set(String::New(t->propArr[i]),Number::New(t->pPropValues_[i].Float));
						}
						break;
					case  PSDATATYPE_WSTRING:
						{
							assert(t->pPropValues_[i].DataType == PSDATATYPE_WSTRING);
							assert(t->pPropValues_[i].String.Data != PSNULL);
							valObj->Set(String::New(t->propArr[i]),String::New((GBK2UTF8(t->pPropValues_[i].String.Data)).c_str()));
						}
						break;
					case  PSDATATYPE_STRING:
						{
							assert(t->pPropValues_[i].DataType == PSDATATYPE_STRING);
							assert(t->pPropValues_[i].String.Data != PSNULL);
							valObj->Set(String::New(t->propArr[i]),String::New((GBK2UTF8(t->pPropValues_[i].String.Data).c_str())));
						}
						break;
					default:
						{
							argv[0] = String::New(GBK2UTF8("数据类型为空或不存在!").c_str());
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
		t->id = t->getTagID(UTF8ToGBK(replace_all(result[0],"/","\\").c_str()).c_str(),ps->hHanle_);
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
				if(settings->Has(v8::String::New("value")))
				{
					Local<Value> obj = settings->Get(String::New("value"));
					if (obj->IsInt32())
					{
						t->pDataValues_[i].DataType = PSDATATYPE_INT32;
						GET_INTER(settings, "value", t->pDataValues_[i].Int32);
					}else if (obj->IsBoolean())
					{
						t->pDataValues_[i].DataType = PSDATATYPE_BOOL;
						t->pDataValues_[i].Bool =obj->ToBoolean()->Value();
						//std::cout<<rbaton->varData_.Bool<<std::endl;
					}else if (obj->IsNumber())
					{

						t->pDataValues_[i].DataType = PSDATATYPE_DOUBLE;
						GET_NUMBER(settings, "value", t->pDataValues_[i].Double);

					}else if (obj->IsString())
					{
						
						t->pDataValues_[i].DataType = PSDATATYPE_STRING;
						String::Utf8Value tmpStr(settings->Get(String::New("value")));
						const char *pstr = ToCString(tmpStr);
						t->pDataValues_[i].String.Data = new char[strlen(pstr)+1];
						strcpy(t->pDataValues_[i].String.Data,pstr);
						t->pDataValues_[i].String.Data[strlen(pstr)]=0;
						t->pDataValues_[i].String.Length = strlen(t->pDataValues_[i].String.Data);
					}	

				}
				//GET_NUMBER(settings, "value", t->pDataValues_[i].Double);
				if (settings->Has(String::New("time")))
				{
					V8DATE2PSTIME(&(t->pTimeStamps_[i]),settings->Get(String::New("time")));
					
				}
				if (settings->Has(String::New("quality")))
				{
                    Local<Value> qObj = settings->Get(String::New("quality"));
                    if (qObj->IsInt32())
                    {
                        GET_INTER(settings, "quality", t->pQualities_[i]);
                    }
                    else if (qObj->IsString())
                    {
                        std::string str1 = "";
                        OBJ_GET_STRING(settings, "quality",str1);
                        t->pQualities_[i] = (PS_QUALITY_ENUM)t->getQuality(str1.c_str());
                    }
				}
				t->tagIds_[i] = t->id;
			}
		}else if(!args[1]->IsUndefined())
		{
			Local<Value> valObj = args[1];
			for (int i=0;i<t->tagCount_;i++)
			{	
				t->tagIds_[i] = t->id;
				if (valObj->IsInt32())
				{
					t->pDataValues_[i].DataType = PSDATATYPE_INT32;
					t->pDataValues_[i].Int32 = valObj->ToInt32()->Value();
				}else if (valObj->IsBoolean())
				{
					t->pDataValues_[i].DataType = PSDATATYPE_BOOL;
					t->pDataValues_[i].Bool = valObj->ToBoolean()->Value(); 
				}else if (valObj->IsNumber())
				{
					t->pDataValues_[i].DataType = PSDATATYPE_DOUBLE;
					t->pDataValues_[i].Double = valObj->ToNumber()->Value();
				}else if (valObj->IsString())
				{
					t->pDataValues_[i].DataType = PSDATATYPE_STRING;
					String::Utf8Value tmpStr(valObj);
					const char *pstr = ToCString(tmpStr);
					t->pDataValues_[i].String.Data = new char[strlen(pstr)+1];
					strcpy(t->pDataValues_[i].String.Data,pstr);
					t->pDataValues_[i].String.Data[strlen(pstr)]=0;
					t->pDataValues_[i].String.Length = strlen(t->pDataValues_[i].String.Data);
				}
				//t->pDataValues_[i].DataType = PSDATATYPE_DOUBLE;
				//t->pDataValues_[i].Double = args[1]->ToNumber()->Value();
				if (!args[2]->IsUndefined())
				{
					String::Utf8Value str(args[2]);
					const char * pstr1 = ToCString(str);
					t->pQualities_[i] = (PS_QUALITY_ENUM)t->getQuality(pstr1);
				}
				if (!args[3]->IsUndefined())
				{
					Local<Date> date = Local<Date>::Cast(args[3]);
					V8DATE2PSTIME(&(t->pTimeStamps_[i]),date);
					 
				}			
			}
		}else{
			t->code_ = -1;
			t->errString = new std::string("参数错误!");
			Local<Object> errObj = Error::newObj();
			errObj->Set(String::New("code"),Number::New(-1));
			errObj->Set(String::New("errString"),String::New(GBK2UTF8("参数错误!").c_str()));
			return errObj;
		}
		if (t->id==PSTAGID_UNUSED)
		{
			t->code_ = -1;
			t->errString = new std::string("tag not found!");
			Local<Object> errObj = Error::newObj();
			errObj->Set(String::New("code"),Number::New(-1));
			errObj->Set(String::New("errString"),String::New(GBK2UTF8("测点不存在!").c_str()));
			return errObj;
		}
		uv_work_t* req = new uv_work_t();
		req->data = t;
		t->psNode->Ref();
		if (result.at(2) == "insertReplace")
		{
			hisInserReplaceWork(req);
		}
		if (result.at(2) == "replace")
		{
			hisReplaceWork(req);
		}
		if (result.at(2) == "insert")
		{
			hisInsertWork(req);
		}
		t->psNode->Unref();
		//如果失败
		if(t->errString) {
			Local<Object> errObj = Error::newObj();
			errObj->Set(String::New("code"),Number::New(t->code_));
			std:string *s =t->errString;
			errObj->Set(String::New("errString"),String::New(GBK2UTF8(s->c_str()).c_str()));
			delete t;
			return errObj;
		}
		//成功返回测点长名
		return String::New(result[0].c_str());
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
		/*if (args.Length()==3)
		{
			REQ_FUN_ARG(2, callback);
			t = new TagHis(ps, &callback);
		}else
		{
			REQ_FUN_ARG(4, callback);
			t = new TagHis(ps, &callback);
		}*/
		REQ_FUN_ARG(args.Length()-1, callback);
		t = new TagHis(ps, &callback);
		t->tagName = result[0];
		t->id = t->getTagID(UTF8ToGBK(replace_all(result[0],"/","\\").c_str()).c_str(),ps->hHanle_);
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
				//t->pDataValues_[i].DataType = PSDATATYPE_DOUBLE;
				//GET_NUMBER(settings, "value", t->pDataValues_[i].Double);
				if(settings->Has(v8::String::New("value")))
				{
					Local<Value> obj = settings->Get(String::New("value"));
					if (obj->IsInt32())
					{
						t->pDataValues_[i].DataType = PSDATATYPE_INT32;
						GET_INTER(settings, "value", t->pDataValues_[i].Int32);
					}else if (obj->IsBoolean())
					{
						t->pDataValues_[i].DataType = PSDATATYPE_BOOL;
						t->pDataValues_[i].Bool =obj->ToBoolean()->Value();
						//std::cout<<rbaton->varData_.Bool<<std::endl;
					}else if (obj->IsNumber())
					{

						t->pDataValues_[i].DataType = PSDATATYPE_DOUBLE;
						GET_NUMBER(settings, "value", t->pDataValues_[i].Double);

					}else if (obj->IsString())
					{

						t->pDataValues_[i].DataType = PSDATATYPE_STRING;
						String::Utf8Value tmpStr(settings->Get(String::New("value")));
						const char *pstr = ToCString(tmpStr);
						t->pDataValues_[i].String.Data = new char[strlen(pstr)+1];
						strcpy(t->pDataValues_[i].String.Data,pstr);
						t->pDataValues_[i].String.Data[strlen(pstr)]=0;
						t->pDataValues_[i].String.Length = strlen(t->pDataValues_[i].String.Data);
					}	

				}
				V8DATE2PSTIME(&(t->pTimeStamps_[i]),settings->Get(String::New("time")));
				std::string str1 = "";
				OBJ_GET_STRING(settings, "quality",str1);
				t->pQualities_[i] = (PS_QUALITY_ENUM)t->getQuality(str1.c_str());
				t->tagIds_[i] = t->id;
			}
		}else if (!args[1]->IsUndefined())
		{
			Local<Value> valObj = args[1];
			for (int i=0;i<t->tagCount_;i++)
			{	
				t->tagIds_[i] = t->id;
				if (valObj->IsInt32())
				{
					t->pDataValues_[i].DataType = PSDATATYPE_INT32;
					t->pDataValues_[i].Int32 = valObj->ToInt32()->Value();
				}else if (valObj->IsBoolean())
				{
					t->pDataValues_[i].DataType = PSDATATYPE_BOOL;
					t->pDataValues_[i].Bool = valObj->ToBoolean()->Value(); 
				}else if (valObj->IsNumber())
				{
					t->pDataValues_[i].DataType = PSDATATYPE_DOUBLE;
					t->pDataValues_[i].Double = valObj->ToNumber()->Value();
				}else if (valObj->IsString())
				{
					t->pDataValues_[i].DataType = PSDATATYPE_STRING;
					String::Utf8Value tmpStr(valObj);
					const char *pstr = ToCString(tmpStr);
					t->pDataValues_[i].String.Data = new char[strlen(pstr)+1];
					strcpy(t->pDataValues_[i].String.Data,pstr);
					t->pDataValues_[i].String.Data[strlen(pstr)]=0;
					t->pDataValues_[i].String.Length = strlen(t->pDataValues_[i].String.Data);
				}
				//t->pDataValues_[i].DataType = PSDATATYPE_DOUBLE;
				//t->pDataValues_[i].Double = args[1]->ToNumber()->Value();
				if (!args[2]->IsUndefined())
				{
					String::Utf8Value str(args[2]);
					const char * pstr1 = ToCString(str);
					t->pQualities_[i] = (PS_QUALITY_ENUM)t->getQuality(pstr1);
				}
				if (!args[3]->IsUndefined())
				{
					Local<Date> date = Local<Date>::Cast(args[3]);
					V8DATE2PSTIME(&(t->pTimeStamps_[i]),date);
				}
			}
		}	
	} catch(PsException &ex) {
		return scope.Close(ThrowException(Exception::Error(String::New(ex.what()))));
	}
	uv_work_t* req = new uv_work_t();
	req->data = t;
	if (result.at(2) == "insertReplace")
	{
		uv_queue_work(uv_default_loop(), req, hisInserReplaceWork, (uv_after_work_cb)afterHis);
	}
	if (result.at(2) == "replace")
	{
		uv_queue_work(uv_default_loop(), req, hisReplaceWork, (uv_after_work_cb)afterHis);
	}
	if (result.at(2) == "insert")
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
				for(int i=0;i<t->tagCount_;i++)
				{
					t->code_ = pAPIErrors[i];
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
			argv[0] = Exception::Error(String::New(GBK2UTF8(t->errString->c_str()).c_str()));
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
				//t->code_ = nRet;
				for(int i=0;i<t->tagCount_;i++)
				{
					t->code_ = pAPIErrors[i];
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
				//t->code_ = nRet;
				for(int i=0;i<t->tagCount_;i++)
				{
					t->code_ = pAPIErrors[i];
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

Handle<Value> PspaceNode::readHisAtTimeSyn(const Arguments& args)
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
		t->id = t->getTagID(UTF8ToGBK(replace_all(result[0],"/","\\").c_str()).c_str(),ps->hHanle_);
		t->tagName = result[0];
		REQ_ARRAY_ARG(1, array);
		t->timeStatusLen_ = array->Length();
		t->nNumTimeStamps_ = array->Length();
		t->pTimeStamps_ = new PS_TIME[array->Length()];
		t->tagCount_ = 1;
		t->tagIds_ = new PSUINT32[t->tagCount_];
		//t->dataStatus_ = new int[array->Length()];
		t->tagIds_[0] = t->id;
		for (int i=0;i<t->nNumTimeStamps_;i++)
		{
			Local<Date> date  =  Local<Date>::Cast(array->Get(i));
			V8DATE2PSTIME(&(t->pTimeStamps_[i]),date);
		}
		uv_work_t* req = new uv_work_t();
		req->data = t;
		t->psNode->Ref();
		readHisAtTimeWork(req);
		t->psNode->Unref();
		//如果失败
		if(t->errString) {
			Local<Object> errObj = Error::newObj();
			errObj->Set(String::New("code"),Number::New(t->code_));
			std:string *s =t->errString;
			//std::cout<<s->c_str()<<std::endl;
			errObj->Set(String::New("errString"),String::New(GBK2UTF8(s->c_str()).c_str()));
			delete t;
			return errObj;
		}
		// 成功返回值数组
		//数组对象
	
		Handle<Array> arrObj = Array::New(t->pHisDataList_->DataCount);
		for (int n=0;n<t->tagCount_;n++)
		{
			
				for (int m = 0; m < (t->pHisDataList_+n)->DataCount; m++)
				{
					if (t->dataStatus_[m]==1)
					{
						//Local<Object> obj = Object::New();
						
						//std::cout<<((t->pHisDataList_+n)->DataList+m)->Value.Double<<std::endl;
						//obj->Set(String::New("value"),Number::New(((t->pHisDataList_+n)->DataList+m)->Value.Double));
						//obj->Set(String::New("quality"),Int32::New(((t->pHisDataList_+n)->DataList+m)->Quality));
						//obj->Set(String::New("time"),PSTIME2V8DATE((((t->pHisDataList_+n)->DataList+m)->Time)));
						arrObj->Set(m,getRealObj(((t->pHisDataList_+n)->DataList+m)));
					}else
					{
						arrObj->Set(m,Undefined());
					}
				}
		}	
		delete t;
		return arrObj;
	} catch(PsException &ex) {
		return ThrowException(Exception::Error(String::New(ex.what())));
	}	
}
Handle<Value> PspaceNode::readHisAtTimeAsy(const Arguments& args)
{
	
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	TagHis* t;
	REQ_FUN_ARG(args.Length()-1, callback);
	try {
		Handle<Object> robj = Object::New();
		t = new TagHis(ps,&callback);
		String::Utf8Value str(args[0]);
		const char * pstr = ToCString(str);
		std::vector<std::string> result=split(pstr,".");
		t->id = t->getTagID(UTF8ToGBK(replace_all(result[0],"/","\\").c_str()).c_str(),ps->hHanle_);
		t->tagName = result[0];
		
		REQ_ARRAY_ARG(1, array);

		t->nNumTimeStamps_ = array->Length();
		t->pTimeStamps_ = new PS_TIME[array->Length()];
		t->tagCount_ = 1;
		t->tagIds_ = new PSUINT32[t->tagCount_];
		t->tagIds_[0] = t->id;
		for (int i=0;i<t->nNumTimeStamps_;i++)
		{
			Local<Date> date  =  Local<Date>::Cast(array->Get(i));
			V8DATE2PSTIME(&(t->pTimeStamps_[i]),date);
			
		}

	} catch(PsException &ex) {
		return scope.Close(ThrowException(Exception::Error(String::New(ex.what()))));
	}
	uv_work_t* req = new uv_work_t();
	req->data = t;

	uv_queue_work(uv_default_loop(), req, readHisAtTimeWork, (uv_after_work_cb)afterReadHisAtTime);
	ps->Ref();
	return scope.Close(Undefined());
}
void PspaceNode::readHisAtTimeWork(uv_work_t* req)
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
		
			nRet = psAPI_His_ReadAtTime(t->psNode->hHanle_, t->nNumTimeStamps_, t->pTimeStamps_, 
				t->tagCount_, t->tagIds_, &(t->pHisDataList_), &pAPIErrors);
			if (PSERR(nRet) && nRet != PSERR_FAIL_IN_BATCH)
			{
				t->code_ = nRet;
				t->errString = new std::string(psAPI_Commom_GetErrorDesc(nRet));
			}
				
			//std::cout<<t->pHisDataList_->DataCount<<std::endl;
			for (int n = 0; n < t->tagCount_; n++)
			{
				t->dataStatus_ = new int[t->pHisDataList_->DataCount];
				for (int m=0;m<t->pHisDataList_->DataCount;m++)
				{
					
					t->dataStatus_[m]=1;
					if (nRet== PSERR_FAIL_IN_BATCH && PSERR(*(pAPIErrors+n)))
					{
						t->dataStatus_[m]=pAPIErrors[n];
					}
				}
			}
			//std::cout<<((t->pHisDataList_+)->DataList+m)->Value.Double<<std::endl;
			if (nRet == PSERR_FAIL_IN_BATCH)
			{
				
				psAPI_Memory_FreeAndNull((PSVOID**)&pAPIErrors);
			}
			//psAPI_Memory_FreeTagHisDataList(&(t->pHisDataList_), 1);
			
		}	
	}catch(PsException &ex) {
		t->errString = new string(ex.what());
	} catch (const exception& ex) {
		t->errString = new string(ex.what());
	} catch (...) {
		t->errString = new string("Unknown Error");
	}
}
void PspaceNode::afterReadHisAtTime(uv_work_t* req,int status)
{
	HandleScope scope;
	TagHis* t = static_cast<TagHis*>(req->data);
	t->psNode->Unref();
	try {
		if(t->errString){
			Handle<Value> argv[2];
			argv[0] = Exception::Error(String::New(GBK2UTF8(t->errString->c_str()).c_str()));
			argv[1] = Undefined();
			node::MakeCallback(Context::GetCurrent()->Global(), t->callback,2, argv);
		}else{
			Handle<Value> argv[2];
			//结果设置
			argv[0] = Undefined();
			Handle<Array> arrObj = Array::New(t->pHisDataList_->DataCount);
			for (int n=0;n<t->tagCount_;n++)
			{
				for (int m = 0; m < (t->pHisDataList_+n)->DataCount; m++)
				{
					if (t->dataStatus_[m]==1)
					{
						//Local<Object> obj = Object::New();
						//std::cout<<((t->pHisDataList_+n)->DataList+m)->Value.Double<<std::endl;
						//obj->Set(String::New("value"),Number::New(((t->pHisDataList_+n)->DataList+m)->Value.Double));
						//obj->Set(String::New("quality"),Int32::New(((t->pHisDataList_+n)->DataList+m)->Quality));
						//obj->Set(String::New("time"),PSTIME2V8DATE((((t->pHisDataList_+n)->DataList+m)->Time)));
						arrObj->Set(m,getRealObj(((t->pHisDataList_+n)->DataList+m)));
					}else
					{
						Local<Object> obj = Object::New();
						//std::cout<<((t->pHisDataList_+n)->DataList+m)->Value.Double<<std::endl;
						obj->Set(String::New("value"),Undefined());
						obj->Set(String::New("quality"),Undefined());
						obj->Set(String::New("time"),Undefined());
						arrObj->Set(m,Undefined());
					}
				}
			}	
			argv[1] = arrObj;
			node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 2, argv);
		}
	} catch(PsException &ex) {
		Handle<Value> argv[2];
		argv[0] = Exception::Error(String::New(ex.what()));
		argv[1] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 2, argv);
	} catch(const exception &ex) {
		Handle<Value> argv[2];
		argv[0] = Exception::Error(String::New(t->errString->c_str()));
		argv[1] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 2, argv);
	}
	delete t;
	scope.Close(Undefined());
}

Handle<Value> PspaceNode::readHisRawSyn(const Arguments& args)
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
		t->id = t->getTagID(UTF8ToGBK(replace_all(result[0],"/","\\").c_str()).c_str(),ps->hHanle_);
		t->tagName = t->getTagName(t->id);
        //added by chengxh at 2016-1-15 for 查询原始数据
        if (args.Length()>=4)
        {
            t->nMaxNumOfReturnValues_ =  args[4]->ToUint32()->Value();
        }
		t->bBounds_ = args[3]->ToBoolean()->Value();
		String::Utf8Value tStr(args[1]);
		Local<Date> date  =  Local<Date>::Cast(args[1]);
		V8DATE2PSTIME(&(t->startTime_),date);
	
		Local<Date> date1  =  Local<Date>::Cast(args[2]);
		V8DATE2PSTIME(&(t->endTime_),date1);
		t->tagCount_ = 1;
		t->tagIds_ = new PSUINT32[t->tagCount_];
		t->tagIds_[0] = t->id;
		
		uv_work_t* req = new uv_work_t();
		req->data = t;
		t->psNode->Ref();
		readHisRawWork(req);
		t->psNode->Unref();
		//如果失败
		if(t->errString) {
			Local<Object> errObj = Error::newObj();
			errObj->Set(String::New("code"),Number::New(t->code_));
			std:string *s =t->errString;
			errObj->Set(String::New("errString"),String::New(GBK2UTF8(s->c_str()).c_str()));
			delete t;
			return errObj;
		}
		// 成功返回值数组
		//数组对象
		Handle<Array> arrObj = Array::New(t->pHisDataList_->DataCount);
		for (int n=0;n<t->tagCount_;n++)
		{
			for (int m = 0; m < (t->pHisDataList_+n)->DataCount; m++)
			{
				//Local<Object> obj = Object::New();
				//obj->Set(String::New("value"),Number::New(((t->pHisDataList_+n)->DataList+m)->Value.Double));
				//obj->Set(String::New("quality"),Int32::New(((t->pHisDataList_+n)->DataList+m)->Quality));
				//obj->Set(String::New("time"),PSTIME2V8DATE(((t->pHisDataList_+n)->DataList+m)->Time));
				if (t->dataStatus_[m]==1)
				{
					arrObj->Set(m,getRealObj(((t->pHisDataList_+n)->DataList+m)));
				}else{
					arrObj->Set(m,Undefined());
				}
				
			}
		}
		delete t;
		return arrObj;
	} catch(PsException &ex) {
		return ThrowException(Exception::Error(String::New(ex.what())));
	}	
}
Handle<Value> PspaceNode::readHisRawAsy(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	TagHis* t;
	REQ_FUN_ARG(args.Length()-1, callback);
	try {
		Handle<Object> robj = Object::New();
		t = new TagHis(ps,&callback);
		String::Utf8Value str(args[0]);
		const char * pstr = ToCString(str);
		std::vector<std::string> result=split(pstr,".");
		t->id = t->getTagID(UTF8ToGBK(replace_all(result[0],"/","\\").c_str()).c_str(),ps->hHanle_);
		t->tagName = t->getTagName(t->id);

		t->bBounds_ = args[3]->ToBoolean()->Value();
		Local<Date> date  =  Local<Date>::Cast(args[1]);
		V8DATE2PSTIME(&(t->startTime_),date);
		Local<Date> date1  =  Local<Date>::Cast(args[2]);
		V8DATE2PSTIME(&(t->endTime_),date1);
		t->tagCount_ = 1;
		t->tagIds_ = new PSUINT32[t->tagCount_];
		t->tagIds_[0] = t->id;
	} catch(PsException &ex) {
		return scope.Close(ThrowException(Exception::Error(String::New(ex.what()))));
	}
	uv_work_t* req = new uv_work_t();
	req->data = t;
	uv_queue_work(uv_default_loop(), req, readHisRawWork, (uv_after_work_cb)afterReadHisRaw);
	ps->Ref();
	return scope.Close(Undefined());
}
void PspaceNode::readHisRawWork(uv_work_t* req)
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
			nRet = psAPI_His_ReadRaw(t->psNode->hHanle_, t->startTime_, t->endTime_, t->nMaxNumOfReturnValues_, t->bBounds_, t->tagCount_, 
				t->tagIds_, &(t->pHisDataList_), &pAPIErrors);

			if (PSERR(nRet) && nRet != PSERR_FAIL_IN_BATCH)
			{
				t->code_ = nRet;
				t->errString = new std::string(psAPI_Commom_GetErrorDesc(nRet));

			}
			for (int n = 0; n < t->tagCount_; n++)
			{
				t->dataStatus_ = new int[t->pHisDataList_->DataCount];
				for (int m=0;m<t->pHisDataList_->DataCount;m++)
				{
					t->dataStatus_[m]=1;
					if (nRet== PSERR_FAIL_IN_BATCH && PSERR(*(pAPIErrors+n)))
					{
						t->dataStatus_[m]=pAPIErrors[n];
					}
				}
			}
			//std::cout<<((t->pHisDataList_+)->DataList+m)->Value.Double<<std::endl;
			if (nRet == PSERR_FAIL_IN_BATCH)
			{
				psAPI_Memory_FreeAndNull((PSVOID**)&pAPIErrors);
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
void PspaceNode::afterReadHisRaw(uv_work_t* req,int status)
{
	HandleScope scope;
	TagHis* t = static_cast<TagHis*>(req->data);
	t->psNode->Unref();
	try {
		if(t->errString){
			Handle<Value> argv[2];
			argv[0] = Exception::Error(String::New(GBK2UTF8(t->errString->c_str()).c_str()));
			argv[1] = Undefined();
			node::MakeCallback(Context::GetCurrent()->Global(), t->callback,2, argv);
		}else{
			Handle<Value> argv[2];
			//结果设置
			argv[0] = Undefined();
			Handle<Array> arrObj = Array::New(t->pHisDataList_->DataCount);
			for (int n=0;n<t->tagCount_;n++)
			{
				for (int m = 0; m < (t->pHisDataList_+n)->DataCount; m++)
				{
					//Local<Object> obj = Object::New();
					//obj->Set(String::New("value"),Number::New(((t->pHisDataList_+n)->DataList+m)->Value.Double));
					//obj->Set(String::New("quality"),Int32::New(((t->pHisDataList_+n)->DataList+m)->Quality));
					//obj->Set(String::New("time"),PSTIME2V8DATE(((t->pHisDataList_+n)->DataList+m)->Time));
					if (t->dataStatus_[m]==1)
					{
						arrObj->Set(m,getRealObj(((t->pHisDataList_+n)->DataList+m)));
					}else{
						arrObj->Set(m,Undefined());
					}
				}
			}
			argv[1] = arrObj;
			node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 2, argv);
		}
	} catch(PsException &ex) {
		Handle<Value> argv[2];
		argv[0] = Exception::Error(String::New(ex.what()));
		argv[1] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 2, argv);
	} catch(const exception &ex) {
		Handle<Value> argv[2];
		argv[0] = Exception::Error(String::New(t->errString->c_str()));
		argv[1] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 2, argv);
	}
	delete t;
	scope.Close(Undefined());
}

Handle<Value> PspaceNode::readHisProcessSyn(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	TagHis* t;
	try { 
		
		t = new TagHis(ps,NULL);
		String::Utf8Value str(args[0]);
		const char * pstr = ToCString(str);
		std::vector<std::string> result=split(pstr,".");
		t->id = t->getTagID(UTF8ToGBK(replace_all(result[0],"/","\\").c_str()).c_str(),ps->hHanle_);
		t->tagCount_ = 1;
		t->pAggregates_ = new PSUINT32[t->tagCount_];
		t->tagIds_ = new PSUINT32[t->tagCount_];
		Local<Object> robj =  Object::New();
		if (!args[1]->IsDate())
		{
			REQ_OBJECT_ARG(1, settings);
			t->tagIds_[0] = t->id;
			Local<Date> date  =  Local<Date>::Cast(settings->Get(String::New("startTime")));
			V8DATE2PSTIME(&(t->startTime_),date);

			Local<Date> date1  =  Local<Date>::Cast(settings->Get(String::New("endTime")));
			V8DATE2PSTIME(&(t->endTime_),date1);
			int interval=0;
			GET_NUMBER(settings,"resampleInterval",interval);
			PS_TIME tmp = {interval,0};
			t->resampleInterval_ = tmp;
			
			for (int i=0;i<t->tagCount_;i++)
			{
				std::string agg = "";
				OBJ_GET_STRING(settings, "aggregates",agg);
				t->pAggregates_[i] = (PS_HIS_AGGREGATE_ENUM)t->getAggregate(agg.c_str());
			}
		}else if(args[1]->IsDate()&&args[2]->IsDate())
		{
			
			t->tagIds_[0] = t->id;
			Local<Date> date = Local<Date>::Cast(args[1]);
			V8DATE2PSTIME(&(t->startTime_),date);
			Local<Date> date1 = Local<Date>::Cast(args[2]);
			V8DATE2PSTIME(&(t->endTime_),date1);
			PS_TIME tmp1 = {args[3]->ToUint32()->Value(),0};
			t->resampleInterval_ = tmp1;
			for (int i=0;i<t->tagCount_;i++)
			{	
				String::Utf8Value t1(args[4]);
				const char * tt1 = ToCString(t1);
				t->pAggregates_[i] = (PS_HIS_AGGREGATE_ENUM)t->getAggregate(tt1);
			}
		}else{
			t->code_ = -1;
			t->errString = new std::string("参数错误!");
			Local<Object> errObj = Error::newObj();
			errObj->Set(String::New("code"),Number::New(-1));
			errObj->Set(String::New("errString"),String::New(GBK2UTF8("参数错误!").c_str()));
			return errObj;
		}
		if (t->id==PSTAGID_UNUSED)
		{
			t->code_ = -1;
			t->errString = new std::string("tag not found!");
			Local<Object> errObj = Error::newObj();
			errObj->Set(String::New("code"),Number::New(-1));
			errObj->Set(String::New("errString"),String::New("tag not found!"));
			return errObj;
		}
		uv_work_t* req = new uv_work_t();
		req->data = t;
		t->psNode->Ref();
		readHisProcessWork(req);
		t->psNode->Unref();
		//如果失败
		if(t->errString) {
			Local<Object> errObj = Error::newObj();
			errObj->Set(String::New("code"),Number::New(t->code_));
			std:string *s =t->errString;
			errObj->Set(String::New("errString"),String::New(GBK2UTF8(s->c_str()).c_str()));
			delete t;
			return errObj;
		}
		//成功返回数据
		Handle<Array> arrObj = Array::New(t->pHisDataList_->DataCount);
		for (int n=0;n<t->tagCount_;n++)
		{
			for (int m = 0; m < (t->pHisDataList_+n)->DataCount; m++)
			{
				//Local<Object> obj = Object::New();
				//obj->Set(String::New("value"),Number::New(((t->pHisDataList_+n)->DataList+m)->Value.Double));
				//obj->Set(String::New("quality"),Int32::New(((t->pHisDataList_+n)->DataList+m)->Quality));
				//obj->Set(String::New("time"),PSTIME2V8DATE(((t->pHisDataList_+n)->DataList+m)->Time));
				if (t->dataStatus_[m]==1)
				{
					arrObj->Set(m,getRealObj(((t->pHisDataList_+n)->DataList+m)));
				}else{
					arrObj->Set(m,Undefined());
				}
			}
		}
		delete t;
		return arrObj;
	} catch(PsException &ex) {
		return ThrowException(Exception::Error(String::New(ex.what())));
	}	
}
Handle<Value> PspaceNode::readHisProcessAsy(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	TagHis* t;
	try {
		Handle<Object> robj = Object::New();
		String::Utf8Value str(args[0]);
		const char * pstr = ToCString(str);
		std::vector<std::string> result=split(pstr,".");
		REQ_FUN_ARG(args.Length()-1, callback);
		t = new TagHis(ps,&callback);
		if (!args[1]->IsDate())
		{
			t->id = t->getTagID(UTF8ToGBK(replace_all(result[0],"/","\\").c_str()).c_str(),ps->hHanle_);
			t->tagCount_ = 1;
			t->pAggregates_ = new PSUINT32[t->tagCount_];
			t->tagIds_ = new PSUINT32[t->tagCount_];
			REQ_OBJECT_ARG(1, settings);
			t->tagIds_[0] = t->id;
			
			Local<Date> date  =  Local<Date>::Cast(settings->Get(String::New("startTime")));
			V8DATE2PSTIME(&(t->startTime_),date);

			Local<Date> date1  =  Local<Date>::Cast(settings->Get(String::New("endTime")));
			V8DATE2PSTIME(&(t->endTime_),date1);
			int interval=0;
			GET_NUMBER(settings,"resampleInterval",interval);
			PS_TIME tmp = {interval,0};
			t->resampleInterval_ = tmp;

			for (int i=0;i<t->tagCount_;i++)
			{
				std::string agg = "";
				OBJ_GET_STRING(settings, "aggregates",agg);
				t->pAggregates_[i] = (PS_HIS_AGGREGATE_ENUM)t->getAggregate(agg.c_str());
			}
		}else if(args[1]->IsDate()&& args[2]->IsDate())
		{
			t->id = t->getTagID(UTF8ToGBK(replace_all(result[0],"/","\\").c_str()).c_str(),ps->hHanle_);
			t->tagCount_ = 1;
			t->pAggregates_ = new PSUINT32[t->tagCount_];
			t->tagIds_ = new PSUINT32[t->tagCount_];
			t->tagIds_[0] = t->id;
			Local<Date> date  =  Local<Date>::Cast(args[1]);
			V8DATE2PSTIME(&(t->startTime_),date);

			Local<Date> date1  =  Local<Date>::Cast(args[2]);
			V8DATE2PSTIME(&(t->endTime_),date1);
			PS_TIME tmp1 = {args[3]->ToUint32()->Value(),0};
			t->resampleInterval_ = tmp1;
			for (int i=0;i<t->tagCount_;i++)
			{	
				String::Utf8Value t1(args[4]);
				const char * tt1 = ToCString(t1);
				//t->pAggregates_[i] = (PSUINT32)malloc(sizeof(t->pAggregates_));
				t->pAggregates_[i] = (PS_HIS_AGGREGATE_ENUM)t->getAggregate(tt1);
			}
		}
	} catch(PsException &ex) {
		return scope.Close(ThrowException(Exception::Error(String::New(ex.what()))));
	}
	uv_work_t* req = new uv_work_t();
	req->data = t;
	uv_queue_work(uv_default_loop(), req, readHisProcessWork, (uv_after_work_cb)afterReadHisRaw);
	ps->Ref();
	return scope.Close(Undefined());
}
void PspaceNode::readHisProcessWork(uv_work_t* req)
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
			nRet = psAPI_His_ReadProcessed(	t->psNode->hHanle_, t->startTime_, 
											t->endTime_,t->resampleInterval_,
											t->tagCount_, t->tagIds_, t->pAggregates_, 
											&(t->pHisDataList_), &pAPIErrors);
			if (PSERR(nRet) && nRet != PSERR_FAIL_IN_BATCH)
			{
				t->code_ = nRet;
				t->errString = new std::string(psAPI_Commom_GetErrorDesc(nRet));
			}
			for (int n = 0; n < t->tagCount_; n++)
			{
				t->dataStatus_ = new int[t->pHisDataList_->DataCount];

				for (int m=0;m<t->pHisDataList_->DataCount;m++)
				{
					t->dataStatus_[m]=1;
					if (nRet== PSERR_FAIL_IN_BATCH && PSERR(*(pAPIErrors+n)))
					{
						t->dataStatus_[m]=pAPIErrors[n];
					}
				}
			}
			//std::cout<<((t->pHisDataList_+)->DataList+m)->Value.Double<<std::endl;
			if (nRet == PSERR_FAIL_IN_BATCH)
			{
				psAPI_Memory_FreeAndNull((PSVOID**)&pAPIErrors);
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

Handle<Value> PspaceNode::alarmRealSyn(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	Alarm* t;
	try { 
		t = new Alarm(ps,NULL);
		String::Utf8Value str(args[0]);
		const char * pstr = ToCString(str);
		std::vector<std::string> result=split(pstr,".");
		t->id = t->getTagID(UTF8ToGBK(replace_all(result[0],"/","\\").c_str()).c_str(),ps->hHanle_);
		Local<Object> robj =  Object::New();
		if (2==args.Length())
		{
			Handle<Object> obj = args[1]->ToObject();
			Local<v8::Value> filterField= obj->Get(v8::String::New("filterField"));
			Local<Value> filter = obj->Get(String::New("filter"));
			//默认情况查询指定的这个测点的报警
			t->filterField_.TagId = PSTRUE;
			t->filter_.TagId = t->id;
			//filterField和filter为0的情况
			if (filterField->IsInt32()&&0==filterField->ToInt32()->Value())
			{
				PS_ALARM_FILTER_FIELD tmpFilterField = {0};
				PS_ALARM_FILTER tmpFilter = {0};
				t->filterField_ = tmpFilterField;
				t->filter_ = tmpFilter;
			}else//指定了过滤条件的情况
			{
				Handle<Object> filterFieldObj = filterField->ToObject();
				Handle<Object> filterObj = filter->ToObject();

				//获取所有的Key
				Handle<Array> filterFieldKeys = filterFieldObj->GetOwnPropertyNames();
				Handle<Array> filterKeys = filterObj->GetOwnPropertyNames();
				//匹配filterFieldKey并赋值
				for (int i=0;i<filterFieldKeys->Length();i++)
				{
					String::Utf8Value str(filterFieldKeys->Get(i));
					const char * pstr = ToCString(str);
					if (0==stricmp(pstr,"All"))
					{
						Local<Value> all = filterFieldObj->Get(String::New("All"));
						t->filterField_.All = all->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmId"))
					{
						Local<Value> alarmId = filterFieldObj->Get(String::New("AlarmId"));
						t->filterField_.AlarmId = alarmId->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"TagId"))
					{
						Local<Value> tagId = filterFieldObj->Get(String::New("TagId"));
						t->filterField_.TagId = tagId->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"UserID"))
					{
						Local<Value> userId = filterFieldObj->Get(String::New("UserID"));
						t->filterField_.UserID = userId->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmHaveAcked"))
					{
						Local<Value> alarmHaveAcked = filterFieldObj->Get(String::New("AlarmHaveAcked"));
						t->filterField_.AlarmHaveAcked = alarmHaveAcked->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmAckedId"))
					{
						Local<Value> alarmAckedId = filterFieldObj->Get(String::New("AlarmAckedId"));
						t->filterField_.AlarmAckedId = alarmAckedId->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmLowLevel"))
					{
						Local<Value> alarmLowLevel = filterFieldObj->Get(String::New("AlarmLowLevel"));
						t->filterField_.AlarmLowLevel = alarmLowLevel->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmHighLevel"))
					{
						Local<Value> alarmHighLevel = filterFieldObj->Get(String::New("AlarmHighLevel"));
						t->filterField_.AlarmHighLevel = alarmHighLevel->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmContent"))
					{
						Local<Value> alarmContent = filterFieldObj->Get(String::New("AlarmContent"));
						t->filterField_.AlarmContent = alarmContent->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"IsQueryOneLevel"))
					{
						Local<Value> isQueryOneLevel = filterFieldObj->Get(String::New("IsQueryOneLevel"));
						t->filterField_.IsQueryOneLevel = isQueryOneLevel->ToBoolean()->Value();
					}
				}

				//匹配filterKey并赋值
				for (int i=0;i<filterKeys->Length();i++)
				{
					String::Utf8Value str1(filterKeys->Get(i));
					const char * pstr1 = ToCString(str1);
					if (0==stricmp(pstr1,"AlarmId"))
					{
						Local<Value> alarmId = filterObj->Get(String::New("AlarmId"));
						t->filter_.AlarmId = alarmId->ToInt32()->Value();
					}
					if (0==stricmp(pstr1,"TagId"))
					{
						Local<Value> tagId = filterObj->Get(String::New("TagId"));
						t->filter_.TagId = tagId->ToInt32()->Value();
					}
					if (0==stricmp(pstr1,"UserID"))
					{
						Local<Value> userID = filterObj->Get(String::New("UserID"));
						t->filter_.UserID = userID->ToInteger()->Value();
					}
					if (0==stricmp(pstr1,"AlarmHaveAcked"))
					{
						Local<Value> alarmHaveAcked = filterObj->Get(String::New("AlarmHaveAcked"));
						t->filter_.AlarmHaveAcked = alarmHaveAcked->ToBoolean()->Value();
					}
					if (0==stricmp(pstr1,"AlarmAckedId"))
					{
						Local<Value> alarmAckedId = filterObj->Get(String::New("AlarmAckedId"));
						t->filter_.AlarmAckedId = alarmAckedId->ToInteger()->Value();
					}
					if (0==stricmp(pstr1,"AlarmLowLevel"))
					{
						Local<Value> alarmLowLevel = filterObj->Get(String::New("AlarmLowLevel"));
						t->filter_.AlarmLowLevel = alarmLowLevel->ToNumber()->Value();
					}
					if (0==stricmp(pstr1,"AlarmHighLevel"))
					{
						Local<Value> alarmHighLevel = filterObj->Get(String::New("AlarmHighLevel"));
						t->filter_.AlarmHighLevel = alarmHighLevel->ToNumber()->Value();
					}
					if (0==stricmp(pstr1,"AlarmContent"))
					{
						Local<Value> alarmContent = filterObj->Get(String::New("AlarmContent"));
						String::Utf8Value sstr(alarmContent);
						const char * content = ToCString(sstr);
						t->filter_.AlarmContent = (PSSTR)content;
					}
					if (0==stricmp(pstr1,"IsQueryOneLevel"))
					{
						Local<Value> isQueryOneLevel = filterObj->Get(String::New("IsQueryOneLevel"));
						t->filter_.IsQueryOneLevel = isQueryOneLevel->ToBoolean()->Value();
					}	
				}
			}
		}else if(3==args.Length())
		{
			Handle<Object> obj1 = args[1]->ToObject();
			Handle<Object> obj2 = args[2]->ToObject();
			//默认情况查询指定的这个测点的报警
			t->filterField_.TagId = PSTRUE;
			t->filter_.TagId = t->id;
			//filterField和filter为0的情况
			if (obj1->IsInt32() && obj2->IsInt32() && 0==obj1->ToInt32()->Value() && 0==obj2->ToInt32()->Value())
			{
				PS_ALARM_FILTER_FIELD tmpFilterField = {0};
				PS_ALARM_FILTER tmpFilter = {0};
				t->filterField_ = tmpFilterField;
				t->filter_ = tmpFilter;
			}else//指定了过滤条件的情况
			{
				//获取所有的Key
				Handle<Array> filterFieldKeys = obj1->GetOwnPropertyNames();
				Handle<Array> filterKeys = obj2->GetOwnPropertyNames();
				//匹配filterFieldKey并赋值
				for (int i=0;i<filterFieldKeys->Length();i++)
				{
					String::Utf8Value str(filterFieldKeys->Get(i));
					const char * pstr = ToCString(str);
					if (0==stricmp(pstr,"All"))
					{
						Local<Value> all = obj1->Get(String::New("All"));
						t->filterField_.All = all->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmId"))
					{
						Local<Value> alarmId = obj1->Get(String::New("AlarmId"));
						t->filterField_.AlarmId = alarmId->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"TagId"))
					{
						Local<Value> tagId = obj1->Get(String::New("TagId"));
						t->filterField_.TagId = tagId->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"UserID"))
					{
						Local<Value> userId = obj1->Get(String::New("UserID"));
						t->filterField_.UserID = userId->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmHaveAcked"))
					{
						Local<Value> alarmHaveAcked = obj1->Get(String::New("AlarmHaveAcked"));
						t->filterField_.AlarmHaveAcked = alarmHaveAcked->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmAckedId"))
					{
						Local<Value> alarmAckedId = obj1->Get(String::New("AlarmAckedId"));
						t->filterField_.AlarmAckedId = alarmAckedId->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmLowLevel"))
					{
						Local<Value> alarmLowLevel = obj1->Get(String::New("AlarmLowLevel"));
						t->filterField_.AlarmLowLevel = alarmLowLevel->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmHighLevel"))
					{
						Local<Value> alarmHighLevel = obj1->Get(String::New("AlarmHighLevel"));
						t->filterField_.AlarmHighLevel = alarmHighLevel->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmContent"))
					{
						Local<Value> alarmContent = obj1->Get(String::New("AlarmContent"));
						t->filterField_.AlarmContent = alarmContent->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"IsQueryOneLevel"))
					{
						Local<Value> isQueryOneLevel = obj1->Get(String::New("IsQueryOneLevel"));
						t->filterField_.IsQueryOneLevel = isQueryOneLevel->ToBoolean()->Value();
					}
				}

				//匹配filterKey并赋值
				for (int i=0;i<filterKeys->Length();i++)
				{
					String::Utf8Value str1(filterKeys->Get(i));
					const char * pstr1 = ToCString(str1);
					if (0==stricmp(pstr1,"AlarmId"))
					{
						Local<Value> alarmId = obj2->Get(String::New("AlarmId"));
						t->filter_.AlarmId = alarmId->ToInt32()->Value();
					}
					if (0==stricmp(pstr1,"TagId"))
					{
						Local<Value> tagId = obj2->Get(String::New("TagId"));
						t->filter_.TagId = tagId->ToInt32()->Value();
					}
					if (0==stricmp(pstr1,"UserID"))
					{
						Local<Value> userID = obj2->Get(String::New("UserID"));
						t->filter_.UserID = userID->ToInteger()->Value();
					}
					if (0==stricmp(pstr1,"AlarmHaveAcked"))
					{
						Local<Value> alarmHaveAcked = obj2->Get(String::New("AlarmHaveAcked"));
						t->filter_.AlarmHaveAcked = alarmHaveAcked->ToBoolean()->Value();
					}
					if (0==stricmp(pstr1,"AlarmAckedId"))
					{
						Local<Value> alarmAckedId = obj2->Get(String::New("AlarmAckedId"));
						t->filter_.AlarmAckedId = alarmAckedId->ToInteger()->Value();
					}
					if (0==stricmp(pstr1,"AlarmLowLevel"))
					{
						Local<Value> alarmLowLevel = obj2->Get(String::New("AlarmLowLevel"));
						t->filter_.AlarmLowLevel = alarmLowLevel->ToNumber()->Value();
					}
					if (0==stricmp(pstr1,"AlarmHighLevel"))
					{
						Local<Value> alarmHighLevel = obj2->Get(String::New("AlarmHighLevel"));
						t->filter_.AlarmHighLevel = alarmHighLevel->ToNumber()->Value();
					}
					if (0==stricmp(pstr1,"AlarmContent"))
					{
						Local<Value> alarmContent = obj2->Get(String::New("AlarmContent"));
						String::Utf8Value sstr(alarmContent);
						const char * content = ToCString(sstr);
						t->filter_.AlarmContent = (PSSTR)content;
					}
					if (0==stricmp(pstr1,"IsQueryOneLevel"))
					{
						Local<Value> isQueryOneLevel = obj2->Get(String::New("IsQueryOneLevel"));
						t->filter_.IsQueryOneLevel = isQueryOneLevel->ToBoolean()->Value();
					}	
				}
			}
		}else{
			t->code_ = -1;
			t->errString = new std::string("参数错误!");
			Local<Object> errObj = Error::newObj();
			errObj->Set(String::New("code"),Number::New(-1));
			errObj->Set(String::New("errString"),String::New(GBK2UTF8("参数错误!").c_str()));
			return errObj;
		}
		if (t->id==PSTAGID_UNUSED)
		{
			t->code_ = -1;
			t->errString = new std::string("tag not found!");
			Local<Object> errObj = Error::newObj();
			errObj->Set(String::New("code"),Number::New(-1));
			errObj->Set(String::New("errString"),String::New("tag not found!"));
			return errObj;
		}
		uv_work_t* req = new uv_work_t();
		GuardPtr<uv_work_t> reqgd(&req);
		req->data = t;
		t->psNode->Ref();
		alarmWork(req);
		t->psNode->Unref();
		//如果失败
		if(t->errString) {
			Local<Object> errObj = Error::newObj();
			errObj->Set(String::New("code"),Number::New(t->code_));
			std:string *s =t->errString;
			errObj->Set(String::New("errString"),String::New(GBK2UTF8(s->c_str()).c_str()));
			delete t;
			return errObj;
		}
		//成功返回数据
		Handle<Array> arrObj = Array::New(t->alarmCount_);
		for (int n=0;n<t->alarmCount_;n++)
		{
			Local<Object> obj = Object::New();
			obj->Set(String::New("AlarmId"),Int32::New(t->alarms_[n].AlarmId));
			obj->Set(String::New("UserID"),Int32::New(t->alarms_[n].UserID));
			obj->Set(String::New("TagId"),Int32::New(t->alarms_[n].TagId));
			obj->Set(String::New("AppType"),Int32::New(t->alarms_[n].AppType));
			obj->Set(String::New("AlarmLevel "),Int32::New(t->alarms_[n].AlarmLevel));
			obj->Set(String::New("AlarmContent"),String::New(GBK2UTF8(t->alarms_[n].AlarmContent).c_str()));
			obj->Set(String::New("AlarmValue"),String::New(t->alarms_[n].AlarmValue));
			obj->Set(String::New("AlarmAckUserName"),String::New(t->alarms_[n].AlarmAckUserName));
			obj->Set(String::New("AlarmTopic"),Int32::New(t->alarms_[n].AlarmTopic));
			obj->Set(String::New("AlarmNeedAck"),Boolean::New(t->alarms_[n].AlarmNeedAck));
			obj->Set(String::New("AlarmHaveAcked"),Boolean::New(t->alarms_[n].AlarmHaveAcked));
			obj->Set(String::New("AlarmStartTime"),PSTIME2V8DATE(t->alarms_[n].AlarmStartTime));
			obj->Set(String::New("AlarmEndTime"),PSTIME2V8DATE(t->alarms_[n].AlarmEndTime));
			obj->Set(String::New("AlarmAckedId"),Int32::New(t->alarms_[n].AlarmAckedId));
			obj->Set(String::New("AlarmAckTime"),PSTIME2V8DATE(t->alarms_[n].AlarmAckTime));
			arrObj->Set(n,obj);
		}
		delete t;
		return arrObj;
	} catch(PsException &ex) {
		return ThrowException(Exception::Error(String::New(ex.what())));
	}	
}
Handle<Value> PspaceNode::alarmRealAsy(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	Alarm* t;
	try {
		Handle<Object> robj = Object::New();
		String::Utf8Value str(args[0]);
		const char * pstr = ToCString(str);
		std::vector<std::string> result=split(pstr,".");
		REQ_FUN_ARG(args.Length()-1, callback);
		t = new Alarm(ps,&callback);
		if (3==args.Length())
		{
			t->id = t->getTagID(UTF8ToGBK(replace_all(result[0],"/","\\").c_str()).c_str(),ps->hHanle_);
			Handle<Object> obj = args[1]->ToObject();
			Local<v8::Value> filterField= obj->Get(v8::String::New("filterField"));
			Local<Value> filter = obj->Get(String::New("filter"));
			//默认情况查询指定的这个测点的报警
			t->filterField_.TagId = PSTRUE;
			t->filter_.TagId = t->id;
			//filterField和filter为0的情况
			if (filterField->IsInt32() && 0==filterField->ToInt32()->Value())
			{
				
				PS_ALARM_FILTER_FIELD tmpFilterField = {0};
				PS_ALARM_FILTER tmpFilter = {0};
				t->filterField_ = tmpFilterField;
				t->filter_ = tmpFilter;
			}else//指定了过滤条件的情况
			{
				Handle<Object> filterFieldObj = filterField->ToObject();
				Handle<Object> filterObj = filter->ToObject();

				//获取所有的Key
				Handle<Array> filterFieldKeys = filterFieldObj->GetOwnPropertyNames();
				Handle<Array> filterKeys = filterObj->GetOwnPropertyNames();
				//匹配filterFieldKey并赋值
				for (int i=0;i<filterFieldKeys->Length();i++)
				{
					String::Utf8Value str(filterFieldKeys->Get(i));
					const char * pstr = ToCString(str);
					if (0==stricmp(pstr,"All"))
					{
						Local<Value> all = filterFieldObj->Get(String::New("All"));
						t->filterField_.All = all->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmId"))
					{
						Local<Value> alarmId = filterFieldObj->Get(String::New("AlarmId"));
						t->filterField_.AlarmId = alarmId->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"TagId"))
					{
						Local<Value> tagId = filterFieldObj->Get(String::New("TagId"));
						t->filterField_.TagId = tagId->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"UserID"))
					{
						Local<Value> userId = filterFieldObj->Get(String::New("UserID"));
						t->filterField_.UserID = userId->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmHaveAcked"))
					{
						Local<Value> alarmHaveAcked = filterFieldObj->Get(String::New("AlarmHaveAcked"));
						t->filterField_.AlarmHaveAcked = alarmHaveAcked->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmAckedId"))
					{
						Local<Value> alarmAckedId = filterFieldObj->Get(String::New("AlarmAckedId"));
						t->filterField_.AlarmAckedId = alarmAckedId->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmLowLevel"))
					{
						Local<Value> alarmLowLevel = filterFieldObj->Get(String::New("AlarmLowLevel"));
						t->filterField_.AlarmLowLevel = alarmLowLevel->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmHighLevel"))
					{
						Local<Value> alarmHighLevel = filterFieldObj->Get(String::New("AlarmHighLevel"));
						t->filterField_.AlarmHighLevel = alarmHighLevel->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmContent"))
					{
						Local<Value> alarmContent = filterFieldObj->Get(String::New("AlarmContent"));
						t->filterField_.AlarmContent = alarmContent->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"IsQueryOneLevel"))
					{
						Local<Value> isQueryOneLevel = filterFieldObj->Get(String::New("IsQueryOneLevel"));
						t->filterField_.IsQueryOneLevel = isQueryOneLevel->ToBoolean()->Value();
					}
				}

				//匹配filterKey并赋值
				for (int i=0;i<filterKeys->Length();i++)
				{
					String::Utf8Value str1(filterKeys->Get(i));
					const char * pstr1 = ToCString(str1);
					if (0==stricmp(pstr1,"AlarmId"))
					{
						Local<Value> alarmId = filterObj->Get(String::New("AlarmId"));
						t->filter_.AlarmId = alarmId->ToInt32()->Value();
					}
					if (0==stricmp(pstr1,"TagId"))
					{
						Local<Value> tagId = filterObj->Get(String::New("TagId"));
						t->filter_.TagId = tagId->ToInt32()->Value();
					}
					if (0==stricmp(pstr1,"UserID"))
					{
						Local<Value> userID = filterObj->Get(String::New("UserID"));
						t->filter_.UserID = userID->ToInteger()->Value();
					}
					if (0==stricmp(pstr1,"AlarmHaveAcked"))
					{
						Local<Value> alarmHaveAcked = filterObj->Get(String::New("AlarmHaveAcked"));
						t->filter_.AlarmHaveAcked = alarmHaveAcked->ToBoolean()->Value();
					}
					if (0==stricmp(pstr1,"AlarmAckedId"))
					{
						Local<Value> alarmAckedId = filterObj->Get(String::New("AlarmAckedId"));
						t->filter_.AlarmAckedId = alarmAckedId->ToInteger()->Value();
					}
					if (0==stricmp(pstr1,"AlarmLowLevel"))
					{
						Local<Value> alarmLowLevel = filterObj->Get(String::New("AlarmLowLevel"));
						t->filter_.AlarmLowLevel = alarmLowLevel->ToNumber()->Value();
					}
					if (0==stricmp(pstr1,"AlarmHighLevel"))
					{
						Local<Value> alarmHighLevel = filterObj->Get(String::New("AlarmHighLevel"));
						t->filter_.AlarmHighLevel = alarmHighLevel->ToNumber()->Value();
					}
					if (0==stricmp(pstr1,"AlarmContent"))
					{
						Local<Value> alarmContent = filterObj->Get(String::New("AlarmContent"));
						String::Utf8Value sstr(alarmContent);
						const char * content = ToCString(sstr);
						t->filter_.AlarmContent = (PSSTR)content;
					}
					if (0==stricmp(pstr1,"IsQueryOneLevel"))
					{
						Local<Value> isQueryOneLevel = filterObj->Get(String::New("IsQueryOneLevel"));
						t->filter_.IsQueryOneLevel = isQueryOneLevel->ToBoolean()->Value();
					}	
				}
			}
		}else if(4==args.Length())
		{
			/*REQ_FUN_ARG(3, callback);
			t = new Alarm(ps,&callback);*/
			t->id = t->getTagID(UTF8ToGBK(replace_all(result[0],"/","\\").c_str()).c_str(),ps->hHanle_);
			Handle<Object> obj1 = args[1]->ToObject();
			Handle<Object> obj2 = args[2]->ToObject();
			//默认情况查询指定的这个测点的报警
			t->filterField_.TagId = PSTRUE;
			t->filter_.TagId = t->id;
			//filterField和filter为0的情况
			if (obj1->IsInt32() && obj2->IsInt32() && 0==obj1->ToInt32()->Value() && 0==obj2->ToInt32()->Value())
			{
				PS_ALARM_FILTER_FIELD tmpFilterField = {0};
				PS_ALARM_FILTER tmpFilter = {0};
				t->filterField_ = tmpFilterField;
				t->filter_ = tmpFilter;
			}else//指定了过滤条件的情况
			{
				//获取所有的Key
				Handle<Array> filterFieldKeys = obj1->GetOwnPropertyNames();
				Handle<Array> filterKeys = obj2->GetOwnPropertyNames();
				//匹配filterFieldKey并赋值
				for (int i=0;i<filterFieldKeys->Length();i++)
				{
					String::Utf8Value str(filterFieldKeys->Get(i));
					const char * pstr = ToCString(str);
					if (0==stricmp(pstr,"All"))
					{
						Local<Value> all = obj1->Get(String::New("All"));
						t->filterField_.All = all->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmId"))
					{
						Local<Value> alarmId = obj1->Get(String::New("AlarmId"));
						t->filterField_.AlarmId = alarmId->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"TagId"))
					{
						Local<Value> tagId = obj1->Get(String::New("TagId"));
						t->filterField_.TagId = tagId->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"UserID"))
					{
						Local<Value> userId = obj1->Get(String::New("UserID"));
						t->filterField_.UserID = userId->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmHaveAcked"))
					{
						Local<Value> alarmHaveAcked = obj1->Get(String::New("AlarmHaveAcked"));
						t->filterField_.AlarmHaveAcked = alarmHaveAcked->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmAckedId"))
					{
						Local<Value> alarmAckedId = obj1->Get(String::New("AlarmAckedId"));
						t->filterField_.AlarmAckedId = alarmAckedId->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmLowLevel"))
					{
						Local<Value> alarmLowLevel = obj1->Get(String::New("AlarmLowLevel"));
						t->filterField_.AlarmLowLevel = alarmLowLevel->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmHighLevel"))
					{
						Local<Value> alarmHighLevel = obj1->Get(String::New("AlarmHighLevel"));
						t->filterField_.AlarmHighLevel = alarmHighLevel->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmContent"))
					{
						Local<Value> alarmContent = obj1->Get(String::New("AlarmContent"));
						t->filterField_.AlarmContent = alarmContent->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"IsQueryOneLevel"))
					{
						Local<Value> isQueryOneLevel = obj1->Get(String::New("IsQueryOneLevel"));
						t->filterField_.IsQueryOneLevel = isQueryOneLevel->ToBoolean()->Value();
					}
				}

				//匹配filterKey并赋值
				for (int i=0;i<filterKeys->Length();i++)
				{
					String::Utf8Value str1(filterKeys->Get(i));
					const char * pstr1 = ToCString(str1);
					if (0==stricmp(pstr1,"AlarmId"))
					{
						Local<Value> alarmId = obj2->Get(String::New("AlarmId"));
						t->filter_.AlarmId = alarmId->ToInt32()->Value();
					}
					if (0==stricmp(pstr1,"TagId"))
					{
						Local<Value> tagId = obj2->Get(String::New("TagId"));
						t->filter_.TagId = tagId->ToInt32()->Value();
					}
					if (0==stricmp(pstr1,"UserID"))
					{
						Local<Value> userID = obj2->Get(String::New("UserID"));
						t->filter_.UserID = userID->ToInteger()->Value();
					}
					if (0==stricmp(pstr1,"AlarmHaveAcked"))
					{
						Local<Value> alarmHaveAcked = obj2->Get(String::New("AlarmHaveAcked"));
						t->filter_.AlarmHaveAcked = alarmHaveAcked->ToBoolean()->Value();
					}
					if (0==stricmp(pstr1,"AlarmAckedId"))
					{
						Local<Value> alarmAckedId = obj2->Get(String::New("AlarmAckedId"));
						t->filter_.AlarmAckedId = alarmAckedId->ToInteger()->Value();
					}
					if (0==stricmp(pstr1,"AlarmLowLevel"))
					{
						Local<Value> alarmLowLevel = obj2->Get(String::New("AlarmLowLevel"));
						t->filter_.AlarmLowLevel = alarmLowLevel->ToNumber()->Value();
					}
					if (0==stricmp(pstr1,"AlarmHighLevel"))
					{
						Local<Value> alarmHighLevel = obj2->Get(String::New("AlarmHighLevel"));
						t->filter_.AlarmHighLevel = alarmHighLevel->ToNumber()->Value();
					}
					if (0==stricmp(pstr1,"AlarmContent"))
					{
						Local<Value> alarmContent = obj2->Get(String::New("AlarmContent"));
						String::Utf8Value sstr(alarmContent);
						const char * content = ToCString(sstr);
						t->filter_.AlarmContent = (PSSTR)content;
					}
					if (0==stricmp(pstr1,"IsQueryOneLevel"))
					{
						Local<Value> isQueryOneLevel = obj2->Get(String::New("IsQueryOneLevel"));
						t->filter_.IsQueryOneLevel = isQueryOneLevel->ToBoolean()->Value();
					}	
				}
			}
		}
	} catch(PsException &ex) {
		return scope.Close(ThrowException(Exception::Error(String::New(ex.what()))));
	}
	uv_work_t* req = new uv_work_t();
	req->data = t;
	uv_queue_work(uv_default_loop(), req, alarmWork, (uv_after_work_cb)afterAlarmReal);
	ps->Ref();
	return scope.Close(Undefined());
}
void PspaceNode::alarmWork(uv_work_t* req)
{
	Alarm* t = static_cast<Alarm*>(req->data);
	t->code_  = PSRET_OK;
	t->errString = NULL;
	PSAPIStatus nRet  = PSRET_OK;
	try {
		if(t->psNode->hHanle_ == PSHANDLE_UNUSED) {
			throw PsException("Connection already closed");
		}else{
			nRet = psAPI_Alarm_Real_Query(t->psNode->hHanle_, &(t->filterField_), &(t->filter_), &(t->alarmCount_), &(t->alarms_));
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
void PspaceNode::afterAlarmReal(uv_work_t* req,int status)
{
	HandleScope scope;
	Alarm* t = static_cast<Alarm*>(req->data);
	t->psNode->Unref();
	try {
		if(t->errString){
			Handle<Value> argv[2];
			argv[0] = Exception::Error(String::New(GBK2UTF8(t->errString->c_str()).c_str()));
			argv[1] = Undefined();
			node::MakeCallback(Context::GetCurrent()->Global(), t->callback,2, argv);
		}else{
			Handle<Value> argv[2];
			//结果设置
			argv[0] = Undefined();
			//成功返回数据
			Handle<Array> arrObj = Array::New(t->alarmCount_);
			for (int n=0;n<t->alarmCount_;n++)
			{
				Local<Object> obj = Object::New();
				obj->Set(String::New("AlarmId"),Int32::New(t->alarms_[n].AlarmId));
				obj->Set(String::New("UserID"),Int32::New(t->alarms_[n].UserID));
				obj->Set(String::New("TagId"),Int32::New(t->alarms_[n].TagId));
				obj->Set(String::New("AppType"),Int32::New(t->alarms_[n].AppType));
				obj->Set(String::New("AlarmLevel "),Int32::New(t->alarms_[n].AlarmLevel));
				obj->Set(String::New("AlarmContent"),String::New(GBK2UTF8(t->alarms_[n].AlarmContent).c_str()));
				obj->Set(String::New("AlarmValue"),String::New(t->alarms_[n].AlarmValue));
				obj->Set(String::New("AlarmAckUserName"),String::New(t->alarms_[n].AlarmAckUserName));
				obj->Set(String::New("AlarmTopic"),Int32::New(t->alarms_[n].AlarmTopic));
				obj->Set(String::New("AlarmNeedAck"),Boolean::New(t->alarms_[n].AlarmNeedAck));
				obj->Set(String::New("AlarmHaveAcked"),Boolean::New(t->alarms_[n].AlarmHaveAcked));
				obj->Set(String::New("AlarmStartTime"),PSTIME2V8DATE(t->alarms_[n].AlarmStartTime));
				obj->Set(String::New("AlarmEndTime"),PSTIME2V8DATE(t->alarms_[n].AlarmEndTime));
				obj->Set(String::New("AlarmAckedId"),Int32::New(t->alarms_[n].AlarmAckedId));
				obj->Set(String::New("AlarmAckTime"),PSTIME2V8DATE(t->alarms_[n].AlarmAckTime));
				arrObj->Set(n,obj);
			}
			argv[1] = arrObj;
			node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 2, argv);
		}
	} catch(PsException &ex) {
		Handle<Value> argv[2];
		argv[0] = Exception::Error(String::New(ex.what()));
		argv[1] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 2, argv);
	} catch(const exception &ex) {
		Handle<Value> argv[2];
		argv[0] = Exception::Error(String::New(t->errString->c_str()));
		argv[1] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 2, argv);
	}
	delete t;
	scope.Close(Undefined());
}

Handle<Value> PspaceNode::hisAlarmSyn(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	Alarm* t;
	try { 
		t = new Alarm(ps,NULL);
		String::Utf8Value str(args[0]);
		const char * pstr = ToCString(str);
		std::vector<std::string> result=split(pstr,".");
		t->id = t->getTagID(UTF8ToGBK(replace_all(result[0],"/","\\").c_str()).c_str(),ps->hHanle_);
		Local<Object> robj =  Object::New();
		if (2==args.Length())
		{
			Handle<Object> obj = args[1]->ToObject();
			Local<v8::Value> filterField= obj->Get(v8::String::New("filterField"));
			Local<Value> filter = obj->Get(String::New("filter"));

			String::Utf8Value startStr(obj->Get(String::New("startTime")));
			const char* startTime = ToCString(startStr);
			PS_TIME *s = new PS_TIME[sizeof(PS_TIME)];
			STR2PSTIME(s,startTime);
			t->startTime_= *s;

			String::Utf8Value endStr(obj->Get(String::New("endTime")));
			const char* endTime = ToCString(endStr);
			PS_TIME *s1 = new PS_TIME[sizeof(PS_TIME)];
			STR2PSTIME(s1,endTime);
			t->endTime_= *s1;

			//默认情况查询指定的这个测点的报警
			t->filterField_.TagId = PSTRUE;
			t->filter_.TagId = t->id;
			//filterField和filter为0的情况
			if (filterField->IsInt32() && 0==filterField->ToInt32()->Value())
			{
				PS_ALARM_FILTER_FIELD tmpFilterField = {0};
				PS_ALARM_FILTER tmpFilter = {0};
				t->filterField_ = tmpFilterField;
				t->filter_ = tmpFilter;
			}else//指定了过滤条件的情况
			{
				Handle<Object> filterFieldObj = filterField->ToObject();
				Handle<Object> filterObj = filter->ToObject();

				//获取所有的Key
				Handle<Array> filterFieldKeys = filterFieldObj->GetOwnPropertyNames();
				Handle<Array> filterKeys = filterObj->GetOwnPropertyNames();
				//匹配filterFieldKey并赋值
				for (int i=0;i<filterFieldKeys->Length();i++)
				{
					String::Utf8Value str(filterFieldKeys->Get(i));
					const char * pstr = ToCString(str);
					if (0==stricmp(pstr,"All"))
					{
						Local<Value> all = filterFieldObj->Get(String::New("All"));
						t->filterField_.All = all->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmId"))
					{
						Local<Value> alarmId = filterFieldObj->Get(String::New("AlarmId"));
						t->filterField_.AlarmId = alarmId->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"TagId"))
					{
						Local<Value> tagId = filterFieldObj->Get(String::New("TagId"));
						t->filterField_.TagId = tagId->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"UserID"))
					{
						Local<Value> userId = filterFieldObj->Get(String::New("UserID"));
						t->filterField_.UserID = userId->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmHaveAcked"))
					{
						Local<Value> alarmHaveAcked = filterFieldObj->Get(String::New("AlarmHaveAcked"));
						t->filterField_.AlarmHaveAcked = alarmHaveAcked->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmAckedId"))
					{
						Local<Value> alarmAckedId = filterFieldObj->Get(String::New("AlarmAckedId"));
						t->filterField_.AlarmAckedId = alarmAckedId->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmLowLevel"))
					{
						Local<Value> alarmLowLevel = filterFieldObj->Get(String::New("AlarmLowLevel"));
						t->filterField_.AlarmLowLevel = alarmLowLevel->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmHighLevel"))
					{
						Local<Value> alarmHighLevel = filterFieldObj->Get(String::New("AlarmHighLevel"));
						t->filterField_.AlarmHighLevel = alarmHighLevel->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmContent"))
					{
						Local<Value> alarmContent = filterFieldObj->Get(String::New("AlarmContent"));
						t->filterField_.AlarmContent = alarmContent->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"IsQueryOneLevel"))
					{
						Local<Value> isQueryOneLevel = filterFieldObj->Get(String::New("IsQueryOneLevel"));
						t->filterField_.IsQueryOneLevel = isQueryOneLevel->ToBoolean()->Value();
					}
				}

				//匹配filterKey并赋值
				for (int i=0;i<filterKeys->Length();i++)
				{
					String::Utf8Value str1(filterKeys->Get(i));
					const char * pstr1 = ToCString(str1);
					if (0==stricmp(pstr1,"AlarmId"))
					{
						Local<Value> alarmId = filterObj->Get(String::New("AlarmId"));
						t->filter_.AlarmId = alarmId->ToInt32()->Value();
					}
					if (0==stricmp(pstr1,"TagId"))
					{
						Local<Value> tagId = filterObj->Get(String::New("TagId"));
						t->filter_.TagId = tagId->ToInt32()->Value();
					}
					if (0==stricmp(pstr1,"UserID"))
					{
						Local<Value> userID = filterObj->Get(String::New("UserID"));
						t->filter_.UserID = userID->ToInteger()->Value();
					}
					if (0==stricmp(pstr1,"AlarmHaveAcked"))
					{
						Local<Value> alarmHaveAcked = filterObj->Get(String::New("AlarmHaveAcked"));
						t->filter_.AlarmHaveAcked = alarmHaveAcked->ToBoolean()->Value();
					}
					if (0==stricmp(pstr1,"AlarmAckedId"))
					{
						Local<Value> alarmAckedId = filterObj->Get(String::New("AlarmAckedId"));
						t->filter_.AlarmAckedId = alarmAckedId->ToInteger()->Value();
					}
					if (0==stricmp(pstr1,"AlarmLowLevel"))
					{
						Local<Value> alarmLowLevel = filterObj->Get(String::New("AlarmLowLevel"));
						t->filter_.AlarmLowLevel = alarmLowLevel->ToNumber()->Value();
					}
					if (0==stricmp(pstr1,"AlarmHighLevel"))
					{
						Local<Value> alarmHighLevel = filterObj->Get(String::New("AlarmHighLevel"));
						t->filter_.AlarmHighLevel = alarmHighLevel->ToNumber()->Value();
					}
					if (0==stricmp(pstr1,"AlarmContent"))
					{
						Local<Value> alarmContent = filterObj->Get(String::New("AlarmContent"));
						String::Utf8Value sstr(alarmContent);
						const char * content = ToCString(sstr);
						t->filter_.AlarmContent = (PSSTR)content;
					}
					if (0==stricmp(pstr1,"IsQueryOneLevel"))
					{
						Local<Value> isQueryOneLevel = filterObj->Get(String::New("IsQueryOneLevel"));
						t->filter_.IsQueryOneLevel = isQueryOneLevel->ToBoolean()->Value();
					}	
				}
			}
		}else if(5==args.Length())
		{
			Handle<Object> obj1 = args[1]->ToObject();
			Handle<Object> obj2 = args[2]->ToObject();

			Local<Date> date  =  Local<Date>::Cast(args[3]);
			V8DATE2PSTIME(&(t->startTime_),date);
		
			Local<Date> date1  =  Local<Date>::Cast(args[4]);
			V8DATE2PSTIME(&(t->endTime_),date1);

			//默认情况查询指定的这个测点的报警
			t->filterField_.TagId = PSTRUE;
			t->filter_.TagId = t->id;
			//filterField和filter为0的情况
			if (obj1->IsInt32() && obj2->IsInt32() && 0==obj1->ToInt32()->Value() && 0==obj2->ToInt32()->Value())
			{
				PS_ALARM_FILTER_FIELD tmpFilterField = {0};
				PS_ALARM_FILTER tmpFilter = {0};
				t->filterField_ = tmpFilterField;
				t->filter_ = tmpFilter;
			}else//指定了过滤条件的情况
			{
				//获取所有的Key
				Handle<Array> filterFieldKeys = obj1->GetOwnPropertyNames();
				Handle<Array> filterKeys = obj2->GetOwnPropertyNames();
				//匹配filterFieldKey并赋值
				for (int i=0;i<filterFieldKeys->Length();i++)
				{
					String::Utf8Value str(filterFieldKeys->Get(i));
					const char * pstr = ToCString(str);
					if (0==stricmp(pstr,"All"))
					{
						Local<Value> all = obj1->Get(String::New("All"));
						t->filterField_.All = all->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmId"))
					{
						Local<Value> alarmId = obj1->Get(String::New("AlarmId"));
						t->filterField_.AlarmId = alarmId->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"TagId"))
					{
						Local<Value> tagId = obj1->Get(String::New("TagId"));
						t->filterField_.TagId = tagId->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"UserID"))
					{
						Local<Value> userId = obj1->Get(String::New("UserID"));
						t->filterField_.UserID = userId->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmHaveAcked"))
					{
						Local<Value> alarmHaveAcked = obj1->Get(String::New("AlarmHaveAcked"));
						t->filterField_.AlarmHaveAcked = alarmHaveAcked->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmAckedId"))
					{
						Local<Value> alarmAckedId = obj1->Get(String::New("AlarmAckedId"));
						t->filterField_.AlarmAckedId = alarmAckedId->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmLowLevel"))
					{
						Local<Value> alarmLowLevel = obj1->Get(String::New("AlarmLowLevel"));
						t->filterField_.AlarmLowLevel = alarmLowLevel->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmHighLevel"))
					{
						Local<Value> alarmHighLevel = obj1->Get(String::New("AlarmHighLevel"));
						t->filterField_.AlarmHighLevel = alarmHighLevel->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmContent"))
					{
						Local<Value> alarmContent = obj1->Get(String::New("AlarmContent"));
						t->filterField_.AlarmContent = alarmContent->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"IsQueryOneLevel"))
					{
						Local<Value> isQueryOneLevel = obj1->Get(String::New("IsQueryOneLevel"));
						t->filterField_.IsQueryOneLevel = isQueryOneLevel->ToBoolean()->Value();
					}
				}

				//匹配filterKey并赋值
				for (int i=0;i<filterKeys->Length();i++)
				{
					String::Utf8Value str1(filterKeys->Get(i));
					const char * pstr1 = ToCString(str1);
					if (0==stricmp(pstr1,"AlarmId"))
					{
						Local<Value> alarmId = obj2->Get(String::New("AlarmId"));
						t->filter_.AlarmId = alarmId->ToInt32()->Value();
					}
					if (0==stricmp(pstr1,"TagId"))
					{
						Local<Value> tagId = obj2->Get(String::New("TagId"));
						t->filter_.TagId = tagId->ToInt32()->Value();
					}
					if (0==stricmp(pstr1,"UserID"))
					{
						Local<Value> userID = obj2->Get(String::New("UserID"));
						t->filter_.UserID = userID->ToInteger()->Value();
					}
					if (0==stricmp(pstr1,"AlarmHaveAcked"))
					{
						Local<Value> alarmHaveAcked = obj2->Get(String::New("AlarmHaveAcked"));
						t->filter_.AlarmHaveAcked = alarmHaveAcked->ToBoolean()->Value();
					}
					if (0==stricmp(pstr1,"AlarmAckedId"))
					{
						Local<Value> alarmAckedId = obj2->Get(String::New("AlarmAckedId"));
						t->filter_.AlarmAckedId = alarmAckedId->ToInteger()->Value();
					}
					if (0==stricmp(pstr1,"AlarmLowLevel"))
					{
						Local<Value> alarmLowLevel = obj2->Get(String::New("AlarmLowLevel"));
						t->filter_.AlarmLowLevel = alarmLowLevel->ToNumber()->Value();
					}
					if (0==stricmp(pstr1,"AlarmHighLevel"))
					{
						Local<Value> alarmHighLevel = obj2->Get(String::New("AlarmHighLevel"));
						t->filter_.AlarmHighLevel = alarmHighLevel->ToNumber()->Value();
					}
					if (0==stricmp(pstr1,"AlarmContent"))
					{
						Local<Value> alarmContent = obj2->Get(String::New("AlarmContent"));
						String::Utf8Value sstr(alarmContent);
						const char * content = ToCString(sstr);
						t->filter_.AlarmContent = (PSSTR)content;
					}
					if (0==stricmp(pstr1,"IsQueryOneLevel"))
					{
						Local<Value> isQueryOneLevel = obj2->Get(String::New("IsQueryOneLevel"));
						t->filter_.IsQueryOneLevel = isQueryOneLevel->ToBoolean()->Value();
					}	
				}
			}
		}else{
			t->code_ = -1;
			t->errString = new std::string("参数错误!");
			robj->Set(String::New("code"),Number::New(-1));
			robj->Set(String::New("errString"),String::New(GBK2UTF8("参数错误!").c_str()));
			return robj;
		}
		if (t->id==PSTAGID_UNUSED)
		{
			t->code_ = -1;
			t->errString = new std::string("tag not found!");
			Local<Object> errObj = Error::newObj();
			errObj->Set(String::New("code"),Number::New(-1));
			errObj->Set(String::New("errString"),String::New("tag not found!"));
			return errObj;
		}
		uv_work_t* req = new uv_work_t();
		req->data = t;
		t->psNode->Ref();
		hisAlarmWork(req);
		t->psNode->Unref();
		//如果失败
		if(t->errString) {
			Local<Object> errObj = Error::newObj();
			errObj->Set(String::New("code"),Number::New(t->code_));
			std:string *s =t->errString;
			errObj->Set(String::New("errString"),String::New(GBK2UTF8(s->c_str()).c_str()));
			delete t;
			return errObj;
		}
		//成功返回数据
		Handle<Array> arrObj = Array::New(t->alarmCount_);
		for (int n=0;n<t->alarmCount_;n++)
		{
			Local<Object> obj = Object::New();
			obj->Set(String::New("AlarmId"),Int32::New(t->alarms_[n].AlarmId));
			obj->Set(String::New("UserID"),Int32::New(t->alarms_[n].UserID));
			obj->Set(String::New("TagId"),Int32::New(t->alarms_[n].TagId));
			obj->Set(String::New("AppType"),Int32::New(t->alarms_[n].AppType));
			obj->Set(String::New("AlarmLevel "),Int32::New(t->alarms_[n].AlarmLevel));
			obj->Set(String::New("AlarmContent"),String::New(GBK2UTF8(t->alarms_[n].AlarmContent).c_str()));
			obj->Set(String::New("AlarmValue"),String::New(t->alarms_[n].AlarmValue));
			obj->Set(String::New("AlarmAckUserName"),String::New(t->alarms_[n].AlarmAckUserName));
			obj->Set(String::New("AlarmTopic"),Int32::New(t->alarms_[n].AlarmTopic));
			obj->Set(String::New("AlarmNeedAck"),Boolean::New(t->alarms_[n].AlarmNeedAck));
			obj->Set(String::New("AlarmHaveAcked"),Boolean::New(t->alarms_[n].AlarmHaveAcked));
			obj->Set(String::New("AlarmStartTime"),PSTIME2V8DATE(t->alarms_[n].AlarmStartTime));
			obj->Set(String::New("AlarmEndTime"),PSTIME2V8DATE(t->alarms_[n].AlarmEndTime));
			obj->Set(String::New("AlarmAckedId"),Int32::New(t->alarms_[n].AlarmAckedId));
			obj->Set(String::New("AlarmAckTime"),PSTIME2V8DATE(t->alarms_[n].AlarmAckTime));
			arrObj->Set(n,obj);
		}
		delete t;
		return arrObj;
	} catch(PsException &ex) {
		return ThrowException(Exception::Error(String::New(ex.what())));
	}	
}
Handle<Value> PspaceNode::hisAlarmAsy(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	Alarm* t;
	try {
		Handle<Object> robj = Object::New();
		String::Utf8Value str(args[0]);
		const char * pstr = ToCString(str);
		std::vector<std::string> result=split(pstr,".");
		REQ_FUN_ARG(args.Length()-1, callback);
		t = new Alarm(ps,&callback);
		if (3==args.Length())
		{
			t->id = t->getTagID(UTF8ToGBK(replace_all(result[0],"/","\\").c_str()).c_str(),ps->hHanle_);
			Handle<Object> obj = args[1]->ToObject();
			Local<v8::Value> filterField= obj->Get(v8::String::New("filterField"));
			Local<Value> filter = obj->Get(String::New("filter"));

			Local<Date> date  =  Local<Date>::Cast(obj->Get(String::New("startTime")));
			V8DATE2PSTIME(&(t->startTime_),date);
			

			Local<Date> date1  =  Local<Date>::Cast(obj->Get(String::New("endTime")));
			V8DATE2PSTIME(&(t->endTime_),date1);


			//默认情况查询指定的这个测点的报警
			t->filterField_.TagId = PSTRUE;
			t->filter_.TagId = t->id;
			//filterField和filter为0的情况
			if (filterField->IsInt32() && 0==filterField->ToInt32()->Value())
			{
				PS_ALARM_FILTER_FIELD tmpFilterField = {0};
				PS_ALARM_FILTER tmpFilter = {0};
				t->filterField_ = tmpFilterField;
				t->filter_ = tmpFilter;
			}else//指定了过滤条件的情况
			{
				Handle<Object> filterFieldObj = filterField->ToObject();
				Handle<Object> filterObj = filter->ToObject();

				//获取所有的Key
				Handle<Array> filterFieldKeys = filterFieldObj->GetOwnPropertyNames();
				Handle<Array> filterKeys = filterObj->GetOwnPropertyNames();
				//匹配filterFieldKey并赋值
				for (int i=0;i<filterFieldKeys->Length();i++)
				{
					String::Utf8Value str(filterFieldKeys->Get(i));
					const char * pstr = ToCString(str);
					if (0==stricmp(pstr,"All"))
					{
						Local<Value> all = filterFieldObj->Get(String::New("All"));
						t->filterField_.All = all->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmId"))
					{
						Local<Value> alarmId = filterFieldObj->Get(String::New("AlarmId"));
						t->filterField_.AlarmId = alarmId->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"TagId"))
					{
						Local<Value> tagId = filterFieldObj->Get(String::New("TagId"));
						t->filterField_.TagId = tagId->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"UserID"))
					{
						Local<Value> userId = filterFieldObj->Get(String::New("UserID"));
						t->filterField_.UserID = userId->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmHaveAcked"))
					{
						Local<Value> alarmHaveAcked = filterFieldObj->Get(String::New("AlarmHaveAcked"));
						t->filterField_.AlarmHaveAcked = alarmHaveAcked->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmAckedId"))
					{
						Local<Value> alarmAckedId = filterFieldObj->Get(String::New("AlarmAckedId"));
						t->filterField_.AlarmAckedId = alarmAckedId->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmLowLevel"))
					{
						Local<Value> alarmLowLevel = filterFieldObj->Get(String::New("AlarmLowLevel"));
						t->filterField_.AlarmLowLevel = alarmLowLevel->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmHighLevel"))
					{
						Local<Value> alarmHighLevel = filterFieldObj->Get(String::New("AlarmHighLevel"));
						t->filterField_.AlarmHighLevel = alarmHighLevel->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmContent"))
					{
						Local<Value> alarmContent = filterFieldObj->Get(String::New("AlarmContent"));
						t->filterField_.AlarmContent = alarmContent->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"IsQueryOneLevel"))
					{
						Local<Value> isQueryOneLevel = filterFieldObj->Get(String::New("IsQueryOneLevel"));
						t->filterField_.IsQueryOneLevel = isQueryOneLevel->ToBoolean()->Value();
					}
				}

				//匹配filterKey并赋值
				for (int i=0;i<filterKeys->Length();i++)
				{
					String::Utf8Value str1(filterKeys->Get(i));
					const char * pstr1 = ToCString(str1);
					if (0==stricmp(pstr1,"AlarmId"))
					{
						Local<Value> alarmId = filterObj->Get(String::New("AlarmId"));
						t->filter_.AlarmId = alarmId->ToInt32()->Value();
					}
					if (0==stricmp(pstr1,"TagId"))
					{
						Local<Value> tagId = filterObj->Get(String::New("TagId"));
						t->filter_.TagId = tagId->ToInt32()->Value();
					}
					if (0==stricmp(pstr1,"UserID"))
					{
						Local<Value> userID = filterObj->Get(String::New("UserID"));
						t->filter_.UserID = userID->ToInteger()->Value();
					}
					if (0==stricmp(pstr1,"AlarmHaveAcked"))
					{
						Local<Value> alarmHaveAcked = filterObj->Get(String::New("AlarmHaveAcked"));
						t->filter_.AlarmHaveAcked = alarmHaveAcked->ToBoolean()->Value();
					}
					if (0==stricmp(pstr1,"AlarmAckedId"))
					{
						Local<Value> alarmAckedId = filterObj->Get(String::New("AlarmAckedId"));
						t->filter_.AlarmAckedId = alarmAckedId->ToInteger()->Value();
					}
					if (0==stricmp(pstr1,"AlarmLowLevel"))
					{
						Local<Value> alarmLowLevel = filterObj->Get(String::New("AlarmLowLevel"));
						t->filter_.AlarmLowLevel = alarmLowLevel->ToNumber()->Value();
					}
					if (0==stricmp(pstr1,"AlarmHighLevel"))
					{
						Local<Value> alarmHighLevel = filterObj->Get(String::New("AlarmHighLevel"));
						t->filter_.AlarmHighLevel = alarmHighLevel->ToNumber()->Value();
					}
					if (0==stricmp(pstr1,"AlarmContent"))
					{
						Local<Value> alarmContent = filterObj->Get(String::New("AlarmContent"));
						String::Utf8Value sstr(alarmContent);
						const char * content = ToCString(sstr);
						t->filter_.AlarmContent = (PSSTR)content;
					}
					if (0==stricmp(pstr1,"IsQueryOneLevel"))
					{
						Local<Value> isQueryOneLevel = filterObj->Get(String::New("IsQueryOneLevel"));
						t->filter_.IsQueryOneLevel = isQueryOneLevel->ToBoolean()->Value();
					}	
				}
			}
		}else if(6==args.Length())
		{
			/*REQ_FUN_ARG(5, callback);
			t = new Alarm(ps,&callback);*/
			t->id = t->getTagID(UTF8ToGBK(replace_all(result[0],"/","\\").c_str()).c_str(),ps->hHanle_);
			Handle<Object> obj1 = args[1]->ToObject();
			Handle<Object> obj2 = args[2]->ToObject();

			Local<Date> date  =  Local<Date>::Cast(args[3]);
			V8DATE2PSTIME(&(t->startTime_),date);

			Local<Date> date1  =  Local<Date>::Cast(args[4]);
			V8DATE2PSTIME(&(t->endTime_),date1);

			//默认情况查询指定的这个测点的报警
			t->filterField_.TagId = PSTRUE;
			t->filter_.TagId = t->id;
			//filterField和filter为0的情况
			if (obj1->IsInt32() && obj2->IsInt32() && 0==obj1->ToInt32()->Value() && 0==obj2->ToInt32()->Value())
			{
				PS_ALARM_FILTER_FIELD tmpFilterField = {0};
				PS_ALARM_FILTER tmpFilter = {0};
				t->filterField_ = tmpFilterField;
				t->filter_ = tmpFilter;
			}else//指定了过滤条件的情况
			{
				//获取所有的Key
				Handle<Array> filterFieldKeys = obj1->GetOwnPropertyNames();
				Handle<Array> filterKeys = obj2->GetOwnPropertyNames();
				//匹配filterFieldKey并赋值
				for (int i=0;i<filterFieldKeys->Length();i++)
				{
					String::Utf8Value str(filterFieldKeys->Get(i));
					const char * pstr = ToCString(str);
					if (0==stricmp(pstr,"All"))
					{
						Local<Value> all = obj1->Get(String::New("All"));
						t->filterField_.All = all->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmId"))
					{
						Local<Value> alarmId = obj1->Get(String::New("AlarmId"));
						t->filterField_.AlarmId = alarmId->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"TagId"))
					{
						Local<Value> tagId = obj1->Get(String::New("TagId"));
						t->filterField_.TagId = tagId->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"UserID"))
					{
						Local<Value> userId = obj1->Get(String::New("UserID"));
						t->filterField_.UserID = userId->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmHaveAcked"))
					{
						Local<Value> alarmHaveAcked = obj1->Get(String::New("AlarmHaveAcked"));
						t->filterField_.AlarmHaveAcked = alarmHaveAcked->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmAckedId"))
					{
						Local<Value> alarmAckedId = obj1->Get(String::New("AlarmAckedId"));
						t->filterField_.AlarmAckedId = alarmAckedId->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmLowLevel"))
					{
						Local<Value> alarmLowLevel = obj1->Get(String::New("AlarmLowLevel"));
						t->filterField_.AlarmLowLevel = alarmLowLevel->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmHighLevel"))
					{
						Local<Value> alarmHighLevel = obj1->Get(String::New("AlarmHighLevel"));
						t->filterField_.AlarmHighLevel = alarmHighLevel->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"AlarmContent"))
					{
						Local<Value> alarmContent = obj1->Get(String::New("AlarmContent"));
						t->filterField_.AlarmContent = alarmContent->ToBoolean()->Value();
					}
					if (0==stricmp(pstr,"IsQueryOneLevel"))
					{
						Local<Value> isQueryOneLevel = obj1->Get(String::New("IsQueryOneLevel"));
						t->filterField_.IsQueryOneLevel = isQueryOneLevel->ToBoolean()->Value();
					}
				}

				//匹配filterKey并赋值
				for (int i=0;i<filterKeys->Length();i++)
				{
					String::Utf8Value str1(filterKeys->Get(i));
					const char * pstr1 = ToCString(str1);
					if (0==stricmp(pstr1,"AlarmId"))
					{
						Local<Value> alarmId = obj2->Get(String::New("AlarmId"));
						t->filter_.AlarmId = alarmId->ToInt32()->Value();
					}
					if (0==stricmp(pstr1,"TagId"))
					{
						Local<Value> tagId = obj2->Get(String::New("TagId"));
						t->filter_.TagId = tagId->ToInt32()->Value();
					}
					if (0==stricmp(pstr1,"UserID"))
					{
						Local<Value> userID = obj2->Get(String::New("UserID"));
						t->filter_.UserID = userID->ToInteger()->Value();
					}
					if (0==stricmp(pstr1,"AlarmHaveAcked"))
					{
						Local<Value> alarmHaveAcked = obj2->Get(String::New("AlarmHaveAcked"));
						t->filter_.AlarmHaveAcked = alarmHaveAcked->ToBoolean()->Value();
					}
					if (0==stricmp(pstr1,"AlarmAckedId"))
					{
						Local<Value> alarmAckedId = obj2->Get(String::New("AlarmAckedId"));
						t->filter_.AlarmAckedId = alarmAckedId->ToInteger()->Value();
					}
					if (0==stricmp(pstr1,"AlarmLowLevel"))
					{
						Local<Value> alarmLowLevel = obj2->Get(String::New("AlarmLowLevel"));
						t->filter_.AlarmLowLevel = alarmLowLevel->ToNumber()->Value();
					}
					if (0==stricmp(pstr1,"AlarmHighLevel"))
					{
						Local<Value> alarmHighLevel = obj2->Get(String::New("AlarmHighLevel"));
						t->filter_.AlarmHighLevel = alarmHighLevel->ToNumber()->Value();
					}
					if (0==stricmp(pstr1,"AlarmContent"))
					{
						Local<Value> alarmContent = obj2->Get(String::New("AlarmContent"));
						String::Utf8Value sstr(alarmContent);
						const char * content = ToCString(sstr);
						t->filter_.AlarmContent = (PSSTR)content;
					}
					if (0==stricmp(pstr1,"IsQueryOneLevel"))
					{
						Local<Value> isQueryOneLevel = obj2->Get(String::New("IsQueryOneLevel"));
						t->filter_.IsQueryOneLevel = isQueryOneLevel->ToBoolean()->Value();
					}	
				}
			}
		}
	} catch(PsException &ex) {
		return scope.Close(ThrowException(Exception::Error(String::New(ex.what()))));
	}
	uv_work_t* req = new uv_work_t();
	req->data = t;
	uv_queue_work(uv_default_loop(), req, hisAlarmWork, (uv_after_work_cb)afterAlarmReal);
	ps->Ref();
	return scope.Close(Undefined());
}
void PspaceNode::hisAlarmWork(uv_work_t* req)
{
	Alarm* t = static_cast<Alarm*>(req->data);
	t->code_  = PSRET_OK;
	t->errString = NULL;
	PSAPIStatus nRet  = PSRET_OK;
	try {
		if(t->psNode->hHanle_ == PSHANDLE_UNUSED) {
			throw PsException("Connection already closed");
		}else{
			nRet = psAPI_Alarm_His_Query( t->psNode->hHanle_,&(t->filterField_), 
										  &(t->filter_),t->startTime_,t->endTime_, 
										  &(t->alarmCount_), &(t->alarms_));
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

Handle<Value> PspaceNode::ackAlarmSyn(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	Alarm* t;
	try { 
		t = new Alarm(ps,NULL);
		
		Local<Object> robj =  Object::New();
		if (args[1]->IsObject())
		{
			REQ_OBJECT_ARG(1, settings);
			
			Local<Date> date  =  Local<Date>::Cast(settings->Get(String::New("ackTime")));
			V8DATE2PSTIME(&(t->ackTime_),date);

			
			GET_NUMBER(settings,"ackUserId",t->ackUserId_);
			
			Handle<v8::Value> idObj = settings->Get(v8::String::New("ackIds"));
			static Array* valID = v8::Array::Cast(*idObj);
			t->ackCount_= valID->Length();
			
			String::Utf8Value uStr(settings->Get(String::New("ackUserName")));
			const char* userName = ToCString(uStr);
			strcpy(t->ackUserName_,(PSSTR)userName);
			
			
			t->alarmIds_ = new PSUINT32[t->ackCount_];
			for (int i=0;i<t->ackCount_;i++)
			{
				t->alarmIds_[i] = (valID->Get(i)->ToUint32())->Value();
			}
			
		}else if(5==args.Length())
		{
			Local<Date> date  =  Local<Date>::Cast(args[3]);
			V8DATE2PSTIME(&(t->startTime_),date);
			
			t->ackUserId_ = args[1]->ToUint32()->Value();
			
			String::Utf8Value uStr(args[2]);
			const char* userName = ToCString(uStr);
			strcpy(t->ackUserName_,(PSSTR)userName);

			Handle<Value> idObj = args[4]->ToObject();
			static Array* valID = v8::Array::Cast(*idObj);
			t->ackCount_ = valID->Length();
			t->alarmIds_ = new PSUINT32[t->ackCount_];
			
			for (int i=0;i<t->ackCount_;i++)
			{
				t->alarmIds_[i] = (valID->Get(i)->ToUint32())->Value();
			}
			
		}else{
			t->code_ = -1;
			t->errString = new std::string("参数错误!");
			Local<Object> errObj = Error::newObj();
			errObj->Set(String::New("code"),Number::New(-1));
			errObj->Set(String::New("errString"),String::New(GBK2UTF8("参数错误!").c_str()));
			return errObj;
		}
		uv_work_t* req = new uv_work_t();
		req->data = t;
		t->psNode->Ref();
		ackAlarmWork(req);
		t->psNode->Unref();
		//如果失败
		if(t->errString) {
			Local<Object> errObj = Error::newObj();
			errObj->Set(String::New("code"),Number::New(t->code_));
			std:string *s =t->errString;
			errObj->Set(String::New("errString"),String::New(GBK2UTF8(s->c_str()).c_str()));
			delete t;
			return errObj;
		}
		//成功返回数据
		robj->Set(String::New("userId"),Int32::New(t->ackUserId_));
		robj->Set(String::New("userName"),String::New(t->ackUserName_));
		robj->Set(String::New("ackTime"),PSTIME2V8DATE(t->ackTime_));
		delete t;
		return robj;
		
	} catch(PsException &ex) {
		return ThrowException(Exception::Error(String::New(ex.what())));
	}	
}

Handle<Value> PspaceNode::ackAlarmAsy(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	Alarm* t;
	try {
		Handle<Object> robj = Object::New();
		REQ_FUN_ARG(args.Length()-1, callback);
		t = new Alarm(ps,&callback);
		if (args[1]->IsObject())
		{
			REQ_OBJECT_ARG(1, settings);
			Local<Date> date  =  Local<Date>::Cast(settings->Get(String::New("ackTime")));
			V8DATE2PSTIME(&(t->ackTime_),date);

			
			GET_NUMBER(settings,"ackUserId",t->ackUserId_);

			Handle<v8::Value> idObj = settings->Get(v8::String::New("ackIds"));
			static Array* valID = v8::Array::Cast(*idObj);
			t->ackCount_= valID->Length();

			String::Utf8Value uStr(settings->Get(String::New("ackUserName")));
			const char* userName = ToCString(uStr);
			strcpy(t->ackUserName_,(PSSTR)userName);


			t->alarmIds_ = new PSUINT32[t->ackCount_];
			for (int i=0;i<t->ackCount_;i++)
			{
				t->alarmIds_[i] = (valID->Get(i)->ToUint32())->Value();
			}

		}else if(6==args.Length())
		{
			/*REQ_FUN_ARG(5, callback);
			t = new Alarm(ps,&callback);*/
			Local<Date> date  =  Local<Date>::Cast(args[3]);
			V8DATE2PSTIME(&(t->startTime_),date);
			
			t->ackUserId_ = args[1]->ToUint32()->Value();

			String::Utf8Value uStr(args[2]);
			const char* userName = ToCString(uStr);
			strcpy(t->ackUserName_,(PSSTR)userName);

			Handle<Value> idObj = args[4]->ToObject();
			static Array* valID = v8::Array::Cast(*idObj);
			t->ackCount_ = valID->Length();
			t->alarmIds_ = new PSUINT32[t->ackCount_];
			
			for (int i=0;i<t->ackCount_;i++)
			{
				t->alarmIds_[i] = (valID->Get(i)->ToUint32())->Value();
			}

		}else{
			t->code_ = -1;
			t->errString = new std::string("参数错误!");
			robj->Set(String::New("code"),Number::New(-1));
			robj->Set(String::New("errString"),String::New(GBK2UTF8("参数错误!").c_str()));
			return robj;
		}
		
	} catch(PsException &ex) {
		return scope.Close(ThrowException(Exception::Error(String::New(ex.what()))));
	}
	uv_work_t* req = new uv_work_t();
	req->data = t;
	uv_queue_work(uv_default_loop(), req, ackAlarmWork, (uv_after_work_cb)afterAckAlarm);
	ps->Ref();
	return scope.Close(Undefined());
}

void PspaceNode::ackAlarmWork(uv_work_t* req)
{
	Alarm* t = static_cast<Alarm*>(req->data);
	t->code_  = PSRET_OK;
	t->errString = NULL;
	PSAPIStatus nRet  = PSRET_OK;
	try {
		if(t->psNode->hHanle_ == PSHANDLE_UNUSED) {
			throw PsException("Connection already closed");
		}else{
			
			nRet=psAPI_Alarm_Ack( t->psNode->hHanle_,t->ackUserId_,
								  t->ackUserName_,t->ackTime_,
								  t->ackCount_,t->alarmIds_);
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

void PspaceNode::afterAckAlarm(uv_work_t* req,int status)
{
	HandleScope scope;
	Alarm* t = static_cast<Alarm*>(req->data);
	t->psNode->Unref();
	try {
		if(t->errString){
			Handle<Value> argv[2];
			argv[0] = Exception::Error(String::New(GBK2UTF8(t->errString->c_str()).c_str()));
			argv[1] = Undefined();
			node::MakeCallback(Context::GetCurrent()->Global(), t->callback,2, argv);
		}else{
			Handle<Value> argv[2];
			//结果设置
			argv[0] = Undefined();
			//成功返回数据
			Local<Object> robj = Object::New();
			robj->Set(String::New("userId"),Int32::New(t->ackUserId_));
			robj->Set(String::New("userName"),String::New(t->ackUserName_));
			robj->Set(String::New("ackTime"),PSTIME2V8DATE(t->ackTime_));
			argv[1] = robj;
			node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 2, argv);
		}
	} catch(PsException &ex) {
		Handle<Value> argv[2];
		argv[0] = Exception::Error(String::New(ex.what()));
		argv[1] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 2, argv);
	} catch(const exception &ex) {
		Handle<Value> argv[2];
		argv[0] = Exception::Error(String::New(t->errString->c_str()));
		argv[1] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), t->callback, 2, argv);
	}
	delete t;
	scope.Close(Undefined());
}

void PspaceNode::batchReadWork(uv_work_t* req)
{
    BatchBaton* t = static_cast<BatchBaton*>(req->data);
    t->code_  = PSRET_OK;
    t->errString = NULL;
    PSAPIStatus nRet  = PSRET_OK;
    //PSAPIStatus *pAPIErrors = PSNULL;
    try {
        if(t->psNode->hHanle_ == PSHANDLE_UNUSED) {
            throw PsException("Connection already closed");
        }else{
            PSUINT32 *tagIDs = *(t->getTagIDList(t->tagName_,t->psNode->hHanle_,t->tagCount_));

            nRet = psAPI_Real_ReadList(t->psNode->hHanle_, t->tagCount_, 
                tagIDs, 
                &(t->realData_), &t->pAPIErrors);
			//读取整个实时数据列表出错
            if (PSERR(nRet) && nRet != PSERR_FAIL_IN_BATCH)
            {
                t->code_ = nRet;
                t->errString = new std::string(psAPI_Commom_GetErrorDesc(nRet));
            } 
            if (nRet == PSERR_FAIL_IN_BATCH)
            {
                t->code_ = nRet;
                t->errString = new std::string(psAPI_Commom_GetErrorDesc(nRet));
                //psAPI_Memory_FreeAndNull((PSVOID**)&pAPIErrors);
            }
            psAPI_Memory_FreeAndNull((PSVOID**)&tagIDs);
		}	
	}catch(PsException &ex) {
		t->errString = new string(ex.what());
	} catch (const exception& ex) {
		t->errString = new string(ex.what());
	} catch (...) {
		t->errString = new string("Unknown Error");
	}
}

void PspaceNode::batchWriteWork(uv_work_t* req)
{
	BatchBaton* t = static_cast<BatchBaton*>(req->data);
	t->code_  = PSRET_OK;
	t->errString = NULL;
	PSAPIStatus nRet  = PSRET_OK;
	PSAPIStatus *pAPIErrors = PSNULL;
	try {
		if(t->psNode->hHanle_ == PSHANDLE_UNUSED) {
			throw PsException("Connection already closed");
		}else{
            PSUINT32 *tagIDs = *(t->getTagIDList(t->tagName_,t->psNode->hHanle_,t->tagCount_));

			nRet = psAPI_Real_WriteList(t->psNode->hHanle_, t->tagCount_, 
				tagIDs,t->dataValues_,t->timeStamps_, t->qualitys_, &pAPIErrors);

            if (PSERR(nRet) && nRet != PSERR_FAIL_IN_BATCH)
            {
                t->code_ = nRet;
                t->errString = new std::string(psAPI_Commom_GetErrorDesc(nRet));
            } 
            if (nRet == PSERR_FAIL_IN_BATCH)
            {
                t->code_ = nRet;
                t->errString = new std::string(psAPI_Commom_GetErrorDesc(nRet));
                psAPI_Memory_FreeAndNull((PSVOID**)&pAPIErrors);
            }
            psAPI_Memory_FreeAndNull((PSVOID**)&tagIDs);
		}	
	}catch(PsException &ex) {
		t->errString = new string(ex.what());
	} catch (const exception& ex) {
		t->errString = new string(ex.what());
	} catch (...) {
		t->errString = new string("Unknown Error");
	}

}
void PspaceNode::afterBatchRead(uv_work_t* req, int status)
{
	HandleScope scope;
	BatchBaton* bat = static_cast<BatchBaton*>(req->data);
	bat->psNode->Unref();
	try {
		if(bat->errString){
			Handle<Value> argv[2];
			argv[0] = Exception::Error(String::New(GBK2UTF8(bat->errString->c_str()).c_str()));
			argv[1] = Undefined();
			node::MakeCallback(Context::GetCurrent()->Global(), bat->callback, 2, argv);
		}else{
			Handle<Value> argv[2];
			//结果设置
			argv[0] = Undefined();
			Local<Array> arrObj = Array::New(bat->tagCount_);
			for (int n=0;n<bat->tagCount_;n++)
			{
				arrObj->Set(n,getRealObj(bat->realData_+n));		
			}
			argv[1] = arrObj;
			node::MakeCallback(Context::GetCurrent()->Global(), bat->callback, 2, argv);
		}
	} catch(PsException &ex) {
		Handle<Value> argv[2];
		argv[0] = Exception::Error(String::New(ex.what()));
		argv[1] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), bat->callback, 2, argv);
        FREE_MEMORY(bat);
        FREE_MEMORY(req);
	} catch(const exception &ex) {
		Handle<Value> argv[2];
		argv[0] = Exception::Error(String::New(GBK2UTF8(bat->errString->c_str()).c_str()));
		argv[1] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), bat->callback, 2, argv);
        FREE_MEMORY(bat);
        FREE_MEMORY(req);
	}
    FREE_MEMORY(bat);
    FREE_MEMORY(req);
	scope.Close(Undefined());
}

void PspaceNode::afterBatchWrite(uv_work_t* req, int status)
{
	HandleScope scope;
	BatchBaton* bat = static_cast<BatchBaton*>(req->data);
	bat->psNode->Unref();
	try {
		if(bat->errString){
			Handle<Value> argv[2];
			
			argv[0] = Exception::Error(String::New((bat->errString->c_str())));
			argv[1] = Undefined();
			node::MakeCallback(Context::GetCurrent()->Global(), bat->callback, 2, argv);
		}else{
			Handle<Value> argv[2];
			//结果设置
			argv[0] = Undefined();
			argv[1] = Boolean::New(true);
			node::MakeCallback(Context::GetCurrent()->Global(), bat->callback, 2, argv);
		}
	} catch(PsException &ex) {
		Handle<Value> argv[2];
		argv[0] = Exception::Error(String::New(ex.what()));
		argv[1] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), bat->callback, 2, argv);
        FREE_MEMORY(bat);
        FREE_MEMORY(req);
	} catch(const exception &ex) {
		Handle<Value> argv[2];
		argv[0] = Exception::Error(String::New(GBK2UTF8(bat->errString->c_str()).c_str()));
		argv[1] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), bat->callback, 2, argv);
        FREE_MEMORY(bat);
        FREE_MEMORY(req);
	}
    FREE_MEMORY(bat);
    FREE_MEMORY(req);

}

Handle<Value> PspaceNode::batRealReadSyn(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	BatchBaton* bat = NULL;
    uv_work_t* req = NULL;
	try {
		//Handle<Object> robj = Object::New();
		bat = new BatchBaton(ps,NULL);
		REQ_ARRAY_ARG(0,tagArr);
		bat->tagCount_ = tagArr->Length();
		bat->tagName_ = new PSSTR[bat->tagCount_];
		for (int i=0;i<bat->tagCount_;i++)
		{
			String::Utf8Value iStr(tagArr->Get(i));
			const char* IDStr = ToCString(iStr);
			std::vector<std::string> result=split(IDStr,".");
            std::string strtemp = UTF8ToGBK(replace_all(result[0],"/","\\"));
			bat->tagName_[i] = new char[strtemp.length()+1];
			strcpy(bat->tagName_[i],strtemp.c_str());
			bat->tagName_[i][strlen(bat->tagName_[i])] = 0;

		}
		req = new uv_work_t();
		req->data = bat;
		bat->psNode->Ref();
		batchReadWork(req);
		bat->psNode->Unref();
		if (PSERR(bat->code_) && bat->code_ != PSERR_FAIL_IN_BATCH)//出现了除操作集元素以外的错误
		{
			Local<Array> arrObj = Array::New(1);
			Local<Object> errObj = Error::newObj();
			errObj->Set(String::New("code"),Number::New(bat->code_));
			errObj->Set(String::New("errString"),String::New(GBK2UTF8(bat->errString->c_str()).c_str()));
			arrObj->Set(0,errObj);
			FREE_MEMORY(bat);
			FREE_MEMORY(req);
			return scope.Close(arrObj);
		}
		else if(bat->code_ ==  PSERR_FAIL_IN_BATCH)//部分错误
		{
 			Local<Array> arrObj = Array::New(bat->tagCount_);
			//“操作集中有出错的元素“
			Local<Object> errObj1 = Error::newObj();
			errObj1->Set(String::New("code"),Number::New(bat->code_));
			errObj1->Set(String::New("errString"),String::New(GBK2UTF8(bat->errString->c_str()).c_str()));
			arrObj->Set(0,errObj1);
 			for (int n=0;n<bat->tagCount_;n++)
			{
				if (bat->pAPIErrors[n])//如果第n个点错误
				{
					//对应操作集中每个元素，有错：显示具体的错误信息； 没错:显示具体测点的pv值
					Local<Object> errObj = Error::newObj();
					errObj->Set(String::New("code"),Number::New(bat->pAPIErrors[n]));
					bat->errString = new std::string(psAPI_Commom_GetErrorDesc(bat->pAPIErrors[n]));
					errObj->Set(String::New("errString"),String::New(GBK2UTF8(bat->errString->c_str()).c_str()));
					arrObj->Set(n+1,errObj);
				} 
				else{
					arrObj->Set(n+1,getRealObj(bat->realData_+n));
				}

			}
			psAPI_Memory_FreeAndNull((PSVOID**)&bat->pAPIErrors);
			FREE_MEMORY(bat);
			FREE_MEMORY(req);
			return scope.Close(arrObj);
		}else{//正确
			Local<Array> arrObj = Array::New(bat->tagCount_);
			for (int n=0;n<bat->tagCount_;n++)
			{
				arrObj->Set(n,getRealObj(bat->realData_+n));
			}
			FREE_MEMORY(bat);
			FREE_MEMORY(req);
			return scope.Close(arrObj);
		}
	} catch(PsException &ex) {
        FREE_MEMORY(bat);
        FREE_MEMORY(req);
		return ThrowException(Exception::Error(String::New(ex.what())));
	}	
}
Handle<Value> PspaceNode::batRealReadAsy(const Arguments& args)
{
	
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	REQ_ARRAY_ARG(0,tagArr);
	REQ_FUN_ARG(args.Length()-1, callback);
	BatchBaton* bat = NULL;
    uv_work_t* req  = NULL;
	try {
		bat = new BatchBaton(ps, &callback);
		bat->tagCount_ = tagArr->Length();
		//bat->tagID_ = new PSUINT32[tagArr->Length()];
		bat->tagName_ = new PSSTR[bat->tagCount_];
		for (int i=0;i<bat->tagCount_;i++)
		{
			String::Utf8Value iStr(tagArr->Get(i));
			const char* IDStr = ToCString(iStr);
			std::vector<std::string> result=split(IDStr,".");
			const char* str = replace_all(result[0],"/","\\").c_str();
			bat->tagName_[i] = new char[strlen(str)+1];
			strcpy(bat->tagName_[i],str);
			bat->tagName_[i][strlen(bat->tagName_[i])] = 0;
		}
	    req = new uv_work_t();
	    req->data = bat;
	    uv_queue_work(uv_default_loop(), req, batchReadWork, (uv_after_work_cb)afterBatchRead);
	    ps->Ref();
	    return scope.Close(Undefined());

    } catch(PsException &ex) {
        FREE_MEMORY(bat);
        FREE_MEMORY(req);
        return scope.Close(ThrowException(Exception::Error(String::New(ex.what()))));
    }
}

Handle<Value> PspaceNode::batRealWriteSyn(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	BatchBaton* bat = NULL;
    uv_work_t* req = NULL;
	try {
		bat = new BatchBaton(ps,NULL);
		REQ_OBJECT_ARG(0,dataObj);
		Handle<Array> keys = dataObj->GetOwnPropertyNames();
		bat->tagCount_ = keys->Length();
		//bat->tagID_ = new PSUINT32[keys->Length()];
		bat->tagName_ = new PSSTR[bat->tagCount_];
		bat->qualitys_ = new PSUINT32[keys->Length()];
		bat->dataValues_ = new PS_VARIANT[keys->Length()];
		bat->timeStamps_ = new PS_TIME [keys->Length()];
		for (int i=0;i<bat->tagCount_;i++)
		{
			String::Utf8Value iStr(keys->Get(i));
			const char* IDStr = ToCString(iStr);
			std::vector<std::string> result=split(IDStr,".");
			const char* str = replace_all(result[0],"/","\\").c_str();
			bat->tagName_[i] = new char[strlen(str)+1];
			strcpy(bat->tagName_[i],str);
			bat->tagName_[i][strlen(bat->tagName_[i])] = 0;	
			bat->qualitys_[i]= PS_QUALITY_UNCERTAIN;
			PS_TIME tmpTime;
			SYSTEMTIME st;
			GetLocalTime(&st);
			tmpTime.Millisec=st.wMilliseconds;
			tmpTime.Second = time(NULL);
			bat->timeStamps_[i] = tmpTime;

			if(dataObj->Get(String::New(IDStr))->IsObject()){
				Local<Object> valObj = dataObj->Get(String::New(IDStr))->ToObject();
				if(valObj->Has(v8::String::New("value")))
				{
					Local<Value> obj = valObj->Get(String::New("value"));
					if (obj->IsInt32())
					{
						bat->dataValues_[i].DataType = PSDATATYPE_INT32;
						GET_INTER(valObj, "value", bat->dataValues_[i].Int32);
					}else if (obj->IsBoolean())
					{
						bat->dataValues_[i].DataType = PSDATATYPE_BOOL;
						bat->dataValues_[i].Bool =obj->ToBoolean()->Value();
					}else if (obj->IsNumber())
					{
						bat->dataValues_[i].DataType = PSDATATYPE_DOUBLE;
						GET_NUMBER(valObj, "value", bat->dataValues_[i].Double);
					}else if (obj->IsString())
					{
						bat->dataValues_[i].DataType = PSDATATYPE_STRING;
						String::Utf8Value tmpStr(obj);
						const char *pstr = ToCString(tmpStr);
						bat->dataValues_[i].String.Data = new char[strlen(pstr)+1];
						strcpy(bat->dataValues_[i].String.Data,pstr);
						bat->dataValues_[i].String.Data[strlen(pstr)]=0;
						bat->dataValues_[i].String.Length = strlen(bat->dataValues_[i].String.Data);
					}	
				}
				if(valObj->Has(v8::String::New("quality")))
				{
                    Local<Value> qObj = valObj->Get(String::New("quality"));
                    if (qObj->IsInt32())
                    {
                        GET_INTER(valObj, "quality", bat->qualitys_[i]);
                    }
                    else if (qObj->IsString())
                    {
                        String::Utf8Value str(valObj->Get(String::New("quality")));
					    const char* pstr = ToCString(str);
				    	bat->qualitys_[i] = bat->getQuality(pstr);
                    }
				}
				//时间戳
				if (valObj->Has(v8::String::New("time"))) 
				{
					Local<v8::Value> value = valObj->Get(v8::String::New("time"));
					if (!value->IsNull())
					{
						v8::Local<v8::Date> date = v8::Local<v8::Date>::Cast(value);
						V8DATE2PSTIME(&bat->timeStamps_[i],date);
					}else{
						bat->timeStamps_[i] = tmpTime;
					}
					
				}
			}else if (!dataObj->Get(String::New(IDStr))->IsObject())
			{
				Local<Value> valObj = dataObj->Get(String::New(IDStr));
				if (valObj->IsInt32())
				{
					bat->dataValues_[i].DataType = PSDATATYPE_INT32;
					bat->dataValues_[i].Int32 = valObj->ToInt32()->Value();
				}else if (valObj->IsBoolean())
				{
					bat->dataValues_[i].DataType = PSDATATYPE_BOOL;
					bat->dataValues_[i].Bool = valObj->ToBoolean()->Value(); 
				}else if (valObj->IsNumber())
				{
					bat->dataValues_[i].DataType = PSDATATYPE_DOUBLE;
					bat->dataValues_[i].Double = valObj->ToNumber()->Value();
				}else if (valObj->IsString())
				{
					bat->dataValues_[i].DataType = PSDATATYPE_STRING;
					String::Utf8Value tmpStr(valObj);
					const char *pstr = ToCString(tmpStr);
					bat->dataValues_[i].String.Data = new char[strlen(pstr)+1];
					strcpy(bat->dataValues_[i].String.Data,pstr);
					bat->dataValues_[i].String.Data[strlen(pstr)]=0;
					bat->dataValues_[i].String.Length = strlen(bat->dataValues_[i].String.Data);
				}
			}
		}
		req = new uv_work_t();
		req->data = bat;
		bat->psNode->Ref();
		
		batchWriteWork(req);
	
		bat->psNode->Unref();
		//如果失败
		if(bat->errString) {
			Local<Object> errObj = Error::newObj();
			errObj->Set(String::New("code"),Number::New(bat->code_));
			errObj->Set(String::New("errString"),String::New((bat->errString->c_str())));
            FREE_MEMORY(bat);
            FREE_MEMORY(req);
			return errObj;
		}
		//  成功
        FREE_MEMORY(bat);
        FREE_MEMORY(req);
		return scope.Close(Boolean::New(true));
	} catch(PsException &ex) {
        FREE_MEMORY(bat);
        FREE_MEMORY(req);
		return ThrowException(Exception::Error(String::New(ex.what())));
	}	
}
Handle<Value> PspaceNode::batRealWriteAsy(const Arguments& args)
{
	
	HandleScope scope;
	
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	REQ_FUN_ARG(args.Length()-1, callback);
	BatchBaton* bat;
	try {
		bat = new BatchBaton(ps, &callback);
		REQ_OBJECT_ARG(0,dataObj);
		Handle<Array> keys = dataObj->GetOwnPropertyNames();
		bat->tagCount_ = keys->Length();
		bat->tagName_ = new PSSTR[bat->tagCount_];
		bat->qualitys_ = new PSUINT32[keys->Length()];
		bat->dataValues_ = new PS_VARIANT[keys->Length()];
		for (int i=0;i<bat->tagCount_;i++)
		{
			String::Utf8Value iStr(keys->Get(i));
			const char* IDStr = ToCString(iStr);
			std::vector<std::string> result=split(IDStr,".");
			const char* str = replace_all(result[0],"/","\\").c_str();
			bat->tagName_[i] = new char[strlen(str)+1];
			strcpy(bat->tagName_[i],str);
			bat->tagName_[i][strlen(bat->tagName_[i])] = 0;
			PS_TIME tmpTime;
			SYSTEMTIME st;
			GetLocalTime(&st);
			tmpTime.Millisec=st.wMilliseconds;
			tmpTime.Second = time(NULL);
			bat->timeStamps_[i] = tmpTime;
			bat->qualitys_[i]= PS_QUALITY_UNCERTAIN;
			if(dataObj->Get(String::New(IDStr))->IsObject()){
				Local<Object> valObj = dataObj->Get(String::New(IDStr))->ToObject();
				if(valObj->Has(v8::String::New("value")))
				{
					Local<Value> obj = valObj->Get(String::New("value"));
					if (obj->IsInt32())
					{
						bat->dataValues_[i].DataType = PSDATATYPE_INT32;
						GET_INTER(valObj, "value", bat->dataValues_[i].Int32);
					}else if (obj->IsBoolean())
					{
						bat->dataValues_[i].DataType = PSDATATYPE_BOOL;
						bat->dataValues_[i].Bool =obj->ToBoolean()->Value();
					}else if (obj->IsNumber())
					{
						bat->dataValues_[i].DataType = PSDATATYPE_DOUBLE;
						GET_NUMBER(valObj, "value", bat->dataValues_[i].Double);
					}else if (obj->IsString())
					{
						bat->dataValues_[i].DataType = PSDATATYPE_STRING;
						String::Utf8Value tmpStr(obj);
						const char *pstr = ToCString(tmpStr);
						bat->dataValues_[i].String.Data = new char[strlen(pstr)+1];
						strcpy(bat->dataValues_[i].String.Data,pstr);
						bat->dataValues_[i].String.Data[strlen(pstr)]=0;
						bat->dataValues_[i].String.Length = strlen(bat->dataValues_[i].String.Data);
					}	
				}
				if(valObj->Has(v8::String::New("quality")))
				{
                    Local<Value> qObj = valObj->Get(String::New("quality"));
                    if (qObj->IsInt32())
                    {
                        GET_INTER(valObj, "quality", bat->qualitys_[i]);
                    }
                    else if (qObj->IsString())
                    {
                        String::Utf8Value str(valObj->Get(String::New("quality")));
                        const char* pstr = ToCString(str);
                        bat->qualitys_[i] = bat->getQuality(pstr);
                    }
				}
				//时间戳
				if (valObj->Has(v8::String::New("time"))) 
				{
					Local<v8::Value> value = valObj->Get(v8::String::New("time"));
					if (!value->IsNull())
					{
						v8::Local<v8::Date> date = v8::Local<v8::Date>::Cast(value);
						V8DATE2PSTIME(&bat->timeStamps_[i],date);
					}else{
						bat->timeStamps_[i] = tmpTime;
					}
				}
			}else if (!dataObj->Get(String::New(IDStr))->IsObject())
			{
				Local<Value> valObj = dataObj->Get(String::New(IDStr));
				if (valObj->IsInt32())
				{
					bat->dataValues_[i].DataType = PSDATATYPE_INT32;
					bat->dataValues_[i].Int32 = valObj->ToInt32()->Value();
				}else if (valObj->IsBoolean())
				{
					bat->dataValues_[i].DataType = PSDATATYPE_BOOL;
					bat->dataValues_[i].Bool = valObj->ToBoolean()->Value(); 
				}else if (valObj->IsNumber())
				{
					bat->dataValues_[i].DataType = PSDATATYPE_DOUBLE;
					bat->dataValues_[i].Double = valObj->ToNumber()->Value();
				}else if (valObj->IsString())
				{
					bat->dataValues_[i].DataType = PSDATATYPE_STRING;
					String::Utf8Value tmpStr(valObj);
					const char *pstr = ToCString(tmpStr);
					bat->dataValues_[i].String.Data = new char[strlen(pstr)+1];
					strcpy(bat->dataValues_[i].String.Data,pstr);
					bat->dataValues_[i].String.Data[strlen(pstr)]=0;
					bat->dataValues_[i].String.Length = strlen(bat->dataValues_[i].String.Data);
				}
			}
		}
	} catch(PsException &ex) {
		return scope.Close(ThrowException(Exception::Error(String::New(ex.what()))));
	}
	uv_work_t* req = new uv_work_t();
	req->data = bat;
	uv_queue_work(uv_default_loop(), req, batchWriteWork, (uv_after_work_cb)afterBatchWrite);
	ps->Ref();
	
	return scope.Close(Undefined());
}

Handle<Value> PspaceNode::querySyn(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	TagQuery* t;
	try { 
		t = new TagQuery(ps,NULL);
		REQ_STRING_ARG(0,tagName);
		String::Utf8Value nameStr(tagName);
		const char *pNameStr =ToCString(nameStr);
		std::string sName = pNameStr;
		if (strstr(str2Upper(pNameStr),"IDROOT"))
		{
			t->tagID_ = PSTAGID_ROOT;
		}else{
			
			t->tagID_ = t->getTagID(UTF8ToGBK(replace_all(sName,"/","\\")).c_str(),t->psNode_->hHanle_);
			if (!isExit(t->psNode_->hHanle_,t->tagID_))
			{
					//std::cout<<"adfasasf"<<std::endl;
				Local<Object> errObj = Error::newObj();
				errObj->Set(String::New("code"),Number::New(-1));
				std::string *s =t->errString_;
				errObj->Set(String::New("errString"),String::New(GBK2UTF8("起始测点不存在!").c_str()));
				//delete t;
				return errObj;
			}
		}
		Local<Object> argObj = args[1]->ToObject();
		Local<Array> propIdsArr;
		
		if (argObj->Has(String::New("propIds")))
		{
			if (argObj->Get(String::New("propIds"))->IsNull())
			{
				t->propCount_ = 0;
				//propIdsArr = Local<Array>::New(0);
			}else{
				 propIdsArr  =Local<Array>::Cast(argObj->Get(String::New("propIds")));
				t->propCount_ = propIdsArr->Length();
				t->propIds_ = new PSUINT16[t->propCount_];
				for(int i=0;i<t->propCount_;i++){
					String::Utf8Value str(propIdsArr->Get(i));
					const char *pstr = ToCString(str);
					t->propIds_[i] = t->getPropID(pstr,t->psNode_->hHanle_);
				}
			}
		}else{
			t->propCount_ = 0;
			t->propIds_ = NULL;
		}
		if(argObj->Has(String::New("filter")))
		{
			t->filter_ = (PS_TAG_QUERY_FILTER*)malloc(sizeof(PS_TAG_QUERY_FILTER));
			t->filter_->FieldPropCount = 0;
			t->filter_->QueryLevel = 0;
			t->filter_->Result_Num = 0;
			t->filter_->Result_Start = 0;

			if (argObj->Get(String::New("filter"))->IsNull())
			{
				t->filter_ = NULL;
				t->fieldPropValues_ = NULL;
				t->fieldPropCnt_=0;
				t->fieldPropids_ = NULL;
			}else{
				Local<Object> fieldObj = argObj->Get(String::New("filter"))->ToObject();
				if (fieldObj->Has(String::New("querySelf")))
				{
					t->filter_->QuerySelf = fieldObj->Get(String::New("querySelf"))->BooleanValue();
					fieldObj->Delete(String::New("querySelf"));
				}
				if (fieldObj->Has(String::New("queryLevel")))
				{
					t->filter_->QueryLevel = fieldObj->Get(String::New("queryLevel"))->Uint32Value();
					fieldObj->Delete(String::New("queryLevel"));
				}
				if (fieldObj->Has(String::New("resultStart")))
				{
					t->filter_->Result_Start = fieldObj->Get(String::New("resultStart"))->Uint32Value();
					fieldObj->Delete(String::New("resultStart"));
				}
				if (fieldObj->Has(String::New("resultNum")))
				{
					t->filter_->Result_Num = fieldObj->Get(String::New("resultNum"))->Uint32Value();
					fieldObj->Delete(String::New("resultNum"));
				}
				Local<Array> filterFieldKeys = fieldObj->GetOwnPropertyNames();
				t->fieldPropCnt_ = filterFieldKeys->Length();
				t->fieldPropids_ = new PSUINT16[t->fieldPropCnt_];
				t->fieldPropValues_ = new PS_VARIANT[t->fieldPropCnt_];

				t->filter_->FieldPropValues = new PS_VARIANT[t->fieldPropCnt_];
				t->filter_->FieldPropIds = new PSUINT16[t->fieldPropCnt_];

				for (int i=0;i<t->fieldPropCnt_;i++)
				{
					String::Utf8Value keyStr(filterFieldKeys->Get(i));
					const char *pstr = ToCString(keyStr);
					t->fieldPropids_[i] = t->getPropID(pstr,t->psNode_->hHanle_);
					t->fieldPropValues_[i].DataType = t->getPropInfo(pstr,t->psNode_->hHanle_).DataType;
					String::Utf8Value str1(filterFieldKeys->Get(i));
					const char * pstr1 = ToCString(str1);
					switch(t->fieldPropValues_[i].DataType)
					{
					case PSDATATYPE_UINT16:
						{
							t->fieldPropValues_[i].UInt16 = (PSUINT16)fieldObj->Get(String::New(pstr1))->Uint32Value();	
						}
						break;
					case  PSDATATYPE_BOOL:
						{
							
							Local<Value> bo = fieldObj->Get(String::New(pstr1));
							PS_DATATYPE_ENUM b = (PS_DATATYPE_ENUM)bo->BooleanValue();
							t->fieldPropValues_[i].Bool = b;
						}
						break;
					case PSDATATYPE_DOUBLE:
						{
							Local<Value> db = fieldObj->Get(String::New(pstr1));
							double d = db->NumberValue();
							t->fieldPropValues_[i].Double = d;
						}
						break;
					case PSDATATYPE_INT16:
						{
							Local<Value> int16 = fieldObj->Get(String::New(pstr1));
							PS_DATATYPE_ENUM i16 = (PS_DATATYPE_ENUM)int16->Int32Value();
							t->fieldPropValues_[i].Int16 = i16;
						}
						break;
					case PSDATATYPE_INT32:
						{
							Local<Value> int32 = fieldObj->Get(String::New(pstr1));
							PSINT32 i32 = (PSINT32)(int32->Int32Value());
							t->fieldPropValues_[i].Int32 = i32;
							
						}
						break;
					case PSDATATYPE_UINT32:
						{
							Local<Value> int32 = fieldObj->Get(String::New(pstr1));
							PSUINT32 i32 = (PSUINT32)int32->Uint32Value();
							t->fieldPropValues_[i].UInt32 = i32;
						}
						break;
					case PSDATATYPE_INT64:
						{
							
							Local<Value> int64 = fieldObj->Get(String::New(pstr1));
							PS_DATATYPE_ENUM i64 = (PS_DATATYPE_ENUM)int64->Int32Value();
							t->fieldPropValues_[i].Int64 = i64;
						}
						break;
					case PSDATATYPE_UINT64:
						{
							Local<Value> uint64 = fieldObj->Get(String::New(pstr1));
							PS_DATATYPE_ENUM u64 = (PS_DATATYPE_ENUM)uint64->Uint32Value();
							t->fieldPropValues_[i].UInt64 = u64;
						}
						break;
					case PSDATATYPE_INT8:
						{
							Local<Value> int8 = fieldObj->Get(String::New(pstr1));
							PS_DATATYPE_ENUM i8 = (PS_DATATYPE_ENUM)int8->Int32Value();
							t->fieldPropValues_[i].Int8 = i8;
						}
						break;
					case PSDATATYPE_UINT8:
						{
							Local<Value> uint8 = fieldObj->Get(String::New(pstr1));
							PS_DATATYPE_ENUM u8 = (PS_DATATYPE_ENUM)uint8->Uint32Value();
							t->fieldPropValues_[i].UInt8 = u8;
						}
						break;
					case PSDATATYPE_FLOAT:
						{
							Local<Value> fl = fieldObj->Get(String::New(pstr1));
							float f = (float)fl->NumberValue();
							t->fieldPropValues_[i].Float = f;	
						}
						break;
					case  PSDATATYPE_WSTRING:
						{
							String::Utf8Value str2(fieldObj->Get(String::New(pstr1)));
							const char * pstr2 = ToCString(str2);
							t->fieldPropValues_[i].String.Data = new char[strlen(pstr2)+1];
							strcpy(t->fieldPropValues_[i].String.Data,(PSSTR)UTF8ToGBK(pstr2).c_str());
							t->fieldPropValues_[i].String.Data[strlen(pstr2)] = 0;
							t->fieldPropValues_[i].String.Length = strlen(t->fieldPropValues_[i].String.Data);
						}
						break;
					case  PSDATATYPE_STRING:
						{
							String::Utf8Value str2(fieldObj->Get(String::New(pstr1)));
							const char * pstr2 = ToCString(str2);
							t->fieldPropValues_[i].String.Data = new char[strlen(pstr2)+1];
							strcpy(t->fieldPropValues_[i].String.Data,(PSSTR)UTF8ToGBK(pstr2).c_str());
							t->fieldPropValues_[i].String.Data[strlen(pstr2)] = 0;
							t->fieldPropValues_[i].String.Length = strlen(t->fieldPropValues_[i].String.Data);
						}
						break;
					case PSDATATYPE_TIME:
						{
							V8DATE2PSTIME(&t->fieldPropValues_[i].Time,fieldObj->Get(String::New(pstr1)));
						}
						break;
					default:
						{
							Local<Object> errObj = Error::newObj();
							errObj->Set(String::New("code"),Number::New(-1));
							std::string *s =t->errString_;
							errObj->Set(String::New("errString"),String::New(GBK2UTF8("数据类型为空或有误!").c_str()));
							delete t;
							return errObj;
						}
					}
					t->filter_->FieldPropIds[i] = t->fieldPropids_[i];
					t->filter_->FieldPropValues[i] = t->fieldPropValues_[i];
				}
				t->filter_->FieldPropCount = t->fieldPropCnt_;
			}
		}else{
			t->filter_ = NULL;
			t->fieldPropValues_ = NULL;
			t->fieldPropCnt_=0;
			t->fieldPropids_ = NULL;
		}
		if (t->tagID_== PSTAGID_UNUSED)
		{
			t->code_ = -1;
			t->errString_ = new std::string("tag not found!");
			Local<Object> errObj = Error::newObj();
			errObj->Set(String::New("code"),Number::New(-1));
			errObj->Set(String::New("errString"),String::New("tag not found!"));
			return errObj;
		}
		
		uv_work_t* req = new uv_work_t();
		req->data = t;
		t->psNode_->Ref();
		queryWork(req);
		t->psNode_->Unref();
		//如果失败
		if(t->errString_) {
			Local<Object> errObj = Error::newObj();
			errObj->Set(String::New("code"),Number::New(t->code_));
			std:string *s =t->errString_;
			errObj->Set(String::New("errString"),String::New(GBK2UTF8(s->c_str()).c_str()));
			delete t;
			return errObj;
		}
		//成功返回数据
		Local<Array> arrObj = Array::New(t->tagCount_);
		//std::cout<<t->tagCount_<<std::endl;
			for (int i=0;i<t->tagCount_;i++)
			{
				if (t->propCount_>0)
				{
					assert((t->tagPropValues_+i)->PropCount == t->propCount_);
					arrObj->Set(i,getPropObj((t->tagPropValues_+i),propIdsArr));	
				}else{
					arrObj->Set(i,Undefined());
				}
			}
		
		return arrObj;
		delete t;
	} catch(PsException &ex) {
		return ThrowException(Exception::Error(String::New(ex.what())));
	}	
}
Handle<Value> PspaceNode::queryAsy(const Arguments& args)
{
	HandleScope scope;
	PspaceNode* ps = ObjectWrap::Unwrap<PspaceNode>(args.This());
	REQ_FUN_ARG(args.Length()-1, callback);
	TagQuery* t;
	try {
		t = new TagQuery(ps, &callback);
		REQ_STRING_ARG(0,tagName);
		String::Utf8Value nameStr(tagName);
		const char *pNameStr = ToCString(nameStr);
		std::string sName = pNameStr;
		if (strstr(str2Upper(pNameStr),"ROOT"))
		{
			t->tagID_ = PSTAGID_ROOT;
		}else{
			t->tagID_ = t->getTagID(UTF8ToGBK(replace_all(sName,"/","\\")).c_str(),t->psNode_->hHanle_);
			if (isExit(t->psNode_->hHanle_,t->tagID_))
			{
				Handle<Value> argv[2];
				argv[0] = Exception::Error(String::New(GBK2UTF8("起始测点不存在!").c_str()));
				argv[1] = Undefined();
				node::MakeCallback(Context::GetCurrent()->Global(), t->callback_, 2, argv);
			}
		}
		Local<Object> argObj = args[1]->ToObject();
		Local<Array> propIdsArr;
		if (argObj->Has(String::New("propIds")))
		{
			if (argObj->Get(String::New("propIds"))->IsNull())
			{
				t->propCount_ = 0;
			}else{
				propIdsArr  =Local<Array>::Cast(argObj->Get(String::New("propIds")));
				t->propCount_ = propIdsArr->Length();
				t->propIds_ = new PSUINT16[t->propCount_];
				t->propArr_ = new char *[t->propCount_];
				for(int i=0;i<t->propCount_;i++){
					String::Utf8Value str(propIdsArr->Get(i));
					const char *pstr = ToCString(str);
					t->propArr_[i] = new char[strlen(pstr)+1];
					strcpy(t->propArr_[i],pstr);
					t->propArr_[i][strlen(t->propArr_[i])]=0;
					t->propIds_[i] = t->getPropID(pstr,t->psNode_->hHanle_);
				}
			}
		}else{
			t->propCount_ = 0;
			t->propIds_ = NULL;
		}
		if(argObj->Has(String::New("filter")))
		{
			t->filter_ = (PS_TAG_QUERY_FILTER*)malloc(sizeof(PS_TAG_QUERY_FILTER));
			t->filter_->FieldPropCount = 0;
			t->filter_->QueryLevel = 0;
			t->filter_->Result_Num = 0;
			t->filter_->Result_Start = 0;
			if (argObj->Get(String::New("filter"))->IsNull())
			{
				t->filter_ = NULL;
				t->fieldPropValues_ = NULL;
				t->fieldPropCnt_=0;
				t->fieldPropids_ = NULL;
			}else{
				Local<Object> fieldObj = argObj->Get(String::New("filter"))->ToObject();
				if (fieldObj->Has(String::New("querySelf")))
				{
					t->filter_->QuerySelf = fieldObj->Get(String::New("querySelf"))->BooleanValue();
					fieldObj->Delete(String::New("querySelf"));
				}
				if (fieldObj->Has(String::New("queryLevel")))
				{
					t->filter_->QueryLevel = fieldObj->Get(String::New("queryLevel"))->Uint32Value();
					fieldObj->Delete(String::New("queryLevel"));
				}
				if (fieldObj->Has(String::New("resultStart")))
				{
					t->filter_->Result_Start = fieldObj->Get(String::New("resultStart"))->Uint32Value();
					fieldObj->Delete(String::New("resultStart"));
				}
				if (fieldObj->Has(String::New("resultNum")))
				{
					t->filter_->Result_Num = fieldObj->Get(String::New("resultNum"))->Uint32Value();
					fieldObj->Delete(String::New("resultNum"));
				}
				Handle<Array> filterFieldKeys = fieldObj->GetOwnPropertyNames();
				
				t->fieldPropCnt_ = filterFieldKeys->Length();
				t->fieldPropids_ = new PSUINT16[t->fieldPropCnt_];
				t->fieldPropValues_ = new PS_VARIANT[t->fieldPropCnt_];

				t->filter_->FieldPropValues = new PS_VARIANT[t->fieldPropCnt_];
				t->filter_->FieldPropIds = new PSUINT16[t->fieldPropCnt_];
				for (int i=0;i<t->fieldPropCnt_;i++)
				{
					String::Utf8Value keyStr(filterFieldKeys->Get(i));
					const char *pstr = ToCString(keyStr);
					t->fieldPropids_[i] = t->getPropID(pstr,t->psNode_->hHanle_);
					t->fieldPropValues_[i].DataType = t->getPropInfo(pstr,t->psNode_->hHanle_).DataType;
					String::Utf8Value str1(filterFieldKeys->Get(i));
					const char *pstr1 = ToCString(str1);
					switch(t->fieldPropValues_[i].DataType)
					{
					case PSDATATYPE_UINT16:
						{
							t->fieldPropValues_[i].UInt16 = (PSUINT16)fieldObj->Get(String::New(pstr1))->Uint32Value();	
						}
						break;
					case  PSDATATYPE_BOOL:
						{
							Local<Value> bo = fieldObj->Get(String::New(pstr1));
							PS_DATATYPE_ENUM b = (PS_DATATYPE_ENUM)bo->BooleanValue();
							t->fieldPropValues_[i].Bool = b;
						}
						break;
					case PSDATATYPE_DOUBLE:
						{
							Local<Value> db = fieldObj->Get(String::New(pstr1));
							double d = db->NumberValue();
							t->fieldPropValues_[i].Double = d;
						}
						break;
					case PSDATATYPE_INT16:
						{
							Local<Value> int16 = fieldObj->Get(String::New(pstr1));
							PS_DATATYPE_ENUM i16 = (PS_DATATYPE_ENUM)int16->Int32Value();
							t->fieldPropValues_[i].Int16 = i16;
						}
						break;
					case PSDATATYPE_INT32:
						{
							Local<Value> int32 = fieldObj->Get(String::New(pstr1));
							PSINT32 i32 = (PSINT32)(int32->Int32Value());
							t->fieldPropValues_[i].Int32 = i32;
						}
						break;
					case PSDATATYPE_UINT32:
						{
							Local<Value> int32 = fieldObj->Get(String::New(pstr1));
							PSUINT32 i32 = (PSUINT32)int32->Uint32Value();
							t->fieldPropValues_[i].UInt32 = i32;
						}
						break;
					case PSDATATYPE_INT64:
						{
							Local<Value> int64 = fieldObj->Get(String::New(pstr1));
							PS_DATATYPE_ENUM i64 = (PS_DATATYPE_ENUM)int64->Int32Value();
							t->fieldPropValues_[i].Int64 = i64;
						}
						break;
					case PSDATATYPE_UINT64:
						{
							Local<Value> uint64 = fieldObj->Get(String::New(pstr1));
							PS_DATATYPE_ENUM u64 = (PS_DATATYPE_ENUM)uint64->Uint32Value();
							t->fieldPropValues_[i].UInt64 = u64;
						}
						break;
					case PSDATATYPE_INT8:
						{
							Local<Value> int8 = fieldObj->Get(String::New(pstr1));
							PS_DATATYPE_ENUM i8 = (PS_DATATYPE_ENUM)int8->Int32Value();
							t->fieldPropValues_[i].Int8 = i8;
						}
						break;
					case PSDATATYPE_UINT8:
						{
							Local<Value> uint8 = fieldObj->Get(String::New(pstr1));
							PS_DATATYPE_ENUM u8 = (PS_DATATYPE_ENUM)uint8->Uint32Value();
							t->fieldPropValues_[i].UInt8 = u8;
						}
						break;
					case PSDATATYPE_FLOAT:
						{
							Local<Value> fl = fieldObj->Get(String::New(pstr1));
							float f = (float)fl->NumberValue();
							t->fieldPropValues_[i].Float = f;	
						}
						break;
					case  PSDATATYPE_WSTRING:
						{
							String::Utf8Value str2(fieldObj->Get(String::New(pstr1)));
							const char * pstr2 = ToCString(str2);
							t->fieldPropValues_[i].String.Data = new char[strlen(pstr2)+1];
							strcpy(t->fieldPropValues_[i].String.Data,(PSSTR)UTF8ToGBK(pstr2).c_str());
							t->fieldPropValues_[i].String.Data[strlen(pstr2)] = 0;
							t->fieldPropValues_[i].String.Length = strlen(t->fieldPropValues_[i].String.Data);
						}
						break;
					case  PSDATATYPE_STRING:
						{
							String::Utf8Value str2(fieldObj->Get(String::New(pstr1)));
							const char * pstr2 = ToCString(str2);
							t->fieldPropValues_[i].String.Data = new char[strlen(pstr2)+1];
							strcpy(t->fieldPropValues_[i].String.Data,(PSSTR)UTF8ToGBK(pstr2).c_str());
							t->fieldPropValues_[i].String.Data[strlen(t->fieldPropValues_[i].String.Data)] = 0;
							t->fieldPropValues_[i].String.Length = strlen(t->fieldPropValues_[i].String.Data);
						}
						break;
					case PSDATATYPE_TIME:
						{
							V8DATE2PSTIME(&t->fieldPropValues_[i].Time,fieldObj->Get(String::New(pstr1)));
						}
						break;
					default:
						{
							Handle<Value> argv[2];
							argv[0] = Exception::Error(String::New(GBK2UTF8("数据类型有错误!").c_str()));
							argv[1] = Undefined();
							node::MakeCallback(Context::GetCurrent()->Global(), t->callback_, 3, argv);
						}
					}
					t->filter_->FieldPropIds[i] = t->fieldPropids_[i];
					t->filter_->FieldPropValues[i] = t->fieldPropValues_[i];
				}
				t->filter_->FieldPropCount = t->fieldPropCnt_;
			}
		}else{
			t->filter_ = NULL;
			t->fieldPropValues_ = NULL;
			t->fieldPropCnt_ = 0;
			t->fieldPropids_ = NULL;
		}
		if (t->tagID_ == PSTAGID_UNUSED)
		{
			Handle<Value> argv[2];
			argv[0] = Exception::Error(String::New(GBK2UTF8("起始测点不存在!").c_str()));
			argv[1] = Undefined();
			node::MakeCallback(Context::GetCurrent()->Global(), t->callback_, 3, argv);
		}
	} catch(PsException &ex) {
		return scope.Close(ThrowException(Exception::Error(String::New(ex.what()))));
	}
	uv_work_t* req = new uv_work_t();
	req->data = t;
	uv_queue_work(uv_default_loop(), req, queryWork,(uv_after_work_cb)afterQueryWork);
	ps->Ref();
	//不可在此释放t,因为此代码先于uv_queue_work执行
	//delete t;
	return scope.Close(Undefined());
}
void PspaceNode::queryWork(uv_work_t* req)
{
	TagQuery* t = static_cast<TagQuery*>(req->data);
	t->code_  = PSRET_OK;
	t->errString_ = NULL;
	PSAPIStatus nRet  = PSRET_OK;
	PSAPIStatus *pAPIErrors = PSNULL;
	try {
		if(t->psNode_->hHanle_ == PSHANDLE_UNUSED) {
			throw PsException("Connection already closed");
		}else{
			nRet = psAPI_Tag_Query(t->psNode_->hHanle_, t->tagID_,t->filter_,
									t->propCount_, t->propIds_,&(t->tagCount_), &(t->tagPropValues_));
			if (PSERR(nRet))
			{
				t->code_ = nRet;
				t->errString_ = new std::string(GBK2UTF8(psAPI_Commom_GetErrorDesc(nRet)).c_str());
			}
		}	
	}catch(PsException &ex) {
		t->errString_ = new string(ex.what());
	} catch (const exception& ex) {
		t->errString_ = new string(ex.what());
	} catch (...) {
		t->errString_ = new string("Unknown Error");
	}
}



void PspaceNode::afterQueryWork(uv_work_t* req)
{
	
	HandleScope scope;
	TagQuery* bat = static_cast<TagQuery*>(req->data);
	bat->psNode_->Unref();
	try {
		if(bat->errString_){
			Handle<Value> argv[2];
			argv[0] = Exception::Error(String::New(bat->errString_->c_str()));
			argv[1] = Undefined();
			node::MakeCallback(Context::GetCurrent()->Global(), bat->callback_, 2, argv);
		}else{
									
			Handle<Value> argv[2];
			argv[0] = Undefined();
			Local<Array> arrObj = Array::New(bat->tagCount_);
			Local<Array> propsArr = Array::New(bat->propCount_);
			for (int i=0;i<bat->propCount_;i++)
			{
				propsArr->Set(i,v8::String::New(bat->propArr_[i]));
			}
			
			for (int i=0;i<bat->tagCount_;i++)
			{
				if (bat->propCount_>0)
				{
					assert((bat->tagPropValues_+i)->PropCount == bat->propCount_);
					arrObj->Set(i,getPropObj((bat->tagPropValues_+i),propsArr));	
				}else{
					arrObj->Set(i,Undefined());
				}
			}
			argv[1] = arrObj;
			if (bat->fieldPropids_!=NULL)
			{
				delete []bat->filter_->FieldPropIds;
				delete []bat->filter_->FieldPropValues;
			}
			node::MakeCallback(Context::GetCurrent()->Global(), bat->callback_, 2, argv);
		}
	} catch(PsException &ex) {
		Handle<Value> argv[2];
		argv[0] = Exception::Error(String::New(ex.what()));
		argv[1] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), bat->callback_, 2, argv);
	} catch(const exception &ex) {
		Handle<Value> argv[2];
		argv[0] = Exception::Error(String::New(GBK2UTF8(bat->errString_->c_str()).c_str()));
		argv[1] = Undefined();
		node::MakeCallback(Context::GetCurrent()->Global(), bat->callback_, 2, argv);
	}
	delete bat;
	
}

Handle<Value> SubChange::New(const Arguments& args)
{
	HandleScope scope;
	SubChange *t = new SubChange();
	t->Wrap(args.This());
	return scope.Close(args.This());
}
void SubChange::init(Handle<Object> target)
{
	HandleScope scope;
	//定义一个函数模板
	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	//constructorTemplate = Persistent<FunctionTemplate>::New(t);
	t->InstanceTemplate()->SetInternalFieldCount(1);

	t->SetClassName(String::NewSymbol("SubChange"));

	constructorTemplate= Persistent<Function>::New(t->GetFunction());
	target->Set(String::NewSymbol("change"),t->GetFunction());

	target->Set(String::NewSymbol("SubChange"), constructorTemplate);
}