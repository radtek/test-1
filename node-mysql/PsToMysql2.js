//����mysqlģ��
var mysql = require('mysql');

//����mysql,�������Ӷ���
function connectMysql(hostName,userName,pass,databaseName)
{
  var PsMysql = mysql.createConnection({
    host     : hostName,
    user     : userName,
    password : pass,
    database : databaseName
  });
  //����mysql
  PsMysql.connect();
  return PsMysql;
}
//�������ݿ�
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
//������
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
