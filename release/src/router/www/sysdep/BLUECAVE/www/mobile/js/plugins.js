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

function getAllWlArray(){
	var wlArrayRet = [{"title":"2.4GHz", "ifname":"0", "suffix": ""}];
	
	if(isSupport("triband")){
		wlArrayRet.push({"title":"5GHz-1", "ifname":"1", "suffix": "_5G-1"})
		wlArrayRet.push({"title":"5GHz-2", "ifname":"2", "suffix": "_5G-2"})
	}
	else if(isSupport("dualband") || isSupport('5G')){
		wlArrayRet.push({"title":"5GHz", "ifname":"1", "suffix": "_5G"})
	}

	if(isSupport('wigig')){
		wlArrayRet.push({"title":"60GHz", "ifname":"3", "suffix": "_60G"});
	}

	return wlArrayRet;
}

function getPAPList(siteSurveyAPList, filterType, filterValue) {
	var modelInfo = httpApi.nvramGet(["productid", "odmpid"], true);
	var based_modelid = modelInfo.productid;
	var papList = [];
	var profile = function(_profile){
		var getBandWidthName = function(ch){
			if(ch >= 1 && ch <= 14){
				return {name: "2.4GHz", unit: 0};
			}
			else{
				if(isSupport("triband"))
				{
					if(based_modelid == "MAP-AC2200")
						return (ch >= 36 && ch <= 64) ? {name: "5GHz-1", unit: 2} : {name: "5GHz-2", unit: 1};
					else
						return (ch >= 36 && ch <= 64) ? {name: "5GHz-1", unit: 1} : {name: "5GHz-2", unit: 2};
				}
				else
					return {name: "5GHz", unit: 1};
			}
		}

		if(_profile == null || _profile.length == 0)
			_profile = ["", "", "", "", "", "", "", "", "", ""];

		this.band = getBandWidthName(_profile[2]).name;
		this.unit = getBandWidthName(_profile[2]).unit;
		this.ssid = htmlEnDeCode.htmlEncode(decodeURIComponent(_profile[1]));
		this.channel = _profile[2];
		this.authentication = _profile[3];
		this.encryption = _profile[4];
		this.signal = (Math.ceil(_profile[5]/25) == 0) ? 1 : Math.ceil(_profile[5]/25);
		this.macaddr = _profile[6];
		this.wlmode = _profile[7];
		this.state = _profile[8];
		this.AiMesh = _profile[9];
		this.thekey = "";
		this.thekeyindex = "";
		this.thekeyauthmode = "";
	}

	for(var i=0; i<siteSurveyAPList.length; i++){
		var site = new profile(siteSurveyAPList[i]);
		if(filterType != undefined && filterType != ""){
			if(site[filterType] == filterValue)
				continue;
		}
		if(papList.indexOf(site.macaddr) === -1){
			papList.push(site.macaddr);
			papList[site.macaddr] = site;
		}
	}
	return papList;
}

function checkPasswd($obj){
	var targetObj = $(this);
	targetObj.toggleClass("icon_eye_close").toggleClass("icon_eye_open");

	$.each( targetObj.attr("for").split(" "), function(i, val){
		$("#"+val).prop("type", (function(){return targetObj.hasClass("icon_eye_close") ? "password" : "text";}()));
	});
}

