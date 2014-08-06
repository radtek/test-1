var addon = require("./pSpace");
var ps = new addon.Client();
var Err = addon.Error;
var connectDate = {
	"hostname":"localhost",
	"user":"admin",
	"password":"admin888",
	"port":8889
};
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
var read = con.read("/tag/tag1.pv");
if(read instanceof Err){
	console.log(read.errString);
}else{
	console.log(read);
}

var res = con.read("/tag/tag4.pv",function(err,tagName,value){
	if(err){
		console.log(err);
	}else{
		console.log(value);
	}
});

var data  = {
	"value":830.001,
	"quality":"good",
	"time":new Date(new Date().getTime())
};
var data1 = {
	"value":true,
	"quality":"bad",
	//"time":new Date(new Date().getTime())
};

var res1 = con.write("/tag/tag1.pv",data);
if (res1 instanceof Err) {
	console.log(res1.errString);
}else{
	console.log("写成功");
}
var res2 = con.write("/tag/tag3.pv",data1);
if (res2 instanceof Err) {
	console.log(res2.errString);
}else{
	console.log("写成功1");
}
*/
/*
var date = new Date("2014/05/08 15:00:00");
var hisDa = {"value":12.00,"quality":"good","time":date};
var his = con.write("/tag/tag1.his.insert",hisDa);
if(his instanceof Err){
	console.log(his.code);
	console.log(his.errString);
}
else{
	console.log("历史数据插入成功!");
}
*/
/*

*/
/*
var prop2 = {"tagType":"analog","DataType":"FLOAT"};
var addSy1 = con.add("/tag/tag12",prop2,function(err,tagName){
	if(err){
		console.log(err);
	}else{
		console.log(tagName);
	}
});
con.del("/tag/tag12");
*/


var date = new Date("2014/05/16 09:56:54.397");

var date1 = new Date("2014/05/08 13:00:00");
var date2 = new Date("2014/05/08 13:01:00");
var histry = {
	"value":"true",
	"time":date1,
	"quality":"good"
};
var histry1 = {
	"value":"false",
	"time":date2,
	"quality":"good"
};

/*
//同步对某一测点插入历史数据
var resInsert = con.write("/tag/tag4.his.insert",histry);
if(resInsert instanceof Err){
	console.log(resInsert.code);
	console.log(resInsert.errString);
}else{
	console.log("对测点:",resInsert,"插入历史数据成功!");
}
var resInsert1 = con.write("/tag/tag4.his.insert",histry1);
if(resInsert1 instanceof Err){
	console.log(resInsert1.code);
	console.log(resInsert1.errString);
}else{
	console.log("对测点:",resInsert1,"插入历史数据成功!");
}

var timeStamps=[date1,date2];
con.read("/tag/tag1.his.atTime",timeStamps,function(err,resVal){
			if(err){
				console.log(err);
			}else{
				for(i=0;i<resVal.length;i++){
					console.log("查询固定时间历史数据成功:");
					console.log(resVal[0]);
				}	
			}
		});
*/
/*
var raw1 = con.read("/tag/tag1.his.raw",date1,date,1,function(err,pv){
	if(err){
		console.log(err);
	}else{
		console.log(pv);
	}
});
*/

/*
var hisPro = {
	"startTime":date1,
	"endTime":date,
	"resampleInterval":10000000,
	"aggregates":"START"
};

var resProcess = con.read("/tag/tag1.his.processed",hisPro);	
if(resProcess instanceof Err){
	console.log(resProcess.errString);
}else{
	console.log("同步查询历史统计数据成功!");
	console.log(resProcess);
}

//同步直接传参查询某一测点一段时间历史统计数据
var resProcess1 = con.read("/tag/tag1.his.processed",date1,date,10000000,"START");	
if(resProcess1 instanceof Err){
	console.log(resProcess1.errString);
}else{
	console.log("直接传参查询：");
	console.log(resProcess1);
}
	
//异步查询某一测点一段时间的历史统计数据
var resProcess2 = con.read("/tag/tag1.his.processed",hisPro,function(err,resVal){
	if(err){
		console.log(err);
	}else{
		console.log("异步查询历史统计数据：");
		console.log(resVal);
	}
});
//异步直接传参查某一测点历史统计数据
var resProcess3 = con.read("/tag/tag1.his.processed",date1,date,10000000,"START",function(err,resVal){
	if(err){
		console.log(err);
	}else{
		console.log("异步查询历史统计数据1：");
		console.log(resVal);
	}
});
*/
/*
var s1 = new Date().getTime();
var timeStart = new Date(s1-72000000);
var timeEnd = new Date(s1);
var hisPro = {
	"startTime":timeStart,
	"endTime":timeEnd,
	"resampleInterval":1000000,
	"aggregates":"PS_HIS_START"
};

var resProcess = con.read("/tag/tag1.his.processed",hisPro);	
if(resProcess.hasOwnProperty("errString")){
	console.log(con.errString);
}else{
	console.log("同步查询历史统计数据成功!");
	console.log(resProcess);
}
//异步查询某一测点一段时间的历史统计数据
var resProcess2 = con.read("/tag/tag1.his.processed",hisPro,function(err,resVal){
	if(err){
		console.log(err);
	}else{
		console.log("异步查询历史统计数据：");
		console.log(resVal);
	}
});
*/

//var sub = con.s;
/*

con.sub('/系统变量/时间变量/time.real',function(err,tagName,value){
			if(err){
				console.log(err);
			}else{
				console.log(value);
			}
		});
		*/
var res  = con.read("/tag/tag2",function(err,va,sdf){
	if(err){
		console.log(err);
	}
});
