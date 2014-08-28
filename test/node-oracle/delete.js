var oracle = require('oracle');
var connectData = {"hostname": "localhost", "database": "XE" ,
                   "user": "system", "password": "yinguangfei"};
 
oracle.connect(connectData, function(err, connection) {
  if ( err ) {
    console.log(err);
  } else {
    connection.execute("DELETE FROM NODE WHERE num = :1",
                       [3],
                       function(err, results) {
      if ( err ) {
        console.log(err);
      } else {
        console.log(results);
      }
      connection.close();
    });
  }
});
