var addon = require("./pSpace");
//var bat = require("../lib/tag.js");
var events = require( "events" );
var event = new events.EventEmitter(); 
var ps = new addon.Client();
var Err = addon.Error;
var con;
if(ps.isConnected()){
	console.log("已经连接!");
}
con = ps.connect("localhost","admin","admin888");
if(ps.isConnected()){
	console.log("asdfa");
}
