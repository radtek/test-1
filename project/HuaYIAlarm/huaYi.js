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
    console.log("check fail");
    handleError(sqlCon);
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
        if(confData[j].TagName==datakeys[k]){
          data[datakeys[k]]["error"].push(errName);
          data[datakeys[k]]["bit"].push(confData[j].Address);
        }
      }
    }
    done();
  });
}


function sub(sqlCon,done){
  if(ps.isConnected()){
    ps_con.sub(datakeys,function(err,subid,curVal,value){
      if(err){
        console.log("sub error");
        logger.error("订阅失败：",err);
        done("订阅失败",null);
      }else{
        if(curVal){
          console.log("current.");
          //将初始状态均写入历史和实时
          sqlExec.initTable(sqlCon,curVal,logger,data,datakeys,done);
        }else if(value){
          console.log("Hello new");
          //更新实时，插入历史
          sqlExec.update(sqlCon,value,logger,data,datakeys,done);
        }
      }
    });
  }else{
    logger.error("pSpace数据库连接已经断开，将重连.");
    done(null,"pSpace数据库连接已经断开");
  }
}

function work(sqlCon){
  console.log("work");
  async.series([
    function(done){
      //读取配置文件
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
            var createSql = "create table ["+configure.real_table+"](风场名称 nvarchar(255),风机名称 nvarchar(255),时间 datetime,故障描述 nvarchar(255) primary key(风场名称,风机名称,时间,故障描述));";
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
                  var createSql = "create table ["+configure.real_table+"](风场名称 nvarchar(255),风机名称 nvarchar(255),时间 datetime,故障描述 nvarchar(255) primary key(风场名称,风机名称,时间,故障描述));";
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
      var createSql = "if OBJECT_ID ('"+configure.his_table+"') is null create table ["+configure.his_table+"](风场名称 nvarchar(255),风机名称 nvarchar(255),时间 datetime,报警状态 nvarchar(20),故障描述 nvarchar(255));";
      sqlExec.execQuerySql(sqlCon,createSql,logger,done);
      logger.trace("历史表创建成功");
    },
    function(done){
      //数据库已经连接就订阅
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
        logger.info("这个点操作结束.");
        return;
      }
    });
}
// 连接sqlserver进行工作
 function main(){
  logger.trace("开始连接sqlserver");
  sql.open(configure.sqlserver, function( err, sqlCon) {
    if(err){
      console.log("sqlcon err");
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
