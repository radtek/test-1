//需要设置地方
//统计测点列表
var tag_path = 'taglist.csv';

//统计起始时间
var start_time = new Date("2017-5-8 1:00:00");

//统计结束时间 real_router设置时无效
var end_time = new Date();
//是否实时统计
var real_router = true;

var con_str = {  //pSpace服务器连接参数，json格式
	"host":"localhost",
	"user":"admin",
	"password":"admin888",
	"port":8889  //可选
};

//链接mysql
var mysql_info  = {
  host: 'localhost',
  user: 'his',
  password: 'his',
  database: 'his',
  port: 3306
};

var table_name = 'table_hour';

//读取历史数据间隔周期
var read_his_timer = 5000;

//////////////////////////////////////////////////////////////

var addon = require('pSpace');
var fs = require('fs');
var csv = require('CSV');
var mysql = require('mysql');
var async = require("async");

var ps = new addon();
var Err = ps.Err;


var taglist = [];
var start_real = new Date();
var cur_aggregate = {};
var last_aggregate = {};


var log4js = require('log4js'); 
var path = require('path');
function  initLog(dirPath) {
    log4js.configure({  
    appenders: [  
        {  
            type: 'console',  
            category: "console"  
        }, //控制台输出  
        {  
            type: "file",  
            filename: 'Router.log',  
            maxLogSize:1048576,
            backups:50, 
            category: 'console'  
        }//日期文件格式  
    ],  
    replaceConsole: true,   //替换console.log  
    levels:{  
        "log_file":"INFO", 
        "console":"INFO" 
    }  
});   
}
initLog();
//同步连接
var con = ps.open(con_str);
if(con instanceof Err){
	console.log('pSpace同步连接失败:',con.errString);
}else{
	console.log('pSpace同步连接成功！');
}

var mysql_con = {};
function mysqlConnect( ){
  mysql_con = mysql.createConnection(mysql_info);
    mysql_con.connect(function(err) {
    if (err) {
      console.log('error connecting: ' + err.stack);
    }else{
     console.log("mysql连接完毕。"); 
    }
  });
}
function CreateTable(){
	mysql_con.query('create table IF NOT EXISTS '+table_name+' (tagname varchar(255) not null,write_time datetime not null,avg_value float not null,max_value float not null,max_time datetime  not null,min_value float not null,min_time datetime  not null)', function (error, results, fields) {
		if (error){
		}else{
			mysql_con.query("ALTER TABLE table_hour ADD UNIQUE INDEX uni(tagname, write_time)", function (error1, results1, fields1) {
				if (!error1){
					console.log("创建表:"+table_name); 
				}
			});
		}
	});
}
mysqlConnect();
CreateTable();
 
function Appendzero(obj)
{
     if(obj<10) return "0" +""+ obj;
     else return obj;     
}

function Append3zero(obj)
{
	if(obj<10) return "00" +""+ obj;
    if(obj<100) return "0" +""+ obj;
    return obj;     
}

function formatDate(time){
    var   year=time.getFullYear();     
    var   month=time.getMonth()+1;     
    var   date=time.getDate();     
    var   hour=time.getHours();     
    var   minute=time.getMinutes();    
    var   second=time.getSeconds(); 
    var   milSecond = time.getMilliseconds();  
    return   year+"-"+Appendzero(month)+"-"+Appendzero(date)+" "+Appendzero(hour)+":"+Appendzero(minute)+":"+Appendzero(second)+"."+Append3zero(milSecond);     
}

/*
var count = 0;
function exec(sqlArr){
  async.series([
		function(done){
	   //执行sql
		for(var i in sqlArr){
			//循环中含有异步，这里使用闭包执行
			(function(idx){
				mysql_con.query(sqlArr[idx],function(err){
					if(err){
						console.log("sql语句执行<", sqlArr[idx], ">出错:",err);
					}else{
						if (count++ % 100==0) {
							console.log("sql语句执行<", sqlArr[idx], ">成功:");
						};
                          if(idx>sqlArr.length-2)
                                done();
					}
				});
			})(i);
		}
    },
	function(done){
	//mysql_con.destroy();
    }],function(err,result){
      if(err){
		console.log("async.series err:",err);
      }else{
	    console.log("async.series result:",result);
        return;
      }
    });
}
*/

