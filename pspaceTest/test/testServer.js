/*
 *服务器相关接口测试 
 */

/*
 *导入pspace的node.js组件
 */
var addon = require("../lib/pSpace");
/*
 *实例化一个组件对象
 */
var ps = new addon.Client();
/*数据库的连接:connect()
 *连接数据库所需要的参数提供两种方式
 *	方式一：json串的方式
 *	方式二：直接传入参数
 *参数中的端口为可选参数，如果不提供，系统将使用默认端口8889连接数据库
 *连接方式提供同步连接和异步连接两种方式
 *返回值：
 *	连接成功返回一个连接对象
 *	连接失败返回一个错误对象，对象含有code,errString两个属性
 */
 var connectDate = {
	"hostname":"localhost",
	"user":"admin",
	"password":"admin888",
	"port":8889
};
/*
 *同步连接
 */
 
var con = ps.connect(connectDate);
if(con.hasOwnProperty("errString")){
	console.log("错误码:",con.code);
	console.log("服务器连接失败:",con.errString);
	throw "connect fail!";
}else{
	console.log("连接成功!");
}

/*这种方式也可以
var con = ps.connect("localhost","admin","admin888");
if(con.hasOwnProperty("errString"))
{
	console.log("错误码:",con.code);
	console.log("服务器连接失败:",con.errString);
	throw "connect fail!";
}
*/
/*
 *异步方式
*/
/*
var res = ps.connect(connectDate,function(err,con){
	if(err){
		console.log(err);
		throw "connect fail!";
	}else{
		console.log("连接成功!");
	}
});
*/
/*
 *判断是否连接：isConnected()
 *返回值：
 *如果已经连接，返回true，如果未连接，返回false
*/

if (con.isConnected()) {
		console.log("con 已经连接！");
}else{
		console.log("con 已经断开连接!");
	 }

/*
 *断开连接:close()
 */
 
 con.close();
 
