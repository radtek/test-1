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

var id  = Ps.getID("\\testengine\\30200003\\30200152\\HJSH127_25\\LJCXLL");

console.log(id);

var tag1 = "\\testengine\\30200003\\30200152\\HJSH127_25\\LJCXLL";
//�����������
var array = Ps.realRead(tag1);
console.log("tagName:",array[0].tagName);
console.log("value:",array[0].value);
console.log("quality:",array[0].quality);
console.log("time:",array[0].time);

