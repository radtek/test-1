var csv = require("csv");
var fs = require('fs');
var addon = require('pSpace');
var configure = require('configure');
var sql = require('msnodesql');
var iconv = require('iconv-lite');
var ps  = new addon();

function Appendzero(obj)
{
     if(obj<10) return "0" +""+ obj;
     else return obj;     
}
var formatDate = function(now){
	var   year=now.getFullYear();     
    var   month=now.getMonth()+1;     
    var   date=now.getDate();     
    var   hour=now.getHours();     
    var   minute=now.getMinutes();    
    var   second=now.getSeconds(); 
   // var   milSecond = now.getMilliseconds(); 
    //console.log("milsecond:",milSecond);    
   return   year+"-"+Appendzero(month)+"-"+Appendzero(date)+" "+Appendzero(hour)+":"+Appendzero(minute)+":"+Appendzero(second);     
}

//新建连接对象
var con = new Object();
//断线重连的操作
function handleError () {
    ps.open(configure.pSpace,function (err,conn) {
        if (err) {
            console.log('error when connecting to db:', err);
        }else{
           con = conn;
        }
    });
}
//监听数据库是否连接的事件
process.on('isConnect',function(){
	if(!ps.isConnected()){
		handleError();
	}
});
var fun = function()
{
	process.emit('isConnect',null);
}
handleError();
//保存文件状态
var fileLock = [];
//任务
function task (filePath,dirPath) {
	//计算文件第三行的起始位置
	var fileData = fs.readFileSync(filePath, 'ASCII');
	var first = fileData.indexOf('\n');
	var two = fileData.indexOf('\n',first+1);
	//获取站名
	var pp = filePath.indexOf("_"); 
	var station = filePath.substring(0,pp);
	var pp1 = station.lastIndexOf("\/");
	var stationName = station.substring(pp1+1);
	///////////////////////////
	//console.log("stationName:",stationName);
	var configPath = configure.configPath;
	fs.exists(configPath,function(exists){
		if(!exists){
			console.log("配置文件不存在.");
			return;
		}else{
			csv()
			.from.path(filePath,{
			 columns:true,
			 start:two,
			 encoding: 'ASCII'
			})
			.transform(function(data,index){
			 	 return data;
			})
			.to.array( function(data){
				csv()
				.from.path(configPath,{
					columns:true,
					encoding: 'binary'
				})
				.transform(function(configData,index){
			 	 	return configData;
				})
				.to.array(function(configData){
					//获取配置文件的行数
					var confLine = configData.length;
					//获取配置文件所有的字段
					var confKeys = Object.keys(configData[0]);
					for(var i=0;i<confLine;i++){
						if(stationName==configData[i]["PLC_Station_Name"]){
							//console.log("message:",configData[i]["PLC_Station_Name"]);
							//获取数据的行数
							var len = data.length;
							//获取所有的字段
							var keys = Object.keys(data[0]);
							for(var k=0;k<keys.length;k++){
								if(configData[i]["PLC_TagName"]===keys[k]){
									//获取长名
									var tagLongName = configData[i]["pSpace_LongTagname"];
									//长名格式转化
									tagLongName = tagLongName.replace(/[\\]/g,"/")+".his.insertReplace";
									var buf = new Buffer(tagLongName, 'binary');
									var strName = iconv.decode(buf, 'gbk');
									//console.log("tagName:",strName);
									//将数据一条一条的插入
									for(var s=0;s<len;s++){
										//console.log(data[s][keys[k]]);
										var hisData = {
											"value":Number(data[s][keys[k]]),
											"time":new Date(data[s]["Time"]),
											"quality":"good"
										};
										var res = con.write(strName,hisData);
										if(res instanceof ps.Err){
											console.log(res.errString);
											continue;
										}else{
											//
											
										}
									}

								}
							}

						}
					}
					//是否需要转储
					if(configure.isSaveToSql){
						toSqlserver(configData,data,stationName);
					}
					//处理完毕，改变文件状态，备份并删除文件
					//截取备份文件文件名
					if (filePath.indexOf(configPath)<0) {
						var p = filePath.lastIndexOf('\/');
				        var fileName = filePath.substring(p);
						//要备份的文件夹是否存在
						fs.exists(dirPath+"/backup", function(exits){
							if(!exits){
								//不存在，创建并拷贝
								fs.mkdirSync(dirPath+"\/backup");
								fs.writeFile(dirPath+"\/backup"+"\/"+fileName,fileData, function(err){ 
									if(err){
										console.log("备份文件错误.");
										return;
									}else{
										//删除原文件
										fs.unlink(filePath, function(err) {
											if(err){
												console.log(err);
												return;
											}else{
												//删除文件状态
												delete fileLock[filePath];
											}
										});
									}
								});
				               
							}else{
								fs.writeFile(dirPath+"\/backup"+"\/"+fileName,fileData, function(err){ 
									if(err){
										console.log("备份文件错误.");
										return;
									}else{
										//删除原文件
										fs.unlink(filePath, function(err) {
											if(err){
												console.log(err);
												return;
											}else{
												//删除文件状态
												delete fileLock[filePath];
											}
										});
									}
								});

							}
						});
			
					}

				})
				
				.on('close', function(count){

				})
				.on('error', function(error){
			  		console.log(error.message);
				});	
			})
			.on('close', function(count){

			})
			.on('error', function(error){
			  console.log(error.message);
			});	

		}
	});
}

