var oracle = require("oracle");

var connectData = {"hostname": "localhost", "database": "xe" ,

                   "user": "system", "password": "yinguangfei"};


oracle.connect(connectData, function(err, connection) {
   console.log(connection);
  if ( err ) {
    console.log(err);
  } else {

    connection.execute("SELECT * FROM NODE", [], function(err, results) {

      if ( err ) {

        console.log(err);

      } else {
       
        console.log(results);
      }
      connection.close();
    });
  }
});
