var addon = require('pSpace');
var bits = require('BitArray');
var configure = require('configure');
var sql = require('msnodesql');
var iconv = require('iconv-lite');
var async = require("async");
var logger = require('log').initLog(__dirname);;
var sqlExec = require('sql');
var ps  = new addon();
var guard= require('guard');
guard.start();

var intervalID = undefined;
var isFirstExec = true;
//
var ps_con = new Object();
var data = {};
var datakeys = [];
function handleError (sqlCon) {
  var conn = ps.open(configure.pSpace);
  if(conn instanceof ps.Err){
    logger.error("connect pSpace error:",conn.errString);
    return;
  }else{
    ps_con = conn;
    conn.del(0);
    work(sqlCon);
  }
}

function checkPSConnect(sqlCon){
  if(!ps.isConnected()){
    //console.log("check fail");
    handleError(sqlCon);
    return;
  }else{
    return;
  }
}

//读取配置文件并填充字典
function readCsv(done){
  bits.readFile(configure.configurePath,logger,function(err,confData){
    for(var i in confData){
      if(confData[i].Address==0){
        var longName = new Buffer(confData[i].TagName, 'binary');
        var strName = iconv.decode(longName, 'gbk');
        data[strName]={}; 
        data[strName]["error"] = [];
        data[strName]["bit"] = [];
      }
    }
    datakeys = Object.keys(data);
    for(var j in confData){
      var errDescName = new Buffer(confData[j].Error_Desc, 'binary');
      var errName = iconv.decode(errDescName, 'gbk');
      for(var k in datakeys){
        var confTagName_buffer = new Buffer(confData[j].TagName,"binary"); 
        var confTagName = iconv.decode(confTagName_buffer, 'gbk');
        if(confTagName==datakeys[k]){
          data[datakeys[k]]["error"].push(errName);
          data[datakeys[k]]["bit"].push(confData[j].Address);
        }
      }
    }
    done();
  });
}
//将数据拆分成32位，保存在数组中
function int2bits(number){
  var arr = bits.parse(number);
  var bitsArr = [];
  for(var i=0;i<32;i++){
    bitsArr.push(configure.normal);
  }
  if(arr.length>32){
    logger.error("数据非法，无法拆分");
    return false;
  }
  if(arr.length<32){
    var j = 31;
    for(var i=arr.length-1;i>=0;i--){
      if(arr[i]==1){
        bitsArr[j]=1;
      }
      --j;
    }
  }
  delete arr;
  return bitsArr.reverse();
}

function sub(sqlCon,done){
  if(ps.isConnected()){
    ps_con.sub(datakeys,function(err,subid,curVal,value){
      if(err){
        //console.log("sub error");
        logger.error("订阅失败：",err);
        done("订阅失败",null);
      }else{
        if(curVal){
         // console.log("current.");
          //将初始状态均写入历史和实时
          for(var i in curVal){
            for(var j in datakeys){
                if(curVal[i].name==datakeys[j]){
                    var valueBits = int2bits(curVal[i].value.value);
                    if(valueBits){
                        logger.info("初始化实时表,历史表");
                        sqlExec.initTable(sqlCon,data,curVal[i].name,valueBits,curVal[i].value.time,logger,done);
                        //return;/////
                    }
                }
            }
        }
          //sqlExec.initTable(sqlCon,curVal,logger,data,datakeys,done);
        }else if(value){
          //更新实时，插入历史
          for(var i in value){
            for(var j in datakeys){
              if(value[i].name==datakeys[j]){
                var valueBits = int2bits(value[i].value.value);
                if(valueBits)
                  sqlExec.update(sqlCon,data,value[i].name,valueBits,value[i].value.time,logger,done); 
              }
            }
          }
        }
      }
    });
  }else{
    logger.error("pSpace数据库连接已经断开，将重连.");
    done(null,"pSpace数据库连接已经断开");
  }
}

