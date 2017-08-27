<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE8" />
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta http-equiv="Expires" content="-1" />
<meta HTTP-EQUIV="Cache-Control" CONTENT="no-cache">
<meta http-equiv="Pragma" content="no-cache" />
<title>Cloud Sync</title>
<script type="text/javascript" src="jquery.js"></script>
</head>
<body>
<script>
var ua = navigator.userAgent.toLowerCase();
var blsIpad = ua.match(/ipad/i);
var blsIphoneOs = ua.match(/iphone os/i);
var blsAndroid = ua.match(/android/i);
var blsCE = ua.match(/windows ce/i);
var blsWM = ua.match(/windows mobile/i);
/*if(blsAndroid == "android" || blsIpad == "ipad" || blsIphoneOs == "iphone os" || blsCE == "windows ce" || blsWM == "windows mobile"){
	self.location = "http://"+ location.host.split(":")[0] +":8081/task_hand.asp";
	}
else{
	location.href = "http://"+ location.host.split(":")[0] +":8081/task.asp";
	}*/
	var width;
	width = window.screen.width;
	if(width <= 480){
		self.location = "http://"+ location.host.split(":")[0] +":"+ location.host.split(":")[1] +"/cloudui/task_hand.asp";
		}
	else{
			self.location = "http://"+ location.host.split(":")[0] +":"+ location.host.split(":")[1] +"/cloudui/Setting.asp";
		}
	
	//alert(ua);
</script>
</body>
</html>
