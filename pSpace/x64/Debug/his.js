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
 *
 *异步插入某一测点的历史数据
 *参数：支持json格式参数和直接传入参数，直接传入参数参数顺序是value,quality,time
 *	第一个参数包含测点长名以及，his,insert标识
 *	第二个参数是插入值，第三个参数为回调函数，带回执行结果
 */
 //模拟sleep（假死）
function sleep(milliSeconds){
    var startTime = new Date().getTime(); // get the current time
    while (new Date().getTime() < startTime + milliSeconds); // hog cpu
}

var d = new Date().getTime();
var time  = new Date(d);
 var histry = {
	"value":12.001,
	"time":time,
	"quality":"RAW"
};


/*
//同步对某一测点插入历史数据
var resInsert = con.write("/tag/tag1.his.insert",histry);
if(resInsert instanceof Err){
	console.log(resInsert.code);
	console.log(resInsert.errString);
}else{
	console.log("对测点:",resInsert,"插入历史数据成功!");
}

//同步直接传参对某一测点插入历史数据
var resInsert1 = con.write("/tag/tag2.his.insert",44.532,1,time);
if(resInsert1 instanceof Err){
	console.log(resInsert1.errString);
}else{
	console.log("对测点:",resInsert1,"同步直接传参插入历史数据成功!");
}
*/
var d1 = new Date().getTime();
var time1 = new Date(d1-720000);
var histry1 = {
	"value":12.001,
	"time":time1,
	"quality":"RAW"
};

/*
//异步插入某一测点历史数据
var resInsertAsy = con.write("/tag/tag3.his.insert",histry1,function(err,tagName,tagId){
	if(err){
		console.log(err);
	}else{
		console.log("异步插入测点:",tagName,"历史数据成功.");
	}
});
//异步直接传参插入某一点历史数据
var resInsertAsy1 = con.write("/tag5/tag6.his.insert",12,0,time1,function(err,tagName,tagId){
	if(err){
		console.log(err);
	}else{
		console.log("异步插入测点:",tagName,"历史数据成功.");
	}
});
*/
/*
 *同步插入替换某一测点的历史数据
 *参数：支持json格式参数和直接传入参数，直接传入参数参数顺序是value,quality,time
 *	第一个参数包含测点长名以及，his,insertReplace标识
 *返回值：
 *	成功返回测点长名，失败返回错误信息
 *
 *异步插入替换某一测点的历史数据
 *参数：支持json格式参数和直接传入参数，直接传入参数参数顺序是value,quality,time
 *	第一个参数包含测点长名以及，his,insert标识
 *	第二个参数是插入值，第三个参数为回调函数，带回执行结果
 */
  var histry2 = {
	"value":23.001,
	"time":time,
	"quality":"raw"
};

/*
//同步插入替换
var resInsertRep = con.write("/tag5/tag7.his.insertReplace",histry2);
if(resInsertRep instanceof Err){
	console.log(resInsertRep.errString);
}else{
	console.log("对测点:",resInsertRep,"同步插入替换历史数据成功!");
}
//同步直接传参插入替换
var resInsertRep1 = con.write("/tag/tag2.his.insertReplace",67.23,"BAD",time1);
if(resInsertRep1 instanceof Err){
	console.log(resInsertRep1.errString);
}else{
	console.log("对测点:",resInsertRep1,"同步直接传参插入替换历史数据成功!");
}
sleep(2000);
//异步插入替换
var resInsertRepAsy = con.write("/tag5/tag8.his.insert",histry2,function(err,tagName,tagId){
	if(err){
		console.log(err);
	}else{
		console.log("异步插入替换测点:",tagName,"历史数据成功.");
	}
});

//异步插入替换直接传参
var resInsertRepAsy1 = con.write("/tag/tag2.his.insert",98.89,"UNCERTAIN",time,function(err,tagName,tagId){
	if(err){
		console.log(err);
	}else{
		console.log("异步插入替换测点:",tagName,"历史数据成功.");
	}
});

*/

/*
 *同步替换某一测点的历史数据
 *参数：支持json格式参数和直接传入参数，直接传入参数参数顺序是value,quality,time
 *	第一个参数包含测点长名以及，his,replace标识
 *返回值：
 *	成功返回测点长名，失败返回错误信息
 *
 *异步替换某一测点的历史数据
 *参数：支持json格式参数和直接传入参数，直接传入参数参数顺序是value,quality,time
 *	第一个参数包含测点长名以及，his,replace标识
 *	第二个参数是插入值，第三个参数为回调函数，带回执行结果
 */

 /*
//同步替换
var resRep = con.write("/tag/tag1.his.replace",histry2);
if(resRep instanceof Err){
	console.log(resRep.errString);
}else{
	console.log("同步替换测点:",resRep,"成功!");
}

//同步直接传参替换
var resRep1 = con.write("/tag/tag1.his.replace",67.89,"GOOD",time1);
if(resRep1 instanceof Err){
	console.log(resRep1.errString);
}else{
	console.log("同步替换测点:",resRep1,"成功!");
}
sleep(2000);
//异步替换
var resRepAsy = con.write("/tag/tag3.his.replace",histry2,function(err,tagName,tagId){
	if(err){
		console.log(err);
	}else{
		console.log("异步替换测点:",tagName,"历史数据成功.");
	}
});

//异步直接传参替换
var resRepAsy1 = con.write("/tag/tag2.his.replace",89.09,"MASK",time,function(err,tagName,tagId){
	if(err){
		console.log(err);
	}else{
		console.log("异步替换测点:",tagName,"历史数据成功.");
	}
});
*/



