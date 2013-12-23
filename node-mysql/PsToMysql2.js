//导入mysql模块
var mysql = require('mysql');

//连接mysql,返回连接对象
function connectMysql(hostName,userName,pass,databaseName)
{
  var PsMysql = mysql.createConnection({
    host     : hostName,
    user     : userName,
    password : pass,
    database : databaseName
  });
  //连接mysql
  PsMysql.connect();
  return PsMysql;
}
//创建数据库
function createDatabase(databaseName,mysqlObj)
{
    return mysqlObj.query('CREATE DATABASE '+databaseName,function(err)
             {
              if(err && err.number != mysqlObj.ERROR_DB_CREATE_EXISTS)
              {
               throw err;
               }
              }
  );
}
//创建表
function createTable(sql,mysqlObj)
{
  return mysqlObj.query( sql,function(err)
  {
     if(err) throw err;
  }      
  );  
}
exports.connectMysql = connectMysql;
exports.createDatabase = createDatabase;
exports.createTable = createTable;
