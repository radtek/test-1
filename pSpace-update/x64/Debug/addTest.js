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


var prop = {"tagType":"analog","DataType":"DOUBLE","Description":"psNode测试"};
/*
console.time('SynAdd');
for(i=1;i<=100000;i++){
	var resAddSyn= con.add("tag"+i,prop);
}
console.timeEnd('SynAdd');
*/
console.time('AsyAdd');
for(var i=1;i<=100000;i++){
	(function(idx){
		var res= con.add("tag"+idx,prop,function(err,tagName){
			if(idx>99999)
				console.timeEnd('AsyAdd');
		});
	})(i);
}




