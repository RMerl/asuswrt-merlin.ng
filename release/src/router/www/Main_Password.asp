<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>ASUS Login</title>
<script language="JavaScript" type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/require/require.min.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script type="text/javascript" src="/js/https_redirect/https_redirect.js"></script>
<style>
body{
	font-family: Arial, MS UI Gothic, MS P Gothic, Microsoft Yahei UI, sans-serif;
}
.wrapper{
	background:#1F1F1F url(images/New_ui/login_bg.png) no-repeat center center fixed;
	background-size: cover;
}
.button{
	background-color: #279FD9;
	border: 0px;
	border-radius: 4px ;
	transition: visibility 0s linear 0.218s,opacity 0.218s,background-color 0.218s;
	height: 48px;
	width: 200px;
	font-size: 14pt;
	color:#fff;
	text-align:center;
	vertical-align:center;
	cursor:pointer;
}
.form_input{
	background-color:rgba(255,255,255,0.2);
	background-color:#576D73\9;
	border-radius: 4px;
	padding:13px 11px;
	width: 380px;
	border: 0;
	height:25px;
	color:#fff;
	font-size:16px
}
.main_content{
	width: 100%;
	max-width: 610px;
	color: #FFF;
}
.main_content > div{
	position: relative;
	margin: 35px 0px 0px 78px;
}
.main_content .title_name{
	font-size: 30pt;
	color:#93d2d9;
}
.main_content .title_name:before{
	position: absolute;
	content: "";
	background-image: url(/images/New_ui/icon_titleName.png);
	width: 73px;
	height: 73px;
	background-repeat: no-repeat;
	left: -73px;
	background-position: 0% 50%;
}
.main_content .sub_title_name{
	font-size: 16pt;
	color: #fff;
}
.main_content .ie_title{
	display: none;
	margin: 20px 0px -20px 78px;
}
.main_content .sub_title_name > div:first-child{
	margin-bottom: 10px;
}
.main_content #error_status_field{
	color: rgb(255, 204, 0);
	margin: 10px 0px -10px 78px;
	display: none;
}
.main_content .btn_bg{
	display: flex;
	justify-content: flex-end;
	height: 68px;
}
.main_content .btn_bg #loadingIcon{
	background-image: url(/images/InternetScan.gif);
	width: 300px;
	background-repeat: no-repeat;
	background-size: 35px 35px;
	background-position: 50%;
	display: none;
}

.main_content .btn_bg > div{
	margin-right: 8px;
	height: 100%;
}

.businessInput{
	background-color: #CCC;
}
/*for mobile device*/
@media screen and (max-width: 1000px){
	.main_content{
		max-width: 1000px;
		position: absolute;
		left: 0;
		top: 30px;
		min-width: 300px;
	}
	.main_content > div{
		margin: 30px 0 0 30px;
		width: calc(100% - 80px);
	}
	.main_content .title_name{
		font-size: 20pt;
		color: #93d2d9;
		margin-left: 70px !important;
	}
	.main_content .sub_title_name{
		font-size: 12pt;
		margin-bottom: -12px;
	}
	.main_content .ie_title{
		margin: 20px 0px -20px 30px;
	}
	.main_content .title_name:before{
		width: 30px;
		height: 30px;
		left: -40px;
		background-size: contain;
	}
	.main_content #error_status_field{
		margin: 10px 0px -10px 30px;
	}
	.main_content .btn_bg{
		justify-content: center;
	}
	.main_content .btn_bg > div{
		width: 100%;
		margin-right: initial;
	}
	.form_input{
		padding: 10px 11px;
		width: 100%;
		height: 30px;
		font-size: 16px;
	}
	.button{
		height: 50px;
		font-size: 14pt;
		width: calc(100% - -22px);
	}
}
</style>
<script>
/* String splice function */
String.prototype.splice = function( idx, rem, s ) {
    return (this.slice(0,idx) + s + this.slice(idx + Math.abs(rem)));
};

/* String repeat function */
String.prototype.repeat = function(times) {
   return (new Array(times + 1)).join(this);
};

String.prototype.strReverse = function() {
	var newstring = "";
	for (var s=0; s < this.length; s++) {
		newstring = this.charAt(s) + newstring;
	}
	return newstring;
	//strOrig = ' texttotrim ';
	//strReversed = strOrig.revstring();
};

