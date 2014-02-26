var server = require("./testServer.js");

var tagReal = require("./tagReal.js");
tagReal.close();
var sub = require("./sub.js");
var tag = require("./tag.js");
tag.close();

var tagHis = require("./tagHis.js");
tagHis.close();

var tagAlarm = require("./tagAlarm.js");
tagAlarm.close();

