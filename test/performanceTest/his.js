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

console.log("先添加10万个点，一级测点98999个二级测点1000个，测点类型为模拟量...");
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
//62758ms
console.log("添加测点完毕，开始历史相关接口测试........");
//同步设置所有测点为保存历史
console.log("先设置所有测点属性为保存历史...");
/*
console.time("SynSetProps");

console.log("开始同步设置测点保存历史的属性....");
var setID = ["PS_TAG_PROP_HIS_ISSAVE"];
var setDataType = ["PSDATATYPE_BOOL"];
var setValues = [1];
for(var i=1;i<=100000;++i){
	if(i>=99000){
		con.write("/tag/test"+i+".props",setID,setDataType,setValues);
	}else{
		con.write("/tag"+i+".props",setID,setDataType,setValues);
	}
}
console.log("同步设置所有测点为保存历史的属性完毕.");
console.timeEnd("SynSetProps");
//CPU:50%-%65,time:85817ms,78182ms,76595ms
*/

//同步对100000测点插入历史数据
var time = add.getCurDate();
var histry = {
	"value":12.001,
	"time":time,
	"quality":"PS_QUALITY_RAW"
};
/*
console.time('SynInsert');
console.log("开始同步插入历史数据....");
for(var i=1;i<=100000;++i){
	if(i>=99000)
		con.write("/tag/test"+i+".his.insert",histry);
	else
		con.write("/tag"+i+".his.insert",histry);
}
console.log("同步插入历史数据完毕");
console.timeEnd('SynInsert');
//CPU:50%-65%   time:54717ms,53832ms,52588ms,52230ms,51839ms,52338ms

var histry1 = {
	"value":55.55,
	"time":time,
	"quality":"PS_QUALITY_RAW"
};
console.time("AsyInsert");

console.time("SynInsertRep");
console.log("开始同步插入替换历史数据....");
for(var i=1;i<=100000;++i){
	if(i>=99000)
		con.write("/tag/test"+i+".his.insertReplace",histry);
	else
		con.write("/tag"+i+".his.insertReplace",histry);
}
console.log("同步插入替换历史数据完毕.");
console.timeEnd("SynInsertRep");
//CPU:55%-65%  time:52303ms,52899,52775ms,52120ms,52291ms,52479ms




var histry2 = {
	"value":56.55,
	"time":time,
	"quality":"PS_QUALITY_RAW"
};

console.time("SynRep");
console.log("开始同步替换历史数据....");
for(var i=1;i<=100000;++i){
	if(i>=99000)
		con.write("/tag/test"+i+".his.replace",histry2);
	else
		con.write("/tag"+i+".his.replace",histry2);
}
console.log("完成同步替换历史数据.");
console.timeEnd("SynRep");

//CPU:55%-65% time:52355ms,51620ms,52167ms,51417ms,51668ms,51782ms
*/

//同步查询某一测点固定时间历史数据
console.time("SynQueryHisAtTime");
console.log("开始同步查询所有测点某一时间的历史数据...");

var times = [time];

for(var i=1;i<=100000;++i){
	if(i>=99000){
		var res = con.read("/tag/test"+i+".his.atTime",times);
		if(res instanceof Err){
			console.log(res.errString);
		}else{
			//for(i=0;i<res.length;i++)
				//console.log(res[i].value);
		}
	}else{
		con.read("/tag"+i+".his.atTime",times);
		if(res instanceof Err){
			console.log(res.errString);
		}else{
			//for(i=0;i<res.length;i++)
				//console.log(res[i].value);
		}
	}		
}
console.log("完成同步查询所有测点某一时间的历史数据。");
console.timeEnd("SynQueryHisAtTime");
//53988ms,53274ms
//同步查询某一测点固定时间历史数据



var t = new Date().getTime();
var d = new Date(t-3600);
var startTime = add.formatDate(d);
var endTime = add.getCurDate();
console.time("SynQueryHisRaw");
console.log("开始同步查询所有测点一段时间的原始数据....");
for(var i=1;i<=100000;++i){
	if(i>=99000){
		var res = con.read("/tag/test"+i+".his.raw",startTime,endTime,1);
		if(res instanceof Err){
			console.log(res.errString);
		}else{
			//for(i=0;i<res.length;i++)
				//console.log(res[i].value);
		}
	}else{
		con.read("/tag"+i+".his.raw",startTime,endTime,1);
		if(res instanceof Err){
			console.log(res.errString);
		}else{
			//for(i=0;i<res.length;i++)
				//console.log(res[i].value);
		}
	}		
}
console.log("完成同步查询所有测点一段时间的历史数据.");
console.timeEnd("SynQueryHisRaw");
//58047ms,59060ms

var hisPro = {
	"startTime":startTime,
	"endTime":endTime,
	"resampleInterval":10000000,
	"aggregates":"PS_HIS_START"
};
console.time("SynQueryHisProcessed");
console.log("开始同步查询所有测点一段时间的统计数据....");
for(var i=1;i<=100000;++i){
	if(i>=99000){
		var res = con.read("/tag/test"+i+".his.processed",hisPro);
		if(res instanceof Err){
			console.log(res.errString);
		}else{
			
			//console.log(res);
		}
	}else{
		con.read("/tag"+i+".his.processed",hisPro);
		if(res instanceof Err){
			console.log(res.errString);
		}else{
			//console.log(res);
		}
	}		
}
console.log("完成同步查询所有测点一段时间的统计数据.");
console.timeEnd("SynQueryHisProcessed");
//56721ms,56363ms,60482ms,62774


/*
//同步删除测点
console.time("SynDel");
console.log("测试完毕开始删除测点.......");
for(var i=1;i<99000;i++){
	var resDelSyn = con.del("/tag"+i);
}
var resDelSyn = con.del("/tag");
con.close();
console.timeEnd("SynDel");
//75099ms
*/