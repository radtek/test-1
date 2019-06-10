const fs=require('fs');
const log4js = require('log4js');
const opcua = require("node-opcua");
const FRtdb = require('frtdb');
const ps=JSON.parse(fs.readFileSync("./confs/ua.json")).pspace;
const sconf = JSON.parse(fs.readFileSync("./confs/ua.json")).server;
const log4jsconf = JSON.parse(fs.readFileSync("./confs/log4js.json")).log;
const log4jslevel = JSON.parse(fs.readFileSync("./confs/log4js.json")).level;

const getFactory = require("node-opcua-factory/src/factories_factories").getFactory;
const DataValue = getFactory("DataValue");
const Variant = require("node-opcua-variant").Variant;
const VariantArrayType = require("node-opcua-variant").VariantArrayType;

const dataTrans = require("./src/transformDataType");
const ps2ua = dataTrans.ps2ua;
const qualityTrans = require("./src/transfromQualityCode");
const qualityTransPs2ua = qualityTrans.qualityPs2ua;
const qualityTransUa2ps = qualityTrans.qualityUa2ps;

const SecurityPolicy = require("node-opcua-secure-channel").SecurityPolicy;
const MessageSecurityMode = require("node-opcua-service-secure-channel").MessageSecurityMode;


var uaserver;
var retJsonTagInfos;
var addressSpace;
var namespace;
var db;
var tags = [];
var tagLeafIds;
var tagNodeMap = new Map();
var anonymous = false;
var interval = parseInt(sconf.realDataFlushTime);
var version = "1.0.0.1";

log4js.configure(log4jsconf);

const logger = log4js.getLogger()
const errlogger = log4js.getLogger('err')
const reqlogger = log4js.getLogger('req')
logger.level = log4jslevel;

function run(parJson,tagMap,parentId){
	let p = tagMap.get(parentId);
	let j = {};
	j.psId = parJson.layerId;
	j.psShortName = parJson.layerName;
	j.psParentId = p.psId;
	j.uaNodeId = "ns=1;s=ps"+ parJson.layerId;
	j.uaBrowseName = parJson.layerName;
	j.uaParentNodeId = p.uaNodeId;
	tagMap.set(j.psId,j);
	if(parJson.childern!=undefined){
		j.psIsLeaf = 0;
		for(let k=0; k<parJson.childern.length;k++){
			run(parJson.childern[k],tagMap,j.psId);
		}
	}
}

var userManager = {
	isValidUser:function(userName,password){
		for(let i=0; i<sconf.userManager.length;i++){
			if(userName===sconf.userManager[i].userName && password===sconf.userManager[i].password){
				return true;
			}
		}
		return false;
	}
};

if(sconf.userManager.length==0){
	anonymous = true;
}

function initTagInfos(retJson,tagNodeMap){
	if(retJson.layerId==0){
		let j = {};
		j.psId = 0;
		j.psShortName = "pspace";
		j.psParentId = -1;
		j.psIsLeaf = 0;
		j.uaNodeId="s=pspace";
		j.uaBrowseName = "pspace";
		tagNodeMap.set(j.psId,j);
		if(retJson.childern!=undefined){
			for(let i=0; i<retJson.childern.length;i++){
				run(retJson.childern[i],tagNodeMap,0);
			}
		}
	}
}


function getLeafsIds(tagJson){
	let ret = [];
	function run(tagPar){
		if(tagPar.childern == undefined){
			ret.push(tagPar.layerId);
			return;
		}else{
			for(let i=0; i<tagPar.childern.length;i++){
				run(tagPar.childern[i]);
			}
		}
	}

	run(tagJson);
	return ret;
}


