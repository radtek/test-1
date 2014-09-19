var csv = require("csv");
var fs = require('fs');
var addon = require('pSpace');
var configure = require('configure');
var sql = require('msnodesql');
var iconv = require('iconv-lite');
var logger = require('log').logger;
var async = require("async");
var ps  = new addon();
var addon= require('guard');
var guard= new addon.Guard();
guard.beginStart();


var configPath = configure.configPath;
var ps_con;
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
var fileLock = [];
var dirPath = configure.workPath;
function handleError () {
   var conn = ps.open(configure.pSpace);
   if(conn instanceof ps.Err){
    logger.error("connect error:",conn.errString);
    handleError();
   }else{
    ps_con=conn;
   }
}

handleError();


//对给定的文件夹进行处理
function dealWithFile(path)
{
    //保存csv文件名
    fs.exists(path,function(exists){
        if(!exists){
            logger.error("指定的工作目录不存在，请重新确定指定路径是否正确!");
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
                        //确保不是配置文件
                        if(files[i]===configPath){
                            continue;
                        }
                        //确保.csv是文件，不是目录
                        var stats = fs.statSync(files[i]);
                        if(stats.isFile() && fileLock[files[i]]===undefined){
                            //设置文件正在被处理
                            fileLock[files[i]] = true;
                            readFile(files[i],configPath);
                           
                        }else{
                                continue;
                            }
                    }else{
                        continue;
                    }
                }
            }
        }
    });
}


function readFile(filePath,configPath){
    //var log_readFile = log4js.getLogger('readFile');
    //log_readFile.setLevel('info');
    //计算文件第三行的起始位置
    var fileData = fs.readFileSync(filePath, 'ASCII');
    var first = fileData.indexOf('\n');
    var two = fileData.indexOf('\n',first+1);
    //获取站名
    //获取站名
    var pp1 = filePath.lastIndexOf("\/");
    var station = filePath.substring(pp1+1);
    var pp = station.indexOf("_"); 
    var stationName = station.substring(0,pp);
    async.waterfall([
    function(cb) { 
        fs.exists(configPath,function(exists){
                cb(null,exists);
            });
     },
    function(exists, cb) { 
        if(!exists){
                logger.error("配置文件不存在.");
                cb("configure file not exit",null);
            }else{
                (function(callback){
                    async.auto({
                        readConfigData:function(cb){
                            csv().from.path(configPath,{columns:true,encoding: 'binary'})
                            .transform(function(configData,index){return configData;})
                            .to.array(function(configData){ cb(null,configData);})
                            .on('close', function(count){logger.info(" 配置文件关闭!");})
                            .on('error', function(error){cb(error.message,null);logger.error("配置文件读取错误:",error.message);}); 
                        },
                        readFileData:function(cb){
                            csv().from.path(filePath,{columns:true,start:two,encoding: 'ASCII'})
                            .transform(function(fileData,index){return fileData;})
                            .to.array(function(fileData){cb(null,fileData);})
                            .on('close', function(count){/*logger.info(" csv文件关闭!");*/})
                            .on('error', function(error){cb(error.message,null);logger.error("csv文件读取错误:",error.message);})
                        }
                    },function(err,result){
                        if(err){
                            logger.error("读文件错误.");
                            callback("read file error!",null);
                        }else{
                            confData=result["readConfigData"];
                            csvData = result["readFileData"];
                            callback(null,true);
                        }
                    });
                })(cb);
            }
    }
    
    ], function (err, result) {
        if(err){
            logger.error("检查配置文件错误");
            return;
            //readFile(filePath,configPath);
        }else{
           logger.info("处理文件成功，开始处理数据."); 
           dealWithData(csvData,confData,filePath);
        }
    });
}


