var addon = require("../lib/pSpace");
var async = require('async');
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
console.log("开始添加10万个测点，其中一级测点98999个，二级测点1000个，测点类型为模拟量...");
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
console.log("添加测点完毕，开始测试.....");
*/
//实时写测试
var data  = {
	"value":67.001,
	"quality":0,
	"time":"2014-01-21 00:20:0.123"
};

var data1 = {
	"value":78.890,
	"quality":1
};
/*
//node:45%-60%,pSpace:15%-22%
console.time("realWriteIncludeTimeSyn");
console.log("开始同步写实时值测试，写入的值不带时间戳.....");
//带时间戳
for(var i=1;i<=100000;++i){
	if(i>=99000){
		var resWrite = con.write("/tag/test"+i+".pv",data);
	}else{
		var resWrite = con.write("/tag"+i+".pv",data);
	}
}
console.log("完成不带时间戳的同步写测试.....");
console.timeEnd("realWriteIncludeTimeSyn");
//CPU:65%-80%,time:24557ms,24240ms,23951ms

//不带时间戳
console.time("realWriteNotIncludeTimeSyn");
console.log("开始同步步写实时值测试，写入的值带时间戳.....");
for(i=1;i<=100000;++i){
	if(i>=99000){
		var resWrite = con.write("/tag/test"+i+".pv",data1);
	}else{
		var resWrite = con.write("/tag"+i+".pv",data1);
	}
}
console.log("完成带时间戳的同步写测试.....");
console.timeEnd("realWriteNotIncludeTimeSyn");
//CPU:65%-80%,time:23462ms,23225ms,23569ms
*/

//直接传参
console.time("realWriteIncludeTimeSynNojson");
console.log("开始同步直接传参写实时测试.....");
for(var i=1;i<=100000;++i){
	if(i>=99000){
		var resWrite = con.write("/tag/test"+i+".pv",89.78,0,"2014-03-19 07:20:0.123");
	}else{
		var resWrite = con.write("/tag"+i+".pv",89.78,0,"2014-03-19 07:20:0.123");
	}
}
console.log("完成直接传参的同步写实时测试.....");
console.timeEnd("realWriteIncludeTimeSynNojson");
//CPU:65%-80%,time:22823ms,23985ms,24195

/*
console.time("readSyn");
console.log("开始同步实时读数据测试....");
for (var i = 1; i <= 100000; ++i) {
	if(i>=99000){
		var pv = con.read("/tag/test"+i+".pv");
		if(pv instanceof Err){
			console.log(pv.errString);
		}else{
			//console.log(pv);
			//console.log(pv.value);
			//console.log(pv.quality);
			//console.log(pv.time);
		}
	}else{
		var pv = con.read("/tag"+i+".pv");
		if(pv instanceof Err){
			console.log(pv.errString);
		}else{
			//console.log(pv);
			//console.log(pv.value);
			//console.log(pv.quality);
			//console.log(pv.time);
		}
	}	
};
console.log("完成同步读实时数据测试。");
console.timeEnd("readSyn");
//CPU:65%~80%,time:22355ms,22916ms,23077ms,23328ms,23041ms,23524ms
*/



/*
//同步读同步写做计算后写入
console.time("SynReadSynWrite:ai=ai*256");
for(var i=1;i<=100000;++i){
	if(i>=99000){
		var value1 = con.read("/tag/test"+i+".pv").value;
		var resWrite = con.write("/tag/test"+i+".pv",value1*256);
	}else{
		var v1 = con.read("/tag"+i+".pv").value;
		var resWrite = con.write("/tag"+i+".pv",(v1*256));
	}
}
console.timeEnd("SynReadSynWrite:ai=ai*256");
//CPU:65%~80%,time:45426ms,45976ms,45802ms,46208ms
*/

/*
//同步读同步写
console.time("SynReadSynWrite:a5=(a1+a2)*a3/a4");
for(var i=1;i<=100000;++i){
	if(i>=99000){
		if(i+4<=100000){
			var value1 = con.read("/tag/test"+i+".pv").value;
			var value2 = con.read("/tag/test"+(i+1)+".pv").value;
			var value3 = con.read("/tag/test"+(i+2)+".pv").value;
			var value4 = con.read("/tag/test"+(i+3)+".pv").value;
			var resWrite = con.write("/tag/test"+(i+4)+".pv",(value1+value2)*value3/value4);
		}	
	}else{
		if(i+4<99000){
			var v1 = con.read("/tag"+i+".pv").value;
			var v2 = con.read("/tag"+(i+1)+".pv").value;
			var v3 = con.read("/tag"+(i+2)+".pv").value;
			var v4 = con.read("/tag"+(i+3)+".pv").value;
			var resWrite = con.write("/tag"+(i+4)+".pv",(v1+v2)*v3/v4);
		}
			
	}
}
console.timeEnd("SynReadSynWrite:a5=(a1+a2)*a3/a4");
//CPU:65%-80%,time:114567ms,112397ms,114692ms,114801ms,115690ms,122367ms
*/
/*
console.time("SynReadsynWrite:a[i+4]=(a[i]+a[i+1])*a[i+2]");
for(var i=1;i<=100000;++i){
	if(i>=99000){
		if(i+4<=100000){
			var value1 = con.read("/tag/test"+i+".pv").value;
			var value2 = con.read("/tag/test"+(i+1)+".pv").value;
			var value3 = con.read("/tag/test"+(i+2)+".pv").value;
			var value4 = con.read("/tag/test"+(i+3)+".pv").value;
			var resWrite = con.write("/tag/test"+(i+4)+".pv",(value1+value2)*value3);
		}	
	}else{
		if(i+4<99000){
			var v1 = con.read("/tag"+i+".pv").value;
			var v2 = con.read("/tag"+(i+1)+".pv").value;
			var v3 = con.read("/tag"+(i+2)+".pv").value;
			var v4 = con.read("/tag"+(i+3)+".pv").value;
			var resWrite = con.write("/tag"+(i+4)+".pv",(v1+v2)*v3);
		}
			
	}
}
console.timeEnd("SynReadsynWrite:a[i+4]=(a[i]+a[i+1])*a[i+2]");
//CPU:65%-75%,time:112975ms,115003ms,114084ms,114535ms
*/
/*
//同步删除测点
console.time("SynDel");
console.log("测试完毕，开始删除测点.......");
for(var i=1;i<=98999;i++){
	con.del("/tag"+i);
}
var resDelSyn = con.del("/tag");
console.log("删除测点完毕,断开连接...");
con.close();
console.timeEnd("SynDel");
*/


