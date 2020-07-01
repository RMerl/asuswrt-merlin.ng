var cachedData = {
	"get": {},
	"clear": function(dataArray){$.each(dataArray, function(idx, val){delete cachedData.get[val];})}
}

var asyncData = {
	"get": {},
	"clear": function(dataArray){$.each(dataArray, function(idx, val){delete asyncData.get[val];})}
}

var httpApi ={
	"detRetryCnt_MAX": 10,
	"detRetryCnt": this.detRetryCnt_MAX,

	"nvramGetAsync": function(q){
		if(!q.success || !q.data) return false;

		var __nvramget = function(_nvrams){
			return _nvrams.map(function(elem){return "nvram_char_to_ascii(" + elem + "," + elem + ")";}).join("%3B");
		};

		$.ajax({
			url: '/appGet.cgi?hook=' + __nvramget(q.data),
			dataType: 'json',
			error: q.error,
			success: function(encNvram){
				var decNvram = {};
				for (var name in encNvram){decNvram[name] = decodeURIComponent(encNvram[name]);}
				q.success(decNvram);
			}
		});
	},

	"hookGetAsync": function(q){
		if(!q.success || !q.data) return false;

		var queryString = q.data.split("-")[0] + "(" + (q.data.split("-")[1] || "") + ")";

		$.ajax({
			url: '/appGet.cgi?hook=' + queryString,
			dataType: 'json',
			error: q.error,
			success: function(res){
				q.success(res[q.data]);
			}
		});
	},

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

	"nvramCharToAscii": function(objItems, forceUpdate){
		var queryArray = [];
		var retData = {};

		var __nvramget = function(_nvrams){
			return _nvrams.map(function(elem){return "nvram_char_to_ascii(" + elem + "," + elem + ")";}).join("%3B");
		};

		objItems.forEach(function(key){
			if(asyncData.get.hasOwnProperty(key)){
				retData[key] = asyncData.get[key];
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
					Object.keys(response).forEach(function(key){retData[key] = response[key];})
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
			error: function(){},
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

		return retData[hookName];
	},

	"startAutoDet": function(){
		$.get("/appGet.cgi?hook=start_force_autodet()");
	},

	"detwanGetRet": function(){
		var wanInfo = httpApi.nvramGet(["wan0_state_t", "wan0_sbstate_t", "wan0_auxstate_t", "autodet_state", "autodet_auxstate", "wan0_proto",
										 "link_internet", "x_Setting", "link_wan"], true);

		var wanTypeList = {
			"dhcp": "DHCP",
			"static": "STATIC",
			"pppoe": "PPPoE",
			"l2tp": "L2TP",
			"pptp": "PPTP",
			"modem": "MODEM",
			"check": "CHECKING",
			"resetModem": "RESETMODEM",
			"connected": "CONNECTED",
			"pppdhcp": "PPPDHCP",
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
			var usbDeviceList = httpApi.hookGet("show_usb_path") || [];
			return (usbDeviceList.join().search(deviceType) != -1)
		}

		var iCanUsePPPoE = (wanInfo.autodet_state == "6" || wanInfo.autodet_auxstate == "6")

		if(wanInfo.isError){
			retData.wanType = wanTypeList.check;
			retData.isIPConflict = false;
			retData.isError = true;
		}
		else if(wanInfo.link_wan == ""){
			retData.wanType = wanTypeList.check;
		}
		else if(wanInfo.link_wan == "0"){
			retData.wanType = wanTypeList.noWan;
		}
		else if(
			wanInfo.link_internet   == "2" &&
			wanInfo.wan0_state_t    == "2" &&
			wanInfo.wan0_sbstate_t  == "0" &&
			wanInfo.wan0_auxstate_t == "0"
		){
			retData.wanType = (iCanUsePPPoE && wanInfo.x_Setting  == "0") ? wanTypeList.pppdhcp : wanTypeList.connected;
		}
		else if( (wanInfo.wan0_state_t    == "2" && wanInfo.wan0_sbstate_t  == "0" && wanInfo.wan0_auxstate_t == "2") ||
				 (wanInfo.wan0_state_t    == "2" && wanInfo.wan0_sbstate_t  == "0" && wanInfo.wan0_auxstate_t == "0")
		){
				retData.wanType = wanTypeList.dhcp;
		}
		else if(wanInfo.autodet_state == ""){
			retData.wanType = wanTypeList.check;
			if(this.detRetryCnt > 0){
				this.detRetryCnt --;
			}
			else{
				this.startAutoDet();
				retData.isIPConflict = false;
				retData.isError = false;
				this.detRetryCnt = this.detRetryCnt_MAX;
			}
		}
		else if(iCanUsePPPoE){
			retData.wanType = wanTypeList.pppoe;
		}
		else if(hadPlugged("modem")){
			retData.wanType = wanTypeList.modem;
		}
		else if(wanInfo.autodet_state == "3" || wanInfo.autodet_state == "5"){
			retData.wanType = wanTypeList.resetModem;
		}
		else if(wanInfo.autodet_state == "4"){
			if(wanInfo.wan0_auxstate_t != "1"){
				this.startAutoDet();
				retData.wanType = wanTypeList.check;
				retData.isIPConflict = false;
				retData.isError = false;
				this.detRetryCnt = this.detRetryCnt_MAX;
			}
			else{
				retData.wanType = wanTypeList.noWan;
			}
		}
		else{
			retData.wanType = wanTypeList.check;
			if(this.detRetryCnt > 0){
				this.detRetryCnt --;
			}
			else{
				this.startAutoDet();
				retData.isIPConflict = false;
				retData.isError = false;
				this.detRetryCnt = this.detRetryCnt_MAX;
			}
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

	"checkCap": function(targetOrigin, callback){
		window.chcap = callback;
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

	"faqURL": function(_faqNum, handler){
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

		var temp_URL_lang = "https://www.asus.com" + faqLang[pLang] + "/support/FAQ/" + _faqNum;
		if(handler) handler(temp_URL_lang.replace(faqLang[pLang], ""));

		$.ajax({
			url: temp_URL_lang,
			dataType: "jsonp",
			statusCode: {
				200: function(response) {
					if(handler) handler(temp_URL_lang);
				}
			}
		});
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
	},

	"enableEula": function(_eulaType, enable, callback){
		var eulaType = _eulaType.toUpperCase()

		$.ajax({
			url: '/set_' + eulaType + '_EULA.cgi?' + eulaType + '_EULA=' + enable,
			error: function(){},
			success: callback
		});
	},

	"unregisterAsusDDNS": function(callback){
		$.ajax({
			url: '/unreg_ASUSDDNS.cgi',
			error: function(){},
			success: callback
		});
	},

	"uiFlag": {
	/*
		the list defined as nvram order, the nvram value define the status.
		the value defined as status, you can use 0~9 to define any status that to used.
		ex. nvram uiFlag=011, defined as feature1/feature2/feature3..., value defined as disable/enable/enable
		"list": { "feature" : 0, "feature1" : 1, "feature2" : 2, ...},
	*/

		"list": {
			"AiMeshHint" : 0
		},

		"get": function(_name){
			var uiFlagValue = httpApi.nvramGet(["uiFlag"], true).uiFlag;
			return uiFlagValue.charAt(httpApi.uiFlag.list[_name]);
		},

		"set": function(_name, _value){
			var replaceValue = function(_oriString, _index, _replacement) {
				return _oriString.substr(0, _index) + _replacement + _oriString.substr(_index + 1);
			};
			var uiFlag_ori = httpApi.nvramGet(["uiFlag"], true).uiFlag;
			var uiFlag_update = replaceValue(uiFlag_ori, httpApi.uiFlag.list[_name], _value);
			httpApi.nvramSet({"action_mode": "apply", "uiFlag" : uiFlag_update});
		}
	},

	"update_wlanlog": function(){
		$.get("/update_wlanlog.cgi");
	},

	"boostKey_support": function(){
		var ch = eval('<% channel_list_5g(); %>');
		if(isSupport("triband"))
			ch += eval('<% channel_list_5g_2(); %>');
		if(ch.indexOf("52") != -1 || ch.indexOf("56") != -1 || ch.indexOf("60") != -1 || ch.indexOf("64") != -1 || ch.indexOf("100") != -1 || ch.indexOf("104") != -1 || ch.indexOf("108") != -1 || ch.indexOf("112") != -1 || ch.indexOf("116") != -1 || ch.indexOf("120") != -1 || ch.indexOf("124") != -1 || ch.indexOf("128") != -1 || ch.indexOf("132") != -1 || ch.indexOf("136") != -1 || ch.indexOf("140") != -1 || ch.indexOf("144") != -1){
			return {
				"GAME_BOOST": {
					"value": 3,
					"text": "Enable GameBoost",
					"desc": "Game Boost analyzes network traffic and prioritizes gaming packets, giving games a second level of acceleration for the best possible performance."
				},
				"ACS_DFS": {
					"value": 1,
					"text": "<#WLANConfig11b_EChannel_dfs#>",
					"desc": "Auto channel selection includes DFS band allows <#Web_Title2#> to utilize extra 5GHz channels for less interferences and greater bandwidth."
				},
				"LED": {
					"value": 0,
					"text": "LED On/Off",
					"desc": "The LED on/off control is used to turn off all LEDs includes Aura light."
				},
				"AURA_RGB": {
					"value": 2,
					"text": "Aura RGB",
					"desc": "Aura sync control is used to get Aura control from other ROG devices, if disabled, it will be customized Aura RGB."
				}
			}
		}else{
			return {
				"GAME_BOOST": {
					"value": 3,
					"text": "Enable GameBoost",
					"desc": "Game Boost analyzes network traffic and prioritizes gaming packets, giving games a second level of acceleration for the best possible performance."
				},
				"LED": {
					"value": 0,
					"text": "LED On/Off",
					"desc": "The LED on/off control is used to turn off all LEDs includes Aura light."
				},
				"AURA_RGB": {
					"value": 2,
					"text": "Aura RGB",
					"desc": "Aura sync control is used to get Aura control from other ROG devices, if disabled, it will be customized Aura RGB."
				}
			}
		}
	},

	"getPAPStatus": function(_band){
		var papStatus = "";
		var get_ssid = function(_band){
			var ssid = "";
			if(_band == undefined)
				ssid = decodeURIComponent(httpApi.nvramCharToAscii(["wlc_ssid"], true).wlc_ssid);
			else
				ssid = decodeURIComponent(httpApi.nvramCharToAscii(["wlc" + _band + "_ssid"], true)["wlc" + _band + "_ssid"]);

			ssid = ssid.replace(/\</g, "&lt;").replace(/\>/g, "&gt;");
			return ssid;
		};
		var dpsta_rep = (httpApi.nvramGet(["wlc_dpsta"]).wlc_dpsta == "") ? false : true;
		if(dpsta_rep){
			var wlc_state = "0";
			if(_band == undefined)
				wlc_state = httpApi.nvramGet(["wlc_state"]).wlc_state;
			else
				wlc_state = httpApi.nvramGet(["wlc" + _band + "_state"])["wlc" + _band + "_state"];
			switch(wlc_state){
				case "0":
					papStatus = "<#Disconnected#>";
					break;
				case "1":
					papStatus = "<#APSurvey_action_ConnectingStatus1#>";
					break;
				case "2":
					papStatus = get_ssid(_band);
					break;
				default:
					papStatus = "<#Disconnected#>";
					break;
			}
		}
		else{
			var wlc_psta_state = httpApi.hookGet("wlc_psta_state", true);
			if(wlc_psta_state.wlc_state == "1" && wlc_psta_state.wlc_state_auth == "0")
				papStatus = get_ssid(_band);
			else if(wlc_psta_state.wlc_state == "2" && wlc_psta_state.wlc_state_auth == "1")
				papStatus = "<#APSurvey_action_ConnectingStatus1#>";
			else
				papStatus = "<#Disconnected#>";

		}
		return papStatus;
	},

	"getISPProfile": function(isp){
		var isp_profiles = httpApi.hookGet("get_iptvSettings").isp_profiles;
		var specified_profile = [];

		$.each(isp_profiles, function(i, isp_profile) {
			if(isp_profile.switch_wantag == isp){
				specified_profile = isp_profile;
			}
		});

		return specified_profile;
	},

	"checkCloudModelIcon": function(modelName, callBackSuccess, callBackError){
		var getCloudModelIconSrc = function(modelName){
			var handle_cloud_icon_model_name = function(_modelName) {
				var transformName = _modelName;
				if(transformName == "RT-AC66U_B1" || transformName == "RT-AC1750_B1" || transformName == "RT-N66U_C1" || transformName == "RT-AC1900U" || transformName == "RT-AC67U")
					transformName = "RT-AC66U_V2";
				else if(transformName == "BLUE_CAVE")
					transformName = "BLUECAVE";
				else if(transformName == "Lyra")
					transformName = "MAP-AC2200";
				else if(transformName == "Lyra_Mini" || transformName == "LyraMini")
					transformName = "MAP-AC1300";
				else if(transformName == "Lyra_Trio")
					transformName = "MAP-AC1750";
				else if(transformName == "LYRA_VOICE")
					transformName = "MAP-AC2200V";
				return transformName;
			};
			var server = "http://nw-dlcdnet.asus.com";
			var fileName = "/plugin/productIcons/" + handle_cloud_icon_model_name(modelName) + ".png";

			return server + fileName;
		};

		$("<img>")
			.attr('src', getCloudModelIconSrc(modelName))
			.on("load", function(e){
				if(callBackSuccess) callBackSuccess($(this).attr("src"));
				$(this).remove();
			})
			.on("error", function(e){
				if(callBackError) callBackError();
				$(this).remove();
			});
	},

	"getAiMeshLabelMac": function(modelName, macAddr, callBackSuccess, callBackError){
		var _modelName = modelName.trim();
		var _macAddr = macAddr.trim().toUpperCase();
		if(_modelName == "" || _macAddr == ""){
			if(callBackError)
				callBackError();
			return;
		}

		if(modelName != "Lyra" && modelName != "Lyra_Mini" && modelName != "LyraMini" && modelName != "LYRA_VOICE" && modelName != "Lyra_Trio")
			return;

		var isMac = function(_mac){
			var hwaddr = new RegExp("(([a-fA-F0-9]{2}(\:|$)){6})", "gi");
			if(hwaddr.test(_mac))
				return true;
			else
				return false;
		};

		if(!isMac(_macAddr)){
			if(callBackError)
				callBackError();
			return;
		}

		var macToHex = function(_mac){
			return _mac.replace(/:/g, "").toString(16);
		};
		var hexToDec = function(_hex){
			return parseInt(_hex, 16);
		};
		var decToHex = function(_dec){
			if(_dec < 0)
				_dec = 0xFFFFFFFFFFFF + _dec + 1;
			return _dec.toString(16).toUpperCase();
		};
		var hexToMac = function(_hex){
			var add_char_before = function(_value, _length, _char) {
				_char = _char || "0";
				_value = _value + "";
				return _value.length >= _length ? _value : new Array(_length - _value.length + 1).join(_char) + _value;
			};
			if(_hex.length < 12)
				_hex = add_char_before(_hex, 12, 0);
			return _hex.match(/.{1,2}/g).join(":").toUpperCase();
		};
		var returnMacAddr = "";
		var offset = 0;
		switch(modelName){
			case "LYRA_VOICE":
			case "Lyra":
			case "Lyra_Mini":
			case "LyraMini":
				offset = -3;
				break;
			case "Lyra_Trio":
				offset = -1;
				break;
			default:
				offset = 0;
				break;
		}

		returnMacAddr = hexToMac(decToHex((hexToDec(macToHex(_macAddr)) + offset)));
		if(isMac(returnMacAddr)){
			if(callBackSuccess)
				callBackSuccess(returnMacAddr);
		}
		else{
			if(callBackError)
				callBackError();
		}
	},

	"updateClientList": function(){
		$.post("/applyapp.cgi?action_mode=update_client_list");
	},

	"hasAiMeshNode": function(){
		var status = false;
		if(amesh_support && (isSwMode("rt") || isSwMode("ap"))) {
			var get_cfg_clientlist = httpApi.hookGet("get_cfg_clientlist", true);
			if(get_cfg_clientlist != undefined && get_cfg_clientlist.length > 1) {
				get_cfg_clientlist.shift();//filter CAP
				var online_node_list = get_cfg_clientlist.filter(function(item) { return item.online == "1"; });
				if(online_node_list.length > 0)
					status = true;
			}
		}
		return status;
	},

	"amazon_wss": {
		"if_support": function(_wl_unit, _wl_subunit){
			var support_if = ["0.2"];
			return (support_if.indexOf("" + _wl_unit + "." + _wl_subunit + "") != -1) ? true : false;
		},

		"getStatue": function(_wl_unit, _wl_subunit){
			var value = "0";
			if(httpApi.amazon_wss.if_support(_wl_unit, _wl_subunit))
				value = httpApi.nvramGet(["wl" + _wl_unit + "." + _wl_subunit + "_gn_wbl_enable"], true)["wl" + _wl_unit + "." + _wl_subunit + "_gn_wbl_enable"];
			if(value == "")
				value = "0";
			return value;
		},

		"getProfile": function(){
			return {
				"Hide SSID": "Yes",
				"Network Name SSID": "simple_setup",	
				"Oauth mode": "Open System",	
				"Bandwidth Setting": "D: 80 kbps, U: 80 kbps",	
				"Access Intranet": "Disable"
			}
		},

		"set": function(postData, parmData){
			var asyncDefault = true;
			var _wl_unit = "", _wl_subunit = "";
			if(parmData != undefined){
				if(parmData.async != undefined)
					asyncDefault = parmData.async;
				if(parmData.wl_unit != undefined && parmData.wl_subunit != undefined){
					_wl_unit = parmData.wl_unit;
					_wl_subunit = parmData.wl_subunit;
				}
			}

			if(httpApi.amazon_wss.if_support(_wl_unit, _wl_subunit)){
				$.ajax({
					url: '/amazon_wss.cgi',
					dataType: 'json',
					data: postData,
					async: asyncDefault,
					error: function(){},
					success: function(response){
						if(parmData.callBack) parmData.callBack.call(response);
					}
				});
			}
		}
	},

	"ftp_port_conflict_check" : {
		usb_ftp : {
			enabled : function(){
				if(noftp_support)
					return 0;
				else{
					var enable_ftp = httpApi.nvramGet(["enable_ftp"], true).enable_ftp;
					if(enable_ftp == "")
						return 0;
					else
						return parseInt(enable_ftp);
				}
			},
			hint : "<#IPConnection_VServer_usb_port_conflict#>\n<#IPConnection_VServer_go_VS_change_lan_port#>"
		},
		port_forwarding : {
			enabled : function(){
				if(isSwMode("rt")){
					var vts_enable_x = httpApi.nvramGet(["vts_enable_x"], true).vts_enable_x;
					if(vts_enable_x == "0")
						return 0;
					else{
						return httpApi.ftp_port_conflict_check.port_forwarding.use_usb_ftp_port();
					}
				}
				else
					return 0;
			},
			use_usb_ftp_port : function(){
				var state = 0;
				var lan_ipaddr = httpApi.nvramGet(["lan_ipaddr"], true).lan_ipaddr;
				var usb_ftp_port = 21;
				var vts_rulelist = decodeURIComponent(httpApi.nvramCharToAscii(["vts_rulelist"], true).vts_rulelist);
				var dual_wan_lb_status = (check_dual_wan_status().status == "1" && check_dual_wan_status().mode == "lb") ? true : false;
				var support_dual_wan_unit_flag = (mtwancfg_support && dual_wan_lb_status) ? true : false;
				if(support_dual_wan_unit_flag)
					vts_rulelist += decodeURIComponent(httpApi.nvramCharToAscii(["vts1_rulelist"], true).vts1_rulelist);

				var eachRulelist = decodeURIComponent(vts_rulelist).split('<');
				break_loop:
					for(var i = 0; i < eachRulelist.length; i += 1){
						if(eachRulelist[i] != "") {
							var eachRuleItem = eachRulelist[i].split('>');
							var externalPort = eachRuleItem[1];
							var internalIP = eachRuleItem[2];
							var eachPort = externalPort.split(",");
							for(var j = 0; j < eachPort.length; j += 1){
								if(eachPort[j].indexOf(":") != -1){//port range
									var portS = eachPort[j].split(":")[0];
									var portE = eachPort[j].split(":")[1];
									if(parseInt(portS) <= usb_ftp_port && parseInt(portE) >= usb_ftp_port && internalIP != lan_ipaddr){
										state = 1;
										break break_loop;
									}
								}
								else if(parseInt(eachPort[j]) == usb_ftp_port && internalIP != lan_ipaddr){
									state = 1;
									break break_loop;
								}
							}
						}
					}
				return state;
			},
			hint : "<#IPConnection_VServer_usb_port_conflict#>\n<#IPConnection_VServer_change_lan_port#>"
		},
		conflict : function(){
			return (httpApi.ftp_port_conflict_check.usb_ftp.enabled() && httpApi.ftp_port_conflict_check.port_forwarding.enabled()) ? true : false;
		}
	},

	"isItSafe_trend": function(queryStr){
		var $form = $("<form>", {
			action: "https://global.sitesafety.trendmicro.com/result.php",
			method: "post",
			target: "_blank"
		});

		$form
			.append($("<input>", {
				type: "hidden",
				name: "urlname",
				value: queryStr
			}))
			.append($("<input>", {
				type: "hidden",
				name: "getinfo",
				value: "Check Now"
			}))			
			.appendTo("body").submit().remove();
	},

	"set_ledg" : function(postData, parmData){
		var asyncDefault = true;
		$.ajax({
			url: '/set_ledg.cgi',
			dataType: 'json',
			data: postData,
			async: asyncDefault,
			error: function(){},
			success: function(response){
				if(parmData.callBack) parmData.callBack.call(response);
			}
		});
	}
}
