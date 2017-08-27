<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE9"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>Captive Portal Redirect</title>
<script language="JavaScript" type="text/javascript" src="/jquery.js"></script>
<script>
//fbwifi
var lan_ip= '<% nvram_get("lan_ipaddr"); %>';
var wan_ip= '<% nvram_get("wan_ipaddr"); %>';
var ddns_host = '<% nvram_get("ddns_hostname_x"); %>';
var local_domain = '<% nvram_get("local_domain"); %>';
	
</script>
<script>
var $j = jQuery.noConflict();	
var href = self.location.hostname;
	//alert(href);
	if(href != lan_ip && href != wan_ip && href != local_domain && href != ddns_host)
	{
 		$j.ajax({
			url:'fbwifi.cgi?host=' + href,
      		//url: 'fbwifi.cgi?ip='+ login_ip +"&host=" + href,
      		//type: 'POST',
	  		//data:'ip=' + login_ip +"&host=" + url_org,
			async:false,
      		success: function(response) {
				self.location = "http://" + lan_ip +":8084/fbwifi/forward.asp?u=" + href;
		}
 	});
	}
</script>
</head>
</html>