/*
var sqlArr = [];
function WriteSql(dat)
{
	//console.log(dat);
  for(var i in dat){
	var write_date =  formatDate(dat[i].time);
	var max_date =  formatDate(dat[i].max_time);
	var min_date =  formatDate(dat[i].min_time);
    var str = "insert into "+table_name+" values('"+dat[i].longName+"','"+write_date+"','"+dat[i].avg+"','"+dat[i].max_value+"','"+max_date+"','"+dat[i].min_value+"','"+min_date+"');";
    sqlArr.push(str);
  }
  exec(sqlArr);
}*/

function WriteMySql(row)
{
	var query = mysql_con.query('INSERT INTO '+table_name+' SET ?', row, function (error, results, fields) {
		if (error){
			//console.log(error);
			fs.appendFileSync("Write.Sql",query.sql+'\n');
			//mysql_con.destroy();
			//mysqlConnect();
		}
	});
}
function WriteSql(dat)
{
	async.series([
		function(done){
	   //执行sql
		for(var i in dat){
			//循环中含有异步，这里使用闭包执行
				(function(idx){
					var row = {};
					row.tagname = dat[idx].longName;
					row.write_time = dat[idx].time;
					row.avg_value = dat[idx].avg;
					row.max_value = dat[idx].max_value;
					row.max_time = dat[idx].max_time;
					row.min_value = dat[idx].min_value;
					row.min_time = dat[idx].min_time;
					if (row.avg_value && row.max_time >= row.write_time) {
						var query = mysql_con.query('INSERT INTO '+table_name+' SET ?', row, function (error, results, fields) {
							if (error){
								fs.appendFileSync("Write.Sql",query.sql+'\n');
							}
							if(idx>=dat.length-1)
							{
								 done();	
							} 
						});
					}else{
						if(idx>=dat.length-1)
						{
							 done();	
						} 
					}       
			})(i);
		}
    },
    function(err,result){
    }]);
}

var cur_his_time = new Date();
cur_his_time.setTime(start_time.getTime());
var end_his_time = new Date();
end_his_time.setTime(cur_his_time.getTime()+3600000);

function WriteHisSql(tagName, cur_time, resVal)
{
	//console.log('WriteHisSql------'tagName,cur_time,resVal[0]);
	if (resVal.length < 1) {
		return ;
	};
	var agg = {
		time:cur_time,
		longName:tagName,
		avg:resVal[0].value,
		count:1,
		max_value:resVal[0].value,
		max_time:resVal[0].time,
		min_value:resVal[0].value,
		min_time:resVal[0].time
	};
	for(var n = 1; n < resVal.length; n++)
	{
		agg.avg = (agg.avg*agg.count+resVal[n].value)/(agg.count+1);
		agg.count++;
		if (agg.max_value < resVal[n].value) {
			agg.max_value = resVal[n].value;
			agg.max_time = resVal[n].time;
		};
		if (agg.min_value > resVal[n].value) {
			agg.min_value = resVal[n].value;
			agg.min_time = resVal[n].time;
		};
	}
	WriteSql([agg]);
}

function ReadHis()
{
	if (cur_his_time >= start_real) {
		console.log('回补历史统计值完成: '+formatDate(cur_his_time));
		return ;
	};
	console.log('读取历史统计值: '+formatDate(cur_his_time));
	  async.series([
		function(done){
	   //执行sql
		for(var i in taglist){
			//循环中含有异步，这里使用闭包执行
			(function(idx){
				con.read(taglist[idx]+'.his.raw',cur_his_time,end_his_time,10000,function(err,resVal){
					if(err){
						console.log(err);
					}else{
						WriteHisSql(taglist[idx], cur_his_time, resVal);
					}
					if(idx>=taglist.length-1)
					{
						 done();	
					}        
				});
			})(i);
		}
    },
    function(err,result){
    	cur_his_time.setTime(end_his_time.getTime());
		end_his_time.setTime(end_his_time.getTime()+3600000);
		setTimeout(ReadHis, read_his_timer);
    }]);
}

