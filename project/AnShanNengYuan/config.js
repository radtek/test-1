//sqlserver驱动配置
var driver = 'SQL Server Native Client 10.0';
var server = '(local)';
var user = '<user>';
var pwd = '<pwd>';
var database = 'test';
var useTrustedConnection = true;
var conn_str = "Driver={" + driver + "};Server=" + server + ";" + (useTrustedConnection == true ? "Trusted_Connection={Yes};" : "UID=" + user + ";PWD=" + pwd + ";") + "Database={" + database + "};";

//pSpace 配置
var psCon = {
  "hostname": "localhost",
  "user": "admin",
  "password": "admin888"
}
//转储周期(单位可变，目前的单位是分钟),每隔15分钟一条数据
var transTime = 15;
//任务执行周期(单位毫秒)，每隔5秒执行以转储任务
var interval = 5000;
// The following need to be exported for building connection strings within a test...
exports.database = database;
exports.server = server;
exports.user = user;
exports.pwd = pwd;
// Driver name needs to be exported for building expected error messages...
exports.driver = driver;
// Here's a complete connection string which can be shared by multiple tests...
exports.conn_str = conn_str;
exports.psCon = psCon;
exports.transTime = transTime;
exports.interval = interval;