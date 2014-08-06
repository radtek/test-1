
var addon = require("./pSpace");
//var bat = require("../lib/tag.js");
var events = require( "events" );
var event = new events.EventEmitter(); 
var ps = new addon.Client();
var Err = addon.Error;
var con;
function connect(){
	con = ps.connect("localhost","admin","admin888");
	if(con instanceof Err){
	console.log("错误码:",con.code);
	console.log("服务器连接失败:",con.errString);
	}else{
		console.log("连接成功!");
	}
}
connect();
/*
event.on('connect',connect);
var doWrite = function(){
	if(ps.isConnected()){
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
		event.emit("connect");
	}
}
setInterval(doWrite,1000);
*/
//con.write("/tag/tag1.pv",23.234);

con.write("/tag/tag1.pv",23.4,function(err,tagName){
	if(err){
		console.log(err);
	}else{
		console.log(tagName);
	}

});