var is_AA_sku = (function(){
        var ttc = '<% nvram_get("territory_code"); %>';
        return (ttc.search("AA") == -1) ? false : true;
})();	
var is_KR_sku = (function(){
	var ttc = '<% nvram_get("territory_code"); %>';
	return (ttc.search("KR") == -1) ? false : true;
})();
var is_SG_sku = (function(){
	var ttc = '<% nvram_get("territory_code"); %>';
	return (ttc.search("SG") == -1) ? false : true;
})();
var isIE8 = navigator.userAgent.search("MSIE 8") > -1; 
var isIE9 = navigator.userAgent.search("MSIE 9") > -1; 
var defaultPass = ("<% check_pw(); %>" == "1");
var timeZoneObj;
var productid = '<% nvram_get("productid"); %>';
var wans_mode = '<% nvram_get("wans_mode"); %>';
var reboot_time = parseInt("<% get_default_reboot_time(); %>");
function isSupport(_ptn){
	var ui_support = [<% get_ui_support(); %>][0];
	return (ui_support[_ptn]) ? ui_support[_ptn] : 0;
}

var gobi_support = isSupport("gobi");

function initial(){
	top.name = "";/* reset cache of state.js win.name */

	if(isSupport("BUSINESS")){
		$(".title_name").css({"color": "#000"})
		$(".sub_title_name").css({"color": "#000"})
		$(".form_input").css({
			"color": "#000",
			"border": "1px solid #ccc"
		})
		$(".businessStyle").css({"color": "#000"})
	}

	if(is_KR_sku || is_SG_sku || is_AA_sku)
		$("#KRHint").show();

	if(isIE8 || isIE9){
		$(".ie_title").show();
	}

	var windowHeight = (function(){
		if(window.innerHeight)
			return window.innerHeight;
		else if(document.body && document.body.clientHeight)
			return document.body.clientHeight;
		else if(document.documentElement && document.documentElement.clientHeight)
			return document.documentElement.clientHeight;
		else
			return 800;
	})();

	document.getElementById("loginTable").style.height = (windowHeight-16) + "px";
	document.getElementById("loginTable").style.display = "";
	document.form.http_username_x.focus();

	if(navigator.userAgent.search("MSIE 8") === -1){
		document.form.http_username_x.onkeyup = function(e){
			if(e.keyCode == 13){
				document.form.http_passwd_x.focus();
			}
		};

		document.form.http_passwd_x.onkeyup = function(e){
			if(e.keyCode == 13){
				document.form.http_passwd_2_x.focus();
			}
		};

		document.form.http_passwd_2_x.onkeyup = function(e){
			if(e.keyCode == 13){
				submitForm();
			}
		};
	}

	if(defaultPass){
		require(['/require/modules/timeZone.js'], function(timeZone) {
			var preferredLang = "<% nvram_get("preferred_lang"); %>";
			timeZoneObj = new timeZone.get(preferredLang);

			document.form.time_zone.value = timeZoneObj.time_zone;
			document.form.time_zone.disabled = false;
			document.form.time_zone_dst.value = (timeZoneObj.hasDst) ? 1 : 0;
			document.form.time_zone_dst.disabled = !timeZoneObj.hasDst;
			document.form.action_script.value = "restart_time";
		});
	}
}

