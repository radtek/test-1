var Ps = require("./lib/server");

//连接服务器
Ps.connect("localhost","admin","admin888");

var tagName ="\\采集站1\\30200003\\30200152\\HJSH127_25\\TY_TYZXLC";
//所有实时值读取
var value = Ps.realRead("\\采集站1\\30200003\\30200152\\HJSH127_25\\TY_TYZXLC");
console.log("TagName:",value[0].name);
console.log("value:",value[0].value);
console.log("quality:",value[0].quality);
console.log("time",value[0].time);

//采用异步回调的方法获取所有实时值
  //1.回调函数
 function realReadCallback(val)
 {
   console.log("value:",val[0].value);
   console.log("quality:",val[0].quality);
   console.log("time",val[0].time);
 }
  //2.传入回调函数
 Ps.realReadAsy(tagName,realReadCallback);
//获取某一测点值
var v1 = Ps.currentValue("\\采集站1\\30200003\\30200152\\HJSH127_25\\TY_TYZXLC");
if(v1 == true)
  console.log("currentValue:",v1);

//采用异步回调方式获取某一测点值
  //1.回调函数
 function currentValueCallback(val)
 {
    console.log(val);
 }
  //2.调用
 Ps.currentValueAsy(tagName,currentValueCallback);
//获取某一测点的质量戳
var quality = Ps.currentQuality("\\采集站1\\30200003\\30200152\\HJSH127_25\\TY_TYZXLC");
if (quality == true)
  console.log("quality:",quality);

//采用异步回调方式获取某一质量戳
  //1.回调函数
 function currentQualityCallback(val)
 {
    console.log(val);
 }
  //2.调用
 Ps.currentQualityAsy(tagName,currentQualityCallback);
//获取某一测点的时间戳
var time  = Ps.currentTime("\\采集站1\\30200003\\30200152\\HJSH127_25\\TY_TYZXLC");
if(time == true)
  console.log("time:",time);
//采用异步回调方式获取某一质量戳
  //1.回调函数
 function currentTimeCallback(val)
 {
    console.log(val);
 }
  //2.调用
 Ps.currentTimeAsy(tagName,currentTimeCallback);
//异步订阅某一测点实时值,无法看到执行效果
//Ps.realSubscribleAsy("\\采集站1\\30200003\\30200152\\HJSH127_25\\TY_TYZXLC",function(err,value,quality,time){
//  console.log("subscrible Value:",value);
//  console.log("subscrible quality:",quality);
//  console.log("subscrible Time",time);
//});

//非异步订阅某一测点实时值
Ps.realSubscribleSyn("\\采集站1\\30200003\\30200152\\HJSH127_25\\TY_TYZXLC",function(err,value,quality,time){
  console.log("subscrible Value:",value);
  console.log("subscrible quality:",quality);
  console.log("subscrible Time",time);
});

 //写实时
var b = Ps.realWrite("\\采集站1\\30200003\\30200152\\HJSH127_25\\TY_TYZXLC",55.55);
if(b==true)
  console.log("Write succeed!");
 else 
  console.log("Write Error!");
  
//断开服务器连接
Ps.disConnect();