var addon = require('pSpace');
var async = require('async');
var tag = require('tag');
var express = require('express');
var path = require('path');
var config = require("./configure/serverConfig.json");
var EventEmitter = require('events').EventEmitter; 
var event = new EventEmitter(); 

var ps = new addon();

var app = express();
app.set('port', process.env.PORT || config.pusher.port);
app.set("ipaddr", config.pusher.host);
var server = app.listen(app.get('port'),app.get('ipaddr'));
var io = require('socket.io').listen(server);


/*
//路由处理
var router = express.Router();
app.use('/', router);
*/
/*
router.use(function(req, res, next) {
	
    console.log(req.method, req.url);
    next();  
});
*/

/*
router.get('/', function(req, res) {
    res.sendfile(__dirname + '/index.html');   
 
});
*/
/*
//处理get请求，资源定位与响应 
app.get('/', function (req, res) {
 	res.sendfile(__dirname + '/index.html');

});
*/
/*
//test页面测试
app.get('/test',function(req,res){
	res.sendfile(__dirname + '/test.html');
});
*/
/*
/// catch 404 and forward to error handler
app.use(function(req, res, next) {
    var err = new Error('Not Found');
    err.status = 404;
    next(err);
});
*/
// 设定views变量，意为视图存放的目录
app.set("views", __dirname + "/views");

//指定静态文件的目录 
app.use(express.static('public'));
app.use(express.static(path.join(__dirname, 'syalias')));
//配置文件相关处理

function dealWithFile(pathname,res)
{
	fs.readdir(pathname,function(err,files){
		if(err){
			res.writeHead(404, {'Content-Type': 'text/plain'}); 
            res.write("This request URL was not found on this server."); 
            res.end();
		}else{
			var data = [];
			files.forEach(function(item){
				var tmpPath = pathname + '/' + item; 
				 var pointPosition = tmpPath.lastIndexOf('.');
  				 var mmieString = tmpPath.substring(pointPosition+1);
  				 if('json'===mmieString){
  				 	//data.push(fs.readFileSync(tmpPath));
  				 	var tmpData = fs.readFileSync(tmpPath);
  				 	if(tmpData.length>0){
  				 		var jsObj = JSON.parse(tmpData);
						data.push(jsObj);
  				 	}
  				 }
			});
			res.send(data);
      res.statusCode = 200;
      //res.end();  
		}
	});

}

function writeConfig(pathname,realPath,req,res)
{
	realPath = realPath .replace(/\\/g,"\/");
    var pointPosition = pathname.lastIndexOf('.');
    var mmieString = pathname.substring(pointPosition+1);
    realPath = realPath.replace(/.write/g,".json");
    //console.log("realPath:",realPath);
    //查看是否有这个json文件，没有就创建
    fs.exists(realPath,function(exists){
    	if(!exists){
    		var position = realPath.lastIndexOf('\/');
            var filePath = realPath.substring(0,position);
            fs.exists(realPath,function(exists){
            	if(!exists){
            		//如果文件夹所在的目录存在，创建文件夹
                var p1 = filePath.lastIndexOf('\/');
                var upFilePath = filePath.substring(0,p1);
                fs.exists(upFilePath,function(exist){
                  if(!exist){
                    res.statusCode = 405;
                    res.end();
                    console.log("write fail");
                  }else{
                    fs.mkdirSync(filePath);
                    fs.writeFile(realPath, JSON.stringify(req.query), function(err){  
                      if(err){
                        res.statusCode = 405;
                        res.end();
                        console.log("write fail");
                      }else{
                         //res.statusCode = 200;
                         res.end();  
                      }  
                          
                   });  
                  }
                });
            		
            	}else{
            		fs.writeFile(realPath, JSON.stringify(req.query), function(err){  
				        if(err){
				        	res.statusCode = 405;
				        	res.end();
				        	console.log("write fail ");
				        }else{
                  res.statusCode = 200;
                  res.end();  
                }  
			           		 
		    		});  
            	}
            });
    		
    	}else{
    		var tmpData = fs.readFileSync(realPath);
    		var reqData = req.query;
    		var jsObj = JSON.parse(tmpData);
    		if(tmpData.length>0){
    			var keys = Object.keys(reqData);
    			for(var i=0;i<keys.length;i++){
    				jsObj[keys[i]] = reqData[keys[i]];
    			}
    			fs.writeFile(realPath,JSON.stringify(jsObj),function(err){
        			if(err){
                res.statusCode = 405;
                res.end();
              }else{
                res.statusCode = 200;
                res.end();  
              }

    			});
    		}else{
    			fs.writeFile(realPath,JSON.stringify(reqData),function(err){
        			if(err){
                res.statusCode = 405;
                res.end();
              }else{
                res.statusCode = 200;
                res.end(); 
              }
        			

    			});
    			//fs.writeSync(realPath,reqData);
    			
    		}
    		
    	}
    });

}