function checkWepKey($obj, wepType){
	var status = false;
	var wepKey = $obj.val();
	if(wepType == "1"){
		if(!(wepKey.length === 5 && validator.string($obj[0])) &&
			!(wepKey.length === 10 && validator.hex($obj[0])))
		{
			$obj.showTextHint("<#JS_wepkey#><#WLANConfig11b_WEPKey_itemtype1#>");
			status = true;
		}
	}
	else if(wepType == "2"){
		if(!(wepKey.length === 13 && validator.string($obj[0])) &&
			!(wepKey.length === 26 && validator.hex($obj[0])))
		{
			$obj.showTextHint("<#JS_wepkey#><#WLANConfig11b_WEPKey_itemtype2#>");
			status = true;
		}
	}
	return status;
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

function rangeCheck(objArray, min, max, reserveHints){//1: reserve previous hints
	if(reserveHints != 1)
		$(".hint").remove();

	$.each(objArray, function(idx, $obj){
		if($obj.val().length > 0 && (isNaN($obj.val()) || $obj.val() < min || $obj.val() > max)){
			$obj.showTextHint('<#JS_validrange#> ' + min + ' <#JS_validrange_to#> ' + max + '.');
		}
	})
	if($(".hint").length > 0) return true;
}

function hadPlugged(deviceType){
	var usbDeviceList = httpApi.hookGet("show_usb_path") || [];
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

var Get_Component_SiteSurvey_List = function(papList, filterBand) {
	var siteSurveyContainer = $("<div>");
	papList.forEach(function(macIndex){
		var AP = papList[macIndex];
		if(AP.ssid == "") return true;
		var unit = AP.unit;
		if(filterBand != undefined) {
			for(var i = 0; i < filterBand.length; i += 1){
				if(unit == filterBand[i]) return true;
			}
		}

		var apListContainer = $("<div>").attr({"id" : AP.macaddr}).addClass("apListContainer apProfile");
		var apListDiv = $("<div>").addClass("apListDiv");
		apListContainer.append(apListDiv);

		var ap_icon_container = $("<div>").addClass("ap_icon_container");
		apListDiv.append(ap_icon_container);

		var ap_icon = $("<div>").addClass("icon_wifi_" + (AP.signal + (AP.encryption == "NONE" ? "" : "_lock")) + " ap_icon");
		ap_icon.appendTo(ap_icon_container);

		var ap_ssid =  $("<div>").addClass("ap_ssid").html(AP.ssid);
		apListDiv.append(ap_ssid);
		ap_ssid.hover(function(){
			$(this).addClass("ap_ssid_hover")
		}, function(){
			$(this).removeClass("ap_ssid_hover")
		})

		var ap_band =  $("<div>").addClass("ap_band").html(AP.band);
		apListDiv.append(ap_band);

		var ap_narrow_container = $("<div>").addClass("ap_narrow_container");
		apListDiv.append(ap_narrow_container);

		var ap_narrow = $("<div>").addClass("icon_arrow_right ap_narrow");
		ap_narrow.appendTo(ap_narrow_container);

		siteSurveyContainer.append(apListContainer);
	});
	return siteSurveyContainer;
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
		var wirelessAP = httpApi.nvramCharToAscii(["wl" + wl.ifname + "_ssid", "wl" + wl.ifname + "_wpa_psk", "wl" + wl.ifname + "_auth_mode_x"]);
		// Do not use default value.
		if(systemVariable.isDefault){
			wirelessAP["wl" + wl.ifname + "_ssid"] = "";
			wirelessAP["wl" + wl.ifname + "_wpa_psk"] = "";
		}
		else{
			if(wirelessAP["wl" + wl.ifname + "_auth_mode_x"] != "psk2")
				wirelessAP["wl" + wl.ifname + "_wpa_psk"] = "";
		}

		if(systemVariable.multiPAP.wlcOrder.length > 0){
			var ssid_tmp = "";
			var wpa_psk_tmp = "";
			if(qisPostData["wlc" + wl.ifname + "_ssid"] == undefined || qisPostData["wlc" + wl.ifname + "_ssid"] == ""){
				for(var i = 0; i < systemVariable.multiPAP.wlcOrder.length; i += 1){
					ssid_tmp = qisPostData["wlc" + systemVariable.multiPAP.wlcOrder[i] + "_ssid"];
					wpa_psk_tmp = qisPostData["wlc" + systemVariable.multiPAP.wlcOrder[i] + "_wpa_psk"];
					break;
				}
			}
			else{
				ssid_tmp = qisPostData["wlc" + wl.ifname + "_ssid"];
				wpa_psk_tmp = qisPostData["wlc" + wl.ifname + "_wpa_psk"];
			}
			switch(parseInt(wl.ifname)){
				case 0 :
					ssid_tmp = ssid_tmp.slice(0,28) + "_RPT";
					break;
				case 1 :
					ssid_tmp = ssid_tmp.slice(0,26) + "_RPT5G";
					break;
				case 2 :
					ssid_tmp = ssid_tmp.slice(0,25) + "_RPT5G2";
					break;
			}
			wirelessAP["wl" + wl.ifname + "_ssid"] = encodeURIComponent(ssid_tmp);

			if(wpa_psk_tmp != "")
				wirelessAP["wl" + wl.ifname + "_wpa_psk"] = encodeURIComponent(wpa_psk_tmp);
			else
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
					if($(e.currentTarget).attr("id").indexOf("0") == -1){
						$("#wireless_sync_checkbox").enableCheckBox(false);
					}

					if(e.keyCode == 13){
						$(".wlInput")[idx*2+1].focus();
					}
				})
				.val(decodeURIComponent(wirelessAP["wl" + wl.ifname + "_ssid"]))
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
					"autocomplete": "new-password",
					"autocorrect": "off",
					"autocapitalize": "off",
					"spellcheck": "false",
					"data-role": "none",
					"data-clear-btn": "true"
				})
				.keyup(function(e){
					if($(e.currentTarget).attr("id").indexOf("0") == -1){
						$("#wireless_sync_checkbox").enableCheckBox(false);
					}

					if(e.keyCode == 13){
						try{
							$(".wlInput")[idx*2+2].focus();
						}
						catch(e){
							apply.wireless();
						}
					}
				})
				.val(decodeURIComponent(wirelessAP["wl" + wl.ifname + "_wpa_psk"]))
			)
			.appendTo(__container);

		container.append(__container);
	})

	return container;
}