/*
 *同步查询某一测点固定时间历史数据
 *参数:
 *	第一个参数是测点长名和标识信息，第二个参数是查询时刻数组
 *	返回值：成功返回一个数组，每一个数组元素是一个值对象，每一个对象包含三个属性，value,quality,time
 *			失败返回错误对象
 *
 *异步查询某一测点固定时间历史数据
 *参数:
 *	第一个参数是测点长名和标识信息，第二个参数是查询时刻数组
 *	第三个参数是回调函数，函数第一个参数是错误信息，
 *  第二个参数返回一个数组，每一个数组元素是一个值对象，每一个对象包含三个属性，value,quality,time
 */

var d2 = new Date().getTime();
var time2 = new Date(d2-3600000);
var timeStamps = [time1];
var times = [time,time1];
/*
//同步查询某一测点固定时间历史数据
var resAtTime= con.read("/tag/tag2.his.atTime",timeStamps);
if(resAtTime.hasOwnProperty("errString")){
	console.log("错误:",resAtTime.errString);
}else{
	for(i=0;i<resAtTime.length;i++){
		console.log(resAtTime[i].value);
	}
}
sleep(1000);
//异步查询某一测点固定时间历史数据
var resAtTime2= con.read("/tag/tag4.his.atTime",times,function(err,resVal){
	if(err){
		console.log(err);
	}else{
		for(i=0;i<resAtTime2.length;i++)
			console.log(resAtTime2[i].value);
	}

});
*/
/*
 *同步查询某一测点一段时间的原始数据
 *参数：第一个参数是测点长名和标识信息，第二个参数是查询开始时间
 *		第三个参数是查询结束时间，第四个参数是一个逻辑值，表示是否保存边界值(0:不保存1:保存)
 *返回值：
 *	失败返回错误对象，成功返回数组，每一个数组元素是一个值对象，每一个对象包含三个属性，value,quality,time
 *
 *异步查询某一测点一段时间的原始数据
 *参数：第一个参数是测点长名和标识信息，第二个参数是查询开始时间
 *		第三个参数是查询结束时间，第四个参数是一个逻辑值，表示是否保存边界值(0:不保存1:保存)
 *		第五个参数是回调函数，函数第一个参数是错误信息，
 * 		回调函数第二个参数返回一个数组，每一个数组元素是一个值对象，每一个对象包含三个属性，value,quality,time
 *返回值：
 *	失败返回错误对象，成功返回数组，每一个数组元素是一个值对象，每一个对象包含三个属性，value,quality,time
 */
 /*
//同步查询某一测点的一段时间的原始数据
var timeStart = time2;
var timeEnd = time1;
var resRaw =  con.read("/tag/tag1.his.raw",timeStart,timeEnd,1);
if(resRaw instanceof Err){
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
sleep(1000);
//异步查询某一测点一段时间内的原始数据
var resRaw1= con.read("/tag/tag4.his.raw",timeStart,timeEnd,1,function(err,resVal){
	if(err){
		console.log(err);
	}else{
		console.log(resVal.length);
	}
});
*/
/*
 *同步查询某一测点一段时间的历史统计数据
 *参数： 
 *		第一个参数是测点长名和标识信息，第二个参数是查询所需的参数
 *返回值：
 *	失败返回错误对象，成功返回数组，每一个数组元素是一个值对象，每一个对象包含三个属性，value,quality,time
 *
 *异步查询某一测点一段时间的历史统计数据
 *参数： 
 *		第一个参数是测点长名和标识信息，第二个参数是查询所需的参数
 *		第三个参数是回调函数，回调函数参数包括err(错误信息)和查询到的值
 */
//同步查询某一测点一段时间的历史统计数据

var hisPro = {
	"startTime":time2,
	"endTime":time1,
	"resampleInterval":10000000,
	"aggregates":"START"
};

var resProcess = con.read("/tag5/tag6.his.processed",hisPro);	
if(resProcess instanceof Err){
	console.log(resProcess.errString);
}else{
	console.log("同步查询历史统计数据成功!");
	console.log(resProcess);
}

//同步直接传参查询某一测点一段时间历史统计数据
var resProcess1 = con.read("/tag5/tag8.his.processed",time2,time,10000000,"START");	
if(resProcess1 instanceof Err){
	console.log(resProcess1.errString);
}else{
	console.log("直接传参查询：");
	console.log(resProcess1);
}
	
//异步查询某一测点一段时间的历史统计数据
var resProcess2 = con.read("/tag5/tag6.his.processed",hisPro,function(err,resVal){
	if(err){
		console.log(err);
	}else{
		console.log("异步查询历史统计数据：");
		console.log(resVal);
	}
});
//异步直接传参查某一测点历史统计数据
var resProcess3 = con.read("/tag5/tag9.his.processed",time2,time1,10000000,"START",function(err,resVal){
	if(err){
		console.log(err);
	}else{
		console.log("异步查询历史统计数据1：");
		console.log(resVal);
	}
});