//last_aggregate 目的是处理所有测点实时数据时间戳可能不一致情况
//目前还没有出来，小于cur_aggregate.time 的丢弃
function PubReal(value)
{
	for(var n = 0; n < value.length; n++)
	{
		if (cur_aggregate.time<=value[n].value.time)
		{
			var t = value[n].value.time.getTime() - cur_aggregate.time.getTime();
			//大于30秒写入
			if (t > 30000 && last_aggregate.time && last_aggregate.agg) 
			{
				console.log('t > 30000)');
				var arr = [];
				for (var name in last_aggregate.agg)
				{
					arr.push(last_aggregate.agg[name]);
				}
				WriteSql(arr);	
				last_aggregate = {};	
			}
			else if (t >= 3600000) 
			{
				console.log('t > 3600000)');
				if (last_aggregate.time && last_aggregate.agg) {
					var arr = [];
					for (var name in last_aggregate.agg)
					{
						arr.push(last_aggregate.agg[name]);
					}
					WriteSql(arr);	
					last_aggregate = {};	
				};
				last_aggregate = cur_aggregate;
				cur_aggregate = {};
				var thour = value[n].value.time.getTime();
				thour = parseInt(thour/3600000)*3600000;
				cur_aggregate.time = new Date();
				cur_aggregate.time.setTime(thour);
				console.log("开始实时统计:"+formatDate(cur_aggregate.time));
			};
			if (!cur_aggregate.agg) {
				cur_aggregate.agg = {};
			};
			if (cur_aggregate.agg[value[n].name]) {
				
				cur_aggregate.agg[value[n].name].avg = (cur_aggregate.agg[value[n].name].avg
					*cur_aggregate.agg[value[n].name].count+value[n].value.value)/(cur_aggregate.agg[value[n].name].count+1);
				cur_aggregate.agg[value[n].name].count++;
				if (cur_aggregate.agg[value[n].name].max_value<value[n].value.value) {
					cur_aggregate.agg[value[n].name].max_value=value[n].value.value;
					cur_aggregate.agg[value[n].name].max_time=value[n].value.time;
				};
				if (cur_aggregate.agg[value[n].name].min_value > value[n].value.value) {
					cur_aggregate.agg[value[n].name].min_value=value[n].value.value;
					cur_aggregate.agg[value[n].name].min_time=value[n].value.time;
				};
			}
			else{
				cur_aggregate.agg[value[n].name] = {
					time:cur_aggregate.time,
					longName:value[n].name,
					avg:value[n].value.value,
					count:1,
					max_value:value[n].value.value,
					max_time:value[n].value.time,
					min_value:value[n].value.value,
					min_time:value[n].value.time
				};
				//console.log("订阅值:",cur_aggregate.agg[value[n].name]);
			}
		}
		//小于cur_aggregate.time 丢弃
	}
	//console.log("订阅值:",value);
}

function ReadReal()
{
	con.sub(taglist,function(err,subid,curVal,value){
		if(err){
			console.log('===sub '+err);
		}else{
			if(curVal){
				var maxTime = curVal[0].value.time;
		        for(var n = 1; n < curVal.length; n++)
		        {
		            if(maxTime < curVal[n].value.time)
		            {
		                maxTime = curVal[n].value.time;
		            }
		        }
		        //console.log(curVal[0]);
		        var thour = maxTime.getTime();
				thour = parseInt(thour/3600000)*3600000+3600000;
				start_real.setTime(thour);
				cur_aggregate.time = start_real;
				end_time = start_real;
				console.log("查询历史起始时间:", formatDate(start_time), "结束时间:", formatDate(end_time));
				setTimeout(ReadHis, read_his_timer);
			}
			if(value && real_router){
				PubReal(value);
			}
		}
	});
}

function main()
{
	var thour = start_real.getTime();
	thour = thour/3600000*3600000+3600000;
	start_real.setTime(thour);
	csv().from.path(tag_path).to.array(function(rows){
	console.log("读取测点数量", rows.length);
        for(var n = 0; n < rows.length; n++)
        {
        	var lname = rows[n][0];
        	if (lname.length > 0) {
        		lname = lname.replace(/\\/g,"/");
        		lname = lname.replace(/-/g,"_");
        		lname = lname.replace(/_PV/g,"");
	        	if (lname.charAt(0)!='/') {
	        		lname = '/'+lname;
	        	};
	        	var props = con.read(lname+'.tagID');
	        	if (props instanceof ps.Err) {

	        	}
	        	else
	        	{
	        		taglist.push(lname);
	        	}
        	};

        }
        if(taglist.length > 1)
        {
            //订阅并读取当前实时值
            ReadReal();
        }
        console.log(taglist);
    });
}
main();