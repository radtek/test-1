var addon = require("./classTest");

var person = new addon.Person();

var animal = new addon.Animal();

person.showPerson();
animal.showAnimal();
person.runcallback(function(results){
	console.log(results);
});

var obj = person.readread();
console.log(obj);
console.log("-----------------------------------------");
var connectData = {
	"hostname":"localhsot",
	"user":"yingf",
	"password":"yingf"
};
person.importdata(connectData);