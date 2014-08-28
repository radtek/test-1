var name = "the Widow";
var object = {
  name : "My Object",
　getName : function(){
            var this = 
　　　　　　return function(){
　　　　　　　　return this.name;
　　　　　　};
　　　　}
　};


var object1 = {
  name : "my Object",
  getName  : function(){
  var that = this;
    return function(){
      return that.name;
    };
  }
};

console.log(object.getName()());
console.log(object1.getName()());