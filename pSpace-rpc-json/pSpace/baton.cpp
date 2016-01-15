#include "baton.h"
#include "pSpaceNode.h"
#define PSVARIANT2STR(pVariant) PSVARIANT2STRHELP((char*)alloca(256),pVariant)

class DataBuffer
{
public:
    DataBuffer(){};
    ~DataBuffer(){
        std::map<std::string,PS_TAG_PROP_INFO>::iterator it = propName2PropInfo.begin();
        for (;it!=propName2PropInfo.end();++it)
        {
            delete []it->second.Name;
            delete []it->second.Desc;
        }
        propName2PropInfo.clear();
    }
std::map<std::string,PS_TAG_PROP_INFO> propName2PropInfo;
protected:
private:
};
//std::list<SubBaton::CallbackData> SubBaton::callbackData_;
std::map<PSUINT32,list<SubBaton::CallbackData>> SubBaton::mapData_;
std::map<PSUINT32,list<SubBaton::CallbackProp>> SubBaton::mapPropData_;
//SubBaton* SubBaton::this_ = NULL;

//std::map<PSUINT32,SubBaton::CallbackOtherProp> SubBaton::propsOther_;
//std::list<char*> SubBaton::callbackProps_;

std::map<PSUINT32, std::string> Baton::id2TagName_;
std::map<std::string, PSUINT32> Baton::tagName2ID_;

//std::map<std::string,PS_TAG_PROP_ENUM> Tag::tag;
//std::map<std::string,PS_DATATYPE_ENUM> Tag::dataTypeMap_;
//std::map<std::string,PS_QUALITY_ENUM> TagHis::qualityMap_;
//std::map<std::string,PS_HIS_AGGREGATE_ENUM> TagHis::aggregates_;
std::map<std::string,PSUINT16> propName2ID;

std::map<std::string,PS_DATATYPE_ENUM> dataTypeMap;
std::map<std::string,PS_QUALITY_ENUM> qualityMap;
std::map<std::string,PS_HIS_AGGREGATE_ENUM> aggregateMap;
std::map<std::string,PS_DATATYPE_ENUM> name2DataType;
DataBuffer dataBuffer;
//std::map<std::string, PSUINT32> tagName2ID_;
//std::map<PSUINT32, std::string> id2TagName_;
PSUINT32 Baton::getTagID(const char* pszTagName,PSHANDLE h)
{
	std::map<std::string, PSUINT32>::iterator iter = tagName2ID_.find(pszTagName);
	if (iter != tagName2ID_.end())
	{
		return iter->second;
	}
	PSUINT32 nTagID = PSTAGID_UNUSED;
	PSAPIStatus nRet = psAPI_Tag_GetIdByLongName(h, (PSSTR)pszTagName, &nTagID);
	if (PSERR(nRet))
	{
		return PSTAGID_UNUSED;
	}
	tagName2ID_[pszTagName] = nTagID;
	id2TagName_[nTagID] = pszTagName;
	return nTagID;
}

PSUINT32** Baton::getTagIDList(PSSTR *tagNames,PSHANDLE h,int tagCount)
{
	PSAPIStatus nRet = PSRET_OK;
	PSAPIStatus *pAPIErrors = PSNULL;
	PSUINT32 *pTagIds = PSNULL;
	nRet = psAPI_Tag_GetIdListByLongName(h, tagCount, tagNames, &pTagIds, &pAPIErrors);

    if (PSERR(nRet) && nRet != PSERR_FAIL_IN_BATCH)
    {
        return &pTagIds;
    } 
    if (nRet == PSERR_FAIL_IN_BATCH)
    {
        for (int n = 0; n < tagCount; n++)
        {
            string strtrmp = tagNames[n];
            strtrmp.append(psAPI_Commom_GetErrorDesc(pAPIErrors[n]));
            //printf("\t测点:%s 返回 %s\n", tagNames[n], psAPI_Commom_GetErrorDesc(pAPIErrors[n]));
        }
        psAPI_Memory_FreeAndNull((PSVOID**)&pAPIErrors);
        return &pTagIds;
    }
	for (int i=0;i<tagCount;i++)
	{
		id2TagName_[pTagIds[i]] = tagNames[i];
	}
	return &pTagIds;
}

std::string Baton::getTagName(PSUINT32 nTagID)
{
	std::map<PSUINT32, std::string>::iterator iter = id2TagName_.find(nTagID);
	if (iter != id2TagName_.end())
	{
		return iter->second;	
	}
	return "";
}

