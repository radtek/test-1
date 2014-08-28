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
var tag = {
		"propID":["PS_TAG_PROP_NAME","PS_TAG_PROP_TAGTYPE"],
		"dataType":["PSDATATYPE_STRING","PSDATATYPE_UINT16"],
		"values":["tag",0]
		};
/*
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

//62758ms
console.log("添加测点完毕，开始历史相关接口测试........");
*/
//异步设置所有测点为保存历史
var setID = ["PS_TAG_PROP_HIS_ISSAVE"];
var setDataType = ["PSDATATYPE_BOOL"];
var setValues = [1];
/*
console.time('AsySetProps');
console.log("开始异步设置所有测点属性为保存历史....");
for(var i=1;i<=100000;++i){
	if(i>=99000){
		(function(idx){
			con.write("/tag/test"+idx+".props",setID,setDataType,setValues,function(err,tagName,tagID){
				if(idx>99999)
					console.timeEnd("AsySetProps");
			});
		})(i);
		
	}else{
		con.write("/tag"+i+".props",setID,setDataType,setValues,function(err,tagName,tagID){});
	}
}
console.log("异步设置所有测点属性为保存历史完毕.");
//CPU:node:50%-60%,pSpace:8%-15%,69264ms
*/
var time = add.getCurDate();
var histry = {
	"value":12.001,
	"time":time,
	"quality":"PS_QUALITY_RAW"
};
var histry1 = {
	"value":55.55,
	"time":time,
	"quality":"PS_QUALITY_RAW"
};
/*
console.log("开始异步插入历史数据....");
console.time("AsyInsert");
for(var i=1;i<=100000;++i){
	if(i>=99000)
		(function(idx){
			con.write("/tag/test"+idx+".his.insert",histry1,function(err,tagName,tagID){
				if(idx>99999)
					console.timeEnd("AsyInsert");
			});
		})(i);
	else
		con.write("/tag"+i+".his.insert",histry1,function(err,tagName,tagID){});
}
console.log("异步插入历史数据完毕.....");

//CPU:node:55%-%65,pSpace:10%-20% time:46893ms,45240,45474ms,45770ms,46222ms,47596ms
*/
/*
console.time("AsyInsertRep");
console.log("开始异步插入替换历史数据测试...");
for(var i=1;i<=100000;++i){
	if(i>=99000)
		(function(idx){
			con.write("/tag/test"+idx+".his.insertReplace",histry1,function(err,tagName,tagID){
				if(idx>99999)
					console.timeEnd("AsyInsertRep");
			});
		})(i);
		
	else
		con.write("/tag"+i+".his.insertReplace",histry1,function(err,tagName,tagID){});
}
console.log("完成异步插入替换历史数据测试.");
//CPU:node:55%-%65,pSpace:10%-20% time:47580ms,46378,,48032ms,46301ms
*/

console.time("AsyRep");
console.log("开始异步替换历史数据...");
for(var i=1;i<=100000;++i){
	if(i>=99000)
		(function(idx){
			con.write("/tag/test"+idx+".his.replace",histry,function(err,tagName,tagID){
				if(idx>99999)
					console.timeEnd("AsyRep");
			});
		})(i);	
	else
		con.write("/tag"+i+".his.replace",histry,function(err,tagName,tagID){});
}
console.log("完成异步替换历史数据...");
//CPU:node:55%-%65,pSpace:10%-20% time:47265ms,46141ms,46677ms,46653


var histry2 = {
	"value":56.55,
	"time":time,
	"quality":"PS_QUALITY_RAW"
};
var times = [time];
/*
console.time("AsyQueryHisAtTime");
console.log("开始异步查询所有测点某一时间的历史数据...");
for(var i=1;i<=100000;++i){
	if(i>=99000){
		(function(idx){
			con.read("/tag/test"+idx+".his.atTime",times,function(err,val){
				if(idx>99999)
					console.timeEnd("AsyQueryHisAtTime");
			});
		})(i);
		
	}else{
		con.read("/tag"+i+".his.atTime",times,function(err,val){
			//for(i=0;i<val.length;i++)
				//console.log(val[i].value);
		});	
	}		
}
console.log("完成异步查询所有测点某一时间的历史数据。");
//CPU:node:50%-%65,pSpace:8%-20% time:48328ms
*/

var t = new Date().getTime();
var d = new Date(t-3600);
var startTime = add.formatDate(d);
var endTime = add.getCurDate();
/*
console.time("AsyQueryHisRaw");
console.log("开始异步查询所有测点某一段时间的原始数据...");
for(var i=1;i<=100000;++i){
	if(i>=99000){
		(function(idx){
			con.read("/tag/test"+idx+".his.raw",startTime,endTime,1,function(err,val){
				if(idx>99999)
					console.timeEnd("AsyQueryHisRaw");
			});
		})(i);
	}else{
		con.read("/tag"+i+".his.raw",startTime,endTime,1,function(err,val){
			//for(i=0;i<val.length;i++)
				//console.log(val[i].value);
		});	
	}		
}
console.log("完成异步查询所有测点某一段时间的原始数据。");
//CPU:node:50%-%65,pSpace:8%-20% time:47424ms,48890ms
*/
var hisPro = {
	"startTime":startTime,
	"endTime":endTime,
	"resampleInterval":10000000,
	"aggregates":"PS_HIS_START"
};

console.time("AsyQueryHisProcessed");
console.log("开始异步查询所有测点某一段时间的历史统计数据...");
for(var i=1;i<=100000;++i){
	if(i>=99000){
		(function(idx){
			con.read("/tag/test"+idx+".his.processed",hisPro,function(err,val){
				if(idx>99999)
					console.timeEnd("AsyQueryHisProcessed");
		});
		})(i);
		
	}else{
		con.read("/tag"+i+".his.processed",hisPro,function(err,val){
			//console.log(val);
		});	
	}		
}
console.log("完成异步查询所有测点某一段时间的历史统计数据。");
//49576ms,49063ms
