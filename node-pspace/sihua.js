var Ps = require("./server");
Ps.connect("localhost","admin","admin888");
//各测点数组
var nodeArray1 = [];
var nodeArray2 = [];
var nodeArray3 = [];

var ptagName1 = "\\采集站1\\30200003\\30200152\\HJSH127_25\\";
var ptagName2 = "\\采集站1\\30200003\\30200152\\HJSH127_28\\";
var ptagName3 = "\采集站1\30200003\30200152\HJSH127_29\\";

nodeArray1.push(ptagName1+"LJCXLL",ptagName1+"ZC_ZQLLLJCLZ",ptagName1+"BPQLJLL1");
nodeArray2.push(ptagName2+"LJCXLL",ptagName2+"ZC_ZQLLLJCLZ",ptagName2+"BPQLJYDL");
nodeArray3.push(ptagName3+"LJCXLL",ptagName3+"ZC_ZQLLLJCLZ",ptagName3+"BPQLJYDL");

//在待计算的数组中添加测点
function TagPush(tagName)
{
  nodeArray1.push(tagName);
}

//采集某一测点所有值
function realData(tag)
{
  var value = Ps.realRead(tag);
  return value;
}
//异步回调方式读取所有实时值
function realDataAsy(tagName,callback)
{
  var val = Ps.realRead(tagName);
  process.nextTick(function(){
  if(val)
    callback(val);
  else
  {
    return false;
  }
  });
}

//采集某一测点质量戳
 function getQuality(tag)
 {
  var value = Ps.currentQuality(tag);
  return value;
 }
 
 //异步回调获取质量戳
function getQualityAsy(tagName,callback)
{
  var val = Ps.currentQuality(tagName);
  process.nextTick(function(){
   if(val)
      callback(val);
   else
  {
    return false;
  }
  });
}
 //采集某一测点时间戳
 function getTime(tag)
 {
  var value = Ps.currentTime(tag);
  return value;
 }
 
 //异步回调获取时间戳
function getTimeAsy(tagName,callback)
{
  var val = Ps.currentTime(tagName);
  process.nextTick(function(){
   if(val)
      callback(val);
   else
  {
    return false;
  }
  });
}

//采集某一个测点当前值
function getCurrentVal(tag1,tag2)
{
  var v1 = Ps.currentValue(tag1);
  var v2 = Ps.currentValue(tag2);
  var restltVal = v1*256 + v2;
  return restltVal;
}
//异步回调单独实时值获取
function getcurrentValAsy(tag1,tag2,callback)
{
  var v1 = Ps.currentValue(tag1);
  var v2 = Ps.currentValue(tag2);
  var restltVal = v1*256 + v2;
  process.nextTick(function(){
    callback(restltVal);
  });
  
}
//订阅实时值，非异步
 function realSubscribleData(tagName,callback)
 {
   Ps.realSubscribleSyn(tagName,callback);
 }
//循环采集\采集站1\30200003\30200152\HJSH127_25中测点当前值
function cycleGetCurrentVal()
{ 
  for(var i=0;i<nodeArray1.length;i++)
    {
      var v1 = Ps.currentValue(nodeArray1[i]);
      var v2 = Ps.currentValue(nodeArray1[1]);
      var restltVal = v1*256+v2;
      console.log("CurrentVal:",restltVal);
     }
    
}
//对某一测点进行实时值写入
function addVal(tagName1,tagName2)
{
   var v1 = Ps.currentValue(tagName1);
   var v2 = Ps.currentValue(tagName2);
   var bWrite = Ps.realWrite(tagName1,v1*256+v2);
   if(bWrite)
    {
      console.log("Write succeed!");
    }
}
//对nodeArray2中测点进行值写入
function CycleAddVal()
{
  for(var i in nodeArray2)
  {
    var v1 = Ps.currentValue(nodeArray1[i]);
    var v2 = Ps.currentValue(nodeArray1[1]);
    var bWrite = Ps.realWrite(nodeArray2[i],v1*256+v2);
    if(bWrite)
    {
      console.log("Write succeed!");
    }
  }
}

exports.TagPush = TagPush;
exports.getCurrentVal = getCurrentVal;
exports.addVal = addVal;
exports.cycleGetCurrentVal = cycleGetCurrentVal;
exports.CycleAddVal = CycleAddVal;
exports.realData = realData;
exports.getQuality = getQuality;
exports.getTime = getTime;
exports.realDataAsy = realDataAsy;
exports.getcurrentValAsy = getcurrentValAsy;
exports.getQualityAsy = getQualityAsy;
exports.getTimeAsy = getTimeAsy;
exports.realSubscribleData = realSubscribleData;



