var addon = require("./pSpace");
var ps = new addon.Client();

var connectDate = {
	"hostname":"localhost",
	"user":"admin",
	"password":"admin888",
	"port":8889
};
//同步断开与连接
/*
//连接方式一
var con = ps.connect(connectDate);
if(con.hasOwnProperty("errString"))
{
	console.log("错误码:",con.code);
	console.log("服务器连接失败:",con.errString);
	throw "connect fail!";
}
*/

//连接方式二,端口号可提供可不提供，不提供时采用默认端口
var con = ps.connect("localhost","admin","admin888",8889);
if(con.hasOwnProperty("errString"))
{
	console.log("错误码:",con.code);
	console.log("服务器连接失败:",con.errString);
	throw "connect fail!";
}
//判断是否连接，如果连接返回true,w未连接返回false
if (con.isConnected()) {
		console.log("con 已经连接！");
}else{
		console.log("con 已经断开连接！");
	 }
 //断开测试
/*
//con.close();
if (con.isConnected()) {
		console.log("con 已经连接！");
}else{
		console.log("con 已经断开连接！");
	 }
*/
/*
//异步断开与连接操作测试，参数也提供了json串和直接传入两种方式
var res = ps.connect(connectDate,function(err,con1){
	if(err)
	{
		console.log(err);
	}
	else
	{
		if (con1.isConnected()) {
			console.log("con1 已经连接！");
		}
		con1.close();
		if (con1.isConnected()) {
			console.log("con1 已经连接！");
		}
		else
		{
			console.log("con1 已经断开连接！");
		}
	}
	
});
*/

/*
//同步读取实时值测试
var pv = con.read("/tag.pv");
if(pv.hasOwnProperty("errString")){
	console.log(pv.code);
	console.log(pv.errString);
	con.close();
}else{
	console.log(pv);
	console.log(pv.value);
	console.log(pv.quality);
	console.log(pv.time);
}

//异步读取实时值测试
var r = con.read("/tag.pv",function(err,pv){
	if(err){
		console.log(err);
		con.close();
	}else{
		console.log(pv);
		console.log(pv.value);
	}
});

*/
/*
var writeData  = {
	"value":67.001,
	"quality":0,
	"time":"2015-01-21 00:20:0.123"
};


//同步写实时测试方式一
var res = con.write("/tag.pv",writeData);
if(res.hasOwnProperty("errString")){
	console.log(res.code);
	console.log(res.errString);
	//con.close();
}else{
	console.log(res,":写实时成功");
}


//同步写实时方式二
var res1 = con.write("/tag.pv",89.78,0);
if(res.hasOwnProperty("errString")){
	console.log(res1.code);
	console.log(res1.errString);
	//con.close();
}else{
	console.log(res1,":写实时成功");
}
*/

/*
//异步写实时测试，参数提供直接传参和json串两种格式
var resAsy = con.write("/tag.pv",writeData, function(err,tagName){
	if(err){
		console.log(err);
		//con.close();
	}else{
		console.log(tagName,":异步写实时成功");
	}
});
*/

/*
// 异步新建订阅测点实时值变化并得到初值测试,此函数并不会阻塞整个程序
var resSub = con.sub("/tag.real",function(err,tagName,val){
	if(err){
		console.log(err);
		//con.close();
	}else
	{
		console.log(val.value);
	}
});
*/
/*
//异步测点属性订阅测试,两个订阅同时执行存在问题
var resProps = con.sub("/tag.props",function(err,tagName,val){
	if (err) {
		console.log(err);
		con.close();
	}else{
		console.log("订阅测点:",tagName,"成功!");
		console.log("点ID:",val.id);
		console.log("订阅属性变化类型",val.type,"变化属性数量:",val.count);
		console.log("属性值:",val.value);
	}
});
*/

var props = {
	"propID":["PS_TAG_PROP_NAME","PS_TAG_PROP_TAGTYPE","PS_TAG_PROP_DESCRIPTION"],
	"dataType":["PSDATATYPE_STRING","PSDATATYPE_UINT16","PSDATATYPE_STRING"],
	"values":["tag2",0,"psNode测试"]
};

/*
//同步添加测点
var resAddSyn = con.add("/tag1",props);
if(resAddSyn.hasOwnProperty("errString")){
	console.log("错误码:",resAddSyn.code);
	console.log("添加测点错误:",resAddSyn.errString);
	//con.close();
}else{
	console.log("添加测点:",resAddSyn,"成功!");
}
*/
/*
//异步添加测点
var resAddAsy = con.add("/tag1",props,function(err,tagName,tagId){
	if(err){
		console.log(err);
		con.close();
	}else{
		console.log("异步添加测点:",tagName,"ID:",tagId,"成功!");
	}
	
});
*/
/*
//同步删除测点测试
var resDelSyn = con.del("/tag1/tag2");
if(resDelSyn.hasOwnProperty("errString")){
	console.log("错误码:",resDelSyn.code);
	console.log("错误信息:",resDelSyn.errString);
}else{
	console.log("删除测点:",resDelSyn,"成功!");
}
*/
/*
//异步删除测点测试
var resDelAsy = con.del("/tag1",function(err,tagName){
	if(err){
		console.log(err);
	}else{
		console.log("异步删除测点:",tagName,"成功!");
	}
});

*/

