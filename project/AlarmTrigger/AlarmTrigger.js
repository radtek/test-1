var addon = require('pSpace');
var configure = require('configure');
var async = require("async");
var iconv = require('iconv-lite');
var mysql = require('mysql');
var sqlExec = require('sql');
var read = require('readCsv');
var ps  = new addon();
var guard= require('guard');
var logger = require('log').initLog(__dirname);
guard.start();
//pSpace连接对象
var ps_con = new Object();
//mysql连接对象

var data = {};

//读取配置文件并填充字典
function readCsv(done){
  logger.info("开始加载配置文件。");
  read.readFile(configure.configurePath,function(err,confData){
    for(var i in confData){
      var longName = new Buffer(confData[i].tagname, 'binary');
      var strName = iconv.decode(longName, 'gbk');
      var typeBuffer = new Buffer(confData[i].type, 'binary');
      var type = iconv.decode(typeBuffer, 'gbk');
      var exists = ps_con.isExists(strName);
      if(exists && !exists.hasOwnProperty("errString")){
        data[strName] = {};
        data[strName]["type"] = type;
        data[strName]["last"] = null;
      }else{
        if(!exists || exists.hasOwnProperty("errString"))
          console.log("测点不存在或者测点长名不合法：",strName);
          logger.warn("测点不存在或者测点长名不合法：",strName);
      }
    }
    logger.info("配置文件加载完毕.");
    console.log("配置文件加载完毕!");
    done();
  });
}
var mysql_con = new Object();

var  propData = {};
function handleError () {
  var conn = ps.open(configure.pSpace);
  if(conn instanceof ps.Err){
    logger.error("pSpace连接断开.");
    return;
  }else{
    ps_con = conn;
    conn.del(0);
	//pSpace连接之后干什么
    work();
  }
}

function checkPSConnect(){
  if(!ps.isConnected()){
    //console.log("check fail");
    handleError();
    return;
  }else{
    return;
  }
}



//属性缓存管理
function getProps(name,callback){
//先从缓存里面找属性，找不到了pSpace中查询
	if(propData.hasOwnProperty(tagName)){
		//有属性返回
		callback(null,propData[tagName]);
	}
	//没有属性，从pSpace中查询返回
	//先判断测点是否是开关量
	var prop = ["TagType","OnMessage","OffMessage","ValueAlarmClass"];
	//如果是开关量,保存属性，否则返回错误
	var tagName = name+'.props';
	var res = ps_con.read(tagName,prop);
	if(res instanceof ps.Err){
		callback(res.errString,null);
	}else{
		if(res.TagType===1){
			var tmp = new Object();
			tmp.on = res.OnMessage;
			tmp.off = res.OffMessage;
			tmp.level = res.ValueAlarmClass;
			propData[name] = tmp;
			callback(null,propData[name]);
		}else{
			callback('点类型非开关量');
		}
	}

}
//格式化获取到的值
function formatValue (value,done) {
  if(value){
    for(var i in value){
      getProps(value[i].name,function(err,prop){
        if(err){
         logger.error("格式化获取到的值失败:",err);
          done(err);
        }else{
          if(value[i].value.value===true){
            value[i].value.value=1;
          }else{
            value[i].value.value=0;
          }
          value[i]['props'] = prop;
        }
      });
    }
    done && done(null,value);
  }
}
//连接mysql
 function mysqlConnect(done){
  mysql_con = mysql.createConnection(configure.mysql);
    mysql_con.connect(function(err) {
    if (err) {
      logger.error('error connecting: ' + err.stack);
      done(err);
    }else{
     logger.info("mysql连接完毕。"); 
     done();
    }
  });
 }
function sub(done){
  if(ps.isConnected()){
    ps_con.sub(Object.keys(data),function(err,subid,curVal,value){
      if(err){
        //console.log("sub error");   
        done("订阅失败",null);
      }else{
        if(curVal){
          formatValue (curVal,function(err,val){
            if(err){
              done(err);
            }else{
              sqlExec.init(mysql_con,data,val,logger,done);
            }
          });
        }else if(value){
    			formatValue (value,function(err,val){
                if(err){
                  done(err);
                }else{
                  sqlExec.update(mysql_con,data,val,logger,done);
                }
              });
        }
      }
    });
  }else{
    done(null,"pSpace数据库连接已经断开");
  }
}

function work(sqlCon){
  async.series([
    function(done){
      //读取配置文件
      readCsv(done);
    },
    function(done){
		//创建mysq连接
      mysqlConnect(done);
    },
    function(done){
      var create = "create table IF NOT EXISTS "+configure.tableName+"(tagname varchar(255) not null,value float not null,time datetime not null,type char(30) not null,level int not null,describle varchar(255) not null, PRIMARY KEY(tagname));";
      sqlExec.execSql(mysql_con,create,logger,done);
	   
    },
    function(done){
      //开始订阅
      sub(done);
    }
    ],function(err,result){
      if(err){
	  //如果整个流程中某一步失败，需要做什么
		    logger.warn("失败，将重新启动程序.");
         //重启之前关闭setInterval
         if(intervalID!=undefined){
   
          clearInterval(intervalID);
         }
         //重启之前关闭关闭pSpace连接
         if(ps.isConnected()){
          //删除所有订阅
          ps_con.del(0);
          ps_con.close();
         }
         //1s后重启
         setTimeout(main,2000);
         return;
      }else{
	  //操作成功干什么

        return;
      }
    });
}

 function main(){
  //连接数据库
  checkPSConnect();
  //开始周期检测
  intervalID = setInterval(checkPSConnect,1000);
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
