var username = getUrlVar("username");
var password = getUrlVar("password");
var chal = getUrlVar("chal");
var login = getUrlVar("login");
var logout = getUrlVar("logout");
var prelogin = getUrlVar("prelogin");
var res = getUrlVar("res");
var uamip = getUrlVar("uamip");
var uamport = getUrlVar("uamport");
var timeleft = getUrlVar("timeleft");
var userurl = getUrlVar("userurl");
var challenge = getUrlVar("challenge");

function sanitizeInput(input) {
    //- Remove or replace common XSS attack characters
	// < and >
	// ;
	// =
	// '
	// "
    return input.replace(/[<>;='"]/g, '');
}

function isValidKey(key) {
    // Only letters, numbers and underscores allowed
    var regex = /^[a-zA-Z0-9_]+$/;
    return regex.test(key);
}

function isUnsafeKey(key) {
    return key === '__proto__' || key === 'constructor' || key === 'prototype';
}

function encodeSafeString(input){
    input = input.replace(/&/g, '&amp;');
    input = input.replace(/<script/g, '<script&lt;');
    input = input.replace(/script>/g, 'script&gt;');
    input = input.replace(/%0A/g, '');
    input = input.replace(/%0D/g, '');
    return input;
}

function getUrlVar(key_name) {
	var url = new URL(window.location.href); //- Safely parsing URLs using the URL API
	var query = url.searchParams.toString(); // Get the query string
	var hashes = query.split('&');
	var key_value = "";

	for (var i = 0; i < hashes.length; i++) {
		var hash = hashes[i].split('=');
		var key = sanitizeInput(decodeURIComponent(hash[0]));
		var value = sanitizeInput(decodeURIComponent(hash[1] || ''));

		if (isValidKey(key) && !isUnsafeKey(key) && key==key_name) {
			key_value = encodeSafeString(value);
		}
	}

	return key_value;
}

$("document").ready(function() {	
	//alert("res=" + res + ", username=" + username + ", password=" + password);
	
	var divObj = document.createElement("div");
	divObj.setAttribute("id","login_section");
	divObj.setAttribute("style","display: none;");
	var html = "";
	html +='<form name="form" method="get" action="/Uam">';
	html +='<input type="hidden" name="chal" id="chal" value="">';
	html +='<input type="hidden" name="uamip" id="uamip" value="">';
	html +='<input type="hidden" name="uamport" id="uamport" value="">';
	html +='<input type="hidden" name="userurl" id="userurl" value="">';
	html +='<center>';
	html +='<table border="0" cellpadding="5" cellspacing="0" style="width: 217px;">';
	html +='<tbody>';
	html +='<tr>';
	html +='<td align="right">Login:</td>';
	html +='<td><input type="text" name="UserName" id="UserName" size="20" maxlength="255"></td>';
	html +='</tr>';
	html +='<tr>';
	html +='<td align="right">Password:</td>';
	html +='<td><input type="password" name="Password" id="Password" size="20" maxlength="255"></td>';
	html +='</tr>';
	html +='<tr>';
	html +='<td align="center" colspan="2" height="23"><input type="button" onclick="formSubmit(1)" value="Submit"><!--<input type="submit" name="login" value="login">--></td>';
	html +='</tr>';
	html +='<tr>';
	html +='<td align="center" colspan="2" height="23"><input type="button" onclick="formSubmit(0)" value="OK"></td>';
	html +='</tr>';
	html +='</tbody>';
	html +='</table>';
	html +='</center>';
	html +='</form>';
	html +='</div>';
	html +='<div id="success_section" style="display: none;">';
	html +='<center><a href="" id="logoff_a">Logout</a></center>';
	html +='</div>';
	html +='<div id="logoff_section" style="display: none;">';
	html +='center><a href="" id="prelogin_a">Login</a></center>';
	divObj.innerHTML = html;
	document.body.appendChild(divObj);
	
	if (res == "notyet") {
		document.getElementById('chal').value = challenge;
		document.getElementById('uamip').value = uamip;
		document.getElementById('uamport').value = uamport;
		document.getElementById('userurl').value = userurl;
	}
	else if (res == "success") {
		if (userurl != null && userurl != undefined && userurl != '') {
			var urlPattern = /^(https?:\/\/[^\s/$.?#].[^\s]*)$/i; // Only support http or https protocols
			if (urlPattern.test(userurl)) {
				window.location = decodeURIComponent(userurl);
			}
			else {
				alert("Invalid URL format. Please check the URL.");
			}
		}
		else
		{
			document.getElementById("logoff_a").href = "http://" + uamip + ":" + uamport + "/logoff";
		}
	}
	else if (res == "failed") {
		alert("The account and password you entered don't match.");
		document.getElementById('chal').value = challenge;
		document.getElementById('uamip').value = uamip;
		document.getElementById('uamport').value = uamport;
		document.getElementById('userurl').value = userurl;
	}
	else if (res == "logoff") {
		document.getElementById("prelogin_a").href = "http://" + uamip + ":" + uamport + "/prelogin";
		//document.getElementById('logoff_section').style.display = "block";
	}
	else if (res == "already") {
		document.getElementById("logoff_a").href = "http://" + uamip + ":" + uamport + "/logoff";
	}
	
});

function formSubmit(auth)
{
	var param = {};
	param.chal = document.getElementById('chal').value;
	param.uamip = document.getElementById('uamip').value;
	param.uamport = document.getElementById('uamport').value;
	param.userurl = document.getElementById('userurl').value;
	switch(auth) {
		case 0 :
			break;
		case 1 : //captive portal
			document.getElementById('UserName').value = document.getElementById('splash_page_account').value;
			if(document.getElementById('splash_page_password') == null)
				document.getElementById('Password').value = "noauth";
			else
				document.getElementById('Password').value = document.getElementById('splash_page_password').value;

			param.UserName = document.getElementById('UserName').value;
			param.Password = document.getElementById('Password').value;
			break;
		case 2 : //free wifi passcode
			document.getElementById('UserName').value = "noauth";

			if(document.getElementById('splash_template_passcode') == null)
				document.getElementById('Password').value = "noauth";
			else
				document.getElementById('Password').value = document.getElementById('splash_template_passcode').value;

			param.UserName = document.getElementById('UserName').value;
			param.Password = document.getElementById('Password').value;
			break;
	}
	param.login = "login";

	$.ajax({
	    	url: '',
	  	data: param,
	      	type: 'GET',
	      	dataType: 'text',
		timeout: 20000,
		error: function(){
		      alert('fail');
		},
		success: function(data){
		      //- success
			window.location = data;
		}
	});

}

