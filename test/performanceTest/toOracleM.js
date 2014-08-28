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

function strtotime(s){ 
	var  d  =  new  Date(Date.parse(s));
	return d;
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
		  	var value = new Array();
			var colName = '';
			var v= '';
			var date = new Date();
			value.push(date);
			for(var i=0;i<data.length;i++){
				  value.push(con.read(data[i].TagLongName+".pv").value);
				  colName+= data[i].TagColName+',';
				}
			for(var j=0;j<=data.length;j++){
				  v+= ":"+j+",";
			}
			v=v.substr(0, v.length-1);
			colName=colName.substr(0, colName.length-1);
			connection.execute("INSERT INTO NODE_TEST(CJSJ,"+colName+")"+
		                        "VALUES("+v+")",
		                        value,
		                        function(err,results){
		                          if (err) {
		                            console.log(err);
		                          }else{
		                            console.log(results);
		                            //console.timeEnd("toOracle");
		                          }
		                        });
			
		 }
		 //for(var j=1;j<=20000;j++){
		 	fun();
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

