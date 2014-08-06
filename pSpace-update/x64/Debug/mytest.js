var conn = require("./pSpace");
var ps = new conn.Client();
var Err=conn.Error;
var con = ps.connect("localhost","admin","admin888");
if ( con instanceof Err)
{
	console.log("错误码",con.code);
	console.log(con.errString);
}else
{
	console.log("连接成功");
}
//process.stdin.resume();

var prop = {"tagType":"node"};
var prop1 = new Object;
prop1.tagType = "analog";
prop1.DataType = "double"

var LonN = con.add("/tag",prop);
if ( LonN instanceof Err )
{
	console.log("添加测点错误：",LonN.errString );
}
else{
	console.log("添加测点",LonN,"成功！");
}
var longn1 = con.add("/tag/tag1",prop1);
if ( longn1 instanceof Err )
{
	console.log("添加测点错误：",longn1.errString);
}else {
	console.log("添加测点:",longn1,"成功！");
}


var setprop = new Object;
setprop.description = "node";
//setprop.tagname = "ntag";

var setprop1 = {"description":"analog"};

var ressetprop = con.write("/tag.props",setprop);
if(ressetprop.hasOwnProperty("errString"))
{
	console.log("错误码:",ressetprop.code);
	console.log("设置测点属性测点错误:",ressetprop.errString);
}else
{
	console.log("设置测点属性:",ressetprop,"成功!");
}
var ressetprop1 = con.write("/tag/tag1.props",setprop1);
if(ressetprop1.hasOwnProperty("errString"))
{
	console.log("错误码:",ressetprop1.code);
	console.log("设置测点属性测点错误:",ressetprop1.errString);
}else
{
	console.log("设置测点属性:",ressetprop1,"成功!");
}

var getprop = ["name","TagType","description"];
var getp = con.read("/tag/tag1.props",getprop);
if(getp instanceof Err)
{
	console.log("错误码：",getp.code);
	console.log("获取测点属性错误",getp.errString);
}
else
{
	console.log("获取测点属性",getp,"成功！");
	console.log(getp.name);
	console.log(getp.TagType);
	//console.log(getp.id);
	console.log(getp.description);
}

var d = new Date().getTime();

console.log(d);
//var TimeStamp = new Date(d);
var TimeStamp = new Date("2014/5/14 10:10:00");
console.log(TimeStamp);
var data = {"value":13.81,"quality":192,"time":TimeStamp};

//var writereal = con.write("/tag/tag1.pv",data);
//if (writereal instanceof Err)
//{
//	console.log(writereal.code);
//	console.log(writereal.errString);
//}else 
//{
//	console.log("success");
//}

var pv = con.read("/tag/tag1.pv");
if(pv.hasOwnProperty("errString")){
	console.log(pv.code);
	console.log(pv.errString);
}else{
	console.log(pv);
	console.log(pv.value);
	console.log(pv.quality);
	console.log(pv.time);
}

var tt = new Date("2014/5/08 10:01:00");

var history = {"value":23.5,"quality":"bad","time":tt};

var inserthis = con.write("/tag/tag1.his.insert",history);
if (inserthis instanceof Err)
{
	console.log(inserthis.code);
	console.log(inserthis.errString);
}else
{
	console.log(inserthis,"insert successs!");
}

var t1 = new Date("2014/5/14 10:00:00");
var t2 = new Date("2014/5/14 10:01:00");
var times = [t1,t2];
var readattime = con.read("/tag/tag1.his.atTime",times);
if (readattime instanceof Err)
{
	console.log(readattime.code);
	console.log(readattime.errString);
}
{
	for (var i = readattime.length - 1; i >= 0; i--) {
		console.log(readattime[i]);
	};
}

var timeStart = new Date("2014/5/08 10:00:00");
var timeEnd = new Date("2014/5/08 10:10:00");
var resRaw1= con.read("/tag/tag1.his.raw",timeStart,timeEnd,1,function(err,resVal){
	if(err){
		console.log(err);
	}else{
		console.log(resVal[1].value);
	}
});

console.log("subscribe start");

var resSub = con.sub("/tag/tag1.real",function(err,tagName,val)
{
	if(err)
	{
		console.log("Error:",err);
	}else
	{
		console.log("value:",val);
	}
}
);