var oracle =  require("oracle");

var connectData = { 'hostname':'localhost',
					'database':'xe',
					'user':'system',
					'password':'yinguangfei'

};
//连接数据库并操作
oracle.connect(connectData,function(err,connection){
	if(err)
		console.log(err);
	else{
		//执行DDL语句
		connection.execute("ALTER TABLE NODE ADD (sex INT)",[],function(err,results){
			if (err)
				console.log(err);
			else
				console.log(results);
			//关闭数据库
			connection.close();
		});
	}
});