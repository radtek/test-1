var os = require("os");

console.log("操作系统默认的临时文件目录:",os.tmpdir());
console.log("hostName:",os.hostname());
console.log("os:",os.type());
//console.log("os plat:",os.platfrom());
console.log("cpu:",os.arch());
//console.log("os runtime:".os.uptime());
console.log("mem:",os.totalmem());

/*
var http = require("http");
http.createServer(function(req,res){
	//res.writeHead(200,{"content-Type":'text/html'});
	//res.write('<hl>Hello<hl>');
	res.end();
}).listen(3000);
*/

var events = require('events');

var emitter = new events.EventEmitter();

emitter.on("showHello",function(arg){
	arg();
});

var fun = function()
{
	console.log("hello");
}
emitter.emit("showHello",fun);

//

var scope = "global Scope";
function checkScope(){
	var scope = "local scope";
	function f(){return scope;}
	return f();
}
console.log(checkScope());

var unique = (function(){
	var counter = 0;
	return function(){return counter++};
}());

var unique1 = (function(){
	var counter = 0;
	return counter++;
});
for(i=0;i<5;i++){
	console.log(unique());
}
for(i=0;i<5;i++){
	console.log(unique1());
}

function counter(){
	var n = 0;
	return {
		count:function(){return n++;},
		result:function(){n=0;}
	};
}

var c = counter(),d = counter();
console.log(c.count());//0
console.log(d.count());//0
console.log(c.result());
console.log(c.count());//0
console.log(d.count());//1

var time  = new Date();
console.log(time);
//var dateObj=new Date(year, month, day, hours, minutes, seconds, microseconds);
//console.log(dateObj);
console.log(time.getMilliseconds());