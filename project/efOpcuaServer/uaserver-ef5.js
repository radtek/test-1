const fs=require('fs');
const log4js = require('log4js');
const opcua = require("node-opcua");
const FRtdb = require('frtdb');
const ef=JSON.parse(fs.readFileSync("./confs/ua.json")).ef5;
const sconf = JSON.parse(fs.readFileSync("./confs/ua.json")).server;
const log4jsconf = JSON.parse(fs.readFileSync("./confs/log4js.json")).log;
const log4jslevel = JSON.parse(fs.readFileSync("./confs/log4js.json")).level;

const getFactory = require("node-opcua-factory/src/factories_factories").getFactory;
const DataValue = getFactory("DataValue");
const Variant = require("node-opcua-variant").Variant;
const VariantArrayType = require("node-opcua-variant").VariantArrayType;

const dataTrans = require("./src/transformDataType");
const ef2ua = dataTrans.ef2ua;
const qualityTrans = require("./src/transfromQualityCode");
const qualityTransEf2ua = qualityTrans.qualityEf2ua;
const qualityTransUa2ef = qualityTrans.qualityUa2ef;

const SecurityPolicy = require("node-opcua-secure-channel").SecurityPolicy;
const MessageSecurityMode = require("node-opcua-service-secure-channel").MessageSecurityMode;


var uaserver;
var retJsonTagInfos;
var addressSpace;
var namespace;
var db;
var tagNodeMap = new Map();
var tagLeafMap = new Map();
var anonymous = false;
var interval = parseInt(sconf.realDataFlushTime);
var tags = [];
var tagInfosMap = new Map();
var version = "1.0.0.1";

log4js.configure(log4jsconf);

const logger = log4js.getLogger()
const errlogger = log4js.getLogger('err')
const reqlogger = log4js.getLogger('req')
logger.level = log4jslevel;

async function run(parJson,tagMap,parentId){
	let p = tagMap.get(parentId);
	let j = {};
	j.efId = parseInt(parJson.layerId);
	j.efShortName = parJson.layerName;
	j.efParentId = p.efId;
	j.uaNodeId = "ns=1;s=efnode"+ parJson.layerId;
	j.uaBrowseName = parJson.layerName;
	j.uaParentNodeId = p.uaNodeId;
	j.efIsLeaf = 0;
	tagMap.set(j.efId,j);
	let a = [];
	a.push(j.efId);
	let r = await db.getTagInfo(a);
	for(let i=0; i<r.retData.length;i++){
		let l = {};
		l.efId = parseInt(r.retData[i].tagId);
		l.efShortName = r.retData[i].tagShortName;
		l.efParentId = j.efId;
		l.efIsLeaf = 1;
		l.uaNodeId="ns=1;s=ef"+r.retData[i].tagId;
		l.uaBrowseName = r.retData[i].tagShortName;
		let p = {};
		p.tagId = l.efId;
		let res = await db.getData([p]);
		l.longName = res.retData[0].tagName;
		l.value = res.retData[0].value;
		l.time = res.retData[0].time;
		l.quality = res.retData[0].quality;
		let o = tagInfosMap.get(l.efId);
		if(o.kind==1){
			l.efDataType = 1;
			l.uaDataType = opcua.DataType.Boolean;
		
			if(l.value>0){
				l.value = true;
			}else{
				l.value = false;
			}
	
		}else{
			l.efDataType = 0;
			l.uaDataType = opcua.DataType.Double;
		}

		tagLeafMap.set(l.efId,l);
	}
	
	if(parJson.children!=undefined){
		for(let k=0; k<parJson.children.length;k++){
			await run(parJson.children[k],tagMap,j.efId);
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

async function initTagInfos(retJson,tagNodeMap){
	let j = {};
	j.efId = 0;
	j.efShortName = "ef5";
	j.efParentId = -1;
	j.efIsLeaf = 0;
	j.uaNodeId="ns=1;s=ef5";
	j.uaBrowseName = "ef5";
	tagNodeMap.set(j.efId,j);
	let r = await db.getTagInfo([0]);
	for(let i=0; i<r.retData.length;i++){
		let l = {};
		l.efId = parseInt(r.retData[i].tagId);
		l.efShortName = r.retData[i].tagShortName;
		l.efParentId = 0;
		l.efIsLeaf = 1;
		l.uaNodeId="ns=1;s=ef"+r.retData[i].tagId;
		l.uaBrowseName = r.retData[i].tagShortName;
		let p = {};
		p.tagId = l.efId;
		let res = await db.getData([p]);
		l.longName = res.retData[0].tagName;
		l.value = res.retData[0].value;
		l.time = res.retData[0].time;
		l.quality = res.retData[0].quality;
		let o = tagInfosMap.get(l.efId);
		if(o.kind==1){
			l.efDataType = 1;
			l.uaDataType = opcua.DataType.Boolean;
			if(l.value>0){
				l.value = true;
			}else{
				l.value = false;
			}
		}else{
			l.efDataType = 0;
			l.uaDataType = opcua.DataType.Double;
		}
		tagLeafMap.set(l.efId,l);
	}
	
	if(retJson.retData.children!=undefined){
		for(let i=0; i<retJson.retData.children.length;i++){
			await run(retJson.retData.children[i],tagNodeMap,0);
		}
	}
}

function initUaAddressspaceNode(){
	let n;
	tagNodeMap.forEach(function (obj, tagId) {
		if(obj.efId==0){
			n = namespace.addFolder("ObjectsFolder",{ browseName: obj.uaBrowseName});
		}else{
			let p = tagNodeMap.get(obj.efParentId);
			n = namespace.addFolder(p.uaNode,{ browseName: obj.uaBrowseName});
		}

		obj.uaNode=n;
		n.tagId = obj.efId;
		n.db = db;
		n.isSub = false;
		n.errlogger = errlogger;
		n.reqlogger = reqlogger;
	});
}


function initUaAddressspaceLeaf(){
		tagLeafMap.forEach(function (obj, tagId) {
			let p = tagNodeMap.get(obj.efParentId);
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
							statusCode: qualityTransEf2ua(obj.quality)});
							return dataValue;
						},
						timestamped_set:async function(dataValue,callback){
							if(obj.uaDataType == opcua.DataType.Double){
								obj.value = parseFloat(dataValue.value.value);
							}else{
								obj.value = dataValue.value.value;
							}
							let s = [];
							let o = {};
							o.tagId = obj.efId;
							o.data =obj.value;
							s.push(o);
							db.setData(s);
							callback(null,opcua.StatusCodes.Good);
						}
					}
				});
			addressSpace.installHistoricalDataNode(n);
			obj.uaNode=n;
			n.tagId = obj.efId;
			n.db = db;
			n.isSub = false;
			n.uaDataType_ = obj.uaDataType;
			n.errlogger = errlogger;
			n.reqlogger = reqlogger;
			addressSpace.installHistoricalDataNode(n);
		});
}

