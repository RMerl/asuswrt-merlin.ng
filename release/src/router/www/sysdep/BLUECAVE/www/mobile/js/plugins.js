jQuery.fn.showTextHint = function(hintStr){
	this.parent().children().remove(".hint");

	$("<div>")
		.html(hintStr)
		.addClass("hint")
		.appendTo(this.parent());
}

jQuery.fn.showSelectorHint = function(hintStr){
	this.parent().children().remove(".hint");

	$("<div>")
		.html(hintStr)
		.addClass("hint")
		.addClass("selectorHint")
		.appendTo(this.parent());
}

jQuery.fn.enableCheckBox = function(checked){
	this
		.checkboxradio()
		.prop('checked', checked)
		.checkboxradio('refresh')
}

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
			'&amp;': '&',
			'&gt;': '>',
			'&lt;': '<',
			'&quot;': '"',
			'&#39;': "'"
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

function checkPasswd($obj){
	var targetObj = $(this);
	targetObj.toggleClass("icon_eye_close").toggleClass("icon_eye_open");

	$.each( targetObj.attr("for").split(" "), function(i, val){
		$("#"+val).prop("type", (function(){return targetObj.hasClass("icon_eye_close") ? "password" : "text";}()));
	});
}

function hasBlank(objArray){
	$(".hint").remove();
	$.each(objArray, function(idx, $obj){
		if($obj.val() == ""){
			$obj.showTextHint("<#JS_fieldblank#>");
		}
	})
	if($(".hint").length > 0) return true;
}

function hadPlugged(deviceType){
	var usbDeviceList = httpApi.hookGet("show_usb_path")["show_usb_path"][0] || [];
	return (usbDeviceList.join().search(deviceType) != -1)
}

var Get_Component_Header = function(){
	var tableTr = $("<tr>");

	tableTr
		.append(
			$("<td>")
				.css({"width":"17px"})
				.append('<div style="margin-right:20px;"><a href="#navigation"><div class="icon_menu"></div></a></div>')
		)
		.append(
			$("<td>").append('<div><div class="icon_logo"></div></div>')
		)
		.append(
			$("<td>").append('<div class="model_welcome"></div>')
		)

	return $("<table>")
		.append(tableTr)
		.css({"width": "100%"})
}

var Get_Component_Loading = function(){
	if($(this).find( ".cssload-bell" ).length === 0){
		var loadContainer = $("<div>").addClass("cssload-bell");
	
		for(var i=0; i<5; i++){
			$("<div>")
				.addClass("cssload-circle")
				.append($("<div>").addClass("cssload-inner"))
				.appendTo(loadContainer);
		}

		return $("<div>").addClass("cssload-bellContainer").append(loadContainer);
	}
}

var Get_Component_btnLoading = function(){
	return $("<img>").attr({
		"width": "30px",
		"src": "/images/InternetScan.gif"
	})
}
var Get_Component_WirelessInput = function(wlArray){
	var container = $("<div>");

	wlArray.forEach(function(wl, idx){
		var wirelessAP = httpApi.nvramGet(["wl" + wl.ifname + "_ssid", "wl" + wl.ifname + "_wpa_psk"]);
		// Do not use default value.
		if(systemVariable.isDefault){
			wirelessAP["wl" + wl.ifname + "_ssid"] = "";
			wirelessAP["wl" + wl.ifname + "_wpa_psk"] = "";
		}

		var __container = $("<div>").addClass("wirelessBand");

		$("<div>")
			.addClass("inputTitleContainer")
			.append($("<div>").addClass("inputTitle").html(wl.title + " <#QIS_finish_wireless_item1#>"))
			.appendTo(__container)

		$("<div>")
			.addClass("inputContainer")
			.append($("<input>")
				.attr({
					"id": "wireless_ssid_" + wl.ifname,
					"type": "text",
					"maxlength": "32",
					"class": "textInput wlInput",
					"autocomplete": "off",
					"autocorrect": "off",
					"autocapitalize": "off",
					"spellcheck": "false",
					"data-role": "none",
					"data-clear-btn": "true"
				})
				.keyup(function(e){
					if(e.keyCode == 13){
						$(".wlInput")[idx*2+1].focus();
					}
				})
				.val(wirelessAP["wl" + wl.ifname + "_ssid"])
			)
			.appendTo(__container)

		$("<div>")
			.addClass("inputTitleContainer")
			.append($("<div>").addClass("inputTitle").html(wl.title + " <#QIS_finish_wireless_item2#>"))
			.append($("<div>").addClass("secureInput icon_eye_close").attr({"for":"wireless_key_" + wl.ifname}))
			.appendTo(__container)

		$("<div>")
			.addClass("inputContainer")
			.append($("<input>")
				.attr({
					"id": "wireless_key_" + wl.ifname,
					"type": "password",
					"maxlength": "64",
					"class": "textInput wlInput",
					"autocomplete": "off",
					"autocorrect": "off",
					"autocapitalize": "off",
					"spellcheck": "false",
					"data-role": "none",
					"data-clear-btn": "true"
				})
				.keyup(function(e){
					if(e.keyCode == 13){
						try{
							$(".wlInput")[idx*2+2].focus();
						}
						catch(e){
							apply.wireless();
						}
					}
				})
				.val(wirelessAP["wl" + wl.ifname + "_wpa_psk"])
			)
			.appendTo(__container);

		container.append(__container);
	})

	return container;
}

