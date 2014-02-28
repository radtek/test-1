/*
 *历史相关接口测试
 */
var addon = require("../lib/pSpace");
var ps = new addon.Client();
var Err = addon.Error;
var con = ps.connect("localhost","admin","admin888");
if(con instanceof Err){
	console.log("错误处理测试1");
}
if(con.constructor == Err){
	console.log("错误处理测试2");
}
if(con.hasOwnProperty("errString"))
{
	console.log("错误码:",con.code);
	console.log("服务器连接失败:",con.errString);
	throw "connect fail!";
}
console.log();
var histry = {
	"value":12.001,
	"time":"2008-05-13 14:28:04.123",
	"quality":"PS_QUALITY_RAW"
};
/*
 *同步插入某一测点的历史数据
 *参数：支持json格式参数和直接传入参数，直接传入参数参数顺序是value,quality,time
 *	第一个参数包含测点长名以及，his,insert标识
 *返回值：
 *	成功返回测点长名，失败返回错误信息
 *说明：判断是否返回了错误对象有三种方法：
 *	方法一：if(con.hasOwnProperty("errString"))错误表达式的值为true
 *	方法二：if(con.constructor == Err)错误表达式的值返回true
 *	方法三：if(con instaceof Err) 错误表达式返回true
 */
var resInsert = con.write("/tag.his.insert",histry);
if(resInsert.hasOwnProperty("errString")){
	console.log(resInsert.code);
	console.log(resInsert.errString);
}else{
	console.log("对测点:",resInsert,"插入历史数据成功!");
}
/*
 *异步插入某一测点的历史数据
 *参数：支持json格式参数和直接传入参数，直接传入参数参数顺序是value,quality,time
 *	第一个参数包含测点长名以及，his,insert标识
 *	第二个参数是插入值，第三个参数为回调函数，带回执行结果
 */
var resInsertAsy = con.write("/tag.his.insert",histry,function(err,tagName,tagId){
	if(err){
		console.log(err);
	}else{
		console.log("异步插入测点:",tagName,"历史数据成功.");
	}
});
/*
 *同步插入替换某一测点的历史数据
 *参数：支持json格式参数和直接传入参数，直接传入参数参数顺序是value,quality,time
 *	第一个参数包含测点长名以及，his,insertReplace标识
 *返回值：
 *	成功返回测点长名，失败返回错误信息
 */
var resInsertRep = con.write("/tag.his.insertReplace",histry);
if(resInsertRep.hasOwnProperty("errString")){
	console.log(resInsertRep.code);
	console.log(resInsertRep.errString);
}else{
	console.log("对测点:",resInsertRep,"插入替换历史数据成功!");
}


/*
 *异步插入替换某一测点的历史数据
 *参数：支持json格式参数和直接传入参数，直接传入参数参数顺序是value,quality,time
 *	第一个参数包含测点长名以及，his,insert标识
 *	第二个参数是插入值，第三个参数为回调函数，带回执行结果
 */
var resInsertRepAsy = con.write("/tag.his.insert",histry,function(err,tagName,tagId){
	if(err){
		console.log(err);
	}else{
		console.log("异步插入替换测点:",tagName,"历史数据成功.");
	}
});


/*
 *同步替换某一测点的历史数据
 *参数：支持json格式参数和直接传入参数，直接传入参数参数顺序是value,quality,time
 *	第一个参数包含测点长名以及，his,replace标识
 *返回值：
 *	成功返回测点长名，失败返回错误信息
 */
var resRep = con.write("/tag.his.replace",histry);
if(resRep.hasOwnProperty("errString")){
	console.log(resRep.errString);
}else{
	console.log("同步替换测点:",resRep,"成功!");
}
/*
 *同步替换某一测点历史数据直接传参
 */
var resRep1 = con.write("/tag.his.replace",23.324,"PS_QUALITY_RAW","2008-05-13 14:28:04.123"); 
if(resRep1.hasOwnProperty("errString")){
	console.log(resRep1.errString);
}else{
	console.log("同步替换测点:",resRep1,"成功!");
}
/*
 *异步替换某一测点的历史数据
 *参数：支持json格式参数和直接传入参数，直接传入参数参数顺序是value,quality,time
 *	第一个参数包含测点长名以及，his,replace标识
 *	第二个参数是插入值，第三个参数为回调函数，带回执行结果
 */
var resRepAsy = con.write("/tag.his.replace",histry,function(err,tagName,tagId){
	if(err){
		console.log(err);
	}else{
		console.log("异步替换测点:",tagName,"历史数据成功.");
	}
});

