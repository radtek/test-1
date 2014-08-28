var addon = require("../lib/pSpace");
var async = require('async');
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

var data  = {
	"value":67.001,
	"quality":0,
	"time":"2014-01-21 00:20:0.123"
};

var data1 = {
	"value":78.890,
	"quality":1
};
/*
//先添加10万个点，一级测点98999个二级测点1000个
var tag = {
		"propID":["PS_TAG_PROP_NAME","PS_TAG_PROP_TAGTYPE"],
		"dataType":["PSDATATYPE_STRING","PSDATATYPE_UINT16"],
		"values":["tag",0]
		};
var resAddSyn= con.add("/tag",tag);
console.log("开始添加10万个测点，其中一级测点98999个，二级测点1000个，测点类型为模拟量...");
var propID = ["PS_TAG_PROP_NAME","PS_TAG_PROP_TAGTYPE","PS_TAG_PROP_DATATYPE","PS_TAG_PROP_DESCRIPTION"];
var dataType = ["PSDATATYPE_STRING","PSDATATYPE_UINT16","PSDATATYPE_UINT8","PSDATATYPE_STRING"];
for(var i=1;i<=100000;i++){
	if(i>=99000)
	{
		var values = ["test"+i,2,"PSDATATYPE_DOUBLE","psNode测试"];
		var resAddSyn4 = con.add("/tag/test"+i,propID,dataType,values);	
	}else{
		var values = ["tag"+i,2,"PSDATATYPE_DOUBLE","psNode测试"];
		var resAddSyn4 = con.add("/tag"+i,propID,dataType,values);
	}			
}
console.log("添加测点完毕，开始测试.....");
*/
/*
//异步
console.time("realWriteIncludeTimeAsy");
console.log("开始异步写实时测试.....");
for(var i=1;i<=100000;++i){
	if(i>=99000){
		(function(idx){
			var resWrite = con.write("/tag/test"+idx+".pv",data,function(err,tagName){
				if(idx>99999)
					console.timeEnd("realWriteIncludeTimeAsy");
			});
		})(i);
		
	}else{
		var resWrite = con.write("/tag"+i+".pv",data,function(err,tagName){});
	}
}
console.log("完成异步写实时测试.");
//CPU:node:60%-75%,pSpace:15%-25%,time:19016ms,20248ms,19546ms
*/
/*
//异步
console.time("realWriteIncludeTimeAsyNojson");
console.log("开始重新异步写入实时值...");
for(var i=1;i<=100000;++i){
	if(i>=99000){
		(function(idx){
			var resWrite = con.write("/tag/test"+idx+".pv",2.5,function(err,tagName){
				if(idx>99999)
					console.timeEnd("realWriteIncludeTimeAsyNojson");
			});
		})(i);
		
	}else{
		var resWrite = con.write("/tag"+i+".pv",1.2,function(err,tagName){});
	}
}
console.log("重新异步写入实时值完毕");

//CPU:85%-95%,time:18018ms,18611ms,18626ms
*/
/*
console.time("readAsy");
console.log("开始异步读取实时值测试...");
for (var i = 1; i <= 100000; ++i) {
	if(i>=99000){
		(function(idx){
			var res = con.read("/tag/test"+idx+".pv",function(err,pv){
				if(idx>99999)
					console.timeEnd("readAsy");
			});	
		})(i);
		var res = con.read("/tag/test"+i+".pv",function(err,pv){});	
	}else{
		var pv = con.read("/tag"+i+".pv",function(err,pv){
			if(err){
				console.log(err);
			}else{
				//console.log(pv.value);
			}
		});
	}	
};
console.log("完成异步读取实时值测试.");
//CPU:node:65%~75%,pSpace:17%-25%,time:19172ms,19406ms,19312ms,18829ms,19040ms,18579ms
*/
/*
//异步读异步写计算
console.time("AsyReadAsyWrite:ai=ai*256");
for(var i=1;i<=100000;i++){
	if(i>=99000){
		(function(index){
			con.read("/tag/test"+index+".pv",function(err,pv){
				con.write("/tag/test"+index+".pv",pv.value*256,function(err,pv){});	
				if(index>=100000)
					console.timeEnd("AsyReadAsyWrite:ai=ai*256");	
			});
		})(i);
	}else{
		(function(index){
			con.read("/tag"+index+".pv",function(err,pv){
					con.write("/tag"+index+".pv",pv.value*256,function(err,pv){});	
			});

		})(i);		
	}
}
//CPU:node:65%~75%,pSpace:17%-25%,time:37502ms,37502ms,36426ms,37082ms,35989ms
*/
/*
//同步读异步写
console.time("SynReadAsyWrite");
for(var i=1;i<=100000;i++){
	if(i>=99000){
		(function(index){
			var val = con.read("/tag/test"+index+".pv");
			con.write("/tag/test"+index+".pv",(val.value)*256,function(err,pv){
				if(index>99999)
					console.timeEnd("SynReadAsyWrite");
			});		
		})(i);
	}else{
		(function(index){
			var val = con.read("/tag"+index+".pv");
			con.write("/tag"+index+".pv",(val.value)*256,function(err,pv){});	
		})(i);		
	}
}
*/
//PCU:node:55%-%65,pSpace:15%-25%,time:40856ms,40076ms,41464ms，41511ms,41620ms,40809ms

