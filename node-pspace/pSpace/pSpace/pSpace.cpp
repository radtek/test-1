#include <time.h>
#include "pSpace.h"

using namespace v8;

#define TRACE_LOG if(true) std::cout
#define ERROR_LOG if(true) std::cout


#include <Windows.h>

std::vector<pSpace*> pSpace::pSpace_;
std::list<pSpace::CallbackData> pSpace::s_CallbackData;
PSHANDLE pSpace::hHandle_ = PSHANDLE_UNUSED;

Persistent<Function> pSpace::constructor;
//将方法暴露出去给node.js
void pSpace::Init()
{  //译者：定义Init方法
	Isolate* isolate = Isolate::GetCurrent();
	// 准备构造函数模板
	Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
	pSpace *p = new pSpace();
	tpl->SetClassName(String::NewSymbol("pSpace"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Prototype
	tpl->PrototypeTemplate()->Set(String::NewSymbol("realsubscribe"),
		FunctionTemplate::New(RealSubscribe)->GetFunction());

	tpl->PrototypeTemplate()->Set(String::NewSymbol("realRead"),
		FunctionTemplate::New(RealRead)->GetFunction());

	tpl->PrototypeTemplate()->Set(String::NewSymbol("realWrite"),
		FunctionTemplate::New(RealWrite)->GetFunction());

	tpl->PrototypeTemplate()->Set(String::NewSymbol("log"),
		FunctionTemplate::New(Log)->GetFunction());

	tpl->PrototypeTemplate()->Set(String::NewSymbol("stoppSpace"),
		FunctionTemplate::New(StoppSpace)->GetFunction());

	tpl->PrototypeTemplate()->Set(String::NewSymbol("startpSpace"),
		FunctionTemplate::New(StartpSpace)->GetFunction());
	

		tpl->PrototypeTemplate()->Set(String::NewSymbol("getID"),
		FunctionTemplate::New(GetID)->GetFunction());

	/*tpl->PrototypeTemplate()->Set(String::NewSymbol("callback"),
	FunctionTemplate::New(OnRealData)->GetFunction());*/

	constructor = Persistent<Function>::New(tpl->GetFunction());
}
//创建对象
Handle<Value> pSpace::New(const Arguments& args) {
	
	TRACE_LOG<<"pSpace::CreatepSpace\n";
	pSpace *p = new pSpace();
	pSpace_.push_back(p);
	args.This()->SetPointerInInternalField(0, p);
	p->jsObj_ = Persistent<Object>::New(args.This());
	return args.This();
	
}

Handle<Value> pSpace::NewInstance(const Arguments& args)
{
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope;

	const unsigned argc = 1;
	Handle<Value> argv[argc] = { args[0] };
	Local<Object> instance = constructor->NewInstance(argc, argv);

	return scope.Close(instance);
}
//编码转换
void ConvertUtf8ToGBK(std::string& strUtf8) 
{
	int len=MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, NULL,0);
	unsigned short * wszGBK = new unsigned short[len+1];
	memset(wszGBK, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, (LPWSTR)wszGBK, len);

	len = WideCharToMultiByte(CP_ACP, 0, (LPWSTR)wszGBK, -1, NULL, 0, NULL, NULL); 
	char *szGBK=new char[len + 1];
	memset(szGBK, 0, len + 1);
	WideCharToMultiByte (CP_ACP, 0, (LPWSTR)wszGBK, -1, szGBK, len, NULL,NULL);

	strUtf8 = szGBK;
	delete[] szGBK;
	delete[] wszGBK;
}
//时间转换为字符串
char* PSTIME2STR(PS_TIME psTm)
{
	static char pszTime[256] = {0};
	time_t tt = psTm.Second;
	struct tm * ptm =  localtime(&(tt));
	sprintf(pszTime,"%04d-%02d-%02d %02d:%02d:%02d.%03d",ptm->tm_year+1900,
		ptm->tm_mon+1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec,psTm.Millisec);
	return pszTime;
}
//字符串转化为时间
void STR2PSTIME(PS_TIME* d, std::string s)
{
	if(s== "")
	{
		d->Millisec=0;
		d->Second=0;
		return;
	}
	d->Second = 0;
	d->Millisec = 0;
	//2008-05-12 14:28:04.123
	PSUINT32  nStrLen = s.length();
	d->Millisec=(PSUINT32)atoi(s.substr(20,3).c_str());
	tm tm_struct;
	tm_struct.tm_mon=atoi(s.substr(5,2).c_str())-1;
	tm_struct.tm_mday=atoi(s.substr(8,2).c_str());
	tm_struct.tm_hour=atoi(s.substr(11,2).c_str());
	tm_struct.tm_min=atoi(s.substr(14,2).c_str());
	tm_struct.tm_sec=atoi(s.substr(17,2).c_str());
	tm_struct.tm_year=atoi(s.substr(0,4).c_str())-1900;
	d->Second= (PSUINT32)mktime(&tm_struct);
}
//格式异常处理
std::string formatException(v8::TryCatch* try_catch)
{
	v8::HandleScope handle_scope;
	v8::String::Utf8Value exception(try_catch->Exception());
	v8::Handle<v8::Message> message = try_catch->Message();
	std::string msgstring = "";
	std::stringstream ss;

	if (message.IsEmpty()) {
		msgstring += *exception;
	} else {
		v8::String::Utf8Value filename(message->GetScriptResourceName());
		int linenum = message->GetLineNumber();
		msgstring += *exception;
		msgstring += " (";
		msgstring += *filename;
		msgstring += ":";
		ss << linenum;
		msgstring += ss.str();
		msgstring += ")";

		v8::Handle<v8::Value> stack = try_catch->StackTrace();
		if (!stack.IsEmpty()) {
			v8::String::Utf8Value sstack(stack);
			msgstring += "\n";
			msgstring += *sstack;
		}
}
return msgstring;
}


//将订阅到的数据转化为node.js支持的数据OnRealData(PSUINT32 nTagID, double db)
void pSpace::OnRealData(PSUINT32 nTagID, double db)
{
	if (OnData.IsEmpty())
	{
		return ;
	}
	
	
	std::string str = GetTagName(nTagID);
	if (str.empty())
	{
		return;
	}
	
	Local<Value> strTag = String::New(str.c_str());
	Local<Value> dbVal = Number::New(db);
	Handle<Value> args[] = {jsObj_,strTag, dbVal};
	//异步回调执行OnData
	OnData->Call(contextObj_, 3, args);
	
}

//添加
Handle<Value> pSpace::GetID(const Arguments& args)
{
	HandleScope scope;
	PSAPIStatus nRet = PSRET_OK;
	PSUINT32 nTagID = PSTAGID_UNUSED;

	//String::Utf8Value names(args[0]);
	//const char * pszName = ToCString(names);

	String::Utf8Value str(args[0]);
	std::string pszName = *str; 
	ConvertUtf8ToGBK(pszName);
	//std::map<std::string, PSUINT32>::iterator iter =  m_TagName2ID.find(pszName);


	nRet = psAPI_Tag_GetIdByLongName(hHandle_, (PSSTR)pszName.c_str(), &nTagID);
	if (PSERR(nRet))
	{
		return Boolean::New(false);
	}

	return scope.Close(Number::New(nTagID));
}

//实时订阅
Handle<Value> pSpace::RealSubscribe(const Arguments& args)
{
	if (!args[0]->IsString())
	{
		TRACE_LOG<<"pSpace::RealSubscribe error\n";
		return Boolean::New(false);
	}
	pSpace *p = reinterpret_cast<pSpace*>(args.This()->GetPointerFromInternalField(0));
	String::Utf8Value str(args[0]);
	std::string strUtf = *str; 
	ConvertUtf8ToGBK(strUtf);
	TRACE_LOG<<"RealSubscribe "<<strUtf<<"\n";
	bool bl = p->RealSubscribe_i(strUtf.c_str());
	return Boolean::New(bl);
}
//Handle<Value> pSpace::SubscribeData(const Arguments& args)
//{
//
//}
Handle<Value> pSpace::RealRead(const Arguments& args)
{
	if (!args[0]->IsString())
	{
		TRACE_LOG<<"pSpace::RealRead error\n";
		return Boolean::New(false);
	}
	pSpace *p = reinterpret_cast<pSpace*>(args.This()->GetPointerFromInternalField(0));
	String::Utf8Value str(args[0]);
	std::string strUtf = *str; 
	ConvertUtf8ToGBK(strUtf);
	
	double db = 0;
	PSUINT32 quality = 0;
	PS_TIME time;
	bool bl = p->RealRead_i(strUtf.c_str(), db,quality,time);
	if (bl)
	{
		//数组对象元素个数
		Handle<Array> array = Array::New(4);
		//创建数组对象
		Local<Object> obj = Object::New();
		//向数组对象中添加元素
		obj->Set(String::New("tagName"),args[0]);
		obj->Set(String::New("value"),Number::New(db));
		obj->Set(String::New("quality"),Int32::New(quality));
		obj->Set(String::New("time"),String::New(PSTIME2STR(time)));
		array->Set(0,obj);
		return array; //将数组输出给node端
	}
	return Undefined();
}

Handle<Value> pSpace::RealWrite(const Arguments& args)
{
	if (args.Length() != 2 || !args[0]->IsString() || !args[1]->IsNumber())
	{
		TRACE_LOG<<"pSpace::RealWrite error\n";
		return Boolean::New(false);
	}
	pSpace *p = reinterpret_cast<pSpace*>(args.This()->GetPointerFromInternalField(0));
	String::Utf8Value str(args[0]);
	std::string strUtf = *str; 
	ConvertUtf8ToGBK(strUtf);
	bool bl = p->RealWrite_i(strUtf.c_str(), args[1]->ToNumber()->Value());
	return Boolean::New(bl);
}

Handle<Value> pSpace::Log(const Arguments& args)
{
	String::Utf8Value str(args[0]);
	std::string strUtf = (*str)?*str:"(null)";
	ConvertUtf8ToGBK(strUtf);
	ERROR_LOG<<strUtf<<"\n";
	return args.This();
}
	

Handle<Value> pSpace::StartpSpace(const Arguments& args)
{
	if ( args.Length()<3 &&!args[0]->IsString() && !args[1]->IsString() && args[2]->IsString())
	{
		TRACE_LOG<<"pSpace::RealWrite error\n";
		return Boolean::New(false);
	}
	String::Utf8Value server(args[0]);
	std::string pszServer = *server; 
	ConvertUtf8ToGBK(pszServer);

	String::Utf8Value userName(args[1]);
	std::string pszUserName = *userName; 
	ConvertUtf8ToGBK(pszUserName);

	String::Utf8Value passwd(args[2]);
	std::string pszPasswd = *passwd; 
	ConvertUtf8ToGBK(pszPasswd);


	psAPI_Common_StartAPI();
	PSHANDLE hServer = PSHANDLE_UNUSED;
	PSAPIStatus nRet = psAPI_Server_Connect((PSSTR)pszServer.c_str(), 
											(PSSTR)pszUserName.c_str(), 
											(PSSTR)pszPasswd.c_str(), 
											&hServer);
	if (PSERR(nRet))
	{
		ERROR_LOG<<"psAPI_Server_Connect 错误:"<<psAPI_Commom_GetErrorDesc(nRet)<<"\n";
		return Boolean::New(false);
	}
	else
	{
		TRACE_LOG<<"psAPI_Server_Connect 成功\n";
		hHandle_ = hServer;
		return Boolean::New(true);
	}
}
Handle<Value> pSpace::StoppSpace(const Arguments& args)
{
	PSAPIStatus nRet = psAPI_Server_Disconnect(hHandle_);
	if (PSERR(nRet))
	{
		ERROR_LOG<<"psAPI_Server_Disconnect 错误:"<<psAPI_Commom_GetErrorDesc(nRet)<<"\n";
		return Boolean::New(false);
	}
	else
	{
		psAPI_Common_StopAPI();
		return Boolean::New(true);
	}
		
}

PSUINT32 pSpace::GetTagID(const char* pszTagName)
{
	std::map<std::string, PSUINT32>::iterator iter = m_TagName2ID.find(pszTagName);
	if (iter != m_TagName2ID.end())
	{
		return iter->second;
	}
	PSUINT32 nTagID = PSTAGID_UNUSED;
	PSAPIStatus nRet = psAPI_Tag_GetIdByLongName(hHandle_, (PSSTR)pszTagName, &nTagID);
	if (PSERR(nRet))
	{
		return PSTAGID_UNUSED;
	}
	m_TagName2ID[pszTagName] = nTagID;
	m_ID2TagName[nTagID] = pszTagName;
	return nTagID;
}
std::string pSpace::GetTagName(PSUINT32 nTagID)
{
	std::map<PSUINT32, std::string>::iterator iter = m_ID2TagName.find(nTagID);
	if (iter != m_ID2TagName.end())
	{
		return iter->second;
	}
	return "";
}
bool pSpace::RealSubscribe_i(const char* pszTagName)
{
	PSUINT32 nTagID = GetTagID(pszTagName);
	if (nTagID==PSTAGID_UNUSED)
	{
		return false;
	}
	PSAPIStatus *pAPIErrors = PSNULL;
	PSAPIStatus nRet = PSRET_OK;
	if (subscribeID_==-1)
	{
		nRet = psAPI_Real_NewSubscribe(hHandle_, 1, &nTagID,
			Real_CallbackFunction, (PSVOID*)this, &subscribeID_, &pAPIErrors);
	}
	else
	{
		nRet = psAPI_Real_AddSubscribe(hHandle_, subscribeID_, 1, &nTagID, &pAPIErrors);
	}
	if(PSERR(nRet))
	{
		if (nRet == PSERR_FAIL_IN_BATCH)
		{
			psAPI_Memory_FreeAndNull((PSVOID**)pAPIErrors);
		}
		return false;
	}
	return true;
}
bool pSpace::RealWrite_i(const char* pszTagName, double db)
{
	PS_VARIANT realData;
	realData.DataType = PSDATATYPE_DOUBLE;
	realData.Double = db;

	PSUINT32 nTagID = GetTagID(pszTagName);
	if (nTagID==PSTAGID_UNUSED)
	{
		return false;
	}

	PSAPIStatus nRet = psAPI_Real_Write(hHandle_, nTagID, &realData, PSNULL, PSNULL);
	if (PSERR(nRet))
	{
		return false;
	}
	return true;
}
bool pSpace::RealRead_i(const char* pszTagName, double& db,PSUINT32& quality,PS_TIME& time)
{
	PS_DATA *pRealData = PSNULL;
	PSUINT32 nTagID = GetTagID(pszTagName);
	if (nTagID==PSTAGID_UNUSED)
	{
		return false;
	}
	PSAPIStatus nRet = psAPI_Real_Read(hHandle_, nTagID, &pRealData);
	if (PSERR(nRet))
	{
		return false;
	}
	db = PS_VARIANT2DOUBLE(pRealData->Value);
	quality = pRealData->Quality;
	time  = pRealData->Time;
	psAPI_Memory_FreeDataList(&pRealData, 1);
	return true;
}
PSVOID PSAPI pSpace::Real_CallbackFunction(
	PSIN PSHANDLE hServer,
	PSIN PSUINT32 nSubscribeId,
	PSIN PSVOID *pUserPara,
	PSIN PSUINT32 nCount,
	PSIN PSUINT32 *pTagIds,
	PSIN PS_DATA *pRealDataList
	)
{
	pSpace* p = (pSpace*)pUserPara;
	for (int n = 0; n < nCount; n++)
	{
		//p->OnRealData(pTagIds[n], PS_VARIANT2DOUBLE(pRealDataList[n].Value));
		CallbackData cd = {p, pTagIds[n], PS_VARIANT2DOUBLE(pRealDataList[n].Value)};
		s_CallbackData.push_back(cd);
	}
}
//把PS_VARIANT 转换为DOUBLE字符串
double pSpace::PS_VARIANT2DOUBLE(const PS_VARIANT &var)
{
	double dbRet = 0;
	switch (var.DataType)
	{
	case PSDATATYPE_BOOL: 
		dbRet = !!var.Bool;
		break;
	case PSDATATYPE_INT8 :
		dbRet = var.Int8;
		break;
	case PSDATATYPE_UINT8 :	
		dbRet = var.UInt8;
		break;
	case PSDATATYPE_INT16 :
		dbRet = var.Int16;
		break;
	case PSDATATYPE_UINT16 :
		dbRet = var.UInt16;
		break;
	case PSDATATYPE_INT32 :	
		dbRet = var.Int32;
		break;
	case PSDATATYPE_UINT32 :
		dbRet = var.UInt32;
		break;
	case PSDATATYPE_INT64 :
		dbRet = var.Int64;
		break;
	case PSDATATYPE_UINT64 :
		dbRet = var.UInt64;
		break;
	case PSDATATYPE_FLOAT :
		dbRet = var.Float ;
		break;
	case PSDATATYPE_DOUBLE :
		dbRet = var.Double;
		break;
	case PSDATATYPE_TIME :		
	case PSDATATYPE_STRING:
	case PSDATATYPE_WSTRING:
	case PSDATATYPE_BLOB:
	default:
		break;
	}
	return dbRet;
}
