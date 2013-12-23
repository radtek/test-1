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

var id  = Ps.getID("\\testengine\\30200003\\30200152\\HJSH127_25\\LJCXLL");

console.log(id);

var tag1 = "\\testengine\\30200003\\30200152\\HJSH127_25\\LJCXLL";
//返回数组对象
var array = Ps.realRead(tag1);
console.log("tagName:",array[0].tagName);
console.log("value:",array[0].value);
console.log("quality:",array[0].quality);
console.log("time:",array[0].time);

