/**********������Ϊ������ѡ�����ѡ���Ĭ��**********/
//pSpace����
var hostname = "localhost:8889";  //���������IP���˿ں�
var user = "admin";  //������û���
var password = "admin888";  //���������
var conStr ={
		"hostname":hostname,
		"user":user,
		"password":password
};
var keyWord = "HL";  //�������ѯ��㳤�������ùؼ��ֵĲ�㣬��ʱ��Ҫ������
var excWord = ["Udc"];  //������ų���㳤�������ùؼ��ֵĲ�㣬�ַ������飬�ԡ�,��������["a","b"];����Ϊ��[],��ʾ���ų��κβ��
var uplimit = 110;  //���������ֵ���ٷ�������110��ʾ������110%Ϊ���ޣ�
var lowlimit = 90;  //���������ֵ���ٷ�������90��ʾ������90%Ϊ���ޣ�
var quality = [192];  //�����Ҫ�Ƚϵĵ�ǰֵ�������������飬�磺[32,192]��ʾ�Ƚ�������Ϊ32��192��ֵ��Ϊ��ʱ:[]��ʾ����ֵ
var symbol = 1;  //�������ǰֵΪ����ʱ�Ƿ�ͳ�ƣ�1����ͳ�ƣ�0��ͳ��

//sqlserver����
var driver = 'SQL Server Native Client 11.0';
var server = '(local)';  //�����IP��ַ
var db_user = 'sa';  //��������ݿ��û���
var pwd = '123456';  //��������ݿ�����
var database = 'knwy';  //��������ݿ���
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



