var csv = require("csv");
var fs = require('fs');
var addon = require('pSpace');
var configure = require('configure');
var ps  = new addon();
//新建连接对象
var con = new Object();
//断线重连的操作
function handleError () {
    ps.open(configure.pSpace,function (err,conn) {
        if (err) {
            console.log('error when connecting to db:', err);
        }else{
           con = conn;
        }
    });
}
//监听数据库是否连接的事件
process.on('isConnect',function(){
	if(!ps.isConnected()){
		handleError();
	}
});
var fun = function()
{
	process.emit('isConnect',null);
}
handleError();
//保存文件状态
var fileLock = [];
//任务
function task (filePath,dirPath) {
	//计算文件第三行的起始位置
	var fileData = fs.readFileSync(filePath, 'ASCII');
	var first = fileData.indexOf('\n');
	var two = fileData.indexOf('\n',first+1);
	csv()
	.from.path(filePath,{
	 columns:true,
	 start:two,
	 encoding: 'ASCII'
	})
	.transform(function(data,index){
	 	 return data;
	})
	.to.array( function(data){
		//获取数据的行数
		var len = data.length;
		//获取所有的字段
		var keys = Object.keys(data[0]);
		//将数据一条一条的插入
		for(var i=0;i<len;i++){
			for(var j=1;j<keys.length;j++){
				var tagName = "/新蚌埠路站/xbbl_"+keys[j]+".his.insertReplace";
				var hisData = {
					"value":data[i][keys[j]],
					"time":new Date(data[i]["SIXLOG Timestamp"])
				};
				var res = con.write(tagName,hisData);
				if(res instanceof ps.Err){
					console.log(res.errString);
					continue;
				}
			}
		}
		//处理完毕，改变文件状态，备份并删除文件
		//截取备份文件文件名
		var p = filePath.lastIndexOf('\/');
        var fileName = filePath.substring(p);
		//要备份的文件夹是否存在
		fs.exists(dirPath+"/backup", function(exits){
			if(!exits){
				//不存在，创建并拷贝
				fs.mkdirSync(dirPath+"\/backup");
				fs.writeFile(dirPath+"\/backup"+"\/"+fileName,fileData, function(err){ 
					if(err){
						console.log("备份文件错误.");
						return;
					}else{
						//删除原文件
						fs.unlink(filePath, function(err) {
							if(err){
								console.log(err);
								return;
							}else{
								//删除文件状态
								delete fileLock[filePath];
							}
						});
					}
				});
               
			}else{
				fs.writeFile(dirPath+"\/backup"+"\/"+fileName,fileData, function(err){ 
					if(err){
						console.log("备份文件错误.");
						return;
					}else{
						//删除原文件
						fs.unlink(filePath, function(err) {
							if(err){
								console.log(err);
								return;
							}else{
								//删除文件状态
								delete fileLock[filePath];
							}
						});
					}
				});

			}
		});
	})
	.on('close', function(count){

	})
	.on('error', function(error){
	  console.log(error.message);
	});	
}

//对给定的文件夹进行处理
function dealWithFile(path)
{
	console.log(path);
	//保存csv文件名
	fs.exists(path,function(exists){
        if(!exists){
        	console.log("指定的文件夹不存在，请重新确定指定路径是否正确!");
        	return;
        }else{
        	//文件夹存在，查看文件夹中是否有文件
        	var files = fs.readdirSync(path);
        	if(0==files.length){
        		//没有文件
        		return;
        	}else{
        		//有文件，查看是否有csv文件
        		for(var i=0;i<files.length;i++){
        			var pointPosition = files[i].lastIndexOf('.');
  			 		var mmieString = files[i].substring(pointPosition+1);
  			 		if(mmieString=="csv"){
  			 			files[i] = path+"\/"+files[i];
  			 			//确保.csv是文件，不是目录
  			 			var stats = fs.statSync(files[i]);
  			 			if(stats.isFile() && fileLock[files[i]]===undefined){
  			 				//设置文件正在被处理
  			 				fileLock[files[i]] = true;
  			 				//直接下至数据task()
  			 						
  			 				task(files[i],path);
  			 			}else{
  			 					//暂时什么也不做
  			 				}
  			 		}else{
  			 			//暂时什么也不做
  			 		}
        		}
        	}

        }
    });
}
//周期检查文件
setInterval(dealWithFile,2000,configure.workPath);
setInterval(fun,1000);

