//1.引入模块
var cluster = require('cluster');
var express= require('express');
var os = require('os');

//获取cpu数目（几核的cpu）
var cpuNum = os.cpus().length;
if (cluster.isMaster) {//判断是否是主进程
    console.log('[master] ' + "start master...");

    for (var i = 0; i < cpuNum; i++) {
         cluster.fork();//根据cpu核心数创建子进程
    }
    //监听
    cluster.on('listening', function (worker, address) {
        console.log('[master] ' + 'listening: worker' + worker.id + ',pid:' + worker.process.pid + ', Address:' + address.address + ":" + address.port);
    });

} else if (cluster.isWorker) {//使用express建立web服务
    console.log('[worker] ' + "start worker ..." + cluster.worker.id);
    var app = express();
    var server = app.listen(3000,"127.0.0.1");
    app.use(function(req,res,next){
         console.log('worker'+cluster.worker.id);
         res.end('worker'+cluster.worker.id+',PID:'+process.pid);
    });
}