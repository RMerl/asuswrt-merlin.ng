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
<title><#Web_Title#></title>
<style>
body, .p1, .form-input{
	color: #FFF;
}
body{
	font-family: Roboto, STHeiti, Microsoft JhengHei, sans-serif, Arial;
	background-color: rgb(31, 31, 31) !important;
}
.bg{
	background: url(/images/New_ui/login_bg.png) no-repeat center center fixed;
	-webkit-background-size: cover;
	-moz-background-size: cover;
	-o-background-size: cover;
	background-size: cover;
	background:#283437\9;
}
.main-field-bg{
	margin:20px auto 0;
	width: 887px;
	height: 849px;
}
.main-field-padding{
	width: 887px;
	margin: 0 auto;	
}
.logo-container{
	display: flex;
	align-items: center;
	margin-left: 90px;
}
.logo-rog{
	background:url('./images/New_ui/ROG-logo.png') no-repeat;
}
.logo-rog{
	width: 290px;
	height: 191px;
}
.login-title{
	width: 420px;
	height: 100%;
	font-size: 48px;
	margin-left: 25px;
	color: #93d2d9;
}
.login-title-desc{
	font-size: 16pt;
	margin: 15px 30px 36px 120px;
}
.desc{
	margin: 12px 0;
}
.input-container{
	display: flex;
	justify-content: center;
	margin: 0 0 20px 0;
}

