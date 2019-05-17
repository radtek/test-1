
function ps2ua(dataType){
    if(dataType==12){
        return 13;
    }else if(dataType==13){
        return 12;
    }
    return dataType;
}

function ua2ps(dataType){
    if(dataType==12){
        return 13;
    }else if(dataType==13){
        return 12;
    }
    return dataType;
}

module.exports.ps2ua = ps2ua;
module.exports.ua2ps = ua2ps;