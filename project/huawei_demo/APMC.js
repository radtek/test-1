var fs = require('fs');
var csv = require('CSV');

///////////////

var predict_path = 'C:/predict.txt';
var read_predict_ms = 100;

//////////////////////////

var last_time = 0;
function readPredict(){
	setTimeout(readPredict, read_predict_ms);
	csv().from.path(predict_path).to.array(function(rows){
		if (rows.length > 0 && rows[0].length > 0) {
			if (last_time != parseInt(rows[0][0])) {
				last_time = parseInt(rows[0][0]);
				if (last_time%30>17) {
					fs.appendFileSync(predict_path, '\n'+(parseFloat(rows[0][12])-3.66));
					console.log('小于当前值'+rows[0][12]+' 限值'+(parseFloat(rows[0][12])-3.66));
				}else{
					fs.appendFileSync(predict_path, '\n'+(parseFloat(rows[0][12])+3.66));
					console.log('大于当前值'+rows[0][12]+' 限值'+(parseFloat(rows[0][12])+3.66));					
				}	
			};
		};
	});
}

setTimeout(readPredict, read_predict_ms);