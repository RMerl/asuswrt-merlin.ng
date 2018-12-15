<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
<link rel="icon" href="images/favicon.png">
<title>ASUS Login</title>
<style>
html{
	background: #005544;
}
body{
	font-family: Arial;
}
.wrapper{
	background-size: 1280px 1076px;
	background-position: center 0%;
	margin: 0px; 
	background: linear-gradient(to bottom, #008866, #005544);
	
}
.title_name {
	font-size: 40pt;
	color:#93d2d9;
}
.prod_madelName{
	font-size: 26pt;
	color:#fff;
	margin-left:78px;
	margin-top: 10px;
}
.login_img{
	width:43px;
	height:43px;
	background-image: url('images/New_ui/icon_titleName.png');
	background-repeat: no-repeat;
}
.p1{
	font-size: 16pt;
	color:#fff;
	width:480px;
}
.button{
	background-color:rgba(0,0,0,.8);
	border-radius: 4px ;
	transition: visibility 0s linear 0.218s,opacity 0.218s,background-color 0.218s;
	height: 68px;
	width: 300px;
	font-size: 28pt;
	color:#fff;
	text-align: center;
	float:right; 
	margin:50px 0px 0px 78px;
	line-height:68px;
	cursor:pointer;
}
.form_input{
	background: rgba(51,51,51,.6);
	background: rgb(51,51,51)\9;
	border-radius: 4px;
	padding:23px 22px;
	width: 480px;
	border: 0;
	height:30px;
	color:#fff;
	font-size:28px;
}
.nologin{
	margin:10px 0px 0px 78px;
	background-color:rgba(51,51,51,0.4);
	padding:20px;
	line-height:36px;
	border-radius: 5px;
	width: 480px;
	border: 0;
	color:#FFF;
	color:#FFF\9; /* IE6 IE7 IE8 */
	font-size:28px;
}
.div_table{
	display:table;
}
.div_tr{
	display:table-row;
}
.div_td{
	display:table-cell;
}
.title_gap{
	margin:20px 0px 0px 78px;
}
.img_gap{
	padding-right:30px;
	vertical-align:middle;
}
.password_gap{
	margin:30px 0px 0px 78px;
}
.error_hint{
	color: rgb(255, 204, 0);
	margin:10px 0px -10px 78px; 
	font-size: 18px;
}

.error_hint1{
	margin:40px 0px -10px 78px; 
	font-size: 24px;
	line-height:32px;
	width: 580px;
}

.main_field_gap{
	margin:100px auto 0;
}
.warming_desc{
	font-size: 16px;
	color:#FC0;
	width: 600px;
}

/*for mobile device*/
@media screen and (max-width: 1000px){
	.title_name {
		font-size: 20pt;
		color:#93d2d9;
		margin-left:15px;
	}
	.prod_madelName{
		font-size: 13pt;
		margin-left: 15px;
	}
	.p1{
		font-size: 12pt;
		width:100%;
	}
	.login_img{
		background-size: 75%;
	}
	.form_input{	
		padding:10px 11px;
		width: 100%;
		height:30px;
		font-size:16px
	}
	.button{
		height: 50px;
		width: 100%;
		font-size: 14pt;
		text-align: center;
		float:right; 
		margin: 25px -22px 40px 15px;
		line-height:50px;
		padding-left: 7px;
	}
	.nologin{
		margin-left:10px; 
		padding:10px;
		line-height:18px;
		width: 100%;
		font-size:14px;
	}
	.error_hint{
		margin-left:10px; 
	}
	.error_hint1{
		width: 100%;
		font-size:14px;
	}
	.main_field_gap{
		width:80%;
		margin:30px 0 0 15px;
		/*margin:30px auto 0;*/
	}
	.title_gap{
		margin-left:15px; 
	}
	.password_gap{
		margin-left:15px; 
	}
	.img_gap{
		padding-right:0;
		vertical-align:middle;
	}
	.warming_desc{
		margin: 10px 15px;
		width: 100%; 
	}
}
</style>
<script>
/* add Array.prototype.forEach() in IE8 */
if(typeof Array.prototype.forEach != 'function'){
	Array.prototype.forEach = function(callback){
		for(var i = 0; i < this.length; i++){
			callback.apply(this, [this[i], i, this]);
		}
	};
}

function tryParseJSON (jsonString){
    try {
        var o = JSON.parse(jsonString);

        if (o && typeof o === "object") {
            return o;
        }
    }
    catch (e) { }

    return false;
};

var login_info =  tryParseJSON('<% login_error_info(); %>');
var isIE8 = navigator.userAgent.search("MSIE 8") > -1; 
var isIE9 = navigator.userAgent.search("MSIE 9") > -1; 
var remaining_time = login_info.lock_time;
var remaining_time_min;
var remaining_time_sec;
var remaining_time_show;
var countdownid, rtime_obj;
var redirect_page = login_info.page;
var isRouterMode = ('<% nvram_get("sw_mode"); %>' == '1') ? true : false;

var header_info = [<% get_header_info(); %>][0];
var ROUTERHOSTNAME = '<% nvram_get("local_domain"); %>';
var domainNameUrl = header_info.protocol+"://"+ROUTERHOSTNAME+":"+header_info.port;
var chdom = function(){window.location.href=domainNameUrl};
(function(){
	if(ROUTERHOSTNAME !== header_info.host && ROUTERHOSTNAME != "" && isRouterMode){
		setTimeout(function(){
			var s=document.createElement("script");s.type="text/javascript";s.src=domainNameUrl+"/chdom.json?hostname="+header_info.host;var h=document.getElementsByTagName("script")[0];h.parentNode.insertBefore(s,h);
		}, 1);
	}
})();

function initial(){
	var flag = login_info.error_status;
	if(isIE8 || isIE9){
		document.getElementById("name_title_ie").style.display ="";
		document.getElementById("password_title_ie").style.display ="";
	}

	if(flag != ""){
		document.getElementById("error_status_field").style.display ="";

		if(flag == 3){
			document.getElementById("error_status_field").innerHTML ="* <#JS_validLogin#>";
		}
		else if(flag == 7){
			document.getElementById("error_status_field").innerHTML ="You have entered an incorrect username or password 5 times. Please try again after "+"<span id='rtime'></span>"+" seconds.";
			document.getElementById("error_status_field").className = "error_hint error_hint1";
			disable_input(1);
			disable_button(1);
			rtime_obj=document.getElementById("rtime");
			countdownfunc();
			countdownid = window.setInterval(countdownfunc,1000);
		}
		else if(flag == 8){
			document.getElementById("login_filed").style.display ="none";
			document.getElementById("logout_field").style.display ="";
		}
		else if(flag == 9){
			document.getElementById("login_filed").style.display ="none";
			document.getElementById("nologin_field").style.display ="";
		}
		else{
			document.getElementById("error_status_field").style.display ="none";
		}
	}

	document.form.login_username.focus();

	/*register keyboard event*/
	document.form.login_username.onkeyup = function(e){
		e=e||event;
		if(e.keyCode == 13){
			document.form.login_passwd.focus();
			return false;
		}
	};
	document.form.login_username.onkeypress = function(e){
		e=e||event;
		if(e.keyCode == 13){
			return false;		}
	};

	document.form.login_passwd.onkeyup = function(e){
		e=e||event;
		if(e.keyCode == 13){
			login();
			return false;
		}
	};
	document.form.login_passwd.onkeypress = function(e){
		e=e||event;
		if(e.keyCode == 13){
			return false;
		}
	};

	if(history.pushState != undefined) history.pushState("", document.title, window.location.pathname);
}

function countdownfunc(){
	remaining_time_min = checkTime(Math.floor(remaining_time/60));
	remaining_time_sec = checkTime(Math.floor(remaining_time%60));
	remaining_time_show = remaining_time_min +":"+ remaining_time_sec;
	rtime_obj.innerHTML = remaining_time_show;
	if (remaining_time==0){
		clearInterval(countdownid);
		setTimeout("top.location.href='/Main_Login.asp';", 2000);
	}
	remaining_time--;
}

function login(){

	var trim = function(val){
		val = val+'';
		for (var startIndex=0;startIndex<val.length && val.substring(startIndex,startIndex+1) == ' ';startIndex++);
		for (var endIndex=val.length-1; endIndex>startIndex && val.substring(endIndex,endIndex+1) == ' ';endIndex--);
		return val.substring(startIndex,endIndex+1);
	}

	if(!window.btoa){
		window.btoa = function(input){
			var keyStr = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
			var output = "";
			var chr1, chr2, chr3, enc1, enc2, enc3, enc4;
			var i = 0;
			var utf8_encode = function(string) {
				string = string.replace(/\r\n/g,"\n");
				var utftext = "";
				for (var n = 0; n < string.length; n++) {
					var c = string.charCodeAt(n);
					if (c < 128) {
						utftext += String.fromCharCode(c);
					}
					else if((c > 127) && (c < 2048)) {
						utftext += String.fromCharCode((c >> 6) | 192);
						utftext += String.fromCharCode((c & 63) | 128);
					}
					else {
						utftext += String.fromCharCode((c >> 12) | 224);
						utftext += String.fromCharCode(((c >> 6) & 63) | 128);
						utftext += String.fromCharCode((c & 63) | 128);
					}
				}
				return utftext;
			};
			input = utf8_encode(input);
			while (i < input.length) {
				chr1 = input.charCodeAt(i++);
				chr2 = input.charCodeAt(i++);
				chr3 = input.charCodeAt(i++);
				enc1 = chr1 >> 2;
				enc2 = ((chr1 & 3) << 4) | (chr2 >> 4);
				enc3 = ((chr2 & 15) << 2) | (chr3 >> 6);
				enc4 = chr3 & 63;
				if (isNaN(chr2)) {
					enc3 = enc4 = 64;
				}
				else if (isNaN(chr3)) {
					enc4 = 64;
				}
				output = output + 
				keyStr.charAt(enc1) + keyStr.charAt(enc2) + 
				keyStr.charAt(enc3) + keyStr.charAt(enc4);
			}
			return output;
		};
	}

	document.form.login_username.value = trim(document.form.login_username.value);
	document.form.login_authorization.value = btoa(document.form.login_username.value + ':' + document.form.login_passwd.value);
	document.form.login_username.disabled = true;
	document.form.login_passwd.disabled = true;

	try{
		if(redirect_page == "" 
			|| redirect_page == "Logout.asp" 
			|| redirect_page == "Main_Login.asp" 
			|| redirect_page.indexOf(" ") != -1 
			|| redirect_page.indexOf("//") != -1 
			|| redirect_page.indexOf("http") != -1
			|| (redirect_page.indexOf(".asp") == -1 && redirect_page.indexOf(".htm") == -1 && redirect_page != "send_IFTTTPincode.cgi" && redirect_page != "cfg_onboarding.cgi")
		){
			document.form.next_page.value = "<% rel_index_page(); %>";
		}
		else{
			document.form.next_page.value = redirect_page;
		}
	}
	catch(e){
		document.form.next_page.value = "<% rel_index_page(); %>";
	}

	document.form.submit();
}

function disable_input(val){
	var disable_input_x = document.getElementsByClassName('form_input');
	for(i=0;i<disable_input_x.length;i++){
		if(val == 0)
			disable_input_x[i].disabled = true;
		else
			disable_input_x[i].style.display = "none";
	}
}

function disable_button(val){
	if(val == 0)
		document.getElementsByClassName('button')[0].disabled = true;
	else
		document.getElementsByClassName('button')[0].style.display = "none";
}

function checkTime(i){
	if (i<10){
		i="0" + i
	}
	return i
}
</script>
</head>
<body class="wrapper" onload="initial();">
<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0"></iframe>
<iframe id="dmRedirection" width="0" height="0" frameborder="0" scrolling="no" src=""></iframe>

<form method="post" name="form" action="login.cgi" target="">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="current_page" value="Main_Login.asp">
<input type="hidden" name="next_page" value="Main_Login.asp">
<input type="hidden" name="login_authorization" value="">
<div class="div_table main_field_gap">
	<div class="div_tr">
		<div class="title_name">
			<div class="div_td img_gap">
				<div class="login_img"></div>
			</div>
			<div class="div_td"><#CTL_signin#></div>
		</div>	
		<div class="prod_madelName"><#Web_Title2#></div>

		<!-- Login field -->
		<div id="login_filed">
			<div class="p1 title_gap"><#Sign_in_title#></div>

			<div id="name_title_ie" style="display:none;margin:20px 0 -10px 78px;" class="p1 title_gap"><#HSDPAConfig_Username_itemname#></div>
			<div class="title_gap">
				<input type="text" id="login_username" name="login_username" tabindex="1" class="form_input" maxlength="20" autocapitalize="off" autocomplete="off" placeholder="<#HSDPAConfig_Username_itemname#>">
			</div>
			<div id="password_title_ie" style="display:none;margin:20px 0 -20px 78px;" class="p1 title_gap"><#HSDPAConfig_Password_itemname#></div>
			<div class="password_gap">
				<input type="password" name="login_passwd" tabindex="2" class="form_input" maxlength="16" placeholder="<#HSDPAConfig_Password_itemname#>" autocapitalize="off" autocomplete="off">
			</div>
			<div class="error_hint" style="display:none;" id="error_status_field"></div>
				<div class="button" onclick="login();"><#CTL_signin#></div>
		</div>

		<!-- No Login field -->
		<div id="nologin_field" style="display:none;">
			<div class="p1 title_gap"></div>
			<div class="nologin">
				<#login_hint2#>
				<div id="logined_ip_str"></div>
			</div>
		</div>

		<!-- Logout field -->
		<div id="logout_field" style="display:none;">
			<div class="p1 title_gap"></div>
			<div class="nologin"><#logoutmessage#></div>		
		</div>
	</div>
</div>
</form>
</body>
</html>