function initUaAddressspace(retJsonTagInfos,tagNodeMap){
	let obj = tagNodeMap.get(retJsonTagInfos.layerId);
	if(retJsonTagInfos.layerId==0){
		let n = namespace.addFolder("ObjectsFolder",{ browseName: obj.uaBrowseName});
		obj.uaNode=n;
	}else{
		let p = tagNodeMap.get(obj.psParentId);
		if(obj.psIsLeaf==0){
			let n = namespace.addFolder(p.uaNode,{ browseName: obj.uaBrowseName});
			obj.uaNode=n;
			n.tagId = obj.psId;
		}else{
			let n = namespace.addVariable(
			{
				nodeId:obj.uaNodeId ,
				componentOf:p.uaNode,
				browseName:obj.uaBrowseName,
				dataType:obj.uaDataType,
				value: {  
					timestamped_get:function(){
						let dataValue = new DataValue({
						value: new Variant({
						dataType: obj.uaDataType,
						arrayType: VariantArrayType.Scalar,
						dimensions: null,
						value:obj.value}),
						serverTimestamp: new Date(obj.time),
						serverPicoseconds: 0,
						sourceTimestamp: new Date(obj.time),
						sourcePicoseconds: 0,
						statusCode: qualityTransPs2ua(obj.quality)});
						return dataValue;
					},
					timestamped_set:async function(dataValue,callback){
					if(obj.uaDataType == opcua.DataType.Double){
						obj.value = parseFloat(dataValue.value.value);
					}else{
						obj.value = dataValue.value.value;
					}
					obj.time = new Date(dataValue.sourceTimestamp);
					let quality = qualityTransUa2ps(dataValue.statusCode);
					retJson = await db.getRealWrite({'id':obj.psId,'time':obj.time,'data':{'dataType':obj.psDataType,'value':obj.value},"quality":quality});
					if(retJson.retCode==0){
						let info = "setdata success [" + obj.psId + "] {dataType:" + obj.psDataType +",value:"+obj.value  +"}";
						reqlogger.info(info);
					}else{
						let errMsg = "setdata error [" + obj.psId + "] {dataType:" + obj.psDataType +",value:"+obj.value  +"}" + " {psCode:" + retJson.retCode+"}";
						errlogger.error(errMsg);
					}
						let status = qualityTransPs2ua(retJson.retCode);
						callback(null,status);
					}
				}
			});
	
			obj.uaNode=n;
			n.db = db;
			n.uaps = true;
			n.isSub = false;
			n.tagId = obj.psId;
			n.uaDataType_ = obj.uaDataType;
			n.errlogger = errlogger;
			n.reqlogger = reqlogger;
			if(obj.isHistory==1){
				addressSpace.installHistoricalDataNode(n);
			}
		}
	}

	if(retJsonTagInfos.childern!=undefined){
		for(let i=0; i<retJsonTagInfos.childern.length;i++){
			initUaAddressspace(retJsonTagInfos.childern[i],tagNodeMap);
		}
	}
}


async function flushRealValue(){
	retJson = await db.getRealReadList(tags);
	if(retJson.retCode==0 || retJson.retCode==-19997){
		for(let i=0; i<retJson.dataList.length;i++){
			let obj = tagNodeMap.get(tags[i]);
			obj.time = retJson.dataList[i].time;
			obj.quality = retJson.dataList[i].quality;
			if(retJson.dataList[i].psvalue.type!=0){
				if(retJson.dataList[i].psvalue.type==1){
					obj.psDataType = retJson.dataList[i].psvalue.type;
					if(retJson.dataList[i].psvalue.value){
						obj.value = true;
					}else{
						obj.value = false;
					}
					
				}else{
					obj.psDataType = retJson.dataList[i].psvalue.type;
					obj.value = retJson.dataList[i].psvalue.value;
				}
			}else{
				obj.value = 0;
				obj.quality = 32;
			}
		}
	}else{
		let errMsg = "刷新获取pspace数据错误 {psCode:" + retJson.retCode+"}";
		errlogger.error(errMsg);
	}
}