function handleSysDep(){
	$("#welcomeTitle").html($("#welcomeTitle").html().replace("BLUE_CAVE", "BLUE CAVE"))
	var enableAMAS = httpApi.nvramGet(["amas_force"], true);
	$(".amasSupport").toggle(isSupport("AMAS") && (enableAMAS.amas_force == "1"));
	$(".noAmasSupport").toggle(!isSupport("AMAS") || (enableAMAS.amas_force != "1"));
	$(".tosSupport").toggle(systemVariable.isDefault && isSupport("QISBWDPI"));
	$(".repeaterSupport").toggle(isSupport("repeater"));
	$(".pstaSupport").toggle(isSupport("psta"));
	$(".dualbandSupport").toggle(isSupport("DUALBAND") || isSupport("TRIBAND"));
	$(".bandStreeringSupport").toggle(isSupport("SMARTCONNECT"));
	$(".vpnClient").toggle(isSupport("VPNCLIENT"));
	$(".iptv").toggle(isSupport("IPTV"));
	$(".defaultSupport").toggle(systemVariable.isDefault);
	$(".configuredSupport").toggle(!systemVariable.isDefault);
	$(".forceUpgrade").toggle(isSupport("FORCEUPGRADE"));

	$("#syncSSID").toggle(!isSupport("SMARTCONNECT") && (isSupport("DUALBAND") || isSupport("TRIBAND")));
	$("#wireless_sync_checkbox").enableCheckBox(!isSupport("SMARTCONNECT") && (isSupport("DUALBAND") || isSupport("TRIBAND")));

	if(systemVariable.forceChangePw && isSupport("AMAS")){
		systemVariable.forceChangePw = false;
		systemVariable.forceChangePwInTheEnd = true;
	}

	var wanType = httpApi.detwanGetRet().wanType;
	$(".amasInAdv").toggle(wanType === 'NOWAN');
}

function handleModelIcon() {
	$('#ModelPid_img').attr('src',
		function() {
			var ttc = '<% nvram_get("territory_code"); %>';
			var based_modelid = '<% nvram_get("productid"); %>';
			var odmpid = '<% nvram_get("odmpid"); %>';
			var color = '<% nvram_get("color"); %>';
			color = color.toUpperCase();
			var LinkCheck = function(url) {
				var http = new XMLHttpRequest();
				http.open('HEAD', url, false);
				http.send();
				return http.status!="404";
			};

			var update_color = function() {
				if(based_modelid == "RT-AC87U") { //MODELDEP: RT-AC87U
					/* MODELDEP by Territory Code */
					if(ttc == "JP/02" || ttc == "AP/02" || ttc == "SG/02")
						return "R";
					else if(ttc == "JP/02")
						return "W";
					else
						return color;
				}
				if(odmpid.length > 0 && odmpid != based_modelid){	//odmpid MODELDEP
					if(odmpid == "RT-N66W" || odmpid == "RT-AC66W" || odmpid == "RT-AC68W" || odmpid == "RT-AC68RW")
						return "W";
					else
						return color;
				}
				else {
					return color;
				}
			};
			var default_png_path = "/images/Model_product.png";
			var MP_png_path = "";
			if(update_color().length > 0) {
				MP_png_path = "/images/Model_product_"+ update_color() +".png";
				if(LinkCheck(MP_png_path))
					return MP_png_path;
				else
					return default_png_path;
			}
			else if(odmpid.length > 0 && odmpid != based_modelid) {
				if(odmpid == "RT-AC66U_B1" || odmpid == "RT-AC1750_B1" || odmpid == "RT-N66U_C1" || odmpid == "RT-AC1900U") {
					MP_png_path = "/images/RT-AC66U_V2/Model_product.png";
					if(LinkCheck(MP_png_path))
						return MP_png_path;
					else
						return default_png_path;
				}
				else
					return default_png_path;
			}
			else
				return default_png_path;
		}
	);
}

