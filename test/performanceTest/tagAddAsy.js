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

/*
console.time("AsyAddTag");
console.log("开始异步加点测试，参数格式为json......");
for(var i=1;i<=100000;i++){
	var tag4 = {
	"propID":["PS_TAG_PROP_NAME","PS_TAG_PROP_TAGTYPE","PS_TAG_PROP_DATATYPE","PS_TAG_PROP_DESCRIPTION"],
	"dataType":["PSDATATYPE_STRING","PSDATATYPE_UINT16","PSDATATYPE_UINT8","PSDATATYPE_STRING"],
	"values":["tag"+i,2,"PSDATATYPE_FLOAT","psNode测试"]
	};
	(function(idx){
		var resAddAsy = con.add("/tag"+idx,tag4,function(err,tagName,tagId){
			if(idx>99999)
				console.timeEnd("AsyAddTag");
		});
	})(i);

}
console.log("完成参数格式为json的异步加点测试.");
//cpu利用率:node:50%-60%,pSpace:22%-32%时间:65033ms,67805ms
*/


//异步直接传参
console.time("AsyAddNotJson");
console.log("开始异步加点测试，参数格式为直接传参....");
var tagAsyID =["PS_TAG_PROP_NAME","PS_TAG_PROP_TAGTYPE","PS_TAG_PROP_DATATYPE","PS_TAG_PROP_DESCRIPTION"];
var tagAsyType = ["PSDATATYPE_STRING","PSDATATYPE_UINT16","PSDATATYPE_UINT8","PSDATATYPE_STRING"];
for(var i=1;i<=100000;++i){
	var tagAsyValues = ["tag"+i,2,"PSDATATYPE_FLOAT","psNode测试"];
	(function(idx){
		var resAddAsy1 = con.add("/tag"+idx,tagAsyID,tagAsyType,tagAsyValues,function(err,tagName,tagId){
				if(idx>99999)
					console.timeEnd("AsyAddNotJson");
			});
	})(i);
	
}
console.log("完成参数格式为直接传参的异步加点测试.");
//cpu利用率:node:50%-62%,pSpace:22%-32%时间:67769ms,67805ms
