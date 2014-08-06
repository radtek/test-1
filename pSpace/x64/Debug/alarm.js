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

//模拟sleep（假死）
function sleep(milliSeconds){
    var startTime = new Date().getTime(); // get the current time
    while (new Date().getTime() < startTime + milliSeconds); // hog cpu
}


/*
 *同步查询实时报警
 *参数：支持json格式参数和直接传参
 *	参数中过滤条件包含过滤条件字段ID(filterField)和过滤条件(filter)，分别对应两个字典数据
 *返回值：
 *	成功返回一个数组，arr.length是报警数量，每一个数组元素是查询到的一个报警对象，每个对象包含了所有的报警内容属性
 *	失败返回错误处理对象，包含code和errString两个属性
 *说明：判断是否返回了错误对象有三种方法：
 *	方法一：if(con.hasOwnProperty("errString"))错误表达式的值为true
 *	方法二：if(con.constructor == Err)错误表达式的值返回true
 *	方法三：if(con instaceof Err) 错误表达式返回true
 *
 *异步查询实时报警
 *参数：支持json格式参数和直接传参
 *	参数中过滤条件包含过滤条件字段ID(filterField)和过滤条件(filter)，分别对应两个字典数据
 *	第三个参数是回调函数，带回执行结果，如果失败，err为错误信息，成功err为undefined
 *	resVal 为获取到的报警内容，是一个数组，每一个数组元素是查询到的一个报警对象，
 *	每个对象包含了所有的报警内容属性
 */

 var filter = {
	"filterField":{"TagId":0,"AlarmHaveAcked":1},
	"filter":{"AlarmHaveAcked":0}
}

var filter2 = {
	"filterField":0,
	"filter":0
}


var filterField = {"TagId":0,"AlarmHaveAcked":1};
var filter1 = {"AlarmHaveAcked":0};
//同步直接传参查询实时报警
var realAlarm = con.read("/tag/tag2.alarm.real",filterField,filter1);
if(realAlarm instanceof Err){
	console.log(realAlarm.errString);
}else{
	console.log("报警数量为:",realAlarm.length);
	console.log(realAlarm);
}
var realAlarm1 = con.read("/tag/tag1.alarm.real",filter);
if(realAlarm1 instanceof Err){
	console.log(realAlarm1.errString);
}else{
	console.log("报警数量为:",realAlarm1.length);
	console.log(realAlarm1);
}

sleep(2000);
//异步查询实时报警
var res = con.read("/tag/tag4.alarm.real",filter2,function(err,resVal){
	if(err){
		console.log(err);
	}else{
		console.log("报警数量为:",resVal.length);
		console.log(resVal);
	}
});
//异步直接传参查询实时报警
var res2= con.read("/tag/tag3/tag5.alarm.real",0,0,function(err,resVal){
	if(err){
		console.log(err);
	}else{
		console.log("报警数量为:",resVal.length);
		console.log(resVal);
	}
});

/*
 *同步查询历史报警
 *参数：支持json格式参数和直接传参
 *	第一个参数包含测点长名和标识，第二个参数是过滤条件
 *	直接传参参数过滤条件顺序为：filterField,filter,startTime，endTime
 *返回值：
 *	成功返回报警内容，它是一个数组，数组大小arr.length是报警数量，每一个元素是一个报警对象
 *	每一个对象包含了所有的报警内容属性
 *	失败返回错误对象，包含code和errString属性。
 *
 *异步查询历史报警
 *参数：支持json格式参数和直接传参
 *	第一个参数包含测点长名和标识，第二个参数是过滤条件
 *	直接传参参数过滤条件顺序为：filterField,filter,startTime，endTime
 *	第三个参数是回调函数，包含错误信息和报警内容两个属性
 *	成功hisVal带回报警内容，它是一个数组，数组大小arr.length是报警数量，每一个元素是一个报警对象
 *	每一个对象包含了所有的报警内容属性
 *	失败带回错误信息err
 */

