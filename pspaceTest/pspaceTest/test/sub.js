/*
 *实时订阅接口测试
 *如果测点实时值订阅和测点属性订阅同时进行，其中的一个订阅的结果将会无法推送到执行界面
 */
 
var addon = require("../lib/pSpace");
var ps = new addon.Client();
var Err = addon.Error;
var con = ps.connect("localhost","admin","admin888");
if(con.hasOwnProperty("errString"))
{
	console.log("错误码:",con.code);
	console.log("服务器连接失败:",con.errString);
	throw "connect fail!";
}
console.log();
/*
 *新建订阅测点实时值变化并得到初值
 *参数:
 *	/tag:测点长名，real:实时值订阅标识
 *	第二个参数是一个回调函数，回调函数第一个参数是错误信息
 *  如果执行成功，这个参数是undefined,第二个参数是测点长名
 *  第三个参数是订阅到的实时值对象，包含三个属性：value,quality,time	
 */
var resSub = con.sub("/tag.real",function(err,tagName,val){
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
 var resProps = con.sub("/tag.props",function(err,tagName,val){
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

