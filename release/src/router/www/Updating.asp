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
	var knv = "<% get_uname_release(); %>";
	var knv_threshold = (parseInt(knv[0])=="")?0:parseInt(knv[0]);
	var reboot_needed_time = eval("<% get_default_reboot_time(); %>");
	parent.document.getElementById("hiddenMask").style.visibility = "hidden";

	if(parent.Bcmwifi_support && knv_threshold >= 4){

		reboot_needed_time += 40;		
		parent.showLoadingBar(reboot_needed_time);
		setTimeout("parent.detect_httpd();", (reboot_needed_time+2)*1000);
	}
	else if(parent.based_modelid == "RT-N11P"){
		parent.showLoadingBar(160);
		setTimeout("parent.detect_httpd();", 162000);
	}
	else{
		parent.showLoadingBar(270);
		setTimeout("parent.detect_httpd();", 272000);		
	}	
</script>
</body>
</html>