//对给定的文件夹进行处理
function dealWithFile(path)
{
	//保存csv文件名
	fs.exists(path,function(exists){
        if(!exists){
        	console.log("指定的文件夹不存在，请重新确定指定路径是否正确!");
        	return;
        }else{
        	//文件夹存在，查看文件夹中是否有文件
        	var files = fs.readdirSync(path);
        	if(0==files.length){
        		//没有文件
        		return;
        	}else{
        		//有文件，查看是否有csv文件
        		for(var i=0;i<files.length;i++){
        			var pointPosition = files[i].lastIndexOf('.');
  			 		var mmieString = files[i].substring(pointPosition+1);
  			 		if(mmieString=="csv"){
  			 			files[i] = path+"\/"+files[i];
  			 			//确保.csv是文件，不是目录
  			 			var stats = fs.statSync(files[i]);
  			 			if(stats.isFile() && fileLock[files[i]]===undefined){
  			 				//设置文件正在被处理
  			 				fileLock[files[i]] = true;
  			 				//直接下至数据task()	
  			 				task(files[i],path);
  			 			}else{
  			 					//暂时什么也不做
  			 				}
  			 		}else{
  			 			//暂时什么也不做
  			 		}
        		}
        	}
        }
    });
}

var isFirstCreateTable = true;
function toSqlserver (configData,data,stationName)
{
	sql.open(configure.sqlserver, function( err, sqlCon) {
		if(err){
			console.log(err);
		}else{
				var confLine = configData.length;
				//获取配置文件所有的字段
				var confKeys = Object.keys(configData[0]);
				for(var i=0;i<confLine;i++){
					if(stationName==configData[i]["PLC_Station_Name"]){
						//获取数据的行数
						var len = data.length;
						//获取所有的字段
						var keys = Object.keys(data[0]);
						for(var k=0;k<keys.length;k++){
							for(var s=0;s<len;s++){
								if(configData[i]["PLC_TagName"]===keys[k]){	
									var tableName = configData[i]["table_Name"];
									(function(idx,idx1,idx2){
										sqlCon.queryRaw("if OBJECT_ID ('"+tableName+"') is null create table ["+tableName+"](Time datetime,tagName nvarchar(255),PV float);",function(err,res){
										if(err){
											console.log("jugeErr:",err);
										}else{
											var sqlStr = "INSERT INTO "+tableName+ "(Time,tagName,PV) VALUES ";
											var time = new Date(data[idx]["Time"]);
											var str = formatDate(time);
											var strTime = "'"+str+"'";
											var sql_TagName = "'"+configData[idx1]["sql_TagName"]+"'";
											var val = strTime+","+sql_TagName+","+data[idx][keys[idx2]];
											sqlStr += "("+val+")";
											//sqlStr = sqlStr + ";";
											//console.log(sqlStr);
											sqlCon.queryRaw(sqlStr, function(err) {
												if(err){
													console.log("table:",err);
												}else{
													//转储成功一条数据
													//console.log("succeed");
												}	
							                });
										}
									});
									})(s,i,k);
								}
							}
						}
					}
				}
		}
	});
}
//周期检查文件
setInterval(dealWithFile,configure.interVal,configure.workPath);
setInterval(fun,1000);

