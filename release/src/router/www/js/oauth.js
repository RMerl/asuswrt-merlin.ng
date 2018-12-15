if (Function.prototype.name === undefined && Object.defineProperty !== undefined) {
	Object.defineProperty(Function.prototype, 'name', {
		get: function() {
			var funcNameRegex = /function\s([^(]{1,})\(/;
			var results = (funcNameRegex).exec((this).toString());
			return (results && results.length > 1) ? results[1].trim() : "";
		},
		set: function(value) {}
	});
}

var oauthServer = "https://oauth.asus.com/aicloud/";
var oauthCallBack = "oauth_callback.htm";
var oauthVariable = {
	"domainName" : (function() {
		var header_info = [<% get_header_info(); %>][0];
		return header_info.protocol + "://" + header_info.host + ":" + header_info.port;
	})(),
	"google" : {
		"server" : "https://accounts.google.com/o/oauth2/auth?",
		"response_type" : "code",
		"client_id" : "103584452676-437qj6gd8o9tuncit9h8h7cendd2eg58.apps.googleusercontent.com",
		"redirect_uri" : encodeURIComponent("" + oauthServer + "google_authorization_code.html"),
		"scope" : "https://www.googleapis.com%2Fauth%2Fgmail.send+https://www.googleapis.com%2Fauth%2Fuserinfo.email"
	},
	"dropbox" : {
		"server" : "https://www.dropbox.com/oauth2/authorize?",
		"response_type" : "token",
		"client_id" : "qah4ku73k3qmigj",
		"redirect_uri" : encodeURIComponent("" + oauthServer + "dropbox.html")
	}
};
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
var gen_callback_url = function(_callBackFunc) {
	var callback_url = oauthVariable.domainName + "/" + oauthCallBack + "," +  _callBackFunc + "";
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
var setListener = function(_callBackFunc) {
	var eventMethod = window.addEventListener ? "addEventListener" : "attachEvent";
	var eventer = window[eventMethod];
	var messageEvent = eventMethod == "attachEvent" ? "onmessage" : "message";
	eventer(messageEvent, function(e) {
		if (e.origin !== oauthVariable.domainName)
			return;

		var key = e.message ? "message" : "data";
		var data = e[key];

		_callBackFunc(data);

		//get value message send to oauth_callback.htm
		e.source.postMessage("close", e.origin);
	}, false);
};

var oauth = {};
oauth.google = function(_callBackFunc) {
	var url = oauthVariable.google.server;
	url += "response_type=" + oauthVariable.google.response_type;
	url += "&redirect_uri=" + oauthVariable.google.redirect_uri;
	url += "&client_id=" + oauthVariable.google.client_id;
	url += "&scope=" + oauthVariable.google.scope;
	url += "&state=base64_" + base64Encode(gen_callback_url(_callBackFunc.name));
	url += "&approval_prompt=force&access_type=offline";
	window.open(url,"mywindow","menubar=1,resizable=1,width=600,height=470,top=100,left=300");
	setListener(_callBackFunc);
};
oauth.dropbox = function(_callBackFunc) {
	var url = oauthVariable.dropbox.server;
	url += "response_type=" + oauthVariable.dropbox.response_type;
	url += "&client_id=" + oauthVariable.dropbox.client_id;
	url += "&redirect_uri=" + oauthVariable.dropbox.redirect_uri;
	url += "&state=base64_" + base64Encode(gen_callback_url(_callBackFunc.name));
	url += "&force_reapprove=true&force_reauthentication=true";
	window.open(url,"mywindow","menubar=1,resizable=1,width=630,height=550");
	setListener(_callBackFunc);
}

