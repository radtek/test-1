//导入pSpace接口模块
var Ps = require('./pSpaceNodeTest');

//定义服务器相关接口对象
var conn = new Ps.Server();
//调用对象方法连接服务器
var isConn = conn.serverConnect("localhost","admin","admin888");
if(isConn>=0)
{
	console.log("Connect succeed!");
}
else
{
	console.log("Connect error");
}

//定义获取ID的对象
//////var getid =  new Ps.GetIDByLongName();
var getid = createObject();


getid.getID("\\testCase_Node1\\testCase_Tag1");
//调用对象方法获取ID
///var id = getid.getID("\\testCase_Node1\\testCase_Tag1");
///console.log("Get the tag testCase_Tag1's ID:",id);

//定义实时操作相关对象
var real = new Ps.Real();
//读取实时数据
var val = real.realRead("\\testCase_Node1\\testCase_Tag1");
console.log("real data:",val);
//写实时数据
var ret = real.realWrite("\\testCase_Node1\\testCase_Tag1",55.55);
if(ret)
{
	console.log("Write real data succeed!");
}
else 
	console.log("Write Error!");
//调用服务器对象断开服务器
var isDisconn= conn.serverdisconnect();

if(isDisconn>=0)
	console.log("Disconnect succeed!");

else
	console.log("Disconnect error");