function work(sqlCon){
  logger.trace("测试是否运行过程中重启！！！！！！！！！！！！！！！！！！！！！！！！！！！！");
  async.series([
    function(done){
      //读取配置文件
      delete data;
      delete datakeys;
      readCsv(done);
    },
    function(done){
      //判断实时表是否存在，不存在就创建,存在删除重建
      async.waterfall([
        function(done){
          var judgeStr = "select count(1) from sys.objects where name ='"+configure.real_table+"';"
          sqlExec.judgeExecSql(sqlCon,judgeStr,logger,done);   
        },
        function(cnt,done) {
          if(cnt==0){
            //表不存在
            var createSql = "create table ["+configure.real_table+"](风场名称 nvarchar(255),风机名称 nvarchar(255),时间 datetime,故障描述 nvarchar(255) primary key(风场名称,风机名称,故障描述));";
            sqlExec.execQuerySql(sqlCon,createSql,logger,done);
            logger.trace("实时表创建成功"); 
          }else{
            //表存在，删除创建(只有程序第一次运行的时候删除重建)
            if(isFirstExec){
              async.series([
                function(done){
                  var dropStr = "drop table "+configure.real_table+";";
                  sqlExec.execQuerySql(sqlCon,dropStr,logger,done);
                  logger.trace("实时表删除成功.");
                },
                function(done){
                  var createSql = "create table ["+configure.real_table+"](风场名称 nvarchar(255),风机名称 nvarchar(255),时间 datetime,故障描述 nvarchar(255) primary key(风场名称,风机名称,故障描述));";
                  sqlExec.execQuerySql(sqlCon,createSql,logger,done);
                  isFirstExec = false;
                  logger.trace("实时表创建成功"); 
                }
                ],function(err,result){
                  if(err){
                    done(err);
                  }else{
                    done();
                  }
              });
            }else{
              logger.trace("实时表创建成功");
              done();
            }
          }
        }
        ],function(err,result){
          if(err){
            done(err);
          }else{
            done();
          }

        });
    },
    function(done){
      //判断历史表是否存在，不存在就创建，存在不做处理
      var createSql = "if OBJECT_ID ('"+configure.his_table+"') is null create table ["+configure.his_table+"](风场名称 nvarchar(255),风机名称 nvarchar(255),时间 datetime,报警状态 nvarchar(20),故障描述 nvarchar(255) primary key(风场名称,风机名称,时间,报警状态,故障描述));";
      sqlExec.execQuerySql(sqlCon,createSql,logger,done);
      logger.trace("历史表创建成功");
    },
    function(done){
      //数据库已经连接就订阅
      //sqlCon.close();
      sub(sqlCon,done);
    }
    ],function(err,result){
      if(err){
         logger.error("启动任务失败，将重新执行");
         //重启之前关闭setInterval
         if(intervalID!=undefined){
          logger.trace("关闭setInterval");
          clearInterval(intervalID);
         }
         //重启之前关闭关闭pSpace连接
         if(ps.isConnected()){
          //删除所有订阅
          ps_con.del(0);
          logger.trace("关闭pSpace");
          ps_con.close();
         }
         //1s后重启
         setTimeout(main,2000);
         return;
      }else{
        //logger.trace("这个点操作结束.");
        return;
      }
    });
}
// 连接sqlserver进行工作
 function main(){
  logger.trace("开始连接sqlserver");
  sql.open(configure.sqlserver, function( err, sqlCon) {
    if(err){
      //console.log("sqlcon err");
      logger.error("sqlserver连接失败，将重连：",err);
      setTimeout(main,1000);
      return;
    }else{
      //连接数据库
      checkPSConnect(sqlCon);
      //检测
      intervalID = setInterval(checkPSConnect,1000,sqlCon);
      //work(sqlCon);  
    }
  });
 }
main();
guard.startError();
guard.onStop(function(err,result){
    if(err){
        //console.log(err);
    }else{
        //guard.setStop();
        process.exit(1);
    }
});
