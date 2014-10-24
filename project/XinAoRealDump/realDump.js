var fd = require('readlines');
var fs = require('fs');
var addon = require('pSpace');
var configure = require('configure');
var sql = require('msnodesql');
var iconv = require('iconv-lite');
var async = require("async");
var ps  = new addon();
var logger = require('log').initLog(__dirname);;
var assert = require( 'assert' );
var sqlFun = require('sql');
var guard= require('guard');
guard.start();


var ps_con = new Object();
//var sql_con = new Object();
var configPath = configure.configPath;
var isFirstCreateTable = true;
//保存测点长名
var ps_tagname = [];
//保存sql的短名
var sql_tagname = [];
var ps_data;

var table_name = configure.realTableName;
//数据锁
var lock = false;
function handleError () {
   var conn = ps.open(configure.pSpace);
   if(conn instanceof ps.Err && !ps.isConnected()){
	   	logger.error("连接pSpace数据库失败，将重连:",conn.errString);
	   	lock = false;
	    //setTimeout(handleError,2000);
	    handleError();
   }else{
    	ps_con=conn;
   }
}

function readFile(path)
{
	var exists = fs.existsSync(path);
	if(!exists){
		logger.error("文件不存在,请检查配置文件路径并重新配置");
	}else{
		logger.trace("正在读取相关配置文件。");
	    var lines =fd.readlinesSync(path,"binary");
	    for(var line in lines){
	        var buf = new Buffer(lines[line], 'binary');
	        var tagName = iconv.decode(buf, 'gbk');
		if(tagName!=''){
			var position = tagName.lastIndexOf("\\"); 
	        	var sqlTagName = tagName.substring(position+1);
	       		sql_tagname.push(sqlTagName);
	        	//长名格式转化
	        	tagName = tagName.replace(/[\\]/g,"/")+".pv";
	        	ps_tagname.push(tagName);
		}
	        
	    };
	    logger.trace("配置文件读取完毕!");
	}
	
}
handleError();
readFile(configPath);

function readData(names)
{
	logger.trace("will read data.");
	if(!lock){
		if(ps.isConnected()){
      //console.log("names:::::",names);
			logger.info("pSpace 连接成功，正在读取实时数据");
			var resVal = ps_con.read(names);
		    if(resVal instanceof ps.Err){
		        logger.error(resVal.errString);
		        return false;
		    }else{
		    	if (resVal==undefined || resVal[0]==undefined || resVal[0].value==undefined) {
		    		lock = false;
		    		logger.error("获取到的数据类型有错误");
		    		return false;
		    	}
		        ps_data = resVal;
		        lock = true;
		        return true;
		    }
		}else{
			logger.error("连接已经断开，正在重连....");
			lock = false;
			handleError();
			//重连之后继续读数据
			readData(names);
		}
	}else{
		logger.info("上一次的数据正在处理，等待操作..");
		return false;
	}
	
}
function work(sqlCon){
	logger.trace("sqlserver连接成功，正在转储....");
	if(ps_tagname.length==0 || sql_tagname.length==0)
		return;
	logger.trace("call readData.");
	var res = readData(ps_tagname);
	if(res){
//console.log("sqllength:",sql_tagname.length);
//console.log("palength:",ps_tagname.length);
//console.log("datalenth:",ps_data.length);
		logger.trace("数据读取成功，开始转储任务...");
		if(isFirstCreateTable){
			logger.info("程序第一次运行，将删除重名的表。");
			async.series([
				function( done ) { 
					var dropSql = "if OBJECT_ID ("+"'"+table_name+"'"+") is not null drop table "+table_name+";";
                	sqlFun.execQuerySql(sqlCon,dropSql,done);
                    logger.trace("第一次运行，有表删除.");            	
            	},
                function( done ) {
                	var createSql = "create table ["+table_name+"](Time datetime,TagName nvarchar(255) primary key,PV float);";
                    sqlFun.execQuerySql(sqlCon,createSql,done);
                    logger.trace("创建表!");
                },
                function( done ) {
                	sqlFun.insertData(sqlCon,ps_data,table_name,sql_tagname,logger,done);
                }
			],function(err,results){
				if(err){
					logger.error("数据插入失败!");
					 lock= false;
					 task();
					 return;
				}else{
					logger.info("数据插入成功!");
					isFirstCreateTable = false;
				    lock= false;
				   // ps_data = null;
				    ps_data = [];
				    return;
				}
			});
		}else{
			logger.info("非首次运行!");
				async.waterfall( [
					function(done) {
				    	//(function(done){
			    		sqlCon.queryRaw("select count(1) from sys.objects where name ='"+table_name+"';",function(err,cnt){
			    			if(err){
			    				logger.error(err);
			    				done(err);
			    			}else{
			    				logger.trace("判断是否存在tbssdata");
								done(null,cnt.rows[0][0]);
			    			}
						});
				    	//})(done);
				    },
				    function(cnt,done) {
				    	if(0===cnt){
				    		logger.trace("tbssdata不存在，将创建!");
			    			async.series( [
				                function( done ) { 
				                	var createSql = "create table ["+table_name+"](Time datetime,TagName nvarchar(255) primary key,PV float);";
				                	sqlFun.execQuerySql(sqlCon,createSql,done);
				                    logger.trace("表创建成功。"); 
				                       
				                },
				                function( done ) {
				                	logger.info("正在插入数据.");
				                	sqlFun.insertData(sqlCon,ps_data,table_name,sql_tagname,logger,done);	
				                } 
			           		],function(err,results){
			           			if(err){
			           				lock = false;
				                	logger.error("非首次运行插入数据失败");
				                	task();
				                	return;
			           			}else{
			           				lock = false;
			           				//ps_data = null;
			           				ps_data =[];
				                	logger.info("非首次运行插入数据完毕!");
				                	return;
			           			}

			           		});

				    	}else{
				    		logger.trace("表存在，将更新!");
				    		sqlFun.updateData(sqlCon,ps_data,table_name,sql_tagname,logger,done);
				    	}
				    }
				],function(err,results){
					if(err){
						lock = false;
						logger.error("更新数据错误!");
						task();
						return;
					}else{
						lock = false;
						//ps_data = null;
						ps_data = [];
						logger.info("更新数据完毕!");
						return;
					}
				});
		}
	}else{
		lock = false;
		return;
	}
}


function task(){
	sql.open(configure.sqlserver, function( err, sqlCon) {
 		if(err){
 			logger.error("sqlserver连接失败，将重连：",err);
 			//console.log("openerr:",err);
 			setTimeout(task,1000);
 			return;
 		}else{
 			work(sqlCon);
 			interVal = setInterval(work,configure.interVal,sqlCon);
 			//return;			
	 	}
 });
}
task();
guard.startError();
guard.onStop(function(err,result){
    if(err){
    }else{
    	logger.info("收到停止信号");
        process.exit(1);
    }
});
