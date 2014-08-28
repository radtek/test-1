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
//add.addFun();

console.log("开始订阅测试.....................");

var resSub = con.sub("/tag/tag1.real",function(err,tagName,val){
if(err){
	console.log(err);
}else
{
	console.log(val.value);
}
});

/*
 *测点属性订阅测试
 *参数:
 *	/tag:测点长名，props:订阅标识
 *	第二个参数是一个回调函数，回调函数第一个参数是错误信息
 *  如果执行成功，这个参数是undefined,第二个参数是测点长名
 *  第三个参数是订阅到的属性值对象，包含四个属性：
 *  id(测点ID),count(测点属性变化数量)，type(订阅变化类型)，value(属性值)
 */
 /*
 var resProps = con.sub("/tag/tag1.props",function(err,tagName,val){
	if (err) {
		console.log(err);
		con.close();
	}else{
		console.log("订阅测点:",tagName,"成功!");
		console.log("点ID:",val.id);
		console.log("订阅属性变化类型",val.type,"变化属性数量:",val.count);
		console.log("属性值:",val.value);
	}
});

*/