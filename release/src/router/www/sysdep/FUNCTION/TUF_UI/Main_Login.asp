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
<link rel="stylesheet" type="text/css" href="css/main_login.css">
<script type="text/javaScript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/js/https_redirect/https_redirect.js"></script>
<title><#Web_Title#></title>
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

var htmlEnDeCode = (function() {
	var charToEntityRegex,
		entityToCharRegex,
		charToEntity,
		entityToChar;

	function resetCharacterEntities() {
		charToEntity = {};
		entityToChar = {};
		// add the default set
		addCharacterEntities({
			'&amp;'	 :   '&',
			'&gt;'	  :   '>',
			'&lt;'	  :   '<',
			'&quot;'	:   '"',
			'&#39;'	 :   "'"
		});
	}

	function addCharacterEntities(newEntities) {
		var charKeys = [],
			entityKeys = [],
			key, echar;
		for (key in newEntities) {
			echar = newEntities[key];
			entityToChar[key] = echar;
			charToEntity[echar] = key;
			charKeys.push(echar);
			entityKeys.push(key);
		}
		charToEntityRegex = new RegExp('(' + charKeys.join('|') + ')', 'g');
		entityToCharRegex = new RegExp('(' + entityKeys.join('|') + '|&#[0-9]{1,5};' + ')', 'g');
	}

	function htmlEncode(value){
		var htmlEncodeReplaceFn = function(match, capture) {
			return charToEntity[capture];
		};

		return (!value) ? value : String(value).replace(charToEntityRegex, htmlEncodeReplaceFn);
	}

	function htmlDecode(value) {
		var htmlDecodeReplaceFn = function(match, capture) {
			return (capture in entityToChar) ? entityToChar[capture] : String.fromCharCode(parseInt(capture.substr(2), 10));
		};

		return (!value) ? value : String(value).replace(entityToCharRegex, htmlDecodeReplaceFn);
	}

	resetCharacterEntities();

	return {
		htmlEncode: htmlEncode,
		htmlDecode: htmlDecode
	};
})();

var login_info =  tryParseJSON('<% login_error_info(); %>');
var isIE8 = navigator.userAgent.search("MSIE 8") > -1; 
var isIE9 = navigator.userAgent.search("MSIE 9") > -1; 
var remaining_time = login_info.lock_time;
var remaining_time_min;
var remaining_time_sec;
var remaining_time_show;
var countdownid, rtime_obj;
var redirect_page = login_info.page;
var cloud_file = '<% get_parameter("file"); %>';
var isRouterMode = ('<% nvram_get("sw_mode"); %>' == '1') ? true : false;
var CoBrand = '<% nvram_get("CoBrand"); %>';

var header_info = [<% get_header_info(); %>][0];
var ROUTERHOSTNAME = '<#Web_DOMAIN_NAME#>';
var domainNameUrl = header_info.protocol+"://"+ROUTERHOSTNAME+":"+header_info.port;
var chdom = function(){window.location.href=domainNameUrl};
(function(){
	if(ROUTERHOSTNAME !== header_info.host && ROUTERHOSTNAME != "" && isRouterMode){
		setTimeout(function(){
			var s=document.createElement("script");s.type="text/javascript";s.src=domainNameUrl+"/chdom.json?hostname="+header_info.host;var h=document.getElementsByTagName("script")[0];h.parentNode.insertBefore(s,h);
		}, 1);
	}
	if(CoBrand == "8")
		$('link').last().after('<link rel="stylesheet" type="text/css" href="css/difference.css">');
})();
<% login_state_hook(); %>

function isSupport(_ptn){
	var ui_support = [<% get_ui_support(); %>][0];
	return (ui_support[_ptn]) ? ui_support[_ptn] : 0;
}
var odm_support = isSupport("odm");
var captcha_support = isSupport("captcha");
var captcha_enable = htmlEnDeCode.htmlEncode(decodeURIComponent('<% nvram_char_to_ascii("", "captcha_enable"); %>'));
if(captcha_support && captcha_enable != "0")
	var captcha_on = (login_info.error_num >= 2 && login_info.error_status != "7" && login_info.error_status != "11")? true : false;
else
	var captcha_on = false;

