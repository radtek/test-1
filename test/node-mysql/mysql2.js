//导入mysql模块
var psql = require('./lib/PsToMysql2');
//导入pSpace 模块
var Ps = require('./lib/server');
//导入csv模块
var csv = require('csv');
//连接Psapce服务器
Ps.connect("localhost","admin","admin888");

var database = "XH";
var table = "SCY_SS_YJ";
//四化油井实时数据表
var tableSql = "CREATE TABLE XH.SCY_SS_YJ\
(\
  JH         VARCHAR(255) comment '井号',\
  CJSJ       TEXT,\
  YXZT       VARCHAR(5) comment '运行状态',\
  HG_YL      DOUBLE comment '汇管压力',\
  HG_WD      DOUBLE comment '汇管温度',\
  JK_YY      DOUBLE comment '汇管油压',\
  JK_WD      DOUBLE comment '井口温度',\
  JK_TY      DOUBLE comment '井口套压'\
)comment '油井实时数据'";

//连接mysql 
var conn = psql.connectMysql("localhost","root","",database );
//创建数据库
//psql.createDatabase(database,conn);


 //创建表
psql.createTable(tableSql,conn);
//读取配置转储点表的配置文件

function transfer()
{
csv()
.from.path('./YJ_HJSH127_25.csv',{
  columns:true
})
.transform(function(data,index){
  return  data;
})
.on('record', function(columns,index){
  
 // console.log(JSON.stringify(columns.TagColName));
  //console.log(columns.TagColName);
  var v1 = Ps.currentValue(columns.TagLongName);
  console.log(columns.TagLongName);
  console.log("currentValue:",v1);
 
conn.query('insert into '+ table +   
    ' set JH = ?,CJSJ = ?,YXZT = ?,HG_YL = ?,HG_WD = ?,JK_YY = ?,JK_WD = ?,JK_TY = ?',  
    [columns.TagLongName,Ps.currentTime(columns.TagLongName),Ps.currentQuality(columns.TagLongName),Ps.currentValue(columns.TagLongName),Ps.currentValue(columns.TagLongName),Ps.currentValue(columns.TagLongName),Ps.currentValue(columns.TagLongName),Ps.currentValue(columns.TagLongName)],function(err,rows,fields){
          if (err) throw err;
          else console.log("插入成功！");
      }); 
  })
.on('close', function(count){
  console.log('Number of lines: '+count);
  })
.on('error', function(error){
  console.log(error.message);
  });
}

setInterval(transfer,2000);
//setTimeout(transfer,2000);



/*
//没从点表配置文件中读取
//STR_TO_DATE(value[0].time,%Y-%m-%d %k:%i:%s
var tagName1 = "\\采集站1\\30200003\\30200152\\HJSH127_25\\TY_TYZXLC";
var tagName2 = "\\采集站1\\30200003\\30200152\\HJSH127_25\\TY_TYXHW";

//从pspace中导入数据到mysql,不带回调函数
function test()
{
var value1 = Ps.realRead(tagName1);
var value2 = Ps.realRead(tagName2);
var value1 = [tagName1,value1[0].time,value1[0].value];
var value2 = [tagName2,value2[0].time,value2[0].value];

var insertSql1 = 'insert into XH.SCY_SS_YJ\
                              set JH  = ?,\
                              CJSJ = ?,\
                              HG_YL  = ?';
var insertSql2 = 'insert into XH.SCY_SS_YJ\
                              set JH  = ?,\
                              CJSJ = ?,\
                              HG_YL  = ?';
 //插入一条数据，不带回调函数
conn.query(insertSql1,value1,function(err,rows,fields){
          if (err) throw err;
          else console.log("插入成功！");
          });
}

setInterval(test,2000);  

conn.query('insert into '+ table +   
    ' set JH = ?,CJSJ = ?,YXZT = ?,HG_YL = ?,HG_WD = ?,JK_YY = ?,JK_WD = ?,JK_TY = ?',  
    [columns.TagLongName,Ps.currentTime(columns.TagLongName),Ps.currentQuality(columns.TagLongName),Ps.currentValue(columns.TagLongName),Ps.currentValue(columns.TagLongName),Ps.currentValue(columns.TagLongName),Ps.currentValue(columns.TagLongName),Ps.currentValue(columns.TagLongName)],function(err,rows,fields){
          if (err) throw err;
          else console.log("插入成功！");
      }); 
 
*/
