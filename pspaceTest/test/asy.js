var addon = require("../lib/pSpace");
var async = require("async");
var ps = new addon.Client();
var con = ps.connect("localhost","admin","admin888");
if(con.hasOwnProperty("errString"))
{
	console.log("错误码:",con.code);
	console.log("服务器连接失败:",con.errString);
	throw "connect fail!";
}
console.log();
var writeData  = {
	"value":67.001,
	"quality":0,
	"time":"2015-01-21 00:20:0.123"
};

var fun1 = function ()
{
	var res = con.read("/tag.pv",function(err,pv){
	if(err){
		console.log(err);
	}else{
		console.log(pv);
		console.log(pv.value);
	}
	});
}
var fun2 = function()
{

	var resAsy = con.write("/tag.pv",writeData, function(err,tagName){
	if(err){
		console.log(err);
	}else{
		console.log(tagName,":异步写实时成功");
	}
	});
}
var fun3 = function()
{
	con.close();
}

/*
async.series({
    one: function(callback){
    	callback(null,fun1());
    },
    two: function(callback){
    	callback(null,fun2());
    },
    three:function(callback){
    	con.close();
    }
},
function(err,results){
   //console.log(results);
});

*/

/*异步读取实时数据
 *参数：
 *	第二个参数是一个回调函数，回调函数第一个参数是错误信息，第二个参数是读取到的实时值对象
 */
 /*
 var res = con.read("/tag.pv",function(err,pv){
	if(err){
		console.log(err);
	}else{
		console.log(pv);
		console.log(pv.value);
	}
});
*/
//fun1();
//fun2();
con.close();
/*
 *对某一个测点进行实时写，提供同步异步两种方式
 *参数提供json串和直接传两种方式
 *参数中要写入的实时值必须提供，质量戳和时间戳为可选参数
 *返回值：
 *	写成功，返回测点长名
 *	写失败，返回错误对象，对象包含两个属性:
 *		错误码：code,错误信息:errString
 */

 var pv = con.read("/tag.pv");
if(pv.hasOwnProperty("errString")){
	console.log(pv.code);
	console.log(pv.errString);
}else{
	console.log("同步读实时成功!");
	console.log(pv);
	console.log(pv.value);
	console.log(pv.quality);
	console.log(pv.time);
}

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
	console.log(resWrite1,":同步写实时成功");
}

 var resAsy = con.write("/tag.pv",writeData, function(err,tagName){
	if(err){
		console.log(err);
	}else{
		console.log(tagName);
		console.log(tagName,":异步写实时成功");
	}
});

var res = con.read("/tag.pv",function(err,pv){
	if(err){
		console.log(err);
	}else{
		console.log("异步读实时成功");
		console.log(pv);
		console.log(pv.value);
	}
});


console.log("I 按摩");