/*
//异步读异步写
console.time("AsyReadAsyWrite:a5=(a1+a2)*a3/a4");
for(var i=1;i<=100000;++i){
	if(i>=99000){
		if(i+4<=100000){
			(function(idx){
				async.series([
				function(callback){
					con.read("/tag/test"+idx+".pv",function(err,pv){
						callback(err,pv.value);
					});
				},
				function(callback){
					con.read("/tag/test"+(idx+1)+".pv",function(err,pv){
						callback(err,pv.value);
					});
				},
				function(callback){
					con.read("/tag/test"+(idx+2)+".pv",function(err,pv){
						callback(err,pv.value);
					});
				},
				function(callback){
					con.read("/tag/test"+(idx+3)+".pv",function(err,pv){
						callback(err,pv.value);
					});
				}],
				function(err,value){
					if(idx>99995)
						console.timeEnd("AsyReadAsyWrite:a5=(a1+a2)*a3/a4");
					con.write("/tag/test"+(idx+4)+".pv",(value[0]+value[1])*value[2]/value[3],function(err,tagName){});
				});

			})(i);
			
		}	
	}else{
		if(i+4<99000){
			(function(idx){
				async.series([
				function(callback){
					con.read("/tag"+idx+".pv",function(err,pv){
						callback(err,pv.value);
					});
				},
				function(callback){
					con.read("/tag"+(idx+1)+".pv",function(err,pv){
						callback(err,pv.value);
					});
				},
				function(callback){
					con.read("/tag"+(idx+2)+".pv",function(err,pv){
						callback(err,pv.value);
					});
				},
				function(callback){
					con.read("/tag"+(idx+3)+".pv",function(err,pv){
						callback(err,pv.value);
					});
				}],
				function(err,value){
					con.write("/tag"+(idx+4)+".pv",(value[0]+value[1])*value[2]/value[3],function(err,tagName){});
				});

			})(i);			
		}
			
	}
}
//
//PCU:node:60%-%75,pSpace:15%-25%,time:101836ms,100432ms,98060ms,100932ms,100932,101996ms
*/

//同步读异步写
console.time("SynReadAsyWrite:a5=(a1+a2)*a3/a4");
for(var i=1;i<=100000;++i){
	if(i>=99000){
		if(i+4<=100000){
			var value1 = con.read("/tag/test"+i+".pv").value;
			var value2 = con.read("/tag/test"+(i+1)+".pv").value;
			var value3 = con.read("/tag/test"+(i+2)+".pv").value;
			var value4 = con.read("/tag/test"+(i+3)+".pv").value;
			(function(idx){
				var resWrite = con.write("/tag/test"+(idx+4)+".pv",(value1+value2)*value3/value4,function(err,tagName){
					if(idx>99995)
						console.timeEnd("SynReadAsyWrite:a5=(a1+a2)*a3/a4");
				});
			})(i);
			
		}	
	}else{
		if(i+4<99000){
			var v1 = con.read("/tag"+i+".pv").value;
			var v2 = con.read("/tag"+(i+1)+".pv").value;
			var v3 = con.read("/tag"+(i+2)+".pv").value;
			var v4 = con.read("/tag"+(i+3)+".pv").value;
			var resWrite = con.write("/tag"+(i+4)+".pv",(v1+v2)*v3/v4,function(err,tagName){});
		}
			
	}
}
//CPU:node:65%-80%,pSpace: time:10893ms,114162ms,112429ms,110307ms,109855ms,110230ms
