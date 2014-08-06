var addon = require("./pSpace");
var async = require('async');
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
/*
for(var i=1;i<=5;i++){
	(function(idx){

	})(i);
	con.read("/tag"+i+".pv",function(err,pv){
		for(i=1;i<=5;i++){
			con.write("/tag"+i+".pv",pv.value,function(err,pv){});
		}	
	});
}
*/

var j=1;
for(var i=1;i<=5;i++){
	con.read("/tag"+i+".pv",function(err,pv){
		(function(idx){
			console.log(idx);
			con.write("/tag"+idx+".pv",pv.value,function(err,pv){});
	})(j++);	
	});
}