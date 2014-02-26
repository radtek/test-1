#include "baton.h"
#include "pSpaceNode.h"
#define PSVARIANT2STR(pVariant) PSVARIANT2STRHELP((char*)alloca(256),pVariant)
std::list<SubBaton::CallbackData> SubBaton::callbackData_;
std::list<PSUINT32> SubBaton::propsOther_;
std::list<char*> SubBaton::callbackProps_;
std::map<std::string,PS_TAG_PROP_ENUM> Tag::tag;
std::map<std::string,PS_DATATYPE_ENUM> Tag::dataTypeMap_;
std::map<std::string,PS_QUALITY_ENUM> TagHis::qualityMap_;
std::map<std::string,PS_HIS_AGGREGATE_ENUM> TagHis::aggregates_;
PSUINT32 Baton::getTagID(const char* pszTagName,PSHANDLE h)
{
	std::map<std::string, PSUINT32>::iterator iter = tagName2ID_.find(pszTagName);
	if (iter != tagName2ID_.end())
	{
		return iter->second;
	}
	PSUINT32 nTagID = PSTAGID_UNUSED;
	//std::cout<<pszTagName<<std::endl;
	PSAPIStatus nRet = psAPI_Tag_GetIdByLongName(h, (PSSTR)pszTagName, &nTagID);
	if (PSERR(nRet))
	{
		//std::cout<<pszTagName<<std::endl;
		std::cout<<psAPI_Commom_GetErrorDesc(nRet)<<std::endl;
		return PSTAGID_UNUSED;
	}
	tagName2ID_[pszTagName] = nTagID;
	id2TagName_[nTagID] = pszTagName;
	return nTagID;
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

RealReadBaton::RealReadBaton(PspaceNode*psNode,v8::Handle<v8::Function>* callback)
{
	this->psNode = psNode;
	if(callback!=NULL) {
		this->callback = Persistent<Function>::New(*callback);
	}
	this->code_ = PSRET_OK;
	this->errString = NULL;
	this->db = 0;
	this->quality = PSNULL;
	this->id = PSTAGID_UNUSED;
	this->tagName = "";

	PS_TIME *d = new PS_TIME[sizeof(PS_TIME)];
	STR2PSTIME(d,"");
	this->time = *d;
	
	
}
RealReadBaton::~RealReadBaton()
{
	 callback.Dispose();
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
}
PSVOID PSAPI SubBaton::Real_CallbackFunction(
	PSIN PSHANDLE hServer,
	PSIN PSUINT32 nSubscribeId,
	PSIN PSVOID *pUserPara,
	PSIN PSUINT32 nCount,
	PSIN PSUINT32 *pTagIds,
	PSIN PS_DATA *pRealDataList
	)
{
	for (int n = 0; n < nCount; n++)
	{
		CallbackData callBackData = { 
			PS_VARIANT2DOUBLE((pRealDataList+n)->Value),
			(pRealDataList+n)->Quality,
			PSTIME2STR((pRealDataList+n)->Time)
		};
		callbackData_.push_back(callBackData);
	}
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
	//std::cout<<"nTagid:"<<nTagId<<std::endl;
	propsOther_.push_back(nTagId);
	propsOther_.push_back(nChangeType);
	propsOther_.push_back(nPropCount);
	for (int n = 0; n < nPropCount; n++)
	{
		callbackProps_.push_back(PSVARIANT2STR(pPropValues+n));
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
	this->pPropIds_ = PSNULL/*new PSUINT16[this->propCount_]*/;
	this->pPropValues_ = PSNULL/*new PS_VARIANT[this->propCount_]*/;

	dataTypeMap_.insert(map<std::string,PS_DATATYPE_ENUM>::value_type("PSDATATYPE_EMPTY",PSDATATYPE_EMPTY));
	dataTypeMap_.insert(map<std::string,PS_DATATYPE_ENUM>::value_type("PSDATATYPE_BOOL",PSDATATYPE_BOOL));
	dataTypeMap_.insert(map<std::string,PS_DATATYPE_ENUM>::value_type("PSDATATYPE_INT8",PSDATATYPE_INT8));
	dataTypeMap_.insert(map<std::string,PS_DATATYPE_ENUM>::value_type("PSDATATYPE_UINT8",PSDATATYPE_UINT8));
	dataTypeMap_.insert(map<std::string,PS_DATATYPE_ENUM>::value_type("PSDATATYPE_INT16",PSDATATYPE_INT16));
	dataTypeMap_.insert(map<std::string,PS_DATATYPE_ENUM>::value_type("PSDATATYPE_UINT16",PSDATATYPE_UINT16));
	dataTypeMap_.insert(map<std::string,PS_DATATYPE_ENUM>::value_type("PSDATATYPE_INT32",PSDATATYPE_INT32));
	dataTypeMap_.insert(map<std::string,PS_DATATYPE_ENUM>::value_type("PSDATATYPE_UINT32",PSDATATYPE_UINT32));
	dataTypeMap_.insert(map<std::string,PS_DATATYPE_ENUM>::value_type("PSDATATYPE_INT64",PSDATATYPE_INT64));
	dataTypeMap_.insert(map<std::string,PS_DATATYPE_ENUM>::value_type("PSDATATYPE_UINT64",PSDATATYPE_UINT64));
	dataTypeMap_.insert(map<std::string,PS_DATATYPE_ENUM>::value_type("PSDATATYPE_FLOAT",PSDATATYPE_FLOAT));
	dataTypeMap_.insert(map<std::string,PS_DATATYPE_ENUM>::value_type("PSDATATYPE_DOUBLE",PSDATATYPE_DOUBLE));
	dataTypeMap_.insert(map<std::string,PS_DATATYPE_ENUM>::value_type("PSDATATYPE_TIME",PSDATATYPE_TIME));
	dataTypeMap_.insert(map<std::string,PS_DATATYPE_ENUM>::value_type("PSDATATYPE_STRING",PSDATATYPE_STRING));
	dataTypeMap_.insert(map<std::string,PS_DATATYPE_ENUM>::value_type("PSDATATYPE_WSTRING",PSDATATYPE_WSTRING));
	dataTypeMap_.insert(map<std::string,PS_DATATYPE_ENUM>::value_type("PSDATATYPE_BLOB",PSDATATYPE_BLOB));

	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_ID", PS_TAG_PROP_ID));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_TAGBASE_MIN", PS_TAG_PROP_TAGBASE_MIN));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_NAME", PS_TAG_PROP_NAME));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_TAGTYPE", PS_TAG_PROP_TAGTYPE));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_LONGNAME", PS_TAG_PROP_LONGNAME));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_ISUNIQUE", PS_TAG_PROP_ISUNIQUE));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_DESCRIPTION", PS_TAG_PROP_DESCRIPTION));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_SECURITYAREA", PS_TAG_PROP_SECURITYAREA));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_PARENTID", PS_TAG_PROP_PARENTID));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_FIRSTCHILDID", PS_TAG_PROP_FIRSTCHILDID));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_PREVID", PS_TAG_PROP_PREVID));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_NEXTID", PS_TAG_PROP_NEXTID));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_NEXTID", PS_TAG_PROP_NEXTID));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_ISNODE", PS_TAG_PROP_ISNODE));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_SUBNODECOUNT", PS_TAG_PROP_SUBNODECOUNT));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_DATATYPE", PS_TAG_PROP_DATATYPE));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_LASTMODIFYTIME", PS_TAG_PROP_LASTMODIFYTIME));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_CREATETIME", PS_TAG_PROP_CREATETIME));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_TAGBASE_MAX", PS_TAG_PROP_TAGBASE_MAX));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_HIS_MIN", PS_TAG_PROP_HIS_MIN));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_HIS_ISSAVE", PS_TAG_PROP_HIS_ISSAVE));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_HIS_COMPRESSRATE", PS_TAG_PROP_HIS_COMPRESSRATE));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_HIS_ISCOMPRESSRATEPERCENTAGE", PS_TAG_PROP_HIS_ISCOMPRESSRATEPERCENTAGE));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_HIS_COMPRESSMODE", PS_TAG_PROP_HIS_COMPRESSMODE));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_HIS_MAX", PS_TAG_PROP_HIS_MAX));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PSPROPFLAG_EXTEND_MIN", PSPROPFLAG_EXTEND_MIN));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_PV", PS_TAG_PROP_PV));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_LASTPV", PS_TAG_PROP_LASTPV));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_PV_TIME", PS_TAG_PROP_PV_TIME));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_PV_QUALITY", PS_TAG_PROP_PV_QUALITY));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_LASTPV_TIME", PS_TAG_PROP_LASTPV_TIME));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_LASTPV_QUALITY", PS_TAG_PROP_LASTPV_QUALITY));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_OFFMESSAGE", PS_TAG_PROP_OFFMESSAGE));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_ONMESSAGE", PS_TAG_PROP_ONMESSAGE));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_INITVALUE", PS_TAG_PROP_INITVALUE));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_SAVEASINITVALUE", PS_TAG_PROP_SAVEASINITVALUE));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_ENABLEALARM", PS_TAG_PROP_ENABLEALARM));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_ENABLEVALUEALARM", PS_TAG_PROP_ENABLEVALUEALARM));


	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_NORMALVALUE",PS_TAG_PROP_NORMALVALUE ));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_VALUEALARMNEEDRESPOND", PS_TAG_PROP_VALUEALARMNEEDRESPOND));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_ENABLEQUALITYALARM",PS_TAG_PROP_ENABLEQUALITYALARM));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_QUALITYALARMNEEDRESPOND", PS_TAG_PROP_QUALITYALARMNEEDRESPOND));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_VALUEALARMLEVEL", PS_TAG_PROP_VALUEALARMLEVEL));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_QUALITYALARMLEVEL", PS_TAG_PROP_QUALITYALARMLEVEL));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_FLAG_MAX_DIGITAL",PS_TAG_PROP_FLAG_MAX_DIGITAL));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_FLAG_MAX_STRING", PS_TAG_PROP_FLAG_MAX_STRING));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_ENABLELLALARM",PS_TAG_PROP_ENABLELLALARM ));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_ENABLELOALARM",PS_TAG_PROP_ENABLELOALARM ));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_ENABLEHIALARM", PS_TAG_PROP_ENABLEHIALARM));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_ENABLEHHALARM",PS_TAG_PROP_ENABLEHHALARM ));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_ENABLERATEALARM",PS_TAG_PROP_ENABLERATEALARM ));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_ENABLEDEVALARM",PS_TAG_PROP_ENABLEDEVALARM ));


	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_LOWLOWALARMNEEDRESPOND", PS_TAG_PROP_LOWLOWALARMNEEDRESPOND));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_LOWALARMNEEDRESPOND", PS_TAG_PROP_LOWALARMNEEDRESPOND));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_HIGHALARMNEEDRESPOND", PS_TAG_PROP_HIGHALARMNEEDRESPOND));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_RATEALARMNEEDRESPOND",PS_TAG_PROP_RATEALARMNEEDRESPOND ));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_DEVALARMNEEDRESPOND",PS_TAG_PROP_DEVALARMNEEDRESPOND ));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_LOWLOWALARM",PS_TAG_PROP_LOWLOWALARM ));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_LOWALARM",PS_TAG_PROP_LOWALARM ));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_HIGHALARM",PS_TAG_PROP_HIGHALARM ));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_HIGHHIGHALARM",PS_TAG_PROP_HIGHHIGHALARM ));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_RATE",PS_TAG_PROP_RATE ));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_RATEPERIOD",PS_TAG_PROP_RATEPERIOD ));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_DEV",PS_TAG_PROP_DEV ));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_SP",PS_TAG_PROP_SP ));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_DEADBAND",PS_TAG_PROP_DEADBAND ));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_LOWLOWALARMLEVEL", PS_TAG_PROP_LOWLOWALARMLEVEL));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_LOWALARMLEVEL",PS_TAG_PROP_LOWALARMLEVEL));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_HIGHALARMLEVEL",PS_TAG_PROP_HIGHALARMLEVEL ));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_HIGHHIGHALARMLEVEL",PS_TAG_PROP_HIGHHIGHALARMLEVEL ));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_RATEALARMLEVEL",PS_TAG_PROP_RATEALARMLEVEL ));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_DEVALARMLEVEL",PS_TAG_PROP_DEVALARMLEVEL ));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_RANGEMINIMUM", PS_TAG_PROP_RANGEMINIMUM));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_RANGEMAXIMUN",PS_TAG_PROP_RANGEMAXIMUN ));

	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_ENGINEERINGUNIT", PS_TAG_PROP_ENGINEERINGUNIT));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_FLAG_MAX_ANALOG", PS_TAG_PROP_FLAG_MAX_ANALOG));
	tag.insert(map<std::string,PS_TAG_PROP_ENUM> :: value_type("PS_TAG_PROP_FLAG_EXTEND_MAX", PS_TAG_PROP_FLAG_EXTEND_MAX));

	
}