var Get_Component_ISPSelect = function(){
	var isp_select = $("<select>").attr({"name": "switch_wantag", "id": "switch_wantag"});
	var isp_profiles = httpApi.hookGet("get_iptvSettings").isp_profiles;
	var original_switch_wantag = httpApi.nvramGet(["switch_wantag"]).switch_wantag;
	var text = "";
	var selected = false;

	$.each(isp_profiles, function(i, isp_profile) {
		text = isp_profile.profile_name;

		if(text == "none")
			text = "<#wl_securitylevel_0#>";
		else if(text == "manual")
			text = "<#Manual_Setting_btn#>";

		if(isp_profile.switch_wantag == original_switch_wantag)
			selected = true;
		else
			selected = false;

		isp_select
			.append($("<option></option>")
			.attr("value",isp_profile.switch_wantag)
			.text(text)
			.attr("selected", selected));
	});

	return isp_select;
}

function handleSysDep(){
	var isNoWAN = (httpApi.detwanGetRet().wanType == 'NOWAN');

	$(".amasSupport").toggle(isSupport("AMAS"));
	$(".noAmasSupport").toggle(!isSupport("AMAS"));
	$(".tosSupport").toggle(systemVariable.isDefault && isSupport("QISBWDPI"));
	$(".repeaterSupport").toggle(isSupport("repeater"));
	$(".pstaSupport").toggle(isSupport("psta"));
	$(".dualbandSupport").toggle(isSupport("dualband") || isSupport("triband"));
	$(".vpnClient").toggle(isSupport("VPNCLIENT"));
	$(".iptv").toggle(isSupport("IPTV"));
	$(".defaultSupport").toggle(systemVariable.isDefault);
	$(".configuredSupport").toggle(!systemVariable.isDefault);
	//$(".forceUpgrade").toggle(isSupport("fupgrade")); 
	$(".routerSupport").toggle(!isSupport("noRouter"));

	if(systemVariable.forceChangePw){
		systemVariable.forceChangePw = false;
		systemVariable.forceChangePwInTheEnd = true;
	}

	if(!isNoWAN) $(".amasNoWAN").remove();
	if(!isSupport("amas")) $(".amasSupport").remove();
}

