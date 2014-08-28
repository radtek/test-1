var   sio     = require('socket.io');
var http = require('http'), 
	 fs = require('fs'), 
	 querystring = require("querystring"),
	 url = require('url'),
	 addon = require("../node_modules/pSpace/build/pSpace"); 
var ps = new addon.Client();
var Err = addon.Error;

/* 创建 http 服务器 */ 
var app = http.createServer(function(req, res) { 
 /* 获取 Web 客户端请求路径 */ 
 var pathname = url.parse(req.url).pathname; 
 /* 打印客户端请求 req 对象中的 url、method 和 headers 属性 */ 
 console.log(pathname);
 console.log(req.url); 
 console.log(req.method); 
 console.log(req.headers); 
 req.setEncoding('utf8');

 /* 根据 pathname，路由调用不同处理逻辑 */ 
 switch(pathname){ 
 case '/index' : resIndex(res); // 响应 HTML 页面到 Web 客户端 
 break; 
 case '/add' : add(res,req); // 添加测点
 break; 
 case '/del' : add(res,req); // 添加测点
 break; 
 case '/read': read(res,req);//read操作
 break;
 case '/write': write(res,req);//write操作
 break;
 case '/sub': sub(res,req);//订阅操作
 break;
 default : resDefault(res); // 响应默认文字信息到 Web 客户端 
 break; 
 } 
}).listen(1337); 

io  = sio.listen(app);

var con = ps.connect("localhost","admin","admin888");
	if(con instanceof Err){
		console.log("错误码:",con.code);
		console.log("服务器连接失败:",con.errString);
	}else{
		console.log("服务器连接成功!");
	}

/** 
 * 
 * @desc 创建 resIndex 响应首页 html 函数 
 * @parameters res HTTP 响应对象 
 */ 
function resIndex(res){ 
 /* 获取当前 index.html 的路径 */ 
 var readPath = __dirname + '/' +url.parse('index.html').pathname; 
 var indexPage = fs.readFileSync(readPath); 
 res.writeHead(200, { 'Content-Type': 'text/html' }); 
 res.end(indexPage); 
} 
 
/** 
 * 
 * @desc 创建 add 响应 add 函数 
 * @parameters http://127.0.0.1:1337/add?argv=/tag11&tagtype=node
 */ 
function add(res,req){ 
 	var str = url.parse(req.url).query; 
 	var argv = querystring.parse(str).argv;
 	var jsObj = querystring.parse(str);
 	delete jsObj.argv;
 	res.writeHead(200, { 'Content-Type': 'text/plain' }); 
 	var add = con.add(argv,jsObj);
 	if(add instanceof Err){
 		res.end(add.errString);
 	}else{
 		res.end("Add "+add+" succeed!"); 
 	}
 	
} 

/** 
 * 
 * @desc 创建 read 响应 read 函数 
 * @parameters http://127.0.0.1:1337/read?argv=/tag/tag1.pv
 */ 
 function read(res,req){
	var str = url.parse(req.url).query; 
 	var argv = querystring.parse(str).argv;
 	res.writeHead(200, { 'Content-Type': 'text/plain'}); 
 	if(argv.indexOf(".pv")){
 		var realRead = con.read(argv);
	 	if(realRead instanceof Err){
	 		res.end(realRead.errString);
	 	}else{
	 		//res.write("读取到的数据!");
	 		res.end(JSON.stringify(realRead)); 
 		}
 	}	
}
/**
 * @desc 创建 write 响应 write 函数 
 * @parameters http://127.0.0.1:1337/write?argv=/tag/tag1.pv&value=56.78&quality=0
 */ 
 function write(res,req){
	var str = url.parse(req.url).query; 
 	var argv = querystring.parse(str).argv;
 	var jsObj = querystring.parse(str);
 	delete jsObj.argv;
 	if(jsObj.hasOwnProperty('value')){
 		jsObj.value = Number(jsObj.value);
 	}
 	if(jsObj.hasOwnProperty('quality')){
 		jsObj.quality = Number(jsObj.quality);
 	}
 	res.writeHead(200, { 'Content-Type': 'text/plain' }); 
 	if(argv.indexOf(".pv")){
 		var realWrite = con.write(argv,jsObj);
	 	if(realWrite instanceof Err){
	 		res.end(realWrite.errString);
	 	}else{
	 		res.end(realWrite+" write succeed!"); 
 		}
 	}	
}
/** 
 * 
 * @desc 创建 sub 响应 sub 函数 
 * @parameters http://127.0.0.1:1337/sub?argv=/tag/tag1.real
 */ 
function sub(res,req){
	var str = url.parse(req.url).query; 
 	var argv = querystring.parse(str).argv;
 	res.writeHead(200, {'Content-Type': 'text/html' }); 
 	if(argv.indexOf(".real")){
 		var sub = con.sub("/tag5/tag6.real",function(err,tagid,val){
 			if(err){
 				res.end(err);
 			}else{
 				res.end(JSON.stringify(val));
 			}
 		});
 	}	
}
/** 
 * 
 * @desc 创建 resDefault 响应 404 函数 
 * @parameters res HTTP 响应对象 
 */ 
function resDefault(res){ 
 	res.writeHead(404, { 'Content-Type': 'text/plain' }); 
	res.end('can not find source,please check your page!'); 
} 

