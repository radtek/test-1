var mysql=  require('mysql');
var userId = 1;
//var sql = "SELECT * FROM ?? WHERE ?? = ?";
[""]
var inserts = {uer:"uer",id:'id', uid:userId};
sql = mysql.format("?", inserts);
console.log(sql);

/*
var sql = "SELECT * FROM ?? WHERE ?? = ?";
var inserts = ['users', 'id', userId];
sql = mysql.format(sql, inserts);
*/
/*
connection = mysql.createConnection({
  host: 'localhost',
    user: 'root',
    password: '123456',
    database: 'mydata',
    port: 3306
    });
    connection.connect(function(err) {
    if (err) {
     // logger.error('error connecting: ' + err.stack);
     // done(err);
    }else{
     //logger.info("mysql¡¨Ω”ÕÍ±œ°£"); 
     //done();
    }
  });
var post  = {id: 1, title: 'Hello MySQL'};
var query = connection.query('INSERT INTO posts SET ?', post, function(err, result) {
  // Neat!
  if(err)
    console.log(err);
    else{
      console.log(result);
    }
});
console.log(query); 
*/