.login-btn-bg{
	height: 60px;
	line-height: 60px;
	border: 2px solid #842500;
	float: right;
	margin: 20px 230px 0 0;
	width: 250px;
	background: #141618;
	border-radius: 8px;
	text-align: center;
	font-size: 28px;
	cursor: pointer;
}
.login-btn-bg:hover{
	border: 2px solid #AB0015;
}
.error-text, 
.logout-text{
	color: #45FFF0;
}
.form-input{
	width: 380px;
	height: 30px;
	font-size: 28px;
	padding: 18px 22px;
	border: none;
	outline: none;
	border-bottom: 2px solid rgb(112, 0, 14);
	border-radius: 4px;
	background-color: rgba(0,0,0,0.5);
	background-color: #576D73\9;
}
.form-input:focus{
	border-bottom: 2px solid #AB0015;
}
.p1{
	font-size: 16pt;
	width: 480px;
	margin: 10px 0 5px 240px;
}
.error-hint-bg{
	width: 537px;
	height: 70px;
	display:flex;
	align-items: center;
	margin: 40px auto;
	background: url('./images/New_ui/icon_Sec_hint.png') no-repeat;
}
.loading-icon{
	margin: 50px 290px 0px 0px;
	float: right;
}
.error-text{
	margin: 0 30px 0 112px;
	font-size: 20px;
}
/*for mobile device*/
@media screen and (max-width: 1000px){
.main-field-bg{
	width: 100%;
	margin: 0;
	background-size: 200%;
	background-position: 50% 20%;
}
.main-field-padding{
	width: 100%;
}
.logo-container{
	margin-left: 0;
}
.logo-rog{
	margin: 0 auto;
}
.login-title{
	display: none;
}
.login-btn-bg, .input-container{
	margin: 0 10px 20px 10px;
}
.login-btn-bg{
	width: 95%;
	background-size: 100%;
	line-height: 58px;
}
.form-input{	
	padding:10px 11px;
	width: 100%;
	font-size: 16px
}
.p1{
	font-size: 12pt;
	width:100%;
	margin: 0 0 5px 10px;
}
.error-hint-bg{
	width: 100%;
	height: 52px;
	background-size: 100%;
}
.loading-icon{
	margin: 0;
	float: none;
	text-align: center;
}
.error-text{
	line-height: 20px;
}
.error-text{
	margin: 0 10px 0 90px ;
}
.login-title-desc{
	font-size: 14pt;
	margin: auto 15px;;
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
	
var is_KR_sku = (function(){
	var ttc = '<% nvram_get("territory_code"); %>';
	return (ttc.search("KR") == -1) ? false : true;
})();
var isIE8 = navigator.userAgent.search("MSIE 8") > -1; 
var isIE9 = navigator.userAgent.search("MSIE 9") > -1; 

function initial(){
	if(is_KR_sku)
		document.getElementById("KRHint").style.display = "";

	if(isIE8 || isIE9){
		document.getElementById("router_name_tr").style.display = "";
		document.getElementById("router_password_tr").style.display = "";
		document.getElementById("router_password_confirm_tr").style.display = "";
	}

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

	if(is_KR_sku){		/* MODELDEP by Territory Code */
		if(!validator.chkLoginPw_KR(document.form.http_passwd_x)){
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

function submitForm(){
	if(validForm()){
		document.getElementById("error_status_field").style.display = "none";
		document.form.http_username.value = document.form.http_username_x.value;
		document.form.http_passwd.value = document.form.http_passwd_x.value;
		document.form.http_username_x.disabled = true;
		document.form.http_passwd_x.disabled = true;
		document.form.http_passwd_2_x.disabled = true;
		document.getElementById('btn_modify').style.display = "none";
		document.getElementById('loadingIcon').style.display = '';
		document.form.submit();

		var nextPage = decodeURIComponent('<% get_ascii_parameter("nextPage"); %>');
		setTimeout(function(){
			location.href = (nextPage != "") ? nextPage : "/";
		}, 3000);
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
			showError("<#JS_short_password#>");
			obj.value = "";
			obj.focus();
			obj.select();
			return false;
		}		

		if(obj.value.length > 16){
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
	
	chkLoginPw_KR: function(obj){		//Alphabets, numbers, specialcharacters mixed
		var string_length = obj.value.length;		
		
		if(!/[A-Za-z]/.test(obj.value) || !/[0-9]/.test(obj.value) || string_length < 8
				|| !/[\!\"\#\$\%\&\'\(\)\*\+\,\-\.\/\:\;\<\=\>\?\@\[\\\]\^\_\`\{\|\}\~]/.test(obj.value)){
				
				showError("<#JS_validPWD#>");
				obj.value = "";
				obj.focus();
				obj.select();
				return false;	
		}

		if(obj.value.length > 16){
            showError("<#JS_max_password#>");
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
	document.getElementById("error_status_field").style.display = "";
	document.getElementById("error_status_field").innerHTML = '<div class="error-text">'+ str + '</div>';
}
</script>
</head>
<body class="bg" onload="initial();">
<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="saveNvram">
<input type="hidden" name="action_wait" value="0">
<input type="hidden" name="current_page" value="Main_Login.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="flag" value="">
<input type="hidden" name="login_authorization" value="">
<input name="foilautofill" style="display: none;" type="password">
<input type="hidden" name="http_username" value="">
<input type="hidden" name="http_passwd" value="">
<div class="main-field-bg">
	<div class="main-field-padding">
		<div class="logo-container">
			<div class="logo-rog"></div>
			<div class="login-title"><#PASS_changepasswd#></div>
		</div>
		<div class="login-title-desc">
			<div class="desc"><#Web_Title2#> is currently not protected and uses an unsafe default username and password.</div>
			<div class="desc"><#QIS_pass_desc1#></div>
			<div id="KRHint" class="desc" style="display: none;"><#JS_validPWD#></div>
		</div>
		<div>
			<div id="router_name_tr" style="display:none" class="p1"><#Router_Login_Name#></div>
			<div class="input-container">
				<input type="text" id="http_username_x" name="http_username_x" tabindex="1" class="form-input" maxlength="20" autocapitalize="off" autocomplete="off" placeholder="<#Router_Login_Name#>">
			</div>
			<div id="router_password_tr" style="display:none" class="p1"><#PASS_new#></div>
			<div class="input-container">
				<input type="password" id="http_passwd_x" name="http_passwd_x" tabindex="2" class="form-input" maxlength="17" autocapitalize="off" autocomplete="off" placeholder="<#PASS_new#>">
			</div>
			<div id="router_password_confirm_tr" style="display:none" class="p1"><#Confirmpassword#></div>
			<div class="input-container">
				<input type="password" id="http_passwd_2_x" name="http_passwd_2_x" tabindex="3" class="form-input" maxlength="17" autocapitalize="off" autocomplete="off" placeholder="<#Confirmpassword#>">
			</div>
			<div id="error_status_field" class="error-hint-bg" style="display: none;" ></div>
			<div id="btn_modify" class="login-btn-bg" onclick="submitForm();"><#CTL_modify#></div>
			<div id="loadingIcon" class="loading-icon" style="display:none;">
				<img style="width:35px;height:35px;" src="/images/InternetScan.gif">
			</div>
		</div>
	</div>
</div>
</form>
</body>
</html>

