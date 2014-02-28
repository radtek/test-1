#include "utils.h"
#include <time.h>

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

bool fun(const Arguments& args)
{
	int length = args.Length();
	while(length){
		if(args[length-1]->IsFunction())
			return true;
		length--;
	}
	return false;
}

const char* ToCString(const String::Utf8Value& value)
{
	return *value ? *value: "string conversion failed";
}

string&   replace_all(std::string& str,const string&  old_value,const  string&  new_value)     
{     
	while(true)   {     
		string::size_type   pos(0);     
		if(   (pos=str.find(old_value))!=string::npos   )     
			str.replace(pos,old_value.length(),new_value);     
		else   break;     
	}     
	return   str;     
}  

std::vector<std::string> split(std::string str,std::string pattern)
{
	std::string::size_type pos;
	std::vector<std::string> result;

	str+=pattern;//扩展字符串以方便操作
	int size=str.size();

	for(int i=0; i<size; i++)
	{
		pos=str.find(pattern,i);
		if(pos<size)
		{
			std::string s=str.substr(i,pos-i);
			result.push_back(s);
			i=pos+pattern.size()-1;
		}
	}
	return result;
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

char* PSVARIANT2STRHELP(char* pszVariant, PS_VARIANT *pVariant)
{
	PS_VARIANT *var = pVariant;
	char *temp = pszVariant;
	struct tm * ptm = PSNULL;
	memset(temp, 0, 256);
	switch (var->DataType)
	{
	case PSDATATYPE_BOOL: 
		strcpy(temp,var->Bool==PSTRUE? "1":"0");
		break;
	case PSDATATYPE_INT8 :
		sprintf(temp,"%d",var->Int8);
		break;
	case PSDATATYPE_UINT8 :    
		sprintf(temp,"%d",var->UInt8);
		break;
	case PSDATATYPE_INT16 :
		sprintf(temp,"%d",var->Int16);
		break;
	case PSDATATYPE_UINT16 :
		sprintf(temp,"%d",var->UInt16);
		break;
	case PSDATATYPE_INT32 :        
		sprintf(temp,"%d",var->Int32);
		break;
	case PSDATATYPE_UINT32 :
		sprintf(temp,"%d",var->UInt32);
		break;
	case PSDATATYPE_INT64 :
		sprintf(temp,"%lld",var->Int64);
		break;
	case PSDATATYPE_UINT64 :
		sprintf(temp,"%llu",var->UInt64);
		break;
	case PSDATATYPE_FLOAT :
		sprintf(temp, "%.4f", var->Float );
		break;
	case PSDATATYPE_DOUBLE :
		sprintf(temp,"%.4f",var->Double);
		break;
	case PSDATATYPE_TIME :  
		{
			ptm=localtime((time_t*)&(var->Time.Second));    sprintf(temp,"%04d-%02d-%02d%02d:%02d:%02d.%03d",ptm->tm_year+1900,ptm->tm_mon+1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec,var->Time.Millisec);
			break;
		}
	case PSDATATYPE_STRING:
		strncpy(temp, var->String.Data, var->String.Length);
		break;
	default:
		strcpy(temp, "");
	}
	return temp;
}

char* GBKToUtf8(const char* strGBK)
{ 
	int len=MultiByteToWideChar(CP_ACP, 0, (LPCCH)strGBK, -1, NULL,0); 
	unsigned short * wszUtf8 = new unsigned short[len+1]; 
	memset(wszUtf8, 0, len * 2 + 2); 
	MultiByteToWideChar(CP_ACP, 0, (LPCCH)strGBK, -1, (LPWSTR)wszUtf8, len); 
	len = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)wszUtf8, -1, NULL, 0, NULL, NULL); 
	char *szUtf8=new char[len + 1]; 
	memset(szUtf8, 0, len + 1); 
	WideCharToMultiByte (CP_UTF8, 0, (LPCWSTR)wszUtf8, -1, (LPSTR)szUtf8, len, NULL,NULL);
	return szUtf8; 
}

