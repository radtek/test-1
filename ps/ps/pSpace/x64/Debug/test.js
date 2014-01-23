var addon = require("./pSpace");
var ps = new addon.Client();

var connectDate = {
	"hostname":"localhost",
	"user":"admin",
	"password":"admin888",
	"port":8889
};
//同步断开与连接
var con = ps.connect(connectDate);
if(con.hasOwnProperty("errString"))
{
	console.log("错误码:",con.code);
	console.log("服务器连接失败:",con.errString);
	throw "connect fail!";
}
//判断是否连接，如果连接返回true,w未连接返回false
if (con.isConnected()) {
		console.log("con 已经连接！");
}else{
		console.log("con 已经断开连接！");
	 }
 //断开测试
	 /*
//con.close();
if (con.isConnected()) {
		console.log("con 已经连接！");
}else{
		console.log("con 已经断开连接！");
	 }
*/
/*
//异步断开与连接操作测试
var res = ps.connect(connectDate,function(err,con1){
	if(err)
	{
		console.log(err);
	}
	else
	{
		if (con1.isConnected()) {
			console.log("con1 已经连接！");
		}
		con1.close();
		if (con1.isConnected()) {
			console.log("con1 已经连接！");
		}
		else
		{
			console.log("con1 已经断开连接！");
		}
	}
	
});
*/
/*
//同步读取实时值测试
var pv = con.read("/tag.pv");
if(pv.hasOwnProperty("errString")){
	console.log(pv.code);
	console.log(pv.errString);
	con.close();
}else{
	console.log(pv);
	console.log(pv.value);
	console.log(pv.quality);
	console.log(pv.time);
}

//异步读取实时值测试
var r = con.read("/tag.pv",function(err,pv){
	if(err){
		console.log(err);
		con.close();
	}else{
		console.log(pv);
		console.log(pv.value);
	}
});
*/

var writeData  = {
	"value":67.001,
	"quality":0,
	"time":"2015-01-21 00:20:0.123"
};

/*
//同步写实时测试
var res = con.write("/tag.pv",writeData);
if(res.hasOwnProperty("errString")){
	console.log(res.code);
	console.log(res.errString);
	con.close();
}else{
	console.log(res,":写实时成功");
}
*/
/*
//异步写实时测试
var resAsy = con.write("/tag.pv",writeData, function(err,tagName){
	if(err){
		console.log(err);
		con.close();
	}else{
		console.log(tagName,":异步写实时成功");
	}
});
*/

/*
// 异步新建订阅测点实时值变化并得到初值测试,此函数并不会阻塞整个程序
var resSub = con.sub("/tag.real",function(err,tagName,val){
	if(err){
		console.log(err);
		con.close();
	}else
	{
		console.log(val.value);
	}
});
*/
/*
//异步测点属性订阅测试,两个订阅同时执行存在问题
var resProps = con.sub("/tag.props",function(err,tagName,val){
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

var props = {
	"propID":["PS_TAG_PROP_NAME","PS_TAG_PROP_TAGTYPE","PS_TAG_PROP_DESCRIPTION"],
	"dataType":["PSDATATYPE_STRING","PSDATATYPE_UINT16","PSDATATYPE_STRING"],
	"values":["tag2",0,"psNode测试"]
};

/*
//同步添加测点
var resAddSyn = con.add("/tag1",props);
if(resAddSyn.hasOwnProperty("errString")){
	console.log("错误码:",resAddSyn.code);
	console.log("添加测点错误:",resAddSyn.errString);
	con.close();
}else{
	console.log("添加测点:",resAddSyn,"成功!");
}
*/