function handleModelIcon() {
	$('#ModelPid_img').css('background-image', 'url(' + function() {
		var modelInfo = httpApi.nvramGet(["territory_code", "productid", "odmpid", "color"], true);
		var ttc = modelInfo.territory_code;
		var based_modelid = modelInfo.productid;
		var odmpid = modelInfo.odmpid;
		var color = modelInfo.color;
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
			if(odmpid == "RT-AC66U_B1" || odmpid == "RT-AC1750_B1" || odmpid == "RT-N66U_C1" || odmpid == "RT-AC1900U" || odmpid == "RT-AC67U")
				MP_png_path = "/images/RT-AC66U_V2/Model_product.png";
			else if(odmpid == "RP-AC1900")
				MP_png_path = "/images/RP-AC1900/Model_product.png";

			if(MP_png_path == "")
				return default_png_path;
			else{
				if(LinkCheck(MP_png_path))
					return MP_png_path;
				else
					return default_png_path;
			}
		}
		else
			return default_png_path;
	}() + ')');
}

function handleSortField(){
	if(!$("#sort_field").is(":visible")){
		$("#sort_field").fadeIn(300)
		$("#sort_background").fadeIn(300)
	}
	else{
		$("#sort_field").fadeOut(300);
		$("#sort_background").fadeOut(300);
	}

	if(systemVariable.multiPAP.wlcOrder.length != 0)
		$("#sortByBand").hide();
	else
		$("#sortByBand").show();
}

function setUpTimeZone(){
	postDataModel.insert(timeObj);

	require(['/require/modules/timeZone.js'], function(timeZone) {
		var timeZoneObj = new timeZone.get(systemVariable.uiLanguage);
		qisPostData.time_zone = timeZoneObj.time_zone;
		qisPostData.time_zone_dst = (timeZoneObj.hasDst) ? 1 : 0;
	});
}

function setupWLCNvram(apProfileID) {
	systemVariable.selectedAP = systemVariable.papList[apProfileID];
	var unit = systemVariable.selectedAP.unit;
	postDataModel.insert(wlcMultiObj["wlc" + unit]);

	var encryption = systemVariable.selectedAP.encryption;
	var authentication = systemVariable.selectedAP.authentication;

	qisPostData["wlc" + unit + "_ssid"] = htmlEnDeCode.htmlDecode(systemVariable.selectedAP.ssid);
	qisPostData["wlc" + unit + "_band"] = unit;
	qisPostData["wlc" + unit + "_ap_mac"] = systemVariable.selectedAP.macaddr;

	if(encryption == "NONE"){
		qisPostData["wlc" + unit + "_auth_mode"] = "open";
		qisPostData["wlc" + unit + "_crypto"] = "";
		qisPostData["wlc" + unit + "_wep"] = "0";
	}
	else{
		if(encryption == "WEP"){
			qisPostData["wlc" + unit + "_auth_mode"] = "open";// open/shared authentication use the same profile, UI don't know which one to use.
			qisPostData["wlc" + unit + "_crypto"] = "";
			qisPostData["wlc" + unit + "_wep"] = "1";
		}
		else if(encryption == "TKIP"){
			qisPostData["wlc" + unit + "_auth_mode"] = "psk";
			qisPostData["wlc" + unit + "_crypto"] = "tkip";
			qisPostData["wlc" + unit + "_wep"] = "0";
		}
		else if(authentication == "WPA-WPA2-Personal"){
			qisPostData["wlc" + unit + "_auth_mode"] = "pskpsk2";
			if(encryption == "AES")
				qisPostData["wlc" + unit + "_crypto"] = "aes";
			else
				qisPostData["wlc" + unit + "_crypto"] = "tkip+aes";
			qisPostData["wlc" + unit + "_wep"] = "0";
		}
		else if(authentication == "WPA2-Personal" && encryption == "AES"){
			qisPostData["wlc" + unit + "_auth_mode"] = "psk2";
			qisPostData["wlc" + unit + "_crypto"] = "aes";
			qisPostData["wlc" + unit + "_wep"] = "0";
		}
		else if(authentication == "WPA-WPA2-Enterprise"){
			qisPostData["wlc" + unit + "_auth_mode"] = "wpawpa2";
			if(encryption == "AES")
				qisPostData["wlc" + unit + "_crypto"] = "aes";
			else
				qisPostData["wlc" + unit + "_crypto"] = "tkip+aes";
			qisPostData["wlc" + unit + "_wep"] = "0";
		}
		else if(authentication == "WPA2-Enterprise" && encryption == "AES"){
			qisPostData["wlc" + unit + "_auth_mode"] = "wpa2";
			qisPostData["wlc" + unit + "_crypto"] = "aes";
			qisPostData["wlc" + unit + "_wep"] = "0";
		}
		else{
			qisPostData["wlc" + unit + "_auth_mode"] = "psk2";
			qisPostData["wlc" + unit + "_crypto"] = "aes";
			qisPostData["wlc" + unit + "_wep"] = "0";
		}
	}

	//temp code for MB
	if(isSupport("concurrep") && isSwMode("MB")){
		qisPostData["wlc_band"] = unit;
	}
}