// ---------- Viz add common string check for password 2015.09 start--------
function check_common_string(pwd, flag){
	//Sequential
	var termAlphas = "abcdefghijklmnopqrstuvwxyz";
	var termNumerics = "01234567890";
	var termSymbols = "~!@#$%^&*()_+";
	var termKeyboards1 = "qwertyuiop";
	var termKeyboards2 = "asdfghjkl";
	var termKeyboards3 = "zxcvbnm";
	var termCommon5 = ["123123","abc123","letmein","master","qazwsx","admin"];
	var termCommon8 = ["adminpassword","loginpassword","passw0rd","password","useradmin","userpassword"];
	var nSeqString = 0;
	if(flag == "httpd_password"){	//at lease length 5		
		if(termAlphas.toLowerCase().indexOf(pwd) != -1 || termAlphas.strReverse().toLowerCase().indexOf(pwd) != -1) { nSeqString++; }
		if(termNumerics.toLowerCase().indexOf(pwd) != -1 || termNumerics.strReverse().toLowerCase().indexOf(pwd) != -1) { nSeqString++; }
		if(termSymbols.toLowerCase().indexOf(pwd) != -1 || termSymbols.strReverse().toLowerCase().indexOf(pwd) != -1) { nSeqString++; }
		if(termKeyboards1.toLowerCase().indexOf(pwd) != -1 || termKeyboards1.strReverse().toLowerCase().indexOf(pwd) != -1) { nSeqString++; }
		if(termKeyboards2.toLowerCase().indexOf(pwd) != -1 || termKeyboards2.strReverse().toLowerCase().indexOf(pwd) != -1) { nSeqString++; }
		if(termKeyboards3.toLowerCase().indexOf(pwd) != -1 || termKeyboards3.strReverse().toLowerCase().indexOf(pwd) != -1) { nSeqString++; }
		for(var s=0;s<termCommon5.length;s++){
			if(pwd == termCommon5[s])	{ nSeqString++; }	
		}
		for(var t=0;t<termCommon8.length;t++){
			if(pwd == termCommon8[t])	{ nSeqString++; }	
		}		
	}
	else if(flag == "wpa_key"){	//at lease length 8
		if(termAlphas.toLowerCase().indexOf(pwd) != -1 || termAlphas.strReverse().toLowerCase().indexOf(pwd) != -1) { nSeqString++; }
		if(termNumerics.toLowerCase().indexOf(pwd) != -1 || termNumerics.strReverse().toLowerCase().indexOf(pwd) != -1) { nSeqString++; }
		if(termSymbols.toLowerCase().indexOf(pwd) != -1 || termSymbols.strReverse().toLowerCase().indexOf(pwd) != -1) { nSeqString++; }
		if(termKeyboards1.toLowerCase().indexOf(pwd) != -1 || termKeyboards1.strReverse().toLowerCase().indexOf(pwd) != -1) { nSeqString++; }
		if(termKeyboards2.toLowerCase().indexOf(pwd) != -1 || termKeyboards2.strReverse().toLowerCase().indexOf(pwd) != -1) { nSeqString++; }
		for(var s=0;s<termCommon8.length;s++){
			if(pwd == termCommon8[s])	{ nSeqString++; }	
		}		
	}
	
	//pure repeat character string
	if(pwd == pwd.charAt(0).repeat(pwd.length)) { nSeqString++; }
	
	if(nSeqString > 0)
		return true;
	else		
		return false;
}
// ---------- Viz add common string check for password 2015.09 end--------

function validForm(){
	if(!validator.chkLoginId(document.form.http_username_x)){
		return false;
	}
	
	if($("#defpassCheckbox").prop('checked')) return true;
	
	if(document.form.http_passwd_x.value == ""){
			showError("<#File_Pop_content_alert_desc6#>");
			document.form.http_passwd_x.value = "";
			document.form.http_passwd_x.focus();
			document.form.http_passwd_x.select();
			return false;
		}
		
	if(document.form.http_passwd_x.value != document.form.http_passwd_2_x.value){
			showError("<#File_Pop_content_alert_desc7#>");
			document.form.http_passwd_x.value = "";
			document.form.http_passwd_x.focus();
			document.form.http_passwd_x.select();
			return false;                   
	}

	if(is_KR_sku || is_SG_sku || is_AA_sku){		/* MODELDEP by Territory Code */
		if(!validator.chkLoginPw_KR(document.form.http_passwd_x)){
			return false;
		}
		if(document.form.http_passwd_x.value == document.form.http_username_x.value){
			alert("<#JS_validLoginPWD#>");
			document.form.http_passwd_x.focus();
			document.form.http_passwd_x.select();
			return false;	
		}
	}
	else{
		if(!validator.chkLoginPw(document.form.http_passwd_x)){
			return false;
		}
	}
	
	if(document.form.http_passwd_x.value == '<% nvram_default_get("http_passwd"); %>'){
			showError("<#QIS_adminpass_confirm0#>");
			document.form.http_passwd_x.value = "";
			document.form.http_passwd_x.focus();
			document.form.http_passwd_x.select();
			return false;
	}
	
	//confirm common string combination	#JS_common_passwd#
	var is_common_string = check_common_string(document.form.http_passwd_x.value, "httpd_password");
	if(document.form.http_passwd_x.value.length > 0 && is_common_string){
		if(!confirm("<#JS_common_passwd#>")){
			document.form.http_passwd_x.focus();
			document.form.http_passwd_x.select();
			return false;	
		}	
	}

	return true;	
}

