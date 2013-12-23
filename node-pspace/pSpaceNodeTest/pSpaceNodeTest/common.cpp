#include "common.h"

using namespace std;
using namespace v8;

//将node.js类型字符串转化为C++可用字符串
const char* ToCString(const String::Utf8Value& value)
{
	return *value ? *value: "string conversion failed";
}

	//把PS_VARIANT 转换为DOUBLE字符串
double PS_VARIANT2DOUBLE(const PS_VARIANT &var)
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