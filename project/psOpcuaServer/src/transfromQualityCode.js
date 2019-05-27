const opcua = require("node-opcua");
const StatusCodes = require("node-opcua-constants").StatusCodes

function qualityPs2ua(quality){
    let code = opcua.StatusCodes.Good;
    switch(quality){
        case 192:
            break;
		case 0:
			code = opcua.StatusCodes.Bad;
			break;
		case 64:
			code = opcua.StatusCodes.Uncertain;
		case 128:
			code = opcua.StatusCodes.Uncertain;
			break;
		case 4:
			code = opcua.StatusCodes.BadConfigurationError;
			break;
		case 8:
			code = opcua.StatusCodes.BadServerNotConnected;
			break;
		case 12:
			code = opcua.StatusCodes.BadDeviceFailure;
			break;
		case 16:
			code = opcua.StatusCodes.BadServerHalted;
			break;
		case 20:
			code = opcua.StatusCodes.Bad;
			break;
		case 24:
			code = opcua.StatusCodes.BadCommunicationError;
			break;
		case 28:
			code = opcua.StatusCodes.BadSensorFailure;
			break;
		case 32:
			code = opcua.StatusCodes.BadWaitingForInitialData;
			break;
		case 68:
			code = opcua.StatusCodes.UncertainNoCommunicationLastUsableValue;
			break;
		case 80:
			code = opcua.StatusCodes.UncertainSensorNotAccurate;
			break;
		case 84:
					code = opcua.StatusCodes.UncertainEngineeringUnitsExceeded;
			break;
		case 88:
					code = opcua.StatusCodes.UncertainSubNormal;
			break;
		case 216:
					code = opcua.StatusCodes.GoodLocalOverride;
			break;
		case 65536:
					code = opcua.StatusCodes.BadTooManyMatches;
			break;
		case 131072:
					code = opcua.StatusCodes.GoodEntryInserted;
			break;
		case 262144:
					code = opcua.StatusCodes.Good;
			break;
		case 524288:
					code = opcua.StatusCodes.Good;
			break;
		case 1048576:
					code = opcua.StatusCodes.BadBoundNotFound;
			break;
		case 2097152:
					code = opcua.StatusCodes.GoodNoData;
			break;
		case 4194304:
					code = opcua.StatusCodes.BadDataLost;
			break;
		case 8388608:
					code = opcua.StatusCodes.Bad;
			break;
		case 16777216:
					code = opcua.StatusCodes.Bad;
			break;
        default:
			code = opcua.StatusCodes.Bad;
            break;
    }
    return code;
}

function qualityUa2ps(statusCode){
		if(statusCode==opcua.StatusCodes.Good){
			return 192;
		}else if(statusCode==opcua.StatusCodes.Uncertain){
			return 64;
		}else if(statusCode==opcua.StatusCodes.BadConfigurationError){
			return 4;
		}else if(statusCode==opcua.StatusCodes.BadServerNotConnected){
			return 8;
		}else if(statusCode==opcua.StatusCodes.BadDeviceFailure){
			return 12;
		}else if(statusCode==opcua.StatusCodes.BadServerHalted){
			return 16;
		}else if(statusCode==opcua.StatusCodes.BadCommunicationError){
			return 24;
		}else if(statusCode==opcua.StatusCodes.BadSensorFailure){
			return 28;
		}else if(statusCode==opcua.StatusCodes.BadWaitingForInitialData){
			return 32;
		}else if(statusCode==opcua.StatusCodes.UncertainNoCommunicationLastUsableValue){
			return 68;
		}else if(statusCode==opcua.StatusCodes.UncertainSensorNotAccurate){
			return 80;
		}else if(statusCode==opcua.StatusCodes.UncertainEngineeringUnitsExceeded){
			return 84;
		}else if(statusCode==opcua.StatusCodes.UncertainSubNormal){
			return 88;
		}else if(statusCode==opcua.StatusCodes.GoodLocalOverride){
			return 216;
		}else if(statusCode==opcua.StatusCodes.BadTooManyMatches){
			return 65536;
		}else if(statusCode==opcua.StatusCodes.GoodEntryInserted){
			return 131072;
		}else if(statusCode==opcua.StatusCodes.BadBoundNotFound){
			return 1048576;
		}else if(statusCode==opcua.StatusCodes.GoodNoData){
			return 2097152;
		}else if(statusCode==opcua.StatusCodes.BadDataLost){
			return 4194304;
		}
		
		return 0;
}

module.exports.qualityPs2ua = qualityPs2ua;
module.exports.qualityUa2ps = qualityUa2ps;