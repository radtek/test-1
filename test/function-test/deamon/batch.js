var addon = require("../build/pSpace");
var bat = require("../lib/tag.js");
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

//tag
var prop ={"tagType":"node"};
//tag1
var prop1 = new Object;
prop1.tagType = "analog";
prop1.DataType = "double";
prop1.description = "测试";
//tag2
var prop2 = {"tagType":"analog","DataType":"FLOAT"};
//tag3
var prop3 = {"tagType":"digital"};
//tag4
var prop4 = {"tagType":"string","DataType":"STRING"};

var batAdd = {"/tag":prop,"/tag/tag1":prop1,"/tag/tag2":prop2,"/tag/tag3":prop3,"/tag/tag4":prop4};
//批量过程采用的同步加点
//返回添加成功的个数，返回0全部添加失败,返回true全部添加成功，返回大于0并且不等于true部分成功部分失败

var addRes = bat.add(con,batAdd);
if(0===addRes){
	console.log("加点失败!");
}else if(true===addRes){
	console.log("批量加点成功!");
}else if(addRes>0 && addRes !=true){
	console.log("批量加点部分成功部分失败");
}else{
	console.log(addRes);
}

//批量写实时值
//过程是同步操作，返回写成功的个数，返回0全部写失败，返回true全部写成功，返回大于0并且不等于true部分成功部分失败
var data  = {
	"value":33.001,
	"quality":0,
	"time":new Date(new Date().getTime())
};
var data1 = {
	"value":23.890,
	"quality":1
};
var batWrite = {"/tag/tag1.pv":data,"/tag/tag2.pv":data1,"/tag/tag3.pv":0};
var writeRes = bat.write(con,batWrite);
if(writeRes===0){
	console.log("写实时值失败!");
}else if(true ===writeRes){
	console.log("批量写实时值成功!");
}else if(writeRes>0 && writeRes!=true){
	console.log("批量写部分成功部分失败.");
}else{
	console.log(writeRes);
}
//批量读实时值
//过程是同步操作，返回一个数组，数组的每一个元素对应着参数数组的的一个测点值对象，每个值对象包含value,quality,time三个属性
var batRead = ["/tag/tag1.pv","/tag/tag2.pv","/tag/tag3.pv"];
var readRes = bat.read(con,batRead);
if(readRes.length===0){
	console.log("读实时值失败!");
}else if(readRes.length ===batRead.length){
	console.log("批量读实时值成功:");
	console.log(readRes);
}else if(readRes.length!=0 && readRes.length!=batRead.length){
	console.log("批量读部分成功部分失败.");
}else{
	console.log(res);
}






