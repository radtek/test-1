var fs= require('fs'); 

function Config(){}
//异步读取文件 
Config.prototype.read = function(callback){
	fs.readFile('config.json','utf-8',function(err,data){ 
	if(err){ console.error(err)
	 }else{ callback && callback(JSON.parse(data).pSpace);
	  }
	});
}
