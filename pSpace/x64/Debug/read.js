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

var prop = {"LongName":"tag10","tagType":"analog","DataType":"double","Description":"psNode测试"};
/*
console.time('AsyAdd');
for(var i=1;i<=100000;i++){
	(function(idx){
		var res= con.add("tag"+idx,prop,function(err,tagName){
			if(idx>99999)
				console.timeEnd('AsyAdd');
		});
	})(i);
}
*/

var batch1 = [];
var batch2 = [];
var batch3 = [];

for(var i=1;i<=100000;i++){
	batch1.push("/tag"+i+".pv");
}

console.time("read");
/*
var res  =con.read(batch1,function(err,value){
	if(err){
		console.log(err);
	}else{
		console.log(value.length);
		console.timeEnd("read");
	}
});
*/
/*
var resVal = con.read(batch1);
if(resVal instanceof Err){
	console.log(resVal.errString);
}else{
	console.log(resVal.length);
	console.log(resVal[99999]);
}
console.timeEnd("read");
//14097,14111,13915ms
*/



var batch = {};
for(var i=1;i<=100000;i++){
	batch["/tag"+i] = i*0.01; 
}
//console.time("write");
//var res = con.write(batch);
//console.timeEnd("write");
/*
console.time("write");
var res1 = con.write(batch,function(err,r){
	if(err){
		console.log(err);
	}else{
		console.log("asy write succeed!");
		console.timeEnd("write")
	}
});
*/
var batRead = ["/tag/tag1.pv","/tag/tag2.pv","/tag/tag1.pv"];
/*
var value  = con.read(batRead);
if(value instanceof Err){
	console.log(value.errString);
}else{
	console.log(value);
}
*/
con.read(batRead,function(err,value){
			if(err){
				console.log(err);
			}else{
				console.log(value);
			}

		});
