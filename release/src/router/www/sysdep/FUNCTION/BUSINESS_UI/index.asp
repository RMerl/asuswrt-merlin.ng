<html style="height:100%; overflow: hidden;">
<body style= "height:100vh; overflow-y:auto; margin-right:-15px;">
<script>
	if(navigator.userAgent.search("asusrouter") == -1){
		var notice_pw_is_default = '<% check_pw(); %>';
		if(notice_pw_is_default == 1 && window.location.pathname.toUpperCase().search("QIS_") < 0) //force to change http_passwd / http_username & except QIS settings
			location.href = 'Main_Password.asp?nextPage=' + window.location.pathname.substring(1 ,window.location.pathname.length);
		else if('<% nvram_get("w_Setting"); %>' == '0' && '<% nvram_get("sw_mode"); %>' != 2 && window.location.pathname.toUpperCase().search("QIS_") < 0)
			location.href = '/QIS_wizard.htm?flag=wireless';
		else
			location.href = "/index.html?url=dashboard&current_theme=white";
	}
</script>
</body>
</html>
