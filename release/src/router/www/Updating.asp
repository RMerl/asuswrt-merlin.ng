<html>
<head>
<title>ASUS Wireless Router Web Manager</title>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
</head>
<body>
<script>
	var reboot_needed_time = eval("<% get_default_reboot_time(); %>");
	parent.document.getElementById("hiddenMask").style.visibility = "hidden";

	//MODELDEP: 4G-AC55U, RT-AC52U, RT-AC55U, RT-AC55UHP, RT-AC56S, RT-AC56U, RT-AC66U, RT-AC68U, RT-AC3200 2014.06
	if( parent.based_modelid == "4G-AC55U"
	 || parent.based_modelid == "DSL-AC68U"
	 || parent.based_modelid == "RT-AC52U"
	 || parent.based_modelid == "RT-AC55U" || parent.based_modelid == "RT-AC55UHP"
	 || parent.based_modelid == "RT-AC56S" || parent.based_modelid == "RT-AC56U"
	 || parent.based_modelid == "RT-AC66U"
	 || parent.based_modelid == "RT-AC68U" || parent.based_modelid == "RT-AC68A" || parent.based_modelid == "4G-AC68U"
	 || parent.based_modelid == "RT-AC3200" 
	 || parent.based_modelid == "RT-AC5300" || parent.based_modelid == "GT-AC5300"
	 || parent.based_modelid == "GT-AC9600"
	 || parent.based_modelid == "RT-AC88U"  || parent.based_modelid == "RT-AX88U" || parent.based_modelid == "RT-AC3100"
	 || parent.based_modelid == "RT-AC86U" || parent.based_modelid == "AC2900" || parent.based_modelid == "GT-AX11000"
	 || parent.based_modelid == "RT-AX92U" || parent.based_modelid == "RT-AX95Q" || parent.based_modelid == "RT-AX56_XD4" || parent.based_modelid == "RT-AX58U" || parent.based_modelid == "TUF-AX3000"
	 || parent.based_modelid == "RT-AX82U" || parent.based_modelid == "RT-AX56U"
	 || parent.based_modelid == "RT-AX86U" || parent.based_modelid == "RT-AX5700" || parent.based_modelid == "RT-AX68U"
	){
		reboot_needed_time += 40;		
		parent.showLoadingBar(reboot_needed_time);
		reboot_needed_time += 2;
		setTimeout("parent.detect_httpd();", reboot_needed_time*1000);
	}
	else if(parent.based_modelid == "RT-N11P"){
		parent.showLoadingBar(160);
		setTimeout("parent.detect_httpd();", 160000);
	}
	else{
		parent.showLoadingBar(270);
		setTimeout("parent.detect_httpd();", 272000);		
	}	
</script>
</body>
</html>
