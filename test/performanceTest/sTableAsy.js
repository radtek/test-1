var addon = require("../lib/pSpace");
var events = require("events"); 
var async = require("async");
var csv  = require('csv');
var oracle  = require('oracle');
var emitter = new events.EventEmitter(); 
var ps = new addon.Client();
var Err = addon.Error;
// 重新封装定时器
function accurateInterval(callback, interval) {
    var now = +new Date();

    setTimeout(function run() {
        now += interval;
        var fix = now - (+Date.now());

        setTimeout(run, interval + fix);

        callback();
    }, interval);
}
var connectData = {
        "hostname":"localhost",
        "database":"xe",
        "user":"system",
        "password":"yingf"
};

var str2TimeStamp = function(s)
{
	var arr = s.split(" "); 
    var arr1 = arr[0].split("-"); 
    var arr2 = arr[1].split(":");
    var arr3 = arr2[2].split("."); 
    var year = arr1[0]; 
    var month = arr1[1]-1; 
    var day = arr1[2]; 
    var hour = arr2[0]; 
    var mon = arr2[1]; 
    var min = arr3[0];
    var mil = arr3[1];
    var timestamp = new Date(year,month,day,hour,mon,min,mil);  

    return timestamp; 
}
var con = ps.connect("localhost","admin","admin888");
if(con instanceof Err){
	console.log("错误码:",con.code);
	console.log("服务器连接失败:",con.errString);
	throw "connect fail!";
}else{
	console.log("连接成功!");
}
console.log();

csv()
.from.path('./Type1.csv',{
  columns:true
})
.transform(function(data,index){
 	 return  data;
})
//所有数据读取作为一个数组 
.to.array( function(data){ 
    oracle.connect(connectData,function(err,connection){
	  if (err) {
	    console.log(err);
	  }else{
	  	var fun = function()
	  	{
	  		//console.time("toOracle");
			for(var i=0;i<data.length;i++){
				(function(da,idx){
					con.read(data[idx].TagLongName+".pv",function(err,pv){
				  	var value = new Array();
				  	value.push(da[idx].TagLongName);
				  	value.push(str2TimeStamp(pv.time));
				  	value.push(pv.value);
				  	
				  	connection.execute("INSERT INTO NODE_TEST3(tagName,time,value)"+
		                       	 		"VALUES(:1,:2,:3)",
		                      	  value,
		                       	 function(err,results){
		                        	  if (err) {
		                          	  console.log(err);
		                        	  }else{
		                         	   	console.log(results);
		                          	  	//console.timeEnd("toOracle");
		                          	}
		                        });
				  });
				})(data,i);
				}
			
		 }
		 //for(var j=1;j<=20000;j++){
		 	//fun();
		// }
		setInterval(fun,1000);
	  }  
	});
} )
/*
//一条一条记录读取
.on('record', function(columns,index){
 	//console.log(JSON.stringify(columns.TagLongName));
 	//console.log(columns.TagLongName);
 	 oracle.connect(connectData,function(err,connection){
	  if (err) {
	    console.log(err);
	  }else{
	  //	console.log(columns.TagLongName);
	  	
	  	var val = con.read(columns.TagLongName+".pv");
	  	var d = new Date();
	    connection.execute("INSERT INTO NODE_TEST (CJSJ,"+columns.TagColName+")"+
	                        "VALUES(:1,:2) ",
	                        [d,val.value],
	                        function(err,results){
	                          if (err) {
	                            console.log(err);
	                          }else{
	                            console.log(results);
	                          }
	                          connection.close();
	                        });

	  }  
});
})
*/
.on('close', function(count){
  console.log('Number of lines: '+count);
})
.on('error', function(error){
  console.log(error.message);
});

