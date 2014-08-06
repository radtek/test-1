//var server = require('./server.js');
var io = require('./node_modules/socket.io/node_modules/socket.io-client');


var socket = io.connect('http://localhost:3000');


//使用字典作为cache,字典key是测点长名
var dataCache = {};

module.exports = DataSource;
//DataSource对象
function DataSource(){

}
socket.on("new_data",function(data){
	if(data){
		for(var i=0;i<data.length;i++){
			dataCache[data[i].name] = data[i].value;
			dataCache[data[i].name].state = true;
		}
	}
});

socket.on('onDataChanged',function(data){
		if(data){
			//to do user
			console.log(data);
		}
});

socket.on('message',function(msg){
		console.log(msg);
	});


function sub(tagNameList, callback){
	if(0!=tagNameList.length){
		socket.emit('sub',tagNameList,function(data){
			if(null!=data){
				for(var i=0;i<data.length;i++){
					dataCache[data[i].name] = data[i].value;
					dataCache[data[i].name].state = false;
				}
				//console.log(dataCache);
				callback && callback(data);
			}
		});
	}
		
}

//DataSource read方法
DataSource.prototype.read = function(args,callback){
	if(0===args.length){
		callback("参数错误",undefined);
	}
	var nameList = [];
	var dataObj = [];
	for(var i=0;i<args.length;i++){
		var val = {};
		if(dataCache.hasOwnProperty(args[i]) && dataCache[args[i]].state===true){
				val.name = args[i];
				val.value = dataCache[args[i]];
				dataObj[i]= val;
				dataObj[i].value.time = new Date(dataObj[i].value.time);
				delete dataObj[i].value.state;
		}else{
			nameList.push(args[i]);
		}
	}
	if(0===nameList.length){
		callback &callback(dataObj);
	}else{
		sub(nameList, function(data){
			dataObj = dataObj.concat(data);
			for(var i=0;i<dataObj.length;i++){
				delete dataObj[i].value.state;
				dataObj[i].value.time = new Date(dataObj[i].value.time);
			}
			callback && callback(dataObj);
		});
	}
}

//DataSource write方法
DataSource.prototype.write = function(args){
	if(args instanceof Object){
			socket.emit("write",args);
	}else{
		return "参数错误!";
	}
}




var da = new DataSource();
var tagName = ["/tag5/tag6","/tag5/tag9"];
da.read(tagName,function(err,data){
	if(err){
		console.log(err);
	}else{
		console.log(data);
	}
});

var writeJson = {name : "/tag5/tag9", value:{value :55.58,time:new Date(),quality:"good"}};
da.write(writeJson);

socket.on('reconnecting',function(){
	console.log("reconnecting");
});
socket.on('reconnect',function(){
	sub(tagName,function(data){
		console.log(data);
	});
});


