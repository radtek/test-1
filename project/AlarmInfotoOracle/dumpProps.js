var addon = require("pSpace"),
   ps  = new addon(),
   param = require('./common/params'),
   dump = require('./common/dump'),
   config = require('./db/configure'),
   guard= require('guard'),
   connectOptions = require('./db/connection');
guard.start();

//pSpace连接对象
var ps_con = new Object();
function handleError () {
  var conn = ps.open(connectOptions.pSpace);
  if(conn instanceof ps.Err){
    return;
  }else{
    ps_con = conn;
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
handleError ();
var params={};
var setIntervalID ;

function dump2oracle(){
  dump.dump(ps_con,params,function(err,result){
          if(err){
            console.log("转储失败,重新转储:",err);
            setTimeout(dump2oracle,5000);
          }else{
            console.log("转储成功:",new Date());
          }
        });
}

function main() {
	if(ps.isConnected()){
    //预先加载配置文件
    param.readParams(ps_con,function(err,param){
      if(err){
        console.log("配置文件加载失败，重新加载....");
        setTimeout(main,3000);
      }else{
        params=param;
       console.log("配置文件加载完毕");
       dump2oracle();
       setInterval(dump2oracle,config.interVal);
      }
    }); 
  }else{
    console.log("pSpace 未连接");
  }
	
}
//心跳检测pSpace 连接状况
setInterval(checkPSConnect,1000);
main();
guard.startError();
guard.onStop(function(err,result){
    if(err){
    }else{
      logger.info("收到停止信号");
        process.exit(1);
    }
});
