var addon = require('pSpace');
var sqlserver = require('msnodesql');
var config = require('./config')
var guard = require('guard');

var ps = new addon();
guard.start();
var isConnect = 0;  //0：断线，1：已连接，2：正在连接
var con = {};
var queryData = {};
var tagData = {};  //节点数据
var subTagName = [];  //要订阅的测点

function run(){
	if(0 == isConnect){
		isConnect = 2;
		ps.open(config.conStr,function (err,conn) {
			if (err) {
				console.log('\npSpace连接错误:', err);
				isConnect = 0;
			}else{				
				console.log('\npSpace连接成功！');
				isConnect = 1;
				con = conn;
				queryData = {};
				tagData = {};
				subTagName = [];
				init();
				con.del(0);
				subFun();
			}			
		});
	}	
}

sqlserver.open(config.dbStr,function(err, conn){
	if(err){
		console.log("连接Sql Server错误：" + err);
	}else{
		console.log("连接Sql Server成功！");
	}
});

function match(str,arr){  //匹配字符串
	if(typeof(str) == "number"){
		if(0 == arr.length) return 1;
		for(var x in arr){
			if(str == arr[x])
				return 1;
			else continue;
		}
	}else{
		for(var x in arr){
			if(str.indexOf(arr[x]) > -1)
				return 1;
			else continue;
		}		
	}
	return 0;	
}

function format(obj){
	return obj < 10 ? ("0" + obj) : obj;     
}

process.on('isConnect',function(){  //监听pSpace是否连接的事件
	if(!ps.isConnected()){
		if(1 == isConnect) isConnect = 0;
		run();
	}
});

var fun = function(){
	process.emit('isConnect',null);
}

setInterval(fun,1000);  //每隔一秒检查一次pSPace是否已经连接

function init(){
	var propid = ["longname","description","pv","pv_quality"];
	var fielter1 = {"ISNODE":false,"querySelf":true,"queryLevel":5};
	var fielter2 = {"longname":"*" + config.keyWord + "*","querySelf":false,"queryLevel":10};
	var condition1 = {
		'propIds':propid,
		'filter':fielter1
		};
	var condition2 = {
		'propIds':propid,
		'filter':fielter2
		};
	queryData = con.query("idroot",condition2);  //查询所有符合条件的测点
	if(queryData instanceof ps.Err){
		console.log(queryData.errString);
	}else{
		for(var i in queryData){
			if(queryData[i].longname.indexOf("汇流箱")<0){
				console.log(queryData[i]);
			}
		}
		console.log("有"+queryData.length+"个测点！");
	}	
	for(var x=0;x<queryData.length;x++){
		if(match(queryData[x].longname,config.excWord)){  //去掉包含过滤字段的节点
			queryData.splice(x,1);
			x--;
			continue;
		}
		var longName = queryData[x].longname;
		var nodeName = longName.substring(0,longName.lastIndexOf('\\'));  //节点名
		if(undefined == tagData[nodeName]) tagData[nodeName] = {};
		tagData[nodeName][longName] = {};
		tagData[nodeName][longName].description = queryData[x].description;
		tagData[nodeName][longName].pv = queryData[x].pv;
		tagData[nodeName][longName].quality = queryData[x].pv_quality;
		subTagName.push(longName);
	}
}

function alarm(pv,des,lnm){
	var now = new Date();
	var date = now.getFullYear() + " " + format(now.getMonth()+1) + "-" + format(now.getDate());
	var time = date + " " + format(now.getHours()) + ":" + format(now.getMinutes()) + ":" + format(now.getSeconds());
	var sql = "insert into baojing (日期,数值,描述,时间,位号,确认) values ('"+ date + "','" + pv + "','" + des + "','" + time + "','" + lnm + "','未确认')";
	sqlserver.query(config.dbStr,sql,function(err, results){
		if(err){
			console.log('Sql Server插入数据错误：',err);
			return;
		}
	});
}

function avgFun(nodeName){
	var sum = 0;
	var num = 0;
	for(var x in tagData[nodeName]){
		if(match(tagData[nodeName][x].quality,config.quality) && ((config.symbol == 1 && tagData[nodeName][x].pv >= 0) || (config.symbol == 0))){  //质量戳为好，值为合理范围的才计入统计
			sum += tagData[nodeName][x].pv;
			num++;	
		}
	}
	if(sum == 0) return 0;
	else return sum/num;
}

function compare(node){  //计算是否超限
	var num = 0;
	for(var x in undefined == node ? tagData : node){
		var avg = avgFun(x);
		if(avg == 0) continue;  //平均值为0，直接进入下一个节点比较
		for(var y in tagData[x]){
			var pv = tagData[x][y].pv;
			if(match(tagData[x][y].quality,config.quality) && ((config.symbol == 1 && pv >= 0) || (config.symbol == 0))){  //质量戳为好，值为合理范围的才计入比较
				if(pv/avg > config.uplimit/100 || pv/avg < config.lowlimit/100){
					alarm(pv,tagData[x][y].description,y);
					num++;
				}			
			}
		}
	}
	console.log("有" + num + "个值超限！");
}

var subFun = function(){
	con.sub(subTagName,function(err,subid,curVal,value){  //订阅测点
		if(err){
			console.log(err);
		}else{
			if(curVal){  //当前值
				console.log("当前值：");
				compare();
			}
			if(value){  //订阅值
				var node = {};
				for(var x in value){
					var longName = value[x].name.replace(/\//g,"\\");
					var nodeName = longName.substring(0,longName.lastIndexOf('\\'));
					tagData[nodeName][longName].pv = value[x].value.value;
					node[nodeName] = 0;
				}
				console.log("订阅值：");
				compare(node);
			}
		}
	});
}

guard.startError();
guard.onStop(function(err,result){
	if(err){
		console.log(err);
	}else{
		
		process.exit(1);
	}
});
