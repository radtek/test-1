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
  YXZT       VARCHAR(5) comment '运行状态',\
  HG_YL      DOUBLE comment '汇管压力',\
  HG_WD      DOUBLE comment '汇管温度',\
  JK_YY      DOUBLE comment '汇管油压',\
  JK_WD      DOUBLE comment '井口温度',\
  JK_TY      DOUBLE comment '井口套压',\
  JRL_YCWD   DOUBLE comment '加热炉烟囱温度',\
  JRL_YW     DOUBLE comment '加热炉油温',\
  JK_YGYW    DOUBLE comment '井口油罐液位',\
  BR_WD      DOUBLE comment '伴随温度',\
  BR_YL      DOUBLE comment '伴随压力',\
  BPQ_SCPL   DOUBLE comment '变频器输出频率',\
  BPQ_SCDY   DOUBLE comment '变频器输出电压',\
  BPQ_SCDL   DOUBLE comment '变频器输出电流',\
  BPQ_MXDY   DOUBLE comment '变频器母线电压',\
  CX_LJLL    DOUBLE comment '掺稀流量(累计)',\
  CX_SSLL    DOUBLE comment '掺稀流量(瞬时)',\
  CX_WD      DOUBLE comment '掺稀温度',\
  CX_YL      DOUBLE comment '掺稀压力',\
  DL_A       DOUBLE comment 'A相电流',\
  DL_B       DOUBLE comment 'B相电流',\
  DL_C       DOUBLE comment 'C相电流',\
  DY_A       DOUBLE comment 'A相电压',\
  DY_B       DOUBLE comment 'B相电压',\
  DY_C       DOUBLE comment 'C相电压',\
  DY_AB      DOUBLE comment 'AB线电压',\
  DY_BC      DOUBLE comment 'BC线电压',\
  DY_CA      DOUBLE comment 'CA线电压',\
  GL_YG      DOUBLE comment '有功功率',\
  GL_WG      DOUBLE comment '无功功率',\
  GL_SZ      DOUBLE comment '视在功率',\
  GLYS       DOUBLE comment '功率因数',\
  ZQ_SSLL    DOUBLE comment '蒸汽流量测量值',\
  ZQ_LJLL    DOUBLE comment '蒸汽流量累计值',\
  ZQ_YL      DOUBLE comment '蒸汽压力',\
  ZQ_WD      DOUBLE comment '蒸汽温度',\
  ZQ_GD      DOUBLE comment '蒸汽干度',\
  Z_XZYGDNZ  DOUBLE comment '正向总有功电能值',\
  STJRL_YW   DOUBLE comment '水套液位'\
)comment '油井实时数据'";

//连接mysql 
var conn = psql.connectMysql("localhost","root","",database );
//创建数据库
//psql.createDatabase(database,conn);


 //创建表
 psql.createTable(tableSql,conn);
//读取配置转储点表的配置文件
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
  //开始转储到mysql中
  conn.query('insert into '+ table +   
    ' set '+columns.TagColName+ '=?',  
    [Ps.currentValue(columns.TagLongName)],function(err,rows,fields){
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
/*
//STR_TO_DATE(value[0].time,%Y-%m-%d %k:%i:%s
var tagName1 = "\\采集站1\\30200003\\30200152\\HJSH127_25\\TY_TYZXLC";
var tagName2 = "\\采集站1\\30200003\\30200152\\HJSH127_25\\TY_TYXHW";
var value1 = Ps.realRead(tagName1);
var value2 = Ps.realRead(tagName2);
 
//从pspace中导入数据到mysql,不带回调函数

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
conn.query(insertSql1,value1);
   
   
//从pspace中导入数据到mysql,带回调函数
conn.query(insertSql2,value2,function(err,rows,fields){
          if (err) throw err;
          else console.log("插入成功！");
});

*/

