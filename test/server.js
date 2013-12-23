//导入模块pSpace模块
var createObject = require('./pSpace');

//创建pSpace数据库对象
var Ps = createObject();
//启动并连接Pspace 数据库
function connect(server,userName,passwd)
{
  var isStart = Ps.startpSpace(server,userName,passwd);
  if(isStart)
  {
    console.log("pSpace start succeed!");
  }
  else
  {
    console.log("Sorry,pSpace start fail!");
    throw "No pSpace server!";
  }

}
//所有实时值读取，返回数组对象
function realRead(tagName)
{
  var val = Ps.realRead(tagName);
  if(val)
    return val;
  else
  {
    return false;
   }
}
//异步回调方式读取所有实时值
function realReadAsy(tagName,callback)
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
//单独实时值获取
function currentValue(tagName)
{
  var val = Ps.realRead(tagName);
  if(val)
    return val[0].value;
  else
  {
    return "No have real-data or read error!";
   }
}
//异步回调单独实时值获取
function currentValueAsy(tagName,callback)
{
  var val = Ps.realRead(tagName);//放在异步进程中此函数有可能来不及执行就进行下一步，所以会出现意想不到的结果
  process.nextTick(function(){
   var curval = val[0].value;
    callback(curval);
  });
  
}
//质量戳获取
function currentQuality(tagName)
{
  var val = Ps.realRead(tagName);
  if(val)
    return val[0].quality;
  else
  {
    return "No have real-data or read error!";
   }

}
//异步回调获取质量戳
function currentQualityAsy(tagName,callback)
{
  var val = Ps.realRead(tagName);
  process.nextTick(function(){
   var quality = val[0].quality;
   if(val)
      callback(quality);
   else
  {
    return false;
  }
  });
}

//时间获取
function currentTime(tagName)
{
  var val = Ps.realRead(tagName);
  if(val)
    return val[0].time;
  else
  {
    return "No have real-data or read error!";
   }
}

//异步回调获取时间戳
function currentTimeAsy(tagName,callback)
{
  var val = Ps.realRead(tagName);
  process.nextTick(function(){
   var time = val[0].time;
   if(val)
      callback(time);
   else
  {
    return false;
  }
  });
}


//写实时值
function realWrite(tagName,value)
{
  var isWrite = Ps.realWrite(tagName,value);
  if(isWrite)
    return true;
  else 
    return false;
}

// 实时订阅实时值
 function realSubscribleAsy(tagName,callback)
 {
    
    //if(isSub)
    //{
    //  console.log("subscrible tag testCase_Tag1 Succeed");
       //采用异步回调
       process.nextTick(function(){
       var isSub = Ps.realsubscribe(tagName);
        Ps.subscribeData(true,callback);
       });
     
    //}
    //else 
    //  Ps.log("subscrible Error!");
 }
 //订阅实时值，非异步
 function realSubscribleSyn(tagName,callback)
 {
    var isSub = Ps.realsubscribe(tagName);
    if(isSub)
    {
      console.log("subscrible tag testCase_Tag1 Succeed");
      Ps.subscribeData(false,callback);
    }
    else 
      Ps.log("subscrible Error!");
 }
//断开数据库连接
function disConnect()
{
	var stop = Ps.stoppSpace();
	if(stop)
		console.log("Disconnect succeed!");
	else
	{
		console.log("Disconnect fail!");
		throw "Disconnect Error!";
	}
}

//var a = currentQuality("\\采集站1\\30200003\\30200152\\HJSH127_25\\LJCXLL");
//console.log(a);

exports.connect = connect;
exports.realRead = realRead;
exports.realReadAsy = realReadAsy;
exports.currentValueAsy = currentValueAsy;
exports.realWrite = realWrite;
exports.realSubscribleAsy = realSubscribleAsy;
exports.realSubscribleSyn = realSubscribleSyn;
exports.disConnect = disConnect;
exports.currentValue = currentValue;
exports.currentQuality = currentQuality;
exports.currentQualityAsy = currentQualityAsy;
exports.currentTimeAsy = currentTimeAsy;
exports.currentTime = currentTime;