PSUINT16 Baton::getPropID(const char* pszPropName,PSHANDLE h)
{
	std::map<std::string,PSUINT16>::iterator iter = propName2ID.find(str2Upper(pszPropName));
	if (iter !=propName2ID.end())
	{
		return iter->second;
	}
	PSAPIStatus nRet = PSRET_OK;
	PSUINT16 nPropId = PSPROPID_UNUSED; 
	nRet = psAPI_TagType_GetId(h, (PSSTR)pszPropName, &nPropId);
	if (PSERR(nRet))
	{
		return PSPROPID_UNUSED;

	}
	propName2ID[str2Upper(pszPropName)] = nPropId;
	return nPropId;
}
PS_TAG_PROP_INFO Baton::getPropInfo(const char* pszPropName,PSHANDLE h)
{
	std::map<std::string,PS_TAG_PROP_INFO>::iterator iter = dataBuffer.propName2PropInfo.find(str2Upper(pszPropName));
	if (iter != dataBuffer.propName2PropInfo.end())
	{
		return iter->second;
	}
	PSAPIStatus nRet = PSRET_OK;
	PSUINT16 nTagType = PSTAGTYPE_ALL;
	PSUINT32 nTagPropCount = 0;
	PS_TAG_PROP_INFO *pTagPropInfos = PSNULL;
	PSUINT32 n = 0;
	nRet = psAPI_TagType_GetPropList(h, nTagType, &nTagPropCount, &pTagPropInfos);
	if (PSERR(nRet))
	{
		PS_TAG_PROP_INFO pro;
		pro.Id = PSPROPID_UNUSED;
		return pro;
	}
	PSUINT16 propID = getPropID(str2Upper(pszPropName),h);
	for (n = 0; n < nTagPropCount; n++)
	{
		if (propID==(pTagPropInfos+n)->Id)
		{
			PS_TAG_PROP_INFO prop;
			prop.Id = (pTagPropInfos+n)->Id;
			prop.DataType = (pTagPropInfos+n)->DataType;
			prop.Name= new char[strlen((pTagPropInfos+n)->Name)+1];
			strcpy(prop.Name,(pTagPropInfos+n)->Name);
			prop.Name[strlen(prop.Name)] = 0;
			prop.Desc = new char[strlen((pTagPropInfos+n)->Desc)+1];
			strcpy(prop.Desc,(pTagPropInfos+n)->Desc);
			prop.Desc[strlen(prop.Name)]=0;
			prop.ReadOnly = (pTagPropInfos+n)->ReadOnly;
			dataBuffer.propName2PropInfo[str2Upper(pszPropName)] = prop;
			return prop;
		}
	}
	psAPI_Memory_FreeTagPropInfoList(&pTagPropInfos, nTagPropCount);
}

PSUINT32 Baton::getParentID(std::vector<std::string> &v,PSHANDLE h)
{
	PSUINT32 parentID = PSTAGID_UNUSED;
	int len = v.size();
	for (int i=0;i<len-1;i++)
	{
		v[i] = "/"+v[i];

	}
	std::string com = "";
	for(int j =1;j<len-1;j++)
	{
		com+=v[j];
	}
	if (com=="")
	{
		parentID = PSTAGID_ROOT;
	}else
	{
		parentID= getTagID(UTF8ToGBK(replace_all(com,"/","\\")).c_str(),h);
		
	}
	return parentID;
}

PSUINT16 Baton::getTagType(const char* pszTagTypeName)
{
	if (strstr(str2Upper(pszTagTypeName),"NODE"))
	{
		return 0;
	}
	if (strstr(str2Upper(pszTagTypeName),"DIGITAL"))
	{
		return 1;
	}
	if (strstr(str2Upper(pszTagTypeName),"ANALOG"))
	{
		return 2;
	}
	if (strstr(str2Upper(pszTagTypeName),"STRING"))
	{
		return 3;
	}
	return -1;
}

PS_DATATYPE_ENUM Baton::getDataType(const char* key)
{
	std::map<std::string,PS_DATATYPE_ENUM>::iterator iter = dataTypeMap.find(str2Upper(key));
	if (iter != dataTypeMap.end())
	{
		return iter->second;
	}
	if (strstr(str2Upper(key),"EMPTY"))
	{
		dataTypeMap["EMPTY"] = PSDATATYPE_EMPTY;
		return PSDATATYPE_EMPTY;
	}
	if (strstr(str2Upper(key),"BOOL"))
	{
		dataTypeMap["BOOL"] = PSDATATYPE_BOOL;
		return PSDATATYPE_BOOL;
	}
	if (strstr(str2Upper(key),"INT8"))
	{
		dataTypeMap["INT8"] = PSDATATYPE_INT8;
		return PSDATATYPE_INT8;
	}
	if (strstr(str2Upper(key),"UINT8"))
	{
		dataTypeMap["UINT8"] = PSDATATYPE_UINT8;
		return PSDATATYPE_UINT8;
	}
	if (strstr(str2Upper(key),"INT16"))
	{
		dataTypeMap["INT16"] = PSDATATYPE_INT16;
		return PSDATATYPE_INT16;
	}	
	if (strstr(str2Upper(key),"UINT16"))
	{
		dataTypeMap["UINT16"] = PSDATATYPE_UINT16;
		return PSDATATYPE_UINT16;
	}		
	if (strstr(str2Upper(key),"INT32"))
	{
		dataTypeMap["INT32"] = PSDATATYPE_INT32;
		return PSDATATYPE_INT32;
	}	
	if (strstr(str2Upper(key),"UINT32"))
	{
		dataTypeMap["UINT32"] = PSDATATYPE_UINT32;
		return PSDATATYPE_UINT32;
	}			
	if (strstr(str2Upper(key),"INT64"))
	{
		dataTypeMap["INT64"] = PSDATATYPE_INT64;
		return PSDATATYPE_INT64;
	}	
	if (strstr(str2Upper(key),"UINT64"))
	{
		dataTypeMap["UINT64"] = PSDATATYPE_UINT64;
		return PSDATATYPE_UINT64;
	}				
	if (strstr(str2Upper(key),"FLOAT"))
	{
		dataTypeMap["FLOAT"] = PSDATATYPE_FLOAT;
		return PSDATATYPE_FLOAT;
	}	
	if (strstr(str2Upper(key),"DOUBLE"))
	{
		dataTypeMap["DOUBLE"] = PSDATATYPE_DOUBLE;
		return PSDATATYPE_DOUBLE;
	}	
	if (strstr(str2Upper(key),"TIME"))
	{
		dataTypeMap["TIME"] = PSDATATYPE_TIME;
		return PSDATATYPE_TIME;
	}			
	if (strstr(str2Upper(key),"STRING"))
	{
		dataTypeMap["STRING"] = PSDATATYPE_STRING;
		return PSDATATYPE_STRING;
	}		
	if (strstr(str2Upper(key),"WSTRING"))
	{
		dataTypeMap["WSTRING"] = PSDATATYPE_WSTRING;
		return PSDATATYPE_WSTRING;
	}			
	if (strstr(str2Upper(key),"BLOB"))
	{
		dataTypeMap["BLOB"] = PSDATATYPE_BLOB;
		return PSDATATYPE_BLOB;
	}
}

