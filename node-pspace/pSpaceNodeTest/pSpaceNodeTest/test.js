//����pSpace�ӿ�ģ��
var Ps = require('./pSpaceNodeTest');

//�����������ؽӿڶ���
var conn = new Ps.Server();
//���ö��󷽷����ӷ�����
var isConn = conn.serverConnect("localhost","admin","admin888");
if(isConn>=0)
{
	console.log("Connect succeed!");
}
else
{
	console.log("Connect error");
}

//�����ȡID�Ķ���
//////var getid =  new Ps.GetIDByLongName();
var getid = createObject();


getid.getID("\\testCase_Node1\\testCase_Tag1");
//���ö��󷽷���ȡID
///var id = getid.getID("\\testCase_Node1\\testCase_Tag1");
///console.log("Get the tag testCase_Tag1's ID:",id);

//����ʵʱ������ض���
var real = new Ps.Real();
//��ȡʵʱ����
var val = real.realRead("\\testCase_Node1\\testCase_Tag1");
console.log("real data:",val);
//дʵʱ����
var ret = real.realWrite("\\testCase_Node1\\testCase_Tag1",55.55);
if(ret)
{
	console.log("Write real data succeed!");
}
else 
	console.log("Write Error!");
//���÷���������Ͽ�������
var isDisconn= conn.serverdisconnect();

if(isDisconn>=0)
	console.log("Disconnect succeed!");

else
	console.log("Disconnect error");
