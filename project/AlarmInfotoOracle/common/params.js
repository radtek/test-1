var read = require('readCsv'),
	configure=require('../db/configure'),
	iconv = require('iconv-lite');

var data = {};
data["props"]=["TagType","LongName","Description","DataType","EnableAlarm","EnableValueAlarm","NormalValue",
    "ValueAlarmNeedRespond","EnableQualityAlarm","QualityAlarmNeedRespond","ValueAlarmClass",
    "QualityAlarmClass","EnableLLAlarm","EnableLOAlarm","EnableHIAlarm","EnableHHAlarm","EnableRateAlarm",
    "EnableDevAlarm","LowLowAlarmNeedRespond","LowAlarmNeedRespond","HighAlarmNeedRespond","HighHighAlarmNeedRespond",
    "RateAlarmNeedRespond","DevAlarmNeedRespond","LowLowAlarm","LowAlarm","HighAlarm","HighHighAlarm","Rate",
    "RatePeriod","Dev","SP","DeadBand","LowLowAlarmPriority","LowAlarmPriority","HighAlarmPriority",
    "HighHighAlarmPriority","RateAlarmPriority","DevAlarmPriority","LastModifyTime"
];
data["longName"]=[];
var last={};
//读取配置文件并填充字典
exports.readParams=function(conn,done){
  read.readFile(configure.configPath,function(err,confData){
    if(err){
    	done(err);
    }else{
    	for(var i in confData){
        if(confData[i].TagType!='psNode'){
          var longName = new Buffer(confData[i].LongName, 'binary');
          var strName = iconv.decode(longName, 'gbk');
          strName = strName.replace(/[\\]/g,"/");
          var exists = conn.isExists(strName);
          if(exists && !exists.hasOwnProperty("errString")){
            data["longName"].push(strName);
            last[strName]=0;
          }
          
        }
    	}
    	done(null,data);
    }
  });
}
//exports.readParams=readParams;