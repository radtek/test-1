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
/*
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
*/
//异步设置所有测点为报警属性
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
console.time('AsySetAlarmProps');
/*
console.log("异步设置所有测点报警属性....");
for(var i=1;i<=100000;++i){
	if(i>=99000){
		(function(idx){
			con.write("/tag/test"+idx+".props",setID,setDataType,setValues,function(err,tagName,tagID){
			if(idx>99999)
				console.timeEnd("AsySetAlarmProps");
			});
		})(i);
		
	}else{
		con.write("/tag"+i+".props",setID,setDataType,setValues,function(err,tagName,tagID){});
	}
}
console.log("报警属性设置完毕,开始报警性能测试.......");
//CPU:node 45%-60%,pSpace:1%-10%,time:85729ms
*/

//异步查询实时报警
console.time("AsyQueryRealAlarm");
for(var i=1;i<=100000;++i){
	if(i>=99000){
		(function(idx){
			con.read("/tag/test"+idx+".alarm.real",0,0,function(err,resVal){
				if(idx>99999)
					console.timeEnd("AsyQueryRealAlarm");
			});
		})(i);
		
	}else{
		con.read("/tag"+i+".alarm.real",0,0,function(err,resVal){});
	}
}
//CPU:node:65%-75%,pSpace:25%-35%,time:20904ms,20951ms

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

/*
//异步查询历史报警


console.time("AsyQueryHisAlarm");
for(var i=1;i<=100000;++i){
	if(i>=99000){
		(function(idx){
			con.read("/tag/test"+idx+".alarm.his",hisFilter,function(err,resVal){
				if(idx>99999)
					console.timeEnd("AsyQueryHisAlarm");
			});
		})(i);
		
	}else{
		con.read("/tag"+i+".alarm.his",hisFilter,function(err,resVal){});
	}
}
//CPU:node:60%-70% pSpace:25%-35%,time:23759ms,23680ms
*/

var ack = {
	"ackUserId":1,
	"ackUserName":"Admin",
	"ackTime":endTime,
	"ackIds":[2]
}

//异步查询应答报警
console.time("AsyQueryAckAlarm");
for(var i=1;i<=100000;++i){
	if(i>=99000){
		(function(idx){
			con.read("/tag/test"+idx+".alarm.ack",ack,function(err,resVal){
				if(idx>99999)
					console.timeEnd("AsyQueryAckAlarm");
			});
		})(i);
		
	}else{
		con.read("/tag"+i+".alarm.ack",ack,function(err,resVal){});
	}
}
//CPU:node:60%-70%,pSpace:35%-45%,time:20186ms,19843ms,20326ms
