﻿<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router Web Manager</title>
<link rel="stylesheet" type="text/css" href="other.css">

<script>
if(parent.tmo_support)
        var theUrl = "cellspot.router"; 
else
        var theUrl = "<#Web_DOMAIN_NAME#>";
var reboot_needed_time = <% get_default_reboot_time(); %>;
var action_mode = '<% get_parameter("action_mode"); %>';
function redirect(){
	parent.stopFlag = 1;
	setTimeout("check_httpd();", reboot_needed_time*1000);
}

function check_httpd(){
	var httpRequest;

	if(window.XMLHttpRequest){ // Mozilla, Safari, ...
		httpRequest = new XMLHttpRequest();
		if(httpRequest.overrideMimeType){
			httpRequest.overrideMimeType('text/xml');
		}
	}
	else if(window.ActiveXObject){ // IE
	try{
		httpRequest = new ActiveXObject("Msxml2.XMLHTTP");
	}
	catch(e){
		try{
			httpRequest = new ActiveXObject("Microsoft.XMLHTTP");
		}
		catch(e){}
	}
	}

	if(!httpRequest){
		return false;
	}

	httpRequest.onreadystatechange = function() { handleResponse(httpRequest); };
	httpRequest.open('GET', "/httpd_check.xml", true);
	httpRequest.send('');
}

function handleResponse(httpRequest){

	if(httpRequest.readyState == 4){
		if(httpRequest.status == 200){
			redirect1();
		}
		else{
			setTimeout("check_httpd();", 1000);
		}
	}

}

function redirect1(){
	if(action_mode == "Restore"){
		parent.document.getElementById('drword').innerHTML = "<#Setting_factorydefault_iphint#><br/>".replace("192.168.1.1", '<% nvram_default_get("lan_ipaddr"); %>');
		setTimeout("parent.hideLoading()",1000);
		setTimeout("parent.dr_advise();",1000);
		if((!parent.tmo_support && parent.location.hostname.search('router.asus') != -1)
		||(parent.tmo_support && parent.location.hostname.search('cellspot.router') != -1))
			parent.location.href = 'http://'+ theUrl +'/QIS_default.cgi?flag=welcome';
		else		
			parent.location.href = 'http://<% nvram_default_get("lan_ipaddr"); %>/QIS_default.cgi?flag=welcome';
	}
	else{
		parent.location.href = "/";
		return false;
	}
}
</script>
</head>

<body onLoad="redirect();">
<script>
parent.hideLoading();
parent.showLoading(reboot_needed_time, "waiting");
</script>
</body>
</html>
