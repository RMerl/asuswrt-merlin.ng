var cachedData = {
	"get": {},
	"clear": function(dataArray){$.each(dataArray, function(idx, val){delete cachedData.get[val];})}
}

var asyncData = {
	"get": {},
	"clear": function(dataArray){$.each(dataArray, function(idx, val){delete asyncData.get[val];})}
}

var httpApi ={
	"nvramGet": function(objItems, forceUpdate){
		var queryArray = [];
		var retData = {};

		var __nvramget = function(_nvrams){
			return _nvrams.map(function(elem){return "nvram_get(" + elem + ")";}).join("%3B");
		};

		if(forceUpdate) cachedData.clear(objItems);

		objItems.forEach(function(key){
			if(cachedData.get.hasOwnProperty(key)){
				retData[key] = cachedData.get[key];
			}
			else if(asyncData.get.hasOwnProperty(key)){
				retData[key] = cachedData.get[key] = asyncData.get[key];
				if(forceUpdate) delete asyncData.get[key];
			}
			else{
				queryArray.push(key);
			}
		});

		if(queryArray.length != 0){
			$.ajax({
				url: '/appGet.cgi?hook=' + __nvramget(queryArray),
				dataType: 'json',
				async: false,
				error: function(){
					for(var i=0; i<queryArray.length; i++){retData[queryArray[i]] = "";}
					retData.isError = true;

					$.ajax({
						url: '/appGet.cgi?hook=' + __nvramget(queryArray),
						dataType: 'json',
						error: function(){
							for(var i=0; i<queryArray.length; i++){asyncData.get[queryArray[i]] = "";}
						},
						success: function(response){
							Object.keys(response).forEach(function(key){asyncData.get[key] = response[key];})
						}
					});
				},
				success: function(response){
					Object.keys(response).forEach(function(key){retData[key] = cachedData.get[key] = response[key];})
					retData.isError = false;
				}
			});
		}
		else{
			retData.isError = false;		
		}
		
		return retData;
	},

	"nvramDefaultGet": function(objItems, forceUpdate){
		var queryArray = [];
		var retData = {};

		var __nvramget = function(_nvrams){
			return _nvrams.map(function(elem){return "nvram_default_get(" + elem + ")";}).join("%3B");
		};

		if(forceUpdate) cachedData.clear(objItems);

		objItems.forEach(function(key){
			if(cachedData.get.hasOwnProperty(key + "_default")){
				retData[key] = cachedData.get[key + "_default"];
			}
			else if(asyncData.get.hasOwnProperty(key + "_default")){
				retData[key] = cachedData.get[key + "_default"] = asyncData.get[key + "_default"];
				if(forceUpdate) delete asyncData.get[key + "_default"];
			}
			else{
				queryArray.push(key);
			}
		});

		if(queryArray.length != 0){
			$.ajax({
				url: '/appGet.cgi?hook=' + __nvramget(queryArray),
				dataType: 'json',
				async: false,
				error: function(){
					for(var i=0; i<queryArray.length; i++){retData[queryArray[i]] = "";}
					retData.isError = true;

					$.ajax({
						url: '/appGet.cgi?hook=' + __nvramget(queryArray),
						dataType: 'json',
						error: function(){
							for(var i=0; i<queryArray.length; i++){asyncData.get[queryArray[i] + "_default"] = "";}
						},
						success: function(response){
							Object.keys(response).forEach(function(key){asyncData.get[key + "_default"] = response[key];})
						}
					});
				},
				success: function(response){
					Object.keys(response).forEach(function(key){retData[key] = cachedData.get[key + "_default"] = response[key];})
					retData.isError = false;
				}
			});
		}
		else{
			retData.isError = false;		
		}
		
		return retData;
	},

	"nvramSet": function(postData, handler){
		delete postData.isError;

		$.ajax({
			url: '/applyapp.cgi',
			dataType: 'json',
			data: postData,
			error: function(response){
				if(handler) handler.call(response);
			},
			success: function(response){
				if(handler) handler.call(response);
			}
		})
	},

	"uploadFile": function(postData, handler){
		delete postData.isError;

		var formData = new FormData();
		formData.append('file', postData);

		$.ajax({
			url: 'upload.cgi',
			dataType: 'multipart/form-data',
			data: formData,
			contentType: false,
			processData: false,
			type: 'POST',
			error: function(response){
				if(handler) handler.call(response);
			},
			success: function(response){
				if(handler) handler.call(response);
			}
		 });
	},

	"nvramFormSet": function(postData, handler){
		if(!postData.hasOwnProperty("action")) return false;
		
		$("<iframe>")
			.attr({
				"id": "hiddenFrame",
				"name": "hiddenFrame",
				"width": "0",
				"height": "0",
				"frameborder": "0"
			})
			.appendTo("body")

		var $form = $("<form>", {
			action: postData.action,
			name: "hiddenForm",
			target: "hiddenFrame"
		});

		delete postData.action;	
		Object.keys(postData).forEach(function(key){
			$form.append($("<input>", {
				type: "hidden",
				name: key,
				value: postData[key]
			}));			
		})

		$form.appendTo("body").submit().remove();
		$("#hiddenFrame").remove();
	},

	"hookGet": function(hookName, forceUpdate){
		var queryString = hookName.split("-")[0] + "(" + (hookName.split("-")[1] || "") + ")";
		var retData = {};

		if(cachedData.get.hasOwnProperty(hookName) && !forceUpdate){
			retData[hookName] = cachedData.get[hookName];
		}
		else if(asyncData.get.hasOwnProperty(hookName)){
			retData[hookName] = asyncData.get[hookName];
			if(forceUpdate) delete asyncData.get[hookName];
		}
		else{
			$.ajax({
				url: '/appGet.cgi?hook=' + queryString,
				dataType: 'json',
				async: false,
				error: function(){
					retData[hookName] = "";
					retData.isError = true;
			
					$.ajax({
						url: '/appGet.cgi?hook=' + queryString,
						dataType: 'json',
						error: function(){
							asyncData.get[hookName] = "";
						},
						success: function(response){
							asyncData.get[hookName] = response[hookName];
						}
					});
				},
				success: function(response){
					retData = response;
					cachedData.get[hookName] = response[hookName]
					retData.isError = false;
				}
			});
		}

		return retData;
	},

	"startAutoDet": function(){
		$.get("/appGet.cgi?hook=start_force_autodet()");
	},

	"detwanGetRet": function(){
		var wanInfo = httpApi.nvramGet(["wan0_state_t", "wan0_sbstate_t", "wan0_auxstate_t", "autodet_state", "autodet_auxstate", "wan0_proto", "link_internet"], true);
	
		var wanTypeList = {
			"dhcp": "DHCP",
			"static": "Static",
			"pppoe": "PPPoE",
			"l2tp": "L2TP",
			"pptp": "PPTP",
			"modem": "MODEM",
			"check": "CHECKING",
			"resetModem": "RESETMODEM",
			"connected": "CONNECTED",
			"noWan": "NOWAN"
		}

		var retData = {
			"wanType": "CHECKING",
			"isIPConflict": (function(){
				return (wanInfo.wan0_state_t == "4" && wanInfo.wan0_sbstate_t == "4")
			})(),
			"isError": false
		};

		var hadPlugged = function(deviceType){
			var usbDeviceList = httpApi.hookGet("show_usb_path")["show_usb_path"][0] || [];
			return (usbDeviceList.join().search(deviceType) != -1)
		}

		if(wanInfo.isError){
			retData.wanType = wanTypeList.check;
			retData.isIPConflict = false;
			retData.isError = true;
		}
		else if(
			wanInfo.link_internet   == "2" &&
			wanInfo.wan0_state_t    == "2" &&
			wanInfo.wan0_sbstate_t  == "0" &&
			wanInfo.wan0_auxstate_t == "0"
		){
			retData.wanType = wanTypeList.connected;
		}
		else if(wanInfo.autodet_state == ""){
			retData.wanType = wanTypeList.check;			
		}
		else if(wanInfo.autodet_state == "6" || wanInfo.autodet_auxstate == "6"){
			retData.wanType = wanTypeList.pppoe;
		}
		else if(hadPlugged("modem")){
			retData.wanType = wanTypeList.modem;
		}
		else if(wanInfo.wan0_auxstate_t == "1"){
			retData.wanType = wanTypeList.noWan;
		}
/*
		else if(wanInfo.autodet_state == "2"){
			retData.wanType = wanTypeList.dhcp;
		}
*/
		else if(wanInfo.autodet_state == "3" || wanInfo.autodet_state == "5"){
			retData.wanType = wanTypeList.resetModem;
		}
		else if(wanInfo.autodet_state == "4"){
			if(wanInfo.wan0_auxstate_t != "1"){
				httpApi.startAutoDet();
				retData.wanType = wanTypeList.check;
				retData.isIPConflict = false;
				retData.isError = false;
			}
			else if(wanInfo.wan0_state_t == "4" && wanInfo.wan0_sbstate_t == "4"){
				retData.wanType = wanTypeList.dhcp;
				retData.isIPConflict = true;
			}
			else{
				retData.wanType = wanTypeList.noWan;
			}
		}
		else{
			retData.wanType = wanTypeList.check;
		}

		return retData;
	},

	"isPppAuthFail": function(){
		if(window.pppAuthFailChecked) return false;

		var wanInfo = httpApi.nvramGet(["wan0_state_t", "wan0_sbstate_t", "wan0_auxstate_t", "wan0_proto", "sw_mode"], true);
		var result = (	
			wanInfo.sw_mode         == "1"     &&
			wanInfo.wan0_proto	    == "pppoe" &&
			wanInfo.wan0_state_t    == "4"     &&
			wanInfo.wan0_sbstate_t  == "2"     &&
			wanInfo.wan0_auxstate_t == "0"
		)

		window.pppAuthFailChecked = result;
		return result;
	},

	"isConnected": function(){
		var wanInfo = httpApi.nvramGet(["wan0_state_t", "wan0_sbstate_t", "wan0_auxstate_t", "link_internet"], true);
		return (
			wanInfo.link_internet   == "2" &&
			wanInfo.wan0_state_t    == "2" &&
			wanInfo.wan0_sbstate_t  == "0" &&
			wanInfo.wan0_auxstate_t == "0"
		)
	},

	"isAlive": function(hostOrigin, token, callback){
		window.chdom = callback;
		$.getJSON(hostOrigin + "/chdom.json?hostname=" + token + "&callback=?");
	},

	"checkCap": function(targetOrigin, targetId){
		window.chcap = function(){
			setTimeout(function(){
				if(isPage("amasconncap_page")) window.location.href = targetOrigin + "/cfg_onboarding.cgi?id=" + targetId;
			}, 3000);

			// $("#connCapAlert").hide();
			$("#loginCapAlert").fadeIn(500);
		}

		$.getJSON(targetOrigin + "/chcap.json?callback=?");
	},

	"cleanLog": function(path, callback) {
		if(path != "") {
			var confirm_flag = confirm("Data will not be able to recover once deleted, are you sure you want to clean?");/*untranslated*/
			if(confirm_flag) {
				$.ajax({
					url: '/cleanlog.cgi?path=' + path,
					dataType: 'script',	
					error: function(xhr) {
						alert("Clean error!");/*untranslated*/
					},
					success: function(response) {
						if(typeof callback == "function")
							callback();
					}
				});
			}
		}
		else {
			alert("Clean error, no path!");/*untranslated*/
		}
	},

	"faqURL": function(_Objid, _faqNum, _URL1, _URL2){
		// https://www.asus.com/tw/support/FAQ/1000906
		var pLang = httpApi.nvramGet(["preferred_lang"]).preferred_lang;		
		var faqLang = {
			EN : "",
			TW : "/tw",
			CN : ".cn",
			BR : "/br",
			CZ : "/cz",
			DA : "/dk",
			DE : "/de",
			ES : "/es",
			FI : "/fi",
			FR : "/fr",
			HU : "/hu",
			IT : "/it",
			JP : "/jp",
			KR : "/kr",
			MS : "/my",
			NL : "/nl",
			NO : "/no",
			PL : "/pl",
			RO : "/ro",
			RU : "/ru",
			SL : "/sk",
			SV : "/se",
			TH : "/th",
			TR : "/tr",
			UK : "/ua"
		}
		var temp_URL_lang = _URL1+faqLang[pLang]+_URL2+_faqNum;
		var temp_URL_global = _URL1+_URL2+_faqNum;
		//console.log(temp_URL_lang);

		if (window.location.hostname == "router.asus.com") {
			$.ajax({
				url: temp_URL_lang,
				type: 'GET',
				timeout: 1500,
				error: function(response){
					//console.log(response);
					document.getElementById(_Objid).href = temp_URL_global;
				},
				success: function(response) {				
					//console.log(response);
					if(response.search("QAPage") >= 0)
						document.getElementById(_Objid).href =  temp_URL_lang;
					else
						document.getElementById(_Objid).href = temp_URL_global;		
				}
			});
		} else {
			document.getElementById(_Objid).href = temp_URL_lang;
		}
	},

	"nvram_match_x": function(postData, compareData, retData){
		var queryString = "nvram_match_x(\"\",\""+postData+"\",\""+compareData+"\",\""+retData+"\")";
		var retData = {};

		$.ajax({
			url: '/appGet.cgi?hook=' + queryString,
			dataType: 'json',
			async: false,
			error: function(){
				retData[postData] = "";
				retData.isError = true;
			},
			success: function(response){
				retData[postData] = response["nvram_match_x-"];
				retData.isError = false;
			}
		});

		return retData;
	}
}
