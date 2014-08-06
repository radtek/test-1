
var addon = require("./pSpace");
//var bat = require("../lib/tag.js");
var events = require( "events" );
var cluster = require('cluster'); 
var child_process=require('child_process');
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
process.on('message', function(m) {
	if(m){
		connect();
	} 
});

var fun = function()
{
	if(ps.isConnected()){
		process.send(con);
	}else{
		con = ps.connect("localhost","admin","admin888");
		process.send(con);
	}
	
}
setInterval(fun,1000);