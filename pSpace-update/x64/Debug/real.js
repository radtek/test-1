var addon = require("./pSpace");
var ps = new addon.Client();
var Err = addon.Error;
var con = ps.connect("localhost","admin","admin888");
if(con instanceof Err){
	console.log("错误码:",con.code);
	console.log("服务器连接失败:",con.errString);
	throw "connect fail!";
}else{
	console.log("连接成功!");
}
console.log();

//模拟sleep（假死）
function sleep(milliSeconds){
    var startTime = new Date().getTime(); // get the current time
    while (new Date().getTime() < startTime + milliSeconds); // hog cpu
}
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
    var   milSecond = now.getMilliseconds();    
    return   year+"-"+Appendzero(month)+"-"+Appendzero(date)+" "+Appendzero(hour)+":"+Appendzero(minute)+":"+Appendzero(second)+"."+Appendzero(milSecond);     

}

var d = new Date().getTime();
var time  = new Date(d);
var pv = con.read("/tag/tag1.pv");
if(pv.hasOwnProperty("errString")){
	console.log(pv.code);
	console.log(pv.errString);
}else{
	console.log(pv);
	console.log(pv.value);
	console.log(pv.quality);
	console.log(formatDate(pv.time));
}
sleep(2000);
//异步读取实时数据
var res = con.read("/tag/tag3.pv",function(err,pv){
	if(err){
		console.log(err);
	}else{
		console.log(pv);
	}
});

 var data  = {
	"value":67.001,
	"quality":0,
	"time":time
};
var data1 = {
	"value":78.890,
	"quality":1
};
//同步写
var resWrite = con.write("/tag/tag1.pv",data);
if(resWrite instanceof Err){
	console.log(resWrite.code);
	console.log(resWrite.errString);
}else{
	console.log(resWrite,":写实时成功");
}
var resWrite1 = con.write("/tag/tag2.pv",data1);
if(resWrite1 instanceof Err){
	console.log(resWrite1.code);
	console.log(resWrite1.errString);
}else{
	console.log(resWrite1,":写实时成功");
}
//直接传参
var resWrite2 = con.write("/tag5/tag6.pv",89.78,0,time);
if(resWrite2.hasOwnProperty("errString")){
	console.log(resWrite2.code);
	console.log(resWrite2.errString);
}else{
	console.log(resWrite2,":写实时成功");
}
sleep(2000);
//异步
var resAsy = con.write("tag5/tag6.pv",data, function(err,tagName){
	if(err){
		console.log(err);
	}else{
		console.log(tagName,":异步写实时成功");
	}
}); 
//异步直接传参
var resAsy1 = con.write("/tag/tag3/tag8.pv",80.23, function(err,tagName){
	if(err){
		console.log(err);
	}else{
		console.log(tagName,":异步直接传参写实时成功");
	}
}); 
var resAsy2 = con.write("/tag5/tag7.pv",20.234,1, function(err,tagName){
	if(err){
		console.log(err);
	}else{
		console.log(tagName,":异步写实时成功");
	}
}); 