PS_TAG_PROP_ENUM finType(map<std::string,PS_TAG_PROP_ENUM> &m,std::string key)
{
	if (m.size()<=0)
	{
		return (PS_TAG_PROP_ENUM)-1;
	}
	map <std::string,PS_TAG_PROP_ENUM>::iterator iter;
	iter = m.begin();
	iter = m.find(key); 
	if (iter !=m.end()) {  
		return iter->second; 
	} else { 
		return (PS_TAG_PROP_ENUM)-1; 
	}
}

enum __PS_DATATYPE_ENUM findDataType(map<std::string,PS_DATATYPE_ENUM> &m,std::string key)
{
	if(m.size()<=0){
		return (PS_DATATYPE_ENUM)-1;
	}
	map <std::string,PS_DATATYPE_ENUM>::iterator iter;
	iter = m.begin();
	iter = m.find(key); 
	if (iter !=m.end()) {  
		//std::cout<<"unil:"<<iter->second<<std::endl;
		return iter->second; 
	} else { 
		return (PS_DATATYPE_ENUM)-1; 
	}
}

void UnicodeToUTF_8(char* pOut,wchar_t* pText)   
{   
	// 注意 WCHAR高低字的顺序,低字节在前，高字节在后   
	char* pchar = (char *)pText;   
	pOut[0] = (0xE0 | ((pchar[1] & 0xF0) >> 4));  
	pOut[1] = (0x80 | ((pchar[1] & 0x0F) << 2)) + ((pchar[0] & 0xC0) >> 6);  
	pOut[2] = (0x80 | (pchar[0] & 0x3F));  
}   

void Gb2312ToUnicode(wchar_t* pOut,char *gbBuffer)   
{     
	::MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,gbBuffer,2,pOut,1);  
}   

string UTF8ToGBK(const std::string& strUTF8)  
{  
	int len = MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, NULL, 0);  
	unsigned short * wszGBK = new unsigned short[len + 1];  
	memset(wszGBK, 0, len * 2 + 2);  
	MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)strUTF8.c_str(), -1, (LPWSTR)wszGBK, len);  

	len = WideCharToMultiByte(CP_ACP, 0, (LPWSTR)wszGBK, -1, NULL, 0, NULL, NULL);  
	char *szGBK = new char[len + 1];  
	memset(szGBK, 0, len + 1);  
	WideCharToMultiByte(CP_ACP,0, (LPWSTR)wszGBK, -1, szGBK, len, NULL, NULL);  
	//strUTF8 = szGBK;  
	std::string strTemp(szGBK);  
	delete[]szGBK;  
	delete[]wszGBK;  
	return strTemp;  
} 

PS_QUALITY_ENUM findQuality(map<std::string,PS_QUALITY_ENUM> &m,std::string key)
{
	if(m.size()<=0){
		return (PS_QUALITY_ENUM)-1;
	}
	map <std::string,PS_QUALITY_ENUM>::iterator iter;
	iter = m.begin();
	iter = m.find(key); 
	if (iter !=m.end()) {  
		//std::cout<<"unil:"<<iter->second<<std::endl;
		return iter->second; 
	} else { 
		return (PS_QUALITY_ENUM)-1; 
	}

}

PS_HIS_AGGREGATE_ENUM findAggregate(map<std::string,PS_HIS_AGGREGATE_ENUM> &m,std::string key)
{
	if(m.size()<=0){
		return (PS_HIS_AGGREGATE_ENUM)-1;
	}
	map <std::string,PS_HIS_AGGREGATE_ENUM>::iterator iter;
	iter = m.begin();
	iter = m.find(key); 
	if (iter !=m.end()) {  
		//std::cout<<"unil:"<<iter->second<<std::endl;
		return iter->second; 
	} else { 
		return (PS_HIS_AGGREGATE_ENUM)-1; 
	}

}