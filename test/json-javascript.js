//建立对json象数组
var employees = [
	{"firstName":"guangfei","lastName":"yin"},
	{"firstName":"cong","lastName":"xu"},
	{"firstName":"qiang","lastName":"wang"}
];

console.log(employees[0].firstName);
console.log(employees[2].lastName);
//json对象
var connectData = {
	"hostname":"localhost",
	"user":"yingf",
	"password":"yingf"
};

console.log(connectData.hostname);
console.log(connectData.user);
   var str1 = '{ "name": "cxh", "sex": "man" }';
//json字符串
var jsonString ='{\
	"hostname":"localhost",\
	"user":"yingf",\
	"password":"yingf"\
}';

//json字符串转换成json对象
var jsonObj = eval('('+jsonString+')');

console.log(jsonObj.hostname);
console.log(jsonObj.password);