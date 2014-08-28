var addon = require("../build/pSpace");
//var add = require('../lib/tag.js');
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

//模拟sleep（假死）
function sleep(milliSeconds){
    var startTime = new Date().getTime(); // get the current time
    while (new Date().getTime() < startTime + milliSeconds); // hog cpu
}
/*
 *读取某一个测点的实时值，提供同步异步两种读取方式
 *参数：
 *	/tag是测点长名，pv是读取实时值标识
 *返回值：
 *	读取成功，返回读取到的数据对象，
 *		这个数据对象包含三个属性：value,quality,time
 *	读取失败，返回错误对象，对象包含两个属性:
 *		错误码：code,错误信息:errString
 *说明：判断是否返回了错误对象有三种方法：
 *	方法一：if(con.hasOwnProperty("errString"))错误表达式的值为true
 *	方法二：if(con.constructor == Err)错误表达式的值返回true
 *	方法三：if(con instaceof Err) 错误表达式返回true
 *异步读取实时数据
 *参数：
 *	第二个参数是一个回调函数，回调函数第一个参数是错误信息，第二个参数是读取到的实时值对象
 */
 //同步读取
var pv = con.read("/tag/tag1.pv");
if(pv.hasOwnProperty("errString")){
	console.log(pv.code);
	console.log(pv.errString);
}else{
	console.log(pv);
	console.log(pv.value);
	console.log(pv.quality);
	console.log(pv.time);
}

//异步读取实时数据
var res = con.read("/tag/tag1.pv",function(err,tagName,pv){
	if(err){
		console.log(err);
	}else{
		console.log(pv);
	}
});
/*
 *对某一个测点进行实时写，提供同步异步两种方式
 *参数提供json串和直接传两种方式
 *参数中要写入的实时值必须提供，质量戳和时间戳为可选参数
 *返回值：
 *	写成功，返回测点长名
 *	写失败，返回错误对象，对象包含两个属性:
 *		错误码：code,错误信息:errString
 *
 *异步写
 *参数：
 *	第三个参数是回调函数，回调函数用来返回执行结果吧，
 *	第一个参数是错误信息，第二个参数是测点长命
 */
 
 var d = new Date().getTime();
 var time = new Date(d);
var data  = {
	"value":67.001,
	"quality":0,
	"time":time
};
var data1 = {
	"value":78.890,
	"quality":1
};
//同步写
var resWrite = con.write("/tag/tag1.pv",data);
if(resWrite instanceof Err){
	console.log(resWrite.code);
	console.log(resWrite.errString);
}else{
	console.log(resWrite,":写实时成功");
}
var resWrite1 = con.write("/tag/tag2.pv",data1);
if(resWrite1 instanceof Err){
	console.log(resWrite1.code);
	console.log(resWrite1.errString);
}else{
	console.log(resWrite1,":写实时成功");
}
//直接传参,tag3是开关量
var resWrite2 = con.write("/tag/tag3.pv",true,0,time);
if(resWrite2.hasOwnProperty("errString")){
	console.log(resWrite2.code);
	console.log(resWrite2.errString);
}else{
	console.log(resWrite2,":写实时成功");
}


//异步
var resAsy = con.write("/tag/tag1.pv",data, function(err,tagName){
	if(err){
		console.log(err);
	}else{
		console.log(tagName,":异步写实时成功");
	}
}); 
//异步直接传参
var resAsy1 = con.write("/tag/tag3.pv",false, function(err,tagName){
	if(err){
		console.log(err);
	}else{
		console.log(tagName,":异步直接传参写实时成功");
	}
}); 
//tag4是字符串
var resAsy2 = con.write("/tag/tag4.pv","string test",1, function(err,tagName){
	if(err){
		console.log(err);
	}else{
		console.log(tagName,":异步写实时成功");
	}
}); 


//con.del("/tag");