/*
//异步添加测点
var resAddAsy = con.add("/tag1",props,function(err,tagName,tagId){
	if(err){
		console.log(err);
		con.close();
	}else{
		console.log("异步添加测点:",tagName,"ID:",tagId,"成功!");
	}
	
});
*/
/*
//同步删除测点测试
var resDelSyn = con.del("/tag1/tag2");
if(resDelSyn.hasOwnProperty("errString")){
	console.log("错误码:",resDelSyn.code);
	console.log("错误信息:",resDelSyn.errString);
}else{
	console.log("删除测点:",resDelSyn,"成功!");
}
*/
/*
//异步删除测点测试
var resDelAsy = con.del("/tag1",function(err,tagName){
	if(err){
		console.log(err);
	}else{
		console.log("异步删除测点:",tagName,"成功!");
	}
});

*/

var setProps = {
	"propID":["PS_TAG_PROP_DESCRIPTION","PS_TAG_PROP_SECURITYAREA"],
	"dataType":["PSDATATYPE_STRING","PSDATATYPE_UINT64"],
	"values":["SetTagProps5",4095]
};

/*
var resSetProps = con.write("/tag2.props",setProps);
if(resSetProps.hasOwnProperty("errString")){
	console.log("错误码:",resSetProps.code);
	console.log("设置测点属性测点错误:",resSetProps.errString);
	con.close();
}else{
	console.log("设置测点属性:",resSetProps,"成功!");
}
*/

/*
var resSetPropsAsy = con.write("/tag2.props",setProps,function(err,tagName,tagId){
	if (err) {
		console.log(err);
	}else{
		console.log("异步设置测点属性成功:",tagName,"ID:",tagId);
	}
});
*/

//同步获取测点属性测试
var getProps = {
	"propID":["PS_TAG_PROP_NAME","PS_TAG_PROP_TAGTYPE"],
	"dataType":["PSDATATYPE_STRING","PSDATATYPE_UINT16"],
};

/*
var props = con.read("/tag.props",getProps);
if(props.hasOwnProperty("errString")){
	console.log(props.code);
	console.log(props.errString);
}else{
	console.log("同步获取属性值:",props);
}

//异步获取测点属性成功
var resGetProps = con.read("/tag.props",getProps,function(err,tagName,props){
	if(err){
		console.log(err);
	}else{
		console.log("异步获取",tagName,"属性值:",props);
	}
});
*/
var histry = {
	"value":12.001,
	"time":"2008-05-13 14:28:04.123",
	"quality":"PS_QUALITY_RAW"
};

/*
//同步插入某一测点历史数据测试
var resInsert = con.write("/tag.his.insert",histry);
if(resInsert.hasOwnProperty("errString")){
	console.log(resInsert.code);
	console.log(resInsert.errString);
	//con.close();
}else{
	console.log("对测点:",resInsert,"插入历史数据成功!");
}

//异步插入某一测点的历史数据测试
var resInsertAsy = con.write("/tag.his.insert",histry,function(err,tagName,tagId){
	if(err){
		console.log(err);
	}else{
		console.log("异步插入测点:",tagName,"历史数据成功.");
	}
});


////同步插入替换某一测点历史数据
var resInsertRep = con.write("/tag.his.insertReplace",histry);
if(resInsertRep.hasOwnProperty("errString")){
	console.log(resInsertRep.code);
	console.log(resInsertRep.errString);
	//con.close();
}else{
	console.log("对测点:",resInsertRep,"插入替换历史数据成功!");
}


//异步插入替换某一测点的历史数据测试
var resInsertRepAsy = con.write("/tag.his.insert",histry,function(err,tagName,tagId){
	if(err){
		console.log(err);
	}else{
		console.log("异步插入替换测点:",tagName,"历史数据成功.");
	}
});


//同步替换某一测点历史数据
var resRep = con.write("/tag.his.replace",histry);
if(resRep.hasOwnProperty("errString")){
	console.log(resRep.errString);
}else{
	console.log("同步替换测点:",resRep,"成功!");
}

//异步替换某一测点历史数据
var resRepAsy = con.write("/tag.his.replace",histry,function(err,tagName,tagId){
	if(err){
		console.log(err);
	}else{
		console.log("异步替换测点:",tagName,"历史数据成功.");
	}
});

*/