var oracle  = require('oracle');

var connectData = {
        "hostname":"localhost",
        "database":"xe",
        "user":"system",
        "password":"yingf"
};
oracle.connect(connectData);
/*
oracle.connect(connectData,function(err,connection){
  if (err) {
    console.log(err);
  }else{
    console.log(connection);
    connection.execute("INSERT INTO NODE (A,B)"+
                        "VALUES(:1,:2) RETURNING A INTO :3",
                        ['5','6',new oracle.OutParam()],
                        function(err,results){
                          if (err) {
                            console.log(err);
                          }else{
                            console.log(results);
                          }
                          connection.close();
                        });
  }
});
*/