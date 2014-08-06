var addon = require("./pSpace");
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
console.log();

var prop ={"tagType":"node"};

var prop1 = new Object;
prop1.tagType = "analog";
prop1.DataType = "double";
prop1.description = "测试";

var prop2 = {"tagType":"analog","DataType":"FLOAT"};

var prop3 = {"tagType":"digital",};

var prop4 = {"tagType":"string","DataType":"STRING"};

var addSy1 = con.add("/tag",prop);
if(addSy1 instanceof Err){
	console.log("添加测点错误:",addSy1.errString);
}else{
	console.log("添加测点:",addSy1,"成功!");
}

var addSy2 = con.add("/tag/tag1",prop1);
if(addSy2.constructor==Err){
	console.log("添加测点错误:",addSy2.errString);
}else{
	console.log("添加测点:",addSy2,"成功!");
}

var addSy3 = con.add("/tag/tag2",prop2);
if(addSy3.constructor==Err){
	console.log("添加测点错误:",addSy3.errString);
}else{
	console.log("添加测点:",addSy3,"成功!");
}

var addSy4 = con.add("/tag/tag3",prop3);
if(addSy4 instanceof Err){
	console.log("添加测点错误:",addSy4.errString);
}else{
	console.log("添加测点:",addSy4,"成功!");
}

var addSy5 = con.add("/tag/tag4",prop4);
if(addSy5 instanceof Err){
	console.log("添加测点错误:",addSy5.errString);
}else{
	console.log("添加测点:",addSy5,"成功!");
}
//异步添加测点
var addAsy = con.add("/tag5",prop,function(err,tagName){
	if(err){
		console.log(err);
	}else{
		console.log("异步添加测点",tagName,"成功!");
	}
});
var addAsy1 = con.add("/tag5/tag6",prop1,function(err,tagName){
	if(err){
		console.log(err);
	}else{
		console.log("异步添加测点",tagName,"成功!");
	}
});
var addAsy1 = con.add("/tag5/tag7",prop2,function(err,tagName){
	if(err){
		console.log(err);
	}else{
		console.log("异步添加测点",tagName,"成功!");
	}
});
var addAsy1 = con.add("/tag5/tag8",prop3,function(err,tagName){
	if(err){
		console.log(err);
	}else{
		console.log("异步添加测点",tagName,"成功!");
	}
});
var addAsy1 = con.add("/tag5/tag9",prop4,function(err,tagName){
	if(err){
		console.log(err);
	}else{
		console.log("异步添加测点",tagName,"成功!");
	}
});

/*
 *同步设置测点属性
 *参数：支持两种方式参数传递
 *	第一个参数包含需要设置的测点长名和设置标识props,
 *	第二个参数是测点属性，json串有三个关键字：propID，dataType,values
 *	每个关键字的值对象为一个数组
 *	
 *异步设置测点属性
 *参数：支持两种参数传递方式
 *	第一个参数包含需要设置的测点长名和设置标识props,
 *	第二个参数是测点属性，json串有三个关键字：propID，dataType,values
 *	每个关键字的值对象为一个数组
 *	第三个参数是回调函数，会调函数包含err(错误信息)，tagName(测点长名)，tagId(测点ID)
 */
 
var setProps = {"description":"pSpaceNode测试","His_IsSave":1};

var setProps1 = new Object();
setProps1.description = "setProps1";
setProps1.SecurityArea = 4095;
setProps1.His_IsSave = 1;

var setProps2 = {"EnableAlarm":1,"EnableValueAlarm":1};

//同步设置测点属性
var setSyn = con.write("/tag/tag1.props",setProps);
if(setSyn instanceof Err){
	console.log(setSyn.errString);
}else{
	console.log("同步设置测点",setSyn,"属性成功!");
}
var setSyn1 = con.write("/tag/tag2.props",setProps1);
if(setSyn1 instanceof Err){
	console.log(setSyn1.errString);
}else{
	console.log("同步设置测点",setSyn1,"属性成功!");
}
var setSyn2 = con.write("/tag/tag3.props",setProps2);
if(setSyn2 instanceof Err){
	console.log(setSyn2.errString);
}else{
	console.log("同步设置测点",setSyn2,"属性成功!");
}
//异步设置测点属性
var setAsy = con.write("/tag/tag4.props",setProps,function(err,tagName,tagID){
	if(err){
		console.log(err);
	}else{
		console.log("异步设置测点",tagName,"属性成功");
	}
});

var setAsy1 = con.write("/tag5/tag6.props",setProps1,function(err,tagName,tagID){
	if(err){
		console.log(err);
	}else{
		console.log("异步设置测点",tagName,"属性成功");
	}
});

var setAsy2 = con.write("/tag5/tag7.props",setProps2,function(err,tagName,tagID){
	if(err){
		console.log(err);
	}else{
		console.log("异步设置测点",tagName,"属性成功");
	}
});



/*
 *同步获取测点属性
 */
 
var getProp = ["name","TagType","SecurityArea","ParentId"];
var getProp1 = ["name","TagID"];

var getSyn = con.read("/tag/tag2.props",getProp);
if(getSyn instanceof Err){
	console.log(getSyn.code);
	console.log(getSyn.errString);
}else{
	console.log("同步获取属性值:",getSyn);
	console.log(getSyn.name);
	console.log(getSyn.TagType);
	console.log(getSyn.SecurityArea);
	console.log(getSyn.ParentId);
}

var getSyn1 = con.read("/tag/tag3.props",getProp);
if(getSyn instanceof Err){
	console.log(getSyn.code);
	console.log(getSyn.errString);
}else{
	console.log("同步获取属性值:",getSyn);
}
//异步获取测点属性
var getAsy = con.read("/tag5/tag6.props",getProp,function(err,tagName,prop){
	if(err){
		console.log(err);
	}else{
		console.log("异步获取",tagName,"属性值成功");
		console.log(prop);
		console.log(prop.name);
		console.log(prop.TagType);
		console.log(prop.SecurityArea);
	}
});

//异步获取测点属性
var getAsy = con.read("/tag5/tag7.props",getProp1,function(err,tagName,prop){
	if(err){
		console.log(err);
	}else{
		console.log("异步获取",tagName,"属性值成功");
		console.log(prop);
	}
});

/*
  *同步删除测点
  *参数
  *	包含了所要删除的测点长名以及父节点，格式为：/node/tag
  *返回值：
  *	成功返回测点长名
  *
  *异步删除测点
  *参数：
  *第一个参数包含了所要删除的测点长名以及父节点，格式为：/node/tag
  *第二个参数是回调函数，回调函数第一个参数是错误信息，第二个参数是tagName
  *返回值：
  *成功返回测点长名
  */
  //同步
  /*
var resDelSyn = con.del("/tag/tag2");
if(resDelSyn instanceof Err){
	console.log("错误码:",resDelSyn.code);
	console.log("错误信息:",resDelSyn.errString);
}else{
	console.log("删除测点:",resDelSyn,"成功!");
}
sleep(2000);
//异步
var resDelAsy = con.del("/tag",function(err,tagName){
	if(err){
		console.log(err);
	}else{
		console.log("异步删除测点:",tagName,"成功!");
	}
});

*/