function setUpTimeZone(){
	postDataModel.insert(timeObj);

	require(['/require/modules/timeZone.js'], function(timeZone) {
		var timeZoneObj = new timeZone.get(systemVariable.uiLanguage);
		qisPostData.time_zone = timeZoneObj.time_zone;
		qisPostData.time_zone_dst = (timeZoneObj.hasDst) ? 1 : 0;
	});
}

function updateSubnet(ipAddr){
	if(ipAddr == systemVariable.lanIpaddr && systemVariable.detwanResult.isIPConflict){
		var ipAddrArray = ipAddr.split(".");
		if(parseInt(ipAddrArray[2]) < 254){
			ipAddrArray[2]++;		
		}
		else{
			ipAddrArray[1]++;
		}

		return ipAddrArray.join(".")
	}

	return ipAddr;
}

var getRestartService = function(){
	var actionScript = [];

	if(isWANChanged()){
		actionScript.push("restart_wan_if 0");
	}

	if(systemVariable.detwanResult.isIPConflict){
		actionScript.push("restart_subnet");
	}

	if(qisPostData.hasOwnProperty("time_zone")){
		actionScript.push("restart_time")
	}

	if(qisPostData.hasOwnProperty("yadns_enable_x")){
		actionScript.push("restart_yadns");
	}

	if(qisPostData.hasOwnProperty("wl0_ssid") || qisPostData.hasOwnProperty("wl0.1_ssid")){
		actionScript.push("restart_wireless");
	}

	if(systemVariable.isDefault && isSupport("lantiq")){
		actionScript.push("stop_bluetooth_service");
	}

	if(qisPostData.hasOwnProperty("wrs_protect_enable")){
		actionScript.push("restart_firewall");
		actionScript.push("restart_wrs");
	}

	if(qisPostData.hasOwnProperty("cfg_master")){
		actionScript.push("restart_cfgsync");
	}

	if( qisPostData.hasOwnProperty("switch_wantag") ||
		qisPostData.hasOwnProperty("wlc_ssid") ||
		qisPostData.hasOwnProperty("lan_proto") ||
		qisPostData.hasOwnProperty("wans_dualwan") ||
		isSwModeChanged()
	){
		return "reboot";
	}

	return actionScript.join(";")
}

var postDataModel = {
	"insert": function(obj){
		if(obj){
			var queryArray = [];
			Object.keys(obj).forEach(function(key){
				if(obj[key] === "")
					queryArray.push(key);
				else
					qisPostData[key] = obj[key];
			});

			var retData = httpApi.nvramGet(queryArray);
			delete retData.isError;
			Object.keys(retData).forEach(function(key){
				qisPostData[key] = retData[key];
			})
		}
	},

	"remove": function(obj){
		if(obj) Object.keys(obj).forEach(function(key){delete qisPostData[key];})
	}
}

function updateOriginWan(){
	systemVariable.originWanType = (qisPostData.hasOwnProperty("wan_proto")) ? qisPostData.wan_proto : systemVariable.originWanType;
	systemVariable.wanDnsenable = (qisPostData.hasOwnProperty("wan_dnsenable_x")) ? qisPostData.wan_dnsenable_x : systemVariable.wanDnsenable;
}

var isWANChanged = function(){
	var isChanged = false;

	if(qisPostData.hasOwnProperty("wan_proto")){
		if(qisPostData.wan_proto != systemVariable.originWanType) isChanged = true;
	}

	if(qisPostData.hasOwnProperty("wan_dnsenable_x")){
		if(qisPostData.wan_dnsenable_x != systemVariable.wanDnsenable) isChanged = true;
	}

	return isChanged;
};

var isPage = function(page){
	return $("#" + page).is(":visible");
}

var isSupport = function(_ptn){
	var matchingResult = false;
	switch(_ptn){
		case "ForceBWDPI":
			matchingResult = false;
			break;
		case "AMAS":
			matchingResult = (systemVariable.rcSupport.search("amas") !== -1) ? true : false;
			break;
		case "QISBWDPI":
			// matchingResult = (systemVariable.rcSupport.search("bwdpi") !== -1) ? true : false;
			matchingResult = false;
			break;
		case "DUALBAND":
			matchingResult = (systemVariable.wirelessBand == 2) ? true : false;
			break;
		case "TRIBAND":
			matchingResult = (systemVariable.wirelessBand == 3) ? true : false;
			break;
		case "VPNCLIENT":
			matchingResult = (isSku("US") || isSku("CA") || isSku("TW") || isSku("CN")) ? false : true;
			break;
		case "IPTV":
			matchingResult = (isSku("EU") || isSku("SG") || isSku("AA") || isSku("TW")) ? true : false;
			break;
		case "FORCEUPGRADE":
			matchingResult = (systemVariable.rcSupport.search("fupgrade") !== -1) ? true : false;
			break;
		case "SMARTCONNECT":
			matchingResult = (systemVariable.rcSupport.search("smart_connect") !== -1 || systemVariable.rcSupport.search("bandstr") !== -1) ? true : false;
			break;
		default:
			matchingResult = ((systemVariable.rcSupport.search(_ptn) !== -1) || (systemVariable.productid.search(_ptn) !== -1)) ? true : false;
			break;
	}

	return matchingResult;
}

