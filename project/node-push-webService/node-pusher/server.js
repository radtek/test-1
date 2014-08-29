var addon = require('pSpace');
var async = require('async');
var tag = require('tag');
var express = require('express');
var path = require('path');
var url = require('url');
var rw = require('configure');
var config = require("./configure/serverConfig.json");
var EventEmitter = require('events').EventEmitter; 
var event = new EventEmitter(); 

var ps = new addon();
var app = express();
app.set('port', process.env.PORT || config.pusher.port);
app.set("ipaddr", config.pusher.host);
var server = app.listen(app.get('port'),app.get('ipaddr'));
var io = require('socket.io').listen(server);


// 设定views变量，意为视图存放的目录
app.set("views", __dirname + "/views");

//指定静态文件的目录 
app.use(express.static('public'));
app.use(express.static(path.join(__dirname, 'syalias')));

//重定向配置文件(read,write)
app.all('/*.(read|write){1}', function(req, res, next){
	var pathname = url.parse(req.url).pathname;
  var pointPosition = pathname.lastIndexOf('.');
  var mmieString = pathname.substring(pointPosition+1);
  var realPath = __dirname + '/syalias' + url.parse(req.url).pathname;
  if(pathname=='/favicon.icon')
      return;
  else if(pathname=='/' || pathname=='/index')
    next();
  else if('read'===mmieString)
    rw.dealWithConfig(pathname,realPath,res);
  else if('write'===mmieString)
     rw.writeConfig(pathname,realPath,req,res);
  else
    next();
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