PS_QUALITY_ENUM Baton::getQuality(const char* key)
{
	std::map<std::string,PS_QUALITY_ENUM>::iterator iter = qualityMap.find(str2Upper(key));
	if (iter != qualityMap.end())
	{
		return iter->second;
	}
	if (strstr(str2Upper(key),"MASK"))
	{
		qualityMap["MASK"] = PS_QUALITY_MASK;
		return PS_QUALITY_MASK;
	}
	if (strstr(str2Upper(key),"BAD"))
	{
		qualityMap["BAD"] = PS_QUALITY_BAD;
		return PS_QUALITY_BAD;
	}
	if (strstr(str2Upper(key),"UNCERTAIN"))
	{
		qualityMap["UNCERTAIN"] = PS_QUALITY_UNCERTAIN;
		return PS_QUALITY_UNCERTAIN;
	}
	if (strstr(str2Upper(key),"NA"))
	{
		qualityMap["NA"] = PS_QUALITY_NA;
		return PS_QUALITY_NA;
	}
	if (strstr(str2Upper(key),"GOOD"))
	{
		qualityMap["GOOD"] = PS_QUALITY_GOOD;
		return PS_QUALITY_GOOD;
	}
	if (strstr(str2Upper(key),"SUBSTATUS_MASK"))
	{
		qualityMap["SUBSTATUS_MASK"] = PS_QUALITY_SUBSTATUS_MASK;
		return PS_QUALITY_SUBSTATUS_MASK;
	}
	if (strstr(str2Upper(key),"CONFIG_ERROR"))
	{
		qualityMap["CONFIG_ERROR"] = PS_QUALITY_CONFIG_ERROR;
		return PS_QUALITY_CONFIG_ERROR;
	}					
	if (strstr(str2Upper(key),"NOT_CONNECTED"))
	{
		qualityMap["NOT_CONNECTED"] = PS_QUALITY_NOT_CONNECTED;
		return PS_QUALITY_NOT_CONNECTED;
	}	
	if (strstr(str2Upper(key),"DEVICE_FAILURE"))
	{
		qualityMap["DEVICE_FAILURE"] = PS_QUALITY_DEVICE_FAILURE;
		return PS_QUALITY_DEVICE_FAILURE;
	}		
	if (strstr(str2Upper(key),"SENSOR_FAILURE"))
	{
		qualityMap["SENSOR_FAILURE"] = PS_QUALITY_SENSOR_FAILURE;
		return PS_QUALITY_SENSOR_FAILURE;
	}		
	if (strstr(str2Upper(key),"LAST_KNOWN"))
	{
		qualityMap["LAST_KNOWN"] = PS_QUALITY_LAST_KNOWN;
		return PS_QUALITY_LAST_KNOWN;
	}			
	if (strstr(str2Upper(key),"COMM_FAILURE"))
	{
		qualityMap["COMM_FAILURE"] = PS_QUALITY_COMM_FAILURE;
		return PS_QUALITY_COMM_FAILURE;
	}		
	if (strstr(str2Upper(key),"OUT_OF_SERVICE"))
	{
		qualityMap["OUT_OF_SERVICE"] = PS_QUALITY_OUT_OF_SERVICE;
		return PS_QUALITY_OUT_OF_SERVICE;
	}
	if (strstr(str2Upper(key),"WAITING_FOR_INITIAL_DATA"))
	{
		qualityMap["WAITING_FOR_INITIAL_DATA"] = PS_QUALITY_WAITING_FOR_INITIAL_DATA;
		return PS_QUALITY_WAITING_FOR_INITIAL_DATA;
	}	
	if (strstr(str2Upper(key),"LAST_USABLE"))
	{
		qualityMap["LAST_USABLE"] = PS_QUALITY_LAST_USABLE;
		return PS_QUALITY_LAST_USABLE;
	}	
	if (strstr(str2Upper(key),"SENSOR_CAL"))
	{
		qualityMap["SENSOR_CAL"] = PS_QUALITY_SENSOR_CAL;
		return PS_QUALITY_SENSOR_CAL;
	}	
	if (strstr(str2Upper(key),"EGU_EXCEEDED"))
	{
		qualityMap["EGU_EXCEEDED"] = PS_QUALITY_EGU_EXCEEDED;
		return PS_QUALITY_EGU_EXCEEDED;
	}	
	if (strstr(str2Upper(key),"SUB_NORMAL"))
	{
		qualityMap["SUB_NORMAL"] = PS_QUALITY_SUB_NORMAL;
		return PS_QUALITY_SUB_NORMAL;
	}	
	if (strstr(str2Upper(key),"LOCAL_OVERRIDE"))
	{
		qualityMap["LOCAL_OVERRIDE"] = PS_QUALITY_LOCAL_OVERRIDE;
		return PS_QUALITY_LOCAL_OVERRIDE;
	}		
	if (strstr(str2Upper(key),"LL_MASK"))
	{
		qualityMap["LL_MASK"] = PS_QUALITY_LL_MASK;
		return PS_QUALITY_LL_MASK;
	}
	if (strstr(str2Upper(key),"EXTRADATA"))
	{
		qualityMap["EXTRADATA"] = PS_QUALITY_EXTRADATA;
		return PS_QUALITY_EXTRADATA;
	}	
	if (strstr(str2Upper(key),"INTERPOLATED"))
	{
		qualityMap["INTERPOLATED"] = PS_QUALITY_INTERPOLATED;
		return PS_QUALITY_INTERPOLATED;
	}	
	if (strstr(str2Upper(key),"RAW"))
	{
		qualityMap["RAW"] = PS_QUALITY_RAW;
		return PS_QUALITY_RAW;
	}	
	if (strstr(str2Upper(key),"CALCULATED"))
	{
		qualityMap["CALCULATED"] = PS_QUALITY_CALCULATED;
		return PS_QUALITY_CALCULATED;
	}		
	if (strstr(str2Upper(key),"NOBOUND"))
	{
		qualityMap["NOBOUND"] = PS_QUALITY_NOBOUND;
		return PS_QUALITY_NOBOUND;
	}									
	if (strstr(str2Upper(key),"NODATA"))
	{
		qualityMap["NODATA"] = PS_QUALITY_NODATA;
		return PS_QUALITY_NODATA;
	}		
	if (strstr(str2Upper(key),"DATALOST"))
	{
		qualityMap["DATALOST"] = PS_QUALITY_DATALOST;
		return PS_QUALITY_DATALOST;
	}		
	if (strstr(str2Upper(key),"CONVERSION"))
	{
		qualityMap["CONVERSION"] = PS_QUALITY_CONVERSION;
		return PS_QUALITY_CONVERSION;
	}	
	if (strstr(str2Upper(key),"PARTIAL"))
	{
		qualityMap["PARTIAL"] = PS_QUALITY_PARTIAL;
		return PS_QUALITY_PARTIAL;
	}	
	return PS_QUALITY_UNCERTAIN;			
}