var showLoading_time = 3000;
function submitForm(){
	var postData = {
		"restart_httpd": "0", 
		"new_username":document.form.http_username_x.value, 
		"new_passwd":document.form.http_passwd_x.value,
		"defpass_enable": $("#defpassCheckbox").prop('checked') ? "1" : "0"
	};
	
	var sw_mode = '<% nvram_get("sw_mode"); %>';

	if(sw_mode == 3 && '<% nvram_get("wlc_psta"); %>' == 2)
		sw_mode = 2;

	if(validForm()){
		$("#error_status_field").hide();
		$("#btn_modify").hide();
		$("#loadingIcon").show();

		if(defaultPass && gobi_support && wans_mode == "lb"){
			var form = document.getElementsByName("form")[0];
			var input = document.createElement("input");
			var rc_service = document.createElement("input");
			rc_service.setAttribute("type", "hidden");
			rc_service.setAttribute("name", "rc_service");
			rc_service.setAttribute("value", "reboot");
			form.appendChild(rc_service);
			input.setAttribute("type", "hidden");
			input.setAttribute("name", "wans_mode");
			input.setAttribute("value", "fo");
			form.appendChild(input);
			form.action = "/apply.cgi";
			showLoading_time = reboot_time * 1000;
		}
		document.form.submit();

		setTimeout(function(){
			httpApi.chpass(postData);
		}, 100);

		setTimeout(function(){
			if('<% nvram_get("w_Setting"); %>' == '0' && sw_mode != 2)
				location.href = '/QIS_wizard.htm?flag=wireless';
			else
				location.href = "/";
		}, showLoading_time);
	}
	else
		return;
}


