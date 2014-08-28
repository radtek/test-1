var addon = require("../build/pSpace");
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
 *同步添加测点
 *参数:
 *	支持pSpace提供的测点属性，测点长名这个属性可提供可不提供，测点类型必须提供，其他属性可选
 *	测点父节点由第一个参数解析获得，为了增加程序可读性，属性使用一个json对象来传递，
 *	对象的属性（关键字）是测点属性名称，不区分大小写，对象的属性值是测点的属性值
 *返回值：
 *  成功返回测点长名，失败返回错误对象，对象包含code和errString两个属性。
 *说明：判断是否返回了错误对象有三种方法：
 *	方法一：if(con.hasOwnProperty("errString"))错误表达式的值为true
 *	方法二：if(con.constructor == Err)错误表达式的值返回true
 *	方法三：if(con instaceof Err) 错误表达式返回true
 *
 *异步添加测点
 *参数：
 *	第三个参数是回调函数，回调函数第一个参数是错误信息，第二个参数是测点长名，第三个参数是测点id
 */
//向数据库添加10个测点,添加模拟量时需要设置数据类型
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
//tag5
var prop5 = new Object();
prop5.tagType = "node";
//tag6
var prop6 = {"tagType":"analog","DataType":"double","description":"psNode 测试"};
//tag7
var prop7 = {"tagType":"analog","DataType":"FLOAT","description":"psNode 测试"};
//tag8
var prop8 = {"tagType":"digital","description":"pSnode test"};
//tag9
var prop9 = new Object();
prop9.tagType = "analog";
prop9.DataType = "double";
prop9.description = "psNode 测试";

//tag
var resAddSyn = con.add("/tag",prop);
if(resAddSyn instanceof Err){
	console.log("添加测点错误:",resAddSyn.errString);
}else{
	console.log("添加测点:",resAddSyn,"成功!");
}
//tag1
var resAddSyn1 = con.add("/tag/tag1",prop1);
if(resAddSyn1.constructor == Err)
{
	console.log("添加测点失败:",resAddSyn1.errString);
}else{
	console.log("添加测点成功");
}
//tag2
var resAddSyn2 = con.add("/tag/tag2",prop2);
if(resAddSyn2 instanceof Err){
	console.log("添加测点错误:",resAddSyn2.errString);
}else{
	console.log("添加测点:",resAddSyn2,"成功!");
}
//tag3
var resAddSyn3 = con.add("/tag/tag3",prop3);
if(resAddSyn3 instanceof Err){
	console.log("添加测点错误:",resAddSyn3.errString);
}else{
	console.log("添加测点:",resAddSyn3,"成功!");
}
//tag4
var resAddSyn4 = con.add("/tag/tag4",prop4);
if(resAddSyn4 instanceof Err){
	console.log("添加测点错误:",resAddSyn4.errString);
}else{
	console.log("添加测点:",resAddSyn4,"成功!");
}
//tag5
var resAddSyn5 = con.add("/tag5",prop5);
if(resAddSyn5 instanceof Err){
	console.log("添加测点错误:",resAddSyn5.errString);
}else{
	console.log("添加测点:",resAddSyn5,"成功!");
}
sleep(3000);
//异步

//tag6
var resAddAsy1 = con.add("/tag5/tag6",prop6,function(err,tagName,tagId){
	if(err){
		console.log(err);
	}else{
		console.log("异步添加测点:",tagName,"ID:",tagId,"成功!");
	}
});
//tag7
var resAddAsy2 = con.add("/tag5/tag7",prop7,function(err,tagName,tagId){
	if(err){
		console.log(err);
	}else{
		console.log("异步添加测点:",tagName,"ID:",tagId,"成功!");
	}
	
});
//tag8
var resAddAsy3 = con.add("/tag5/tag8",prop8,function(err,tagName,tagId){
	if(err){
		console.log(err);
	}else{
		console.log("异步添加测点:",tagName,"ID:",tagId,"成功!");
	}
	
});
//tag9
var resAddAsy4 = con.add("/tag5/tag9",prop9,function(err,tagName,tagId){
	if(err){
		console.log(err);
	}else{
		console.log("异步添加测点:",tagName,"ID:",tagId,"成功!");
	}
});

/*
 *同步设置测点属性
 *参数：
 *	使用javascript的一个对象作为参数，每个对象的属性就是pSpace所支持的测点的属性名称（不区分大小写）
 *	每个对象的属性值就是需要添加的测点的属性值
 *异步设置测点属性
 *参数：
 *	第三个参数是回调函数，回调函数包含err(错误信息)，tagName(测点长名)，tagId(测点ID)
 */

var setProps = {"description":"set props","His_IsSave":1};