PS_HIS_AGGREGATE_ENUM Baton::getAggregate(const char* key)
{
	std::map<std::string,PS_HIS_AGGREGATE_ENUM>::iterator iter = aggregateMap.find(str2Upper(key));
	if (iter != aggregateMap.end())
	{
		return iter->second;
	}
	if (strstr(str2Upper(key),"NOAGGREGATE"))
	{
		aggregateMap["NOAGGREGATE"] = PS_HIS_NOAGGREGATE;
		return PS_HIS_NOAGGREGATE;
	}
	if (strstr(str2Upper(key),"INTERPOLATIVE"))
	{
		aggregateMap["INTERPOLATIVE"] = PS_HIS_INTERPOLATIVE;
		return PS_HIS_INTERPOLATIVE;
	}
	if (strstr(str2Upper(key),"TOTAL"))
	{
		aggregateMap["TOTAL"] = PS_HIS_TOTAL;
		return PS_HIS_TOTAL;
	}	
	if (strstr(str2Upper(key),"TOTALIZEAVERAGE"))
	{
		aggregateMap["TOTALIZEAVERAGE"] = PS_HIS_TOTALIZEAVERAGE;
		return PS_HIS_TOTALIZEAVERAGE;
	}		
	if (strstr(str2Upper(key),"AVERAGE"))
	{
		aggregateMap["AVERAGE"] = PS_HIS_AVERAGE;
		return PS_HIS_AVERAGE;
	}			
	if (strstr(str2Upper(key),"TIMEAVERAGE"))
	{
		aggregateMap["TIMEAVERAGE"] = PS_HIS_TIMEAVERAGE;
		return PS_HIS_TIMEAVERAGE;
	}				
	if (strstr(str2Upper(key),"COUNT"))
	{
		aggregateMap["COUNT"] = PS_HIS_COUNT;
		return PS_HIS_COUNT;
	}
	if (strstr(str2Upper(key),"STDEV"))
	{
		aggregateMap["STDEV"] = PS_HIS_STDEV;
		return PS_HIS_STDEV;
	}
	if (strstr(str2Upper(key),"MINIMUMACTUALTIME"))
	{
		aggregateMap["MINIMUMACTUALTIME"] = PS_HIS_MINIMUMACTUALTIME;
		return PS_HIS_MINIMUMACTUALTIME;
	}
	if (strstr(str2Upper(key),"MINIMUM"))
	{
		aggregateMap["MINIMUM"] = PS_HIS_MINIMUM;
		return PS_HIS_MINIMUM;
	}
	if (strstr(str2Upper(key),"MAXIMUMACTUALTIME"))
	{
		aggregateMap["MAXIMUMACTUALTIME"] = PS_HIS_MAXIMUMACTUALTIME;
		return PS_HIS_MAXIMUMACTUALTIME;
	}
	if (strstr(str2Upper(key),"MAXIMUM"))
	{
		aggregateMap["MAXIMUM"] = (PS_HIS_AGGREGATE_ENUM)11;
		return (PS_HIS_AGGREGATE_ENUM)11;
	}
	if (strstr(str2Upper(key),"START"))
	{
		aggregateMap["START"] = (PS_HIS_AGGREGATE_ENUM)12;
		return (PS_HIS_AGGREGATE_ENUM)12;
	}
	if (strstr(str2Upper(key),"END"))
	{
		aggregateMap["END"] = (PS_HIS_AGGREGATE_ENUM)13;
		return (PS_HIS_AGGREGATE_ENUM)13;
	}	
	if (strstr(str2Upper(key),"DELTA"))
	{
		aggregateMap["DELTA"] = (PS_HIS_AGGREGATE_ENUM)14;
		return (PS_HIS_AGGREGATE_ENUM)14;
	}		
	if (strstr(str2Upper(key),"REGSLOPE"))
	{
		aggregateMap["REGSLOPE"] = (PS_HIS_AGGREGATE_ENUM)15;
		return (PS_HIS_AGGREGATE_ENUM)15;
	}			
	if (strstr(str2Upper(key),"REGCONST"))
	{
		aggregateMap["REGCONST"] = (PS_HIS_AGGREGATE_ENUM)16;
		return (PS_HIS_AGGREGATE_ENUM)16;
	}	
	if (strstr(str2Upper(key),"REGDEV"))
	{
		aggregateMap["REGDEV"] = (PS_HIS_AGGREGATE_ENUM)17;
		return (PS_HIS_AGGREGATE_ENUM)17;
	}		
	if (strstr(str2Upper(key),"VARIANCE"))
	{
		aggregateMap["VARIANCE"] = (PS_HIS_AGGREGATE_ENUM)18;
		return (PS_HIS_AGGREGATE_ENUM)18;
	}			
	if (strstr(str2Upper(key),"RANGE"))
	{
		aggregateMap["RANGE"] = (PS_HIS_AGGREGATE_ENUM)19;
		return (PS_HIS_AGGREGATE_ENUM)19;
	}		
	if (strstr(str2Upper(key),"DURATIONGOOD"))
	{
		aggregateMap["DURATIONGOOD"] = (PS_HIS_AGGREGATE_ENUM)20;
		return (PS_HIS_AGGREGATE_ENUM)20;
	}		
	if (strstr(str2Upper(key),"DURATIONBAD"))
	{
		aggregateMap["DURATIONBAD"] = (PS_HIS_AGGREGATE_ENUM)21;
		return (PS_HIS_AGGREGATE_ENUM)21;
	}		
	if (strstr(str2Upper(key),"PERCENTGOOD"))
	{
		aggregateMap["PERCENTGOOD"] = (PS_HIS_AGGREGATE_ENUM)22;
		return (PS_HIS_AGGREGATE_ENUM)22;
	}	
	if (strstr(str2Upper(key),"PERCENTBAD"))
	{
		aggregateMap["PERCENTBAD"] = (PS_HIS_AGGREGATE_ENUM)23;
		return (PS_HIS_AGGREGATE_ENUM)23;
	}	
	if (strstr(str2Upper(key),"WORSTQUALITY"))
	{
		aggregateMap["WORSTQUALITY"] = (PS_HIS_AGGREGATE_ENUM)24;
		return (PS_HIS_AGGREGATE_ENUM)24;
	}	
	return PS_HIS_NOAGGREGATE;
		
}


