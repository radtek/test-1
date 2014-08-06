var addon = require("./pSpace");
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

var d = new Date().getTime();
var time  = new Date(d);
 var histry = {
	"value":"sadf",
	"time":time,
	"quality":"RAW"
};


var ins1 = con.write("/tag/tag4.his.insert","asyjklj","raw",time,function(){});
if(ins1 instanceof Err){
	console.log(ins1.errString);
}else{
	console.log("tag/tag1 插入历史数据成功");
}