var faq_href = "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=SG_TeleStand&lang=&kw=&num=";
function initial(){
	top.name = "";/* reset cache of state.js win.name */

	/*handle sysdep for ROG or ODM product*/
	if(odm_support){
		document.getElementsByClassName("main-field-bg")[0].className = "main-field-bg-odm";
		document.getElementsByClassName("logo-container")[0].className = "logo-container-odm";
		document.getElementsByClassName("model-name")[0].className = "logo-odm";
		document.getElementsByClassName("login-bg")[0].className = "login-bg-odm";
		document.getElementsByClassName("login-btn-bg")[0].className = "login-btn-bg-odm";
		document.getElementsByClassName("login-title")[0].style.display = "block";
		document.getElementsByClassName("login-field-padding")[0].className = "login-field-padding-odm";
	}
	else{
		document.getElementsByClassName("model-name")[0].innerHTML = "<#Web_Title2#>"
	}
	
	var flag = login_info.error_status;
	if(isIE8 || isIE9){
		document.getElementById("name_title_ie").style.display ="";
		document.getElementById("password_title_ie").style.display ="";
	}

	if(flag != 11 && login_info.last_time_lock_warning){
		document.getElementById("last_time_lock_warning").style.display ="";
		document.getElementById("last_time_lock_warning").innerHTML ="You have entered an incorrect username or password 9 times. If there's one more failed account or password attempt, your router will be blocked from accessing, and need to be reset to factory setting.";
	}

	if(flag != ""){
		document.getElementById("error_status_field").style.display ="";
		if(flag == 3){
			document.getElementById("error_status_field").innerHTML ="<div class='error-text'>* <#JS_validLogin#></div>";
		}
		else if(flag == 7){
			document.getElementById("error_status_field").innerHTML ="<div class='error-text'>You have entered an incorrect username or password 5 times. Please try again after "+"<span id='rtime'></span>"+" seconds.</div>";
	
			disable_input(1);
			disable_button(1);
			rtime_obj=document.getElementById("rtime");
			countdownfunc();
			countdownid = window.setInterval(countdownfunc,1000);
		}
		else if(flag == 8){
			document.getElementById("login_filed").style.display ="none";
			document.getElementById("hint_field").style.display ="";
			var element = document.createElement('div');
			element.className = 'logout-text';
			element.innerHTML = '<#logoutmessage#>';
			document.getElementById('hint_field').appendChild(element);
		}
		else if(flag == 9){
			var loginUserIp = (function(){
				return (typeof login_ip_str === "function") ? login_ip_str().replace("0.0.0.0", "") : "";
			})();

			var getLoginUser = function(){
				if(loginUserIp === "") return "";

				var dhcpLeaseInfo = [];
				var hostName = "";

				dhcpLeaseInfo.forEach(function(elem){
				if(elem[0] === loginUserIp){
					hostName = " (" + elem[1] + ")";
					return false;
					}
				})

				return "<div>* <#login_hint1#> " + loginUserIp + hostName + "</div>";
			};

			document.getElementById("login_filed").style.display ="none";
			document.getElementById("hint_field").style.display ="";

			var element = document.createElement('div');
			element.setAttribute('class', 'nologin-container');
			var _child_element = document.createElement('div');
			_child_element.setAttribute('class', 'nologin-text');
			_child_element.innerHTML = '<#login_hint2#>';
			element.appendChild(_child_element);

			_child_element = document.createElement('div');
			_child_element.setAttribute('id', 'logined_ip_str');
			element.appendChild(_child_element);
			document.getElementById('hint_field').appendChild(element);
			document.getElementById("logined_ip_str").innerHTML = getLoginUser();
		}
		else if(flag == 10){
			document.getElementById("error_status_field").style.display ="none";
			document.getElementById("error_captcha_field").style.display ="";
		}
		else if(flag == 11){
			document.getElementById("error_status_field").innerHTML ="For security reasons, this router has been locked out because of 10 times of incorrect username and password attempts.<br>To unlock, please manually reset your router to factory setting by pressing the reset button on the back.<br>Click <a id=\"faq_SG\" href=\"\" target=\"_blank\" style=\"color:#FC0;text-decoration:underline;\">here</a> for more details.";
			document.getElementById("faq_SG").href = faq_href;
			document.getElementById("error_status_field").className = "error_hint error_hint1";
			disable_input(1);
			disable_button(1);
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
			return false;
		}
	};

	document.form.login_passwd.onkeyup = function(e){
		e=e||event;
		if(e.keyCode == 13){
			if(captcha_on)
				document.form.captcha_text.focus();
			else
				preLogin();
			return false;
		}
	};
	document.form.login_passwd.onkeypress = function(e){
		e=e||event;
		if(e.keyCode == 13){
			return false;
		}
	};

	if(captcha_on){
		var timestamp = new Date().getTime();
		var captcha_pic = document.getElementById("captcha_pic");
		captcha_pic.src = "captcha.gif?t=" + timestamp;

		document.form.captcha_text.onkeyup = function(e){
			e=e||event;
			if(e.keyCode == 13){
				preLogin();
				return false;
			}
		};

		document.form.captcha_text.onkeypress = function(e){
			e=e||event;
			if(e.keyCode == 13){
				return false;
			}
		};

		document.getElementById("captcha_field").style.display = "";
	}
	else
		document.getElementById("captcha_field").style.display = "none";

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

function preLogin(){
    if(document.querySelector('#button')?.classList.contains('disabled') || document.querySelector('.button')?.classList.contains('disabled')) return;
    document.querySelector('#button')?.classList.add('disabled');
    document.querySelector('.button')?.classList.add('disabled');
    let id = randomString(10);
    fetch('get_Nonce.cgi', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({id: id})
    })
    .then(response => response.json())
    .then(data => {
        const { nonce } = data;
        login(id, nonce);
    })
    .catch(error => top.location.href='/Main_Login.asp');
}

