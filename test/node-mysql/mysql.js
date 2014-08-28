//导入mysql模块
var psql = require('./lib/PsToMysql');
//导入pSpace 模块
var Ps = require('./lib/server');
//连接Psapce服务器
Ps.connect("localhost","admin","admin888");

var database = "pSpaceTag";
var table  = "tag";
//连接mysql 
var conn = psql.connectMysql("localhost","root","","pSpaceTag");

//创建数据库
//psql.createDatabase(database,conn);
//创建表
//psql.createTable(table,conn);
//将指定测点导入mysql的一张表

var tag = "\\采集站1\\30200003\\30200152\\HJSH127_25\\HGYL_CPXLH_XH1";

//不带回调函数读取并存入一条记录
//psql.psToMysqlSyn(tag,Ps,conn,table);

//带回调函数读取并存入一条记录
/*
psql.psToMysqlAsy(tag,Ps,conn,table,function(err,rows,fields){
  if (err) throw err;
  console.log(rows);

});
 */
//conn.query('insert into  testTable' +   
  //  ' set value = ?',  
    //[23432.4234]);
//查询,异步回调方式
updateSQLString = "update testTable set value=100.100 where tagName = '\\采集站1\\30200003\\30200152\\HJSH127_25\\HGYL_CPXLH_XH1'";
deleteSQLString = 'delete from teachers where ID=10';
var sql = "select * from testTable";

//conn.query(updateSQLString);

/*
psql.queryTag(sql,conn,function(err,rows,fields){
  if (err) throw err;
  console.log(rows);
});
*/
var i = 0;
//边读边存
function fun()
{
  
  var b = psql.psToMysqlSyn(tag,Ps,conn,table);
  
  if(b!=null)
  {
    console.log(i++);
  }
}
//边读边循环存储,周期为一秒
//setInterval(fun,1000);

//死循环读取并存储10000条记录
console.time('time');
while(i<=100)
{
  fun();
}
console.timeEnd('time');
//边实时订阅边存储数据,无结果
//psql.subscribleAndStore(tag,Ps,conn,table);
   
//边订阅边修改表中的值
 //psql.subscribleAndUpdate(tag,Ps,conn,table);


//断开连接
//Ps.disConnect();
//psql.disConnectMysql(conn);
