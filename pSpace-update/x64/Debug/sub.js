var addon = require("./pSpace");
//var add = require('../lib/tag.js');
var ps = new addon.Client();
var Err = addon.Error;
var con = ps.connect("localhost","admin","admin888");
if(con instanceof Err){
	console.log("错误码:",con.code);
	console.log("服务器连接失败:",con.errString);
	throw "connect fail!";
}else{
	console.log("连接成功!");
}
var tag = ["/tag5/tag6.props","/tag/tag1.props"];
var tag1 = ["/tag/tag1"];
var tag2 = ["/系统变量/时间变量/date"];
var ID;
var ID1;

	var resSub = con.sub(tag2,function(err,subid,curData,data){
	if(err){
		console.log(err);
	}else if(curData){
		ID = subid;
		console.log("subid:",subid);
	}else{
		console.log("1111111111111111111111:");
		console.log(data);
	}
	});

/*
	var resSub1 = con.sub(tag1,function(err,subid,curData,data){
	if(err){
		console.log(err);
	}else if(curData){
		ID1 = subid;
		console.log("subid2:",subid);
	}else{
		console.log("2222222222222222222:");
		console.log(data);
	}
	});

function del(){
	//con.del(ID1);
	con.del(ID1);
	console.log("已删除!");
}

setTimeout(del,5000);
*/
/*
var prop = con.sub(tag,function(err,subid,value){
	if(err){
		console.log(err);
	}else{
		console.log(subid);
		console.log(value);
	}
});
*/
/*
var tag1 = ["/tag5/tag6"];
var resSub = con.sub(tag1,function(err,subid,curData,data){
if(err){
	console.log(err);
}else if(curData){
	console.log("subid1:",subid);
}else{
	console.log("subid1:",subid);
	

}
});
*/
/*
var resSub1 = con.sub("/tag5/tag9.real",function(err,tagName,val){
if(err){
	console.log(err);
}else
{
	console.log(tagName);
}
});
*/
/*
 *测点属性订阅测试
 *参数:
 *	/tag:测点长名，props:订阅标识
 *	第二个参数是一个回调函数，回调函数第一个参数是错误信息
 *  如果执行成功，这个参数是undefined,第二个参数是测点长名
 *  第三个参数是订阅到的属性值对象，包含四个属性：
 *  id(测点ID),count(测点属性变化数量)，type(订阅变化类型)，value(属性值)
 */
 /*
 var resProps = con.sub("/tag/tag1.props",function(err,tagName,val){
	if (err) {
		console.log(err);
		con.close();
	}else{
		console.log("订阅测点:",tagName,"成功!");
		console.log("点ID:",val.id);
		console.log("订阅属性变化类型",val.type,"变化属性数量:",val.count);
		console.log("属性值:",val.value);
	}
});

*/