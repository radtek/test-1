var mysql = require('mysql');
var connection = mysql.createConnection({
  host     : 'localhost',
  user     : 'root',
  password : '',
  database : 'test'
});

var table_test = 'tableTest';

//�������ݿ�
connection.connect();


//����һ����
connection.query('create table if not exists ' + table_test + 
  '(id INT(11) AUTO_INCREMENT, '+  
  'title VARCHAR(255), '+  
  'text TEXT, '+   
  'PRIMARY KEY (id))'  , 
  function(err) {
    if(err) throw err;
});

//����һ�����ݣ������ص�������
connection.query('insert into '+ table_test +   
  ' set title = ? , text = ? ',  
  ['super 333', 'this is a nice text'] 
);

//����һ�����ݣ����ص�������
connection.query('insert into '+ table_test +   
  ' set title = ? , text = ? ',  
  ['super 444', 'this is a nice text'],
  function(err, rows, fields){
    if (err) throw err;
    console.log('The rows is: ', rows);
    console.log('The fields is: ', fields);
  }
);

//��ѯ�������ʾǰ5��
connection.query('select * from ' + table_test + ' limit 0, 5', function(err, rows, fields) {
  if (err) throw err;
  console.log('The solution is: ', rows);
});

//�ر�����
connection.end();