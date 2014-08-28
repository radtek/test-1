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
    /*
      name = :1 where num = :1中的:1均是占位符，无实际意义，它对应着[]中的值。
    */
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