RealReadBaton::RealReadBaton(PspaceNode*psNode,v8::Handle<v8::Function>* callback)
{
	this->psNode = psNode;
	if(callback!=NULL) {
		this->callback = Persistent<Function>::New(*callback);
	}
	this->code_ = PSRET_OK;
	this->errString = NULL;
	this->id = PSTAGID_UNUSED;
	this->tagName = "";
	this->realData_ = PSNULL;
	this->quality_ = PS_QUALITY_GOOD;
	this->time_ = PSNULL;
}
RealReadBaton::~RealReadBaton()
{
	 callback.Dispose();
     FREE_MEMORY(this->errString);
     FREE_MEMORY(this->time_);
     if (this->realData_ != PSNULL)
     {
         psAPI_Memory_FreeDataList(&this->realData_, 1);
     }
     if (this->varData_.DataType == PSDATATYPE_STRING)
     {
         delete this->varData_.String.Data;
     }
}

SubBaton::SubBaton(PspaceNode* psNode,v8::Handle<v8::Function>* callback)
{
	this->psNode = psNode;
	if(callback!=NULL) {
		this->callback = Persistent<Function>::New(*callback);
	}
	this->code_ = PSRET_OK;
	this->errString = NULL;
	this->id = PSTAGID_UNUSED;
	this->tagName = "";
	this->subID = -1;
	this->tagName_ = PSNULL;
	this->subData_ = PSNULL;
	this->tagCount_ = 0;
	this->timer = new uv_timer_t();  
}
SubBaton::~SubBaton()
{
    if (this->tagName_ != PSNULL)
    {
        for (int i=0;i<this->tagCount_;i++)
        {
            delete []this->tagName_[i];
        }
        delete []this->tagName_;
        this->tagName_ = PSNULL;
    }

	uv_timer_stop(timer);
    delete this->timer;
    this->timer = NULL;
	
    if (subData_!=NULL)
	{
		psAPI_Memory_FreeDataList(&subData_, tagCount_);
        this->subData_ = PSNULL;
	}
	if (this->errString != NULL)
	{
		delete errString;
        this->errString = NULL;
	}
	callback.Dispose();
}

