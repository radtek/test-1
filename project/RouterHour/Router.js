/*************************************************配置区域BEGIN******************************************************/
var tag_path = 'taglist.csv';//统计测点列表
var start_time = new Date("2017-11-6 8:00:00");//统计起始时间
var end_time = new Date();//统计结束时间 real_router设置时无效
var real_router = true;//是否实时统计
var read_his_timer = 2000;//读取历史数据间隔周期 单位毫秒
var router_timer = 3600000;//转储周期 单位毫秒

//pSpace服务器连接参数，json格式
var con_str = {  
	"host": "localhost",
	"user": "admin",
	"password": "admin888",
	"port": 8889  //可选
};

//链接mysql
var mysql_info = {
	host: '192.168.2.104',
	user: 'zdf',
	password: '123456',
	database: 'test',
	port: 3306
};
var table_name = 'table_hour';
/*************************************************配置区域END******************************************************/

var pSpace = require('pSpace');
var fs = require('fs');
var csv = require('CSV');
var mysql = require('mysql');
var async = require("async");
var log4js = require('log4js');
var path = require('path');

var taglist = [];//存储测点长名                 
var start_real = new Date();
var cur_aggregate = {};
var last_aggregate = {};

//初始化日志管理
function initLog(dirPath) {
	log4js.configure({
		appenders: [
			{
				type: 'console',
				category: "console"
			}, //控制台输出  
			{
				type: "file",
				filename: 'Router.log',
				maxLogSize: 1048576,
				backups: 50,
				category: 'console'
			}//日期文件格式  
		],
		replaceConsole: true,   //替换console.log  
		levels: {
			"log_file": "INFO",
			"console": "INFO"
		}
	});
}
initLog();

//维护pSpace长链接
pSpace.openConn('pSpace', con_str.host, con_str.user, con_str.password);
pSpace.openConnPro();

//链接mysql
var mysql_con = {};
function mysqlConnect() {
	mysql_con = mysql.createConnection(mysql_info);
	mysql_con.connect(function (err) {
		if (err) {
			console.log('error connecting: ' + err.stack);
		} else {
			console.log("mysql连接完毕。");
		}
	});
}
mysqlConnect();

//mysql建表
function CreateTable() {
	mysql_con.query('create table IF NOT EXISTS ' + table_name + ' (tagname varchar(255) not null,write_time datetime not null,avg_value float not null,max_value float not null,max_time datetime  not null,min_value float not null,min_time datetime  not null)', function (error, results, fields) {
		if (error) {
		} else {
			mysql_con.query("ALTER TABLE table_hour ADD UNIQUE INDEX uni(tagname, write_time)", function (error1, results1, fields1) {
				if (!error1) {
					console.log("创建表:" + table_name);
				}
			});
		}
	});
}
CreateTable();

//格式化时间
function Appendzero(obj) {
	if (obj < 10) return "0" + "" + obj;
	else return obj;
}
function Append3zero(obj) {
	if (obj < 10) return "00" + "" + obj;
	if (obj < 100) return "0" + "" + obj;
	return obj;
}
function formatDate(time) {
	var year = time.getFullYear();
	var month = time.getMonth() + 1;
	var date = time.getDate();
	var hour = time.getHours();
	var minute = time.getMinutes();
	var second = time.getSeconds();
	var milSecond = time.getMilliseconds();
	return year + "-" + Appendzero(month) + "-" + Appendzero(date) + " " + Appendzero(hour) + ":" + Appendzero(minute) + ":" + Appendzero(second) + "." + Append3zero(milSecond);
}