Tag::~Tag()
{
	callback.Dispose();
	delete []pPropIds_;
	delete []pPropValues_;
	
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
	qualityMap_.insert(map<std::string,PS_QUALITY_ENUM>::value_type("PS_QUALITY_MASK",PS_QUALITY_MASK));
	qualityMap_.insert(map<std::string,PS_QUALITY_ENUM>::value_type("PS_QUALITY_BAD",PS_QUALITY_BAD));
	qualityMap_.insert(map<std::string,PS_QUALITY_ENUM>::value_type("PS_QUALITY_UNCERTAIN",PS_QUALITY_UNCERTAIN));
	qualityMap_.insert(map<std::string,PS_QUALITY_ENUM>::value_type("PS_QUALITY_GOOD",PS_QUALITY_GOOD));
	qualityMap_.insert(map<std::string,PS_QUALITY_ENUM>::value_type("PS_QUALITY_SUBSTATUS_MASK",PS_QUALITY_SUBSTATUS_MASK));
	qualityMap_.insert(map<std::string,PS_QUALITY_ENUM>::value_type("PS_QUALITY_EXTRADATA",PS_QUALITY_EXTRADATA));
	qualityMap_.insert(map<std::string,PS_QUALITY_ENUM>::value_type("PS_QUALITY_INTERPOLATED",PS_QUALITY_INTERPOLATED));
	qualityMap_.insert(map<std::string,PS_QUALITY_ENUM>::value_type("PS_QUALITY_PARTIAL",PS_QUALITY_PARTIAL));
	qualityMap_.insert(map<std::string,PS_QUALITY_ENUM>::value_type("PS_QUALITY_RAW",PS_QUALITY_RAW));
	qualityMap_.insert(map<std::string,PS_QUALITY_ENUM>::value_type("PS_QUALITY_CALCULATED",PS_QUALITY_CALCULATED));
	qualityMap_.insert(map<std::string,PS_QUALITY_ENUM>::value_type("PS_QUALITY_NOBOUND",PS_QUALITY_NOBOUND));
	qualityMap_.insert(map<std::string,PS_QUALITY_ENUM>::value_type("PS_QUALITY_CONVERSION",PS_QUALITY_CONVERSION));
	qualityMap_.insert(map<std::string,PS_QUALITY_ENUM>::value_type("PS_QUALITY_DATALOST",PS_QUALITY_DATALOST));
	qualityMap_.insert(map<std::string,PS_QUALITY_ENUM>::value_type("PS_QUALITY_NODATA",PS_QUALITY_NODATA));
	qualityMap_.insert(map<std::string,PS_QUALITY_ENUM>::value_type("PS_QUALITY_LL_MASK",PS_QUALITY_LL_MASK));
	qualityMap_.insert(map<std::string,PS_QUALITY_ENUM>::value_type("PS_QUALITY_LOCAL_OVERRIDE",PS_QUALITY_LOCAL_OVERRIDE));
	qualityMap_.insert(map<std::string,PS_QUALITY_ENUM>::value_type("PS_QUALITY_SUB_NORMAL",PS_QUALITY_SUB_NORMAL));
	qualityMap_.insert(map<std::string,PS_QUALITY_ENUM>::value_type("PS_QUALITY_CONFIG_ERROR",PS_QUALITY_CONFIG_ERROR));
	qualityMap_.insert(map<std::string,PS_QUALITY_ENUM>::value_type("PS_QUALITY_NOT_CONNECTED",PS_QUALITY_NOT_CONNECTED));
	qualityMap_.insert(map<std::string,PS_QUALITY_ENUM>::value_type("PS_QUALITY_DEVICE_FAILURE",PS_QUALITY_DEVICE_FAILURE));
	qualityMap_.insert(map<std::string,PS_QUALITY_ENUM>::value_type("PS_QUALITY_SENSOR_FAILURE",PS_QUALITY_SENSOR_FAILURE));
	qualityMap_.insert(map<std::string,PS_QUALITY_ENUM>::value_type("PS_QUALITY_LAST_KNOWN",PS_QUALITY_LAST_KNOWN));
	qualityMap_.insert(map<std::string,PS_QUALITY_ENUM>::value_type("PS_QUALITY_COMM_FAILURE",PS_QUALITY_COMM_FAILURE));
	qualityMap_.insert(map<std::string,PS_QUALITY_ENUM>::value_type("PS_QUALITY_OUT_OF_SERVICE",PS_QUALITY_OUT_OF_SERVICE));
	qualityMap_.insert(map<std::string,PS_QUALITY_ENUM>::value_type("PS_QUALITY_WAITING_FOR_INITIAL_DATA",PS_QUALITY_WAITING_FOR_INITIAL_DATA));
	qualityMap_.insert(map<std::string,PS_QUALITY_ENUM>::value_type("PS_QUALITY_LAST_USABLE",PS_QUALITY_LAST_USABLE));
	qualityMap_.insert(map<std::string,PS_QUALITY_ENUM>::value_type("PS_QUALITY_SENSOR_CAL",PS_QUALITY_SENSOR_CAL));
	qualityMap_.insert(map<std::string,PS_QUALITY_ENUM>::value_type("PS_QUALITY_EGU_EXCEEDED",PS_QUALITY_EGU_EXCEEDED));

	aggregates_.insert(map<std::string,PS_HIS_AGGREGATE_ENUM>::value_type("PS_HIS_NOAGGREGATE",PS_HIS_NOAGGREGATE));
	aggregates_.insert(map<std::string,PS_HIS_AGGREGATE_ENUM>::value_type("PS_HIS_NOAGGREGATE",PS_HIS_NOAGGREGATE));
	aggregates_.insert(map<std::string,PS_HIS_AGGREGATE_ENUM>::value_type("PS_HIS_INTERPOLATIVE",PS_HIS_INTERPOLATIVE));
	aggregates_.insert(map<std::string,PS_HIS_AGGREGATE_ENUM>::value_type("PS_HIS_TOTAL",PS_HIS_TOTAL));
	aggregates_.insert(map<std::string,PS_HIS_AGGREGATE_ENUM>::value_type("PS_HIS_TOTALIZEAVERAGE",PS_HIS_TOTALIZEAVERAGE));
	aggregates_.insert(map<std::string,PS_HIS_AGGREGATE_ENUM>::value_type("PS_HIS_AVERAGE",PS_HIS_AVERAGE));
	aggregates_.insert(map<std::string,PS_HIS_AGGREGATE_ENUM>::value_type("PS_HIS_TIMEAVERAGE",PS_HIS_TIMEAVERAGE));
	aggregates_.insert(map<std::string,PS_HIS_AGGREGATE_ENUM>::value_type("PS_HIS_COUNT",PS_HIS_COUNT));
	aggregates_.insert(map<std::string,PS_HIS_AGGREGATE_ENUM>::value_type("PS_HIS_STDEV",PS_HIS_STDEV));
	aggregates_.insert(map<std::string,PS_HIS_AGGREGATE_ENUM>::value_type("PS_HIS_MINIMUMACTUALTIME",PS_HIS_MINIMUMACTUALTIME));
	aggregates_.insert(map<std::string,PS_HIS_AGGREGATE_ENUM>::value_type("PS_HIS_MINIMUM",PS_HIS_MINIMUM));
	aggregates_.insert(map<std::string,PS_HIS_AGGREGATE_ENUM>::value_type("PS_HIS_MAXIMUMACTUALTIME",PS_HIS_MAXIMUMACTUALTIME));
	aggregates_.insert(map<std::string,PS_HIS_AGGREGATE_ENUM>::value_type("PS_HIS_MAXIMUM",PS_HIS_MAXIMUM));
	aggregates_.insert(map<std::string,PS_HIS_AGGREGATE_ENUM>::value_type("PS_HIS_START",PS_HIS_START));
	aggregates_.insert(map<std::string,PS_HIS_AGGREGATE_ENUM>::value_type("PS_HIS_END",PS_HIS_END));
	aggregates_.insert(map<std::string,PS_HIS_AGGREGATE_ENUM>::value_type("PS_HIS_DELTA",PS_HIS_DELTA));
	aggregates_.insert(map<std::string,PS_HIS_AGGREGATE_ENUM>::value_type("PS_HIS_REGSLOPE",PS_HIS_REGSLOPE));
	aggregates_.insert(map<std::string,PS_HIS_AGGREGATE_ENUM>::value_type("PS_HIS_REGCONST",PS_HIS_REGCONST));
	aggregates_.insert(map<std::string,PS_HIS_AGGREGATE_ENUM>::value_type("PS_HIS_REGDEV",PS_HIS_REGDEV));
	aggregates_.insert(map<std::string,PS_HIS_AGGREGATE_ENUM>::value_type("PS_HIS_VARIANCE",PS_HIS_VARIANCE));
	aggregates_.insert(map<std::string,PS_HIS_AGGREGATE_ENUM>::value_type("PS_HIS_RANGE",PS_HIS_RANGE));
	aggregates_.insert(map<std::string,PS_HIS_AGGREGATE_ENUM>::value_type("PS_HIS_DURATIONGOOD",PS_HIS_DURATIONGOOD));
	aggregates_.insert(map<std::string,PS_HIS_AGGREGATE_ENUM>::value_type("PS_HIS_DURATIONBAD",PS_HIS_DURATIONBAD));
	aggregates_.insert(map<std::string,PS_HIS_AGGREGATE_ENUM>::value_type("PS_HIS_PERCENTGOOD",PS_HIS_PERCENTGOOD));
	aggregates_.insert(map<std::string,PS_HIS_AGGREGATE_ENUM>::value_type("PS_HIS_PERCENTBAD",PS_HIS_PERCENTBAD));
	aggregates_.insert(map<std::string,PS_HIS_AGGREGATE_ENUM>::value_type("PS_HIS_WORSTQUALITY",PS_HIS_WORSTQUALITY));
}
TagHis::~TagHis()
{
	delete []pTimeStamps_;
	delete []pQualities_;
	delete []tagIds_;
	delete []pAggregates_;
	psAPI_Memory_FreeTagHisDataList(&(this->pHisDataList_),this->tagCount_);
	callback.Dispose();
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
	
}

Alarm::~Alarm()
{
	
	delete []alarmIds_;
	psAPI_Memory_FreeAlarmList(&alarms_, alarmCount_);
	callback.Dispose();
}