async function flushRealValue_(){
	tagLeafMap.forEach(async function (obj, tagId) {
		let p = {};
		p.tagId = tagId;
		let res = await db.getData([p]);
		obj.value = res.retData[0].value;
		obj.time = res.retData[0].time;
		obj.quality = res.retData[0].quality;
		if(obj.uaDataType==opcua.DataType.Boolean){
			if(obj.value>0){
				obj.value = true;
			}else{
				obj.value = false;
			}
		}
	});
}

async function flushRealValue(){
	let res = await db.getData(tags);
	for(let i=0; i<tags.length;i++){
		let obj = tagLeafMap.get(tags[i].tagId);
		obj.value = res.retData[i].value;
		obj.time = res.retData[i].time;
		obj.quality = res.retData[i].quality;
		if(obj.uaDataType==opcua.DataType.Boolean){
			if(obj.value>0){
				obj.value = true;
			}else{
				obj.value = false;
			}
		}
	}
}



async function business(){
    let options = {
        dialect:'ef5',
        username:ef.user,
        password:ef.pass,
        logging: console.log,
        path:''
	}
	
	db = new FRtdb(ef.host,ef.port,options);
	await db.login();
	db.dialect = options.dialect;

	let tagInfos = await db.getTagInfoAddForUa();
	for(let i=0; i<tagInfos.length;i++){
		let j = {};
		j.kind = parseInt(tagInfos[i].sKind);
		j.parentId = tagInfos[i].nodeId;
		j.tagLongName = tagInfos[i].tagName;
		tagInfosMap.set(parseInt(tagInfos[i].tagId),j);
	}

	tagInfosMap.forEach(function (value, key) {
	});

	retJsonTagInfos = await db.getLayerInfo();
	retJsonTagInfos.layerId = "0";
	retJsonTagInfos.layerName = "";
	delete retJsonTagInfos.retCode;
	await initTagInfos(retJsonTagInfos,tagNodeMap);

	tagNodeMap.forEach(function (value, key) {
	});

	tagLeafMap.forEach(function (value, key) {
		let p = {};
		p.tagId = key;
		tags.push(p);
	});


	db.on('dataChange',function(data){
		for(let i=0; i<data.length;i++){
			let obj = tagLeafMap.get(data[i].tagId);
			obj.value = data[i].data;
			obj.time = new Date().getTime();
			obj.quality = 192;
			if(obj.uaDataType==opcua.DataType.Boolean){
				if(obj.value>0){
					obj.value = true;
				}else{
					obj.value = false;
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
		initUaAddressspaceNode();
		initUaAddressspaceLeaf();
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