function login(id, nonce){

    const cnonce = randomString(32);

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

	document.form.id.value = id;
    document.form.cnonce.value = cnonce;
	document.form.login_authorization.value = sha256(`${document.form.login_username.value}:${nonce}:${document.form.login_passwd.value}:${cnonce}`);
	document.form.login_username.disabled = true;
	document.form.login_passwd.disabled = true;
	document.form.login_captcha.value = btoa(document.form.captcha_text.value);
	document.form.captcha_text.disabled = true;

	try{
		if(redirect_page == "" 
			|| redirect_page == "Logout.asp" 
			|| redirect_page == "Main_Login.asp" 
			|| redirect_page.indexOf(" ") != -1 
			|| redirect_page.indexOf("//") != -1 
			|| redirect_page.indexOf("http") != -1
			|| (redirect_page.indexOf(".asp") == -1 && redirect_page.indexOf(".htm") == -1 && redirect_page != "send_IFTTTPincode.cgi" && redirect_page != "cfg_onboarding.cgi" && redirect_page != "ig_s2s_link.cgi")
		){
			document.form.next_page.value = "";
		}
		else{
			document.form.next_page.value = redirect_page;
		}
	}
	catch(e){
		document.form.next_page.value = "";
	}		

	if(document.form.next_page.value == "cloud_sync.asp"){
		document.form.cloud_file.disabled = false;
		document.form.cloud_file.value = cloud_file;
	}

	document.form.submit();
}

function disable_input(val){
	var disable_input_x = document.getElementsByClassName('form-input');
	for(i=0;i<disable_input_x.length;i++){
		if(val == 0)
			disable_input_x[i].disabled = true;
		else
			disable_input_x[i].style.display = "none";
	}
}

function disable_button(val){
	if(val == 0)
		document.getElementById('button').disabled = true;
	else
		document.getElementById('button').style.display = "none";
}

function checkTime(i){
	if (i<10){
		i="0" + i
	}
	return i
}

function regen_captcha(){
	var timestamp = new Date().getTime();
	var captcha_pic = document.getElementById("captcha_pic");
	var queryString = "?t=" + timestamp;
	captcha_pic.src = "captcha.gif" + queryString;
}

function randomString(length) {
    let chars = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ';
    let result = '';
    for (var i = length; i > 0; --i) result += chars[Math.floor(Math.random() * chars.length)];
    return result;
}

