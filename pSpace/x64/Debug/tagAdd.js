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




var str = "/tag";
/*
console.time('SynAddTag');
for(i=1;i<=100000;i++){
	var tag4 = {
	"propID":["PS_TAG_PROP_NAME","PS_TAG_PROP_TAGTYPE","PS_TAG_PROP_DATATYPE","PS_TAG_PROP_DESCRIPTION"],
	"dataType":["PSDATATYPE_STRING","PSDATATYPE_UINT16","PSDATATYPE_UINT8","PSDATATYPE_STRING"],
	"values":["tag"+i,2,"PSDATATYPE_FLOAT","psNode测试"]
	};
	var resAddSyn4 = con.add(str+i,tag4);
	//if(resAddSyn4 instanceof Err){
		//console.log("添加测点错误:",resAddSyn4.errString);
	//}else{
	//	console.log("添加测点:",resAddSyn4,"成功!");
	//}
}
console.timeEnd('SynAddTag');
//cpu利用率：%50-%60
//执行时间：113352ms
//注释了打印信息：103279ms
//注释打印信息：94251ms,102973
*/

/*
//直接传参
console.time('SynAddTagNoJson');
var tagID =["PS_TAG_PROP_NAME","PS_TAG_PROP_TAGTYPE","PS_TAG_PROP_DATATYPE","PS_TAG_PROP_DESCRIPTION"];
var tagType = ["PSDATATYPE_STRING","PSDATATYPE_UINT16","PSDATATYPE_UINT8","PSDATATYPE_STRING"];
for(i=1;i<=100000;i++){
	var tagValues = ["tag"+i,2,"PSDATATYPE_FLOAT","psNode测试"];
	var resAddSyn= con.add(str+i,tagID,tagType,tagValues);
	//if(resAddSyn instanceof Err){
	//	console.log("添加测点错误:",resAddSyn.errString);
	//}else{
	//	console.log("添加测点:",resAddSyn,"成功!");
	//}
}
console.timeEnd('SynAddTagNoJson');
//115167ms,125383ms
//注释了打印信息：107678ms
*/
/*
console.time("AsyAddTag");
for(i=1;i<=100000;i++){
	var tag4 = {
	"propID":["PS_TAG_PROP_NAME","PS_TAG_PROP_TAGTYPE","PS_TAG_PROP_DATATYPE","PS_TAG_PROP_DESCRIPTION"],
	"dataType":["PSDATATYPE_STRING","PSDATATYPE_UINT16","PSDATATYPE_UINT8","PSDATATYPE_STRING"],
	"values":["tag"+i,2,"PSDATATYPE_FLOAT","psNode测试"]
	};
var resAddAsy = con.add(str+i,tag4,function(err,tagName,tagId){
	//if(err){
	//	console.log(err);
	//}else{
	//	console.log("异步添加测点:",tagName,"ID:",tagId,"成功!");
	//}
});
}
//cpu利用率达到%80-%90,时间:62834ms,65033ms
console.timeEnd("AsyAddTag");
*/
//异步直接传参

console.time("AsyAddNotJson");
var tagAsyID =["PS_TAG_PROP_NAME","PS_TAG_PROP_TAGTYPE","PS_TAG_PROP_DATATYPE","PS_TAG_PROP_DESCRIPTION"];
var tagAsyType = ["PSDATATYPE_STRING","PSDATATYPE_UINT16","PSDATATYPE_UINT8","PSDATATYPE_STRING"];
for(i=1;i<=100000;++i){
	var tagAsyValues = ["tag"+i,2,"PSDATATYPE_FLOAT","psNode测试"];
	var resAddAsy1 = con.add("/tag"+i,tagAsyID,tagAsyType,tagAsyValues,function(err,tagName,tagId){
	//if(err){
	//	console.log(err);
	//}else{
	//	console.log("异步添加测点:",tagName,"ID:",tagId,"成功!");
	//}
});
}
console.timeEnd("AsyAddNotJson");
//cpu利用率达到%80-%95,时间63971ms，64452ms