var d = new Date().getTime();
var time  = new Date(d);
var time1 = new Date(d-7200000);
var hisFilter = {
	'filterField':{"TagId":0,"AlarmHaveAcked":1},
	'filter':{"AlarmHaveAcked":0},
	'startTime':time1,
	'endTime':time
}
//同步查询历史报警
var hisAlarm1 = con.read("/tag/tag3.alarm.his",hisFilter);
if(hisAlarm1 instanceof Err){
	console.log(hisAlarm1.errString);
}else{
	console.log("同步查询历史报警数量:",hisAlarm1.length);
	console.log(hisAlarm1);
}

//直接传参
var hisAlarm = con.read("/tag/tag1.alarm.his",filterField,filter,time1,time);
if(hisAlarm.hasOwnProperty("errString")){
	console.log(hisAlarm.errString);
}else{
	console.log("同步查询历史报警数量:",hisAlarm.length);
	console.log(hisAlarm);
}

sleep(2000);
//异步查询历史报警
var hisAlarm2 = con.read("/tag/tag4.alarm.his",hisFilter,function(err,hisVal){
	if(err){
		console.log(err);
	}else{
		console.log("异步历史查询报警数量:",hisVal.length);
		console.log(hisVal);
	}
});
//异步直接传参查询历史报警
var hisAlarm3 = con.read("/tag/tag4.alarm.his",filterField,filter,time1,time,function(err,hisVal){
	if(err){
		console.log(err);
	}else{
		console.log("异步历史查询报警数量:",hisVal.length);
		console.log(hisVal);
	}
});
/*
 *同步查询应答报警
 *参数：支持直接传参和json格式串
 *	ack为过滤条件
 * 返回值
 *	成功返回一个对象，对象包含userId,userName,ackTime三个属性
 *	失败返回错误对象，对象包括code,errstring两个属性
 *
 *异步查询应答报警
 *参数：支持直接传参和json格式串
 *	ack为查询应答报警过滤条件，第三个参数是回调函数
 *	失败返回错误对象，对象包括code,errstring两个属性
 */

 //同步查询应答报警
 var ack = {
	"ackUserId":1,
	"ackUserName":"Admin",
	"ackTime":time,
	"ackIds":[2]
}
var ackeIds = [2];
//直接传参查询应答报警
var ackAlarm1 = con.read("/tag/tag1.alarm.ack",1,"Admin",time,ackeIds);
if(ackAlarm1 instanceof Err){
	console.log(ackAlarm1.code);
	console.log(ackAlarm1.errString);
}else{
	console.log("同步应答报警成功!");
	console.log("应答者ID:",ackAlarm1.userId,",应答者用户名:",ackAlarm1.userName,",应答时间:",ackAlarm1.ackTime);
}
//同步查询历史报警
var ackAlarm = con.read("/tag/tag2.alarm.ack",ack);
if(ackAlarm instanceof Err){
	console.log(ackAlarm.errString);
}else{
	console.log("同步应答报警成功!");
	console.log("应答者ID:",ackAlarm.userId,",应答者用户名:",ackAlarm.userName,",应答时间:",ackAlarm.ackTime);
}

sleep(2000);
//异步查询应答报警
var ackAlarm2 = con.read("/tag/tag4.alarm.ack",ack,function(err,ackVal){
	if(err){
		console.log(err);
	}else{
		console.log("异步应答报警成功!");
		console.log("应答者ID:",ackVal.userId,",应答者用户名:",ackVal.userName,",应答时间:",ackVal.ackTime);
	}
});

var ackAlarm3 = con.read("/tag/tag3/tag5.alarm.ack",1,"Admin",time,ackeIds,function(err,ackVal){
	if(err){
		console.log(err);
	}else{
		console.log("异步应答报警成功!");
		console.log("应答者ID:",ackVal.userId,",应答者用户名:",ackVal.userName,",应答时间:",ackVal.ackTime);
	}
});

sleep(2000);
con.del("/tag");