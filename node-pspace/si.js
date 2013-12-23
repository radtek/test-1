//导入模块pSpace模块
var createObject = require('./pSpace');

//创建pSpace数据库对象
var Ps = createObject();

//启动并连接Pspace 数据库
var server = "localhost";
var userName = "admin";
var passwd = "admin888";

var isStart = Ps.startpSpace(server,userName,passwd);

if(isStart)
{
	console.log("pSpace1 start succeed!");
}
else
{
	console.log("Sorry,pSpace1 start fail!");
	throw "NO SERVER";
}
var ptagName1 = "\\testengine\\30200003\\30200152\\HJSH127_25\\";
var ptagName2 = "\\testengine\\30200003\\30200152\\HJSH127_28\\";
var ptagName3 = "\testengine\30200003\30200152\HJSH127_29\\";

 function valObj()
{
    var array = [];
    var array2 = [];
    this.pushTag = pushTag;
    function pushTag(tag)
    {
      array.push(tag);
    }
    this.currentVal = currentVal;
    function currentVal(tag1,tag2)
    {
        array.push(tag1,tag2);
        var v1 = Ps.realRead(array[0]);
        var v2 = Ps.realRead(array[1]);
        var val = v1*256+v2;
        console.log("val:",val);
      
    }
    this.addVal =addVal;
    function addVal()
    {
      for(var i in array)
        {
          var v1 = Ps.realRead(array[i]);
          var v2 = Ps.realRead(array[0]);
          var bWrite = Ps.realWrite(array[i],v1*256+v2);
          if(bWrite)
          {
            console.log("Write succeed!");
          }
      }
    }
    this.getCurrentVal = getCurrentVal;
    function getCurrentVal(tag1,tag2)
    {
      var v1 = Ps.realRead(tag1);
      var v2 = Ps.realRead(tag2);
      var restltVal = v1*256+v2;
      console.log("CurrentVal:",restltVal);
    }
}

module.exports = valObj;

//var curVal =  new valObj();
//每五秒写入一次值
//function read()
//{
 // curVal.currentVal("\\testengine\\30200003\\30200152\\HJSH127_25\\LJCXLL","\\testengine\\30200003\\30200152\\HJSH127_25\\BPQLJLL");
//}
//function write()
//{
//  curVal.addVal();
//}
//setInterval(read, 1000);

//curVal.pushTag(ptagName1+"LJCXLL");
//write();



