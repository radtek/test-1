var addon = require("../lib/pSpace");
var add = require("../lib/tag.js");
var ps = new addon.Client();
var Err = addon.Error;
var con = ps.connect("localhost","admin","admin888");
if(con instanceof Err){
	console.log("错误码:",con.code);
	console.log("服务器连接失败:",con.errString);
	throw "connect fail!";
}else{
	console.log("连接成功!");
}
console.log();
//模拟sleep（假死）
function sleep(milliSeconds){
    var startTime = new Date().getTime(); // get the current time
    while (new Date().getTime() < startTime + milliSeconds); // hog cpu
}
/*
//先添加10万个点，一级测点98999个二级测点1000个
var tag = {
		"propID":["PS_TAG_PROP_NAME","PS_TAG_PROP_TAGTYPE"],
		"dataType":["PSDATATYPE_STRING","PSDATATYPE_UINT16"],
		"values":["tag",0]
		};
var resAddSyn= con.add("/tag",tag);
var propID = ["PS_TAG_PROP_NAME","PS_TAG_PROP_TAGTYPE","PS_TAG_PROP_DATATYPE","PS_TAG_PROP_DESCRIPTION"];
var dataType = ["PSDATATYPE_STRING","PSDATATYPE_UINT16","PSDATATYPE_UINT8","PSDATATYPE_STRING"];
for(var i=1;i<=100000;i++){
	if(i>=99000)
	{
		var values = ["test"+i,2,"PSDATATYPE_DOUBLE","psNode测试"];
		var resAddSyn4 = con.add("/tag/test"+i,propID,dataType,values);	
	}else{
		var values = ["tag"+i,2,"PSDATATYPE_DOUBLE","psNode测试"];
		var resAddSyn4 = con.add("/tag"+i,propID,dataType,values);
	}			
}
console.log("测点添加完毕，开始测试....");

//先设置测点报警属性
//同步设置所有测点报警属性
*/
console.log("同步设置所有测点报警属性....");
var setID = [ "PS_TAG_PROP_ENABLEALARM","PS_TAG_PROP_ENABLELLALARM",
					"PS_TAG_PROP_ENABLELOALARM","PS_TAG_PROP_ENABLEHIALARM",
					"PS_TAG_PROP_ENABLEHHALARM","PS_TAG_PROP_LOWLOWALARMNEEDRESPOND",
					"PS_TAG_PROP_LOWALARMNEEDRESPOND","PS_TAG_PROP_HIGHALARMNEEDRESPOND",
					"PS_TAG_PROP_HIGHHIGHALARMNEEDRESPOND","PS_TAG_PROP_ENABLERATEALARM","PS_TAG_PROP_ENABLEDEVALARM",
					"PS_TAG_PROP_LOWLOWALARM","PS_TAG_PROP_LOWALARM",
					"PS_TAG_PROP_HIGHALARM","PS_TAG_PROP_HIGHHIGHALARM",
					"PS_TAG_PROP_RATE","PS_TAG_PROP_DEV"];
	var setDataType = ["PSDATATYPE_BOOL","PSDATATYPE_BOOL","PSDATATYPE_BOOL","PSDATATYPE_BOOL","PSDATATYPE_BOOL",
						"PSDATATYPE_BOOL","PSDATATYPE_BOOL","PSDATATYPE_BOOL","PSDATATYPE_BOOL",
						"PSDATATYPE_BOOL","PSDATATYPE_BOOL",
						"PSDATATYPE_DOUBLE","PSDATATYPE_DOUBLE",
						"PSDATATYPE_DOUBLE","PSDATATYPE_DOUBLE",
						"PSDATATYPE_DOUBLE","PSDATATYPE_DOUBLE"];
	var setValues = [1,1,1,1,1,1,1,1,1,1,1,10.00,60.00,90.00,200.00,10.00,20.00];
console.time("SynSetAlarmProps");
for(var i=1;i<=100000;++i){
	if(i>=99000){
		con.write("/tag/test"+i+".props",setID,setDataType,setValues);
	}else{
		con.write("/tag"+i+".props",setID,setDataType,setValues);
	}
}
console.log("同步设置测点报警属性完毕.");
console.timeEnd("SynSetAlarmProps");
//time:85817ms,90465ms,89622ms

//CPU:node:40%-55%,pSpace:15%-30%
//同步查询实时报警
console.time("SynQueryRealAlarm");
console.log("开始同步查询实时报警...");
for(var i=1;i<=100000;++i){
	if(i>=99000){
		con.read("/tag/test"+i+".alarm.real",0,0);
	}else{
		con.read("/tag"+i+".alarm.real",0,0);
	}
}
console.log("完成同步查询实时报警.");
console.timeEnd("SynQueryRealAlarm");
//CPU： 29173ms,28267ms,29016ms,29780ms

var t = new Date().getTime();
var d = new Date(t-3600);
var startTime = add.formatDate(d);
var endTime = add.getCurDate();
var hisFilter = {
	'filterField':{"TagId":0,"AlarmHaveAcked":1},
	'filter':{"AlarmHaveAcked":0},
	'startTime':startTime,
	'endTime':endTime
}
//同步查询历史报警
console.time("SynQueryHisAlarm");
console.log("开始同步查询历史报警...");
for(var i=1;i<=100000;++i){
	if(i>=99000){
		con.read("/tag/test"+i+".alarm.his",hisFilter);
	}else{
		con.read("/tag"+i+".alarm.his",hisFilter);
	}
}
console.log("完成同步查询历史报警!");
console.timeEnd("SynQueryHisAlarm");
//time:32011ms,31823ms
//同步查询应答报警
var ack = {
	"ackUserId":1,
	"ackUserName":"Admin",
	"ackTime":endTime,
	"ackIds":[2]
}
var ackeIds = [2];

console.time("SynQueryAckAlarm");
console.log("开始同步查询应答报警...");
for(var i=1;i<=100000;++i){
	if(i>=99000){
		con.read("/tag/test"+i+".alarm.ack",ack);
	}else{
		con.read("/tag"+i+".alarm.ack",ack);
	}
}
console.log("完成同步查询应答报警!");
console.timeEnd("SynQueryAckAlarm");
//32745ms,33181ms
/*
//同步删除测点
console.time("SynDel");
console.log ("开始删除测点...");
for(var i=1;i<99000;i++){
	var resDelSyn = con.del("/tag"+i);
}
var resDelSyn = con.del("/tag");
console.log("删除测点完毕.");
console.timeEnd("SynDel");
*/