list<SubBaton::CallbackData> SubBaton::getSubMapData(PSUINT32 subid)
{
	std::map<PSUINT32,list<SubBaton::CallbackData>>::iterator iter = mapData_.find(subid);
	list<SubBaton::CallbackData> tmp;
	if (iter !=mapData_.end())
	{
		tmp = iter->second;
		mapData_.erase(iter);
	}
	return tmp;
}

list<SubBaton::CallbackProp> SubBaton::getSubMapPropData(PSUINT32 subid)
{
	std::map<PSUINT32,list<SubBaton::CallbackProp>>::iterator iter = mapPropData_.find(subid);
	list<SubBaton::CallbackProp> tmp;
	if (iter !=mapPropData_.end())
	{
		tmp = iter->second;
		SubBaton::CallbackProp callBackData = tmp.front();
		//if ((callBackData.propData->String.Data)==NULL)
		//{
		//	std::cout<<"I am Null!"<<std::endl;
		//}
		//std::cout<<"getData:"<<(callBackData.propData->String.Data)<<std::endl;
		mapPropData_.erase(iter);
		
	}
	//SubBaton::CallbackProp callBackData = tmp.front();
	//std::cout<<"getData:"<<callBackData.propData.String.Data<<std::endl;
	
	return tmp;
}

/*
SubBaton::CallbackOtherProp SubBaton::getOtherPropData(PSUINT32 subid)
{
	map<PSUINT32,SubBaton::CallbackOtherProp>::iterator iter = propsOther_.find(subid);
	CallbackOtherProp otherData ;
	otherData.changeType = -1;
	otherData.propCount = -1;
	if (iter!= propsOther_.end())
	{
		otherData = iter->second;
		propsOther_.erase(iter);
	}
	
	return otherData;
}
*/
DelSubBaton::DelSubBaton(PspaceNode* psNode,v8::Handle<v8::Function>* callback)
{
	this->psNode = psNode;
	if(callback!=NULL) {
		this->callback = Persistent<Function>::New(*callback);
	}
	this->code_ = PSRET_OK;
	this->errString = NULL;
	this->subID=-1;
	this->tagCount_=0;
	this->tagName_ = PSNULL;
}
DelSubBaton::~DelSubBaton()
{
    if (this->tagName_ != PSNULL)
    {
        for (int i=0;i<this->tagCount_;i++)
        {
            delete []tagName_[i];
        }
        delete tagName_;
        this->tagName_ = PSNULL;
    }
	callback.Dispose();
    if (this->errString != NULL)
    {
        delete this->errString;
        this->errString = NULL;
    }
	
}

PSVOID PSAPI SubBaton::Real_CallbackFunction(
	PSIN PSHANDLE hServer,
	PSIN PSUINT32 nSubscribeId,
	PSIN void *pUserPara,
	PSIN PSUINT32 nCount,
	PSIN PSUINT32 *pTagIds,
	PSIN PS_DATA *pRealDataList
	)
{
	list<CallbackData> pushTmp;
	for (int n = 0; n < nCount; n++)
	{
		SubBaton::CallbackData callBackData;
		//字符串类型变量单独处理
		callBackData.tagID = *(pTagIds+n);
		if ((pRealDataList+n)->Value.DataType==PSDATATYPE_STRING )
		{
			callBackData.date.Quality = (pRealDataList+n)->Quality;
			callBackData.date.Time = (pRealDataList+n)->Time;
			callBackData.date.Value.DataType = (pRealDataList+n)->Value.DataType;
			callBackData.date.Value.String.Data = new char[strlen((pRealDataList+n)->Value.String.Data)+1];
			strcpy(callBackData.date.Value.String.Data,(pRealDataList+n)->Value.String.Data);
			callBackData.date.Value.String.Data[strlen(callBackData.date.Value.String.Data)] = 0;
		}else{
			callBackData.date = *(pRealDataList+n);
		}
		pushTmp.push_back(callBackData);
	}
	mapData_.insert(map<PSUINT32,list<CallbackData>>::value_type(nSubscribeId,pushTmp));
	
}

