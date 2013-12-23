//导入模块pSpace模块
var createObject = require('./pSpace');


//创建pSpace数据库对象
var Ps = createObject();

//启动并连接Pspace 数据库
var server = "localhost";
var userName = "admin";
var passwd = "admin888";

var isStart = Ps.startpSpace(server,userName,passwd);

if(isStart)
{
	console.log("pSpace1 start succeed!");
}
else
	console.log("Sorry,pSpace1 start fail!");

//读取实时数据
function  realRead()
{
  var val = Ps.realRead("\\testengine\\30200003\\30200152\\HJSH127_25\\LJCXLL");
  if(val)
    console.log("Read real-data succeed:",val);
  else
    console.log("Real data is 0.0000000!");
}

//写实时数据
function realWrite()
{
  var isWrite = Ps.realWrite("\\testengine\\30200003\\30200152\\HJSH127_25\\BPQLJLL",99.9989);
  if(isWrite)
    console.log("Write real-data succeed!");
  else 
    console.log("Write real-data error!");
 }

//创建定时器，每隔1秒读取一次实时值
setInterval(realRead, 1000);

//创建定时器，每隔两秒往pSpace中获取实时数据
setInterval(realWrite, 2000);

//回调函数
function callStop()
{
	var stop = Ps.stoppSpace();

	if(stop)
		console.log("Disconnect succeed!");
	else
		console.log("Disconnect fail!");
}

//创建定时器，50秒以后断开数据库,断开连接之后将导致数据读取无效
//setTimeout(callStop,60000);
