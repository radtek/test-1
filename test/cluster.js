var cluster = require('cluster');
var os = require('os');

var os = os.cpus().length;

var workers = {};
if(cluster.ismaster)
{
  console.log("This is a master!");
  cluster.on('death',function(worker){
    //当一个工作进程结束时，重启工作进程
    delete workes[worker.pid];
    worker = cluster.fork();
    workers[worker.pid] = worker;
  });
   for(var i = 0;i<numCPUs;i++){
    var worker = cluster.fork();
    workers[worker.pid] = worker;
  }
}
else
{
  var app = require('./test');
  
}
process.on('SIGTERM',function(){
  console.log("The process is stop!");
  for(var pid in workers){
    process.kill(pid);
  }
  
  process.exit(0);
});