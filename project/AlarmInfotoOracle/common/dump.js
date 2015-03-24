var async = require('async'),
	dbhelper = require('./dbhelper'),
	sqlString = require('./sqlString'),
	utf8 = require('utf8'),
	iconv = require('iconv-lite'),
	config = require("../db/configure"),
	props = require('./props');

function formatBoolean(value){
	if(typeof(value)==="boolean"){
		if(value==true){
			value = 1;
		}else{
			value = 0;
		}
	}
	return value;
}
function getUpdateValue (value) {
	var keys = Object.keys(value);
	value["LastModifyTime"]="to_date(substr("+sqlString.escape(sqlString.dateToString(value["LastModifyTime"],'local'))+",1,19), 'yyyy-mm-dd hh24:mi:ss' )";
	var str = "set Time=sysdate";
	for(var i in keys){
		if(keys[i]==="LastModifyTime"){
			str += ","+keys[i]+"="+value[keys[i]];
		}else
			str += ","+keys[i]+"="+sqlString.escape(formatBoolean(value[keys[i]]));
	}
	return str;
}
function getInsertValue (value) {
	var keys = Object.keys(value);
	var field="(TIme";
	var values = "(sysdate";
	for(var i in keys){
		field += ","+keys[i]+"";
		if(keys[i]==="LastModifyTime"){
			values +=","+value[keys[i]];
		}else
			values +=","+sqlString.escape(formatBoolean(value[keys[i]]));
	}
	///console.log(field+") values"+values+")");
	return field+") values"+values+")";
}


//生成sql
function format(tagProps){
	var sql=[];
	var propKeys = Object.keys(tagProps[0]);
	for(var i in tagProps){
		var strTmp = "begin update "+config.table+" " +getUpdateValue(tagProps[i])+" where LongName="+sqlString.escape(tagProps[i]["LongName"])+";if sql%notfound then insert into "+config.table+""+getInsertValue(tagProps[i])+";end if;end;";
		sql.push(iconv.encode(strTmp, "GBK").toString('binary'));
		
	}
	return sql;

}
//转储
function dump(psConn,params,done){
	async.waterfall([
		function(cb){
			//获取属性
			props.getProps(psConn,params,function(err,tagProps){
				if(err){
					cb(err);
				}else{
					cb(null,tagProps);
				}
			});
		},
		function(tagProps,cb){
			if(tagProps.length>0){
				var sql = format(tagProps);
				dbhelper.execQuery(sql,function(err,ret){
					if(err){
						console.log(err);
						cb(err);
					}else{
						cb(null,ret);
					}
				});
			}else
				cb(null,"NO_TAG");
			//tagProps = [{"LongName":"test","age":1,"LastModifyTime":new Date()},{"LongName":"test2","age":2,"LastModifyTime":new Date()}];
			
		}
	],function(err,ret){
		if(err)
			done(err);
		else
			done(null,ret);
	});

}
exports.dump=dump;