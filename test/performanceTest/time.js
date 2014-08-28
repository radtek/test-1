var time = new Date().getTime();
function Appendzero(obj)
{
     if(obj<10) return "0" +""+ obj;
     else return obj;
     
}
function   formatDate(now)   {     
          var   year=now.getFullYear();     
          var   month=now.getMonth()+1;     
          var   date=now.getDate();     
          var   hour=now.getHours();     
          var   minute=now.getMinutes();     
          var   second=now.getSeconds(); 
          var   milSecond = now.getMilliseconds();    
          return   year+"-"+Appendzero(month)+"-"+Appendzero(date)+" "+Appendzero(hour)+":"+Appendzero(minute)+":"+Appendzero(second)+"."+Appendzero(milSecond);     
          }     
var   d=new   Date(time);     
console.log(formatDate(d));  

function getLocalTime(nS) {     
   return new Date(parseInt(nS) * 1000).toLocaleString().replace(/年|月/g, "-").replace(/日/g, " ");      
}     
console.log(getLocalTime(time)); 