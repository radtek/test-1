
var csv  = require('csv');
csv()
.from.path('./rw.csv',{
  columns:true
})
//数据处理
.transform(function(data,index){
 	 return  data;
})
//所有数据读取作为一个数组 
.to.array( function(data){ 
   
	  		
	   
} )
//一行一行的读取
.on('data',function(index,data){

})
.on('close', function(count){
  console.log('Number of lines: '+count);
})
.on('error', function(error){
  console.log(error.message);
});