function sortAP(kind, sequence){
	var array_tmp = new Array();
	array_tmp = httpApi.hookGet("get_ap_info", false).sort(function(a,b){
		if(kind == "signal"){
			var a_tmp = parseInt(a[5]);	//compare signal strength
			var b_tmp = parseInt(b[5]);
		}
		else if(kind == "alpha"){
			var a_tmp = a[1].toUpperCase();	//compare SSID
			var b_tmp = b[1].toUpperCase();
		}
		else{
			var a_tmp = a[0];	//compare band
			var b_tmp = b[0];
		}

		if(sequence == "top"){
			if(a_tmp > b_tmp)
				return -1;
			else if(a_tmp < b_tmp)
				return 1;
			else
				return 0;
		}
		else{
			if(a_tmp > b_tmp)
				return 1;
			else if(a_tmp < b_tmp)
				return -1;
			else
				return 0;
		}
	});
	systemVariable.papList = getPAPList(array_tmp);
	genPAPList(systemVariable.papList, systemVariable.multiPAP.wlcOrder);
	handleSortField();
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
	var original_switch_wantag = httpApi.nvramGet(["switch_wantag"]).switch_wantag;

	if(isWANChanged()){
		actionScript.push("restart_wan_if 0");
	}

	if(systemVariable.detwanResult.isIPConflict){
		actionScript.push("restart_subnet");
	}

	if(qisPostData.hasOwnProperty("http_passwd")){
		actionScript.push("chpass")
	}

	if(qisPostData.hasOwnProperty("time_zone")){
		actionScript.push("restart_time")
	}

	if(qisPostData.hasOwnProperty("yadns_enable_x")){
		actionScript.push("restart_yadns");
	}

	if(
		qisPostData.hasOwnProperty("wl0_ssid") || 
		qisPostData.hasOwnProperty("wl0.1_ssid") || 
		qisPostData.hasOwnProperty("wl0_he_features") || 
		systemVariable.isDefault || 
		isSmartConnectChanged()
	){
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

	if(isSupport("2p5G_LWAN")) {
		actionScript.push("start_br_addif");
	}

	if(isSwModeChanged() && isSwMode("RT")){
		return "restart_all";
	}

	if(isSupport("2p5G_LWAN") || isSupport("10G_LWAN") || isSupport("10GS_LWAN")){
		if(isWANLANChange())
			return "reboot";
	}

	if((qisPostData.hasOwnProperty("switch_wantag") && (qisPostData.switch_wantag != original_switch_wantag)) ||
		qisPostData.hasOwnProperty("wlc_ssid") ||
		qisPostData.hasOwnProperty("lan_proto") ||
		qisPostData.hasOwnProperty("wans_dualwan") ||
		isSwModeChanged()
	){
		return "reboot";
	}

	return actionScript.join(";")
}

var Get_Value_Available_WL_Band = function(){
	var availableBand = [];
	var allWlArray =  getAllWlArray();
	for(var i = 0; i < allWlArray.length; i += 1){
		availableBand.push(allWlArray[i].ifname);
	}

	for(var i = 0; i < systemVariable.multiPAP.wlcOrder.length; i += 1){
		availableBand = availableBand.filter(function(item) {
			return item != systemVariable.multiPAP.wlcOrder[i]
		})
	}

	return availableBand;
};

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

	if(qisPostData.hasOwnProperty("wan_pppoe_username")){
		if(qisPostData.wan_pppoe_username != systemVariable.originPppAccount.username) isChanged = true;
	}

	if(qisPostData.hasOwnProperty("wan_pppoe_passwd")){
		if(qisPostData.wan_pppoe_passwd != systemVariable.originPppAccount.password) isChanged = true;
	}

	return isChanged;
};

