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
{
	console.log("Sorry,pSpace1 start fail!");
}

var nodeArray1 = [];
var nodeArray2 = [];
var nodeArray3 = [];

var ptagName1 = "\\testengine\\30200003\\30200152\\HJSH127_25\\";
var ptagName2 = "\\testengine\\30200003\\30200152\\HJSH127_28\\";
var ptagName3 = "\testengine\30200003\30200152\HJSH127_29\\";

nodeArray1.push(ptagName1+"LJCXLL",ptagName1+"ZC_ZQLLLJCLZ",ptagName1+"BPQLJLL1");
nodeArray2.push(ptagName2+"LJCXLL",ptagName2+"ZC_ZQLLLJCLZ",ptagName2+"BPQLJYDL");
nodeArray3.push(ptagName3+"LJCXLL",ptagName3+"ZC_ZQLLLJCLZ",ptagName3+"BPQLJYDL");

var tag1 ="\\testengine\\30200003\\30200152\\HJSH127_25\\LJCXLL";
var tag2 ="\\testengine\\30200003\\30200152\\HJSH127_25\\BPQLJLL";

//�ɼ�ĳһ����㵱ǰֵ
function getCurrentVal()
{
  var v1 = Ps.realRead(tag1);
  var v2 = Ps.realRead(tag2);
  var restltVal = v1*256+v2;
  console.log("CurrentVal:",restltVal);
}

//ÿ��һ��ɼ�һ��ĳһ��㵱ǰֵ
setInterval(getCurrentVal, 1000);