var addon = require("../lib/pSpace");
var async = require("async");
var ps = new addon.Client();
var con = ps.connect("localhost","admin","admin888");
if(con.hasOwnProperty("errString"))
{
	console.log("错误码:",con.code);
	console.log("服务器连接失败:",con.errString);
	throw "connect fail!";
}
console.log();
var writeData  = {
	"value":67.001,
	"quality":0,
	"time":"2015-01-21 00:20:0.123"
};

var fun1 = function ()
{
	var res = con.read("/tag.pv",function(err,pv){
	if(err){
		console.log("异步读:",err);
	}else{
		console.log("异步读:",pv.value);
	}
	});
}
var fun2 = function()
{

	var resAsy = con.write("/tag.pv",writeData, function(err,tagName){
	if(err){
		console.log("异步写:",err);
	}else{
		console.log(tagName,":异步写实时成功");
	}
	});
}
var fun3 = function()
{
	function f(){con.close();}
	return f();
}

var fun4 = function()
{
	var pv = con.read("/tag.pv");
if(pv.hasOwnProperty("errString")){
	console.log("同步读:",pv.errString);
}else{
	console.log("同步读:",pv);
}
}
/*
async.series({
    one: function(callback){
    	setTimeout(fun3(),3000);
    	callback(null,1);
    },
    two: function(callback){
    	setTimeout(fun2(),1000);
    	callback(null,2);
    },
    three:function(callback){
    	setTimeout(fun1(),500);
    	callback(null,3);
    },
    four:function(callback){
    	setTimeout(fun4(),100);
    	callback(null,4);
    }
},
function(err,results){
   console.log(results);
});
s
*/

async.series({
    one: function(callback){
    	setTimeout(fun1,5000);
        
    },
    two: function(callback){
    	setTimeout(fun4,6000);
        //setTimeout(function(){
         //   callback(null, fun2);
       // }, 1000);

    },
    three:function(callback){
    	console.log(3);
    	callback(null,fun4());
    },
    four:function(callback){
    	//setTimeout(function(){
           callback(null, console.log("HEHEHEHH"));
       // }, 4000);
    }
},
function(err,results){
   console.log(results);
});






/*

//fun1();
//fun2();

//先将连接关闭
con.close();

console.log("同步读数据操作：");
var pv = con.read("/tag.pv");
if(pv.hasOwnProperty("errString")){
	console.log(pv.code);
	console.log(pv.errString);
}else{
	console.log("同步读实时成功!");
	console.log(pv);
}

var writeData  = {
	"value":67.001,
	"quality":0,
	"time":"2015-01-21 00:20:0.123"
};



/*
 *同步写
 */

/*

 console.log("同步写数据操作:");
var resWrite1 = con.write("/tag.pv",writeData);
if(resWrite1.hasOwnProperty("errString")){
	console.log(resWrite1.code);
	console.log(resWrite1.errString);
}else{
	console.log(resWrite1,":同步写实时成功");
}
//异步写

 var resAsy = con.write("/tag.pv",writeData, function(err,tagName){
	if(err){
		console.log(err);
	}else{
		console.log("异步写数据操作:");
		console.log(tagName,":异步写实时成功");
	}
});

var res = con.read("/tag.pv",function(err,pv){
	if(err){
		console.log(err);
	}else{
		console.log("异步写数据操作:");
		console.log("异步读实时成功");
		console.log(pv);
		console.log(pv.value);
	}
});

console.log("I am work first!");

*/

/*
fun1();
fun2();
fun4();
setTimeout(fun3,6000);

(function(){
	console.log("Hello");
	return !this;
})();


var scope = "global scope";

function checkScope()
{
	var scope = "local scope";
	function f(){return scope;}
	return f();
}
console.log(checkScope());
function checkScope1()
{
	var scope = "local scope";
	function f(){return scope;}
	return f;
}
console.log(checkScope1()());
*/
