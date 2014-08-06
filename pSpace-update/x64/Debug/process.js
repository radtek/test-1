var addon = require("./pSpace");
//var bat = require("../lib/tag.js");
var events = require( "events" );
var cluster = require('cluster'); 
var child_process=require('child_process');
var event = new events.EventEmitter(); 
var ps = new addon.Client();
var Err = addon.Error;
function connect(){
	con = ps.connect("localhost","admin","admin888");
	if(con instanceof Err){
	console.log("错误码:",con.code);
	console.log("服务器连接失败:",con.errString);
	}else{
		console.log("连接成功!");
	}
}
//connect();
var fun = function(co)
{
	var d = new Date().getTime();
		var time = new Date(d);
		var data  = {
			"value":67.001,
			"quality":1,
			"time":time
		};
	var res = co.write("/tag/tag1.pv",data);
		if(res instanceof Err){
			console.log(res.errString);
		}else{
			console.log(res);

		}

}

var n = child_process.fork('./child.js');
n.on('message', function(m) {
  if(m){
  	var d = new Date().getTime();
		var time = new Date(d);
		var data  = {
			"value":67.001,
			"quality":1,
			"time":time
		};
  	var res = m.write("/tag/tag1.pv",data);
  	//fun(m);
  }
});




	