var validator = {
	chkLoginId: function(obj){
		var re = new RegExp("^[a-zA-Z0-9][a-zA-Z0-9\-\_]+$","gi");

		if(obj.value == ""){
			showError("<#File_Pop_content_alert_desc1#>");
			obj.value = "";
			obj.focus();
			obj.select();
			return false;
		}
		else if(re.test(obj.value)){
			if(obj.value == "root" || obj.value == "guest" || obj.value == "anonymous"){
				showError("<#USB_Application_account_alert#>");
				obj.value = "";
				obj.focus();
				obj.select();
				return false;
			}

			return true;
		}
		else{
			if(obj.value.length < 2)
				showError("<#JS_short_username#>");
			else
				showError("<#JS_validhostname#>");
			obj.value = "";
			obj.focus();
			obj.select();
			return false;
		}
	},

	chkLoginPw: function(obj){
		
		if(obj.value.length > 0 && obj.value.length < 5){
			showError("<#JS_short_password#> <#JS_password_length#>");
			obj.value = "";
			obj.focus();
			obj.select();
			return false;
		}
		
		if(obj.value.length > 32){
            showError("<#JS_max_password#>");
            obj.value = "";
            obj.focus();
            obj.select();
            return false;
        }

		if(obj.value.charAt(0) == '"'){
			showError('<#JS_validstr1#> ["]');
			obj.value = "";
                        obj.focus();
                        obj.select();
                        return false;
                }
                else if(obj.value.charAt(obj.value.length - 1) == '"'){
                        showError('<#JS_validstr3#> ["]');
			obj.value = "";
			obj.focus();
			obj.select();
                        return false;
                }
		else{
			var invalid_char = ""; 
			for(var i = 0; i < obj.value.length; ++i){
				if(obj.value.charAt(i) < ' ' || obj.value.charAt(i) > '~'){
					invalid_char = invalid_char+obj.value.charAt(i);
				}
			}

			if(invalid_char != ""){
				showError("<#JS_validstr2#> '"+invalid_char+"' !");
				obj.value = "";
				obj.focus();
				obj.select();
				return false;
			}
		}		

		return true;
	},
	
	chkLoginPw_KR: function(obj){		//KR: Alphabets, numbers, specialcharacters mixed. 8 chars at least.
						//S2: Mixed 2 out of Alphabets(Upper/Lower case), numbers, specialcharacters.
						//    10 chars at least. Not have consecutive identical characters.
		var string_length = obj.value.length;		
		
		if(!/[A-Za-z]/.test(obj.value) || !/[0-9]/.test(obj.value) || string_length < 10
				|| !/[\!\"\#\$\%\&\'\(\)\*\+\,\-\.\/\:\;\<\=\>\?\@\[\\\]\^\_\`\{\|\}\~]/.test(obj.value)
				|| /([A-Za-z0-9\!\"\#\$\%\&\'\(\)\*\+\,\-\.\/\:\;\<\=\>\?\@\[\\\]\^\_\`\{\|\}\~])\1/.test(obj.value)
		){
				
			showError("<#JS_validLoginPWD#>");
			obj.value = "";
			obj.focus();
			obj.select();
			return false;	
		}
		
		if(obj.value.length > 32){
			showError("<#JS_max_password#>");
			obj.value = "";
			obj.focus();
			obj.select();
			return false;
		}

		if(obj.value.charAt(0) == '"'){
			showError('<#JS_validstr1#> ["]');
			obj.value = "";
			obj.focus();
			obj.select();
			return false;
		}
		else if(obj.value.charAt(obj.value.length - 1) == '"'){
			showError('<#JS_validstr3#> ["]');
			obj.value = "";
			obj.focus();
			obj.select();
			return false;
		}

		var invalid_char = "";
		for(var i = 0; i < obj.value.length; ++i){
			if(obj.value.charAt(i) <= ' ' || obj.value.charAt(i) > '~'){
				invalid_char = invalid_char+obj.value.charAt(i);
			}
		}

		if(invalid_char != ""){
			showError("<#JS_validstr2#> '"+invalid_char+"' !");
			obj.value = "";
			obj.focus();
			obj.select();
			return false;
		}
		
		return true;
	}
}

function showError(str){
	$("#error_status_field").show();
	$("#error_status_field").html(str);
}
</script>
</head>
<body class="wrapper" onload="initial();">
<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="saveNvram;restart_chpass">
<input type="hidden" name="action_wait" value="0">
<input type="hidden" name="current_page" value="Main_Password.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="flag" value="">
<input type="hidden" name="login_authorization" value="">
<input name="foilautofill" style="display: none;" type="password">
<input type="hidden" name="time_zone" value="" disabled>
<input type="hidden" name="time_zone_dst" value="" disabled>
<input type="hidden" name="cfg_pause" value="0">
<table id="loginTable" align="center" cellpadding="0" cellspacing="0" style="display:none">
	<tr>
		<td>
			<div class="main_content">
				<div class="title_name"><#PASS_changepasswd#></div>
				<div class="sub_title_name">
					<div>
						<#QIS_pass_desc1#>
					</div>
					<div id="KRHint" style="display:none">
						<#JS_validLoginPWD#>
					</div>
				</div>
				<div id="router_name_tr" class="ie_title">
					<div><#Router_Login_Name#></div>
				</div>
				<div>
					<input type="text" name="http_username_x" tabindex="1" class="form_input" maxlength="32" value="" autocapitalize="off" autocomplete="off" placeholder="<#Router_Login_Name#>">
				</div>
				<div id="router_password_tr" class="ie_title">
					<div><#PASS_new#></div>
				</div>
				<div>
					<input type="password" autocapitalize="off" autocomplete="off" value="" name="http_passwd_x" tabindex="2" class="form_input" maxlength="33" onkeyup="" onpaste="return false;"/ onBlur="" placeholder="<#PASS_new#>">
				</div>
				<div id="router_password_confirm_tr" class="ie_title" >
					<div><#Confirmpassword#></div>
				</div>
				<div>
					<input type="password" autocapitalize="off" autocomplete="off" value="" name="http_passwd_2_x" tabindex="3" class="form_input" maxlength="33" onkeyup="" onpaste="return false;"/ onBlur="" placeholder="<#Confirmpassword#>">
				</div>
				<div style="font-size: 16pt; display:none" class="businessStyle">
					<input id="defpassCheckbox" type="checkbox" style="height:30px;width:30px;vertical-align: middle;">Use the default settings
				</div>
				<script>
					$("#defpassCheckbox").change(function(){
						var status = $(this).is(':checked');
						if(status){
							$("[name='http_passwd_x']")
								.val("")
								.prop('disabled', true)
								.css({opacity: "0.3"})

							$("[name='http_passwd_2_x']")
								.val("")
								.prop('disabled', true)
								.css({opacity: "0.3"})

							$("[name='http_passwd_x']").addClass("businessInput")
							$("[name='http_passwd_2_x']").addClass("businessInput")
						}
						else{
							$("[name='http_passwd_x']")
								.prop('disabled', false)
								.css({opacity: "1"})

							$("[name='http_passwd_2_x']")
								.prop('disabled', false)
								.css({opacity: "1"})						

							$("[name='http_passwd_x']").removeClass("businessInput")
							$("[name='http_passwd_2_x']").removeClass("businessInput")
						}
					})

					if(isSupport("defpass")){
						$("#defpassCheckbox").parent().show();
						$("#defpassCheckbox").prop('checked', true).change()
					}
				</script>

				<div id="error_status_field"></div>
				<div class="btn_bg">
					<div id="btn_modify">
						<input name="btn_modify" type="button" class="button" tabindex="4" onclick="submitForm();" value="<#CTL_modify#>">
					</div>
					<div id="loadingIcon"></div>
				</div>
			</div>
		</td>
	</tr>
</table>
</form>
</body>
</html>

