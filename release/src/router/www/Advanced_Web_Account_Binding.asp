<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>Account Binding</title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<script language="JavaScript" type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/httpApi.js"></script>
<style type="text/css">
.oauth_status{
	float: left;
	margin-top: 3px;
	margin-left: 5px;
	display: none;
}
.result__btn{
	background-color: rgba(0,198,184,1);
	border: 0;
	border-radius: 2px;
	-webkit-box-sizing: border-box;
	box-sizing: border-box;
	color: rgba(0,0,0,0.6);
	border-radius: 6px !important;
	display: inline-block;
	font-size: 1.6rem;
	font-weight: 600;
	font-family: inherit;
	height: 40px;
	line-height: 40px;
	overflow: hidden;
	outline-width: 2px;
	padding: 0 24px;
	position: relative;
	text-align: center;
	text-decoration: none;
	display: flex;
	width: 180px;
	height: 42px;
	-webkit-box-align: center;
	-ms-flex-align: center;
	align-items: center;
	-webkit-box-pack: center;
	-ms-flex-pack: center;
	justify-content: center;
	margin-top: 12px;
}
.alternate-signin__btn{
	background-color: rgba(255,255,255,1);
	border: 0;
	border-radius: 2px;
	-webkit-box-sizing: border-box;
	box-sizing: border-box;
	color: rgba(0,0,0,0.6);
	cursor: pointer;
	display: inline-block;
	font-size: 1.6rem;
	font-weight: 600;
	font-family: inherit;
	height: 40px;
	line-height: 40px;
	overflow: hidden;
	outline-width: 2px;
	padding: 0 24px;
	position: relative;
	text-align: center;
	text-decoration: none;
	-webkit-transition-duration: 167ms;
	transition-duration: 167ms;
	-webkit-transition-property: background-color,color,-webkit-box-shadow;
	transition-property: background-color,color,-webkit-box-shadow;
	transition-property: background-color,box-shadow,color;
	transition-property: background-color,box-shadow,color,-webkit-box-shadow;
	-webkit-transition-timing-function: cubic-bezier(0, 0, 0.2, 1);
	transition-timing-function: cubic-bezier(0, 0, 0.2, 1);
	vertical-align: middle;
	z-index: 0;
	-webkit-box-shadow: inset 0 0 0 1px rgb(0 0 0 / 60%), inset 0 0 0 2px rgb(0 0 0 / 0%), inset 0 0 0 1px rgb(0 0 0 / 0%);
	box-shadow: inset 0 0 0 1px rgb(0 0 0 / 60%), inset 0 0 0 2px rgb(0 0 0 / 0%), inset 0 0 0 1px rgb(0 0 0 / 0%);
	color: rgba(0,0,0,0.6) !important;
	border-radius: 28px !important;
	display: -webkit-box;
	display: -ms-flexbox;
	display: flex;
	width: 500px;
	height: 42px;
	-webkit-box-align: center;
	-ms-flex-align: center;
	align-items: center;
	-webkit-box-pack: center;
	-ms-flex-pack: center;
	justify-content: center;
	margin-top: 12px;
}
.btn-text{
	overflow: hidden;
	margin-left: 8px;
	white-space: nowrap;
	text-overflow: ellipsis;
}

.divTable{
	display: table;
	width: 100%;
}
.divTableRow {
	display: table-row;
}
.divTableHeading {
	background-color: #EEE;
	display: table-header-group;
}
.divTableCell, .divTableHead {
	border: 1px solid #999999;
	display: table-cell;
	padding: 3px 10px;
	text-align: left;
}
.divTableHeading {
	background-color: #EEE;
	display: table-header-group;
	font-weight: bold;
}
.divTableFoot {
	background-color: #EEE;
	display: table-footer-group;
	font-weight: bold;
}
.divTableBody {
	display: table-row-group;
}
</style>
<script>
function parseJwt (token) {
	var base64Url = token.split('.')[1];
	var base64 = base64Url.replace(/-/g, '+').replace(/_/g, '/');
	var jsonPayload = decodeURIComponent(atob(base64).split('').map(function(c) {
		return '%' + ('00' + c.charCodeAt(0).toString(16)).slice(-2);
	}).join(''));

	return JSON.parse(jsonPayload);
};


function b64DecodeUnicode(str) {
	return decodeURIComponent(
	atob(str).replace(/(.)/g, function (m, p) {
		var code = p.charCodeAt(0).toString(16).toUpperCase();
		if (code.length < 2) {
			code = "0" + code;
		}
			return "%" + code;
		})
	);
}

function base64_url_decode(str) {
	var output = str.replace(/-/g, "+").replace(/_/g, "/");
	switch (output.length % 4) {
		case 0:
			break;
		case 2:
			output += "==";
			break;
		case 3:
			output += "=";
			break;
		default:
			throw "Illegal base64url string!";
	}

	try {
		return b64DecodeUnicode(output);
	} catch (err) {
		return atob(output);
	}
}

function jwtDecode(token, options) {
	options = options || {};
	var pos = options.header === true ? 0 : 1;
	try {
		return JSON.parse(base64_url_decode(token.split(".")[pos]));
	} catch (e) {
		console.log(e.message);
	}
}

function show_auth_status(_status) {

	$("#oauth_hint").show();
	var auth_status_hint = "<#Authenticated_non#>";
	switch(_status) {
		case 0 :
			auth_status_hint = "<#Main_alert_processing#>";
			break;
		case 1 :
			auth_status_hint = "<#qis_fail_desc1#>";
			break;
		case 2 :
			auth_status_hint = "<#Authenticated#>";
			break;
	}
	$("#oauth_hint").html(auth_status_hint);
}