var setProps = {
	"propID":["PS_TAG_PROP_DESCRIPTION","PS_TAG_PROP_SECURITYAREA"],
	"dataType":["PSDATATYPE_STRING","PSDATATYPE_UINT64"],
	"values":["SetTagProps5",4095]
};

/*
var resSetProps = con.write("/tag2.props",setProps);
if(resSetProps.hasOwnProperty("errString")){
	console.log("错误码:",resSetProps.code);
	console.log("设置测点属性测点错误:",resSetProps.errString);
	con.close();
}else{
	console.log("设置测点属性:",resSetProps,"成功!");
}
*/

/*
var resSetPropsAsy = con.write("/tag2.props",setProps,function(err,tagName,tagId){
	if (err) {
		console.log(err);
	}else{
		console.log("异步设置测点属性成功:",tagName,"ID:",tagId);
	}
});
*/

//同步获取测点属性测试
var getProps = {
	"propID":["PS_TAG_PROP_NAME","PS_TAG_PROP_TAGTYPE"],
	"dataType":["PSDATATYPE_STRING","PSDATATYPE_UINT16"],
};

/*
var props = con.read("/tag.props",getProps);
if(props.hasOwnProperty("errString")){
	console.log(props.code);
	console.log(props.errString);
}else{
	console.log("同步获取属性值:",props);
}

//异步获取测点属性成功
var resGetProps = con.read("/tag.props",getProps,function(err,tagName,props){
	if(err){
		console.log(err);
	}else{
		console.log("异步获取",tagName,"属性值:",props);
	}
});
*/
var histry = {
	"value":12.001,
	"time":"2008-05-13 14:28:04.123",
	"quality":"PS_QUALITY_RAW"
};

/*
//同步插入某一测点历史数据测试, 支持json格式参数和直接传入参数
var resInsert = con.write("/tag.his.insert",histry);
if(resInsert.hasOwnProperty("errString")){
	console.log(resInsert.code);
	console.log(resInsert.errString);
	//con.close();
}else{
	console.log("对测点:",resInsert,"插入历史数据成功!");
}

//异步插入某一测点的历史数据测试
var resInsertAsy = con.write("/tag.his.insert",histry,function(err,tagName,tagId){
	if(err){
		console.log(err);
	}else{
		console.log("异步插入测点:",tagName,"历史数据成功.");
	}
});


////同步插入替换某一测点历史数据
var resInsertRep = con.write("/tag.his.insertReplace",histry);
if(resInsertRep.hasOwnProperty("errString")){
	console.log(resInsertRep.code);
	console.log(resInsertRep.errString);
	//con.close();
}else{
	console.log("对测点:",resInsertRep,"插入替换历史数据成功!");
}


//异步插入替换某一测点的历史数据测试
var resInsertRepAsy = con.write("/tag.his.insert",histry,function(err,tagName,tagId){
	if(err){
		console.log(err);
	}else{
		console.log("异步插入替换测点:",tagName,"历史数据成功.");
	}
});

*/
/*
//同步替换某一测点历史数据
var resRep = con.write("/tag.his.replace",histry);
if(resRep.hasOwnProperty("errString")){
	console.log(resRep.errString);
}else{
	console.log("同步替换测点:",resRep,"成功!");
}

//同步替换某一测点历史数据直接传参
var resRep1 = con.write("/tag.his.replace",23.324,"PS_QUALITY_RAW","2008-05-13 14:28:04.123"); 
if(resRep1.hasOwnProperty("errString")){
	console.log(resRep1.errString);
}else{
	console.log("同步替换测点:",resRep1,"成功!");
}
//异步替换某一测点历史数据
var resRepAsy = con.write("/tag.his.replace",histry,function(err,tagName,tagId){
	if(err){
		console.log(err);
	}else{
		console.log("异步替换测点:",tagName,"历史数据成功.");
	}
});
*/

//同步查询某一测点固定时间历史数据
//查询时刻数组
var timeStamps = ["2014-01-21 14:28:04.123"];

