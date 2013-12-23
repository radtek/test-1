
function person(name)
{
  this.name = name;
  this.showName =  showName;
}

person.prototype.showName = function()
{
  console.log("My name is :",this.name);
}

exports.person = person;