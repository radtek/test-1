var addon = require("pSpace"); 
var events = require( "events" );
var emitter = new events.EventEmitter(); 
var ps = new addon();
var Err = ps.Err;
function Appendzero(obj)
{
     if(obj<10) return "0" +""+ obj;
     else return obj;     
}
var formatDate = function(now){
	var   year=now.getFullYear();     
    var   month=now.getMonth()+1;     
    var   day=now.getDate();     
    return   year+"-"+Appendzero(month)+"-"+Appendzero(day);     

}
var conStr = {
	"host":"localhost",
	"user":"admin",
	"password":"admin888"
};
function handleError () {
    ps.open(conStr,function (err,conn) {
        if (err) {
            console.log('error when connecting to db:', err);
        }else{
           con = conn;
        }
    });
}
handleError();
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
//每隔一秒检查一次数据库是否已经连接
setInterval(fun,1000);
function accurateInterval(callback, interval) {
        var now = +new Date();
        setTimeout(function run() {
            now += interval;
            var fix = now - (+Date.now());
            setTimeout(run, interval + fix);
            callback();
        }, interval);
    }
function task(){
    var d = new Date();
    con.write("/系统变量/时间变量/date.pv",formatDate(d));
    con.write("/系统变量/时间变量/time.pv",d.toLocaleTimeString());
    con.write("/系统变量/时间变量/curtime.pv",d.getTime()/1000);
    con.write("/系统变量/时间变量/minute.pv",d.getMinutes());
    con.write("/系统变量/时间变量/second.pv",d.getSeconds());
    con.write("/系统变量/时间变量/hour.pv",d.getHours());
    con.write("/系统变量/时间变量/day.pv",d.getDate());
    con.write("/系统变量/时间变量/month.pv",d.getMonth()+1);
    con.write("/系统变量/时间变量/year.pv",d.getFullYear());
}
accurateInterval(task,1000);
process.on('SIGINT', function () {
  // wait connections to close
  process.exit();
  console.log("gracefully shutting down from  SIGINT (Crtl-C)".yellow);
});

