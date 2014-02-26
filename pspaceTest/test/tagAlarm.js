/*
 *报警相关接口测试
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
 *同步查询实时报警
 *参数：支持json格式参数和直接传参
 *	参数中过滤条件包含过滤条件字段ID(filterField)和过滤条件(filter)，分别对应两个字典数据
 *返回值：
 *	成功返回一个数组，arr.length是报警数量，每一个数组元素是查询到的一个报警对象，每个对象包含了所有的报警内容属性
 *	失败返回错误处理对象，包含code和errString两个属性
 */

var filter = {
	"filterField":{"TagId":0,"AlarmHaveAcked":1},
	"filter":{"AlarmHaveAcked":0}
}

var filterField = {"TagId":0,"AlarmHaveAcked":1};
var filter1 = {"AlarmHaveAcked":0};
/*
 *直接传参
 */
var realAlarm = con.read("/tag.alarm.real",filterField,filter1);

var realAlarm1 = con.read("/tag.alarm.real",filter);
if(realAlarm1.hasOwnProperty("errString")){
	console.log(realAlarm1.errString);
}else{
	console.log("报警数量为:",realAlarm1.length);
	console.log(realAlarm1);
}

/*
 *异步查询实时报警
 *参数：支持json格式参数和直接传参
 *	参数中过滤条件包含过滤条件字段ID(filterField)和过滤条件(filter)，分别对应两个字典数据
 *	第三个参数是回调函数，带回执行结果，如果失败，err为错误信息，成功err为undefined
 *	resVal 为获取到的报警内容，是一个数组，每一个数组元素是查询到的一个报警对象，
 *	每个对象包含了所有的报警内容属性
 */
var realAlarm2 = con.read("/tag.alarm.real",filter,function(err,resVal){
	if(err){
		console.log(err);
	}else{
		console.log("报警数量为:",realAlarm1.length);
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
 */
var hisFilter = {
	'filterField':{"TagId":0,"AlarmHaveAcked":1},
	'filter':{"AlarmHaveAcked":0},
	'startTime':"2014-02-03 12:23:45.000",
	'endTime':"2014-02-20 09:00:00.000"
}
var hisAlarm1 = con.read("/tag.alarm.his",hisFilter);
if(hisAlarm1.hasOwnProperty("errString")){
	console.log(hisAlarm1.errString);
}else{
	console.log("同步查询历史报警数量:",hisAlarm1.length);
	console.log(hisAlarm1);
}
//直接传参
var hisAlarm = con.read("/tag.alarm.his",filterField,filter,"2014-02-03 12:23:45.000","2014-02-20 09:00:00.000");
if(hisAlarm.hasOwnProperty("errString")){
	console.log(hisAlarm.errString);
}else{
	console.log("同步查询历史报警数量:",hisAlarm.length);
	console.log(hisAlarm);
}
/*
 *异步查询历史报警
 *参数：支持json格式参数和直接传参
 *	第一个参数包含测点长名和标识，第二个参数是过滤条件
 *	直接传参参数过滤条件顺序为：filterField,filter,startTime，endTime
 *	第三个参数是回调函数，包含错误信息和报警内容两个属性
 *	成功hisVal带回报警内容，它是一个数组，数组大小arr.length是报警数量，每一个元素是一个报警对象
 *	每一个对象包含了所有的报警内容属性
 *	失败带回错误信息err
 */
var hisAlarm2 = con.read("/tag.alarm.his",hisFilter,function(err,hisVal){
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
 */
var ack = {
	"ackUserId":1,
	"ackUserName":"Admin",
	"ackTime":"2014-02-13 10:20:00.000",
	"ackIds":[2]
}
var ackeIds = [2];
/*
 *直接传参
 */
var ackAlarm1 = con.read("/tag.alarm.ack",1,"Admin","2014-02-13 10:20:00.000",ackeIds);
var ackAlarm = con.read("/tag.alarm.ack",ack);
if(ackAlarm.hasOwnProperty("errString")){
	console.log(ackAlarm.errString);
}else{
	console.log("同步应答报警成功!");
	console.log("应答者ID:",ackAlarm.userId,",应答者用户名:",ackAlarm.userName,",应答时间:",ackAlarm.ackTime);
}
/*
 *异步查询应答报警
 *参数：支持直接传参和json格式串
 *	ack为查询应答报警过滤条件，第三个参数是回调函数
 *	失败返回错误对象，对象包括code,errstring两个属性
 */
var ackAlarm2 = con.read("/tag.alarm.ack",ack,function(err,ackVal){
	if(err){
		console.log(err);
	}else{
		console.log("异步应答报警成功!");
		console.log("应答者ID:",ackVal.userId,",应答者用户名:",ackVal.userName,",应答时间:",ackVal.ackTime);
	}
});

module.exports.close = function(){
	con.close();
};