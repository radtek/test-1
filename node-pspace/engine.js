//����ģ��pSpaceģ��
var createObject = require('./pSpace');


//����pSpace���ݿ����
var Ps = createObject();

//����������Pspace ���ݿ�
var server = "localhost";
var userName = "admin";
var passwd = "admin888";

var isStart = Ps.startpSpace(server,userName,passwd);

if(isStart)
{
	console.log("pSpace1 start succeed!");
}
else
	console.log("Sorry,pSpace1 start fail!");

//��ȡʵʱ����
function  realRead()
{
  var val = Ps.realRead("\\testengine\\30200003\\30200152\\HJSH127_25\\LJCXLL");
  if(val)
    console.log("Read real-data succeed:",val);
  else
    console.log("Real data is 0.0000000!");
}

//дʵʱ����
function realWrite()
{
  var isWrite = Ps.realWrite("\\testengine\\30200003\\30200152\\HJSH127_25\\BPQLJLL",99.9989);
  if(isWrite)
    console.log("Write real-data succeed!");
  else 
    console.log("Write real-data error!");
 }

//������ʱ����ÿ��1���ȡһ��ʵʱֵ
setInterval(realRead, 1000);

//������ʱ����ÿ��������pSpace�л�ȡʵʱ����
setInterval(realWrite, 2000);

//�ص�����
function callStop()
{
	var stop = Ps.stoppSpace();

	if(stop)
		console.log("Disconnect succeed!");
	else
		console.log("Disconnect fail!");
}

//������ʱ����50���Ժ�Ͽ����ݿ�,�Ͽ�����֮�󽫵������ݶ�ȡ��Ч
//setTimeout(callStop,60000);
