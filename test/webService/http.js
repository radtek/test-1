var http = require('http'), 
fs = require('fs'), 
url = require('url'), 
BASE_DIR = __dirname; 

http.createServer(function(req,res){
	var pathname = url.parse(req.url).pathname;
	var realPath = __dirname+'/static'+pathname;
	console.log(realPath);
	if(pathname == 'favicon.ico')
		return;
	if(pathname == '/index' || pathname == '/')
		goIndex(res);
	else{
		dealWithStatic(pathname,realPath,res);
	}
}).listen(1337,'127.0.0.1');


function goIndex(res){ 
	var readPath = BASE_DIR + '/' +url.parse('index2.html').pathname; 
	var indexPage = fs.readFileSync(readPath); 
	res.writeHead(200, { 'Content-Type': 'text/html' }); 
	res.end(indexPage); 
} 
function dealWithStatic(pathname,realPath,res){
	fs.exists(realPath,function(exists){
		if(!exists){
			res.writeHead(404,{'Content-type':'text/plain'});
			res.write("This request URL " + pathname + " was not found on this server.");
			res.end();
		}else{
			var pointerPosition = pathname.lastIndexOf('.'),
				mmieString = pathname.substring(pointerPosition+1), 
				mmieType;
			switch(mmieString){
				case 'css': mmieType = "text/css";
				break;
				case 'png': mmieType = "image/png";
				break;
				default: 
				mmieType = "text/plain"
			}
			fs.readFile(realPath,'binary',function(err,file){
				if(err){
					res.writeHead(500, {'Content-Type': 'text/plain'}); 
					res.end(err);
				}else{
					res.writeHead(200, {'Content-Type': mmieType}); 
					res.write(file, "binary"); 
					res.end();
				}
			});
		}
	});
}