//将数据拼接成sql语句并插入mysql
function WriteSql(dat) {
	async.series([
		function (done) {
			//执行sql
			for (var i in dat) {
				//循环中含有异步，这里使用闭包执行
				(function (idx) {
					var row = {};
					row.tagname = dat[idx].longName;
					row.write_time = dat[idx].time;
					row.avg_value = dat[idx].avg;
					row.max_value = dat[idx].max_value;
					row.max_time = dat[idx].max_time;
					row.min_value = dat[idx].min_value;
					row.min_time = dat[idx].min_time;
					if (row.avg_value && row.max_time >= row.write_time) {
						var query = mysql_con.query('INSERT INTO ' + table_name + ' SET ?', row, function (error, results, fields) {
							if (error) {
								console.log(error);
								fs.appendFileSync("Write.Sql", query.sql + '\n');
							}
							if (idx >= dat.length - 1) {
								done();
							}
						});
					} else {
						if (idx >= dat.length - 1) {
							done();
						}
					}
				})(i);
			}
		},
		function (err, result) {
		}]);
}

//1、WriteHisSql对读到的历史数据进行最大、最小、平均值统计;
//2、调用WriteSql插入mysql
var cur_his_time = new Date();
cur_his_time.setTime(start_time.getTime());
var end_his_time = new Date();
end_his_time.setTime(cur_his_time.getTime() + router_timer);
function WriteHisSql(tagName, cur_time, resVal) {
	//console.log('WriteHisSql------'tagName,cur_time,resVal[0]);
	if (resVal.length < 1) {
		return;
	};
	var agg = {
		time: cur_time,
		longName: tagName,
		avg: resVal[0].value,
		count: 1,
		max_value: resVal[0].value,
		max_time: resVal[0].time,
		min_value: resVal[0].value,
		min_time: resVal[0].time
	};
	for (var n = 1; n < resVal.length; n++) {
		agg.avg = (agg.avg * agg.count + resVal[n].value) / (agg.count + 1);
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

function ReadHis() {
	if (cur_his_time >= start_real) {
		console.log('回补历史统计值完成: ' + formatDate(cur_his_time));
		return;
	};
	console.log('读取历史统计值: ' + formatDate(cur_his_time));
	async.series([
		function (done) {
			//执行sql
			for (var i in taglist) {
				//循环中含有异步，这里使用闭包执行
				(function (idx) {
					var hisArray = pSpace.readHisRaw(taglist[idx],cur_his_time,end_his_time,false,10000);//不能开bound，否则会读到不在统计时段内的数据;
					if(hisArray.hasOwnProperty("errString")){
							console.log(hisArray.code,hisArray.errString);
						}else{
							//过滤掉value为undefined的数据
							var TempArr = [];
							for (var i in hisArray) {
								if (hisArray[i] != undefined) {
									TempArr.push(hisArray[i]);
								}
							}
							WriteHisSql(taglist[idx], cur_his_time, TempArr);
						}
					if (idx >= taglist.length - 1) {
						done();
					}
				})(i);
			}
		},
		function (err, result) {
			cur_his_time.setTime(end_his_time.getTime());
			end_his_time.setTime(end_his_time.getTime() + router_timer);
			setTimeout(ReadHis, read_his_timer);
		}]);
}

//last_aggregate 目的是处理所有测点实时数据时间戳可能不一致情况
//目前还没有出来，小于cur_aggregate.time 的丢弃
function PubReal(value) {
	for (var n = 0; n < value.length; n++) {
		if (cur_aggregate.time <= value[n].value.time) {
			var t = value[n].value.time.getTime() - cur_aggregate.time.getTime();
			//大于30秒写入
			if (t > 30000 && last_aggregate.time && last_aggregate.agg) {
				console.log('t > 30000)');
				var arr = [];
				for (var name in last_aggregate.agg) {
					arr.push(last_aggregate.agg[name]);
				}
				WriteSql(arr);
				last_aggregate = {};
			}
			else if (t >= router_timer) {
				console.log('t > ' + router_timer);
				if (last_aggregate.time && last_aggregate.agg) {
					var arr = [];
					for (var name in last_aggregate.agg) {
						arr.push(last_aggregate.agg[name]);
					}
					WriteSql(arr);
					last_aggregate = {};
				};
				last_aggregate = cur_aggregate;
				cur_aggregate = {};
				var thour = value[n].value.time.getTime();
				thour = parseInt(thour / router_timer) * router_timer;
				cur_aggregate.time = new Date();
				cur_aggregate.time.setTime(thour);
				console.log("开始实时统计:" + formatDate(cur_aggregate.time));
			};
			if (!cur_aggregate.agg) {
				console.log("cur_aggregate:", cur_aggregate);
				cur_aggregate.agg = {};
			};
			if (cur_aggregate.agg[value[n].name]) {

				cur_aggregate.agg[value[n].name].avg = (cur_aggregate.agg[value[n].name].avg
					* cur_aggregate.agg[value[n].name].count + value[n].value.value) / (cur_aggregate.agg[value[n].name].count + 1);
				cur_aggregate.agg[value[n].name].count++;
				if (cur_aggregate.agg[value[n].name].max_value < value[n].value.value) {
					cur_aggregate.agg[value[n].name].max_value = value[n].value.value;
					cur_aggregate.agg[value[n].name].max_time = value[n].value.time;
				};
				if (cur_aggregate.agg[value[n].name].min_value > value[n].value.value) {
					cur_aggregate.agg[value[n].name].min_value = value[n].value.value;
					cur_aggregate.agg[value[n].name].min_time = value[n].value.time;
				};
			}
			else {
				cur_aggregate.agg[value[n].name] = {
					time: cur_aggregate.time,
					longName: value[n].name,
					avg: value[n].value.value,
					count: 1,
					max_value: value[n].value.value,
					max_time: value[n].value.time,
					min_value: value[n].value.value,
					min_time: value[n].value.time
				};
				//console.log("订阅值:",cur_aggregate.agg[value[n].name]);
			}
		}
		//小于cur_aggregate.time 丢弃
	}
	//console.log("订阅值:",value);
}

function ReadReal() {
	pSpace.realDataSub(taglist, function (err, subid, curVal, value) {
		if (err) {
			console.log('===sub ' + err);
		} else {
			if (curVal) {
				var maxTime = curVal[0].value.time;
				for (var n = 1; n < curVal.length; n++) {
					if (maxTime < curVal[n].value.time) {
						maxTime = curVal[n].value.time;
					}
				}
				var thour = maxTime.getTime();
				thour = parseInt(thour / router_timer) * router_timer + router_timer;
				start_real.setTime(thour);

				//统计完历史进行实时数据统计，进行实时统计时的起始时间应该是当前时刻的整点时间;
				//比如：14:24统计完历史，再进行实时数据统计时应该是从14:00开始
				var time = new Date();
				time.setTime(thour - router_timer);
				cur_aggregate.time = time;

				end_time = start_real;
				console.log("查询历史起始时间:", formatDate(start_time), "结束时间:", formatDate(end_time));
				setTimeout(ReadHis, read_his_timer);
			}
			if (value && real_router) {
				PubReal(value);
			}
		}
	});
}

function main() {
	var thour = start_real.getTime();
	thour = thour / router_timer * router_timer + router_timer;
	start_real.setTime(thour);
	csv().from.path(tag_path).to.array(function (rows) {
		console.log("读取测点数量", rows.length, ' 转储周期', router_timer / 1000, '秒');
		for (var n = 0; n < rows.length; n++) {
			var lname = rows[n][0];
			if (lname.length > 0) {
				lname = lname.replace(/\\/g, "/");
				lname = lname.replace(/-/g, "_");
				lname = lname.replace(/_PV/g, "");
				if (lname.charAt(0) != '/') {
					lname = '/' + lname;
				};
				lname = 'pSpace' + lname;
				var props = pSpace.getTagProps(lname, ['TagId']);
				if (props.hasOwnProperty("errString")) {

				} else {
					taglist.push(lname);
				}
			};

		}
		if (taglist.length > 0) {
			//订阅并读取当前实时值
			ReadReal();
		}
		//console.log("taglist:",taglist);
	});
}
main();