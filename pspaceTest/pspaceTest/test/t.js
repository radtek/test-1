var async = require("async");

var fun1 = function(){
		setTimeout(function(){
		console.log("Hello One!");
	},3000);
	}
var fun2 = function(){
		console.log("Hello Two!");
	}
var fun3 = function(){
		setTimeout(function(){
		console.log("Hello three!");
	},100)
	}
/*
async.series({
	one:function(callback){
		callback(null,fun1());
	},
	two:function(callback){
		callback(null,fun2());
	},
	three:function(callback){
		callback(null,fun3());
	}
},
function(err,results){
		//console.log(v1);
});
*/

/*
fun1();
fun2();
fun3();
*/



async.series({
    one: function(callback){
    	console.log("1");
        setTimeout(function(){
           callback(null, fun2());
        }, 4000);
    },
    two: function(callback){
    	console.log("2");
        //setTimeout(function(){
            callback(null, console.log("Hello2"));
       // }, 1000);

    },
    three:function(callback){
    	console.log(3);
    	callback(null,fun3());
    }
},
function(err,results){
   //console.log(results);
});

/*
async.parallel([
	function(cb){
		setTimeout(function(){console.log("I am One")},5000);
		cb(null,1);
	},
	function(cb){
		setTimeout(function(){console.log("I am two")},2000);
		cb(null,2);
	},
	function(cb){
		setTimeout(function(){console.log("I am three")},1000);
		cb(null,3);
	},
	],function(err,results){
	console.log(results);
});
*/
/*
var iter = async.iterator([
	function(){
		fun1();
	},
	function(){
		fun2();
	},
	function(){
		fun3();
	},
	]);
iter();
iter.next();
*/