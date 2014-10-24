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

function handleError () {
   var conn = ps.open(configure.pSpace);
   if(conn instanceof ps.Err){
    logger.error("connect error:",conn.errString);
    handleError();
   }else{
    conn.write("/fengChang/fengJi/tag.pv",8);
   }
}
handleError();

