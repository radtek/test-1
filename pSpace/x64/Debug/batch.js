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
var batRead = ["/tag/tag1.pv","/tag/tag2.pv","/tag/tag3.pv"];
var resVal = con.read(batRead);
if(resVal instanceof Err){
	console.log(reaVal.errString);
}else{
	console.log(resVal);
}

//异步批量读取
var val = con.read(batRead,function(err,value){
	if(err){
		console.log(err);
	}else{
		console.log(value);
	}

});
*/
//同步批量写实时值
var data  = {
	"value":830.001,
	"quality":"good",
	"time":new Date("2014-07-14T15:37:00+08:00")
};
var data1 = {
	"value":29.890,
	"quality":"good",
	
};

var batWrite = {"/tag/tag1.pv":data,"/tag/tag2.pv":data1};

var res = con.write(batWrite);
if(res instanceof Err){
	console.log(res.errString);
}else{
	console.log("批量写实时成功!");
}

/*
//异步批量写
var res1 = con.write(batWrite,function(err,nu){
	if(err){
		console.log(err);
	}else{
		console.log("异步批量写实时成功！");
	}
});
*/