var isWANLANChange = function(){
	var isChanged = false;

	if(isSupport("2p5G_LWAN") && qisPostData.hasOwnProperty("wans_extwan")){
		if(qisPostData.wans_extwan != systemVariable.originWansExtwan) isChanged = true;
	}

	if((isSupport("10G_LWAN") || isSupport("10GS_LWAN")) && qisPostData.hasOwnProperty("wans_dualwan")){
		if(qisPostData.wans_dualwan != systemVariable.originWansDualwan) isChanged = true;
	}

	return isChanged;
};

var isPage = function(page){
	return $("#" + page).is(":visible");
}

var isSupport = function(_ptn){
	var ui_support = JSON.parse(JSON.stringify(httpApi.hookGet("get_ui_support")));
	var matchingResult = false;
	var odmpid = httpApi.nvramGet(["odmpid"]).odmpid;

	if(ui_support["triband"] && ui_support["concurrep"] && (isSwMode("RP") || isSwMode("MB"))){
		/* setup as dualband models, will copy wlc1 to wlc2 in apply.submitQIS */
		ui_support["SMARTREP"] = 1;
		ui_support["triband"] = 0;
		ui_support["dualband"] = 1;
	}
	else{
		ui_support = httpApi.hookGet("get_ui_support");
	}

	switch(_ptn){
		case "ForceBWDPI":
			matchingResult = false;
			break;
		case "QISBWDPI":
			matchingResult = false;
			break;
		case "VPNCLIENT":
			matchingResult = (isSku("US") || isSku("CA") || isSku("TW") || isSku("CN")) ? false : true;
			break;
		case "IPTV":
			matchingResult = (isSku("US") || isSku("CN") || isSku("CA")) ? false : true;
			break;
		case "SMARTCONNECT":
			matchingResult = (ui_support["smart_connect"] == 1 || ui_support["bandstr"] == 1) ? true : false;
			break;
		case "MB_mode_concurrep":
			if(isSwMode("MB") && isSupport("concurrep") && odmpid != "RP-AC1900")
				matchingResult = true;
			else
				matchingResult = false;
			break;
		default:
			matchingResult = ((ui_support[_ptn] == 1) || (systemVariable.productid.search(_ptn) !== -1)) ? true : false;
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

var isSmartConnectChanged = function(){
	var flag = false;
	if(isSupport("SMARTCONNECT")){
		if(qisPostData.hasOwnProperty("smart_connect_x")) {
			var smart_connect_x = httpApi.nvramGet(["smart_connect_x"]).smart_connect_x;
			if(qisPostData.smart_connect_x != smart_connect_x)
				flag = true;
		}
	}
	return flag;
}
var isSdk = function(ver){
	return (ver == systemVariable.sdkVersion) ? true : false;
}

var isWlUser = (function(){
	var isWL = true;
	var clientList = httpApi.hookGet("get_clientlist");
	Object.keys(clientList).forEach(function(key){
		if(clientList[key].isLogin == 1){
			isWL = (clientList[key].isWL != 0) ? true : false;
		}
	});

	return isWL;
})()

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

function startDetectLinkInternet(){
	systemVariable.linkInternet = httpApi.isConnected();

	if(!systemVariable.linkInternet){
		setTimeout(arguments.callee, 1000);
	}
}

function startLiveUpdate(){
	if(!systemVariable.linkInternet){
		setTimeout(arguments.callee, 1000);
	}
	else{
		httpApi.nvramSet({"action_mode":"apply", "webs_update_trigger":"QIS", "rc_service":"start_webs_update"}, function(){
			setTimeout(function(){
				var fwInfo = httpApi.nvramGet(["webs_state_update", "webs_state_info_am", "webs_state_flag"], true);

				if(fwInfo.webs_state_flag == "1" || fwInfo.webs_state_flag == "2"){
					systemVariable.isNewFw = fwInfo.webs_state_flag;
					systemVariable.newFwVersion = fwInfo.webs_state_info;
				}
				if(fwInfo.webs_state_update == "0" || fwInfo.webs_state_update == ""){
					setTimeout(arguments.callee, 1000);
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
	if(testResult.isError && str.length < 2)
		testResult.errReason = "<#JS_short_username#>";

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

validator.KRSkuPwd = function(str){
	var testResult = {
		'isError': false,
		'errReason': ''
	}

	if( !/[A-Za-z]/.test(str) || !/[0-9]/.test(str) || str.length < 8 
		|| !/[\!\"\#\$\%\&\'\(\)\*\+\,\-\.\/\:\;\<\=\>\?\@\[\\\]\^\_\`\{\|\}\~]/.test(str)
	){
		testResult.isError = true;
		testResult.errReason = "<#JS_validPWD#>";
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

transformWLCObj = function(){
	var wlcUnit = systemVariable.multiPAP.wlcOrder[0];
	Object.keys(qisPostData).forEach(function(key){
		qisPostData[key.replace("wlc" + wlcUnit, "wlc")] = qisPostData[key];
	});

	postDataModel.remove(wlcMultiObj["wlc" + wlcUnit]);
};
copyWLCObj_wlc1ToWlc2 = function(){
	var wlcPostData = wlcMultiObj.wlc2;
	$.each(wlcPostData, function(item){wlcPostData[item] = qisPostData[item.replace("2", "1")];});
	qisPostData.wlc2_band = 2;
	postDataModel.insert(wlcPostData);
};

transformWLToGuest = function(){
	var transformWLIdx = function(_wlcUnit){
		Object.keys(qisPostData).forEach(function(key){
			qisPostData[key.replace("wl" + _wlcUnit, "wl" + _wlcUnit + ".1")] = qisPostData[key];
		});
		postDataModel.remove(wirelessObj["wl" + _wlcUnit]);
	};
	if(systemVariable.multiPAP.wlcOrder.length == 1){//single pap
		var wlcUnit = systemVariable.multiPAP.wlcOrder[0];
		transformWLIdx(wlcUnit);
	}
	else{//multi pap
		var allWlArray =  getAllWlArray();
		for(var i = 0; i < allWlArray.length; i += 1){
			var wlcUnit = allWlArray[i].ifname;
			transformWLIdx(wlcUnit);
		}
	}
};
handleWirelessClientSSID = function(_wlArray, _autoStr){
	_wlArray.forEach(function(wl, idx){
		var ssid_tmp = "";
		var wpa_psk_tmp = "";
		if(qisPostData["wlc" + wl.ifname + "_ssid"] == undefined || qisPostData["wlc" + wl.ifname + "_ssid"] == ""){
			for(var i = 0; i < systemVariable.multiPAP.wlcOrder.length; i += 1){
				ssid_tmp = qisPostData["wlc" + systemVariable.multiPAP.wlcOrder[i] + "_ssid"];
				wpa_psk_tmp = qisPostData["wlc" + systemVariable.multiPAP.wlcOrder[i] + "_wpa_psk"];
				break;
			}
		}
		else{
			ssid_tmp = qisPostData["wlc" + wl.ifname + "_ssid"];
			wpa_psk_tmp = qisPostData["wlc" + wl.ifname + "_wpa_psk"];
		}
		if(!_autoStr){
			switch(parseInt(wl.ifname)){
				case 0 :
					ssid_tmp = ssid_tmp.slice(0,28) + "_RPT";
					break;
				case 1 :
					ssid_tmp = ssid_tmp.slice(0,26) + "_RPT5G";
					break;
				case 2 :
					ssid_tmp = ssid_tmp.slice(0,25) + "_RPT5G2";
					break;
			}
		}
		$("#wireless_ssid_" + wl.ifname).val(ssid_tmp);
		if(wpa_psk_tmp != "")
			$("#wireless_key_" + wl.ifname).val(wpa_psk_tmp);
		else
			$("#wireless_key_" + wl.ifname).val("");
	});
};
isAllPAPSet = function(){
	var status = true;
	var bandCount = getAllWlArray().length;
	for(var i = 0; i < bandCount; i += 1) {
		if(!systemVariable.multiPAP.wlcStatus["wlc" + i + "_checked"]) {
			status = false;
			break;
		}
	}
	return status;
};
genPAPList = function(papList, filterBand){
	var Get_Text_PAP_Title = function(){
		var title = "<#QIS_extend_AP#>";
		if(isSupport("RPMesh"))
			title = "Here are WiFi networks nearby, select one to continue.";/* untranslated */

		if(systemVariable.multiPAP.wlcOrder.length > 0){
			var band_setted_text = "";
			var band_notSetted_text = "";
			var available_band = Get_Value_Available_WL_Band();
			for(var i = 0; i < available_band.length; i += 1){
				if(band_notSetted_text != "")
					band_notSetted_text += " / " + getAllWlArray()[available_band[i]].title;
				else
					band_notSetted_text += getAllWlArray()[available_band[i]].title;
			}
			for(var i = 0; i < systemVariable.multiPAP.wlcOrder.length; i += 1){
				if(band_setted_text != "")
					band_setted_text += " / " + getAllWlArray()[systemVariable.multiPAP.wlcOrder[i]].title;
				else
					band_setted_text += getAllWlArray()[systemVariable.multiPAP.wlcOrder[i]].title;
			}
			title = "You have already selected the " + band_setted_text + " wireless network. Do you want to select the " + band_notSetted_text + " wireless network?";
			title += "&nbsp;";
			title += "If not, click [<span class='textClick' onclick='goTo.skip_pap();'>SKIP</span>]";
		}
		return title;
	};
	$("#papList_page").find(".pageDesc").html(Get_Text_PAP_Title());
	$("#papList_page").find("#apList")
		.html(Get_Component_SiteSurvey_List(papList, filterBand))
		.find(".apProfile").click(function() {
			setupWLCNvram(this.id);
			goTo.papSet();
		});
};
genWLBandOption = function(){
	$("#wlc_band_manual").find("option").remove();
	if($("#wlc_band_manual").has("option").length == 0){
		var wlArray = Get_Value_Available_WL_Band();
		wlArray.forEach(function(band){
			$("#wlc_band_manual").append($("<option>").val(band).html(getAllWlArray()[band].title));
		});
	}
};
handleWLWepOption = function(authMode){
	if(authMode == "open"){
		$("#wlc_wep_manual option[value='0']").show();
		$("#wlc_wep_manual option[value='0']").prop("selected", true).change();
	}
	else if(authMode == "shared"){
		$("#wlc_wep_manual option[value='0']").hide();
		$("#wlc_wep_manual option[value='1']").prop("selected", true).change();
	}
};
handleWLAuthModeItem = function(){
	var auth_mode = $("#wlc_auth_mode_manual").val();
	var crypto = $("#wlc_crypto_manual").val();
	var wep = $("#wlc_wep_manual").val();
	$("#manual_pap_setup-crypto").hide();
	$("#manual_pap_setup-wep").hide();
	$("#manual_pap_setup-key-index").hide();
	$("#manual_pap_setup-key").hide();
	$("#manual_pap_setup-nmode_hint").hide();
	if(auth_mode == "open" && wep == "0"){
		$("#manual_pap_setup-wep").show();
	}
	else if((auth_mode == "open" && wep != "0") || auth_mode == "shared"){
		$("#manual_pap_setup-wep").show();
		$("#manual_pap_setup-key-index").show();
		$("#manual_pap_setup-key").show();
		$("#manual_pap_setup-nmode_hint").show();
	}
	else if(auth_mode == "psk" || auth_mode == "psk2"){
		$("#manual_pap_setup-crypto").show();
		$("#manual_pap_setup-key").show();
		if(crypto == "tkip")
			$("#manual_pap_setup-nmode_hint").show();
	}
};
