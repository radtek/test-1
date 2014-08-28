var addon = require("../../lib/pSpace");
var events = require("events"); 
var async = require("async");
var csv  = require('csv');
var oracle  = require('oracle');
var emitter = new events.EventEmitter(); 
var ps = new addon.Client();
var Err = addon.Error;

function strtotime(s){ 
	var  d  =  new  Date(Date.parse(s));
	return d;
}
var con = ps.connect("localhost","admin","admin888");
if(con instanceof Err){
	console.log("错误码:",con.code);
	console.log("服务器连接失败:",con.errString);
	throw "connect fail!";
}else{
	console.log("连接成功!");
}
console.log();
for(var i=1;i<=100;i++){
	var tag4 = {
	"propID":["PS_TAG_PROP_NAME","PS_TAG_PROP_TAGTYPE","PS_TAG_PROP_DATATYPE","PS_TAG_PROP_DESCRIPTION"],
	"dataType":["PSDATATYPE_STRING","PSDATATYPE_UINT16","PSDATATYPE_UINT8","PSDATATYPE_STRING"],
	"values":["tag"+i,2,"PSDATATYPE_DOUBLE","psNode测试"]
	};
	var resAddSyn4 = con.add("/tag"+i,tag4);
	//if(resAddSyn4 instanceof Err){
		//console.log("添加测点错误:",resAddSyn4.errString);
	//}else{
	//	console.log("添加测点:",resAddSyn4,"成功!");
	//}
}
