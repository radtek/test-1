
var addon = require("../build/pSpace");
var bat = require("../lib/tag.js");
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
/*
console.log();
function connect(){
	var con = ps.connect("localhost","admin","admin888");
	if(con instanceof Err){
	console.log("错误码:",con.code);
	console.log("服务器连接失败:",con.errString);
}else{
	return true;
}
}
event.on('connect',connect);

var fun = function(){
	event.emit("connect");
}

var doWrite = function(){
	if(con.isConnected()){
		console.log(con.isConnected());
		var d = new Date().getTime();
		var time = new Date(d);
		var data  = {
			"value":67.001,
			"quality":1,
			"time":time
		};
		var res = con.write("/tag/tag1.pv",data);
		if(res instanceof Err){
			console.log(res.errString);
		}else{
			console.log(res);
		}
	}else{
		setInterval(fun,3000);
	}
}

setInterval(doWrite,1000);
*/
