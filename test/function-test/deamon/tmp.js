var addon = require("../build/pSpace");
//var bat = require("../lib/tag.js");
var events = require( "events" );
var event = new events.EventEmitter(); 
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

var d = new Date().getTime();
 var time = new Date(d);
 var t = new Date();
var data  = {
	value:68.001,
	quality:"good",
	time:t
};
var data1 = {
	"value":78.890,
	"quality":1
};
//同步写
var resWrite = con.write("/tag5/tag9.pv",data);
if(resWrite instanceof Err){
	console.log(resWrite.code);
	console.log(resWrite.errString);
}else{
	console.log(resWrite,":写实时成功");
}