var base64Encode = function(input) {
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

var gen_callback_url = function(service_idx) {
	var oauth_salt = [<% gen_oauth_salt(); %>][0];
	var callback_url = oauthVariable.domainName + "/" + oauthCallBack + "," +  "onGoogleLogin" + "," + service_idx+ "," + oauth_salt;
	//workaround for encode issue, if the original string is not a multiple of 6, the base64 encode result will display = at the end
	//Then Dropbox will encode the url twice, the char = will become %3D, and callback oauth.asus.com will cause url not correct.
	//So need add not use char at callback_url for a multiple of 6
	var remainder = callback_url.length % 6;
	if(remainder != 0) {
		var not_use = "";
		for(var i = remainder; i < 6; i += 1) {
			not_use += ",";
		}
		callback_url += not_use;
	}
	return callback_url
};

var oauthCallBack = "oauth_callback.cgi";
var oauthVariable = {
	"domainName" : (function() {
	var header_info = [<% get_header_info(); %>][0];
	return header_info.protocol + "://" + header_info.host + ":" + header_info.port;
	})(),
	"server" : "https://webauthorizationcode.herokuapp.com?",
};

var oauth = {};
oauth.signin = function(item, service_idx) {
	if(item == "asuscn"){
		httpApi.nvramSet({"action_mode": "apply", "aae_portal" : "aae-spweb.asuscomm.cn", "aae_area" : "aae-sgweb086-1.asuscomm.cn"});
	}else
		httpApi.nvramSet({"action_mode": "apply", "aae_portal" : "aae-spweb.asuscomm.com", "aae_area" : "aae-sgweb001-1.asuscomm.com"});
	var url = oauthVariable.server;
	url += "state=base64_" + base64Encode(gen_callback_url(service_idx));
	window.open(url,"mywindow","menubar=1,resizable=1,width=600,height=470,top=100,left=300");
	setTimeout("check_signin_status();", 5000);
};

function GenServiceBtn(item, index) {

	var ServiceName = "#sign-in-with-"+item+"-button";
	$(ServiceName).click(
	function() {
		oauth.signin(item, index);
	})
}

oauth.unset = function() {
	httpApi.nvramSet({"action_mode": "apply", "oauth_user_id" : "", "oauth_dm_cusid" : "", "oauth_dm_user_ticket" : "", "oauth_dm_refresh_ticket" : ""});
	setTimeout("show_oauth_info();", 5000);
};

function show_oauth_info(){
	var oauth_user_id = httpApi.nvramGet(["oauth_user_id"], true).oauth_user_id;
	var oauth_dm_cusid = httpApi.nvramGet(["oauth_dm_cusid"], true).oauth_dm_cusid;
	var oauth_dm_user_ticket = httpApi.nvramGet(["oauth_dm_user_ticket"], true).oauth_dm_user_ticket;
	$("#oauth_user_id").html(oauth_user_id);
	$("#oauth_dm_cusid").html(oauth_dm_cusid);
	$("#oauth_dm_user_ticket").html(oauth_dm_user_ticket);
	if(oauth_user_id != "" || oauth_dm_cusid != "" || oauth_dm_user_ticket != "")
		$("#oauth_unset_btn").show();
	else
		$("#oauth_unset_btn").hide();
}

var org_dm_refresh_ticket = httpApi.nvramGet(["oauth_dm_refresh_ticket"], true).oauth_dm_refresh_ticket;

var retry = 30;
function check_signin_status(FistCheck){
	var oauth_dm_cusid = httpApi.nvramGet(["oauth_dm_cusid"], true).oauth_dm_cusid;
	var dm_refresh_ticket = httpApi.nvramGet(["oauth_dm_refresh_ticket"], true).oauth_dm_refresh_ticket;
	var oauth_dm_user_ticket = httpApi.nvramGet(["oauth_dm_user_ticket"], true).oauth_dm_user_ticket;
	var oauth_status_code = httpApi.nvramGet(["oauth_status"], true).oauth_status;
	if(FistCheck == 1){
		if(dm_refresh_ticket != "" && oauth_dm_cusid != "" && oauth_dm_user_ticket != "")
			show_auth_status(2);
		else
			show_auth_status();
	}else{
		if(org_dm_refresh_ticket == dm_refresh_ticket || oauth_status_code == 0){
			if(retry > 0){
				$("#oauth_loading").show();
				show_auth_status(0);
				retry --;
				setTimeout("check_signin_status();", 3000);
				return;
			}else{
				setTimeout("check_signin_status(1);", 3000);
			}
		}else if(org_dm_refresh_ticket != dm_refresh_ticket && oauth_status_code == 200)
			show_auth_status(2);
		else
			show_auth_status(1);
	}
	$("#oauth_loading").hide();
	show_oauth_info();
}

function initial(){

	var service_array=["google", "facebook", "apple", "wechat", "qq", "weibo", "asus", "asuscn"];

	service_array.forEach(GenServiceBtn);

	//init check token_status
	check_signin_status(1);

	show_oauth_info();

	$(oauth_unset_btn).click(
		function() {
			oauth.unset();
	})
}
</script>
</head>

<body onload="initial();">

<div style="margin: 100px 0px 0px 0px; text-align: center;">
	<button id="oauth_unset_btn" class="result__btn mt-12" style="margin:30px auto;background-color: rgba(198,152,0,1);cursor:pointer;" type="button">
	<span class="oauth_status" ></span>unset</button>
</div>

<div id="oauth_info" style="margin: 0px 0px 0px 450px; text-align: center;">
	<div class="divTable" style="width: 80%; border: 2px solid #000;">
		<div class="divTableBody">
			<div class="divTableRow">
				<div class="divTableCell" style="width: 30%;">&nbsp;oauth_user_id</div>
				<div id="oauth_user_id" class="divTableCell" style="width: 70%;">&nbsp;</div>
			</div>
			<div class="divTableRow">
				<div class="divTableCell" style="width: 30%;">&nbsp;oauth_dm_cusid</div>
				<div id="oauth_dm_cusid" class="divTableCell">&nbsp;</div>
			</div>
			<div class="divTableRow">
				<div class="divTableCell" style="width: 30%;">&nbsp;oauth_dm_user_ticket</div>
				<div id="oauth_dm_user_ticket" class="divTableCell">&nbsp;</div>
			</div>
		</div>
	</div>
</div>
<div style="margin:30px auto;">
	<button class="result__btn mt-12" style="margin:30px auto;" type="button"><img id="oauth_loading" src="/images/InternetScan.gif" class="oauth_status">
	<span id="oauth_hint" class="oauth_status"></span></button>
</div>

<div style="width: 500px;margin:30px auto;">
<button id="sign-in-with-google-button" class="alternate-signin__btn mt-12" type="button" aria-label="使用 Google 登入"><svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 22 24" fill="none"><path fill-rule="evenodd" clip-rule="evenodd" d="M12.1354 5.75C14.0004 5.75 15.4794 6.396 16.4204 7.33L19.0744 4.676C17.3544 3 14.9584 2 12.1354 2C8.1984 2 4.8554 4.148 3.1704 7.302L6.2004 9.7C7.0974 7.39 9.3304 5.75 12.1354 5.75Z" fill="#E94435"></path><path fill-rule="evenodd" clip-rule="evenodd" d="M5.7708 11.9896C5.7708 11.1806 5.9248 10.4106 6.2008 9.7006L3.1708 7.3016C2.4238 8.7006 1.9998 10.2946 1.9998 11.9896C1.9998 13.7206 2.4098 15.3266 3.1358 16.7256L6.1958 14.3026C5.9248 13.5956 5.7708 12.8206 5.7708 11.9896Z" fill="#F8BB15"></path><path fill-rule="evenodd" clip-rule="evenodd" d="M15.8107 17.3084C14.8667 17.8694 13.6267 18.2294 12.0107 18.2294C9.3627 18.2294 7.1007 16.6654 6.1957 14.3034L3.1357 16.7254C4.7837 19.9024 8.0767 22.0004 12.0107 22.0004C14.7537 22.0004 17.0727 21.1524 18.7877 19.6654L15.8107 17.3084Z" fill="#34A751"></path><path fill-rule="evenodd" clip-rule="evenodd" d="M22 11.9896C22 11.3086 21.931 10.6436 21.801 9.9996H12V13.9996H18.062L18.018 14.2496C17.784 15.4466 17.068 16.5606 15.811 17.3086L18.788 19.6656C20.818 17.9056 22 15.2466 22 11.9896Z" fill="#547DBE"></path></svg><span class="btn-text">使用 Google 登入</span></button>
</div>

<div style="width: 500px;margin:30px auto;">
<button  id="sign-in-with-apple-button" class="alternate-signin__btn" type="button" aria-label="使用 Apple 登入"><svg width="24" height="24" viewBox="0 2 24 24" fill="none" xmlns="http://www.w3.org/2000/svg"><rect width="24" height="24" fill="transparent"></rect><path d="M17.569 12.6254C17.597 15.652 20.2179 16.6592 20.247 16.672C20.2248 16.743 19.8282 18.1073 18.8662 19.5166C18.0345 20.735 17.1714 21.9488 15.8117 21.974C14.4756 21.9986 14.046 21.1799 12.5185 21.1799C10.9915 21.1799 10.5142 21.9489 9.2495 21.9987C7.93704 22.0485 6.93758 20.6812 6.09906 19.4673C4.38557 16.9842 3.0761 12.4508 4.83438 9.39061C5.70786 7.87092 7.26882 6.90859 8.96311 6.88391C10.2519 6.85927 11.4683 7.753 12.2562 7.753C13.0436 7.753 14.5219 6.67821 16.0759 6.83605C16.7265 6.8632 18.5527 7.09947 19.7253 8.81993C19.6309 8.87864 17.5463 10.095 17.569 12.6254ZM15.058 5.1933C15.7548 4.34789 16.2238 3.171 16.0959 2C15.0915 2.04046 13.877 2.67085 13.1566 3.5158C12.5109 4.26404 11.9455 5.46164 12.0981 6.60946C13.2176 6.69628 14.3612 6.03925 15.058 5.1933Z" fill="black"></path></svg><span class="btn-text">使用 Apple 登入</span></button>
</div>

<div style="width: 500px;margin:30px auto;">
<button  id="sign-in-with-facebook-button" class="alternate-signin__btn" type="button" aria-label="使用 Facebook 登入"><svg style="color: blue" xmlns="http://www.w3.org/2000/svg" width="24" height="24" fill="currentColor" class="bi bi-facebook" viewBox="0 0 16 16"> <path d="M16 8.049c0-4.446-3.582-8.05-8-8.05C3.58 0-.002 3.603-.002 8.05c0 4.017 2.926 7.347 6.75 7.951v-5.625h-2.03V8.05H6.75V6.275c0-2.017 1.195-3.131 3.022-3.131.876 0 1.791.157 1.791.157v1.98h-1.009c-.993 0-1.303.621-1.303 1.258v1.51h2.218l-.354 2.326H9.25V16c3.824-.604 6.75-3.934 6.75-7.951z" fill="blue"></path> </svg><span class="btn-text">使用 Facebook 登入</span></button>
</div>

<div style="width: 500px;margin:30px auto;">
<button  id="sign-in-with-wechat-button" class="alternate-signin__btn" type="button" aria-label="使用 Wechat 登入"><svg id="weChat" xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns="http://www.w3.org/2000/svg" height="24" width="24" version="1.1" xmlns:cc="http://creativecommons.org/ns#" xmlns:xlink="http://www.w3.org/1999/xlink" viewBox="0 0 368.10109 351.00342" xmlns:dc="http://purl.org/dc/elements/1.1/">
 <defs>
  <filter id="d" style="color-interpolation-filters:sRGB" height="1.073" width="1.071" y="-.03639" x="-.03562">
   <feGaussianBlur stdDeviation="4.545524"/>
  </filter>
  <radialGradient id="a" gradientUnits="userSpaceOnUse" cy="-44.21" cx="3538" gradientTransform="matrix(1.014 -.2270 .2141 .9563 -36.91 766.7)" r="153.2">
   <stop stop-color="#a3dc27" offset="0"/>
   <stop stop-color="#62c627" offset="1"/>
  </radialGradient>
  <radialGradient id="b" gradientUnits="userSpaceOnUse" cy="-60.47" cx="4113" gradientTransform="matrix(1 0 0 .8866 5.176 1.581)" r="117.6">
   <stop stop-color="#a6a5a4" offset="0"/>
   <stop stop-color="#b3b3b3" offset="1"/>
  </radialGradient>
  <filter id="e" style="color-interpolation-filters:sRGB" height="1.1" width="1.088" y="-.04979" x="-.04415">
   <feGaussianBlur stdDeviation="4.3256066"/>
  </filter>
  <linearGradient id="c" y2="403.7" gradientUnits="userSpaceOnUse" x2="3642" y1="-48.37" x1="3635">
   <stop stop-color="#fff" offset="0"/>
   <stop stop-color="#fff" stop-opacity="0" offset="1"/>
  </linearGradient>
 </defs>
 <g transform="translate(649.8 -39.72)">
  <g transform="translate(-4038 242.7)">
   <path d="m3704 107.7c-0.1926-0.3117 0.058-0.8653 0.5571-1.23 2.74-0.9721 0.2238 1.352-0.5571 1.23zm-259-42.95c-1.811-2.765 0.3306-10.92 9.262-35.27 0.9528-2.598 1.588-4.946 1.411-5.217-0.1765-0.2714-2.29-1.888-4.697-3.593-28.47-20.16-46.57-47.72-51.25-78.01-1.147-7.429-1.19-25.57-0.076-32.54 2.93-18.34 10.25-34.91 21.93-49.62 16.77-21.12 44.56-38.61 74.36-46.81 15.93-4.382 27.27-5.832 45.27-5.786 50.78 0.1286 92.84 14.72 119.6 41.49 8.652 8.654 14.75 17.22 20.17 28.33 5.444 11.16 8.925 23.27 10.28 35.78 0.9284 8.557 0.4514 29.65-0.6965 30.8-0.1309 0.131-3.066-0.8776-6.524-2.241-17.63-6.952-31.91-9.649-50.82-9.589-48.09 0.1506-86.67 16.87-106.4 46.11-10.85 16.11-15.94 36.4-14.28 56.99 0.3316 4.123 0.7339 8.106 0.894 8.85 0.2355 1.095 0.1157 1.355-0.6273 1.365-0.5052 0.007-6.056-1.259-12.34-2.813l-11.42-2.825-5.313 3.618c-26.12 17.79-31.92 21.33-35.93 21.97-1.798 0.2876-2.094 0.1861-2.858-0.9791z" filter="url(#d)" fill="#808080"/>
   <path d="m3704 100.9c-0.1926-0.3117 0.058-0.8653 0.5571-1.23 1.409-1.03 1.806-0.7856 0.92 0.5667-0.9196 1.404-0.9979 1.439-1.477 0.6635zm-258-42.93c-1.811-2.765 0.3307-10.92 9.262-35.27 0.9528-2.598 1.588-4.946 1.411-5.217-0.1765-0.2714-2.29-1.888-4.697-3.593-28.47-20.16-46.57-47.72-51.25-78.01-1.147-7.429-1.19-25.57-0.076-32.54 2.93-18.34 10.25-34.91 21.93-49.62 16.77-21.12 44.56-38.61 74.36-46.81 15.93-4.382 27.27-5.832 45.27-5.786 50.78 0.1286 92.84 14.72 119.6 41.49 8.652 8.654 14.75 17.22 20.17 28.33 5.444 11.16 8.925 23.27 10.28 35.78 0.9283 8.557 0.4514 29.65-0.6965 30.8-0.131 0.131-3.066-0.8776-6.524-2.241-17.63-6.952-31.91-9.649-50.82-9.589-48.09 0.1506-86.67 16.87-106.4 46.11-10.85 16.11-15.94 36.4-14.28 56.99 0.3316 4.123 0.7339 8.106 0.894 8.85 0.2355 1.095 0.1157 1.355-0.6273 1.365-0.5052 0.007-6.056-1.259-12.34-2.813l-11.42-2.825-5.313 3.618c-26.12 17.79-31.92 21.33-35.94 21.97-1.798 0.2876-2.094 0.1861-2.858-0.9791z" fill="url(#a)"/>
   <circle stroke-opacity="0" cy="-113.6" cx="3498" r="20.44" fill="#363b2a"/>
   <circle stroke-opacity="0" cy="-109.3" cx="3600" r="20.44" fill="#363b2a"/>
   <path d="m4117-156.3c72.84 1.583 119.5 45.13 118.8 98.17-0.7917 53.04-45.17 75.44-45.17 75.44s7.338 19.53 8.854 30.08c0.1492 1.038 1.001 3.837 0.227 4.544-0.6626 0.6051-1.826-0.0229-2.673-0.3177-12.59-4.378-33.23-21.22-33.23-21.22-68.17 26.94-164.7-17.88-163.1-86.55 2.068-63.4 42.55-95.08 116.4-100.1z" fill-rule="evenodd" transform="matrix(.9821 0 0 .9821 -413.5 86.52)" filter="url(#e)" stroke="url(#b)" stroke-miterlimit="25.2" fill="#808080"/>
   <path d="m3627-77.22c72.84 1.583 119.5 45.13 118.8 98.17-0.7917 53.04-45.17 75.44-45.17 75.44s7.338 19.53 8.854 30.08c0.1492 1.038 1.001 3.837 0.227 4.544-0.6626 0.6051-1.826-0.0229-2.673-0.3177-12.59-4.378-33.23-21.22-33.23-21.22-68.17 26.94-164.7-17.88-163.1-86.55 2.068-63.4 42.55-95.08 116.4-100.1z" fill-rule="evenodd" fill="url(#c)"/>
   <circle cy="-4.632" cx="3593" r="15.95" fill="#363b2a"/>
   <circle cy="-6.032" cx="3675" r="15.95" fill="#363b2a"/>
  </g>
 </g>
</svg><span class="btn-text">使用 Wechat 登入</span></button>
</div>

<div style="width: 500px;margin:30px auto;">
<button  id="sign-in-with-qq-button" class="alternate-signin__btn" type="button" aria-label="使用 QQ 登入"><svg version="1.1" id="Capa_1" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" x="0px" y="0px"
   width="24px" height="24px" viewBox="0 0 98.802 98.803" style="enable-background:new 0 0 98.802 98.803;"
   xml:space="preserve">
<g>
  <g>
    <path d="M95.568,57.454c-1.74-4.367-3.976-8.49-6.733-12.316c-0.295-0.408-0.602-0.729-0.329-1.404
      c1.326-3.281,0.896-6.463-0.798-9.515c-0.763-1.376-1.668-2.684-1.755-4.342c-0.127-2.393-0.734-4.692-1.356-6.994
      c-2.17-8.031-6.494-14.449-13.937-18.479c-4.224-2.287-8.764-3.589-13.545-4.115C52.19-0.253,47.321-0.04,42.472,0.987
      c-8.02,1.701-13.92,6.429-18.489,12.984c-3.001,4.308-5.137,8.993-5.776,14.3c-0.123,1.021,0.25,2.146-0.41,3.085
      c-0.573,0.812-0.9,1.724-1.063,2.675c-0.245,1.425-0.573,2.778-1.304,4.073c-0.888,1.57-1.127,3.374-0.764,5.138
      c0.157,0.758-0.005,1.153-0.531,1.548c-3.109,2.327-5.68,5.131-7.84,8.373c-3.077,4.616-4.894,9.619-5.189,15.16
      c-0.119,2.225,0.15,4.398,0.933,6.505c0.379,1.02,0.88,1.498,2.084,1.148c1.013-0.293,1.878-0.748,2.645-1.423
      c1.6-1.404,2.905-3.04,3.769-5.004c0.1-0.228,0.074-0.579,0.439-0.561c0.332,0.016,0.363,0.306,0.42,0.573
      c0.518,2.398,1.633,4.556,2.829,6.659c1.276,2.247,3.105,4.056,5.017,5.75c0.667,0.592,1.614,0.868,1.987,1.871
      c-1.38-0.002-2.656,0.194-3.863,0.609c-2.062,0.711-3.895,1.764-4.372,4.145c-0.456,2.275-0.613,4.522,1.467,6.206
      c0.823,0.666,1.734,1.195,2.716,1.614c3.463,1.477,7.142,1.956,10.837,2.194c4.568,0.294,9.156,0.404,13.635-0.838
      c2.596-0.722,4.999-1.891,7.251-3.366c0.213-0.14,0.354-0.46,0.658-0.372c1.79,0.518,3.677-0.02,5.49,0.687
      c2.91,1.136,5.917,2.001,9.02,2.501c4.605,0.744,9.227,1.093,13.874,0.502c3.149-0.401,6.235-1.094,8.993-2.768
      c3.546-2.153,3.654-5.891,0.326-8.31c-1.64-1.192-3.38-2.186-5.205-3.05c-0.472-0.223-0.991-0.376-1.364-0.893
      c3.672-3.374,5.523-7.843,7.374-12.409c1.054,1.952,2.08,3.805,3.441,5.433c1.449,1.731,2.711,1.69,4.132-0.04
      c0.566-0.69,0.981-1.451,1.239-2.315C98.51,67.896,97.619,62.604,95.568,57.454z M55.018,22.695
      c-0.062-2.094,0.374-4.126,1.512-5.984c2.2-3.594,5.927-3.671,8.122-0.082c1.899,3.109,1.954,7.003,0.982,10.438
      c-0.47,1.66-1.153,3.151-2.801,3.994c-2.239,1.145-4.307,0.692-5.812-1.331C55.482,27.662,54.927,25.299,55.018,22.695z
       M40.416,15.943c2.095-2.708,5.158-2.722,7.237-0.017c1.574,2.05,2.052,4.435,2.091,7.159c-0.076,2.407-0.588,4.892-2.398,6.899
      c-2.086,2.315-4.877,2.194-6.817-0.231C37.729,26.254,37.674,19.486,40.416,15.943z M31.089,39.146
      c3.005-2.065,6.387-3.264,9.902-4.027c7.729-1.682,15.478-1.892,23.2,0.086c3.134,0.803,6.169,1.89,8.897,3.668
      c1.692,1.104,1.673,1.513-0.021,2.552c-1.81,1.109-3.694,2.027-6.063,2.02c0.854-0.947,1.935-1.479,2.597-2.923
      c-11.517,7.921-22.792,8.559-34.122,0.353c0.501,0.808,1.002,1.614,1.618,2.606c-2.195-0.55-4.16-1.071-5.952-2.04
      C29.729,40.672,29.748,40.068,31.089,39.146z M45.498,94.378c-1.388,1.356-3.231,1.805-4.997,2.193
      c-6.68,1.475-13.408,1.794-20.09,0.042c-2.074-0.543-4.159-1.262-5.741-2.864c-1.172-1.185-1.151-2.205,0.02-3.421
      c0.726-0.755,1.572-1.359,2.358-2.14c-0.603,0.107-1.211,0.196-1.808,0.337c-0.297,0.069-0.646,0.303-0.824-0.039
      c-0.122-0.235-0.103-0.648,0.025-0.892c0.29-0.544,0.689-1.041,1.236-1.357c0.763-0.443,1.53-0.892,2.332-1.255
      c1.908-0.865,3.584-0.936,5.472,0.514c3.637,2.791,7.861,4.532,12.245,5.885c3.109,0.96,6.28,1.586,9.487,2.072
      c0.244,0.038,0.583-0.093,0.711,0.2C46.091,94.035,45.705,94.175,45.498,94.378z M81.455,84.153
      c1.248,0.611,2.564,1.141,4.022,2.31c-1.181,0.092-2.198,0.127-3.067,0.681c-0.171,0.106-0.416,0.311-0.405,0.454
      c0.028,0.373,0.373,0.263,0.621,0.262c1.151-0.001,2.304-0.059,3.452,0.001c2.125,0.109,3.197,1.731,2.403,3.692
      c-1.039,2.568-3.396,3.5-5.763,4.248c-7.481,2.366-14.902,1.625-22.27-0.625c-0.812-0.249-1.776-0.215-2.169-1.324
      c7.716-1.221,14.533-4.239,20.361-9.354C79.717,83.552,80.247,83.559,81.455,84.153z M84.223,68.128
      c-0.26,4.43-1.97,8.329-4.652,11.788c-5.173,6.673-11.993,10.796-20.188,12.656c-3.104,0.706-6.256,0.349-9.376,0.045
      c-4.791-0.465-9.515-1.327-13.972-3.219c-2.77-1.177-5.435-2.546-7.813-4.473c-4.629-3.753-8.246-8.165-9.446-14.146
      c-1.086-5.412-0.645-10.715,1.674-15.791c0.164-0.358,0.373-0.696,0.543-1.052c0.414-0.856,0.823-1.223,1.793-0.484
      c1.042,0.791,2.265,1.348,3.431,1.966c0.447,0.237,0.563,0.432,0.49,1.003c-0.504,4.039-0.938,8.08-0.483,12.171
      c0.272,2.438,1.731,3.976,3.747,4.851c2.783,1.207,5.785,1.057,8.735,0.577c1.204-0.195,2.569-1.76,2.516-3.548l-0.192-8.102
      l-0.069-1.684c3.209,0.899,6.507,1.185,9.782,1.263c7.792,0.186,15.094-1.702,22.083-5.021c2.072-0.983,4.073-2.088,5.977-3.359
      c0.473-0.315,0.655-0.347,1.007,0.171C82.755,58.09,84.538,62.793,84.223,68.128z M36.888,64.798l-0.091-3.047
      c0.059-0.565-0.266-1.596,0.643-1.748c1.124-0.188,2.169,0.613,2.277,1.747c0.269,2.827,0.451,5.684,0.349,8.552
      c-0.049,1.381-0.726,2.211-2.281,2.291c-2.221,0.117-4.431,0.192-6.611-0.293c-3.059-0.683-4.14-2.181-4.231-5.647
      c-0.087-3.265,0.691-6.405,1.279-9.576c0.094-0.508,0.288-0.49,0.706-0.312c1.94,0.832,3.841,1.771,5.895,2.308
      c0.619,0.161,0.524,0.587,0.541,1.025c0.076,2.042,0.341,4.055,1.032,5.99c0.113,0.316,0.279,0.617,0.525,1.172L36.888,64.798z
       M87.863,41.959c-0.713,3.928-2.98,6.794-6.25,8.828c-6.996,4.354-14.417,7.735-22.591,9.235
      c-4.74,0.869-9.478,0.834-14.262,0.222c-5.7-0.728-11.113-2.364-16.314-4.708c-4.34-1.956-8.464-4.3-11.461-8.165
      c-2.191-2.824-2.488-5.776-0.475-8.403c0.613,3.759,2.714,6.468,5.648,8.647c-1.113-1.906-2.246-3.8-3.333-5.72
      c-1.16-2.046-1.057-4.28-0.949-6.513c0.127-0.013,0.255-0.054,0.276-0.023c3.985,5.908,9.673,9.502,16.248,11.818
      c8.313,2.933,16.929,3.846,25.633,2.862c8.854-1,16.799-4.403,23.481-10.46c0.426-0.385,0.882-0.734,1.218-1.014
      c-1.527,6.333-6.051,10.371-11.515,13.634c7.514-2.716,11.403-8.663,14.022-15.749C88.027,37.638,88.234,39.91,87.863,41.959z"/>
    <path d="M57.824,24.385c0.522,0.103,0.59-0.406,0.691-0.783c0.194-0.719,0.302-1.658,1.196-1.672
      c0.82-0.011,0.854,0.921,0.957,1.529c0.082,0.484,0.37,0.993,0.901,0.919c0.674-0.094,0.597-3.508-1.134-4.097
      c-1.595-0.601-3.162,0.939-3.122,3.106C57.321,23.776,57.325,24.288,57.824,24.385z"/>
    <path d="M46.776,26.242c0.833,0.062,1.306-0.495,1.617-1.142c0.776-1.614,0.754-3.243-0.183-4.788
      c-0.681-1.121-1.811-1.173-2.591-0.158c-0.619,0.805-0.779,1.753-0.757,2.742c0.015,0.705,0.073,1.401,0.379,2.056
      C45.552,25.621,45.975,26.179,46.776,26.242z"/>
  </g>
</g>
</svg><span class="btn-text">使用 QQ 登入</span></button>
</div>

<div style="width: 500px;margin:30px auto;">
<button  id="sign-in-with-weibo-button" class="alternate-signin__btn" type="button" aria-label="使用 Weibo 登入"><svg xmlns="http://www.w3.org/2000/svg" width="24px" height="24px" viewBox="0 0 64 64" aria-labelledby="title"
aria-describedby="desc" role="img" xmlns:xlink="http://www.w3.org/1999/xlink">
  <title>Weibo</title>
  <desc>A solid styled icon from Orion Icon Library.</desc>
  <path data-name="layer3"
  d="M48.068 31.241c-.9-.3-1.5-.5-1-1.606 1-2.609 1.1-4.818 0-6.423-2.1-3.011-7.811-2.81-14.32-.1 0 0-2 .9-1.5-.7 1-3.212.9-5.922-.7-7.527-3.5-3.513-12.918.1-21.03 8.23C3.5 29.133 0 35.556 0 41.177c0 10.639 13.619 17.062 26.938 17.062 17.425 0 29.041-10.137 29.041-18.166-.201-4.918-4.306-7.628-7.911-8.832zm-21.23 23.184c-10.615 1-19.728-3.714-20.429-10.739s7.31-13.449 17.925-14.553c10.615-1 19.728 3.714 20.429 10.739s-7.31 13.549-17.925 14.553z"
  fill="#202020"></path>
  <path data-name="layer2" d="M59.584 11.87a16.616 16.616 0 0 0-16.123-5.219 2.46 2.46 0 0 0 1 4.818 11.809 11.809 0 0 1 11.516 3.714 12.022 12.022 0 0 1 2.5 11.843 2.471 2.471 0 1 0 4.707 1.505 17.057 17.057 0 0 0-3.6-16.661z"
  fill="#202020"></path>
  <path data-name="layer2" d="M45.264 15.182a2.106 2.106 0 0 0 .9 4.115 3.91 3.91 0 0 1 3.805 1.2 3.8 3.8 0 0 1 .8 3.914 2.154 2.154 0 1 0 4.106 1.3 8.409 8.409 0 0 0-1.7-8.13 8.422 8.422 0 0 0-7.911-2.399z"
  fill="#202020"></path>
  <path data-name="layer1" d="M27.839 34.553a11.586 11.586 0 0 0-12.918 5.62 7.893 7.893 0 0 0 5.007 11.241c5.307 1.706 11.516-.9 13.719-5.821 2.103-4.717-.5-9.736-5.808-11.04zM24.034 46.2a4.014 4.014 0 0 1-4.907 1.6 2.816 2.816 0 0 1-1.1-4.215 4 4 0 0 1 4.807-1.606c1.701.603 2.201 2.51 1.2 4.221zm3.4-4.416a1.672 1.672 0 0 1-1.8.7 1.171 1.171 0 0 1-.5-1.606 1.672 1.672 0 0 1 1.8-.7 1.171 1.171 0 0 1 .505 1.601z"
  fill="#202020"></path>
</svg><span class="btn-text">使用 Weibo 登入</span></button>
</div>

<div style="width: 500px;margin:30px auto;">
<button  id="sign-in-with-asus-button" class="alternate-signin__btn" type="button" aria-label="使用 ASUS 登入"><svg version="1.1"
   id="svg3875" inkscape:version="0.48.5 r10040" sodipodi:docname="A.svg" xmlns:cc="http://creativecommons.org/ns#" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape" xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd" xmlns:svg="http://www.w3.org/2000/svg"
   xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" x="0px" y="0px" width="24px" height="24px" viewBox="0 0 520 106.6"
   style="enable-background:new 0 0 537.1 106.6;" xml:space="preserve">
<style type="text/css">
  .st0{fill:#00539B;}
</style>
<path class="st0" d="M514.3,13.5c0-6.1,4.9-11.1,10.9-11.1c2.9,0,5.6,1.2,7.7,3.2c2.1,2.1,3.2,4.9,3.2,7.8s-1.1,5.7-3.2,7.8
  c-2.1,2.1-4.8,3.3-7.7,3.3C519.2,24.5,514.3,19.6,514.3,13.5 M513.4,13.5c0,6.6,5.3,12,11.9,12c3.2,0,6.1-1.2,8.3-3.5
  s3.5-5.3,3.5-8.5s-1.2-6.2-3.5-8.5s-5.2-3.5-8.3-3.5C518.7,1.5,513.4,6.9,513.4,13.5 M520.8,20.6h1.7V15h3.8c0.6,0,1.1,0,1.5,0.2
  c0.5,0.3,0.8,0.9,0.9,1.7l0.2,2.2c0,0.5,0,0.8,0,1c0.1,0.2,0.1,0.3,0.2,0.4h2.1v-0.3c-0.3-0.1-0.4-0.4-0.5-0.8
  c-0.1-0.2-0.1-0.6-0.1-1.1l-0.1-1.7c0-0.8-0.2-1.3-0.4-1.6c-0.3-0.3-0.7-0.6-1.2-0.8c0.6-0.3,1-0.7,1.3-1.2s0.4-1.1,0.4-1.9
  c0-1.5-0.6-2.5-1.7-3.1c-0.6-0.3-1.4-0.4-2.3-0.4h-5.7L520.8,20.6L520.8,20.6z M522.4,13.5V9h4c0.6,0,1.1,0.1,1.4,0.3
  c0.6,0.3,0.9,1,0.9,1.9s-0.2,1.5-0.7,1.8c-0.4,0.3-1,0.5-1.8,0.5H522.4z M48.6,30L0,106.4h33.2l46.8-73L48.6,30z M248.7,28.3V0
  h-94.5c-12.9,0.8-18.5,7-22.7,11.4c-4.5,4.7-7,14.6-7,14.6V0.2H74.2c-3.8,0-7.4,1.6-10.1,5.4c-2.8,3.7-14.3,22.7-14.3,22.7
  L248.7,28.3z M505.7,28.3V0H404.6c-12.8,0.8-18.5,7-22.7,11.4c-4.5,4.7-7,14.6-7,14.6v2.3H505.7z M281.9,0.1h-28.3v28.2h28.3V0.1z
   M374.6,0.1h-28.2v28.2h28.2V0.1z M374.6,38.5l-27.8-1.7v32.7c0,0-0.1,9.4-9.2,9.4H290c0,0-8.1-0.7-8.1-9.3V32.4l-28-2v50.1
  c4.5,24,26.2,25.9,26.2,25.9s2.2,0.1,2.6,0.2h64.5c0,0,27.6-2.2,27.6-28.8L374.6,38.5L374.6,38.5z M95.7,106.4h132.2
  c23-4.6,25.4-25.6,25.4-25.6c1.1-6.2,0.5-11.3,0.5-11.3c-0.7-4.2-8.7-22.8-25.9-24.4c-10.3-1-102-8.3-102-8.3
  c1.8,9.2,5.9,13.9,8.7,16.5c6.4,6.1,16.5,7.9,16.5,7.9c2.4,0.2,70.7,5.9,70.7,5.9c2.2,0.1,6.3,0.7,6.2,6.1c0,0.7-0.6,5.4-5.8,5.4
  h-98.9v-42l-27.5-2C95.7,34.6,95.7,106.4,95.7,106.4z M374.8,106.6H476c25.4-1.5,29-25.6,29-25.6c0.9-6.3,0.2-11.5,0.2-11.5
  c-0.7-5.7-8.8-22.9-26.1-24.6c-10.2-1-102.5-6.4-102.5-6.4c2.7,9.5,6.4,12.4,9.2,15c6.4,6.2,15.9,7.6,15.9,7.6
  c2.4,0.2,71.4,6,71.4,6c2.2,0.1,6.7,0.3,6.7,5.7c0,1.6-0.8,3.3-1.8,4.2c0,0-2.4,1.3-4.7,1.3h-98.2L374.8,106.6z"/>
</svg><span class="btn-text">使用 ASUS 登入</span></button>
</div>

<div style="width: 500px;margin:30px auto;">
<button id="sign-in-with-asuscn-button" class="alternate-signin__btn" type="button" aria-label="使用 ASUS 登入(CN)"><svg version="1.1"
id="svg3875" inkscape:version="0.48.5 r10040" sodipodi:docname="A.svg" xmlns:cc="http://creativecommons.org/ns#" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape" xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd" xmlns:svg="http://www.w3.org/2000/svg"
xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" x="0px" y="0px" width="24px" height="24px" viewBox="0 0 520 106.6"
style="enable-background:new 0 0 537.1 106.6;" xml:space="preserve">
<style type="text/css">
.st0{fill:#00539B;}
</style>
<path class="st0" d="M514.3,13.5c0-6.1,4.9-11.1,10.9-11.1c2.9,0,5.6,1.2,7.7,3.2c2.1,2.1,3.2,4.9,3.2,7.8s-1.1,5.7-3.2,7.8
c-2.1,2.1-4.8,3.3-7.7,3.3C519.2,24.5,514.3,19.6,514.3,13.5 M513.4,13.5c0,6.6,5.3,12,11.9,12c3.2,0,6.1-1.2,8.3-3.5
s3.5-5.3,3.5-8.5s-1.2-6.2-3.5-8.5s-5.2-3.5-8.3-3.5C518.7,1.5,513.4,6.9,513.4,13.5 M520.8,20.6h1.7V15h3.8c0.6,0,1.1,0,1.5,0.2
c0.5,0.3,0.8,0.9,0.9,1.7l0.2,2.2c0,0.5,0,0.8,0,1c0.1,0.2,0.1,0.3,0.2,0.4h2.1v-0.3c-0.3-0.1-0.4-0.4-0.5-0.8
c-0.1-0.2-0.1-0.6-0.1-1.1l-0.1-1.7c0-0.8-0.2-1.3-0.4-1.6c-0.3-0.3-0.7-0.6-1.2-0.8c0.6-0.3,1-0.7,1.3-1.2s0.4-1.1,0.4-1.9
c0-1.5-0.6-2.5-1.7-3.1c-0.6-0.3-1.4-0.4-2.3-0.4h-5.7L520.8,20.6L520.8,20.6z M522.4,13.5V9h4c0.6,0,1.1,0.1,1.4,0.3
c0.6,0.3,0.9,1,0.9,1.9s-0.2,1.5-0.7,1.8c-0.4,0.3-1,0.5-1.8,0.5H522.4z M48.6,30L0,106.4h33.2l46.8-73L48.6,30z M248.7,28.3V0
h-94.5c-12.9,0.8-18.5,7-22.7,11.4c-4.5,4.7-7,14.6-7,14.6V0.2H74.2c-3.8,0-7.4,1.6-10.1,5.4c-2.8,3.7-14.3,22.7-14.3,22.7
L248.7,28.3z M505.7,28.3V0H404.6c-12.8,0.8-18.5,7-22.7,11.4c-4.5,4.7-7,14.6-7,14.6v2.3H505.7z M281.9,0.1h-28.3v28.2h28.3V0.1z
M374.6,0.1h-28.2v28.2h28.2V0.1z M374.6,38.5l-27.8-1.7v32.7c0,0-0.1,9.4-9.2,9.4H290c0,0-8.1-0.7-8.1-9.3V32.4l-28-2v50.1
c4.5,24,26.2,25.9,26.2,25.9s2.2,0.1,2.6,0.2h64.5c0,0,27.6-2.2,27.6-28.8L374.6,38.5L374.6,38.5z M95.7,106.4h132.2
c23-4.6,25.4-25.6,25.4-25.6c1.1-6.2,0.5-11.3,0.5-11.3c-0.7-4.2-8.7-22.8-25.9-24.4c-10.3-1-102-8.3-102-8.3
c1.8,9.2,5.9,13.9,8.7,16.5c6.4,6.1,16.5,7.9,16.5,7.9c2.4,0.2,70.7,5.9,70.7,5.9c2.2,0.1,6.3,0.7,6.2,6.1c0,0.7-0.6,5.4-5.8,5.4
h-98.9v-42l-27.5-2C95.7,34.6,95.7,106.4,95.7,106.4z M374.8,106.6H476c25.4-1.5,29-25.6,29-25.6c0.9-6.3,0.2-11.5,0.2-11.5
c-0.7-5.7-8.8-22.9-26.1-24.6c-10.2-1-102.5-6.4-102.5-6.4c2.7,9.5,6.4,12.4,9.2,15c6.4,6.2,15.9,7.6,15.9,7.6
c2.4,0.2,71.4,6,71.4,6c2.2,0.1,6.7,0.3,6.7,5.7c0,1.6-0.8,3.3-1.8,4.2c0,0-2.4,1.3-4.7,1.3h-98.2L374.8,106.6z"/>
</svg><span class="btn-text">使用 ASUS 登入(CN)</span></button>
</div>

</body>
</html>