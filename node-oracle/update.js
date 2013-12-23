var oracle  = require('oracle');
var connectData = {
        "hostname":"localhost",
        "database":"xe",
        "user":"system",
        "password":"yinguangfei"
};

oracle.connect(connectData, function(err, connection) {
  if ( err ) {

    console.log(err);
  } else {
    connection.execute("UPDATE NODE SET name = :1 WHERE num = :1",
                       ['lisi', 2],
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
