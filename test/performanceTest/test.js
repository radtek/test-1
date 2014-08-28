var addon = require("../lib/pSpace");
var events = require("events"); 
var async = require("async");
var emitter = new events.EventEmitter(); 
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





var status = 0;

//删除点的函数
var fun=function(s)
{
	console.log("开始删除测点,请稍等...");
	for(var i=1;i<=100;++i){
		con.del("/tag"+i);
	}
	console.log("删除测点完毕。");
	status = s;
}


var synAdd = function(s){
	console.time('SynAddTag');
	console.log("开始同步加点测试，参数格式为json......");
	for(var i=1;i<=100;i++){
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
	status = s;
	//cpu利用率：%50-%60
	//执行时间：113352ms
	//注释了打印信息：103279ms
	//注释打印信息：94251ms,102973
}


var asyAdd  = function(s)
{
	console.time("AsyAddTag");
	console.log("开始异步加点测试，参数格式为json......");
	for(var i=1;i<=100;i++){
		var tag4 = {
		"propID":["PS_TAG_PROP_NAME","PS_TAG_PROP_TAGTYPE","PS_TAG_PROP_DATATYPE","PS_TAG_PROP_DESCRIPTION"],
		"dataType":["PSDATATYPE_STRING","PSDATATYPE_UINT16","PSDATATYPE_UINT8","PSDATATYPE_STRING"],
		"values":["tag"+i,2,"PSDATATYPE_FLOAT","psNode测试"]
		};
		(function(idx,t){
			var resAddAsy = con.add("/tag"+idx,tag4,function(err,tagName,tagId){
				if(idx>99){
					console.timeEnd("AsyAddTag");
					status = t;
				}
			});
		})(i,s);

	}
//cpu利用率达到%80-%90,时间:62834ms,65033ms
}



emitter.on("synAdd",synAdd); 
emitter.once("asyAdd",asyAdd); 
emitter.on("del",fun);

emitter.emit("synAdd",2);
if(status==2)
		emitter.emit("del",1);
if(status==1)
	emitter.emit("asyAdd",3);
		

	

