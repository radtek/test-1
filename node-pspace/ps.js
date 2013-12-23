//导入模块pSpace模块
var createObject = require('./pSpace');
//导入mysql模块
var mysql = require('mysql');

//创建mysql连接
var connection = mysql.createConnection({
  host     : 'localhost',
  user     : 'root',
  password : '',
  database : 'test'
});

//定义一张表存放pspace相关数据
var table_test = 'Tag';
//连接mysql数据库
connection.connect();

//创建pSpace数据库对象
var Ps = createObject();

//第二个pSpace连接对象
//var Ps2 = createObject();

//第三个连接对象
//var Ps3 = createObject();
 
//启动并连接Pspace 数据库
var server = "localhost";
var userName = "admin";
var passwd = "admin888";

var isStart = Ps.startpSpace(server,userName,passwd);

var isStart2 = Ps.startpSpace(server,userName,passwd);


if(isStart)
{
	console.log("pSpace1 start succeed!");
}
else
	console.log("Sorry,pSpace1 start fail!");

if(isStart2)
{
	console.log("pSpace2 start succeed!");
}
else
	console.log("Sorry,pSpace2 start fail!");

//读取实时数据
var val = Ps.realRead("\\testCase_Node1\\testCase_Tag1");
if(val)
	console.log("Read real-data succeed:",val);
else
	console.log("No have real-data or read error!");


//写实时数据
var isWrite = Ps.realWrite("\\testCase_Node1\\testCase_Tag1",666.888);
if(isWrite)
	console.log("Write real-data succeed!");
else 
	console.log("Write real-data error!");

//实时订阅
var isSub = Ps.realsubscribe("\\testCase_Node1\\testCase_Tag1")

if(isSub)
{
	console.log("subscrible tag testCase_Tag1 Succeed");
}
else 
	Ps.log("订阅测点testCase_Tag1的实时值失败");

//回调函数,读取实时值
function callRead()
{
	
	var val1 = Ps.realRead("\\testCase_Node1\\testCase_Tag1" );
	console.log("val:",val1);
	
}

//在mysql中创建一张Tag表

function CreateTag()
{
	console.log("Create table Tag!");
	connection.query('create table if not exists ' + table_test + 
  		'(id INT(11) AUTO_INCREMENT, '+  
 		 'TagName VARCHAR(255), '+  
 		 'realValue DOUBLE, '+   
 		 'PRIMARY KEY (id))'  , 
		  function(err) {
  		  if(err) throw err;
		});
}
//在Tag表中插入一条数据
function insertTag()
{
	console.log("Start insert Tag to Myslql's table Tag:");
	//插入一条数据（带回调函数）
	connection.query('insert into '+ table_test +   
  	' set TagName = ? , realValue = ? ',  
  	['\\testCase_Node1\\testCase_Tag1', Ps.realRead("\\testCase_Node1\\testCase_Tag1")],
 	 function(err, rows, fields){
    	if (err) throw err;
   	 console.log('The rows is: ', rows);
   	 console.log('The fields is: ', fields);
 	 }
	);
}

//把mysql中的数据查询后返回

function GetTagFromMysql()
{
	console.log("The result from table Tag:");
	//查询结果，显示前5条
	connection.query('select * from ' + table_test + ' limit 0, 5', function(err, rows, fields) {
  	if (err) throw err;
  	console.log('The solution is: ', rows);
	});
}
//创建定时器，每隔1秒读取一次实时值
setInterval(callRead, 1000);

//调用函数创建表
CreateTag();
//创建定时器10秒以后将数据插入mysql

setTimeout(insertTag,10000);
//12秒以后取出数据
setTimeout(GetTagFromMysql,10000);


//回调函数
function callStop()
{
	var stop = Ps.stoppSpace();

	if(stop)
		console.log("Disconnect succeed!");
	else
		console.log("Disconnect fail!");
}

function stopMysql()
{
	//关闭链接
	connection.end();
}
//创建定时器，50秒以后断开数据库,断开连接之后将导致数据读取无效
setTimeout(callStop,50000);
//40秒以后断开Mysql连接
setTimeout(stopMysql,40000);




