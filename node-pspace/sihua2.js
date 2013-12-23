var Ps = require("./server");
Ps.connect("localhost","admin","admin888");

//对某一测点值写入
function addTagval(tagName,value)
{
   var bWrite = Ps.realWrite(tagName,value);
   if(bWrite)
    {
      console.log("Write succeed!");
    }
}

//采集值并写入当前点
function getCurrentVal(tag1,tag2,tag3)
{
  var args = arguments.length;
  if(args==1)
  {
    var v1 = Ps.currentValue(tag1);
    return v1;
  }
  if(args==2)
  {
    var v1 = Ps.currentValue(tag1);
    var v2 = Ps.currentValue(tag2);
    var restltVal = v1*256 + v2;
    return restltVal;
  }
  else
  {
    var v1 = Ps.currentValue(tag1);
    var v2 = Ps.currentValue(tag2);
    var restltVal = v1*256 + v2;
    addTagval(tag3,restltVal);
    return restltVal;
  }
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
exports.addTagval = addTagval;
exports.getCurrentVal = getCurrentVal;
exports.getcurrentValAsy = getcurrentValAsy;

