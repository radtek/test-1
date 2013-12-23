var createObject = require('./pSpaceNodeTest');

var obj = createObject();

obj.serverConnect("localhost","admin","admin888");

var obj2 = createObject2();

obj2.realRead("\\testCase_Node1\\testCase_Tag1");