PSVOID PSAPI SubBaton::Tag_CallbackFunction( PSIN PSHANDLE hServer,
	PSIN PSUINT32 nSubscribeId,
	PSIN PSVOID *pUserPara,
	PSIN PSUINT32 nTagId,
	PSIN PSUINT32 nChangeType,
	PSIN PSUINT32 nPropCount,
	PSIN PSUINT16 *pPropIds,
	PSIN PS_VARIANT *pPropValues)
{
	list<CallbackProp> dataTmp;
	for (int n = 0; n < nPropCount; n++)
	{
		CallbackProp proData;
		////
		//proData.propData = NULL;
		
		proData.changeType = nChangeType;
		proData.propID = *(pPropIds+n);
		if ((pPropValues+n)->DataType == PSDATATYPE_STRING)
		{
			proData.propData.DataType = PSDATATYPE_STRING;
			proData.propData.String.Data = new char[proData.propData.String.Length+1];
			strcpy(proData.propData.String.Data,(pPropValues+n)->String.Data);
			proData.propData.String.Data[strlen(proData.propData.String.Data)]=0;
		}else{
			proData.propData = *(pPropValues+n);
		}
		proData.tagID = nTagId;
		dataTmp.push_back(proData);
		
		Sleep(1);
	}
	mapPropData_.insert(map<PSUINT32,list<CallbackProp>>::value_type(nSubscribeId,dataTmp));
	//SubBaton::CallbackProp callBackData = dataTmp.front();
	//std::cout<<"callBackData:"<<callBackData->propData->String.Data<<std::endl;
	
} 

void SubBaton::timer_cb(uv_timer_t *handle,int status)
{
	
	HandleScope scope;
	Handle<Value> argv[4];
	SubBaton* sbaton = static_cast<SubBaton*>(handle->data);
	//Local<Array> arrObj1 = Array::New(sbaton->tagCount_);
	list<CallbackData> callData = sbaton->getSubMapData(sbaton->subID);

	if (callData.size()>=1)
	{
		int len = callData.size();
		Local<Array> arrObj = Array::New(len);
		
		for (int i=0;i<len;i++)
		{
			Local<Object> tmpObj = Object::New();
			SubBaton::CallbackData callBackData = callData.front();
			callData.pop_front();
			tmpObj->Set(String::New("name"),String::New(GBK2UTF8(replace_all(sbaton->getTagName(callBackData.tagID),"\\","/").c_str()).c_str()));
			tmpObj->Set(String::New("value"),getRealObj(&callBackData.date));
			arrObj->Set(i,tmpObj);
		}
		
		argv[0] = Undefined();
		argv[1] = Uint32::New(sbaton->subID);
		argv[2] =  Undefined();
		argv[3] = arrObj;
		//scope.Close(arrObj);
		//异步回调执行cb
		node::MakeCallback(Context::GetCurrent()->Global(), sbaton->callback, 4, argv);
	}
	
}

void SubBaton::time_propCb(uv_timer_t *handle,int status)
{
	Handle<Value> argv[3];
	SubBaton* sbaton = static_cast<SubBaton*>(handle->data);
//	Local<Array> arrObj1 = Array::New(sbaton->tagCount_);
	list<CallbackProp> callData = sbaton->getSubMapPropData(sbaton->subID);
	
	//CallbackOtherProp callOther  = sbaton->getOtherPropData(sbaton->subID);
	if (callData.size()>=1)
	{
		int len = callData.size();
		Local<Array> arrObj = Array::New(len);
		for (int i=0;i<len;i++)
		{
			Local<Object> tmpObj = Object::New();
			SubBaton::CallbackProp callBackData = callData.front();
			callData.pop_front();
			
			tmpObj->Set(String::New("changeType"),Uint32::New(callBackData.changeType));
			
			tmpObj->Set(String::New("propID"),Uint32::New(callBackData.propID));
			tmpObj->Set(String::New("name"),String::New(GBK2UTF8(replace_all(sbaton->getTagName(callBackData.tagID),"\\","/").c_str()).c_str()));
			getpropValue(&(callBackData.propData),tmpObj);
		
			arrObj->Set(i,tmpObj);
		}
		
		argv[0] = Undefined();
		argv[1] = Uint32::New(sbaton->subID);
		argv[2] = arrObj;
		//异步回调执行cb
		node::MakeCallback(Context::GetCurrent()->Global(), sbaton->callback, 3, argv);
	}
}
Tag::Tag(PspaceNode* psNode,v8::Handle<v8::Function>* callback)
{
	this->psNode = psNode;
	if(callback!=NULL) {
		this->callback = Persistent<Function>::New(*callback);
	}
	this->code_ = PSRET_OK;
	this->errString = NULL;
	this->id = PSTAGID_UNUSED;
	this->tagName = "";
	this->parentTagId_ = PSTAGID_UNUSED;
	this->propCount_ = 0;
	this->pPropIds_ = PSNULL;
	this->pPropValues_ = PSNULL;
	this->propArr = PSNULL;
    this->rName_ = NULL;
}

Tag::~Tag()
{
	callback.Dispose();

    if (this->pPropIds_ != PSNULL)
    {
        delete []pPropIds_;
        this->pPropIds_ = PSNULL;
    }
	
    if (this->pPropValues_ != PSNULL)
    {
        for (int i=0;i<this->propCount_;i++)
        {

        }
        delete []pPropValues_;
        this->pPropValues_ = PSNULL;
    }
	
	delete []propArr;
	if (this->errString != NULL)
	{
        delete this->errString;
        this->errString = NULL;
	}
}