/*
var resAtTime= con.read("/tag.his.atTime",timeStamps);
if(resAtTime.hasOwnProperty("errString")){
	console.log("错误:",resAtTime.errString);
}else{
	console.log(resAtTime[0].value);
}

//异步查询某一测点固定时间历史数据
var resAtTime2= con.read("/tag.his.atTime",timeStamps,function(err,resVal){
	if(err){
		console.log(err);
	}else{
		console.log(resVal);
	}
});
*/
//同步查询某一测点一段时间的原始数据
var timeStart = "2012-02-01 16:42:29.000";
var timeEnd = "2014-02-017 17:42:29.000";
/*
var resRaw =  con.read("/tag.his.raw",timeStart,timeEnd,1);
if(resRaw.hasOwnProperty("errString")){
	console.log(resRaw.errString);
}else{
	console.log("共查询到:",resRaw.length,"条数据.");
	//打印出所有数据
	console.log(resRaw);
	//将数据分类分条打印
	console.log(resRaw[0].value);
	console.log(resRaw[1].value);
	console.log(resRaw[1].time);
}
//异步查询某一测点一段时间的原始数据
var resRaw1= con.read("/tag.his.raw",timeStart,timeEnd,1,function(err,resVal){
	if(err){
		console.log(err);
	}else{
		console.log(resVal);
	}
});
*/
//同步查询某一测点一段时间的历史统计数据
var hisPro = {
	"startTime":"2013-01-02 12:21:59.000",
	"endTime":"2014-2-18 13:21:59.000",
	"resampleInterval":10000000,
	"aggregates":"PS_HIS_START"
};
/*
var resProcess = con.read("/tag.his.processed",hisPro);	
if(resProcess.hasOwnProperty("errString")){
	console.log(con.errString);
}else{
	console.log(resProcess);
}	
//直接传参
var resProcess1 = con.read("/tag.his.processed","2013-01-02 12:21:59.000","2014-2-18 13:21:59.000",10000000,"PS_HIS_START");	
if(resProcess1.hasOwnProperty("errString")){
	console.log(con.errString);
}else{
	console.log(resProcess1);
}	
*/
//异步查询某一测点一段时间的历史统计数据
/*
var resProcess2 = con.read("/tag.his.processed",hisPro,function(err,resVal){
	if(err){
		console.log(err);
	}else{
		console.log(resVal);
	}
});
*/

//同步查询实时报警
var filter = {
	"filterField":{"TagId":0,"AlarmHaveAcked":1},
	"filter":{"AlarmHaveAcked":0}
}
//直接传参
var filterField = {"TagId":0,"AlarmHaveAcked":1};
var filter1 = {"AlarmHaveAcked":0};

var realAlarm = con.read("/tag.alarm.real",filterField,filter1);

var realAlarm1 = con.read("/tag.alarm.real",filter);
if(realAlarm1.hasOwnProperty("errString")){
	console.log(realAlarm1.errString);
}else{
	console.log("报警数量为:",realAlarm1.length);
	console.log(realAlarm1);
}

//异步查询实时报警
var realAlarm2 = con.read("/tag.alarm.real",filter,function(err,resVal){
	if(err){
		console.log(err);
	}else{
		console.log("报警数量为:",realAlarm1.length);
		console.log(resVal);
	}
});

//同步查询历史报警
var hisFilter = {
	'filterField':{"TagId":0,"AlarmHaveAcked":1},
	'filter':{"AlarmHaveAcked":0},
	'startTime':"2014-02-03 12:23:45.000",
	'endTime':"2014-02-20 09:00:00.000"
}
var hisAlarm1 = con.read("/tag.alarm.his",hisFilter);
if(hisAlarm1.hasOwnProperty("errString")){
	console.log(hisAlarm1.errString);
}else{
	console.log("同步查询历史报警数量:",hisAlarm1.length);
	console.log(hisAlarm1);
}
//直接传参
var hisAlarm = con.read("/tag.alarm.his",filterField,filter,"2014-02-03 12:23:45.000","2014-02-20 09:00:00.000");
if(hisAlarm.hasOwnProperty("errString")){
	console.log(hisAlarm.errString);
}else{
	console.log("同步查询历史报警数量:",hisAlarm.length);
	console.log(hisAlarm);
}
//异步历史查询报警
var hisAlarm2 = con.read("/tag.alarm.his",hisFilter,function(err,hisVal){
	if(err){
		console.log(err);
	}else{
		console.log("异步历史查询报警数量:",hisVal.length);
		console.log(hisVal);
	}
});

//同步查询应答报警
var ack = {
	"ackUserId":1,
	"ackUserName":"Admin",
	"ackTime":"2014-02-13 10:20:00.000",
	"ackIds":[2]
}
var ackeIds = [2];
//直接传参数
var ackAlarm1 = con.read("/tag.alarm.ack",1,"Admin","2014-02-13 10:20:00.000",ackeIds);
var ackAlarm = con.read("/tag.alarm.ack",ack);
if(ackAlarm.hasOwnProperty("errString")){
	console.log(ackAlarm.errString);
}else{
	console.log("同步应答报警成功!");
	console.log("应答者ID:",ackAlarm.userId,",应答者用户名:",ackAlarm.userName,",应答时间:",ackAlarm.ackTime);
}
//异步查询应答报警
var ackAlarm2 = con.read("/tag.alarm.ack",ack,function(err,ackVal){
	if(err){
		console.log(err);
	}else{
		console.log("异步应答报警成功!");
		console.log("应答者ID:",ackVal.userId,",应答者用户名:",ackVal.userName,",应答时间:",ackVal.ackTime);
	}
});