async function business(){
    let options = {
        dialect:'ps6',
        username:ps.user,
        password:ps.pass,
        logging: console.log,
        path:''
	}
	
	db = new FRtdb(ps.host,ps.port,options);
	
	let ret = await db.login();
	if(!ret){
		console.log("pspace login fail...");
		errlogger.error("pspace login fail...");
		process.exit(1);
	}
	
	retJsonTagInfos = await db.getLayerInfo();
	initTagInfos(retJsonTagInfos,tagNodeMap);

	tagLeafIds = getLeafsIds(retJsonTagInfos);

	let retJson = await db.getTagListProps({'tagIds':tagLeafIds,'propIds':[30,13,11]});
	
	if(retJson.retCode!=0){
		console.log("get attributes error!");
		errlogger.error("get attributes error!");
		process.exit(0);
	}

	for(let i=0;i<retJson.tagCount;i++){
		let obj = tagNodeMap.get(tagLeafIds[i]);
		if(obj != undefined){
			if(retJson.tagPropList[i][0].data==1){
				obj.isHistory = 1;
			}else{
				obj.isHistory = 0;
			}

			obj.psDataType = retJson.tagPropList[i][1].data;
			obj.uaDataType = ps2ua(obj.psDataType);

			if(retJson.tagPropList[i][2].data==1)
			{
				obj.psIsLeaf = 0;
			}else{
				obj.psIsLeaf = 1;
				tags.push(tagLeafIds[i]);
			}
		}	
	}

	retJson = await db.getRealReadList(tags);
	if(retJson.retCode==0 || retJson.retCode==-19997){
		let info = "初始化获取pspace数据成功 {psCode:" + retJson.retCode+"}";
		reqlogger.info(info);
		for(let i=0; i<retJson.dataList.length;i++){
			let obj = tagNodeMap.get(tags[i]);
			obj.time = retJson.dataList[i].time;
			obj.quality = retJson.dataList[i].quality;
			if(retJson.dataList[i].psvalue.type!=0){
				if(retJson.dataList[i].psvalue.type==1){
					obj.psDataType = retJson.dataList[i].psvalue.type;
					if(retJson.dataList[i].psvalue.value){
						obj.value = true;
					}else{
						obj.value = false;
					}
					
				}else{
					obj.psDataType = retJson.dataList[i].psvalue.type;
					obj.value = retJson.dataList[i].psvalue.value;
				}
			}else{
				obj.value = 0;
				obj.quality = 32;
			}
		}
	}else{
		let errMsg = "初始化获取pspace数据错误 {psCode:" + retJson.retCode+"}";
		errlogger.error(errMsg);
		exit(0);
	}

	db.on('dataChange',function(data){
		for(let i=0; i<data.tagDataList.length;i++){
			let obj = tagNodeMap.get(data.tagDataList[i].id);
			if(obj !=undefined){
				if(obj.psDataType==1){
					if(data.tagDataList[i].value){
						obj.value = true;
					}else{
						obj.value = false;
					}
					obj.quality = data.tagDataList[i].quality;
					obj.time = data.tagDataList[i].pstime;
				}else{
					obj.value = data.tagDataList[i].value;
					obj.quality = data.tagDataList[i].quality;
					obj.time = data.tagDataList[i].pstime;
				}

			}
		}
	});


	options = {
		port: sconf.port, 
		resourcePath: sconf.resourcePath,
		applicationName: sconf.applicationName,
		userManager:userManager,
		allowAnonymous:anonymous,
		buildInfo : {
			productName:sconf.productName,
			buildNumber:sconf.buildNumber,
			manufacturerName:sconf.manufacturerName,
			productUri:sconf.productUrl,
			buildDate: new Date()
		}
	};

	if(sconf.securityPolicies.length>0){
		options.securityPolicies = [];
	}

	for(let i=0;i<sconf.securityPolicies.length;i++){
		switch(sconf.securityPolicies[i]){
			case "None":
				options.securityPolicies.push(SecurityPolicy.None);
				break;
			case "Basic128Rsa15":
				options.securityPolicies.push(SecurityPolicy.Basic128Rsa15);
				break;
			case "Basic256":
				options.securityPolicies.push(SecurityPolicy.Basic256);
				break;
			default:
				break;
		}
	}

	if(sconf.securityModes.length>0){
		options.securityModes = [];
	}

	for(let i=0;i<sconf.securityModes.length;i++){
		switch(sconf.securityModes[i]){
			case "None":
				options.securityModes.push(MessageSecurityMode.NONE);
				break;
			case "Sign":
				options.securityModes.push(MessageSecurityMode.SIGN);
				break;
			case "Signandencrypt":
				options.securityModes.push(MessageSecurityMode.SIGNANDENCRYPT);
				break;
			default:
				break;
		}
	}

	uaserver = new opcua.OPCUAServer(options);

	uaserver.initialize(()=>{
		addressSpace = uaserver.engine.addressSpace;
		namespace = addressSpace.getOwnNamespace();
		initUaAddressspace(retJsonTagInfos,tagNodeMap);
		uaserver.start(()=>{
			console.log("server is now listening ... ( press CTRL+C to stop)");
			console.log("port ", uaserver.endpoints[0].port);
			const endpointUrl = uaserver.endpoints[0].endpointDescriptions()[0].endpointUrl;
			console.log(" the primary server endpoint url is ", endpointUrl );
			let info = "the primary server endpoint url is " + endpointUrl;
			reqlogger.info(info);
			setInterval(async function(){
				await flushRealValue();
			}, interval*1000);
		});
	})
}
business();