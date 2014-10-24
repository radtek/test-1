var bits = require('BitArray');
var assert = require( 'assert' );
var async = require('async');
var util = require( 'util' );
var configure = require('configure');
var writeDebugComments = false;
var dataComparisonFailed = "Results do not match expected values";
//   non-optional message 
function errorComments(Message) {
    console.log(Message);
};

//   optional debugging information
function debugComments(Message) {
    if (writeDebugComments == true)
        errorComments(Message);
};

function dataComparisonFailedMessage(expected, actual, done) {
    errorComments("\nExpected: \n" + util.inspect(expected));
    errorComments("\nReceived: \n" + util.inspect(actual));
    done(new Error("\nxxxTEST FAILED"));
};
function Appendzero(obj)
{
     if(obj<10) return "0" +""+ obj;
     else return obj;     
}
function Append3zero(obj)
{
     if(obj<100) return "0" +""+ obj;
     else return obj;     
}
var formatDate = function(now){
    var   year=now.getFullYear();     
    var   month=now.getMonth()+1;     
    var   date=now.getDate();     
    var   hour=now.getHours();     
    var   minute=now.getMinutes();    
    var   second=now.getSeconds(); 
    var   milSecond = now.getMilliseconds(); 
    //console.log("milsecond:",milSecond);    
    return   year+"-"+Appendzero(month)+"-"+Appendzero(date)+" "+Appendzero(hour)+":"+Appendzero(minute)+":"+Appendzero(second)+"."+Append3zero(milSecond);     
}
var formatDate = function(now){
    var   year=now.getFullYear();     
    var   month=now.getMonth()+1;     
    var   date=now.getDate();     
    var   hour=now.getHours();     
    var   minute=now.getMinutes();    
    var   second=now.getSeconds(); 
    var   milSecond = now.getMilliseconds(); 
    return   year+"-"+Appendzero(month)+"-"+Appendzero(date)+" "+Appendzero(hour)+":"+Appendzero(minute)+":"+Appendzero(second);     
}
function judgeExecSql(Connection,sql,logger,done){
    Connection.queryRaw(sql,function(err,cnt){
     if(err){
        logger.error("执行判断sql语句失败：",err);
        done(err);
      }else{
        //logger.trace("判断是否存在实时表");
        done(null,cnt.rows[0][0]);
      }
    });
}
function judge2ExecSql(Connection,sql,logger,done){
    async.series([
        function(done){
            Connection.queryRaw(sql[0],function(err,cnt){
             if(err){
                logger.error("执行判断sql语句失败：",err);
                done(err);
              }else{
                //logger.trace("判断是否存在实时表");
                done(null,cnt.rows[0][0]);
              }
            });
        },
        function(done){
            Connection.queryRaw(sql[1],function(err,cnt){
             if(err){
                logger.error("执行判断sql语句失败：",err);
                done(err);
              }else{
                //logger.trace("判断是否存在实时表");
                done(null,cnt.rows[0][0]);
              }
            });
        }
        ],function(err,result){
            if(err){
                logger.error("对数组中的sql语句执行有失败的数据:",err);
                done(err);
            }else{
                done(null,result);
            }
    });
}
function execQuerySql(Connection,sql,logger,done){
    Connection.queryRaw(sql,function(err){
        if(err){
            logger.error("sql执行失败:",err);
            done(err);
        }else{
            done();
        }
    });
}
function compoundQueryTSQL(Connection, tsql,len,ExpectedData,done) {
    var NewExpectedData = ExpectedData;
    var called = 0;
    Connection.queryRaw(tsql, [], function (e, r, more) {
        if(e){
            //console.log(tsql);
            console.log(e);
            done(e);
        }else{
            
            ++called;
            if (more) {
                try {
                    assert.deepEqual(r, NewExpectedData, dataComparisonFailed);
                }
                catch (assert) {
                    dataComparisonFailedMessage(NewExpectedData, r, done);
                    return;
                }
            }
            else {
                try {
                    assert.deepEqual(r, NewExpectedData, dataComparisonFailed);
                }
                catch (assert) {
                    dataComparisonFailedMessage(NewExpectedData, r, done);
                    return;
                }
            }
            if (called > len-1) {
                done();
            }
        }
        
    });
}