/*
 *同步查询某一测点固定时间历史数据
 *参数:
 *	第一个参数是测点长名和标识信息，第二个参数是查询时刻数组
 *	返回值：成功返回一个数组，每一个数组元素是一个值对象，每一个对象包含三个属性，value,quality,time
 *			失败返回错误对象
 */
/*
 *查询时刻数组
 */
var timeStamps = ["2014-01-21 14:28:04.123"];


var resAtTime= con.read("/tag.his.atTime",timeStamps);
if(resAtTime.hasOwnProperty("errString")){
	console.log("错误:",resAtTime.errString);
}else{
	for(i=0;i<resAtTime.length;i++){
		console.log(resAtTime[i].value);
	}
	
}

/*
 *异步查询某一测点固定时间历史数据
 *参数:
 *	第一个参数是测点长名和标识信息，第二个参数是查询时刻数组
 *	第三个参数是回调函数，函数第一个参数是错误信息，
 *  第二个参数返回一个数组，每一个数组元素是一个值对象，每一个对象包含三个属性，value,quality,time
 */
var resAtTime2= con.read("/tag.his.atTime",timeStamps,function(err,resVal){
	if(err){
		console.log(err);
	}else{
		console.log(resVal);
	}
});

/*
 *同步查询某一测点一段时间的原始数据
 *参数：第一个参数是测点长名和标识信息，第二个参数是查询开始时间
 *		第三个参数是查询结束时间，第四个参数是一个逻辑值，表示是否保存边界值(0:不保存1:保存)
 *返回值：
 *	失败返回错误对象，成功返回数组，每一个数组元素是一个值对象，每一个对象包含三个属性，value,quality,time
 */
var timeStart = "2012-02-01 16:42:29.000";
var timeEnd = "2014-02-017 17:42:29.000";

var resRaw =  con.read("/tag.his.raw",timeStart,timeEnd,1);
if(resRaw.hasOwnProperty("errString")){
	console.log(resRaw.errString);
}else{
	console.log("共查询到:",resRaw.length,"条数据.");
	//打印出所有数据
	console.log(resRaw);
	//将数据分类分条打印
	//console.log(resRaw[0].value);
	//console.log(resRaw[1].value);
	//console.log(resRaw[1].time);
}
/*
 *异步查询某一测点一段时间的原始数据
 *参数：第一个参数是测点长名和标识信息，第二个参数是查询开始时间
 *		第三个参数是查询结束时间，第四个参数是一个逻辑值，表示是否保存边界值(0:不保存1:保存)
 *		第五个参数是回调函数，函数第一个参数是错误信息，
 * 		第二个参数返回一个数组，每一个数组元素是一个值对象，每一个对象包含三个属性，value,quality,time
 *返回值：
 *	失败返回错误对象，成功返回数组，每一个数组元素是一个值对象，每一个对象包含三个属性，value,quality,time
 */
var resRaw1= con.read("/tag.his.raw",timeStart,timeEnd,1,function(err,resVal){
	if(err){
		console.log(err);
	}else{
		console.log(resVal);
	}
});


/*
 *同步查询某一测点一段时间的历史统计数据
 *参数： 
 *		第一个参数是测点长名和标识信息，第二个参数是查询所需的参数
 *返回值：
 *	失败返回错误对象，成功返回数组，每一个数组元素是一个值对象，每一个对象包含三个属性，value,quality,time
 */
var hisPro = {
	"startTime":"2013-01-02 12:21:59.000",
	"endTime":"2014-2-18 13:21:59.000",
	"resampleInterval":10000000,
	"aggregates":"PS_HIS_START"
};

var resProcess = con.read("/tag.his.processed",hisPro);	
if(resProcess.hasOwnProperty("errString")){
	console.log(con.errString);
}else{
	console.log("同步查询历史统计数据成功!");
	console.log(resProcess);
}	
/*
 *直接传参查询一段时间历史统计数据
 */
var resProcess1 = con.read("/tag.his.processed","2013-01-02 12:21:59.000","2014-2-18 13:21:59.000",10000000,"PS_HIS_START");	
if(resProcess1.hasOwnProperty("errString")){
	console.log(con.errString);
}else{
	console.log(resProcess1);
}	

/*
 *异步查询某一测点一段时间的历史统计数据
 *参数： 
 *		第一个参数是测点长名和标识信息，第二个参数是查询所需的参数
 *		第三个参数是回调函数，回调函数参数包括err(错误信息)和查询到的值
 */
var resProcess2 = con.read("/tag.his.processed",hisPro,function(err,resVal){
	if(err){
		console.log(err);
	}else{
		console.log("异步查询历史统计数据：");
		console.log(resVal);
	}
});


module.exports.close = function(){
	con.close();
};