TagHis::TagHis(PspaceNode* psNode,v8::Handle<v8::Function>* callback)
{
	this->psNode = psNode;
	if(callback!=NULL) {
		this->callback = Persistent<Function>::New(*callback);
	}
	this->code_ = PSRET_OK;
	this->errString = NULL;
	this->id = 0;
	this->tagName = "";
	this->tagCount_ = 0;
	this->bBounds_ = 0;
	this->pTimeStamps_ = PSNULL;
	this->pQualities_ = PSNULL;
	this->tagIds_ = PSNULL;
	this->pAggregates_=PSNULL;
	this->pHisDataList_ = PSNULL;
	this->pDataValues_ = PSNULL;
	this->nNumTimeStamps_ = 0;
	this->dataStatus_ = NULL;
	this->timeStatusLen_ = 0;
    this->nMaxNumOfReturnValues_ = 0;
	
}
TagHis::~TagHis()
{
	
	delete []pTimeStamps_;
	delete []pQualities_;
	delete []tagIds_;
	delete []pAggregates_;
	delete []dataStatus_;
	psAPI_Memory_FreeTagHisDataList(&(this->pHisDataList_),this->tagCount_);
	callback.Dispose();
    if (this->errString = NULL)
    {
        delete this->errString;
        this->errString = NULL;
    }
}
Alarm::Alarm(PspaceNode* psNode,v8::Handle<v8::Function>* callback)
{
	this->psNode = psNode;
	if(callback!=NULL) {
		this->callback = Persistent<Function>::New(*callback);
	}
	this->alarmCount_=0;
	this->alarms_ = PSNULL;
	this->code_ = PSRET_OK;
	PS_TIME tmp = {0,0};
	this->startTime_ = tmp;
	this->endTime_ = tmp;
	PS_ALARM_FILTER_FIELD tmp1 = {0};
	PS_ALARM_FILTER tmp2 = {0};
	this->filter_ = tmp2;
	this->filterField_ = tmp1;
	this->id = PSTAGID_UNUSED;
	this->ackUserId_ = PSUSERID_UNUSED;
	this->alarmIds_ = PSNULL;
	this->ackCount_ = 0;
    this->errString = PSNULL;
	
}

Alarm::~Alarm()
{
	delete []alarmIds_;
	psAPI_Memory_FreeAlarmList(&alarms_, alarmCount_);
	callback.Dispose();
    if (this->errString = PSNULL)
    {
        delete this->errString;
        this->errString = PSNULL;
    }
}

BatchBaton::BatchBaton(PspaceNode* psNode,v8::Handle<v8::Function>* callback)
{
	this->psNode = psNode;
	if(callback!=NULL) {
		this->callback = Persistent<Function>::New(*callback);
	}
	this->tagCount_=0;
	//this->tagID_ = PSNULL;
	this->code_ = PSRET_OK;
	this->errString = NULL;
	this->realData_ = PSNULL;
	this->dataValues_ = NULL;
	this->timeStamps_ = NULL;
	this->qualitys_ = NULL;
	this->tagName_ = PSNULL;
}
BatchBaton::~BatchBaton()
{
     if (this->dataValues_ != NULL)
    {
      delete []dataValues_;
      this->dataValues_ = NULL;
    }
    for (int i=0;i<this->tagCount_;i++)
    {
      delete []tagName_[i];
    }
    this->tagCount_=0;
    if (this->tagName_ != PSNULL)
    {
      delete []tagName_;
      this->tagName_ = PSNULL;
    }
    if (this->qualitys_ != NULL)
    {
      delete []qualitys_;
      this->qualitys_ = NULL;
    }

    if (this->errString != NULL)
    {
      delete errString;
      this->errString = NULL;
    }
    if (timeStamps_!=NULL)
    {
      delete []timeStamps_;
      this->timeStamps_ = NULL;

    }
	//psAPI_Memory_FreeDataList(&realData_, tagCount_);
	callback.Dispose();

}

TagQuery::TagQuery(PspaceNode* psNode,v8::Handle<v8::Function>* callback)
{
	this->psNode_ = psNode;
	if(callback!=NULL) {
		this->callback_ = Persistent<Function>::New(*callback);
	}
	
	this->tagID_ = PSTAGID_UNUSED;
	this->propIds_ = PSNULL;
	this->filter_ = PSNULL;
	this->fieldPropids_ = PSNULL;
	this->fieldPropValues_ = PSNULL;
	this->propCount_ = 0;
	this->fieldPropCnt_ = 0;
	this->tagCount_ = 0;
	this->tagPropValues_ = PSNULL;
	this->propArr_ = PSNULL;
    this->errString_ = PSNULL;
}

TagQuery::~TagQuery()
{
	delete []filter_;
	delete []propIds_;
	if(fieldPropids_!=NULL){
		delete []fieldPropids_;
		delete []fieldPropValues_;
	}
    if (this->errString_ != PSNULL)
    {
        delete errString_;
        this->errString_ = PSNULL;
    }
	delete propArr_;
	psAPI_Memory_FreeTagPropList(&(tagPropValues_),tagCount_);
	callback_.Dispose();

}