var isSku = function(_ptn){
	return (systemVariable.territoryCode.search(_ptn) == -1) ? false : true;
}

var isSwMode = function(mode){
	return (mode.toLowerCase() == systemVariable.opMode.toLowerCase()) ? true : false;
}

var isOriginSwMode = function(mode){
	return (mode.toLowerCase() == systemVariable.originOpMode.toLowerCase()) ? true : false;
}

var isSwModeChanged = function(){
	return (systemVariable.opMode.toLowerCase() != systemVariable.originOpMode.toLowerCase()) ? true : false;
}

var isSdk = function(ver){
	return (ver == systemVariable.sdkVersion) ? true : false;
}

var isWlUser = (function(){
	var isWL = true;
	var clientList = httpApi.hookGet("get_clientlist").get_clientlist;
	Object.keys(clientList).forEach(function(key){
		if(clientList[key].isLogin == 1){
			isWL = (clientList[key].isWL != 0) ? true : false;
		}
	});

	return isWL;
})()

var isNewFw = function(fwver){
	var Latest_firmver = fwver.split("_");
	var originFirmwareVersion = httpApi.nvramGet(["firmver", "buildno", "extendno"]);

	if(Latest_firmver.length > 2){
		var Latest_firm = parseInt(Latest_firmver[0]);
		var Latest_buildno = parseInt(Latest_firmver[1]);
		var Latest_extendno = parseInt(Latest_firmver[2].split("-g")[0]);
		var current_firm = parseInt(originFirmwareVersion.firmver.replace(/[.]/gi,""));
		var current_buildno = parseInt(originFirmwareVersion.buildno);
		var current_extendno = parseInt(originFirmwareVersion.extendno.split("-g")[0]);

		if( current_buildno < Latest_buildno || 
			current_firm < Latest_firm && current_buildno == Latest_buildno ||
			current_extendno < Latest_extendno && current_buildno == Latest_buildno && current_firm == Latest_firm
		){
			return true;
		}
	}
	
	return false;
}

var isWeakString = function(pwd, flag){
	String.prototype.repeat = function(times){return (new Array(times + 1)).join(this);};

	String.prototype.strReverse = function() {
		var newstring = "";
		for (var s=0; s<this.length; s++) {
			newstring = this.charAt(s) + newstring;
		}
		return newstring;
	};

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
			if(pwd == termCommon8[s]){ nSeqString++; }
		}		
	}
	
	//pure repeat character string
	if(pwd == pwd.charAt(0).repeat(pwd.length)) { nSeqString++; }
	
	if(nSeqString > 0)
		return true;
	else		
		return false;
}

function addNewScript(scriptName){
	var script = document.createElement("script");
	script.type = "text/javascript";
	script.src = scriptName;
	document.getElementsByTagName("head")[0].appendChild(script);
}

function startLiveUpdate(){
	var linkLnternet = httpApi.isConnected();
	if(!linkLnternet){
		setTimeout(arguments.callee, 1000);
	}
	else{
		httpApi.nvramSet({"action_mode":"apply", "rc_service":"start_webs_update"}, function(){
			setTimeout(function(){
				var fwInfo = httpApi.nvramGet(["webs_state_update", "webs_state_info"], true);

				if(fwInfo.webs_state_update == "0" || fwInfo.webs_state_update == ""){
					setTimeout(arguments.callee, 1000);
				}
				else if(fwInfo.webs_state_info !== ""){
					systemVariable.isNewFw = isNewFw(fwInfo.webs_state_info);
					systemVariable.newFwVersion = fwInfo.webs_state_info;
				}
			}, 1000);
		});
	}
}

validator.hostNameString = function(str){
	var testResult = {
		'isError': false,
		'errReason': "<#JS_validhostname#>"
	}

	var re = new RegExp("^[a-zA-Z0-9][a-zA-Z0-9\-\_]+$","gi");
	testResult.isError = re.test(str) ? false : true;

	return testResult;
};

validator.invalidChar = function(str){
	var testResult = {
		'isError': false,
		'errReason': ''
	}

	var invalid_char = [];
	for(var i = 0; i < str.length; ++i){
		if(str.charAt(i) < ' ' || str.charAt(i) > '~'){
			invalid_char.push(str.charAt(i));
		}
	}

	if(invalid_char.length != 0){
		testResult.isError = true;
		testResult.errReason = "<#JS_validstr2#> '" + invalid_char.join('') + "' !";
	}

	return testResult;
};
