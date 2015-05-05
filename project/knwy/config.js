/**********配置项为可配置选项，其他选项保持默认**********/
//pSpace配置
var hostname = "localhost:8889";  //配置项：主机IP及端口号
var user = "admin";  //配置项：用户名
var password = "admin888";  //配置项：密码

var keyWord = "HL";  //配置项：查询测点长名包含该关键字的测点，暂时不要有中文，“”为查询所有
var excWord = ["Udc"];  //配置项：排除测点长名包含该关键字的测点，字符串数组，以“,”隔开如["a","b"];不填为：[],表示不排除任何测点
var uplimit = 110;  //配置项：上限值（百分数，如110表示，超过110%为超限）
var lowlimit = 90;  //配置项：下限值（百分数，如90表示，低于90%为超限）
var quality = [192];  //配置项：要比较的当前值的质量戳，数组，如：[32,192]表示只比较质量戳为32、192的值；为空时:[]表示比较所有值
var symbol = 1;  //配置项：当前值为负数时是否统计，1：不统计；0：统计

//sqlserver配置
var server = '(local)';  //配置项：IP地址
var db_user = 'sa';  //配置项：数据库用户名
var pwd = '123456';  //配置项：数据库密码
var database = 'knwy';  //配置项：数据库名

var conStr ={
		"hostname":hostname,
		"user":user,
		"password":password
};
var driver = 'SQL Server Native Client 11.0';
var useTrustedConnection = true;
var dbStr = "Driver={" + driver + "};Server=" + server + ";" + (useTrustedConnection == true ? "Trusted_Connection={Yes};" : "UID=" + db_user + ";PWD=" + pwd + ";") + "Database={" + database + "};";

exports.conStr = conStr;
exports.dbStr = dbStr;
exports.keyWord = keyWord;
exports.excWord = excWord;
exports.uplimit = uplimit;
exports.lowlimit = lowlimit;
exports.quality = quality;
exports.symbol = symbol;