var setProps1 = new Object();
setProps1.description = "setProps1";
setProps1.SecurityArea = 4095;
setProps1.His_IsSave = 1;

var setProps2 = {"EnableAlarm":1,"EnableValueAlarm":1,"His_IsSave":1};

var setProps3 = new Object();
setProps3.EnableAlarm = 1;
setProps3.EnableValueAlarm = 1;
setProps3.ValueAlarmNeedRespond = 1;
setProps3.EnableQualityAlarm = 1;
setProps3.EnableRateAlarm = 1;
setProps3.EnableDevAlarm = 1;
setProps3.LowLowAlarmNeedRespond = 1;
setProps3.LowAlarmNeedRespond = 1;
setProps3.HighAlarmNeedRespond = 1;
setProps3.RateAlarmNeedRespond = 1;
setProps3.His_IsSave = 1;
setProps3.LowLowAlarm = 5.00;
setProps3.LowAlarm = 30.00;
setProps3.HighAlarm = 80.00;
setProps3.HighHighAlarm = 180.00;
setProps3.Dev= 20.00;



//同步设置测点属性
var resSetProps = con.write("/tag/tag2.props",setProps);
if(resSetProps.hasOwnProperty("errString")){
	console.log("错误码:",resSetProps.code);
	console.log("设置测点属性测点错误:",resSetProps.errString);
}else{
	console.log("设置测点属性:",resSetProps,"成功!");
}
var resSetProps1 = con.write("/tag/tag3.props",setProps3);
if(resSetProps1.hasOwnProperty("errString")){
	console.log("错误码:",resSetProps1.code);
	console.log("设置测点属性测点错误:",resSetProps1.errString);
}else{
	console.log("设置测点属性:",resSetProps1,"成功!");
}
sleep(2000);
//异步设置测点属性
var resSetPropsAsy = con.write("/tag5/tag6.props",setProps1,function(err,tagName,tagId){
	if (err) {
		console.log(err);
	}else{
		console.log("异步设置测点属性成功:",tagName,"ID:",tagId);
	}
});
var resSetPropsAsy = con.write("/tag/tag1.props",setProps3,function(err,tagName,tagId){
	if (err) {
		console.log(err);
	}else{
		console.log("异步设置测点属性成功:",tagName,"ID:",tagId);
	}
});

/*
 *同步获取测点属性
 *参数：
 *	参数采用一个数组，数组中的每一个元素是要获取的属性的属性名称
 *返回值：
 *	成功返回值对象，对象的属性是参数中的属性名称（值对象的属性必须为数组中元素的属性名称）
 *异步获取测点属性
 *参数：
 *	参数与同步一样，但第三个参数是回调函数，函数参数包含err(错误信息),tagName(测点长名)，props(测点属性值对象)
 */
 
var getProp = ["name","TagType","SecurityArea","ParentId"];
var getProp1 = ["name","LongName"];

//同步获取
var props = con.read("/tag/tag1.props",getProp);
if(props instanceof Err){
	console.log(props.code);
	console.log(props.errString);
}else{
	console.log("同步获取属性值:",props);
	console.log("分条打印:");
	console.log(props.name);
	console.log(props.TagType);
	console.log(props.SecurityArea);
	console.log(props.ParentId);
}

//异步获取
var getAsy = con.read("/tag5/tag7.props",getProp1,function(err,tagName,prop){
	if(err){
		console.log(err);
	}else{
		console.log("异步获取",tagName,"属性值成功");
		console.log(prop);
	}
});

/*
  *同步删除测点
  *参数
  *	包含了所要删除的测点长名以及父节点，格式为：/node/tag
  *返回值：
  *	成功返回测点长名
  *
  *异步删除测点
  *参数：
  *第一个参数包含了所要删除的测点长名以及父节点，格式为：/node/tag
  *第二个参数是回调函数，回调函数第一个参数是错误信息，第二个参数是tagName
  *返回值：
  *成功返回测点长名
  */
  //同步
 /*
var resDelSyn = con.del("/tag/tag2");
if(resDelSyn instanceof Err){
	console.log("错误码:",resDelSyn.code);
	console.log("错误信息:",resDelSyn.errString);
}else{
	console.log("删除测点:",resDelSyn,"成功!");
}
sleep(2000);
//异步
var resDelAsy = con.del("/tag",function(err,tagName){
	if(err){
		console.log(err);
	}else{
		console.log("异步删除测点:",tagName,"成功!");
	}
});

*/

var props = con.read("/tag/tag1.name");
if(props instanceof Err){
	console.log(props.code);
	console.log(props.errString);
}else{
	console.log("获取测点属性：");
	console.log(props);
}