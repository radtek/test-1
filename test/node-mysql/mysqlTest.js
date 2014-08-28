var mysql = require('mysql');
var connection = mysql.createConnection({
  host     : 'localhost',
  user     : 'root',
  password : '',
  database : 'test'
});

var table_test = 'tableTest';

//连接数据库
connection.connect();


//创建一个表
connection.query('create table if not exists ' + table_test + 
  '(id INT(11) AUTO_INCREMENT, '+  
  'title VARCHAR(255), '+  
  'text TEXT, '+   
  'PRIMARY KEY (id))'  , 
  function(err) {
    if(err) throw err;
});

//插入一条数据（不带回调函数）
connection.query('insert into '+ table_test +   
  ' set title = ? , text = ? ',  
  ['super 333', 'this is a nice text'] 
);

//插入一条数据（带回调函数）
connection.query('insert into '+ table_test +   
  ' set title = ? , text = ? ',  
  ['super 444', 'this is a nice text'],
  function(err, rows, fields){
    if (err) throw err;
    console.log('The rows is: ', rows);
    console.log('The fields is: ', fields);
  }
);

//查询结果，显示前5条
connection.query('select * from ' + table_test + ' limit 0, 5', function(err, rows, fields) {
  if (err) throw err;
  console.log('The solution is: ', rows);
});

//关闭链接
connection.end();