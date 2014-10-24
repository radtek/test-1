var addon = require('pSpace');
var configure = require('configure');
var sql = require('msnodesql');
var iconv = require('iconv-lite');
var async = require("async");
var ps  = new addon();
var guard= require('guard');
guard.start();

var ps_con;
function Appendzero(obj)
{
     if(obj<10) return "0" +""+ obj;
     else return obj;     
}
var formatDate = function(now){
	   var  year=now.getFullYear();     
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
      fun(conn);
   }
}
handleError();
longNameList = ["/kaiGuan1","/kaiGuan2","/kaiGuan3"];
function fun(ps_con){
  sql.open(configure.sqlserver, function( err, sqlCon) {
    if(err){
      console.log(err);
    }else{
      ps_con.sub(longNameList,function(err,subid,curVal,value){
        if(err){
          console.log(err);
        }else if(curVal){
          console.log(curVal[0].value.value);
          //将状态全部写入历史
          curhIS(curVal,sqlCon);
        }else if(value){
          work(value,sqlCon);
        }
      }); 
    }
  });
}
 
function work(value,sqlCon){
  for(var len in value){
    value[len].name = value[len].name.substring(value[len].name.lastIndexOf('\/')+1);
    var name = "'"+value[len].name+"'";
    if(value[len].value.value==true){  
      (function(name,value){
        async.waterfall([
        function(done){
          sqlCon.queryRaw("select count(1) from realAlarm where tagName ="+name+" and alarmValue = 1;",function(err,cnt){
                if(err){
                  console.log(err);
                  done(err);
                }else{
                done(null,cnt.rows[0][0]);
                }
            });
        },
        function(cnt,done){
          console.log("cnt::",cnt);
          if(cnt==0){
            console.log("cnt为0");
            var str = formatDate(value[len].value.time);
            var strTime = "'"+str+"'";
            sqlCon.queryRaw("INSERT INTO realAlarm(tagName,startTime,alarmValue) VALUES("+name+","+strTime+",1)",function(err){
              if(err){
                console.log(err);
              }else{
                sqlCon.queryRaw("INSERT INTO hisAlarm(tagName,Time,alarmState) VALUES("+name+","+strTime+",1)",function(err){
                  if(err){
                    console.log(err);
                  }else{
                    done();
                  }
                });
  
              }
            });

          }
        }
      ]);
      })(name,value);
      //保存在实时表和历史表,产生报警
      
    }else{
      //实时表中是否有这条记录，有删除，报警恢复
      var name = "'"+value[len].name+"'";
      console.log("实时值：",value);
      (function(name,value){
          async.waterfall([
        function(done){
          sqlCon.queryRaw("select count(1) from realAlarm where tagName ="+name+" and alarmValue = 1;",function(err,cnt){
                if(err){
                  done(err);
                }else{
                done(null,cnt.rows[0][0]);
                }
            });
        },
        function(cnt,done){
          if(cnt==1){
            console.log("删除记录");
            sqlCon.queryRaw("DELETE FROM realAlarm where tagName ="+name+"",function(err){
              if(err){
                console.log(err);
              }else{
                done();
              }
            });
          }
        },
        function(done){
          //历史表中更新报警状态，有记录更新，无的话添加
          //async.waterfall([
            //function(done){
              //sqlCon.queryRaw("select count(1) from hisAlarm where tagName ="+value[len].name+" and alarmValue = 0;",function(err,cnt){
               //     if(err){
              //        done(err);
              //      }else{
                    //done(null,cnt.rows[0][0]);
               //     }
              //  });
           // },
           // function(cnt,done){
              //if(cnt==0){
                var str = formatDate(value[len].value.time);
                var strTime = "'"+str+"'";
                sqlCon.queryRaw("INSERT INTO hisAlarm(tagName,Time,alarmState) VALUES("+name+","+strTime+",0)",function(err){
                  if(err){
                    console.log(err);
                  }else{
                    done();
                  }
                });
              //}else{
               // sqlCon.queryRaw("update hisAlarm ",function(err){
                //  if(err){
                 //   console.log(err);
                 // }else{
                  //  done();
                  //}
                //});
              //}
          //  }
         // ]);
        }
      ]);
      
      })(name,value);
    }
  }

}
function curhIS(value,sqlCon){
  for(var len in value){
    value[len].name = value[len].name.substring(value[len].name.lastIndexOf('\/')+1);
    if(value[len].value.value==false){//恢复报警
      var name = "'"+value[len].name+"'";
      (function(name,value){
        async.waterfall([
        function(done){
          sqlCon.queryRaw("select count(1) from hisAlarm where tagName ="+name+" and alarmState = 0;",function(err,cnt){
                if(err){
                  done(err);
                }else{
                done(null,cnt.rows[0][0]);
                }
            });
        },
        function(cnt,done){
          if(cnt==0){
            var str = formatDate(value[len].value.time);
            var strTime = "'"+str+"'";
            sqlCon.queryRaw("INSERT INTO hisAlarm(tagName,Time,alarmState) VALUES("+name+","+strTime+",0)",function(err){
              if(err){
                console.log(err);
              }else{
                done();
              }
            });
          }
        }
      ]);
      })(name,value);
      
    }else{//产生报警
      var name = "'"+value[len].name+"'";
      (function(name,value){
        async.waterfall([
        function(done){
          sqlCon.queryRaw("select count(1) from hisAlarm where tagName ="+name+" and alarmState = 1;",function(err,cnt){
                if(err){
                   console.log(err);
                  done(err);
                }else{

                  done(null,cnt.rows[0][0]);
                }
            });
        },
        function(cnt,done){
        
          if(cnt==0){
            var str = formatDate(value[len].value.time);
            var strTime = "'"+str+"'";
            sqlCon.queryRaw("INSERT INTO hisAlarm(tagName,Time,alarmState) VALUES("+name+","+strTime+",1)",function(err){
              if(err){
                console.log(err);
              }else{
                done();
              }
            });
          }
        }
      ]);

      })(name,value);
      
    }
  }
}
