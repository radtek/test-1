var addon = require("../lib/pSpace");
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

//删除点的函数
var fun=function()
{
	console.log("开始删除测点,请稍等...");
	for(var i=1;i<=100000;++i){
		con.del("/tag"+i);
	}
	console.log("删除测点完毕。");
}

var str = "/tag";
console.log("开始加点测试，添加10万个一级测点，测点类型为模拟量........");
console.time('SynAddTag');
console.log("开始同步加点测试，参数格式为json......");
for(var i=1;i<=100000;i++){
	var tag4 = {
	"propID":["PS_TAG_PROP_NAME","PS_TAG_PROP_TAGTYPE","PS_TAG_PROP_DATATYPE","PS_TAG_PROP_DESCRIPTION"],
	"dataType":["PSDATATYPE_STRING","PSDATATYPE_UINT16","PSDATATYPE_UINT8","PSDATATYPE_STRING"],
	"values":["tag"+i,2,"PSDATATYPE_FLOAT","psNode测试"]
	};
	var resAddSyn4 = con.add("/tag"+i,tag4);
	//if(resAddSyn4 instanceof Err){
		//console.log("添加测点错误:",resAddSyn4.errString);
	//}else{
	//	console.log("添加测点:",resAddSyn4,"成功!");
	//}
}
console.log("完成参数格式为json的同步加点测试.");
console.timeEnd('SynAddTag');
//cpu利用率：node:30%-40%,pspace:15%-22%
//注释了打印信息：103279ms,104566,94251ms,102973


fun();

//直接传参
console.time('SynAddTagNoJson');
console.log("开始同步添加测点测试，参数格式为直接传参....");
var tagID =["PS_TAG_PROP_NAME","PS_TAG_PROP_TAGTYPE","PS_TAG_PROP_DATATYPE","PS_TAG_PROP_DESCRIPTION"];
var tagType = ["PSDATATYPE_STRING","PSDATATYPE_UINT16","PSDATATYPE_UINT8","PSDATATYPE_STRING"];
for(i=1;i<=100000;i++){
	var tagValues = ["tag"+i,2,"PSDATATYPE_FLOAT","psNode测试"];
	var resAddSyn= con.add("tag"+i,tagID,tagType,tagValues);
	//if(resAddSyn instanceof Err){
	//	console.log("添加测点错误:",resAddSyn.errString);
	//}else{
	//	console.log("添加测点:",resAddSyn,"成功!");
	//}
}
console.log("完成参数格式为直接传参的同步加点测试.");
console.timeEnd('SynAddTagNoJson');
//cpu利用率：node:30%-40%,pspace:15%-22%
//注释了打印信息：107678ms,104414ms
fun();
