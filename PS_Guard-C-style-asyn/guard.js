var addon= require('./PS_Guard');
addon.start();

addon.startError();
addon.onStop(function(err,result){
	if(err){
		console.log(err);
	}else{
		addon.setStop();
		process.exit(1);
	}
});
