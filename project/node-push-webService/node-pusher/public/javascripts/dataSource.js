
//使用字典作为cache,字典key是测点长名
var dataCache = {};

//DataSource对象
function DataSource(){

}

function sub(tagNameList, callback){
	if(0!=tagNameList.length){
		socket.emit('sub',tagNameList,function(err,data){
			if(err){
				callback && callback(err,undefined);
			}else{
				for(var i=0;i<data.length;i++){
					dataCache[data[i].name] = data[i].value;
					dataCache[data[i].name].state = false;
					delete data[i].value.state;
					//data[i].value.time = new Date(data[i].value.time);
				}
				callback && callback(null,data);
				
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
				//dataObj[i].value.time = new Date(dataObj[i].value.time);
				delete dataObj[i].value.state;
		}else{
			nameList.push(args[i]);
		}
	}
	if(0===nameList.length){
		callback &callback(null,dataObj);
	}else{
		sub(nameList, function(err,data){
			if(err){
				callback && callback(err,undefined);
			}else{
				dataObj = dataObj.concat(data);
				callback && callback(null,dataObj);
			}
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



