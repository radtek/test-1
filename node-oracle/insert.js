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
    /*
      两种写法，带返回参数和不带返回参数
      带返回参数方法：
       connection.execute("INSERT INTO NODE (num,name)"+
                        "VALUES(:1,:2) RETURNING num INTO:3",
                        ['003','denghs',new oracle.OutParam()],
    */
    connection.execute("INSERT INTO NODE (num,name)"+
                        "VALUES(:1,:2) ",
                        ['003','denghs'],
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