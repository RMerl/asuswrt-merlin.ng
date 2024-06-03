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
	"app_dataHandler": false,

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

	"nvramGetWanByUnit": function(unit, nvrams){
		if(!nvrams.every(function(nvram){return nvram.indexOf("wan_") !== -1}) || isNaN(unit)) return {};

		var reult = {};
		var nvramsByUnit = function(nvram){return nvram.replace("wan_", "wan" + unit + "_");}
		var wanInfo = httpApi.nvramGet(nvrams.map(nvramsByUnit), 1);
		$.each(wanInfo, function(item){reult[item.replace(unit, "")] = wanInfo[item];});

		return reult;
	},

	"nvramSet": function(postData, handler){
		delete postData.isError;

		if(this.app_dataHandler){
			if(typeof postMessageToApp == "function")
				postMessageToApp(postData);
		}
		else{
			$.ajax({
				url: '/applyapp.cgi',
				dataType: 'json',
				data: encodeURIComponent(JSON.stringify(postData)),
				type: 'POST',
				error: function(){},
				success: function(response){
					if(handler) handler.call(response);

					if(typeof postMessageToApp == "function"){
						if(postData.rc_service == undefined) postData.rc_service = "nvramSet";
						postMessageToApp({rc_service: postData.rc_service});
					}
				}
			})
		}
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

	"uploadFreeWiFi": function(postData, handler){
		delete postData.isError;

		var formData = new FormData();
		for(var key in postData){
			if(postData.hasOwnProperty(key)){
				formData.append(key, postData[key]);
			}
		}

		$.ajax({
			url: '/splash_page_SDN.cgi',
			dataType: 'text',
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

	"uploadOvpnFile": function(postData, handler){
		delete postData.isError;

		var formData = new FormData();
		for(var key in postData){
			if(postData.hasOwnProperty(key)){
				formData.append(key, postData[key]);
			}
		}

		$.ajax({
			url: '/vpnupload.cgi',
			dataType: 'text',
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

	"uploadServerOvpnCert": function(postData, handler){
		delete postData.isError;

		var formData = new FormData();
		for(var key in postData){
			if(postData.hasOwnProperty(key)){
				formData.append(key, postData[key]);
			}
		}

		$.ajax({
			url: '/upload_server_ovpn_cert.cgi',
			dataType: 'text',
			data: formData,
			contentType: false,
			processData: false,
			type: 'POST',
			error: function(response){
				if(handler) handler(response);
			},
			success: function(response){
				if(handler) handler(response);
			}
		 });
	},

	"uploadWGCFile": function(postData, handler){
		delete postData.isError;

		var formData = new FormData();
		for(var key in postData){
			if(postData.hasOwnProperty(key)){
				formData.append(key, postData[key]);
			}
		}

		$.ajax({
			url: '/upload_wgc_config.cgi',
			dataType: 'text',
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

	"hookGetMore": function(objItems, forceUpdate){
		var queryArray = [];
		var retData = {};

		var __hookNames = function(hookNames){
			return hookNames.map(function(hookName){
				return hookName.split("-")[0] + "(" + (hookName.split("-")[1] || "") + ")";
			}).join("%3B");
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
				url: '/appGet.cgi?hook=' + __hookNames(queryArray),
				dataType: 'json',
				async: false,
				error: function(){
					for(var i=0; i<queryArray.length; i++){retData[queryArray[i]] = "";}
					retData.isError = true;

					$.ajax({
						url: '/appGet.cgi?hook=' + __hookNames(queryArray),
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

	"startAutoDet": function(){
		$.get("/appGet.cgi?hook=start_force_autodet()");
	},

	"startDSLAutoDet": function(){
		$.get("/appGet.cgi?hook=start_dsl_autodet()");
	},

	"startWan46AutoDet": function(){
		$.get("/appGet.cgi?hook=restart_auto46det()");
	},

	"detwanGetRet": function(){
		var wanInfo = httpApi.nvramGet(["wan0_state_t", "wan0_sbstate_t", "wan0_auxstate_t", "autodet_state", "autodet_auxstate", "wan0_proto",
										 "link_internet", "x_Setting", "usb_modem_act_sim", "link_wan", "wans_dualwan", "wan1_state_t", "wan1_sbstate_t", "wan1_auxstate_t"], true);
		var tcode = httpApi.nvramGet(["territory_code"], true).territory_code;

		var sessionId = (typeof systemVariable != "undefined") ? systemVariable.qisSession : ""; 
		httpApi.log("httpApi.detwanGetRet", JSON.stringify(wanInfo), sessionId);

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
			"dhcpSpecialISP": "DHCPSPECIALISP", //DHCP-Only, link_internet == "2", and special ISP settings is needed
			"noWan": "NOWAN"
		}

		var simStateList = {
			"nosim": "NOSIM",
			"ready": "READY",
			"pin": "PIN",
			"puk": "PUK",
			"pin2": "PIN2",
			"puk2": "PUK2",
			"wait": "WAITING",
			"fail": "FAIL"
		}

		var retData = {
			"wanType": "CHECKING",
			"isIPConflict": (function(){
				return (wanInfo.wan0_state_t == "4" && wanInfo.wan0_sbstate_t == "4")
			})(),
			"simState": "WAITING",
			"isError": false
		};

		var hadPlugged = function(deviceType){
			var usbDeviceList = httpApi.hookGet("show_usb_path", true) || [];
			return (usbDeviceList.join().search(deviceType) != -1)
		}

		var iCanUsePPPoE = (wanInfo.autodet_state == "6" || wanInfo.autodet_auxstate == "6");
		var sim_state = parseInt(wanInfo.usb_modem_act_sim);
		var iptvSupport = (
					tcode.search("TW") == -1 && tcode.search("US") == -1 &&
					tcode.search("U2") == -1 && tcode.search("CA") &&
					tcode.search("CN") == -1 && tcode.search("CT") == -1 &&
					tcode.search("GD") == -1 && tcode.search("TC") == -1
				);

		if(isSupport("gobi") && (sim_state >= 1 && sim_state <= 6)){
			switch(wanInfo.usb_modem_act_sim){
				case "1":
					retData.simState = simStateList.ready;
					break;
				case "2":
					retData.simState = simStateList.pin;
					break;
				case "3":
					retData.simState = simStateList.puk;
					break;
				case "4":
					retData.simState = simStateList.pin2;
					break;
				case "5":
					retData.simState = simStateList.puk2;
					break;
				case "6":
					retData.simState = simStateList.wait;
					break;
			}
		}

		if(wanInfo.isError){
			retData.wanType = wanTypeList.check;
			retData.isIPConflict = false;
			retData.isError = true;
		}
		else if(wanInfo.link_wan == ""){
			retData.wanType = wanTypeList.check;
		}
		else if(wanInfo.link_wan == "0" && (isSupport("gobi") || !hadPlugged("modem"))){
			retData.wanType = wanTypeList.noWan;
		}
		else if(
			wanInfo.wan0_state_t    == "2" &&
			wanInfo.wan0_sbstate_t  == "0"
		){
			if(wanInfo.link_internet   == "2"){
				var choosePPPoE = (
						iCanUsePPPoE &&
						wanInfo.x_Setting == "0" &&
						(tcode.search("JP") != -1 || tcode.search("TW") != -1)
				)

				var specialISP = (!iCanUsePPPoE && wanInfo.x_Setting == "0" && iptvSupport);

				retData.wanType = choosePPPoE ? wanTypeList.pppoe : (specialISP ? wanTypeList.dhcpSpecialISP : wanTypeList.connected);
			}
			else{
				retData.wanType = (iCanUsePPPoE) ? wanTypeList.pppoe : wanTypeList.dhcp;
			}
		}
		else if(isSupport("usb_bk") && wanInfo.wans_dualwan == "wan usb" &&
				wanInfo.link_internet == "2" && wanInfo.wan1_state_t == "2" && wanInfo.wan1_sbstate_t  == "0"){//USB tethering
			retData.wanType = wanTypeList.connected;
		}
		else if(iCanUsePPPoE){
			retData.wanType = wanTypeList.pppoe;
		}
		else if(!isSupport("gobi") && hadPlugged("modem")){
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
		else if(wanInfo.wan0_state_t == "4" && wanInfo.wan0_sbstate_t == "4"){
			retData.wanType = iptvSupport ? wanTypeList.dhcpSpecialISP : wanTypeList.connected;
			retData.isIPConflict = true;
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

	"detDSLwanGetRet": function(){
		var wanInfo = httpApi.nvramGet(["wan0_state_t", "wan0_sbstate_t", "wan0_auxstate_t", 
										//dsl_autodet_state     dsl_line_state        wan_type                   dslx_annex_state
										"dsltmp_autodet_state", "dsltmp_adslsyncsts", "dsltmp_autodet_wan_type", "dslx_annex",
										"link_internet", "x_Setting", "usb_modem_act_sim", "link_wan"], true);

		var wanTypeList = {
			"check": "CHECKING",
			"dhcp": "DHCP",
			"ppp": "PPP",
			"ptm": "PTM_Manual",
			"atm": "Manual",
			"modem": "MODEM",
			"resetModem": "RESETMODEM",
			"connected": "CONNECTED",
			"noWan": "NOWAN"
		}

		var retData = {
			"wanType": "CHECKING",
			"dsl_line_state": wanInfo.dsltmp_autodet_state,
			"isIPConflict": (function(){
				return (wanInfo.wan0_state_t == "4" && wanInfo.wan0_sbstate_t == "4")
			})(),
			"simState": "WAITING",
			"isError": false
		};

		if(wanInfo.isError){
			retData.wanType = wanTypeList.check;
			retData.isIPConflict = false;
			retData.isError = true;
		}
		else if(wanInfo.link_wan == ""){
			retData.wanType = wanTypeList.check;
		}
		else if(wanInfo.dsltmp_adslsyncsts == "up"){

			if(wanInfo.dsltmp_autodet_wan_type == "PTM"){
				retData.wanType = wanTypeList.ptm;
			}
			else if(wanInfo.dsltmp_autodet_wan_type == "ATM" && (wanInfo.dslx_annex == "5" || wanInfo.dslx_annex == "6")){
				retData.wanType = wanTypeList.atm;
			}
			else if(wanInfo.dsltmp_autodet_state == "pppoe" || wanInfo.dsltmp_autodet_state == "pppoa"){
				retData.wanType = wanTypeList.ppp;
			}
			else if(wanInfo.dsltmp_autodet_state == "dhcp"){
				retData.wanType = wanTypeList.dhcp;
			}
			else if(wanInfo.dsltmp_autodet_state == "Fail"){
				if(wanInfo.dsltmp_autodet_wan_type == "PTM"){
					retData.wanType = wanTypeList.ptm;
				}
				else{
					retData.wanType = wanTypeList.atm;
				}
			}
			else {
				retData.wanType = wanTypeList.check;	//annex re-detect... or timeout
			}

		}
		else if(wanInfo.dsltmp_adslsyncsts == "init" || wanInfo.dsltmp_adslsyncsts == "initializing" || wanInfo.dsltmp_adslsyncsts == "wait"
			|| wanInfo.dsltmp_adslsyncsts == "Detecting" || wanInfo.dsltmp_adslsyncsts == "detecting" || wanInfo.dsltmp_adslsyncsts == "wait for init"
			|| wanInfo.dsltmp_adslsyncsts == "")
		{
			set_state_info(wanInfo.dsltmp_adslsyncsts);
			retData.wanType = wanTypeList.check;
		}
		else if(wanInfo.link_wan == "0")
		{
			retData.wanType = wanTypeList.noWan;
		}
		else{
			set_state_info(wanInfo.dsltmp_adslsyncsts);
			retData.wanType = wanTypeList.check;
		}


		return retData;
	},

	"detwan46GetRet": function(){
		var wanInfo = httpApi.nvramGet(["wan46det_state","link_internet","x_Setting","link_wan"], true);

		var wanTypeList = {
			"init":"INITIALIZING",
			"nolink":"NOLINK", 
			"unknow":"UNKNOW",
			"v6plus":"V6PLUS",
			"hgw_v6plus":"HGW_V6PLUS",
			"ocnvc":"OCNVC",
			"dslite_xpass":"DSLITE_XPASS",
			"dslite_transix":"DSLITE_TRANSIX"
		}

		var retData = {
			"wan46State": "",
			"isError": false
		};

		if(wanInfo.isError){
			retData.wan46State = wanTypeList.init;
			retData.isError = true;
		}
		else if(wanInfo.link_wan == "0"){
			retData.wan46State = wanTypeList.nolink;
		}
		else if(wanInfo.wan46det_state == "0"){
			retData.wan46State = wanTypeList.init;
		}
		else if(wanInfo.wan46det_state == "1"){
			retData.wan46State = wanTypeList.nolink;
		}
		else if(wanInfo.wan46det_state == "2"){
			retData.wan46State = wanTypeList.unknow;
		}
		else if(wanInfo.wan46det_state == "3"){
			retData.wan46State = wanTypeList.v6plus;
		}
		else if(wanInfo.wan46det_state == "4"){
			retData.wan46State = wanTypeList.hgw_v6plus;
		}
		else if(wanInfo.wan46det_state == "5"){
			retData.wan46State = wanTypeList.ocnvc;
		}
		else if(wanInfo.wan46det_state == "6"){
			retData.wan46State = wanTypeList.dslite_xpass;
		}
		else if(wanInfo.wan46det_state == "7"){
			retData.wan46State = wanTypeList.dslite_transix;
		}

		return retData;
	},

	"getWanInfo": function(_index){
		var connect_proto_array = {
			"dhcp": "<#BOP_ctype_title1#>",
			"static": "<#BOP_ctype_title5#>",
			"pppoe": "PPPoE",
			"pptp": "PPTP",
			"l2tp": "L2TP",
			"pppoa": "PPPoA",
			"ipoa": "IPoA",
			"lw4o6": "LW 4over6",
			"map-e": "MAP-E",
			"v6plus": "<#IPv6_plus#>",
			"ocnvc": "<#IPv6_ocnvc#>",
			"dslite": "DS-Lite",
			"usb modem": "USB Modem"
		};
		var result = {
			"status": "",
			"status_text": "",
			"ipaddr": "",
			"proto": "",
			"proto_text": ""
		};
		var wans_info = httpApi.nvramGet(["wans_dualwan", "wans_mode"], true);
		var dualwan_enabled = (isSupport("dualwan") && wans_info.wans_dualwan.search("none") == -1) ? 1 : 0;
		var active_wan_unit = httpApi.hookGet("get_wan_unit", true);
		var wan_index = (_index == undefined) ? 0 : _index;
		if(dualwan_enabled){
			if(active_wan_unit != wan_index && (wans_info.wans_mode == "fo" || wans_info.wans_mode == "fb")){
				result.status = "standby";
				result.status_text = "<#Standby_str_cold#>";

			}
			else{//lb
				result.status = (httpApi.isConnected(wan_index)) ? "connected" : "disconnected";
				result.status_text = (result.status == "connected") ? "<#Connected#>" : "<#Disconnected#>";
			}
		}
		else{
			result.status = (httpApi.isConnected(wan_index)) ? "connected" : "disconnected";
			result.status_text = (result.status == "connected") ? "<#Connected#>" : "<#Disconnected#>";
		}
		if(result.status == "connected"){
			var wanInfo = httpApi.nvramGet(["wan" + wan_index + "_ipaddr", "wan" + wan_index + "_proto"], true);
			result.ipaddr = wanInfo["wan" + wan_index + "_ipaddr"];
			result.proto = wanInfo["wan" + wan_index + "_proto"];
			if(isSupport("usbX") && wans_info.wans_dualwan.split(" ")[wan_index] == "usb"){
				result.proto = "USB Modem";
			}
			if(isSupport("dsl")){
				if(wans_info.wans_dualwan.split(" ")[wan_index] == "dsl"){
					var dslInfo = httpApi.nvramGet(["dsl0_proto", "dslx_transmode"], true);
					if(dslInfo.dslx_transmode == "atm") {
						if(dslInfo.dsl0_proto == "pppoa" || dslInfo.dsl0_proto == "ipoa")
							result.proto = dslInfo.dsl0_proto;
					}
				}
			}
			if(result.proto != ""){
				var proto_text = connect_proto_array[(result.proto).toLowerCase()];
				result.proto_text = ((proto_text != undefined) ? proto_text : result.proto);
				if(isSupport("gobi") && result.proto == "USB Modem"){
					var modem_operation = httpApi.nvramGet(["usb_modem_act_operation"], true).usb_modem_act_operation;
					result.proto_text = ((modem_operation != "") ? modem_operation : "<#Mobile_title#>");
				}
			}
		}
		return result;
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

	"isConnected": function(_index){
		var sw_mode = httpApi.nvramGet(["sw_mode"], true).sw_mode;
		var wan_index = (_index == undefined) ? 0 : _index;
		var wanInfo = httpApi.nvramGet(["wan" + wan_index + "_state_t", "wan" + wan_index + "_sbstate_t", "wan" + wan_index + "_auxstate_t", "link_internet"], true);
		return (sw_mode != "1")? (wanInfo.link_internet == "2"):(
			wanInfo.link_internet == "2" &&
			wanInfo["wan" + wan_index + "_state_t"] == "2" &&
			wanInfo["wan" + wan_index + "_sbstate_t"] == "0" &&
			wanInfo["wan" + wan_index + "_auxstate_t"] == "0"
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
			var confirm_flag = confirm("<#JS_cleanLog#>");
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

	"privateEula": {
		"set": function(enable, callback){
			window.localStorage.setItem("PP", enable)
			window.localStorage.setItem("PP_time", Date.now())

			$.ajax({
				url: '/set_ASUS_privacy_policy.cgi',
				data: {
					"ASUS_privacy_policy": enable
				},
				dataType: 'json',
				success: callback
			});
		},

		"get": function(feature){
			if(feature == undefined || feature == "") feature = "ASUS_privacy_policy";

			var retData = {
				ASUS_PP_EULA: window.localStorage["PP"],
				ASUS_PP_EULA_time: window.localStorage["PP_time"]
			};

			$.ajax({
				url: '/get_ASUS_privacy_policy.cgi',
				dataType: 'json',
				async: false,
				success: function(resp){
					var ASUS_privacy_policy = resp.ASUS_privacy_policy;
					var ASUS_privacy_policy_time = resp.ASUS_privacy_policy_time;

					if(feature == "SIGNED"){
						var securityUpdate = httpApi.securityUpdate.get()
						var audoUpgrade = httpApi.nvramGet(["webs_update_enable"]).webs_update_enable == "1";

						if(ASUS_privacy_policy == "0" && ASUS_privacy_policy_time != ""){
							retData.ASUS_PP_EULA = "1";
							retData.ASUS_PP_EULA_time = "";
						}
						else if(
							ASUS_privacy_policy_time == "" ||
							ASUS_privacy_policy_time == undefined
						){
							retData.ASUS_PP_EULA = "0";
							retData.ASUS_PP_EULA_time = "";
						}
						else if(
							(ASUS_privacy_policy > "0" && resp.AHS > ASUS_privacy_policy && securityUpdate) ||
							(ASUS_privacy_policy > "0" && resp.ASD > ASUS_privacy_policy && securityUpdate) ||
							(ASUS_privacy_policy > "0" && resp.AUTOUPGRADE > ASUS_privacy_policy && audoUpgrade)
						){
							retData.ASUS_PP_EULA = "0";
							retData.ASUS_PP_EULA_time = ASUS_privacy_policy_time;
						}
						else{
							retData.ASUS_PP_EULA = "1";
							retData.ASUS_PP_EULA_time = ASUS_privacy_policy_time;
						}
					}
					else{
						retData.ASUS_PP_EULA = ((resp[feature] > ASUS_privacy_policy) || resp[feature] == 0 || resp[feature] == "") ? 0 : 1;
						retData.ASUS_PP_EULA_time = ASUS_privacy_policy_time;
					}
				}
			});

			return retData;
		}
	},

	"securityUpdate": {
		"set": function(enable, callback){
			$.ajax({
				url: '/set_security_update.cgi?' + 'security_update=' + enable,
				success: callback
			});
		},

		"get": function(){
			var retData;
			$.ajax({
				url: '/get_security_update.cgi',
				dataType: 'json',
				async: false,
				success: function(resp){
					retData = resp.security_update;
				}
			});

			return retData;
		}
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
		var retData = {
				"boost_led": {
					"title": "<#BoostKey_LED#>",
					"value": 0,
					"text": `<#BoostKey_LED#>`,
					"desc": `<#BoostKey_LED_desc#>`
				},
				"boost_dfs": {
					"title": "<#BoostKey_DFS#>",
					"value": 1,
					"text": `<#WLANConfig11b_EChannel_dfs#>`,
					"desc": `<#BoostKey_DFS_desc#>`
				},
				"boost_aura": {
					"title": "<#BoostKey_Aura_RGB#>",
					"value": 2,
					"text": `<#BoostKey_Aura_RGB#>`,
					"desc": `<#BoostKey_Aura_RGB_desc#>`
				},
				"boost_qos": {
					"title": "<#BoostKey_Boost#>",
					"value": 3,
					"text": `<#BoostKey_enable#>`,
					"desc": `<#BoostKey_Boost_desc#>`
				}
		};

		var productid = httpApi.nvramGet(["productid"]).productid;
		if(productid == "GT-AC2900"){
			delete retData.LED;
			delete retData.AURA_RGB;

			retData.boost_shuffle = {
				"title": "<#BoostKey_AURA_Shuffle#>",
				"value": 4,
				"text": "<#BoostKey_AURA_Shuffle#>",
				"desc": "<#BoostKey_AURA_Shuffle_desc#>"				
			}

			retData.boost_geforce = {
				"title": "<#BoostKey_GeForce#>",
				"value": 5,
				"text": "<#BoostKey_GeForce#>",
				"desc": "<#BoostKey_GeForce_desc#>"				
			}
		}

		var sw_mode = (window.hasOwnProperty("qisPostData") && qisPostData.hasOwnProperty("sw_mode")) ? qisPostData.sw_mode : httpApi.nvramGet(["sw_mode"]).sw_mode;
		if(sw_mode != "1"){
			delete retData.GAME_BOOST;
		}

		if(sw_mode == "1" || sw_mode == "3"){
			var ch_5g1 = httpApi.hookGet("channel_list_5g");
			var ch_5g2 = httpApi.hookGet("channel_list_5g_2");
			var ch = ch_5g1.concat(ch_5g2).toString().split(",");

			if((ch.indexOf("52") == -1 && ch.indexOf("56") == -1 && ch.indexOf("60") == -1 && ch.indexOf("64") == -1 && ch.indexOf("100") == -1 && ch.indexOf("104") == -1 && ch.indexOf("108") == -1 && ch.indexOf("112") == -1 && ch.indexOf("116") == -1 && ch.indexOf("120") == -1 && ch.indexOf("124") == -1 && ch.indexOf("128") == -1 && ch.indexOf("132") == -1 && ch.indexOf("136") == -1 && ch.indexOf("140") == -1 && ch.indexOf("144") == -1)){
				delete retData.ACS_DFS;
			}
		}
		else{
			delete retData.ACS_DFS;
		}

		return retData;
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
		if(isSupport("proxysta") && !dpsta_rep){
			var wlc_psta_state = httpApi.hookGet("wlc_psta_state", true);
			if(wlc_psta_state.wlc_state == "1" && wlc_psta_state.wlc_state_auth == "0")
				papStatus = get_ssid(_band);
			else if(wlc_psta_state.wlc_state == "2" && wlc_psta_state.wlc_state_auth == "1")
				papStatus = "<#APSurvey_action_ConnectingStatus1#>";
			else
				papStatus = "<#Disconnected#>";
		}
		else{
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

	"transformCloudModelName": function(model_info){
		var modelName = (model_info.model_name != undefined) ? model_info.model_name : "";
		var tcode = (model_info.tcode != undefined) ? model_info.tcode : "";
		var cobrand = (model_info.cobrand != undefined) ? model_info.cobrand : "";
		var icon_model_name = (model_info.icon_model_name != undefined) ? model_info.icon_model_name : "";

		var transformName = modelName;
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

		if(icon_model_name != undefined && icon_model_name != ""){
			transformName = icon_model_name;
		}
		else if(cobrand != undefined && cobrand != ""){
			transformName = transformName + "_CB_" + cobrand;
		}
		else if(tcode != undefined && tcode != ""){
			if(transformName == "RT-AX86U" && tcode == "GD/01")
				transformName = "RT-AX86U_GD01";
			else if((transformName == "RT-AX82U" || transformName == "RT-AX82U_V2") && tcode == "GD/01")
				transformName = "RT-AX82U_GD01";
		}
		return transformName;
	},

	"checkCloudModelIcon": function(model_info, callBackSuccess, callBackError){
		var server = "https://nw-dlcdnet.asus.com";
		var cloudModelName = "";
		if(model_info.cloudModelName != undefined && model_info.cloudModelName != "")
			cloudModelName = model_info.cloudModelName;
		else
			cloudModelName = httpApi.transformCloudModelName(model_info);

		var fileName = "/plugin/productIcons/" + cloudModelName + ".png";
		var img_src = server + fileName;
		if(cloudModelName != ""){
			$("<img>")
				.attr('src', img_src)
				.on("load", function(e){
					if(callBackSuccess) callBackSuccess($(this).attr("src"));
					$(this).remove();
				})
				.on("error", function(e){
					if(callBackError) callBackError();
					$(this).remove();
				});
		}
	},

	"getAiMeshLabelMac": function(modelName, macAddr, callBackSuccess, callBackError){
		var _modelName = modelName.trim();
		var _macAddr = macAddr.trim().toUpperCase();
		if(_modelName == "" || _macAddr == ""){
			if(callBackError)
				callBackError();
			return;
		}

		if(modelName != "Lyra" && modelName != "Lyra_Mini" && modelName != "LyraMini" && modelName != "LYRA_VOICE" && modelName != "Lyra_Trio" && modelName != "GT-AXY16000" && modelName != "RT-AX89X" && modelName != "SH-AC1300")
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
			case "Lyra":
				{
					var last_mac = _macAddr.substr(-1);
				        if (last_mac == "9") offset = -9;
				        else if (last_mac == "5" || last_mac == "D") offset = -5;
				        else offset = -3;
				}
				break;
			case "LYRA_VOICE":
			case "Lyra_Mini":
			case "LyraMini":
			case "SH-AC1300":
				offset = -3;
				break;
			case "Lyra_Trio":
				offset = -1;
				break;
			case "GT-AXY16000":
			case "RT-AX89X":
				offset = -5;
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
		if(amesh_support && (isSwMode("rt") || isSwMode("ap")) && ameshRouter_support) {
			var get_cfg_clientlist = httpApi.hookGet("get_cfg_clientlist");
			if(get_cfg_clientlist != undefined && get_cfg_clientlist.length > 1) {
				var cfg_clientlist_tmp = JSON.parse(JSON.stringify(get_cfg_clientlist));
				cfg_clientlist_tmp.shift();//filter CAP
				var online_node_list = cfg_clientlist_tmp.filter(function(item) { return item.online == "1"; });
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
				if(parmData != undefined && parmData.callBack) parmData.callBack.call(response);
			}
		});
	},
	"get_port_status": function(mac, callBack){
		var set_cap_support = function(_port_info){
			$.each(_port_info, function(index, data){
				var cap = data.cap;
				if(data["cap_support"] == undefined)
					data["cap_support"] = {};
				$.each(capability_map, function(index, capability_item){
					data["cap_support"][capability_item.type] = ((parseInt(cap) & (1 << parseInt(capability_item.bit))) > 0) ? true : false;
				});
			});
		};
		var capability_map = [
				{type:"WAN", bit:0},
				{type:"LAN", bit:1},
				{type:"GAME", bit:2},
				{type:"PLC", bit:3},
				{type:"WAN2", bit:4},
				{type:"WAN3", bit:5},
				{type:"SFPP", bit:6},
				{type:"USB", bit:7},
				{type:"MOBILE", bit:8},
				{type:"WANLAN", bit:9},
				{type:"MOCA", bit:10},
				{type:"POE", bit:11},
				{type:"WANAUTO", bit:12},
				{type:"IPTV_BRIDGE", bit:26},
				{type:"IPTV_VOIP", bit:27},
				{type:"IPTV_STB", bit:28},
				{type:"DUALWAN_SECONDARY_WAN", bit:29},
				{type:"DUALWAN_PRIMARY_WAN", bit:30}
			];

		$.ajax({
			url: "/get_port_status.cgi?node_mac=" + mac,
			dataType: 'json',
			async: true,
			error: function(){},
			success: function(response){
				if(response["port_info"] != undefined){
					if(mac == "all"){
						$.each(response["port_info"], function(node_mac, node_port_info){
							set_cap_support(response["port_info"][node_mac]);
						});
					}
					else{
						if(response["port_info"][mac] != undefined){
							set_cap_support(response["port_info"][mac]);
						}
					}
				}
				if(callBack){
					callBack(response);
				}
			}
		});
	},
	"get_port_status_array": function(mac, callBack){
		var rate_map = [
			{value:"10",text:"10 Mbps"},
			{value:"100",text:"100 Mbps"},
			{value:"1000",text:"1 Gbps"},
			{value:"2500",text:"2.5 Gbps"},
			{value:"5000",text:"5 Gbps"},
			{value:"10000",text:"10 Gbps"}
		];
		var rate_map_USB = [
			{value:"480",text:"USB2.0"},
			{value:"5000",text:"USB3.0"},
			{value:"10000",text:"USB3.2"}
		];
		httpApi.get_port_status(mac, function(response){
			var response_temp = JSON.parse(JSON.stringify(response));
			var port_info_temp = {};
			if(response_temp["port_info"] != undefined){
				if(response_temp["port_info"][mac] != undefined){
					port_info_temp = {"WAN":[], "LAN":[]};
					var port_info = response_temp["port_info"][mac];
					$.each(port_info, function(index, data){
						var label = index.substr(0,1);
						var label_idx = index.substr(1,1);
						data["label"] = label;
						data["label_priority"] = ((label == "W") ? 1 : ((label == "L") ? 2 : 3));
						data["label_idx"] = label_idx;
						data["label_port_name"] = (function(){
							if(data.cap_support.WAN || data.cap_support.WANAUTO){
								if(label_idx == "0")
									return "WAN";
								else
									return "WAN " + label_idx;
							}
							else if(data.cap_support.LAN){
								return "LAN " + label_idx;
							}
							else if(data.cap_support.USB){
								return "USB";
							}
							else if(data.cap_support.MOCA){
								return "MoCa";
							}
						})();


						var link_rate = isNaN(parseInt(data.link_rate)) ? 0 : parseInt(data.link_rate);
						var max_rate = isNaN(parseInt(data.max_rate)) ? 0 : parseInt(data.max_rate);
						data["link_rate_text"] = (data.is_on == "1") ? "0 Mbps" : "";
						var link_rate_data = rate_map.filter(function(item, index, array){
							return (item.value == link_rate);
						})[0];
						if(link_rate_data != undefined){
							data["link_rate_text"] = link_rate_data.text;
						}
						if(data["label"] == "C"){
							var _rate = link_rate
							if(isNaN(_rate)) _rate = 0;
							else if(_rate < 0) _rate = 0;

							if(_rate >= 1000){
								_rate = Math.round((_rate/1000)*10)/10;
								_rate += " Gbps";
							}
							else{
								_rate = Math.round(_rate*10)/10;
								_rate += " Mbps";
							}
							data["link_rate_text"] = _rate;
						}

						if(data.cap_support.USB){
							data["link_rate_text"] = ((data.is_on == "1") ? (link_rate + " Mbps") : "");
							var max_rate_data = rate_map_USB.filter(function(item, index, array){
								return (item.value == max_rate);
							})[0];
						}
						else{
							var max_rate_data = rate_map.filter(function(item, index, array){
								return (item.value == max_rate);
							})[0];
						}

						data["max_rate_text"] = "0 Mbps";
						if(max_rate_data != undefined){
							data["max_rate_text"] = max_rate_data.text;
							data["special_port_name"] = "";
							if(data["cap_support"]["GAME"] == true){
								data["special_port_name"] = "<#Port_Gaming#>";
							}
							else{
								if(data.cap_support.USB){
									data["special_port_name"] = (data.is_on == "1") ? "USB Modem" : max_rate_data.text;
								}
								else{
									const max_rate_value = parseInt(max_rate_data.value);
									if(max_rate_value > 1000){
										data["special_port_name"] = max_rate_data.text.replace(" Gbps", "");
										if(max_rate_value == 10000){
											if(data["cap_support"]["SFPP"] == true)
												data["special_port_name"] = data["special_port_name"] + "G SFP+";
											else
												data["special_port_name"] = data["special_port_name"] + "G baseT";
										}
										else
											data["special_port_name"] = data["special_port_name"] + "G";
									}
								}
							}
						}

						data["link_rate_status"] = 1;//normal
						if(!(data.cap_support.USB)){
							if(data.is_on == "1" && link_rate < 1000)
								data["link_rate_status"] = 0;//abnormal
						}

						var sort_key = "";
						if(data.cap_support.DUALWAN_PRIMARY_WAN || data.cap_support.DUALWAN_SECONDARY_WAN){
							port_info_temp["WAN"].push(data);
							sort_key = "WAN";
						}
						else{
							port_info_temp["LAN"].push(data);
							sort_key = "LAN";
						}

						port_info_temp[sort_key].sort(function(a, b){
							//first compare label priority, W>L>U
							var a_label_priority = parseInt(a.label_priority);
							var b_label_priority = parseInt(b.label_priority);
							var label_priority = ((a_label_priority == b_label_priority) ? 0 : ((a_label_priority > b_label_priority) ? 1 : -1));
							if(label_priority != 0){
								return label_priority;
							}
							else {//second compare label idx
								var a_label_idx = parseInt(a.label_idx);
								var b_label_idx = parseInt(b.label_idx);
								return ((a_label_idx == b_label_idx) ? 0 : ((a_label_idx > b_label_idx) ? 1 : -1));
							}
						});
					});
				}
			}
			if(callBack)
				callBack(port_info_temp);
		});
	},

	"set_antled" : function(postData, parmData){
		var asyncDefault = true;
		$.ajax({
			url: '/set_antled.cgi',
			dataType: 'json',
			data: postData,
			async: asyncDefault,
			error: function(){},
			success: function(response){
				if(parmData != undefined && parmData.callBack) parmData.callBack.call(response);
			}
		});
	},

	"get_wl_sched": function(wl_unit, callBack){
		var _wl_unit = "all";
		if(wl_unit != undefined && wl_unit.toString() != "")
			_wl_unit = wl_unit;

		$.ajax({
			url: "/get_wl_sched.cgi?unit=" + _wl_unit,
			dataType: 'json',
			async: true,
			error: function(){},
			success: function(response){
				if(callBack)
					callBack(response);
			}
		});
	},
	"set_wl_sched": function(postData){
		$.ajax({
			url: "/set_wl_sched.cgi",
			type: "POST",
			dataType: 'json',
			data: JSON.stringify(postData),
			async: true,
			error: function(){},
			success: function(response){}
		});
	},
	"aimesh_get_node_capability" : function(_node_info){
		if(_node_info == undefined) _node_info = {};

		var node_capability_list = {
			"led_control" : {
				"value" : 1,
				"def" : {
					"central_led" : {"bit" : 0},
					"lp55xx_led" : {"bit" : 1},
					"led_on_off" : {"bit" : 2},
					"led_brightness" : {"bit" : 3},
					"led_aura" : {"bit" : 4}
				}
			},
			"reboot_ctl" : {
				"value" : 2,
				"def" : {
					"manual_reboot" : {"bit" : 0}
				}
			},
			"force_topology_ctl" : {
				"value" : 3,
				"def" : {
					"preferable_backhaul" : {"bit" : 0},
					"prefer_node_apply" : {"bit" : 1}
				}
			},
			// rc_support is repored by cm_addRcSupport()
			"rc_support" : {
				"value" : 4,
				"def" : {
					"usb" : {"bit" : 0},
					"guest_network" : {"bit" : 1},
					"wpa3" : {"bit" : 2},
					"vif_onboarding" : {"bit" : 3},
					"sched_v2" : {"bit" : 4},
					"wifi_radio" : {"bit" : 5},
					"switchctrl" : {"bit" : 8},
					"port_status" : {"bit" : 9},
					"local_access" : {"bit" : 10},
					"wpa3_enterprise" : {"bit" : 16},
					"mlo_bh" : {"bit" : 20},
					"mlo_fh" : {"bit" : 21},
					"smart_home_master_ui" : {"bit" : 22}
				}
			},
			"link_aggregation" : {
				"value" : 5,
				"def" : {
					"lacp" : {"bit" : 0},
				}
			},
			"GN_2G_NO" : {//Number of supported RE 2G guest network
				"value" : 6,
				"def" : {
					"GN_2G_1" : {"bit" : 0},
					"GN_2G_2" : {"bit" : 1},
					"GN_2G_3" : {"bit" : 2}
				}
			},
			"GN_5G_NO" : {//Number of supported RE 5G guest network
				"value" : 7,
				"def" : {
					"GN_5G_1" : {"bit" : 0},
					"GN_5G_2" : {"bit" : 1},
					"GN_5G_3" : {"bit" : 2}
				}
			},
			"GN_5GH_NO" : {//Number of supported RE 5GH guest network
				"value" : 8,
				"def" : {
					"GN_5GH_1" : {"bit" : 0},
					"GN_5GH_2" : {"bit" : 1},
					"GN_5GH_3" : {"bit" : 2}
				}
			},
			"wans_cap" : {
				"value" : 15,
				"def" : {
					"wans_cap_wan" : {"bit" : 0}//Support ethernet wan or not
				}
			},
			"re_reconnect" : {
				"value" : 16,
				"def" : {
					"manual_reconn" : {"bit" : 0}
				}
			},
			"force_roaming" : {
				"value" : 17,
				"def" : {
					"manual_force_roaming" : {"bit" : 0}
				}
			},
			"fronthaul_ap_ctl" : {
				"value" : 18,
				"def" : {
					"fronthaul_ap_option_off" : {"bit" : 0},
					"fronthaul_ap_option_auto" : {"bit" : 1},
					"fronthaul_ap_option_on" : {"bit" : 2}
				}
			},
			"sta_binding_ap" : {
				"value" : 19,
				"def" : {
					"manual_sta_binding" : {"bit" : 0}
				}
			},
			"reset_default" : {
				"value" : 20,
				"def" : {
					"manual_reset_default" : {"bit" : 0}
				}
			},
			"wifi_radio_ctl" : {
				"value" : 22,
				"def" : {
					"wifi_radio_0" : {"bit" : 0}, //2G
					"wifi_radio_1" : {"bit" : 1}, //5G or 5G-1
					"wifi_radio_2" : {"bit" : 2}, //5G-2
					"wifi_radio_3" : {"bit" : 3}, //6G or 6G-1
					"wifi_radio_4" : {"bit" : 4}  //6G-2
				}
			},
			"conn_eap_mode" : {
				"value" : 23,
				"def" : {
					"ethernet_backhaul_mode" : {"bit" : 0}
				}
			},
			"GN_6G_NO" : {//Number of supported RE 6G guest network
				"value" : 24,
				"def" : {
					"GN_6G_1" : {"bit" : 0},
					"GN_6G_2" : {"bit" : 1},
					"GN_6G_3" : {"bit" : 2}
				}
			},
			"GN_6GH_NO" : {//Number of supported RE 6GH guest network
				"value" : 34,
				"def" : {
					"GN_6GH_1" : {"bit" : 0},
					"GN_6GH_2" : {"bit" : 1},
					"GN_6GH_3" : {"bit" : 2}
				}
			},
		};
		var node_capability_status = {};
		if("capability" in _node_info) {
			for(var type in node_capability_list) {
				if(node_capability_list.hasOwnProperty(type)) {
					var capability_type_idx = node_capability_list[type].value;
					var capability_value = 0;
					if(capability_type_idx in _node_info.capability) //check capability idx exist
						capability_value = (_node_info.capability[capability_type_idx] == "") ? 0 : _node_info.capability[capability_type_idx];
					else if(capability_type_idx == "15")//exception, for old FW, not have this capability
						capability_value = 1;
					var capability_type_def_list = node_capability_list[type].def;
					for(var def_item in capability_type_def_list) {
						if(capability_type_def_list.hasOwnProperty(def_item)) {
							var def_item_bitwise = capability_type_def_list[def_item]["bit"];
							var support = (capability_value & (1 << def_item_bitwise)) ? true : false;
							node_capability_status[def_item] = support;
						}
					}
				}
			}
		}
		return node_capability_status;
	},
	"aimesh_get_misc_info" : function(_node_info){
		var misc_info_type_list = {
			"cobrand" : {"idx" : 1},
			"rc_support" : {
				"idx" : 2,
				"def" : {
					"wpa3" : {"bit" : 0}
				}
			}
		};
		var misc_info_status = {};
		if("misc_info" in _node_info) {
			for(var type in misc_info_type_list) {
				if(misc_info_type_list.hasOwnProperty(type)) {
					var type_idx = misc_info_type_list[type].idx;
					var type_def_list = misc_info_type_list[type].def;
					var type_value = "";
					if(type_idx in _node_info.misc_info){
						type_value = (_node_info.misc_info[type_idx] == "") ? "" : _node_info.misc_info[type_idx];
					}
					if(type_def_list != undefined){
						for(var def_item in type_def_list) {
							if(type_def_list.hasOwnProperty(def_item)) {
								var def_item_bitwise = type_def_list[def_item]["bit"];
								misc_info_status[def_item] = ((type_value & (1 << def_item_bitwise)) ? true : false);
							}
						}
					}
					else{
						misc_info_status[type] = type_value;
					}
				}
			}
		}
		return misc_info_status;
	},
	"aimesh_get_win_open_url" : function(_node_info, _page){
		var node_capability = httpApi.aimesh_get_node_capability(_node_info);
		var url = "http://" + _node_info.ip + "/" + _page;
		if(node_capability.local_access){
			var header_info = httpApi.hookGet("get_header_info");
			if(header_info.protocol == "https"){
				url = "https://" + _node_info.ip + ":" + header_info.port + "/" + _page;
			}
		}
		return url;
	},
	"aimesh_get_node_wifi_band" : function(_node_info){
		var wifi_band_info = [];
		if("capability" in _node_info){
			if("26" in _node_info.capability){
				var wifi_band = _node_info.capability["26"]["wifi_band"];
				$.each(wifi_band, function(index, value){
					var band_data = value;
					var band_info = {};
					band_info.band = parseInt(value.band);
					band_info.count = parseInt(value.count);
					band_info.no_used = 0;
					$.each(band_data.vif, function(vif_if, vif_data){
						if(vif_data.type == "1")
							band_info.no_used += 1;
					});
					band_info.mode = 2;//support SDN capability wifi_band
					wifi_band_info.push(band_info);
				});
			}
			if(wifi_band_info.length == 0){//not support SDN, then check Guestnetwork support
				var node_capability = httpApi.aimesh_get_node_capability(_node_info);
				if(node_capability.GN_2G_1){
					wifi_band_info.push(get_band_info(1));//2.4G
				}
				if(node_capability.GN_5G_1 && !node_capability.GN_5GH_1){
					wifi_band_info.push(get_band_info(2));//5G
				}
				if(node_capability.GN_5G_1 && node_capability.GN_5GH_1){
					wifi_band_info.push(get_band_info(4));//5G-1
					wifi_band_info.push(get_band_info(8));//5G-2
				}
				if(node_capability.GN_6G_1 && !node_capability.GN_6GH_1){
					wifi_band_info.push(get_band_info(16));//6G
				}
				if(node_capability.GN_6G_1 && node_capability.GN_6GH_1){
					wifi_band_info.push(get_band_info(32));//6G-1
					wifi_band_info.push(get_band_info(64));//6G-2
				}

				function get_band_info(_band){
					var band_info = {};
					band_info.band = parseInt(_band);
					band_info.count = 1;//support one Guestnetwork
					band_info.no_used = 1;
					band_info.mode = 1;//support Guestnetwork first group sync to node
					return band_info;
				}
			}
		}
		return wifi_band_info;
	},
	"aimesh_get_node_lan_port" : function(_node_info){
		var lan_port_info = [];
		if("capability" in _node_info){
			if("27" in _node_info.capability){
			var lan_port = _node_info.capability["27"]["lan_port"];
				$.each(lan_port, function(index, value){
					var lan_port_data = value;
					var port_info = {};
					port_info.index = parseInt(value.index);
					port_info.if_name = value.if_name;
					port_info.phy_port_id = parseInt(value.phy_port_id);
					port_info.label_name = value.label_name;
					port_info.max_rate = parseInt(value.max_rate);
					lan_port_info.push(port_info);
				});
			}
		}
		return lan_port_info;
	},
	"get_ipsec_cert_info": function(callBack){
		$.ajax({
			url: "/ipsec_cert_info.cgi",
			dataType: 'json',
			async: true,
			error: function(){},
			success: function(response){
				if(callBack)
					callBack(response);
			}
		});
	},
	"get_ipsec_clientlist": function(callBack){
		$.ajax({
			url: "/get_ipsec_clientlist.cgi",
			dataType: 'json',
			data: {"get_json":"1"},
			async: true,
			error: function(){},
			success: function(response){
				if(callBack)
					callBack(response);
			}
		});
	},
	"set_ipsec_clientlist": function(postData){
		$.ajax({
			url: "/set_ipsec_clientlist.cgi",
			type: "POST",
			dataType: 'json',
			data: JSON.stringify(postData),
			async: true,
			error: function(){},
			success: function(response){}
		});
	},
	"renew_ikev2_cert_key": function(callBack){
		$.ajax({
			url: "/renew_ikev2_cert_key.cgi",
			async: true,
			error: function(){},
			success: function(response){
				if(callBack)
					callBack(response);
			}
		});
	},
	"get_ipsec_conn": function(callBack){
		$.ajax({
			url: "/appGet.cgi",
			async: true,
			error: function(){},
			success: function(response){
				if(callBack)
					callBack(response);
			}
		});
	},
	"clean_ipsec_log": function(callBack) {
		$.ajax({
			url: '/clear_file.cgi?clear_file_name=ipsec',
			dataType: 'script',
			error: function(xhr) {
				alert("Clean error!");/*untranslated*/
			},
			success: function(response) {
				if(callBack)
					callBack(response);
			}
		});
	},
	"set_ig_config": function(postData){
		$.ajax({
			url: "/set_ig_config.cgi",
			type: "POST",
			dataType: 'json',
			data: JSON.stringify(postData),
			async: true,
			error: function(){},
			success: function(response){}
		});
	},
	"get_ig_config": function(callBack){
		$.ajax({
			url: "/get_ig_config.cgi",
			async: true,
			error: function(){},
			success: function(response){
				if(callBack)
					callBack(response);
			}
		});
	},

	"chpass": function(postData){
		var statusCode = "-1";
		if(postData.cur_username)
			postData.cur_username = hexMD5(postData.cur_username).toLowerCase();

		if(postData.cur_passwd)
			postData.cur_passwd = hexMD5(postData.cur_passwd).toLowerCase();

		if(postData.new_username)
			postData.new_username = btoa(postData.new_username);

		if(postData.new_passwd)
			postData.new_passwd = btoa(postData.new_passwd);

		$.ajax({
			url: '/chpass.cgi',
			dataType: 'json',
			data: postData,
			async: false,
			error: function(){
			},
			success: function(response){
				statusCode = response.statusCode
			}
		})

		return statusCode;
	},
        "get_app_client_stats": function(queryParam, handler){
                $.ajax({
                        url: '/get_app_client_stats.cgi?' + queryParam,
                        dataType: 'json',
                        type: "GET",
                        error: function(jqXHR, textStatus, errorThrown){
                                //console.log("status:${jqXHR.status} error:${jqXHR.responseText}");
                                console.log("error:${textStatus}");
                        },
                        success: function(response){
                                if(handler) handler(response);
                        }
                });
        },

	"log": function(funcName, content, sessionId){
		var deviceId = httpApi.nvramGet(["extendno", "productid"]);

		if(typeof window.localStorage === 'undefined') return false;
		if(!sessionId) sessionId = deviceId.productid + "#" + deviceId.extendno;

		try{
			setTimeout(function(){
				window.localStorage.setItem(Date.now(), "[" + sessionId + "][" + funcName + "] " + content);
			}, 100*Math.random())
		}catch(err){
			localStorage.clear();
			setTimeout(function(){
				window.localStorage.setItem(Date.now(), "[" + sessionId + "][" + funcName + "] " + content);
			}, 100*Math.random())
		}
	},
	
	"getLog": function(){
		if(typeof window.localStorage === 'undefined') return false;

		function _download(filename, text) {
			var element = document.createElement('a');
			element.setAttribute('href', 'data:text/plain;charset=utf-8,' + encodeURIComponent(text));
			element.setAttribute('download', filename);
			element.style.display = 'none';
			document.body.appendChild(element);
			element.click();
			document.body.removeChild(element);
		}

		var logContent = [];
		var logContentArray = [];

		for(var key in window.localStorage){
			if(typeof window.localStorage[key] !== "function" && key !== "length"){
				logContentArray.push([key, window.localStorage[key]])
			}
		};

		logContentArray.sort(function(a, b){
			return a[0] - b[0];
		});

		logContentArray.forEach(function(data){
			var logTime = new Date(parseInt(data[0])).toString().split(" ")
			var logTimeArray = []
			logTimeArray.push(logTime[1], logTime[2], logTime[4])
			logContent.push("[" + logTimeArray.join(" ") + "]" + data[1])
		})

		_download("uiLog.txt", logContent.join("\n"));
	},

	"get_diag_avg_data": function(queryParam, handler){
/*
		example:
		httpApi.get_diag_avg_data({
				db: "wifi_detect",
				content: "tx_diff;rx_diff",
				duration: 60,
				point: 10,
				ts: new Date().getTime()
			}, function(r){
			console.log(r)
		})
*/		
		queryParam.ts = parseInt(queryParam.ts/1000);
		
		$.ajax({
			url: '/get_diag_avg_data.cgi',
			dataType: 'json',
			type: "POST",
			data: queryParam,
			error: function(){},
			success: function(response){
				if(handler) handler(response);
			}
		});
	},

	"get_diag_content_data": function(queryParam, handler){
/*
		example:
		httpApi.get_diag_content_data({
				db: "stainfo",
				content: "sta_rssi",
				duration: 360,
				ts: new Date().getTime()
			}, function(r){
			console.log(r)
		})
*/
		queryParam.ts = parseInt(queryParam.ts/1000);
		
		$.ajax({
			url: '/get_diag_content_data.cgi',
			dataType: 'json',
			type: "POST",
			data: queryParam,
			error: function(){},
			success: function(response){
				if(handler) handler(response);
			}
		});
	},

	"get_diag_active_client": function(queryParam, handler){
		queryParam.ts = parseInt(queryParam.ts/1000);
		
		$.ajax({
			url: '/get_diag_active_client.cgi',
			dataType: 'json',
			type: "POST",
			data: queryParam,
			error: function(){},
			success: function(response){
				if(handler) handler(response);
			}
		});
	},

    "get_diag_eth_traffic_data": function(queryParam, handler){
		queryParam.ts = parseInt(queryParam.ts/1000);
        queryParam.is_bh = 1;
		
		$.ajax({
			url: '/get_diag_eth_traffic_data.cgi',
			dataType: 'json',
			type: "POST",
			data: queryParam,
			error: function(){},
			success: function(response){
                var ret = {};
                var getAvg = function(x){return parseInt(x/queryParam.duration/10.24)/100};
                //ret.rx_rate = response.rx_diff.map(getAvg)
                //ret.tx_rate = response.tx_diff.map(getAvg)

				ret.traffic = (function(){
					var array = [];
					for(var i=0; i<response.tx_diff.length;i++){
						var combinedValue = parseInt(response.rx_diff[i]) + parseInt(response.tx_diff[i]);
						// var combinedValue = parseInt(response.rx_diff[i]);
						array.push(combinedValue);
					}

					return array;
				})();

				if(handler) handler(ret);
			}
		});
	},

	"diag_ping": {
		"start": function(){
			$.ajax({
				url: '/dns_ping.cgi',
				success: function(response){
					console.log(response)
				}
			});			
		},

		"getResult": function(){
			var _content = ["dns_ip", "alias", "valid", "min", "avg", "max", "pkt_sent", "pkt_recv", "pkt_loss_rate", "data_time"];
			var retData = {};
			var dns_ping_state = httpApi.nvramGet(["dns_ping_state"], true).dns_ping_state;

			if(dns_ping_state == "3"){
				retData.status = "FINISH";
			}
			else{
				retData.status = "PROCEEDING";
			}

			$.ajax({
				url: '/get_diag_content_data.cgi',
				async: false,
				data: {
					"db": "dns_ping",
					"content": _content.join(";")
				},
				success: function(response){
					try{
						var pingArray = JSON.parse(response).contents;
						for(var i=0; i<pingArray.length; i++){
							var target = pingArray[i];
							retData[target[0]] = {}
							
							for(j=1; j<target.length; j++){
								retData[target[0]][_content[j]] = target[j];
							}
						}
					}catch(e){
						retData = {"status": "PARSE ERROR"}
						httpApi.log("httpApi.diag_ping.getResult", response);
					}
				}
			});

			return retData;
		}
	},

	"iperf": {
		"start": function(target, handler){
			$.ajax({
				url: "/do_iperf.cgi?caller=" + target.caller + "&serverMac=" + target.serverMac + "&nodeMac=" + target.nodeMac,
				success: function(response){
					if(handler) handler.call(response);
				}
			});
		},

		"getResult": function(target){
			var retData = [];

			var _content = [
				"data_time",
				"sec1_Mbits", 
				"sec2_Mbits", 
				"sec3_Mbits", 
				"sec4_Mbits", 
				"sec5_Mbits", 
				"sec6_Mbits", 
				"sec7_Mbits",
				"sec8_Mbits",
				"sec9_Mbits",
				"sec10_Mbits",				
			];

			var queryData = {
				"db": "iperf_client",
				"content": _content.join(";"),
				"filter": ""
			}

			if(!target.serverMac && !target.nodeMac && !target.caller) return false;
			
			if(target.serverMac !== undefined)
				queryData["filter"] += "server_mac>txt>" + target.serverMac + ">0;";

			if(target.nodeMac !== undefined)
				queryData["filter"] += "node_mac>txt>" + target.nodeMac + ">0;";

			if(target.caller !== undefined)
				queryData["filter"] += "caller>txt>" + target.caller + ">0;";

			$.ajax({
				url: '/get_diag_content_data.cgi',
				async: false,
				data: queryData,
				success: function(response){
					var values = JSON.parse(response).contents;		
					retData = values.map(function(item){
						var sum = 0;
						var count = item.length-1;
						var timestamp = item[0];
						for(i=1;i<item.length;i++){
							sum += parseInt(item[i])
						}		
						
						return [sum/count, timestamp];
					});
				}
			});

			return retData;
		}
	},

	"fbwifi": {
		"register": function(){
			var statusCode = "-1";

			$.ajax({
				url: "/aae_fbwifi2_reg.cgi",
				type: "POST",
				dataType: 'json',

				error: function(){
					console.log("httpApi.fbwifi2.register error");
				},
				success: function(response) {
					statusCode = response.statusCode;
					fbwifi_id = response.fbwifi_id;
					fbwifi_secret = response.fbwifi_secret;

					if(statusCode != "0" || fbwifi_id == "off" || fbwifi_id == "" || fbwifi_secret == ""){
						setTimeout(httpApi.fbwifi.register, 2000);
					}
				}
			});
		},

		"isAvailable": function(){
			var fbInfo = httpApi.nvramGet(["link_internet", "fbwifi_cp_config_url", "fbwifi_id", "fbwifi_secret"], true);
			var isAvailable = (
				fbInfo.link_internet == 2 && 
				fbInfo.fbwifi_cp_config_url != "" && 
				fbInfo.fbwifi_id != "" && 
				fbInfo.fbwifi_id != "off" &&
				fbInfo.fbwifi_secret != ""  
			)

			// debug log
			httpApi.log("httpApi.fbwifi.isAvailable fbwifi_cp_config_url", fbInfo.fbwifi_cp_config_url);
			httpApi.log("httpApi.fbwifi.isAvailable fbwifi_id", fbInfo.fbwifi_id);
			httpApi.log("httpApi.fbwifi.isAvailable fbwifi_secret", fbInfo.fbwifi_secret);

			return isAvailable;
		}
	}
}
