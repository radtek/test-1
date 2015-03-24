
var isFirst=true;
var last={};
function formatProps(props){
	//获取所有的key
	var keys = Object.keys(props);
	for(var i in keys){
		if(!props[keys[i]].EnableAlarm || (!isFirst && props[keys[i]].LastModifyTime<=last[keys[i]])){
			delete props[keys[i]];
		}
	}
	
	//获取产生了报警点的key
	var tagKeys = Object.keys(props);
	//获取所有属性key,所有有点属性key相同，获取一次即可
	var fieldValue = [];
	if(tagKeys.length>0){
		var propsKeys = Object.keys(props[tagKeys[0]]);
		//console.log(propsKeys);
		for(var j in tagKeys){
			last[tagKeys[j]]=props[tagKeys[j]].LastModifyTime.getTime();
			for(var key in propsKeys){
				if(props[tagKeys[j]][propsKeys[key]]===undefined){
					props[tagKeys[j]][propsKeys[key]]='';
				}
			}

			fieldValue.push(props[tagKeys[j]]);
		}
	}
	
	return fieldValue;
}
//读取属性并且格式化
function getProps (conn,params,done) {
	var props = conn.readPropsList(params);
	if(props.hasOwnProperty('errString')){
		done(props.errString)
	}else{
		props = formatProps(props);
		isFirst = false;
		//格式化数据之后返回（过滤,格式化成oracle直接能用的）
		done(null,props);
	}	
}
/*
var con = ps.open({
	"hostname":"localhost",
	"user":"admin",
	"password":"admin888"
});

var prop =["LongName","LastModifyTime","Description","EnableAlarm","EnableLOAlarm","EnableHIAlarm","LowAlarm","HighAlarm","ValueAlarmClass"]
var params = {
	"longName":["/tag/tag1","/tag/tag2","/tag/tag5"],
	"props":prop
};
getProps(con,params,function(){

});
*/
exports.getProps=getProps;