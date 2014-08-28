var addon = require("../build/pSpace");
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

var rand = {};
rand.get = function (begin,end){
 return Math.floor(Math.random()*(end-begin))+begin;
}

var v = rand.get(10,100)
var data = Math.sin(v);

var doWrite = function()
{
	var resWrite = con.write("/tag/tag1.pv",v);
	if(resWrite instanceof Err){
		console.log(resWrite.code);
		console.log(resWrite.errString);
	}else{
		console.log(resWrite,":写实时成功");
	}
}
setInterval(doWrite,1000);