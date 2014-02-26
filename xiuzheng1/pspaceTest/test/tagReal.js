/*
 *实时读写接口测试
 */
var addon = require("../lib/pSpace");
var ps = new addon.Client();
var con = ps.connect("localhost","admin","admin888");
if(con.hasOwnProperty("errString"))
{
	console.log("错误码:",con.code);
	console.log("服务器连接失败:",con.errString);
	throw "connect fail!";
}
console.log();
/*
 *读取某一个测点的实时值，提供同步异步两种读取方式
 *参数：
 *	/tag是测点长名，pv是读取实时值标识
 *返回值：
 *	读取成功，返回读取到的数据对象，
 *		这个数据对象包含三个属性：value,quality,time
 *	读取失败，返回错误对象，对象包含两个属性:
 *		错误码：code,错误信息:errString
 */
 /*
  *同步读取实时数据
  */
var pv = con.read("/tag.pv");
if(pv.hasOwnProperty("errString")){
	console.log(pv.code);
	console.log(pv.errString);
}else{
	console.log(pv);
	console.log(pv.value);
	console.log(pv.quality);
	console.log(pv.time);
}
/*
 *异步读取实时数据
 *参数：
 *	第二个参数是一个回调函数，回调函数第一个参数是错误信息，第二个参数是读取到的实时值对象
 */
 var res = con.read("/tag.pv",function(err,pv){
	if(err){
		console.log(err);
	}else{
		console.log(pv);
		console.log(pv.value);
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
 */
var writeData  = {
	"value":67.001,
	"quality":0,
	"time":"2015-01-21 00:20:0.123"
};
/*
 *同步写
 */
var resWrite1 = con.write("/tag.pv",writeData);
if(resWrite1.hasOwnProperty("errString")){
	console.log(resWrite1.code);
	console.log(resWrite1.errString);
}else{
	console.log(resWrite1,":写实时成功");
}
/*
 *直接传参
 */
 var resWrite2 = con.write("/tag.pv",89.78);
if(resWrite2.hasOwnProperty("errString")){
	console.log(resWrite2.code);
	console.log(resWrite2.errString);
}else{
	console.log(resWrite2,":写实时成功");
}

/*
 *异步写
 *参数：
 *	第三个参数是回调函数，回调函数用来返回执行结果吧，
 *	第一个参数是错误信息，第二个参数是测点长命
 */
 var resAsy = con.write("/tag.pv",writeData, function(err,tagName){
	if(err){
		console.log(err);
	}else{
		console.log(tagName,":异步写实时成功");
	}
});

module.exports.close = function(){
	con.close();
};