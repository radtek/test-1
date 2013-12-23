//导入csv模块
var csv = require('csv');
var Ps = require("./lib/server");
//连接服务器
Ps.connect("localhost","admin","admin888");
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
    console.log("currentValue:",v1);
})
.on('close', function(count){
  console.log('Number of lines: '+count);
})
.on('error', function(error){
  console.log(error.message);
});

//四化油井实时数据表
var tableSql = "CREATE TABLE XH.SCY_SS_YJ\
(\
  JH         VARCHAR(255) not null comment '井号',\
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
  STJRL_YW   DOUBLE comment '水套液位',\
  ZQCXDLZDZ  DOUBLE comment '周期内C相电流最大值',\
  ZQZYGGLZDZ DOUBLE comment '周期内总有功功率最大值',\
  ZQZWGGLZDZ DOUBLE comment '周期内总无功功率最大值',\
  ZQZGLYSZDZ DOUBLE comment '周期内总功率因数最大值',\
  ZQAXDLZXZ  DOUBLE comment '周期内A相电流最小值',\
  ZQBXDLZXZ  DOUBLE comment '周期内B相电流最小值',\
  ZQCXDLZXZ  DOUBLE comment '周期内C相电流最小值',\
  ZQZYGGLZXZ DOUBLE comment '周期内总有功功率最小值',\
  ZQZWGGLZXZ DOUBLE comment '周期内总无功功率最小值',\
  ZQZGLYSZXZ DOUBLE comment '周期内总功率因数最小值',\
  ZQAXDLPJZ  DOUBLE comment '周期内A相电流平均值',\
  ZQBXDLPJZ  DOUBLE comment '周期内B相电流平均值',\
  ZQCXDLPJZ  DOUBLE comment '周期内C相电流平均值',\
  ZQZYGGLPJZ DOUBLE comment '周期内总有功功率平均值',\
  ZQZWGGLPJZ DOUBLE comment '周期内总无功功率平均值',\
  ZQZGLYSPJZ DOUBLE comment '周期内总功率因数平均值'\
)comment '油井实时数据'";