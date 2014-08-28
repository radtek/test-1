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

var propid = ["tagId","Name"];
var fielter = {"longName":"*采集站1*","querySelf":false,"queryLevel":5,"tagType":1};

var fielter1 = {"longname":"*采集站1*","querySelf":false,"queryLevel":1};
var condition = {
	'propIds':propid,
	'filter':fielter
};

var condition1 = {
	'propIds':propid,
	'filter':fielter
};

var condition2 = {
	'propIds':propid,
	'filter':null
};

var res = con.query("idRoot",condition1);
if (res instanceof Err) {
	console.log(res.errString);
}else{
	console.log("同步查询:");
	console.log(res);
}


/*
var resAsy = con.query("idroot",condition,function (err,val) {
	if (err) {
		console.log(err);
	}else{
		console.log("异步查询:");
		console.log(val);
	}
});

*/

//设置测点属性
var setProps = {"description":"set props1","His_IsSave":1};
var set = con.write("/采集站1/30200003/30200152/HJSH127_25/ZC_ZQLLLJCLZ.props",setProps);
var getProp1 = ["name","LongName"];
var get = con.read("/采集站1/30200003/30200152/HJSH127_25/ZC_ZQLLLJCLZ.props",getProp1);
console.log("get:",get);


var prop = con.read("/采集站1/30200003/30200152/HJSH127_25/ZC_ZQLLLJCLZ.description");
if(prop instanceof Err){
	console.log(prop.errString);
}else{
	console.log(prop);
}

/*
var propRes = con.read("/tag/tag1.description",function(err,val){
	if(err){
		console.log(err);
	}else{
		console.log(val);
	}
});
*/