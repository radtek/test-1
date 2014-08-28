var addon = require("../build/pSpace");
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
/*
async.series([
	function(callback){
		con.write("/tag1.pv",12.12,function(err,pv){
			callback(err,pv);
		});
	},
	function(callback){
		con.write("/tag2.pv",30.12,function(err,pv){
			callback(err,pv);
		});
	},
	function(callback){
		con.read("/tag3.pv",function(err,pv){
			callback(err,pv.value);
		});
	},
	function(callback){
		con.read("/tag4.pv",function(err,pv){
			callback(err,pv.value);
		});
	}
	],function(err,result){
	console.log(result[3]);
});
*/

for(var i=1;i<=5;i++)
{
	(function(idx){
		async.series([
			function(callback){
				console.log(idx);
				con.read("/tag"+idx+".pv",function(err,pv){
					callback(err,pv.value);
				});
		},
		function(callback){
			console.log(idx);
			con.read("/tag"+(idx+1)+".pv",function(err,pv){
				callback(err,pv.value);
			});
		}
	],function(err,result){
		
		con.write("/tag"+(idx+2)+".pv",result[0]+result[1],function(err,pv){
			console.log(pv);
			console.log(result);
		});
		});

	})(i);
}

/*
//异步做计算后写入
console.time("Asy:ai=ai*256");
for(i=1;i<=100000;i++){
	if(i>=99000){
		async.series([
			function(callback){
				con.read("/tag/test"+i+".pv",function(err,pv){
					callback(err,pv.value);
				});
			}
		],
		function(err,result){
			con.write("/tag/test"+i+".pv",result[0],function(err,pv){});
		});	
		
	}else{
		async.series([
			function(callback){
				con.read("/tag"+i+".pv",function(err,pv){
					console.log("/tag"+i+".pv");
					callback(err,pv.value);
				});
			}
		],
		function(err,result){
			con.write("/tag"+i+".pv",result[0],function(err,pv){});
		});	
	}
}
console.timeEnd("Asy:ai=ai*256");
//CPU:65%~80%,time:45426ms
*/

/*
//异步
console.time("realWriteIncludeTimeAsyNojson");
for(i=3;i<=10;++i){
	if(i>=99000){
		var resWrite = con.write("/tag/test"+i+".pv",22.2,function(err,tagName){
			console.log(tagName);
		});
	}else{
		var resWrite = con.write("/tag"+i+".pv",123.234,function(err,tagName){
			console.log(tagName);
		});
	}
}
console.timeEnd("realWriteIncludeTimeAsyNojson");

console.time("Asy:ai=ai*256");
for(i=1;i<=10;i++){
	console.log(i);
	con.read("/tag"+i+".pv",function(err,pv){
		console.log(pv.value);
		//con.write("/tag"+i+".pv",pv.value,function(err,pv){
		//	console.log(i);
		//});
	});
}
*/
/*
console.timeEnd("Asy:ai=ai*256");
for(i=1;i<=10;i++){
	con.read("/tag"+i+".pv",function(err,pv){
		for(i=1;i<=10;i++){
			con.write("/tag"+i+".pv",pv.value,function(err,pv){
				console.log(pv);
			});
		}	
	});
}	
*/
/*
for(i=99000;i<=100000;i++){
	con.read("/tag/test"+i+".pv",function(err,pv){
		for(i=99000;i<=100000;i++){
			con.write("/tag/test"+i+".pv",pv.value*256,function(err,pv){});
		}				
	});
}
*/
/*
for(var i=1;i<=10;i++){
	(function(index){
		con.read("/tag"+index+".pv",function(err,pv){
				console.log(index);
				con.write("/tag"+(index)+".pv",3,function(err,pv){});	
		});
	})(i);	
}
*/
/*
var j=1;
for(var i=1;i<=100000;i++){
	if(i>=99000){
		con.read("/tag/test"+i+".pv",function(err,pv){
			(function(idx){
				con.write("/tag/test"+idx+".pv",pv.value*256,function(err,pv){});
				if(idx>=100000)
					console.timeEnd("Asy:ai=ai*256");
			})(j++);						
		});	
	}else{
		con.read("/tag"+i+".pv",function(err,pv){
			(function(idx){
				con.write("/tag"+idx+".pv",pv.value*256,function(err,pv){});
			})(j++);		
		});
	}		
}
*/