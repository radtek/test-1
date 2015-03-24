var persist = require('persist'),
	options = require('../db/connection');
	async = require('async');

var getConnection = function(cb) {
	persist.connect(options.oracle, function(err, connection){
		if(err){
			console.log("connection---------------",err);
			cb(err);
		}else{
			cb(null,connection);
		}
	});
};



//
// @sql - array
// @ cb - callback(err, ret), function.
// 
exports.execTransaction = function(sql, cb) {
	getConnection(function(err, connection){
		if (err){
			cb && cb(err);
		}else{
			
			connection.tx(function(err,tx){
				if(err){
					cb(err);
				}else{
					connection.runSql(sql, function(err, result){
						if(err){
							tx.rollback(function(err) {});
							cb(err);
						}else{
							tx.commit(function(err,ret) {});
							cb(null,result);
						}
				    });
				}
				
		    });	
		}	
		
	});
};


//
// @sql - formatted sql string.
// @ cb - callback(err, ret), function.
// 
var execQuery = function(sql, cb) {
	var self = this;
	getConnection(function(err, connection){
		if (err){
			connection.close();
			cb && cb(err);
		}
		connection.runSql(sql, function(err, ret){
			connection.close();
			cb && cb(err, ret);
		});
	});
};
exports.execQuery=execQuery;

	