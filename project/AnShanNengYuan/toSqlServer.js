var sql = require('msnodesql');
var config = require( './config' );
var csv  = require('csv');
var addon = require("pSpace");
var ps = new addon.Client();
var ps = new addon();
var con = ps.open(config.psCon);
if(con instanceof ps.Err){
	console.log("错误码:",con.code);
	console.log("服务器连接失败:",con.errString);
	throw "connect fail!";
}else{
	console.log("连接成功!");
}
console.log();
function Appendzero(obj)
{
     if(obj<10) return "0" +""+ obj;
     else return obj;     
}
 var formatDate = function(now){
	var   year=now.getFullYear();     
    var   month=now.getMonth()+1;     
    var   date=now.getDate();     
    var   hour=now.getHours();     
    var   minute=now.getMinutes();     
    var   second=now.getSeconds(); 
    var   milSecond = now.getMilliseconds();    
    return   year+"-"+Appendzero(month)+"-"+Appendzero(date)+" "+Appendzero(hour)+":"+Appendzero(minute)+":"+Appendzero(second)+"."+Appendzero(milSecond);     

}
//重新封装setInterval
function accurateInterval(callback, interval) {
    var now = +new Date();

    setTimeout(function run() {
        now += interval;
        var fix = now - (+Date.now());

        setTimeout(run, interval + fix);

        callback();
    }, interval);
}
csv()
.from.path('./rw.csv',{
  columns:true
})
.transform(function(data,index){
 	 return  data;
})
//所有数据读取作为一个数组 
.to.array( function(data){ 
   
	  		//每张表的字段
		  	var table1 ="Time,";
		  	var table2 = "Time,";
		  	var table3 = "Time,";
		  	var table4 = "Time,";
		  	var table5 = "Time,";
		  	var table6 = "Time,";
		  	var table7 = "Time,";
		  	var table8 = "Time,";
		  	//每张表包含的点
		  	var table1Name = new Array();
		  	var table2Name = new Array();
		  	var table3Name = new Array();
		  	var table4Name = new Array();
		  	var table5Name = new Array();
		  	var table6Name = new Array();
		  	var table7Name = new Array();
		  	var table8Name = new Array();
		  

			for(var i=0;i<data.length;i++){
				if(data[i].strTableName=='GKEQ'){
					var s1 = data[i].strValueCol;
					var str1 = '"'+s1+'"';
					table1+= str1+',';
					table1Name.push(data[i].strTagName.replace(/[\\]/g,"/")+".his.raw");
				}
				console.log("arrLenth:",arr.length);
				if(data[i].strTableName=='GKJK'){
					var s1 = data[i].strValueCol;
					var str1 = '"'+s1+'"';
					table2+= str1+',';
					table2Name.push(data[i].strTagName.replace(/[\\]/g,"/")+".his.raw");
				} 
				if(data[i].strTableName=='GKLT'){

					var s1 = data[i].strValueCol;
					var str1 = '"'+s1+'"';
					table3+= str1+',';
					table3Name.push(data[i].strTagName.replace(/[\\]/g,"/")+".his.raw");
				} 
				if(data[i].strTableName=='GKXK'){
					var s1 = data[i].strValueCol;
					var str1 = '"'+s1+'"';
					table4+= str1+',';
					table4Name.push(data[i].strTagName.replace(/[\\]/g,"/")+".his.raw");
				} 
				if(data[i].strTableName=='GKXK1'){
					var s1 = data[i].strValueCol;
					var str1 = '"'+s1+'"';
					table5+= str1+',';
					table5Name.push(data[i].strTagName.replace(/[\\]/g,"/")+".his.raw");
				} 
				if(data[i].strTableName=='GKXK2'){
					var s1 = data[i].strValueCol;
					var str1 = '"'+s1+'"';
					table6+= str1+',';
					table6Name.push(data[i].strTagName.replace(/[\\]/g,"/")+".his.raw");
				} 
				if(data[i].strTableName=='GKXK3'){
					var s1 = data[i].strValueCol;
					var str1 = '"'+s1+'"';
					table7+= str1+',';
					table7Name.push(data[i].strTagName.replace(/[\\]/g,"/")+".his.raw");
				} 
				if(data[i].strTableName=='GKYQ'){
					var s1 = data[i].strValueCol;
					var str1 = '"'+s1+'"';
					table8+= str1+',';
					table8Name.push(data[i].strTagName.replace(/[\\]/g,"/")+".his.raw");
				} 
			}
			//删除每一字符串最后一个字符
			table1=table1.substr(0, table1.length-1);
			table2=table2.substr(0, table2.length-1);
			table3=table3.substr(0, table3.length-1);
			table4=table4.substr(0, table4.length-1);
			table5=table5.substr(0, table5.length-1);
			table6=table6.substr(0, table6.length-1);
			table7=table7.substr(0, table7.length-1);
			table8=table8.substr(0, table8.length-1);
			
			
		 	//数据读取的开始时间
		 	var startTime = new Date("2014/05/08 20:00:00.000");
		 	//数据读取的结束时间
		 	var endTime = new Date("2014/05/08 20:00:00.000");
		 	//数据转储的起始时间
		 	var time = new Date("2014/05/08 20:00:00.000");
		 	//转储任务函数
		 	function task(){
		 		if(Date.parse(eTime) - Date.parse(time) < 0){
					console.log("数据转储完毕!");
					process.exit();	
				}
				if(!con.isConnected()){
					console.log("数据库未连接，请检查！");
					process.exit();	
				}
		 		//每张表的value,读取数据的起始时间和结束时间间隔1s
		 		startTime.setSeconds(startTime.getSeconds()+1);
		 		endTime.setSeconds(startTime.getSeconds()+1);
		 		//每隔多长时间转储一次数据
		 		time.setMinutes(time.getMinutes()+config.transTime);
		 		var d = new Date(time);
 				var str = formatDate(d);
				var strTime = "'"+str+"'";
		 		var value1 = new Array();
		 		//每张表的第一个字段都是值都是时间
		 		value1.push(strTime);
		 	
		 		var insert1 = "INSERT INTO GKEQ ("+table1+") VALUES ";
				var insert2 = "INSERT INTO GKJK ("+table2+") VALUES ";
				var insert3 = "INSERT INTO GKLT ("+table3+") VALUES ";
				var insert4 = "INSERT INTO GKXK ("+table4+") VALUES ";
				var insert5 = "INSERT INTO GKXK1 ("+table5+") VALUES ";
				var insert6 = "INSERT INTO GKXK2 ("+table6+") VALUES ";
				var insert7 = "INSERT INTO GKXK3 ("+table7+") VALUES ";
				var insert8 = "INSERT INTO GKYQ ("+table8+") VALUES ";
				//读取每张表的历史数据存放在数组
		 		for(var i=0;i<table1Name.length;i++){
		 			var v1= con.read(table1Name[i],startTime,endTime,1);
		 			if(v1 instanceof ps.Err){
		 				value1.push(0);
		 			}else{
		 				value1.push(v1[0].value);
		 			}	
		 		}
		 		var value2 = new Array();
		 		value2.push(strTime);
		 		for(var i=0;i<table2Name.length;i++){
		 			var v2= con.read(table2Name[i],startTime,endTime,1);
		 			if(v2 instanceof ps.Err){
		 				value2.push(0);
		 			}else{
		 				value2.push(v2[0].value);
		 			}	
		 		}
		 		var value3 = new Array();
		 		value3.push(strTime);
		 		for(var i=0;i<table3Name.length;i++){
		 			var v3= con.read(table3Name[i],startTime,endTime,1);
		 			if(v3 instanceof ps.Err){
		 				value3.push(0);
		 			}else{
		 				value3.push(v3[0].value);
		 			}	
		 		}
		 		var value4 = new Array();
		 		value4.push(strTime);
		 		for(var i=0;i<table4Name.length;i++){
		 			var v4= con.read(table4Name[i],startTime,endTime,1);
		 			if(v4 instanceof ps.Err){
		 				value4.push(0);
		 			}else{
		 				value4.push(v4[0].value);
		 			}	
		 		}
		 		var value5 = new Array();
		 		value5.push(strTime);
		 		for(var i=0;i<table5Name.length;i++){
		 			var v5= con.read(table5Name[i],startTime,endTime,1);
		 			if(v5 instanceof ps.Err){
		 				value5.push(0);
		 			}else{
		 				value5.push(v5[0].value);
		 			}	
		 		}
		 		var value6 = new Array();
		 		value6.push(strTime);
		 		for(var i=0;i<table6Name.length;i++){
		 			var v6= con.read(table6Name[i],startTime,endTime,1);
		 			if(v6 instanceof ps.Err){
		 				value6.push(0);
		 			}else{
		 				value6.push(v6[0].value);
		 			}	
		 		}
		 		var value7 = new Array();
		 		value7.push(strTime);
		 		for(var i=0;i<table7Name.length;i++){
		 			var v7= con.read(table7Name[i],startTime,endTime,1);
		 			if(v7 instanceof ps.Err){
		 				value7.push(0);
		 			}else{
		 				value7.push(v7[0].value);
		 			}	
		 		}
		 		var value8 = new Array();
		 		value8.push(strTime);
		 		for(var i=0;i<table8Name.length;i++){
		 			var v8= con.read(table8Name[i],startTime,endTime,1);
		 			if(v8 instanceof ps.Err){
		 				value8.push(0);
		 			}else{
		 				value8.push(v8[0].value);
		 			}	
		 		}
		 		
		 		//构造每一张表的sql语句
		 		var sql1 = '';
		 		for(var j=0;j<value1.length;j++){
				  sql1+= value1[j]+",";
				}
				sql1=sql1.substr(0, sql1.length-1);
				insert1 += "("+sql1+")";
  				insert1 = insert1 + ";";
  				/////////////////////////////////////
  				
  				var sql2 = '';
		 		for(var j=0;j<value2.length;j++){
				  sql2+= value2[j]+",";
				}
				sql2=sql2.substr(0, sql2.length-1);
				insert2 += "("+sql2+")";
  				insert2 = insert2 + ";";

  				var sql3 = '';
		 		for(var j=0;j<value3.length;j++){
				  sql3+= value3[j]+",";
				}
				sql3=sql3.substr(0, sql3.length-1);
				insert3 += "("+sql3+")";
  				insert3 = insert3 + ";";

  				var sql4 = '';
		 		for(var j=0;j<value4.length;j++){
				  sql4+= value4[j]+",";
				}
				sql4=sql4.substr(0, sql4.length-1);
				insert4 += "("+sql4+")";
  				insert4 = insert4 + ";";

  				var sql5 = '';
		 		for(var j=0;j<value5.length;j++){
				  sql5+= value5[j]+",";
				}
				sql5=sql5.substr(0, sql5.length-1);
				insert5 += "("+sql5+")";
  				insert5 = insert5 + ";";

  				var sql6 = '';
		 		for(var j=0;j<value6.length;j++){
				  sql6+= value6[j]+",";
				}
				sql6=sql6.substr(0, sql6.length-1);
				insert6 += "("+sql6+")";
  				insert6 = insert6 + ";";

  				var sql7 = '';
		 		for(var j=0;j<value7.length;j++){
				  sql7+= value7[j]+",";
				}
				sql7=sql7.substr(0, sql7.length-1);
				insert7 += "("+sql7+")";
  				insert7 = insert7 + ";";

  				var sql8 = '';
		 		for(var j=0;j<value8.length;j++){
				  sql8+= value8[j]+",";
				}
				sql8=sql8.substr(0, sql8.length-1);
				insert8 += "("+sql8+")";
  				insert8 = insert8 + ";";
  				//sql语句执行
				sql.open( config.conn_str, function( err, conn ) {
					if(err){
						console.log(err);
					}else{
						conn.queryRaw( insert1, function(err) {
							if(err){
								console.log("table1:",err);
							}
							
                		});
                		conn.queryRaw( insert2, function(err) {
                			if(err){
                				console.log("table2:",err);
                			}
							
                		});
                		conn.queryRaw( insert3, function(err) {
                			if(err){
                				console.log("table3:",err);
                			}
							
                		});
                		conn.queryRaw( insert4, function(err) {
                			if (err) {
                				console.log("table4:",err);
                			};
							
                		});
                		conn.queryRaw( insert5, function(err) {
                			if (err) {
                				console.log("table5:",err);
                			};
							
                		});
                		conn.queryRaw( insert6, function(err) {
							if (err) {
                				console.log("table6:",err);
                			};
                		});
                		conn.queryRaw( insert7, function(err) {
							if (err) {
                				console.log("table7:",err);
                			};
                		});
                		conn.queryRaw( insert8, function(err) {
							if (err) {
                				console.log("table8:",err);
                			};
                		});
					}	
				});
		 	}
		//每隔5s转储一次数据
		setInterval(task,config.intreval);
	   
} )
.on('close', function(count){
  console.log('Number of lines: '+count);
})
.on('error', function(error){
  console.log(error.message);
});

