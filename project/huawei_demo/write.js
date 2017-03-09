var addon = require('pSpace');
var fs = require('fs');
var csv = require('CSV');
var async = require("async");
var log4js = require('log4js'); 
var path = require('path');
var MongoClient = require('mongodb').MongoClient;

var ps = new addon();
var Err = ps.Err;

/////////////////////

var tag_path = "2#空分机组点数.csv";
var data_path = "1.5.csv";
var alarm_path = 'd2.csv';
var predict_path = 'C:/predict.txt';

var write_timer_ms = 2000;

var mongodb_url = 'mongodb://196.168.65.157:27017/AlarmCenter';
var mongodb_db = {};
var con_str = {  //pSpace服务器连接参数，json格式
	"host":"localhost",
	"user":"admin",
	"password":"admin888",
	"port":8889  //可选
};

var csv_datas = [];
var alarm_datas = [];
var exist_tas = {};
var write_index = 0;
var read_predict_ms = 100;

/////////////////////////

function  initLog(dirPath) {
    log4js.configure({  
    appenders: [  
        {  
            type: 'console',  
            category: "console"  
        }, //控制台输出  
        {  
            type: "file",  
            filename: 'Write.log',  
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

var con = ps.open(con_str);
if(con instanceof Err){
	console.log('pSpace同步连接失败:',con.errString);
}else{
	console.log('pSpace同步连接成功！');
}


function createTag(tagname, unit, desc)
{
	var props = con.read('/'+tagname+'.tagID');
	if (props instanceof ps.Err) {
		var analog = {
		TagType : "analog",
		DataType : "double",
		EngineeringUnit:unit,
		Description : desc
		}
		con.add('/'+tagname,analog);
		return 1;
	}
	return 0;
}

var read_predict_idx = 0;
var read_predict_len = 0;
var read_predict_t = 0;

var TIA024500A_alarm = false;
var TIA024500A_doc = {};

var VIA024502X_alarm = false;
var VIA024502X_doc = {};

function startAlarm(doc)
{
	var collection = mongodb_db.collection('Alarm_Real');
	collection.insertOne(doc, function(err, r){
		if (err) {
			console.log('写报警失败 '+err);
		}else{
			console.log('产生报警 '+doc["ruleName"]+' 值'+doc["curData"][0]);
		}
	});
}
function endAlarm(filter, doc)
{
	var collection = mongodb_db.collection('Alarm_Real');
	collection.updateOne(filter, doc, function(err, result) {
		if (err) {
			console.log('更新报警失败 '+err);
		}else{
			console.log('报警结束 '+doc["ruleName"]);
		}
	});
}
function readPredict()
{
	setTimeout(readPredict, read_predict_ms);
	fs.readFile(predict_path, 'utf8', function(err, data){
		if (!err){
			if (read_predict_idx > 0 && data.length > read_predict_len) {
				var limit = parseFloat(data.substr(read_predict_len+1));
				//console.log('limit '+limit+' '+alarm_datas[read_predict_idx][12]);
				if (limit < alarm_datas[read_predict_idx][12]) {
					if (!VIA024502X_alarm) {
						var doc = {
							"startTime" : read_predict_t,
						    "companyName" : "华为",
						    "modelName" : "空压机",
						    "ruleName" : "VIA024502X_HIGH_"+limit,
						    "tags" : [ 
						        "空压机排气侧振动X um"
						    ],
						    "ruleLevel" : 1,
						    "curData" : [ 
						        parseFloat(alarm_datas[read_predict_idx][12])
						    ],
						    "lastRealTime" : 0,
						    "endTime" : 0,
						    "stillTime" : 0,
						    "modelType" : 0,
						    "confirmFlag" : 0
						};
						VIA024502X_alarm = true;
						VIA024502X_doc = doc;
						startAlarm(doc);
					};
					read_predict_idx = 0;
				}else{
					if (VIA024502X_alarm) {
						VIA024502X_alarm = false;
						var filter = {
							"startTime" : VIA024502X_doc["startTime"],
							"ruleName" : VIA024502X_doc["ruleName"]
						}
						VIA024502X_doc["endTime"] = read_predict_t;
						endAlarm(filter, VIA024502X_doc);
						console.log('limit '+limit+' '+alarm_datas[read_predict_idx][12]);
					};
					read_predict_idx = 0;
				}
			};	
		}
	});
}

function writeAlarm(idx, t)
{
	if (idx < alarm_datas.length) {
		if (parseFloat(alarm_datas[idx][8]) > 72.6) {
			if (!TIA024500A_alarm) {
				var doc = {
					"startTime" : t,
				    "companyName" : "华为",
				    "modelName" : "空压机",
				    "ruleName" : "TIA024500A_HIGH_72.6",
				    "tags" : [ 
				        "空压机排气侧轴承温度 ℃"
				    ],
				    "ruleLevel" : 1,
				    "curData" : [ 
				        parseFloat(alarm_datas[idx][8])
				    ],
				    "lastRealTime" : 0,
				    "endTime" : 0,
				    "stillTime" : 0,
				    "modelType" : 0,
				    "confirmFlag" : 0
				};
				TIA024500A_doc = doc;
				TIA024500A_alarm = true;
				startAlarm(doc);
			};
		}else{
			if (TIA024500A_alarm) {
				TIA024500A_alarm = false;
				var filter = {
					"startTime" : TIA024500A_doc["startTime"],
					"ruleName" : TIA024500A_doc["ruleName"]
				}
				TIA024500A_doc["endTime"] = t;
				endAlarm(filter, TIA024500A_doc);
			};
		}

		if (read_predict_idx==0) {
			var wd = ''+parseInt(t/1000);
			for (var n = 1; n < alarm_datas[idx].length; n++) {
				wd += ','+alarm_datas[idx][n];
			};
			fs.writeFileSync(predict_path, wd);
			read_predict_len = wd.length;
			read_predict_idx = idx;
			read_predict_t = t;
			setTimeout(readPredict, read_predict_ms);
		};

	}
}

function writeReal()
{
	write_index++;
	if (write_index >= csv_datas.length) {
		console.log('写实时数据完成');
		return ;
	};
	setTimeout(writeReal, write_timer_ms);
	var wdata = {};
	var t = new Date();
	for(var tag in exist_tas){
		var data  = {
			"value":parseFloat(csv_datas[write_index][exist_tas[tag]]),
			"quality":"good",
			"time":t
		};
		wdata[tag] = data;
	}
	//console.log(wdata);
	var res = con.write(wdata);
	if (res instanceof ps.Err) {
		console.log(res);
	}
	else
	{
		writeAlarm(write_index, t);
		console.log('write real '+ write_index);
	}
}

function existTag()
{
	var tag_count = 0;
	for(var col in csv_datas[0]){
		if (col == 0) {
			continue;
		};
		var props = con.read('/'+csv_datas[0][col]+'.tagID');
		if (props instanceof ps.Err) {
		}else{
			exist_tas['/'+csv_datas[0][col]+'.pv'] = col;
			tag_count++;	
		}
	}
	console.log('读取数据条数'+ (csv_datas.length-1)+' 总测点数'+(csv_datas[0].length-1)+' 有效测点数'+tag_count);
	setTimeout(writeReal, write_timer_ms);
}

function main()
{
	var create_tags = 0;
	MongoClient.connect(mongodb_url, function(err, db) {
		if (err) {
			console.log('连接mongodb失败 '+err);
			return ;
		};
		console.log('连接mongodb成功');
		mongodb_db = db;
		csv().from.path(tag_path).to.array(function(rows){
			for(var n = 0; n < rows.length; n++)
	        {
	            if(parseInt(rows[n][0]) > 0)
	            {
	                create_tags += createTag(rows[n][1], rows[n][3],rows[n][4]);
	            }
	        }
	        console.log('创建tag数量'+create_tags);

	        csv().from.path(data_path).to.array(function(datas){
	        	if (datas.length > 1) {
	        		csv_datas = datas;
	        		csv().from.path(alarm_path).to.array(function(datas_d2){
			        	if (datas_d2.length > 1) {
			        		alarm_datas = datas_d2;
							existTag();
			        	}
	        		});
	        	};
			});
		});
	});
}

main();