function dealWithData(csvData,confData,filePath){
    if(ps.isConnected()){
        logger.info("连接pSpace成功,开始数据处理!");
         //获取站名
        var pp1 = filePath.lastIndexOf("\/");
        var station = filePath.substring(pp1+1);
        var pp = station.indexOf("_"); 
        var stationName = station.substring(0,pp);
            //获取配置文件的行数
        var confLine = confData.length;
        //获取配置文件所有的字段
        var confKeys = Object.keys(confData[0]);
        //获取数据的行数
        var len = csvData.length;
        //获取csv文件所有的字段
        var keys = Object.keys(csvData[0]);
        //filePath = filePath.replace(/[\\]/g,"/");
        //文件数据
        var fileData = fs.readFileSync(filePath, 'ASCII');
        //如果配置文件名和csv文件名相同，不处理此文件，跳出这个函数return
        if(configPath==filePath){
            return;
        }
        async.auto({
            insertHisData:function(cb){
                for(var i=0;i<confLine;i++){
                    console.log(stationName);
                    console.log(confData[i]["PLC_Station_Name"]);
                    if(stationName==confData[i]["PLC_Station_Name"]){
                        for(var k=0;k<keys.length;k++){
                            if(confData[i]["PLC_TagName"]===keys[k]){
                                //获取长名
                                var tagLongName = confData[i]["pSpace_LongTagname"];
                                //长名格式转化
                                tagLongName = tagLongName.replace(/[\\]/g,"/")+".his.insertReplace";
                                var buf = new Buffer(tagLongName, 'binary');
                                var strName = iconv.decode(buf, 'gbk');
                                //将数据一条一条的插入
                                for(var s=0;s<len;s++){
                                    var hisData = {
                                        "value":Number(csvData[s][keys[k]]),
                                        "time":new Date(csvData[s]["Time"]),
                                        "quality":"good"
                                    };
                                    var res = ps_con.write(strName,hisData);
                                    if(res instanceof ps.Err){
                                        //console.log(res.errString);
                                        logger.error(res.errString);
                                        continue;
                                    }else{
                                        //
                                    }
                                }

                            }else{
                                continue;
                            }
                        }

                    }else{
                        logger.warn("站名和PLC_Station_Name不匹配.");
                        cb("站名和PLC_Station_Name不匹配",null);
                    }
                }
                cb(null,1);
            },
            toSqlServer:["insertHisData",function(cb){
                if(configure.isSaveToSql){
                    if(confData[0].hasOwnProperty("table_Name") && confData[0].hasOwnProperty("sql_TagName")){
                        toSqlserver(confData,csvData,stationName,confLine,confKeys,len,keys,cb);
                    }else{
                       logger.info("配置文件转储相关信息错误.");
                       cb("配置文件转储相关信息错误",null);
                    } 
                }else{
                    logger.info("不需要转储.");
                    cb(null,1);
                }
            }],
            backup:["toSqlServer",function(cb){
                logger.info("转储完毕，开始备份文件.");
                if (filePath.indexOf(configPath)<0) {
                    logger.info("配置文件名不属于csv文件名的字串，开始备份.");
                    var p = filePath.lastIndexOf('\/');
                    var fileName = filePath.substring(p);
                    async.waterfall([
                        function(cb1) { 
                            fs.exists(dirPath+"/backup",function(exists){
                                    cb1(null,exists);
                                });
                         },
                        function(exists, cb2) { 
                            if(!exists){
                                    logger.info("backup文件名不存在，将创建.");
                                    async.auto({
                                        createBackupFile:function(cb21){
                                            fs.mkdir(dirPath+"\/backup",function(err){
                                                if(err){
                                                    cb21("创建backup错误",null);
                                                }else{
                                                    cb21(null,1);
                                                }
                                            });
                                        },
                                        backupFIle:function(cb22){
                                            log.info("backup创建完毕，将备份.");
                                            //console.log(csvData);
                                            fs.writeFile(dirPath+"\/backup"+"\/"+fileName,fileData, function(err){ 
                                                if(err){
                                                    cb22(err,null);
                                                }else{
                                                    logger.info("文件备份完毕.")
                                                    cb22(null,1);
                                                }
                                            });
                                        }
                                    },function(err,result){
                                        if(err){
                                            cb2(err,null)
                                        }else{
                                            cb2(null,1);
                                        }
                                    }); 
                                }else{
                                    logger.info("backup已经存在将备份.");
                                    fs.writeFile(dirPath+"\/backup"+"\/"+fileName,fileData, function(err){ 
                                        if(err){
                                            cb2(err,null);
                                        }else{
                                            logger.info("文件备份完毕.");
                                            cb2(null,1);
                                        }
                                    });
                                }
                        }
                        ], function (err, result) {
                            if(err){
                                logger.error("备份文件错误。");
                                 cb("备份文件错误",null);
                            }else{
                               logger.info("备份文件成功."); 
                               cb(null,1);
                            }
                        });

                }else{
                    logger.warn("配置文件名包含在文件名中，跳过此文件.");
                    //跳过处理
                    cb(null,1);
                }
            }],
            deleteFile:["backup",function(cb){
                fs.unlink(filePath, function(err) {
                    if(err){
                        logger.error("删除文件失败.");
                        cb("删除文件失败.",null);
                    }else{
                        //删除文件状态
                         logger.info("删除文件状态.");
                        delete fileLock[filePath];
                        logger.info("删除文件成功.");
                        cb(null,1);
                    }
                });
            }]
        },function(err,result){
            if(err){
                 logger.error("数据处理失败，重新处理.");
                 //退出程序，重新循环执行
                 return;
                 //dealWithData(csvData,confData,filePath);
            }else{
               
                logger.info("数据处理成功.");
            }
        });
    }else{
        logger.error("pSpace已经断开连接!");
        handleError();
        //此处注意，要清楚断线重连之后干什么
        dealWithData(csvData,confData,filePath)
    }
}
function toSqlserver (configData,data,stationName,confLine,confKeys,len,keys,cb)
{
    sql.open(configure.sqlserver, function( err, sqlCon) {
        if(err){
            console.log("fadfadf");
            logger.error("连接到sqlserver失败，正在进行重连!");
            setTimeout(toSqlserver,1000,configData,data,stationName,confLine,confKeys,len,keys,cb);
        }else{
               logger.info("连接到sqlserver成功，正在转储");
                for(var i=0;i<confLine;i++){
                    if(stationName==configData[i]["PLC_Station_Name"]){
                        for(var k=0;k<keys.length;k++){
                            for(var s=0;s<len;s++){
                                if(configData[i]["PLC_TagName"]===keys[k]){ 
                                    var tableName = configData[i]["table_Name"];
                                    (function(idx,idx1,idx2){
                                        sqlCon.queryRaw("if OBJECT_ID ('"+tableName+"') is null create table ["+tableName+"](Time datetime,tagName nvarchar(255),PV float);",function(err,res){
                                        if(err){
                                             logger.error("judge error:",err);
                                        }else{
                                            var sqlStr = "INSERT INTO "+tableName+ "(Time,tagName,PV) VALUES ";
                                            var time = new Date(data[idx]["Time"]);
                                            var str = formatDate(time);
                                            var strTime = "'"+str+"'";
                                            var sql_TagName = "'"+configData[idx1]["sql_TagName"]+"'";
                                            var val = strTime+","+sql_TagName+","+data[idx][keys[idx2]];
                                            sqlStr += "("+val+")";
                                            sqlStr = sqlStr + ";";
                                            //console.log(sqlStr);
                                            sqlCon.queryRaw(sqlStr, function(err) {
                                                if(err){
                                                    logger.error("exec sql error:",err);
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
                cb(null,1);
        }
    });
}

setInterval(dealWithFile,configure.interVal,configure.workPath);


guard.startError();

guard.onStop(function(err,result){
    if(err){
        //console.log(err);
    }else{
        guard.setStop();
        process.exit(1);
    }
});