const sha256 = function a(b){function c(a,b){return a>>>b|a<<32-b}for(var d,e,f=Math.pow,g=f(2,32),h="length",i="",j=[],k=8*b[h],l=a.h=a.h||[],m=a.k=a.k||[],n=m[h],o={},p=2;64>n;p++)if(!o[p]){for(d=0;313>d;d+=p)o[d]=p;l[n]=f(p,.5)*g|0,m[n++]=f(p,1/3)*g|0}for(b+="\x80";b[h]%64-56;)b+="\x00";for(d=0;d<b[h];d++){if(e=b.charCodeAt(d),e>>8)return;j[d>>2]|=e<<(3-d)%4*8}for(j[j[h]]=k/g|0,j[j[h]]=k,e=0;e<j[h];){var q=j.slice(e,e+=16),r=l;for(l=l.slice(0,8),d=0;64>d;d++){var s=q[d-15],t=q[d-2],u=l[0],v=l[4],w=l[7]+(c(v,6)^c(v,11)^c(v,25))+(v&l[5]^~v&l[6])+m[d]+(q[d]=16>d?q[d]:q[d-16]+(c(s,7)^c(s,18)^s>>>3)+q[d-7]+(c(t,17)^c(t,19)^t>>>10)|0),x=(c(u,2)^c(u,13)^c(u,22))+(u&l[1]^u&l[2]^l[1]&l[2]);l=[w+x|0].concat(l),l[4]=l[4]+w|0}for(d=0;8>d;d++)l[d]=l[d]+r[d]|0}for(d=0;8>d;d++)for(e=3;e+1;e--){var y=l[d]>>8*e&255;i+=(16>y?0:"")+y.toString(16)}return i};

</script>
</head>
<body onload="initial();" class="bg_mainLogin">
<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0"></iframe>
<iframe id="dmRedirection" width="0" height="0" frameborder="0" scrolling="no" src=""></iframe>
<form method="post" name="form" action="login_v2.cgi" target="">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="current_page" value="Main_Login.asp">
<input type="hidden" name="next_page" value="Main_Login.asp">
<input type="hidden" name="login_authorization" value="">
<input type="hidden" name="id" value="">
<input type="hidden" name="cnonce" value="">
<input type="hidden" name="login_captcha" value="">
<input type="hidden" name="cloud_file" value="" disabled>
<div class="main-field-bg">
	<div class="main-field-padding">
		<div class="logo-container">
			<div class="logo-rog"></div>
			<div class="model-name"></div>
		</div>

		<div class="login-bg">
			<div class="login-field-padding">
				<div class="login-title" style="display:none"><#CTL_signin#></div>
				<!-- Login field -->
				<div id="login_filed">
					<div class="login-title-desc"><#Sign_in_title#></div>
					<div id="name_title_ie" style="display:none" class="p1"><#Username#></div>
					<div class="input-container">
						<input type="text" id="login_username" name="login_username" tabindex="1" class="form-input" maxlength="128" autocapitalize="off" autocomplete="off" placeholder="<#Username#>">
					</div>
					<div id="password_title_ie" style="display:none" class="p1"><#HSDPAConfig_Password_itemname#></div>
					<div class="input-container">
						<input type="password" name="login_passwd" tabindex="2" class="form-input" maxlength="128" placeholder="<#HSDPAConfig_Password_itemname#>" autocapitalize="off" autocomplete="off">
					</div>
					<div id="error_status_field" class="error-hint-bg" style="display: none;" ></div>
					<div id="last_time_lock_warning" class="warming_desc" style="display:none;" ></div>
					<div class="input-container">
						<div id="captcha_field" style="display: none;">
							<div id="captcha_input_div"><input id ="captcha_text" name="captcha_text" tabindex="3" maxlength="5" autocapitalize="off" autocomplete="off"></div>
							<div id="captcha_img_div"><img id="captcha_pic"></div>
							<div id="reCaptcha" onclick="regen_captcha();"></div>
							<div class="error_hint" style="display:none; clear:left;" id="error_captcha_field">Captcha is wrong. Please input again.</div>
						</div>
					</div>
					<div id="button" class="login-btn-bg" onclick="preLogin();"><#CTL_signin#></div>
				</div>
 
				<!-- Message field --> 
				<div id="hint_field" class="error-hint-bg" style="display: none;"></div>
			</div>
		</div>
	</div>
</div>
</form>
</body>
</html>