//将数据拆分成32位，保存在数组中
function int2bits(number){
  var arr = bits.parse(number);
  var bitsArr = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0];
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
  return bitsArr.reverse();
}
function alarmAddress(valBits,ifAlarm){
    var address = [];
    for(var i in valBits){
        if(valBits[i]==ifAlarm){
            address.push(i);
        }
    }
    return address;
}
function realBits2Table(Connection,data,name,valueBits,time,logger,done){
    var name1 = name.substring(1);
    var position1 = name1.indexOf('\/');
    var fengChang_name = name1.substring(0,position1);
    var position2 = name1.lastIndexOf('\/');
    var fengJi_name = name1.substring(position1+1,position2);
    var str = formatDate(time);
    var strTime = "'"+str+"'";
    var addressAlarm = alarmAddress(valueBits,configure.alarm);
    Connection.beginTransaction(function(err){
        if(err){
            done(err);
        }else{
            var tSql = '';
            var len=0;
            console.log("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
            async.series([
                function(done){
                    for(var i in addressAlarm){
                        //产生了报警，插入实时表，数据存在更新，不存在插入
                        (function(idx,len,tSql,i){
                            async.waterfall([
                                function(done){
                                    var judge = [];
                                    var state = '报警产生';
                                    judge[0] = "select count(1) from "+configure.real_table+" where 风场名称 ='"+fengChang_name+"'"+" and 风机名称='"+fengJi_name+"'"+" and 时间=convert(datetime,"+strTime+",21)"+" and 故障描述='"+data[name]["error"][idx]+"'"+";";
                                    judge[1] = "select count(1) from "+configure.his_table+" where 风场名称 ='"+fengChang_name+"'"+" and 风机名称='"+fengJi_name+"'"+" and 时间=convert(datetime,"+strTime+",21)"+" and 报警状态='"+state+"'"+" and 故障描述='"+data[name]["error"][idx]+"'"+";";
                                    judge2ExecSql(Connection,judge,logger,done); 

                                },
                                function(cnt,done){
                                    if(cnt[0]==0 && cnt[1]==0){
                                        len +=2;
                                        console.log(len);
                                        //插入实时
                                        var insertStr = "insert into "+configure.real_table+"(风场名称,风机名称,时间,故障描述) values('"+fengChang_name+"'"+",'"+fengJi_name+"',"+"convert(datetime,"+strTime+",21),'"+data[name]["error"][idx]+"'"+");";
                                        var insertStr1 = "insert into "+configure.his_table+"(风场名称,风机名称,时间,报警状态,故障描述) values('"+fengChang_name+"'"+",'"+fengJi_name+"',"+"convert(datetime,"+strTime+",21),"+"'报警产生',"+"'"+data[name]["error"][idx]+"'"+");";
                                        //console.log(insertStr);
                                        tSql += insertStr+insertStr1; 
                                        done();
                                    }else if(cnt[0]==1 && cnt[1]==0){
                                        len+=1;
                                        var insertStr1 = "insert into "+configure.his_table+"(风场名称,风机名称,时间,报警状态,故障描述) values('"+fengChang_name+"'"+",'"+fengJi_name+"',"+"convert(datetime,"+strTime+",21),"+"'报警产生',"+"'"+data[name]["error"][idx]+"'"+");";  
                                        tSql += insertStr1;
                                        done();
                                    }else if(cnt[0]==0 && cnt[1]==1){
                                        len+=1;
                                        var insertStr = "insert into "+configure.real_table+"(风场名称,风机名称,时间,故障描述) values('"+fengChang_name+"'"+",'"+fengJi_name+"',"+"convert(datetime,"+strTime+",21),'"+data[name]["error"][idx]+"'"+");";
                                        tSql += insertStr;
                                        done();
                                    }else{
                                        done();
                                    }

                                }
                            ],function(err,result){
                                if(err){
                                    logger.error("数据插入失败");
                                    done("数据插入失败");
                                }else{
                                    //console.log(idx);
                                    if(i>=addressAlarm.length-1){
                                        console.log("leng::::",len);
                                        var expected = {meta:null,rowcount:1};
                                        console.log(tSql);
                                        compoundQueryTSQL(Connection, tSql,len,expected,done);  
                                    }
                                }
                            });
                        })(addressAlarm[i],len,tSql,i);
                    }
                },
                function( done ) {
                    Connection.commit(function( err ) { 
                        if(err){
                            logger.error("数据插入失败");
                            done("数据插入失败");
                        }else{
                            logger.info("插入实时历史数据后提交成功!");
                            done(); 
                        }
                    });
                }
                ],function(err,result){
                    if(err){
                        logger.error("数据插入失败,回滚!");
                        Connection.rollback( function( err ) { 
                            assert.ifError( err );
                        });
                        done(err);
                    }else{
                        done();
                    }
            });
        }
    });
    
}

function initTable(Connection,curValue,logger,data,dataKeys,done){
    for(var i in curValue){
        for(var j in dataKeys){
            if(curValue[i].name==dataKeys[j]){
                var valueBits = int2bits(curValue[i].value.value);
                if(valueBits){
                  //  (function(idx){
                        logger.info("初始化实时表,历史表");
                        realBits2Table(Connection,data,curValue[i].name,valueBits,curValue[i].value.time,logger,done);

                    //})(i);
                }
            }
        }
    }
}
function updateTable(sqlCon,value,logger,data,datakeys,done){

}
module.exports.execQuerySql = execQuerySql;
module.exports.judgeExecSql = judgeExecSql;
module.exports.initTable = initTable;
module.exports.updateTable = updateTable;


