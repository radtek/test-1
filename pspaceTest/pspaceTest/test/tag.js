/*
 *测点相关接口测试
 */
var addon = require("../lib/pSpace");
var ps = new addon.Client();
var Err = addon.Error;
var con = ps.connect("localhost","admin","admin888");
if(con instanceof Err){
	console.log("错误处理测试1");
}
if(con.constructor == Err){
	console.log("错误处理测试2");
}
if(con.hasOwnProperty("errString"))
{
	console.log("错误码:",con.code);
	console.log("服务器连接失败:",con.errString);
	throw "connect fail!";
}
console.log();

/*
 *同步添加测点
 *参数：
 *	支持pSpace提供的测点属性，测点长名和点类型两个属性必须提供，其他属性可选
 *	测点父节点由第一个参数解析获得，为了增加程序可读性，属性参数采用枚举字符串传递
 *返回值：
 *  成功返回测点长名，失败返回错误对象，对象包含code和errString两个属性。
 *说明：判断是否返回了错误对象有三种方法：
 *	方法一：if(con.hasOwnProperty("errString"))错误表达式的值为true
 *	方法二：if(con.constructor == Err)错误表达式的值返回true
 *	方法三：if(con instaceof Err) 错误表达式返回true
 */
var props = {
	"propID":["PS_TAG_PROP_NAME","PS_TAG_PROP_TAGTYPE","PS_TAG_PROP_DESCRIPTION"],
	"dataType":["PSDATATYPE_STRING","PSDATATYPE_UINT16","PSDATATYPE_STRING"],
	"values":["tag1",0,"psNode测试"]
};
var resAddSyn = con.add("/tag1",props);
if(resAddSyn.constructor == Err)
{
	console.log("错误处理测试1");
}
if(resAddSyn instanceof Err){
	console.log("错误处理测试2");
}
if(resAddSyn.hasOwnProperty("errString")){
	console.log("错误码:",resAddSyn.code);
	console.log("添加测点错误:",resAddSyn.errString);
}else{
	console.log("添加测点:",resAddSyn,"成功!");
}
/*
 *异步添加测点
 *参数：
 *	第一个参数包含的信息包括测点长名，父节点，第二个参数是测点属性
 *	第三个参数是回调函数，回调函数第一个参数是错误信息，第二个参数是测点长名，第三个参数是测点id
 */
 var props1 = {
	"propID":["PS_TAG_PROP_NAME","PS_TAG_PROP_TAGTYPE","PS_TAG_PROP_DESCRIPTION"],
	"dataType":["PSDATATYPE_STRING","PSDATATYPE_UINT16","PSDATATYPE_STRING"],
	"values":["tag2",1,"psNode测试"]
};
 var resAddAsy = con.add("/tag1/tag2",props1,function(err,tagName,tagId){
	if(err){
		console.log(err);
	}else{
		console.log("异步添加测点:",tagName,"ID:",tagId,"成功!");
	}
	
});
/*
 *同步设置测点属性
 *参数：
 *	第一个参数包含需要设置的测点长名和设置标识props,
 *	第二个参数是测点属性，json串有三个关键字：propID，dataType,values
 *	每个关键字的值对象为一个数组
 *	
 */
var setProps = {
	"propID":["PS_TAG_PROP_DESCRIPTION","PS_TAG_PROP_SECURITYAREA"],
	"dataType":["PSDATATYPE_STRING","PSDATATYPE_UINT64"],
	"values":["SetTagProps5",4095]
};
var resSetProps = con.write("/tag2.props",setProps);
if(resSetProps.hasOwnProperty("errString")){
	console.log("错误码:",resSetProps.code);
	console.log("设置测点属性测点错误:",resSetProps.errString);
	con.close();
}else{
	console.log("设置测点属性:",resSetProps,"成功!");
}
/*
 *异步设置测点属性
 *参数：
 *	第一个参数包含需要设置的测点长名和设置标识props,
 *	第二个参数是测点属性，json串有三个关键字：propID，dataType,values
 *	每个关键字的值对象为一个数组
 *	第三个参数是回调函数，会调函数包含err(错误信息)，tagName(测点长名)，tagId(测点ID)
 */
 var resSetPropsAsy = con.write("/tag2.props",setProps,function(err,tagName,tagId){
	if (err) {
		console.log(err);
	}else{
		console.log("异步设置测点属性成功:",tagName,"ID:",tagId);
	}
});
/*
 *同步获取测点属性
 *参数：
 *	第一个参数包含需要获取的测点长名和设置标识props,
 *	第二个参数是要获取的测点属性，json串有两个关键字：propID，dataType
 *返回值：
 *成功返回对应的属性值对象
 */
var getProps = {
	"propID":["PS_TAG_PROP_NAME","PS_TAG_PROP_TAGTYPE"],
	"dataType":["PSDATATYPE_STRING","PSDATATYPE_UINT16"],
};
var props = con.read("/tag.props",getProps);
if(props.hasOwnProperty("errString")){
	console.log(props.code);
	console.log(props.errString);
}else{
	console.log("同步获取属性值:",props);
}

/*
 *异步获取测点属性
 *参数：
 *	第一个参数包含需要获取的测点长名和设置标识props,
 *	第二个参数是要获取的测点属性，json串有两个关键字：propID，dataType
 *	第三个参数是回调函数，函数参数包含err(错误信息),tagName(测点长名)，props(测点属性值)
 */
var resGetProps = con.read("/tag.props",getProps,function(err,tagName,props){
	if(err){
		console.log(err);
	}else{
		console.log("异步获取",tagName,"属性值:",props);
	}
});

/*
  *同步删除测点
  *参数
  *	包含了所要删除的测点长名以及父节点，格式为：/node/tag
  *返回值：
  *	成功返回测点长名
  */
 var resDelSyn = con.del("/tag1/tag2");
if(resDelSyn.hasOwnProperty("errString")){
	console.log("错误码:",resDelSyn.code);
	console.log("错误信息:",resDelSyn.errString);
}else{
	console.log("删除测点:",resDelSyn,"成功!");
}
/*
  *异步删除测点
  *参数：
  *	第一个参数包含了所要删除的测点长名以及父节点，格式为：/node/tag
  *	第二个参数是回调函数，回调函数第一个参数是错误信息，第二个参数是tagName
  *返回值：
  *	成功返回测点长名
  */
var resDelAsy = con.del("/tag1",function(err,tagName){
	if(err){
		console.log(err);
	}else{
		console.log("异步删除测点:",tagName,"成功!");
	}
});

module.exports.close = function(){
	con.close();
};
