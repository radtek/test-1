var addon = require("../build/pSpace");
var fs = require('fs');
var schedule = require ("node-schedule");

var ps = new addon.Client();
var Err = addon.Error;
var con = ps.connect("localhost","admin","admin888");
if(con instanceof Err){
	console.log("错误码:",con.code);
	console.log("服务器连接失败:",con.errString);
	throw "connect fail!";
} else{
	console.log("连接成功!");
}
console.log();

var tagList = new Array();
var resultTagList = new Array();

//解析点表config.csv
fs.readFile('config.csv','utf8',function (err, data) {
  if(!err) {
    //console.log(data);
    var dataTemp = data.replace(/\r\n/ig,",");
    dataTemp = dataTemp.substring(45,dataTemp.length-1);
    var dataList = dataTemp.split(",");
    for (i=0; i<dataList.length; i++) {
    	if (i%2==0) {
    		tagList[i/2]=dataList[i];
    	} else {
    		resultTagList[i/2-0.5]=dataList[i];
    	}
    }
	//console.log("parse CSV done!");
  }else {
    throw err;
  }
})

//时间调度
var rule = new schedule.RecurrenceRule();
rule.hour = 16;
rule.minute = 34;
//rule.second = 0;
//console.log("Set Rule done!");
var j = schedule.scheduleJob(rule, function(){
	var t1 = new Date().getTime();						
	var time1 = new Date(t1);
	//var t2 = new Date().getTime();
 	//var time2 = new Date(t2-60*1000);	
 	var time2 = new Date(t1-1000*60*60*24);		
	var timeStamps = [time1,time2];	
	//console.log("Set Time done!");	
	//console.log(time1);
	//console.log(time2);

	for (m=0;m<tagList.length;m++) {
	console.log("TAG:"+tagList[m]);
		
    //查询今天和昨天的总耗电量,并计算差值
    var resAtTime= con.read(tagList[m]+".his.atTime",timeStamps);
		if(resAtTime.hasOwnProperty("errString")){
			console.log("错误:",resAtTime.errString);
		}else{	//console.log("Test"+resAtTime[0].value);

			for(i=0;i<resAtTime.length;i++){
						console.log(resAtTime);
				if (i==0) {console.log(time1+"总耗电量: "+resAtTime[i].value);}
				if (i==1) {console.log(time2+"总耗电量: "+resAtTime[i].value);}
			}
			var powerConsumptionDaily = resAtTime[0].value-resAtTime[1].value;
			console.log("计算得日耗电量: "+powerConsumptionDaily);
		}
		
		//将计算的结果写入数据库
		var data = {
			"value":powerConsumptionDaily
		};
		var resWrite = con.write(resultTagList[m]+".pv",data);
			if(resWrite instanceof Err){
			console.log(resWrite.code);
			console.log(resWrite.errString);
		}else{
			console.log(resWrite,":日耗电量写入数据库成功");
		}

  }
});

