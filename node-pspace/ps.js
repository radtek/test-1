//����ģ��pSpaceģ��
var createObject = require('./pSpace');
//����mysqlģ��
var mysql = require('mysql');

//����mysql����
var connection = mysql.createConnection({
  host     : 'localhost',
  user     : 'root',
  password : '',
  database : 'test'
});

//����һ�ű���pspace�������
var table_test = 'Tag';
//����mysql���ݿ�
connection.connect();

//����pSpace���ݿ����
var Ps = createObject();

//�ڶ���pSpace���Ӷ���
//var Ps2 = createObject();

//���������Ӷ���
//var Ps3 = createObject();
 
//����������Pspace ���ݿ�
var server = "localhost";
var userName = "admin";
var passwd = "admin888";

var isStart = Ps.startpSpace(server,userName,passwd);

var isStart2 = Ps.startpSpace(server,userName,passwd);


if(isStart)
{
	console.log("pSpace1 start succeed!");
}
else
	console.log("Sorry,pSpace1 start fail!");

if(isStart2)
{
	console.log("pSpace2 start succeed!");
}
else
	console.log("Sorry,pSpace2 start fail!");

//��ȡʵʱ����
var val = Ps.realRead("\\testCase_Node1\\testCase_Tag1");
if(val)
	console.log("Read real-data succeed:",val);
else
	console.log("No have real-data or read error!");


//дʵʱ����
var isWrite = Ps.realWrite("\\testCase_Node1\\testCase_Tag1",666.888);
if(isWrite)
	console.log("Write real-data succeed!");
else 
	console.log("Write real-data error!");

//ʵʱ����
var isSub = Ps.realsubscribe("\\testCase_Node1\\testCase_Tag1")

if(isSub)
{
	console.log("subscrible tag testCase_Tag1 Succeed");
}
else 
	Ps.log("���Ĳ��testCase_Tag1��ʵʱֵʧ��");

//�ص�����,��ȡʵʱֵ
function callRead()
{
	
	var val1 = Ps.realRead("\\testCase_Node1\\testCase_Tag1" );
	console.log("val:",val1);
	
}

//��mysql�д���һ��Tag��

function CreateTag()
{
	console.log("Create table Tag!");
	connection.query('create table if not exists ' + table_test + 
  		'(id INT(11) AUTO_INCREMENT, '+  
 		 'TagName VARCHAR(255), '+  
 		 'realValue DOUBLE, '+   
 		 'PRIMARY KEY (id))'  , 
		  function(err) {
  		  if(err) throw err;
		});
}
//��Tag���в���һ������
function insertTag()
{
	console.log("Start insert Tag to Myslql's table Tag:");
	//����һ�����ݣ����ص�������
	connection.query('insert into '+ table_test +   
  	' set TagName = ? , realValue = ? ',  
  	['\\testCase_Node1\\testCase_Tag1', Ps.realRead("\\testCase_Node1\\testCase_Tag1")],
 	 function(err, rows, fields){
    	if (err) throw err;
   	 console.log('The rows is: ', rows);
   	 console.log('The fields is: ', fields);
 	 }
	);
}

//��mysql�е����ݲ�ѯ�󷵻�

function GetTagFromMysql()
{
	console.log("The result from table Tag:");
	//��ѯ�������ʾǰ5��
	connection.query('select * from ' + table_test + ' limit 0, 5', function(err, rows, fields) {
  	if (err) throw err;
  	console.log('The solution is: ', rows);
	});
}
//������ʱ����ÿ��1���ȡһ��ʵʱֵ
setInterval(callRead, 1000);

//���ú���������
CreateTag();
//������ʱ��10���Ժ����ݲ���mysql

setTimeout(insertTag,10000);
//12���Ժ�ȡ������
setTimeout(GetTagFromMysql,10000);


//�ص�����
function callStop()
{
	var stop = Ps.stoppSpace();

	if(stop)
		console.log("Disconnect succeed!");
	else
		console.log("Disconnect fail!");
}

function stopMysql()
{
	//�ر�����
	connection.end();
}
//������ʱ����50���Ժ�Ͽ����ݿ�,�Ͽ�����֮�󽫵������ݶ�ȡ��Ч
setTimeout(callStop,50000);
//40���Ժ�Ͽ�Mysql����
setTimeout(stopMysql,40000);