function dealWithConfig(pathname,realPath,res)
{
    realPath = realPath .replace(/\\/g,"\/");
    var pointPosition = pathname.lastIndexOf('.');
    var mmieString = pathname.substring(pointPosition+1);
    realPath = realPath.replace(/.read/g,".json");
    //先查看文件是否为一个json配置文件
    fs.exists(realPath,function(exists){
        if(!exists){
        	//请求的url不是一个json文件，那么判断是否存在这样一个请求的文件夹
        	var position = realPath.lastIndexOf('.');
        	var p = realPath.substring(0,position);//file/config
        	fs.exists(p,function(exists){
        		if(!exists){
        			//文件夹不存在
              res.statusCode = 404;
        			res.writeHead(404, {'Content-Type': 'text/plain'}); 
           		    res.write("This request URL " + pathname + " was not found on this server."); 
           		    res.end();
        		}else{
        			var stat = fs.statSync(p);
        			if(stat.isDirectory()&&'read'===mmieString){
        				//对文件夹处理并返回数据
        				dealWithFile(p,res);
        			}else{
        				//错误
                res.statusCode = 404;
        				res.writeHead(404,{'Content-Type': 'text/plain'}); 
           		  res.write("This request URL was not found on this server."); 
           		  res.end();
        			}

        		}
        	});
        }else{
        	
        	var stat = fs.statSync(realPath);
          	//请求的是是否一个json文件还是文件夹
          	if(stat.isFile()){
          		//判断是否为读数据
          		if('read'===mmieString){
          			//对文件处理并返回数据
          			var tmpData = fs.readFileSync(realPath);
          			if(tmpData.length>0){
          				res.send(fs.readFileSync(realPath));
                  res.statusCode = 200;
          			}else{
          				//请求错误
          				res.statusCode = 501;
          				res.end();
          			}
          			
          		}else{
          			//错误
                res.statusCode = 404;
          			res.writeHead(404, {'Content-Type': 'text/plain'}); 
           		  res.write("This request  was not found on this server."); 
           		  res.end();
          		}
          		
          	}else if(stat.isDirectory() && 'read'===mmieString){
          		//对文件夹处理并返回数据
          		dealWithFile(realPath,res);
          	}else{
          		//错误
              res.statusCode = 404;
          		res.writeHead(404, {'Content-Type': 'text/plain'}); 
           		res.write("This request  was not found on this server."); 
           		res.end();
          	}
        }     
    });

}

//重定向配置文件(read)
app.all('/*.(read){1}', function(req, res, next){
	var pathname = url.parse(req.url).pathname;
    var realPath = __dirname + '/syalias' + url.parse(req.url).pathname;
    if(pathname=='/favicon.icon')
        return;
    else if(pathname=='/' || pathname=='/index')
       	next();
    else
        dealWithConfig(pathname,realPath,res);
});

//重定向配置文件(read)
app.all('/*.(write){1}', function(req, res, next){
	var pathname = url.parse(req.url).pathname;
    var realPath = __dirname + '/syalias' + url.parse(req.url).pathname;
    if(pathname=='/favicon.icon')
        return;
    else if(pathname=='/' || pathname=='/index')
       	next();
    else
        writeConfig(pathname,realPath,req,res);
});


//新建连接对象
var con = new Object();
//存放连接到服务器的用户
var conns = {};
//存放订阅任务,任务id就是连接之后的sockeid,对应着本次连接之后的订阅任务
var taskList = {};
//断线重连的操作
function handleError () {
    ps.open(config.pSpace,function (err,conn) {
        if (err) {
            console.log('error when connecting to db:', err);
        }else{
           con = conn;
           var userKeys = Object.keys(conns);
           var taskKeys = Object.keys(taskList);
           if(userKeys.length!=0 && taskList.length!=0){
           	for(var i=0;i<taskKeys.length;i++){
           		(function(idx){
           			var soc = conns[taskKeys[idx]]; 
           			con.sub(taskList[taskKeys[idx]],function(err,subid,curVal,value){
						if(err){
							con.close();
							handleError();
						}else if(value){ 
	                    	soc.emit("new_data",value);
							soc.emit("onDataChanged",value);  
						}
					});	
           		})(i);	
           	}
           }
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
//监听客户端连接事件 
io.sockets.on('connection', function (socket) {
	var cid = socket.id;
	//console.log("cid:",cid);
	conns[cid] = socket;
	//保存subID
	var subID = -1;
	socket.on('sub',function(longNameList,fn){
	  	if(0==longNameList.length){
			return undefined;
		}else{
			//订阅
			con.sub(longNameList,function(err,subid,curVal,value){
				if(err){
					fn(err.toString(),undefined)
				}else if(curVal){
					subID = subid;
					fn(null,curVal);
				}else if(value){
					socket.emit("new_data",value);
					socket.emit("onDataChanged",value);
				}
			});
			//将任务放到任务表中
			taskList[cid] = [];
			taskList[cid] = taskList[cid].concat(longNameList);

		}
		//注册写实时事件
		socket.on("write",function(args){
			args.value.time = new Date(args.value.time);
			var res =  con.write(args.name+".pv",args.value);
			if(res instanceof ps.Err){
				return false;
			}else{
				return res;
			}
		});
		//断开连接处理
		socket.on('disconnect', function() {
			//console.log("disconnect");
			con.del(subID);
			delete taskList[cid];
			delete conns[cid];
		});
  });

});

 //每隔一秒检查一次数据库是否已经连接
 setInterval(fun,1000);


