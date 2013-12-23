var oracle  = require('oracle');
var connectData = {
        "hostname":"localhost",
        "database":"xe",
        "user":"system",
        "password":"yinguangfei"
};

oracle.connect(connectData,function(err,connection){
  if (err) {
    console.log(err);
  }else{
    connection.execute("INSERT INTO NODE_TEST (A,B)"+
                        "VALUES(:1,:2) RETURNING A INTO :3",
                        ['5','6',new oracle.OutParam()],
                        function(err,results){
                          if (err) {
                            console.log(err);
                          }else{
                            console.log(results);
                          }
                          connection.console();
                        });
  }
});