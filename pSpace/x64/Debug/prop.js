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
var res = con.read("/tag/tag1.pv",function(err,tagName,pv){
	if(err){
		console.log(err);
	}else{
		console.log(pv);
	}
});