/* Plugin */
if (!Object.keys) {
  Object.keys = (function() {
    'use strict';
    var hasOwnProperty = Object.prototype.hasOwnProperty,
        hasDontEnumBug = !({ toString: null }).propertyIsEnumerable('toString'),
        dontEnums = [
          'toString',
          'toLocaleString',
          'valueOf',
          'hasOwnProperty',
          'isPrototypeOf',
          'propertyIsEnumerable',
          'constructor'
        ],
        dontEnumsLength = dontEnums.length;

    return function(obj) {
      if (typeof obj !== 'object' && (typeof obj !== 'function' || obj === null)) {
        throw new TypeError('Object.keys called on non-object');
      }

      var result = [], prop, i;

      for (prop in obj) {
        if (hasOwnProperty.call(obj, prop)) {
          result.push(prop);
        }
      }

      if (hasDontEnumBug) {
        for (i = 0; i < dontEnumsLength; i++) {
          if (hasOwnProperty.call(obj, dontEnums[i])) {
            result.push(dontEnums[i]);
          }
        }
      }
      return result;
    };
  }());
}

String.prototype.toArray = function(){
	var ret = eval(this.toString());
	if(Object.prototype.toString.apply(ret) === '[object Array]')
		return ret;
	return [];
}

var wl_nband_title = [];
var wl_nband_array = "<% wl_nband_info(); %>".toArray();
var band2g_count = 0;
var band5g_count = 0;
var band6g_count = 0;
var band60g_count = 0;
for (var j=0; j<wl_nband_array.length; j++) {
	if(wl_nband_array[j] == '2'){
		band2g_count++;
		wl_nband_title.push("2.4 GHz" + ((band2g_count > 1) ? ("-" + band2g_count) : ""));
	}
	else if(wl_nband_array[j] == '1'){
		band5g_count++;
		wl_nband_title.push("5 GHz" + ((band5g_count > 1) ? ("-" + band5g_count) : ""));
	}
	else if(wl_nband_array[j] == '4'){
		band6g_count++;
		wl_nband_title.push("6 GHz" + ((band6g_count > 1) ? ("-" + band6g_count) : ""));
	}
	else if(wl_nband_array[j] == '6'){
		band60g_count++;
		wl_nband_title.push("60 GHz" + ((band60g_count > 1) ? ("-" + band60g_count) : ""));
	}
}

if(wl_nband_title.indexOf("2.4 GHz-2") > 0) wl_nband_title[wl_nband_title.indexOf("2.4 GHz")] = "2.4 GHz-1";
if(wl_nband_title.indexOf("5 GHz-2") > 0) wl_nband_title[wl_nband_title.indexOf("5 GHz")] = "5 GHz-1";
if(wl_nband_title.indexOf("6 GHz-2") > 0) wl_nband_title[wl_nband_title.indexOf("6 GHz")] = "6 GHz-1";
if(wl_nband_title.indexOf("60 GHz-2") > 0) wl_nband_title[wl_nband_title.indexOf("60 GHz")] = "60 GHz-1";

function get_wl_unit_by_band(_band){
	if(_band == undefined)
		return "";

	_band = (_band).toString().toUpperCase();
	var wl_nband = "";
	switch(_band){
		case "2G":
			wl_nband = "2";
			break;
		case "5G":
		case "5G1":
		case "5G2":
			wl_nband = "1";
			break;
		case "6G":
		case "6G1":
		case "6G2":
			wl_nband = "4";
			break;
		case "60G":
			wl_nband = "6";
			break;
	}
	if(wl_nband == "")
		return "";

	var wl_unit = "";
	var suffix_num = _band.substr(_band.indexOf("G") + 1);
	var ordinal_num = (suffix_num == "") ? 1 : parseInt(suffix_num);
	var count = 1;
	var wl_nband_array = httpApi.hookGet("wl_nband_info");
	$.each(wl_nband_array, function(wlx, value){
		if(value == wl_nband){
			if(count == ordinal_num){
				wl_unit = wlx;
				return false;
			}
			else{
				count+=1;
			}
		}
	});
	return wl_unit.toString();
}

function inet_network(ip_str) {
	if (!ip_str) return -1;

	var re = /^(\d+)\.(\d+)\.(\d+)\.(\d+)$/;
	if (re.test(ip_str)) {
		var v1 = parseInt(RegExp.$1);
		var v2 = parseInt(RegExp.$2);
		var v3 = parseInt(RegExp.$3);
		var v4 = parseInt(RegExp.$4);

		if (v1 < 256 && v2 < 256 && v3 < 256 && v4 < 256)
			return v1 * 256 * 256 * 256 + v2 * 256 * 256 + v3 * 256 + v4;
	}

	return -2;
}
if(typeof isSwMode != "function"){
	var isSwMode = function(mode){
		var ui_sw_mode = "rt";
		var sw_mode = '<% nvram_get("sw_mode"); %>';
		var wlc_psta = '<% nvram_get("wlc_psta"); %>' == '' ? 0 : '<% nvram_get("wlc_psta"); %>';
		var wlc_express = '<% nvram_get("wlc_express"); %>' == '' ? 0 : '<% nvram_get("wlc_express"); %>';

		if(((sw_mode == '2' && wlc_psta == '0') || (sw_mode == '3' && wlc_psta == '2')) && wlc_express == '0'){	// Repeater
			ui_sw_mode = "re";
		}
		else if((sw_mode == '3' && wlc_psta == '0') || (sw_mode == '3' && wlc_psta == '')){	// Access Point
			ui_sw_mode = "ap";
		}
		else if((sw_mode == '3' && wlc_psta == '1' && wlc_express == '0') || (sw_mode == '3' && wlc_psta == '3' && wlc_express == '0') || (sw_mode == '2' && wlc_psta == '1' && wlc_express == '0')){	// MediaBridge
			ui_sw_mode = "mb";
		}
		else if(sw_mode == '2' && wlc_psta == '0' && wlc_express == '1'){	// Express Way 2G
			ui_sw_mode = "ew2";
		}
		else if(sw_mode == '2' && wlc_psta == '0' && wlc_express == '2'){	// Express Way 5G
			ui_sw_mode = "ew5";
		}
		else if(sw_mode == '5'){	// Hotspot
			ui_sw_mode = 'hs';
		}
		else ui_sw_mode = "rt"; // Router

		return (ui_sw_mode.search(mode) !== -1);
	}
}

function isSupport(_ptn){
	const ui_support = httpApi.hookGet("get_ui_support");
	return (ui_support[_ptn]) ? ui_support[_ptn] : 0;
}
const smart_connect_version = isSupport("smart_connect_v2") ? "v2" : isSupport("smart_connect") || isSupport("bandstr") ? "v1" : "";

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

var isJsonChanged = function(objNew, objOld){
	for(var i in objOld){	
		if(typeof objOld[i] == "object" && i != "fromNetworkmapd"){
			if(objNew[i].join() != objOld[i].join()){
				return true;
			}
		}
		else if( i == "fromNetworkmapd"){
			if(JSON.stringify(objNew[i]) != JSON.stringify(objOld[i]))
				return true;
		}
		else{
			if(typeof objNew[i] == "undefined" || JSON.stringify(objOld[i]) != JSON.stringify(objNew[i]))
				return true;
		}
	}

    return false;
};
var client_convRSSI = function(val) {
	var result = 1;
	val = parseInt(val);
	if(val >= -50) result = 4;
	else if(val >= -80) result = Math.ceil((24 + ((val + 80) * 26)/10)/25);
	else if(val >= -90) result = Math.ceil((((val + 90) * 26)/10)/25);
	else return 1;

	if(result == 0) result = 1;
	return result;
};

var ipState = new Array();
ipState["Static"] = "<#BOP_ctype_title5#>";
ipState["DHCP"] = "<#BOP_ctype_title1#>";
ipState["Manual"] = "<#Clientlist_IPMAC_Binding#>";
ipState["OffLine"] = "<#Clientlist_OffLine_Hint#>";

var vendorArrayRE = /(adobe|amazon|apple|asus|belkin|bizlink|buffalo|dell|d-link|fujitsu|google|hon hai|htc|huawei|ibm|lenovo|nec|microsoft|panasonic|pioneer|ralink|samsung|sony|synology|toshiba|tp-link|vmware)/;

var networkmap_fullscan = '<% nvram_get("networkmap_fullscan"); %>';

var originDataTmp;
var originData = {
	fromNetworkmapd : [httpApi.hookGet("get_clientlist")],
	nmpClient : [httpApi.hookGet("get_clientlist_from_json_database")], //Record the client connected to the router before.
	init: true
}

var totalClientNum = {
	online: 0,
	wireless: 0,
	wired: 0,
	wireless_ifnames: []
}

var AiMeshTotalClientNum = [];
var isWL_map = {
	"0" : {
		"text": "Wired",
		"type": "eth",
		"idx": 1
	},
	"1" : {
		"text": "2.4G",
		"type": "2g",
		"idx": 1
	},
	"2" : {
		"text": "5G",
		"type": "5g",
		"idx": 1
	},
	"3" : {
		"text": "5G",
		"type": "5g",
		"idx": 2
	},
	"4" : {
		"text": "6G",
		"type": "6g",
		"idx": 1
	},
	"5" : {
		"text": "6G",
		"type": "6g",
		"idx": 2
	}
};
var _wl_band_count = (function(){
	var _wl_nband_array = "<% wl_nband_info(); %>".toArray();
	var counts = {};
	for(var i = 0; i < _wl_nband_array.length; i++){
		var band_text = (function(_wl_band){
			if(_wl_band == "2")
				return "2g";
			else if(_wl_band == "1")
				return "5g";
			else if(_wl_band == "4")
				return "6g";
		})(_wl_nband_array[i]);
		counts[band_text] = (counts[band_text] + 1) || 1;
	}
	return counts;
})();
for(var index in isWL_map){
	if(index == "0")//filter wired
		continue;
	var wl_item = isWL_map[index];
	if(_wl_band_count[wl_item.type] != undefined && _wl_band_count[wl_item.type] > 1){
		wl_item["text"] = ((wl_item.type).toLocaleUpperCase() + "-" + wl_item["idx"]);
	}
}

var setClientAttr = function(){
	this.type = "0";
	this.defaultType = "0";
	this.name = "";
	this.nickName = "";
	this.ip = "offline";
	this.mac = "";
	this.from = "";
	this.macRepeat = 1;
	this.group = "";
	this.rssi = "";
	this.ssid = "";
	this.isWL = 0; // 0: wired, 1: 2.4GHz, 2: 5GHz/5GHz-1 3:5GHz-2.
	this.isGN = "";
	this.qosLevel = "";
	this.curTx = "";
	this.curRx = "";
	this.totalTx = "";
	this.totalRx = "";
	this.callback = "";
	this.keeparp = "";
	this.isGateway = false;
	this.isWebServer = false;
	this.isPrinter = false;
	this.isITunes = false;
	this.isASUS = false;
	this.isLogin = false;
	this.isOnline = false;
	this.ipMethod = "Static";
	this.opMode = 0;
	this.wlConnectTime = "00:00:00";
	this.vendor = "";
	this.dpiType = "";
	this.dpiDevice = "";
	this.internetMode = "allow";
	this.internetState = 1; // 1:Allow Internet access, 0:Block Internet access
	this.wtfast = 0;
	this.wlInterface = "";
	this.amesh_isRe = false;
	this.amesh_isReClient = false;
	this.amesh_papMac = "";
	this.amesh_bind_mac = "";
	this.amesh_bind_band = "0";
	this.sdn_idx = "0";
	this.ROG = false;
}

const saveCloudAsusClientIcon = (mac, name) => {
	fetch(`https://nw-dlcdnet.asus.com/plugin/productIcons/${name}.png`)
		.then(response => {
			if (response.status === 200) {
				response.blob().then(blob => {
					const reader = new FileReader();
					reader.readAsDataURL(blob);
					reader.onloadend = function () {
						let nvramSet_obj = {"action_mode": "apply"};
						nvramSet_obj["custom_usericon"] = `${mac.replace(/\:/g, "")}>${reader.result}`
						httpApi.nvramSet(nvramSet_obj)
					}
				});
			}
		})
}

var clientList = new Array(0);
function genClientList(){
	clientList = [];
	totalClientNum.online = 0;
	totalClientNum.wired = 0;
	totalClientNum.wireless = 0;
	AiMeshTotalClientNum = [];
	var uploadIconMacList = getUploadIconList().replace(/\.log/g, "");

	for(var index in isWL_map){
		if(index == "0")//filter wired
			continue;
		totalClientNum.wireless_ifnames[index - 1] = 0;
	}
	if(originData.fromNetworkmapd != undefined && originData.fromNetworkmapd.length > 0 && originData.fromNetworkmapd[0].maclist != undefined) {
		for(var i=0; i<originData.fromNetworkmapd[0].maclist.length; i++){
			var thisClient = originData.fromNetworkmapd[0][originData.fromNetworkmapd[0].maclist[i]];
			var thisClientMacAddr = (typeof thisClient.mac == "undefined") ? false : thisClient.mac.toUpperCase();

			if(!thisClientMacAddr){
				continue;
			}

			if(typeof clientList[thisClientMacAddr] == "undefined"){
				clientList.push(thisClientMacAddr);
				clientList[thisClientMacAddr] = new setClientAttr();
				clientList[thisClientMacAddr].from = thisClient.from;
			}
			else{
				clientList[thisClientMacAddr].macRepeat++;
				if(clientList[thisClientMacAddr].isOnline)
					totalClientNum.online++;
				continue;
			}

			clientList[thisClientMacAddr].isOnline = (thisClient.isOnline == "1") ? true : false;
			if(clientList[thisClientMacAddr].isOnline)
				totalClientNum.online++;

			if(!isSupport("sfp4m")) {
				clientList[thisClientMacAddr].type = thisClient.type;
				clientList[thisClientMacAddr].defaultType = thisClient.defaultType;
			}
			
			clientList[thisClientMacAddr].ip = thisClient.ip;
			clientList[thisClientMacAddr].ip6 = typeof thisClient.ip6 === "undefined" ? "" : thisClient.ip6;
			clientList[thisClientMacAddr].ip6_prefix = typeof thisClient.ip6_prefix === "undefined" ? "" : thisClient.ip6_prefix;
			clientList[thisClientMacAddr].mac = thisClient.mac.toUpperCase();

			clientList[thisClientMacAddr].name = thisClient.name.trim();
			if(clientList[thisClientMacAddr].name == ""){
				clientList[thisClientMacAddr].name = clientList[thisClientMacAddr].mac;
			}
			clientList[thisClientMacAddr].nickName = thisClient.nickName.trim();
			clientList[thisClientMacAddr].isGateway = (thisClient.isGateway == "1");
			clientList[thisClientMacAddr].isWebServer = (thisClient.isWebServer == "1");
			clientList[thisClientMacAddr].isPrinter = (thisClient.isPrinter == "1");
			clientList[thisClientMacAddr].isITunes = (thisClient.isITunes == "1");
			clientList[thisClientMacAddr].dpiDevice = ( thisClient.dpiDevice == "undefined") ? "" : thisClient.dpiDevice;
			clientList[thisClientMacAddr].vendor = thisClient.vendor;
			clientList[thisClientMacAddr].rssi = parseInt(thisClient.rssi);
			clientList[thisClientMacAddr].isWL = parseInt(thisClient.isWL);
			clientList[thisClientMacAddr].sdn_idx = parseInt(thisClient.sdn_idx);

			if(isSupport("amas"))
				clientList[thisClientMacAddr].isGN = ((thisClient.isGN != "") ? parseInt(thisClient.isGN) : "");
			if(isSupport("amas") && isSupport("dualband") && clientList[thisClientMacAddr].isWL == 3)
				clientList[thisClientMacAddr].isWL = 2;
			if(clientList[thisClientMacAddr].isOnline) {
				if(clientList[thisClientMacAddr].isWL > 0) {
					totalClientNum.wireless += clientList[thisClientMacAddr].macRepeat;
					totalClientNum.wireless_ifnames[clientList[thisClientMacAddr].isWL-1] += clientList[thisClientMacAddr].macRepeat;
				}
				else {
					totalClientNum.wired += clientList[thisClientMacAddr].macRepeat;
				}
			}

			clientList[thisClientMacAddr].opMode = parseInt(thisClient.opMode); //0:unknow, 1: router, 2: repeater, 3: AP, 4: Media Bridge
			clientList[thisClientMacAddr].isLogin = (thisClient.isLogin == "1");
			clientList[thisClientMacAddr].group = thisClient.group;
			clientList[thisClientMacAddr].callback = thisClient.callback;
			clientList[thisClientMacAddr].keeparp = thisClient.keeparp;
			clientList[thisClientMacAddr].ipMethod = thisClient.ipMethod;
			if(clientList[thisClientMacAddr].sdn_idx > 0){
				clientList[thisClientMacAddr].ipMethod = "DHCP";
			}
			clientList[thisClientMacAddr].qosLevel = thisClient.qosLevel;
			clientList[thisClientMacAddr].wtfast = parseInt(thisClient.wtfast);
			clientList[thisClientMacAddr].internetMode = thisClient.internetMode;
			clientList[thisClientMacAddr].internetState = thisClient.internetState;
			if(isSupport("stainfo")) {
				clientList[thisClientMacAddr].curTx = (thisClient.curTx == "") ? "": thisClient.curTx;
				clientList[thisClientMacAddr].curRx = (thisClient.curRx == "") ? "": thisClient.curRx;
				clientList[thisClientMacAddr].wlConnectTime = thisClient.wlConnectTime;
			}

			if(isSupport("amas")) {
				if(thisClient.amesh_isRe != undefined) {
					clientList[thisClientMacAddr].amesh_isRe = (thisClient.amesh_isRe == "1") ? true : false;
					if(clientList[thisClientMacAddr].amesh_isRe && clientList[thisClientMacAddr].isOnline) { // re set amesh re device to offline
						clientList[thisClientMacAddr].isOnline = false;
						totalClientNum.online--;
						if(clientList[thisClientMacAddr].isWL > 0) {
							totalClientNum.wireless -= clientList[thisClientMacAddr].macRepeat;
							totalClientNum.wireless_ifnames[clientList[thisClientMacAddr].isWL-1] -= clientList[thisClientMacAddr].macRepeat;
						}
						else {
							totalClientNum.wired -= clientList[thisClientMacAddr].macRepeat;
						}
						if(AiMeshTotalClientNum[thisClientMacAddr] == undefined)
							AiMeshTotalClientNum[thisClientMacAddr] = 0;
					}
				}

				if(thisClient.amesh_isReClient != undefined && thisClient.amesh_papMac != undefined) {
					clientList[thisClientMacAddr].amesh_isReClient = (thisClient.amesh_isReClient == "1") ? true : false;
					clientList[thisClientMacAddr].amesh_papMac = thisClient.amesh_papMac;
					if(clientList[thisClientMacAddr].amesh_papMac == ""){
						clientList[thisClientMacAddr].amesh_papMac = '<% get_lan_hwaddr(); %>';
					}
					if(clientList[thisClientMacAddr].isOnline) {
						if(AiMeshTotalClientNum[thisClient.amesh_papMac] == undefined)
							AiMeshTotalClientNum[thisClient.amesh_papMac] = 1;
						else
							AiMeshTotalClientNum[thisClient.amesh_papMac] = AiMeshTotalClientNum[thisClient.amesh_papMac] + 1;
					}
				}

				if(isSupport("force_roaming") && isSupport("sta_ap_bind")) {
					clientList[thisClientMacAddr].amesh_bind_mac = (typeof thisClient.amesh_bind_mac == "undefined") ? "" : thisClient.amesh_bind_mac;
					clientList[thisClientMacAddr].amesh_bind_band = (typeof thisClient.amesh_bind_band == "undefined") ? "0" : thisClient.amesh_bind_band;
				}
			}

			clientList[thisClientMacAddr].ROG = (thisClient.ROG == "1");
			clientList[thisClientMacAddr].isUserUplaodImg = (uploadIconMacList.indexOf(thisClient.mac.toUpperCase().replace(/\:/g, "")) >= 0) ? true : false;
			if(isSupport("mlo")){
				clientList[thisClientMacAddr].mlo = (typeof thisClient.mlo == "undefined") ? false : (thisClient.mlo == "1" ? true : false);
			}
			clientList[thisClientMacAddr].isASUS = (thisClient.isASUS == "1");

			if (clientList[thisClientMacAddr].type != '' && !clientList[thisClientMacAddr].isUserUplaodImg && clientList[thisClientMacAddr].isASUS && clientList[thisClientMacAddr].type == clientList[thisClientMacAddr].defaultType && clientList[thisClientMacAddr].name != "ASUS") {
				saveCloudAsusClientIcon(clientList[thisClientMacAddr].mac, clientList[thisClientMacAddr].name);
			}
		}
	}

	var nmpCount = 0;
	if(originData.nmpClient.length > 0) {
		for(var i=0; i<originData.nmpClient[0].maclist.length; i++){
			var thisClient = originData.nmpClient[0][originData.nmpClient[0].maclist[i]];
			var thisClientMacAddr = (typeof thisClient.mac == "undefined") ? false : thisClient.mac.toUpperCase();
			if(!thisClientMacAddr){
				continue;
			}

			if(nmpCount > 100)
				break;

			if(typeof clientList[thisClientMacAddr] == "undefined") {
				var thisClientType = (typeof thisClient.type == "undefined") ? "0" : thisClient.type;
				var thisClientDefaultType = (typeof thisClient.defaultType == "undefined") ? thisClientType : thisClient.defaultType;
				var thisClientName = (typeof thisClient.name == "undefined") ? thisClientMacAddr : (thisClient.name.trim() == "") ? thisClientMacAddr : thisClient.name.trim();
				thisClientName = htmlEnDeCode.htmlEncode(thisClientName);
				var thisClientNickName = (typeof thisClient.nickName == "undefined") ? "" : (thisClient.nickName.trim() == "") ? "" : thisClient.nickName.trim();
				thisClientNickName = htmlEnDeCode.htmlEncode(thisClientNickName);
				var thisClientReNode = (typeof thisClient.amesh_isRe == "undefined") ? false : ((thisClient.amesh_isRe == "1") ? true : false);

				clientList.push(thisClientMacAddr);
				clientList[thisClientMacAddr] = new setClientAttr();
				clientList[thisClientMacAddr].from = thisClient.from;
				if(!isSupport("sfp4m")) {
					clientList[thisClientMacAddr].type = thisClientType;
					clientList[thisClientMacAddr].defaultType = thisClientDefaultType;
				}
				clientList[thisClientMacAddr].mac = thisClientMacAddr;
				clientList[thisClientMacAddr].name = thisClientName;
				clientList[thisClientMacAddr].nickName = thisClientNickName;
				clientList[thisClientMacAddr].vendor = thisClient.vendor.trim();
				if(isSupport("amas")) {
					clientList[thisClientMacAddr].amesh_isRe = thisClientReNode;
					if(isSupport("force_roaming") && isSupport("sta_ap_bind")) {
						clientList[thisClientMacAddr].amesh_bind_mac = (typeof thisClient.amesh_bind_mac == "undefined") ? "" : thisClient.amesh_bind_mac;
						clientList[thisClientMacAddr].amesh_bind_band = (typeof thisClient.amesh_bind_band == "undefined") ? "0" : thisClient.amesh_bind_band;
					}
				}

				clientList[thisClientMacAddr].ROG = (thisClient.ROG == "1");
				clientList[thisClientMacAddr].isUserUplaodImg = (uploadIconMacList.indexOf(thisClient.mac.toUpperCase().replace(/\:/g, "")) >= 0) ? true : false;
				nmpCount++;
			}
			else if(!clientList[thisClientMacAddr].isOnline) {
				clientList[thisClientMacAddr].from = thisClient.from;
				nmpCount++;
			}
		}
	}

	//initial Gateway client
	if(isSupport("amas")) {
		var cap_mac = '<% nvram_get("lan_hwaddr"); %>';
		if(typeof clientList[cap_mac] == "undefined"){
			clientList.push(cap_mac);
			clientList[cap_mac] = new setClientAttr();
			clientList[cap_mac].from = "manual";
			clientList[cap_mac].mac = cap_mac;
			clientList[cap_mac].name = '<% nvram_get("productid"); %>';
		}
	}
}

//Initialize client list obj immediately
genClientList();

function getUploadIcon(clientMac) {
	var result = "NoIcon";
	if(isSupport("usericon")){
		$.ajax({
			url: '/appGet.cgi?hook=get_upload_icon()&clientmac=' + clientMac,
			dataType: 'json',
			async: false,
			success: function(response){
				var base64_image = htmlEnDeCode.htmlEncode(response.get_upload_icon);
				result = (isImageBase64(base64_image)) ? base64_image : "NoIcon";
			}
		});
	}
	return result;

	function isImageBase64(str){
		var str_tmp = str.slice();
		if(str_tmp.substring(0,11) == "data:image/"){
			var str_tmp_arr = str_tmp.substring(11).split(";");
			if(str_tmp_arr.length != 2){
				return false;
			}
			var mimeTypeRegExp = /(jpg|jpeg|gif|png|bmp|ico)/;
			var mimeType_str = str_tmp_arr[0];
			if(mimeType_str.length > 5){
				return false;
			}
			var match_data = mimeType_str.match(mimeTypeRegExp);
			if(!Boolean(match_data)){
				return false;
			}
			var base64_str = str_tmp_arr[1];
			if(base64_str != undefined && (base64_str.substring(0,7) == "base64,")){
				var img_str = base64_str.substring(7);//filter base64,
				var len = img_str.length;
				if(!len || len % 4 != 0 || /[^A-Z0-9+\/=]/i.test(img_str)){
					return false;
				}
				var firstPaddingChar = img_str.indexOf('=');
				return (firstPaddingChar === -1 || firstPaddingChar === len - 1 || (firstPaddingChar === len - 2 && img_str[len - 1] === '='));
			}
		}
		return false;
	}
}

function getUploadIconCount() {
	var count = 0;
	if(isSupport("usericon")){
		$.ajax({
			url: '/appGet.cgi?hook=get_upload_icon_count_list()',
			dataType: 'json',
			async: false,
			success: function(response){
				count = parseInt(response.get_upload_icon_count_list.upload_icon_count);
				if(isNaN(count)) count = 0;
			}
		});
	}
	return count
}

function getUploadIconList() {
	var list = "";
	if(isSupport("usericon")){
		$.ajax({
			url: '/appGet.cgi?hook=get_upload_icon_count_list()',
			dataType: 'json',
			async: false,
			success: function(response){
				list = response.get_upload_icon_count_list.upload_icon_list;
			}
		});
	}
	return list
}

function getVendorIconClassName(vendorName) {
	var vendor_class_name = "";
	var match_data = vendorName.match(vendorArrayRE);
	if(Boolean(match_data) && match_data[0] != undefined) {
		vendor_class_name = match_data[0];
		if(vendor_class_name == "hon hai")
			vendor_class_name = "honhai";
	}
	else {
		vendor_class_name = "";
	}
	return vendor_class_name;
}

function removeElement(element) {
    element && element.parentNode && element.parentNode.removeChild(element);
}
var temp_clickedObj = null;
var client_hide_flag = false;
function hide_edit_client_block() {
	if(client_hide_flag) {
		fadeOut(document.getElementById("edit_client_block"), 10, 0);
		if(temp_clickedObj.className.search("clientIcon") != -1) {
			temp_clickedObj.className = temp_clickedObj.className.replace("clientIcon_clicked","clientIcon");
			temp_clickedObj.className = temp_clickedObj.className.replace(" card_clicked", "");
		}
		else if(temp_clickedObj.className.search("vendorIcon") != -1) {
			temp_clickedObj.className = temp_clickedObj.className.replace("vendorIcon_clicked","vendorIcon");
			temp_clickedObj.className = temp_clickedObj.className.replace(" card_clicked", "");
		}
		document.body.onclick = null;
		temp_clickedObj = null;
	}
	client_hide_flag = true;
}
function show_edit_client_block() {
	client_hide_flag = false;
}

function card_closeClientListView() {
	hide_edit_client_block();
}

var card_client_variable = {
	"firstTimeOpenBlock" : false,
	"custom_usericon_del" : "",
	"userIconBase64" : "NoIcon",
	"userIconBase64_ori" : "NoIcon",
	"userUploadFlag" : false,
	"ipBindingFlag" : false,
	"timeSchedulingFlag" : false,
	"blockInternetFlag" : false,
	"custom_clientlist" : decodeURIComponent('<% nvram_char_to_ascii("", "custom_clientlist"); %>').replace(/&#62/g, ">").replace(/&#60/g, "<"),
	"manual_dhcp_list" : (function(){
		var manual_dhcp_list_array = new Array();
		var manual_dhcp_list = decodeURIComponent('<% nvram_char_to_ascii("", "dhcp_staticlist"); %>').replace(/&#62/g, ">").replace(/&#60/g, "<");
		var manual_dhcp_list_row = manual_dhcp_list.split("<");
		for(var dhcpIndex = 0; dhcpIndex < manual_dhcp_list_row.length; dhcpIndex += 1) {
			if(manual_dhcp_list_row[dhcpIndex] != "") {
				var manual_dhcp_list_col = manual_dhcp_list_row[dhcpIndex].split(">");
				var mac = manual_dhcp_list_col[0].toUpperCase();
				var ip = manual_dhcp_list_col[1];
				var dns = (manual_dhcp_list_col[2] == undefined) ? "" : manual_dhcp_list_col[2];
				var item_para = {"ip" : ip, "dns" : dns};
				manual_dhcp_list_array[mac] = item_para;
			}
		}
		return manual_dhcp_list_array;
	})(),
	"MULTIFILTER_ENABLE" : decodeURIComponent('<% nvram_char_to_ascii("", "MULTIFILTER_ENABLE"); %>').replace(/&#62/g, ">").replace(/&#60/g, "<"),
	"MULTIFILTER_MAC" : decodeURIComponent('<% nvram_char_to_ascii("", "MULTIFILTER_MAC"); %>').replace(/&#62/g, ">").replace(/&#60/g, "<"),
	"MULTIFILTER_DEVICENAME" : decodeURIComponent('<% nvram_char_to_ascii("", "MULTIFILTER_DEVICENAME"); %>').replace(/&#62/g, ">").replace(/&#60/g, "<"),
	"MULTIFILTER_MACFILTER_DAYTIME" : (function(){
		if(isSupport("PC_SCHED_V3"))
			return decodeURIComponent('<% nvram_char_to_ascii("", "MULTIFILTER_MACFILTER_DAYTIME_V2"); %>').replace(/&#62/g, ">").replace(/&#60/g, "<")
		else
			return decodeURIComponent('<% nvram_char_to_ascii("", "MULTIFILTER_MACFILTER_DAYTIME"); %>').replace(/&#62/g, ">").replace(/&#60/g, "<")
	})()
};
function popClientListEditTable(event) {
	var mac = event.data.mac;
	var obj = event.data.obj;
	var name = event.data.name;
	var ip = event.data.ip;
	var callBack = event.data.callBack;
	var panel_block_top_value = event.data.adjust_panel_block_top;
	var adv_setting = event.data.adv_setting;
	if(mac != "") {
		var isMacAddr = mac.split(":");
		if(isMacAddr.length != 6)
			return false;
	}
	else 
		return false;

	card_client_variable.firstTimeOpenBlock = false;
	mac = mac.toUpperCase();
	var clientInfo = clientList[mac];
	if(clientInfo == undefined) {
		clientInfo = new setClientAttr();
		clientInfo.type = "0";
		clientInfo.name = "New device";
		clientInfo.mac = mac;
		clientInfo.defaultType = "0";
	}
	if(name != "" && name != undefined) {
		clientInfo.nickName = name;
	}
	if(ip != "" && ip != undefined)
		clientInfo.ip = ip;	

	if(obj.className.search("card_clicked") != -1) {
		return true;
	}

	client_hide_flag = false;

	if(document.getElementById("edit_client_block") != null) {
		removeElement(document.getElementById("edit_client_block"));
	}

	var divObj = document.createElement("div");
	divObj.setAttribute("id","edit_client_block");
	divObj.className = "clientlist_content";

	var code = "";
	code += '<table class="card_table" align="center" cellpadding="5" cellspacing="0" title="">';
	code += '<tbody>';

	//device title info. start
	code += '<tr><td colspan="3">';
	code += '<table style="width:100%" cellpadding="0" cellspacing="0">';
	code += '<tr>';
	code += '<td>';
	code += '<div id="card_client_state_div" class="clientState">';
	code += '<span id="card_client_ipMethod" class="ipMethodTag" style="color:#FFFFFF;margin-right:5px;"></span>';
	code += '<span id="card_client_login" class="ipMethodTag" style="color:#FFFFFF;margin-right:5px;"></span>';
	code += '<span id="card_client_printer" class="ipMethodTag" style="color:#FFFFFF;margin-right:5px;"></span>';
	code += '<span id="card_client_iTunes" class="ipMethodTag" style="color:#FFFFFF;margin-right:5px;"></span>';
	code += '<span id="card_client_opMode" class="ipMethodTag" style="color:#FFFFFF;margin-right:5px;"></span>';
	code += '<span id="card_client_sdnIdx" class="ipMethodTag" style="color:#FFFFFF;margin-right:5px;"></span>';
	code += '</div>';
	code += '<div id="card_client_interface" style="height:28px;width:28px;float:right;"></div>';
	code += '</td>';
	code += '</tr>';
	code += '</table>';
	code += '</td></tr>';
	//device title info. end

	code += '<tr><td colspan="3"><div class="clientList_line"></div></td></tr>';

	//device icon and device info. start
	code += '<tr>';
	code += '<td style="text-align:center;vertical-align:top;width:85px;">';
	if(!isSupport("sfp4m"))
		code += '<div id="card_client_preview_icon" class="client_preview_icon" title="Change client icon" onclick="card_show_custom_image();">';
	else
		code += '<div id="card_client_preview_icon" class="client_preview_icon" title="Change client icon">';
	code += '<div id="card_client_image" style="width:85px;height:85px;margin:0 auto;cursor:pointer;"></div>';
	code += '</div>';
	if(!isSupport("sfp4m")) {
		code += '<div class="changeClientIcon">';
		code += '<span title="Change to default client icon" onclick="card_setDefaultIcon();"><#CTL_Default#></span>';
		code += '<span id="card_changeIconTitle" title="Change client icon" style="margin-left:10px;" onclick="card_show_custom_image();"><#CTL_Change#></span>';
		code += '</div>';
	}
	code += '</td>';

	code += `
	  <td style="vertical-align:top;text-align:center;">
		<div style="display: flex; flex-direction: column; gap:10px;">
			<div style="display: flex; justify-content: space-between; align-items: center;">
				<div class="clientTitle">
				  <#Clientlist_name#>
				</div>
				<div>
					<input id="card_client_name" name="card_client_name" type="text" value="" class="input_32_table" maxlength="32" style="width:290px;">
				</div>
			</div>
			<div style="display: flex; justify-content: space-between; align-items: center;">
				<div class="clientTitle">
				  IP
				</div>
				<div>
					<input id="card_client_ipaddr_field_orig" type="hidden" value="" disabled="">
					<input id="card_client_ipaddr_field" type="text" value="" class="input_32_table client_input_text_disabled" disabled>
				</div>
			</div>
			<div style="display: flex; justify-content: space-between; align-items: center;">
				<div class="clientTitle">
				  <#IPv6_wan_addr#>
				</div>
				<div>
					<input id="card_client_ip6addr_prefix_field" type="text" value="" class="input_32_table client_input_text_disabled" disabled>
				</div>
			</div>
			<div style="display: flex; justify-content: space-between; align-items: center;">
				<div class="clientTitle">
				  WAN IPv6 Link-Local
				</div>
				<div>
					<input id="card_client_ip6addr_field" type="text" value="" class="input_32_table client_input_text_disabled" disabled>
				</div>
			</div>
			<div style="display: flex; justify-content: space-between; align-items: center;">
				<div class="clientTitle">
				  MAC
				</div>
				<div>
					<input id="card_client_macaddr_field" type="text" value="" class="input_32_table client_input_text_disabled" disabled>
				</div>
			</div>
			<div style="display: flex; justify-content: space-between; align-items: center;">
				<div class="clientTitle">
				  <#Clientlist_device#>
				</div>
				<div>
					<input id="card_client_manufacturer_field" type="text" value="Loading manufacturer.." class="input_32_table client_input_text_disabled" disabled>
				</div>
			</div>
		</div>
	  </td>
	`;
	code += '</tr>';
	//device icon and device info. end

	//device icon list start
	code += '<tr>';
	code += '<td colspan="3">';
	code += '<div id="card_custom_image" class="custom_icon_list_bg" style="display:none;"></div>';
	code += '</td>';
	code += '</tr>';
	//device icon list end

	//adv setting start
	if(adv_setting) {
		var block_internet_hint = "Enable this button to block this device to access internet.";/* untranslated */
		var time_scheduling_hint = "<#ParentalCtrl_Desc_TS#>";
		var ip_binding_hint = "Enable this button to bind specific IP with MAC Address of this device.";/* untranslated */
		var internetTimeScheduling_title = (isSupport("bwdpi")) ? "<#Time_Scheduling#>" : "<#Parental_Control#>";
		code += '<tr id="tr_adv_setting">';
		code += '<td colspan="3">';
		code += '<div class="clientList_line"></div>';
		code += '<div style="height:33px;width:100%;margin:5px 0;">';
		code += '<div style="width:65%;float:left;line-height:33px;">';
		code += "<span onmouseover='return overlib(\"" + htmlEnDeCode.htmlEncode(block_internet_hint) +"\");' onmouseout='return nd();'><#Clientlist_block_internet#></span>";
		code += '</div>';
		code += '<div class="left" style="cursor:pointer;float:right;" id="card_radio_BlockInternet_enable"></div>';
		code += '</div>';
		code += '<div class="clientList_line"></div>';
		code += '<div id="div_card_time_scheduling" style="height:33px;width:100%;margin:5px 0;">';
		code += '<div style="width:65%;float:left;line-height:33px;">';
		code += "<span onmouseover='return overlib(\"" + htmlEnDeCode.htmlEncode(time_scheduling_hint) + "\");' onmouseout='return nd();'>" + internetTimeScheduling_title + "</span>";
		code += '</div>';
		code += '<div align="center" class="left" style="cursor:pointer;float:right;" id="card_radio_TimeScheduling_enable"></div>';
		code += '<div id="card_internetTimeScheduling" class="internetTimeEdit" style="float:right;margin-right:10px;" title="<#Time_Scheduling#>"></div>';
		code += '</div>';
		code += '<div class="clientList_line"></div>';
		code += '<div id="div_card_ipmac_binding" style="height:33px;width:100%;margin:5px 0;">';
		code += '<div style="width:65%;float:left;line-height:33px;">';
		code += "<span onmouseover='return overlib(\"" + htmlEnDeCode.htmlEncode(ip_binding_hint) +"\");' onmouseout='return nd();'><#Clientlist_IPMAC_Binding#></span>";
		code += '</div>';
		code += '<div align="center" class="left" style="cursor:pointer;float:right;" id="card_radio_IPBinding_enable" ></div>';
		code += '</div>';
		code += '<div class="clientList_line"></div>';
		code += '</td>';
		code += '</tr>';
	}
	//adv setting end

	code += '<tr>';
	code += '<td colspan="3" style="text-align: center;">';
    code += '<div style="display:flex;justify-content:center;align-items:center;">';
    code += '<input class="button_gen" type="button" style="margin-right:5px;" onclick="card_closeClientListView();" value="<#CTL_Cancel#>">';
    code += '<input id="card_client_confirm" class="button_gen" type="button" value="<#CTL_apply#>">';
    code += '<img id="card_client_loadingIcon" style="margin-left:5px;width:24px;height:24px;display:none;" src="/images/InternetScan.gif">';
    code += '</div>'
	code += '</td>';
	code += '</tr>';
	code += '</tbody></table>';

	divObj.innerHTML = code;
	obj.parentNode.appendChild(divObj);

	$('#edit_client_block').find('#card_client_confirm').unbind("click");
	$('#edit_client_block').find('#card_client_confirm').click(
		{"callBack": callBack, "adv_setting": adv_setting},
		card_confirm);

	if(isSupport("force_roaming") && isSupport("sta_ap_bind") && clientInfo.amesh_bind_mac != "")
		$('#edit_client_block').find('.client_preview_icon').addClass("client_bind_icon");

	//Clear the last record clicked obj
	if(temp_clickedObj != null) {
		if(temp_clickedObj.className.search("clientIcon") != -1) {
			temp_clickedObj.className = temp_clickedObj.className.replace("clientIcon_clicked","clientIcon");
			temp_clickedObj.className = temp_clickedObj.className.replace(" card_clicked", "");
		}
		else if(temp_clickedObj.className.search("vendorIcon") != -1) {
			temp_clickedObj.className = temp_clickedObj.className.replace("vendorIcon_clicked","vendorIcon");
			temp_clickedObj.className = temp_clickedObj.className.replace(" card_clicked", "");
		}
		temp_clickedObj = null;
	}
	temp_clickedObj = obj;
	if(obj.className.search("clientIcon") != -1) {
		obj.className = obj.className.replace("clientIcon","clientIcon_clicked");
		obj.className = obj.className  + " card_clicked";
	}
	else if(obj.className.search("vendorIcon") != -1) {
		obj.className = obj.className.replace("vendorIcon","vendorIcon_clicked");
		obj.className = obj.className  + " card_clicked";
	}
	
	fadeIn(document.getElementById("edit_client_block"));
	if(panel_block_top_value)
		adjust_panel_block_top("edit_client_block", panel_block_top_value);
	document.body.onclick = function() {hide_edit_client_block();}
	document.getElementById("edit_client_block").onclick = function() {show_edit_client_block();}

	//build device icon list start
	if(!isSupport("sfp4m")) {
		custom_icon_list_api.paramObj.container = $('#edit_client_block').find("#card_custom_image");
		custom_icon_list_api.paramObj.source = "local";
		custom_icon_list_api.paramObj.select_icon_callBack = card_select_custom_icon;
		custom_icon_list_api.paramObj.upload_callBack = previewCardUploadIcon;
		custom_icon_list_api.gen_component(custom_icon_list_api.paramObj);
		$.getJSON("/ajax/extend_custom_icon.json",
			function(data){
				custom_icon_list_api.paramObj.container = $('#edit_client_block').find("#card_custom_image");
				custom_icon_list_api.paramObj.source = "cloud";
				custom_icon_list_api.paramObj.db = data;
				custom_icon_list_api.paramObj.select_icon_callBack = card_select_custom_icon;
				custom_icon_list_api.gen_component(custom_icon_list_api.paramObj);
			}
		);
	}
	//build device icon list end

	//settup value
	//initial state start
	document.getElementById("card_client_preview_icon").ondrop = null;
	//initial state end

	//device title info. start
	document.getElementById("card_client_name").value = (clientInfo.nickName == "") ? clientInfo.name : clientInfo.nickName;

	document.getElementById("card_client_ipMethod").style.display = "none";
	document.getElementById("card_client_login").style.display = "none";
	document.getElementById("card_client_printer").style.display = "none";
	document.getElementById("card_client_iTunes").style.display = "none";
	document.getElementById("card_client_opMode").style.display = "none";
	document.getElementById("card_client_sdnIdx").style.display = "none";
	if(clientInfo.isOnline) {
		var rssi_t = 0;
		var connectModeTip = "";
		var clientIconHtml = "";
		if(clientInfo.isWL == "0") {
			rssi_t = "wired";
			connectModeTip = "<#tm_wired#>";
		}
		else {
			rssi_t = client_convRSSI(clientInfo.rssi);
			switch(rssi_t) {
				case 1:
					connectModeTip = "<#Radio#>: <#PASS_score1#>\n";
					break;
				case 2:
					connectModeTip = "<#Radio#>: <#PASS_score2#>\n";
					break;
				case 3:
					connectModeTip = "<#Radio#>: <#PASS_score3#>\n";
					break;
				case 4:
					connectModeTip = "<#Radio#>: <#PASS_score4#>\n";
					break;
			}
			if(isSupport("stainfo")) {
				if(clientInfo.curTx != "")
					connectModeTip += "Tx Rate: " + clientInfo.curTx + "\n"; /*untranslated*/
				if(clientInfo.curRx != "")
					connectModeTip += "Rx Rate: " + clientInfo.curRx + "\n"; /*untranslated*/
				connectModeTip += "<#Access_Time#>: " + clientInfo.wlConnectTime + "";
			}
		}

		if(sw_mode != 4){
			var radioIcon_css = "radioIcon";
			if((clientInfo.isGN != "" && clientInfo.isGN != undefined) || (isSupport("mtlancfg") && clientInfo.sdn_idx > 0)){
				radioIcon_css += " GN";
			}
			clientIconHtml += '<div class="' + radioIcon_css + ' radio_' + rssi_t +'" title="' + connectModeTip + '"></div>';
			if(clientInfo.isWL != 0 || (isSupport("mtlancfg") && clientInfo.sdn_idx > 0)){
				var bandClass = (navigator.userAgent.toUpperCase().match(/CHROME\/([\d.]+)/)) ? "band_txt_chrome" : "band_txt";
				let band_text = isWL_map[clientInfo.isWL]["text"];
				if(isSupport("mlo") && (clientInfo.mlo == "1")) band_text = `MLO`;
				clientIconHtml += `<div class="band_block"><span class="${bandClass}" style="color:#000000;">${band_text}</span></div>`;
			}
			document.getElementById('card_client_interface').innerHTML = clientIconHtml;
			document.getElementById("card_client_interface").title = connectModeTip;
		}
	}

	if(clientInfo.isOnline) {
		if(sw_mode == "1") {
			document.getElementById("card_client_ipMethod").style.display = "";
			document.getElementById("card_client_ipMethod").innerHTML = clientInfo.ipMethod;
			document.getElementById("card_client_ipMethod").onmouseover = function() {return overlib(ipState[clientInfo.ipMethod]);};
			document.getElementById("card_client_ipMethod").onmouseout = function() {nd();};
		}
	}
	else {
		document.getElementById("card_client_ipMethod").style.display = "";
		document.getElementById("card_client_ipMethod").innerHTML = "<#Clientlist_OffLine#>";
		document.getElementById("card_client_ipMethod").onmouseover = function() {return overlib(ipState["OffLine"]);};
		document.getElementById("card_client_ipMethod").onmouseout = function() {nd();};
		document.getElementById("card_client_interface").style.display = "none";
	}
	if(clientInfo.isLogin) {
		document.getElementById("card_client_login").style.display = "";
		document.getElementById("card_client_login").innerHTML = "<#Clientlist_Logged_In_User#>";
	}
	if(clientInfo.isPrinter) {
		document.getElementById("card_client_printer").style.display = "";
		document.getElementById("card_client_printer").innerHTML = "<#Clientlist_Printer#>";
	}
	if(clientInfo.isITunes) {
		document.getElementById("card_client_iTunes").style.display = "";
		document.getElementById("card_client_iTunes").innerHTML = "iTunes"; /*untranslated*/
	}
	if(clientInfo.opMode != 0) {
		var opModeDes = ["none", "<#wireless_router#>", "<#OP_RE_item#>", "<#OP_AP_item#>", "<#OP_MB_item#>"];
		document.getElementById("card_client_opMode").style.display = "";
		document.getElementById("card_client_opMode").innerHTML = opModeDes[clientInfo.opMode];
	}
	if(clientInfo.sdn_idx > 0) {
		document.getElementById('card_client_sdnIdx').style.display = "";
		document.getElementById('card_client_sdnIdx').innerHTML = "SDN " + sdn_rl_for_clientlist[clientInfo.sdn_idx].apg_rl.ssid;
		$('#tr_adv_setting').hide();
	}else{
		$('#tr_adv_setting').show();
	}
	//device title info. end

	//device icon and device info. start
	document.getElementById("card_client_ipaddr_field_orig").value = clientInfo.ip;
	document.getElementById("card_client_ipaddr_field").value = clientInfo.ip;
	document.getElementById("card_client_ip6addr_field").value = clientInfo.ip6;
	document.getElementById("card_client_ip6addr_prefix_field").value = clientInfo.ip6_prefix;
	document.getElementById("card_client_macaddr_field").value = clientInfo.mac;
	if (clientInfo.ip6 == '' || clientInfo.ip6 == undefined) {
		document.getElementById('card_client_ip6addr_field').parentNode.parentNode.style.display = "none";
	}
	if (clientInfo.ip6_prefix == '' || clientInfo.ip6_prefix == undefined) {
		document.getElementById('card_client_ip6addr_prefix_field').parentNode.parentNode.style.display = "none";
	}
	select_image(clientInfo);
	if(card_client_variable.manual_dhcp_list[clientInfo.mac] != undefined) {
		var client_manual_ip = card_client_variable.manual_dhcp_list[clientInfo.mac].ip;
		//handle device offine but dhcp had been setted.
		if(clientInfo.ip == "offline")
			document.getElementById("card_client_ipaddr_field").value = client_manual_ip;
	}

	var deviceTitle = (clientInfo.dpiDevice == "") ? clientInfo.vendor : clientInfo.dpiDevice;
	if(deviceTitle == undefined || deviceTitle == "") {
		setTimeout(function(){
			if('<% nvram_get("x_Setting"); %>' == '1' && wanConnectStatus && clientInfo.internetState)
				oui_query_card(clientInfo.mac);
		}, 1000);
	}
	else {
		document.getElementById("card_client_manufacturer_field").value = deviceTitle;
		document.getElementById("card_client_manufacturer_field").title = "";
		if(deviceTitle.length > 38) {
			document.getElementById("card_client_manufacturer_field").value = deviceTitle.substring(0, 36) + "..";
			document.getElementById("card_client_manufacturer_field").title = deviceTitle;
		}
	}
	//device icon and device info. end

	//adv setting start
	if(adv_setting) {
		var manual_dhcp_list_num = Object.keys(card_client_variable.manual_dhcp_list).length;
		var client_MULTIFILTER_num = (card_client_variable.MULTIFILTER_MAC == "") ? 0 : card_client_variable.MULTIFILTER_ENABLE.split(">").length;
		$("#edit_client_block #card_client_ipaddr_field").prop("disabled", false);
		$("#edit_client_block #card_client_ipaddr_field").removeClass("client_input_text_disabled");
		$("#edit_client_block #card_client_ipaddr_field").css("width", "290px");
		$("#edit_client_block #card_client_ipaddr_field").unbind("keypress");
		$("#edit_client_block #card_client_ipaddr_field").keypress(function(){
			if(!card_client_variable.ipBindingFlag) {
				$("#edit_client_block #card_radio_IPBinding_enable").click();
				card_client_variable.ipBindingFlag = true;
			}
		});
		$("#edit_client_block #card_client_ipaddr_field").unbind("blur");
		$("#edit_client_block #card_client_ipaddr_field").blur(function(){
			if(!card_client_variable.ipBindingFlag) {
				$("#edit_client_block #card_radio_IPBinding_enable").click();
				card_client_variable.ipBindingFlag = true;
			}
		});

		var setRadioControl = function (state, mode, mac) {
			const manually_dhcp_maximum  = (isSupport("MaxRule_extend_limit") == 0) ? 64: isSupport("MaxRule_extend_limit");
			const parentctrl_maximum = (isSupport("MaxRule_parentctrl") == 0) ? 16 : isSupport("MaxRule_parentctrl");
			switch (mode) {
				case "ipBinding" :
					$('#edit_client_block #card_radio_IPBinding_enable').iphoneSwitch(state,
						function(){
							if(card_client_variable.manual_dhcp_list[mac] == undefined) {
								if(manual_dhcp_list_num >= manually_dhcp_maximum) {
									if(confirm(stringSafeGet("<#Clientlist_IPMAC_Binding_max#>".replace("64", manually_dhcp_maximum)))) {
										location.href = "/Advanced_DHCP_Content.asp" ;
									}
									else {
										$('#edit_client_block #card_client_ipaddr_field').val($('#edit_client_block #card_client_ipaddr_field_orig').val());
										setRadioControl(0, "ipBinding", mac);
										card_client_variable.ipBindingFlag = false;
										return false;
									}
								}
							}
							card_client_variable.ipBindingFlag = true;
						},
						function(){
							$('#edit_client_block #card_client_ipaddr_field').val($('#edit_client_block #card_client_ipaddr_field_orig').val());
							card_client_variable.ipBindingFlag = false;
						}
					);
					break;
				case "blockInternet" :
					$('#edit_client_block #card_radio_BlockInternet_enable').iphoneSwitch(state,
						function(){
							if(card_client_variable.MULTIFILTER_MAC.search(mac) == -1) {
								if(client_MULTIFILTER_num >= parentctrl_maximum) {
									if(confirm(stringSafeGet("<#Clientlist_block_internet_max#>".replace("16", parentctrl_maximum)))) {
										location.href = "/ParentalControl.asp" ;
									}
									else {
										setRadioControl(0, "blockInternet", mac);
										return false;
									}
								}
							}
							setRadioControl(0, "timeScheduling", mac);
							card_client_variable.timeSchedulingFlag = false;
							card_client_variable.blockInternetFlag = true;
						},
						function(){
							card_client_variable.timeSchedulingFlag = false;
							card_client_variable.blockInternetFlag = false;
						}
					);
					break;
				case "timeScheduling" :
					$('#edit_client_block #card_radio_TimeScheduling_enable').iphoneSwitch(state,
						function(){
							if(card_client_variable.MULTIFILTER_MAC.search(mac) == -1) {
								if(client_MULTIFILTER_num >= parentctrl_maximum) {
									if(confirm(stringSafeGet("<#Clientlist_block_internet_max#>".replace("16", parentctrl_maximum)))) {
										location.href = "/ParentalControl.asp" ;
									}
									else {
										setRadioControl(0, "timeScheduling", mac);
										return false;
									}
								}
							}
							setRadioControl(0, "blockInternet", mac);
							card_client_variable.timeSchedulingFlag = true;
							card_client_variable.blockInternetFlag = false;
						},
						function(){
							card_client_variable.timeSchedulingFlag = false;
							card_client_variable.blockInternetFlag = false;
						}
					);
					break;
			}
		};
		$("#edit_client_block #card_internetTimeScheduling").hide();
		$("#edit_client_block #card_internetTimeScheduling").unbind("click");
		$("#edit_client_block #card_internetTimeScheduling").click(function(){
			redirectTimeScheduling($("#edit_client_block #card_client_macaddr_field").val());
		});

		if(card_client_variable.manual_dhcp_list[clientInfo.mac] != undefined) { //check mac>ip is combination the the ipLockIcon is manual
			setRadioControl(1, "ipBinding", clientInfo.mac);
			card_client_variable.ipBindingFlag = true;
		}
		else {
			setRadioControl(0, "ipBinding", clientInfo.mac);
			card_client_variable.ipBindingFlag = false;
		}

		switch(clientInfo.internetMode) {
			case "allow" :
				setRadioControl(0, "blockInternet", clientInfo.mac);
				setRadioControl(0, "timeScheduling", clientInfo.mac);
				card_client_variable.timeSchedulingFlag = false;
				card_client_variable.blockInternetFlag = false;
				break;
			case "block" :
				setRadioControl(1, "blockInternet", clientInfo.mac);
				setRadioControl(0, "timeScheduling", clientInfo.mac);
				card_client_variable.timeSchedulingFlag = false;
				card_client_variable.blockInternetFlag = true;
				break;
			case "time" :
				setRadioControl(0, "blockInternet", clientInfo.mac);
				setRadioControl(1, "timeScheduling", clientInfo.mac);
				$("#edit_client_block #card_internetTimeScheduling").show();
				card_client_variable.timeSchedulingFlag = true;
				card_client_variable.blockInternetFlag = false;
				break;
		}
	}
	//adv setting end

	//setting user upload icon attribute start.
	//1.check rc_support
	if(isSupport("usericon")) {
		//2.check browswer support File Reader and Canvas or not.
		if(isSupportFileReader() && isSupportCanvas()) {
			//Setting drop event
			var holder = document.getElementById("card_client_preview_icon");
			holder.ondragover = function () { return false; };
			holder.ondragend = function () { return false; };
			holder.ondrop = function (e) {
				e.preventDefault();
				var userIconLimitFlag = userIconNumLimit(document.getElementById("card_client_macaddr_field").value);
				if(userIconLimitFlag) {	//not over 100	
					var file = e.dataTransfer.files[0];
					//check image
					if(file.type.search("image") != -1) {
						var reader = new FileReader();
						reader.onload = function (event) {
							var img = document.createElement("img");
							img.src = event.target.result;
							img.className="clientIcon_no_hover";
							var mimeType = img.src.split(",")[0].split(":")[1].split(";")[0];
							let canvas = document.createElement('canvas');
							canvas.width=85;
							canvas.height=85;
							var ctx = canvas.getContext("2d");
							ctx.clearRect(0,0,85,85);
							$("#card_client_image").empty();
							$("#card_client_image").append(img);
							setTimeout(function() {
								ctx.drawImage(img, 0, 0, 85, 85);
								var dataURL = canvas.toDataURL(mimeType);
								card_client_variable.userIconBase64 = dataURL;
							}, 100); //for firefox FPS(Frames per Second) issue need delay
						};
						reader.readAsDataURL(file);
						card_client_variable.userUploadFlag = true;
						return false;
					}
					else {
						alert("<#Setting_upload_hint#>");
						return false;
					}
				}
				else {	//over 100 then let usee select delete icon or nothing
					showUploadIconList();
				}
			};
		} 
	}
	//setting user upload icon attribute end.

	//form
	if(document.getElementById("card_clientlist_form") != null) {
		removeElement(document.getElementById("card_clientlist_form"));
	}
	var formHTML = "";
	var formObj = document.createElement("form");
	document.body.appendChild(formObj);
	formObj.method = "POST";
	formObj.setAttribute("id","card_clientlist_form");
	formObj.setAttribute("name","card_clientlist_form");
	formObj.action = "/start_apply2.htm";
	formObj.target = "hidden_frame";

	formHTML += '<input type="hidden" name="modified" value="0">';
	formHTML += '<input type="hidden" name="flag" value="background">';
	formHTML += '<input type="hidden" name="action_mode" value="apply">';
	formHTML += '<input type="hidden" name="action_script" value="saveNvram">';
	formHTML += '<input type="hidden" name="action_wait" value="1">';
	formHTML += '<input type="hidden" name="custom_clientlist" value="">';
	formHTML += '<input type="hidden" name="custom_usericon" value="">';
	formHTML += '<input type="hidden" name="usericon_mac" value="" disabled>';
	formHTML += '<input type="hidden" name="custom_usericon_del" value="" disabled>';
	if(adv_setting) {
		formHTML += '<input type="hidden" name="dhcp_staticlist" value="" disabled>';
		formHTML += '<input type="hidden" name="dhcp_static_x" value="<% nvram_get("dhcp_static_x"); %>" disabled>';
		formHTML += '<input type="hidden" name="MULTIFILTER_ALL" value="<% nvram_get("MULTIFILTER_ALL"); %>" disabled>';
		formHTML += '<input type="hidden" name="MULTIFILTER_ENABLE" value="" disabled>';
		formHTML += '<input type="hidden" name="MULTIFILTER_MAC" value="" disabled>';
		formHTML += '<input type="hidden" name="MULTIFILTER_DEVICENAME" value="" disabled>';
		if(isSupport("PC_SCHED_V3"))
			formHTML += '<input type="hidden" name="MULTIFILTER_MACFILTER_DAYTIME_V2" value="" disabled>';
		else
			formHTML += '<input type="hidden" name="MULTIFILTER_MACFILTER_DAYTIME" value="" disabled>';
	}

	formObj.innerHTML = formHTML;
	card_client_variable.firstTimeOpenBlock = true;
}
function previewCardUploadIcon($obj) {
	var userIconLimitFlag = userIconNumLimit(document.getElementById("card_client_macaddr_field").value);

	if(userIconLimitFlag) {	//not over 100
		var checkImageExtension = function (imageFileObject) {
		var  picExtension= /\.(jpg|jpeg|gif|png|bmp|ico)$/i;  //analy extension
			if (picExtension.test(imageFileObject)) 
				return true;
			else
				return false;
		};

		//1.check image extension
		if (!checkImageExtension($obj.val()))
			alert("<#Setting_upload_hint#>");
		else {
			//2.Re-drow image
			var fileReader = new FileReader(); 
			fileReader.onload = function (fileReader) {
				var img = document.createElement("img");
				img.src = fileReader.target.result;
				img.className="clientIcon_no_hover";
				var mimeType = img.src.split(",")[0].split(":")[1].split(";")[0];
				let canvas = document.createElement('canvas');
				canvas.width=85;
				canvas.height=85;
				var ctx = canvas.getContext("2d");
				ctx.clearRect(0,0,85,85);
				$("#card_client_image").empty();
				$("#card_client_image").append(img);
				setTimeout(function() {
					ctx.drawImage(img, 0, 0, 85, 85);
					var dataURL = canvas.toDataURL(mimeType);
					card_client_variable.userIconBase64 = dataURL;
				}, 100); //for firefox FPS(Frames per Second) issue need delay
			}
			fileReader.readAsDataURL($obj.prop("files")[0]);
			card_client_variable.userUploadFlag = true;
		}
	}
	else {	//over 100 then let usee select delete icon or nothing
		showUploadIconList();
	}
}
function card_show_custom_image(flag) {
	if(!slideFlag) {
		var display_state = document.getElementById("card_custom_image").style.display;
		if(display_state == "none") {
			slideFlag = true;
			slideDown("card_custom_image", 500);
			document.getElementById("card_changeIconTitle").innerHTML = "<#CTL_close#>";
		}
		else {
			slideFlag = true;
			slideUp("card_custom_image", 500);
			document.getElementById("card_changeIconTitle").innerHTML = "<#CTL_Change#>";
		}
	}
}
function card_confirm(event) {
	var callBack = event.data.callBack;
	var adv_setting = event.data.adv_setting;
	var validClientListForm = function() {
		if(adv_setting){
			var validateIpRange = function(ip_obj){
				var retFlag = 1
				var ip_num = inet_network(ip_obj.value);
				if(ip_num <= 0){
					alert(ip_obj.value+" <#JS_validip#>");
					ip_obj.value = $('#edit_client_block #card_client_ipaddr_field_orig').val();
					ip_obj.focus();
					retFlag = 0;
				}
				else if(card_client_variable.ipBindingFlag && (ip_num <= getSubnet('<% nvram_get("lan_ipaddr"); %>', '<% nvram_get("lan_netmask"); %>', "head") ||
					 ip_num >= getSubnet('<% nvram_get("lan_ipaddr"); %>', '<% nvram_get("lan_netmask"); %>', "end"))){
					alert(ip_obj.value+" <#JS_validip#>");
					ip_obj.value = $('#edit_client_block #card_client_ipaddr_field_orig').val();
					ip_obj.focus();
					retFlag = 0;
				}
				else if(!validator.validIPForm($('#edit_client_block #card_client_ipaddr_field')[0], 0)){
					ip_obj.value = $('#edit_client_block #card_client_ipaddr_field_orig').val();
					ip_obj.focus();
					retFlag = 0;
				}

				Object.keys(card_client_variable.manual_dhcp_list).forEach(function(key) {
					var existMac = key;
					var existIP = card_client_variable.manual_dhcp_list[existMac].ip;
					if(existIP == $('#edit_client_block #card_client_ipaddr_field').val()) {
						if(existMac != $('#edit_client_block #card_client_macaddr_field').val()) {
							alert("<#JS_duplicate#>");
							ip_obj.value = $('#edit_client_block #card_client_ipaddr_field_orig').val();
							ip_obj.focus();
							retFlag = 0;
						}
					}
				});
				return retFlag;
			};
			if(validateIpRange($("#edit_client_block #card_client_ipaddr_field")[0]) == 0)
				return false;
		}

		document.getElementById("card_client_name").value = document.getElementById("card_client_name").value.trim();
		if(document.getElementById("card_client_name").value.length == 0){
			alert("<#File_Pop_content_alert_desc1#>");
			document.getElementById("card_client_name").focus();
			document.getElementById("card_client_name").select();
			document.getElementById("card_client_name").value = "";
			return false;
		}
		else if(document.getElementById("card_client_name").value.indexOf(">") != -1 || document.getElementById("card_client_name").value.indexOf("<") != -1){
			alert("<#JS_validstr2#> '<', '>'");
			document.getElementById("card_client_name").focus();
			document.getElementById("card_client_name").select();
			document.getElementById("card_client_name").value = "";
			return false;
		}

		if(isSupport("utf8_ssid")){
			var len = validator.lengthInUtf8(document.getElementById('card_client_name').value);
			if(len > 32){
				alert("Username cannot be greater than 32 characters.");/* untranslated */
				document.getElementById('card_client_name').focus();
				document.getElementById('card_client_name').select();
				document.getElementById('card_client_name').value = "";
				return false;
			}
		}
		else if(!validator.haveFullWidthChar(document.getElementById("card_client_name"))) {
			document.getElementById('card_client_name').focus();
			document.getElementById('card_client_name').select();
			document.getElementById('card_client_name').value = "";
			return false;
		}
		return true;
	};

	if(validClientListForm()){
		document.card_clientlist_form.custom_clientlist.disabled = false;
		// customize device name
		var originalCustomListArray = new Array();
		var onEditClient = new Array();
		var clientTypeNum = "";
		if(document.getElementById('card_client_image').className.search("vendorIcon") != -1) {
			clientTypeNum = "0";
		}
		else {
			if($('#card_client_image i').length>0) {
				clientTypeNum = $('#card_client_image i').attr('class').replace("type", "");
			}
		}

		var clientName = document.getElementById("card_client_name").value.trim();
		var clientMac = document.getElementById('card_client_macaddr_field').value.toUpperCase();
		originalCustomListArray = card_client_variable.custom_clientlist.split('<');
		onEditClient[0] = clientName;
		onEditClient[1] = clientMac;
		onEditClient[2] = 0;
		onEditClient[3] = clientTypeNum;
		onEditClient[4] = "";
		onEditClient[5] = "";

		for(var i=0; i<originalCustomListArray.length; i++){
			if(originalCustomListArray[i].split('>')[1] != undefined) {
				if(originalCustomListArray[i].split('>')[1].toUpperCase() == clientMac){
					onEditClient[4] = originalCustomListArray[i].split('>')[4]; // set back callback for ROG device
					onEditClient[5] = originalCustomListArray[i].split('>')[5]; // set back keeparp for ROG device
					var app_group_tag = originalCustomListArray[i].split('>')[6]; // for app group tag
					if(typeof app_group_tag != "undefined")	onEditClient[6] = app_group_tag;
					var app_age_tag = originalCustomListArray[i].split('>')[7]; // for app age tag
					if(typeof app_age_tag != "undefined")	onEditClient[7] = app_age_tag;
					var app_groupid_tag = originalCustomListArray[i].split('>')[8]; // for app groupid tag
					if(typeof app_groupid_tag != "undefined")	onEditClient[8] = app_groupid_tag;
					originalCustomListArray.splice(i, 1); // remove the selected client from original list
				}
			}
		}

		originalCustomListArray.push(onEditClient.join('>'));
		card_client_variable.custom_clientlist = originalCustomListArray.join('<');
		document.card_clientlist_form.custom_clientlist.value = card_client_variable.custom_clientlist;

		if(adv_setting){
			// IP Binding
			var dhcp_staticlist_ori = "";
			var dhcp_staticlist = "";
			Object.keys(card_client_variable.manual_dhcp_list).forEach(function(key) {
				dhcp_staticlist_ori += "<" + key + ">"  + card_client_variable.manual_dhcp_list[key].ip + ">" + card_client_variable.manual_dhcp_list[key].dns;
			});

			if(card_client_variable.ipBindingFlag) {
				if(card_client_variable.manual_dhcp_list[clientMac] == undefined){//new
					var ip = $('#edit_client_block #card_client_ipaddr_field').val();
					var dns = "";
					var item_para = {"ip" : ip, "dns" : dns};
					card_client_variable.manual_dhcp_list[clientMac] = item_para;
				}
				else
					card_client_variable.manual_dhcp_list[clientMac].ip = $('#edit_client_block #card_client_ipaddr_field').val();
			}
			else
				delete card_client_variable.manual_dhcp_list[clientMac];

			Object.keys(card_client_variable.manual_dhcp_list).forEach(function(key) {
				dhcp_staticlist += "<" + key + ">"  + card_client_variable.manual_dhcp_list[key].ip + ">" + card_client_variable.manual_dhcp_list[key].dns;
			});
			if(dhcp_staticlist == dhcp_staticlist_ori || sw_mode != "1"){
				document.card_clientlist_form.action_script.value = "saveNvram";
				document.card_clientlist_form.action_wait.value = "1";
				document.card_clientlist_form.flag.value = "background";
				document.card_clientlist_form.dhcp_staticlist.disabled = true;
				document.card_clientlist_form.dhcp_static_x.disabled = true;
			}
			else {
				document.card_clientlist_form.action_script.value = "restart_net_and_phy";
				document.card_clientlist_form.action_wait.value = "35";
				document.card_clientlist_form.flag.value = "";
				document.card_clientlist_form.dhcp_staticlist.disabled = false;
				document.card_clientlist_form.dhcp_staticlist.value = dhcp_staticlist;
				document.card_clientlist_form.dhcp_static_x.value = 1;
				document.card_clientlist_form.dhcp_static_x.disabled = false;
			}

			//Time Scheduling
			document.card_clientlist_form.MULTIFILTER_ENABLE.value = card_client_variable.MULTIFILTER_ENABLE;
			document.card_clientlist_form.MULTIFILTER_MAC.value = card_client_variable.MULTIFILTER_MAC;
			document.card_clientlist_form.MULTIFILTER_DEVICENAME.value = card_client_variable.MULTIFILTER_DEVICENAME;
			if(isSupport("PC_SCHED_V3"))
				document.card_clientlist_form.MULTIFILTER_MACFILTER_DAYTIME_V2.value = card_client_variable.MULTIFILTER_MACFILTER_DAYTIME;
			else
				document.card_clientlist_form.MULTIFILTER_MACFILTER_DAYTIME.value = card_client_variable.MULTIFILTER_MACFILTER_DAYTIME;

			if(document.card_clientlist_form.MULTIFILTER_MAC.value.indexOf(clientMac) == -1){//new rule
				if(card_client_variable.timeSchedulingFlag || card_client_variable.blockInternetFlag) {
					if(document.card_clientlist_form.MULTIFILTER_MAC.value == "") {
						if(card_client_variable.timeSchedulingFlag)
							document.card_clientlist_form.MULTIFILTER_ENABLE.value = "1";
						else if(card_client_variable.blockInternetFlag)
							document.card_clientlist_form.MULTIFILTER_ENABLE.value = "2";
						document.card_clientlist_form.MULTIFILTER_MAC.value = clientMac;
						document.card_clientlist_form.MULTIFILTER_DEVICENAME.value = clientName;
						if(isSupport("PC_SCHED_V3"))
							document.card_clientlist_form.MULTIFILTER_MACFILTER_DAYTIME_V2.value = "W03E21000700<W04122000800";
						else
							document.card_clientlist_form.MULTIFILTER_MACFILTER_DAYTIME.value = "<";
					}
					else {
						document.card_clientlist_form.MULTIFILTER_ENABLE.value += ">";
						if(card_client_variable.timeSchedulingFlag)
							document.card_clientlist_form.MULTIFILTER_ENABLE.value += "1";
						else if(card_client_variable.blockInternetFlag)
							document.card_clientlist_form.MULTIFILTER_ENABLE.value += "2";
						document.card_clientlist_form.MULTIFILTER_MAC.value += ">";
						document.card_clientlist_form.MULTIFILTER_MAC.value += clientMac;
						document.card_clientlist_form.MULTIFILTER_DEVICENAME.value += ">";
						document.card_clientlist_form.MULTIFILTER_DEVICENAME.value += clientName;
						if(isSupport("PC_SCHED_V3"))
							document.card_clientlist_form.MULTIFILTER_MACFILTER_DAYTIME_V2.value += ">W03E21000700<W04122000800";
						else
							document.card_clientlist_form.MULTIFILTER_MACFILTER_DAYTIME.value += "><";
					}
				}
			}
			else {//exist rule
				document.card_clientlist_form.MULTIFILTER_MAC.value.split(">").forEach(function(element, index){
					if(element.indexOf(clientMac) != -1){
						var tmpArray = document.card_clientlist_form.MULTIFILTER_ENABLE.value.split(">");
						tmpArray[index] = 0;
						if(card_client_variable.timeSchedulingFlag)
							tmpArray[index] = 1;
						else if(card_client_variable.blockInternetFlag)
							tmpArray[index] = 2;
						document.card_clientlist_form.MULTIFILTER_ENABLE.value = tmpArray.join(">");
					}
				})
			}

			var turnOnTimeScheduling = false;
			if(document.card_clientlist_form.MULTIFILTER_ALL.value == "0" && (card_client_variable.timeSchedulingFlag || card_client_variable.blockInternetFlag))
				turnOnTimeScheduling = true;

			if((document.card_clientlist_form.MULTIFILTER_MAC.value == card_client_variable.MULTIFILTER_MAC && 
				document.card_clientlist_form.MULTIFILTER_ENABLE.value == card_client_variable.MULTIFILTER_ENABLE) && 
				!turnOnTimeScheduling ||
				sw_mode != "1"){
				document.card_clientlist_form.MULTIFILTER_ALL.disabled = true;
				document.card_clientlist_form.MULTIFILTER_ENABLE.disabled = true;
				document.card_clientlist_form.MULTIFILTER_MAC.disabled = true;
				document.card_clientlist_form.MULTIFILTER_DEVICENAME.disabled = true;
				if(isSupport("PC_SCHED_V3"))
					document.card_clientlist_form.MULTIFILTER_MACFILTER_DAYTIME_V2.disabled = true;
				else
					document.card_clientlist_form.MULTIFILTER_MACFILTER_DAYTIME.disabled = true;
			}
			else {
				document.card_clientlist_form.flag.value = "";
				if(document.card_clientlist_form.action_script.value == "restart_net_and_phy") {
					document.card_clientlist_form.action_script.value += ";restart_firewall";
					document.card_clientlist_form.action_wait.value = "35";
				}
				else {
					document.card_clientlist_form.action_script.value = "restart_firewall";
					document.card_clientlist_form.action_wait.value = "1";
					document.card_clientlist_form.flag.value = "background";
				}
				document.card_clientlist_form.MULTIFILTER_ALL.disabled = false;
				document.card_clientlist_form.MULTIFILTER_ALL.value = "1";
				document.card_clientlist_form.MULTIFILTER_ENABLE.disabled = false;
				document.card_clientlist_form.MULTIFILTER_MAC.disabled = false;
				document.card_clientlist_form.MULTIFILTER_DEVICENAME.disabled = false;
				if(isSupport("PC_SCHED_V3"))
					document.card_clientlist_form.MULTIFILTER_MACFILTER_DAYTIME_V2.disabled = false;
				else
					document.card_clientlist_form.MULTIFILTER_MACFILTER_DAYTIME.disabled = false;
			}
			card_client_variable.MULTIFILTER_ENABLE = document.card_clientlist_form.MULTIFILTER_ENABLE.value;
			card_client_variable.MULTIFILTER_MAC = document.card_clientlist_form.MULTIFILTER_MAC.value;
			card_client_variable.MULTIFILTER_DEVICENAME = document.card_clientlist_form.MULTIFILTER_DEVICENAME.value;
			if(isSupport("PC_SCHED_V3"))
				card_client_variable.MULTIFILTER_MACFILTER_DAYTIME = document.card_clientlist_form.MULTIFILTER_MACFILTER_DAYTIME_V2.value;
			else
				card_client_variable.MULTIFILTER_MACFILTER_DAYTIME = document.card_clientlist_form.MULTIFILTER_MACFILTER_DAYTIME.value;
		}

		// handle user image
		document.card_clientlist_form.custom_usericon.disabled = true;
		if(isSupport("usericon")) {
			var clientMac = document.getElementById("card_client_macaddr_field").value.replace(/\:/g, "");
			document.card_clientlist_form.custom_usericon.disabled = false;
			if(card_client_variable.userIconBase64 != "NoIcon" && (card_client_variable.userIconBase64 != card_client_variable.userIconBase64_ori)) {
				if(card_client_variable.userUploadFlag)
					document.card_clientlist_form.custom_usericon.value = clientMac + ">" + card_client_variable.userIconBase64;
				else{
					document.card_clientlist_form.custom_usericon.value = clientTypeNum + ">" + card_client_variable.userIconBase64;
					document.card_clientlist_form.usericon_mac.disabled = false;
					document.card_clientlist_form.usericon_mac.value = clientMac;
				}
			}
			else if(card_client_variable.userIconBase64 == "NoIcon"){
				document.card_clientlist_form.custom_usericon.value = clientMac + ">noupload";
			}
			else{
				document.card_clientlist_form.custom_usericon.disabled = true;
				document.card_clientlist_form.usericon_mac.disabled = true;
			}
		}

		// submit card_clientlist_form
		document.card_clientlist_form.submit();

		// display waiting effect
		document.getElementById("card_client_loadingIcon").style.display = "";
		setTimeout(function(){
			hide_edit_client_block();
			if(card_client_variable.timeSchedulingFlag && $("#edit_client_block #card_internetTimeScheduling").css("display") == "none") { //if the latest internetMode is not time mode, then redirect to ParentalControl
				redirectTimeScheduling($("#edit_client_block #card_client_macaddr_field").val());
			}
			else {
				var updateClientListObj = function () {
					$.ajax({
						url: '/update_clients.asp',
						dataType: 'script',
						error: function(xhr) {
							setTimeout("updateClientListObj();", 1000);
						},
						success: function(response){
							genClientList();

							switch(callBack) {
								case "DHCP" :
									showDropdownClientList('setClientIP', 'mac>ip', 'all', 'ClientList_Block_PC', 'pull_arrow', 'all');
									showdhcp_staticlist();
									break;
								case "WOL" :
									showDropdownClientList('setClientIP', 'mac', 'all', 'ClientList_Block_PC', 'pull_arrow', 'all');
									showwollist();
									break;
								case "ACL" :
									showDropdownClientList('setClientmac', 'mac', 'wl', 'WL_MAC_List_Block', 'pull_arrow', 'all');
									show_wl_maclist_x();
									break;
								case "ParentalControl" :
									showDropdownClientList('setClientIP', 'mac', 'all', 'ClientList_Block_PC', 'pull_arrow', 'all');
									gen_mainTable();
									break;
								case "GuestNetwork" :
									showDropdownClientList('setClientmac', 'mac', 'wl', 'WL_MAC_List_Block', 'pull_arrow', 'all');
									show_wl_maclist_x();
									break;
								case "WebProtector" :
									showDropdownClientList('setClientIP', 'mac', 'all', 'ClientList_Block_PC', 'pull_arrow', 'all');
									genMain_table();
									break;
								case "ATF" :
									showDropdownClientList('setClientmac', 'mac', 'wl', 'WL_MAC_List_Block', 'pull_arrow', 'all');
									show_wl_atf_by_client();
									break;
								case "WTFast" :
									showDropdownClientList('setClientmac', 'mac', 'all', 'ClientList_Block_PC', 'pull_arrow', 'all');
									show_rulelist();
									break;
								case "RoamingBlock" :
									showDropdownClientList('setClientmac', 'mac', 'wl', 'WL_MAC_List_Block', 'pull_arrow', 'all');
									show_wl_maclist_x();
									break;
								case "AiMesh" :
									client_remove_client_upload_icon(document.getElementById("card_client_macaddr_field").value);
									display_client_block();
									break;
								case "DNSFilter" :
									showDropdownClientList('setClientmac', 'mac', 'all', 'ClientList_Block_PC', 'pull_arrow', 'all');
									show_dnsfilter_list();
									break;
								default :
									refreshpage();
							}
						}
					});
				};
				updateClientListObj();
			}
		}, document.card_clientlist_form.action_wait.value * 1000);
	}
}
function card_setDefaultIcon() {
	var mac = document.getElementById("card_client_macaddr_field").value;
	select_image(clientList[mac], true);
}

//check user icon num is over 100 or not.
function userIconNumLimit(mac) {
	var flag = true;
	var uploadIconMacList = getUploadIconList().replace(/\.log/g, "");
	var selectMac = mac.replace(/\:/g, "");
	var existFlag = (uploadIconMacList.search(selectMac) == -1) ? false : true;
	//check mac exist or not
	if(!existFlag) {
		var userIconCount = getUploadIconCount();
		if(userIconCount >= 100) {	//mac not exist, need check use icnon number whether over 100 or not.
			flag = false;
		}
	}
	return flag;
}
function showUploadIconList() {
	var confirmFlag = true;
	confirmFlag = confirm(stringSafeGet("<#Client_Icon_overload#>"));
	if(confirmFlag) {

		hide_edit_client_block();
		if(document.getElementById("edit_client_block") != null) {
			document.getElementById("edit_client_block").remove();
		}

		if(document.getElementById("edit_uploadicons_block") != null) {
			document.getElementById("edit_uploadicons_block").remove();
		}

		var divObj = document.createElement("div");
		divObj.setAttribute("id","edit_uploadicons_block");
		divObj.className = "usericons_content";

		var code = "";
		code += '<table width="95%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table" style="margin-top:15px;">';
		code += '<thead><tr>';
		code += '<td colspan="4">Client upload icon&nbsp;(<#List_limit#>&nbsp;100)</td>'; /*untranslated*/
		code += '</tr></thead>';
		code += '<tr>';
		code += '<th width="45%"><#ParentalCtrl_username#></th>';
		code += '<th width="30%"><#ParentalCtrl_hwaddr#></th>';
		code += '<th width="15%"><#Client_Icon#></th>';
		code += '<th width="10%"><#CTL_del#></th>';
		code += '</tr>';
		code += '</table>';
		code += '<div id="card_usericons_block"></div>';
		code += '<div style="margin-top:15px;padding-bottom:10px;width:100%;text-align:center;">';
		code += '<input class="button_gen" type="button" onclick="uploadIcon_cancel();" value="<#CTL_Cancel#>">';
		code += '<input class="button_gen" type="button" onclick="uploadIcon_confirm();" value="<#CTL_ok#>">';
		code += '<img id="card_upload_loadingIcon" style="margin-left:5px;display:none;" src="/images/InternetScan.gif">';
		code += '</div>	';
		divObj.innerHTML = code;
		document.body.appendChild(divObj);

		document.getElementById("edit_uploadicons_block").style.display = "block";

		showUploadIconsTable();
		return false;
	}
	else {
		document.getElementById("cardUploadIcon").value = "";
		return false;
	}
}
function showUploadIconsTable() {
	genClientList();
	var uploadIconMacList = getUploadIconList().replace(/\.log/g, "");
	var custom_usericon_row = uploadIconMacList.split('>');
	var code = "";
	var clientIcon = "";
	var custom_usericon_length = custom_usericon_row.length;
	code +='<table width="95%" cellspacing="0" cellpadding="4" align="center" class="list_table" id="cardUploadIcons_table">';
	if(custom_usericon_length == 1) {
		code +='<tr><td class="hintColor" colspan="4"><#IPConnection_VSList_Norule#></td></tr>';
		document.getElementById('edit_uploadicons_block').style.height = "170px";
	}
	else {
		for(var i = 0; i < custom_usericon_length; i += 1) {
			if(custom_usericon_row[i] != "") {
				var formatMac = custom_usericon_row[i].slice(0,2) + ":" + custom_usericon_row[i].slice(2,4) + ":" + custom_usericon_row[i].slice(4,6) + ":" + 
								custom_usericon_row[i].slice(6,8) + ":" + custom_usericon_row[i].slice(8,10)+ ":" + custom_usericon_row[i].slice(10,12);
				code +='<tr>';
				var clientObj = clientList[formatMac];
				var clientName = "";
				if(clientObj != undefined) {
					clientName = (clientObj.nickName.toString() == "") ? clientObj.name.toString() : clientObj.nickName.toString();
				}
				code +='<td width="45%">'+ clientName +'</td>';
				code +='<td width="30%">'+ formatMac +'</td>';
				clientIcon = getUploadIcon(custom_usericon_row[i]);
				code +='<td width="15%"><img id="imgUploadIcon_'+ i +'" class="card_imgUploadIcon" src="' + clientIcon + '"</td>';
				code +='<td width="10%"><input class="remove_btn" onclick="delUploadIcon(this);" value=""/></td></tr>';
			}
		}
		document.getElementById('edit_uploadicons_block').style.height = (61 * custom_usericon_length + 75) + "px";
	}
	code +='</table>';
	document.getElementById("card_usericons_block").innerHTML = code;
};
function delUploadIcon(rowdata) {
	var delIdx = rowdata.parentNode.parentNode.rowIndex;
	var delMac = rowdata.parentNode.parentNode.childNodes[1].innerHTML;
	document.getElementById("cardUploadIcons_table").deleteRow(delIdx);
	card_client_variable.custom_usericon_del += delMac + ">";
	var trCount = document.getElementById("cardUploadIcons_table").rows.length;
	document.getElementById('edit_uploadicons_block').style.height = (61 * (trCount + 1) + 75) + "px";
	if(trCount == 0) {
		var code = "";
		code +='<table width="95%" cellspacing="0" cellpadding="4" align="center" class="list_table" id="cardUploadIcons_table">';
		code +='<tr><td class="hintColor" colspan="4"><#IPConnection_VSList_Norule#></td></tr>';
		code +='</table>';
		document.getElementById('edit_uploadicons_block').style.height = "170px";
		document.getElementById("card_usericons_block").innerHTML = code;
	}
}
function uploadIcon_confirm() {
	document.card_clientlist_form.custom_clientlist.disabled = true;
	document.card_clientlist_form.custom_usericon.disabled = true;
	document.card_clientlist_form.custom_usericon_del.disabled = false;
	document.card_clientlist_form.custom_usericon_del.value = card_client_variable.custom_usericon_del.replace(/\:/g, "");

	// submit card_clientlist_form
	document.card_clientlist_form.submit();
	document.getElementById("card_upload_loadingIcon").style.display = "";
	setTimeout(function(){
		refreshpage();
		card_client_variable.custom_usericon_del = "";
		document.card_clientlist_form.custom_usericon_del.disabled = true;
	}, document.card_clientlist_form.action_wait.value * 1000);
}
function uploadIcon_cancel() {
	card_client_variable.custom_usericon_del = "";
	document.getElementById("edit_uploadicons_block").style.display = "none";
}
function card_select_custom_icon($obj){
	var type = $obj.find('i').attr("class");
	var icon_url = $obj.find('i').css("mask").replace('url(','').replace(')','').replace(/\"/gi, "");
	icon_url = icon_url.replace(/\s*50%\s+50%\s*\/\s*contain\s+no-repeat\s*/, '');
	$("#card_client_image").empty();
	$("#card_client_image").append($('<i>').addClass(type).attr('style','--svg:url(' + icon_url + ')'));
	$("#card_client_image").removeClass().addClass("clientIcon_no_hover");
	card_client_variable.userIconBase64 = icon_url;
	card_client_variable.userUploadFlag = false;
}
function select_image(clientObj, useDefaultType = false) {
	function useTypeIcon(clientObj) {
		let type = (useDefaultType) ? "type" + clientObj.defaultType : "type" + clientObj.type;
		let vendor = clientObj.vendor;
		$("#card_client_image").empty();
		$("#card_client_image").empty();
		$("#card_client_image").append($('<i>').addClass(type));
		$("#card_client_image").removeClass().addClass("clientIcon_no_hover");
		if (vendor != "" && type == "type0" && !isSupport("sfp4m")) {
			var vendorIconClassName = getVendorIconClassName(vendor.toLowerCase());
			if (vendorIconClassName != "") {
				$("#card_client_image").empty();
				$("#card_client_image").append($('<i>').addClass("vendor-icon").addClass(vendorIconClassName));
				$("#card_client_image").removeClass().addClass("vendorIcon_no_hover");
			}
		}
		let userImageFlag = false;
		if (!card_client_variable.firstTimeOpenBlock) {
			if (isSupport("usericon")) {
				card_client_variable.userIconBase64 = getUploadIcon(clientObj.mac.replace(/\:/g, ""));
				card_client_variable.userIconBase64_ori = card_client_variable.userIconBase64;
				if (card_client_variable.userIconBase64 != "NoIcon") {
					$("#card_client_image").empty();
					if (clientObj.isUserUplaodImg) {
						$('#card_client_image').append($('<img>').addClass('clientIcon_no_hover').attr('src', card_client_variable.userIconBase64));
					} else {
						$('#card_client_image').append($('<i>').addClass(type).attr('style', '--svg:url(' + card_client_variable.userIconBase64 + ');'));
					}
					userImageFlag = true;
				}
			}
		}
		if (!userImageFlag) {
			card_client_variable.userIconBase64 = "NoIcon";
			if (type == "type36")
				$("#card_client_image").find("i").addClass("flash");
		}
	}
	if(useDefaultType && clientObj.isASUS && clientObj.name!=="ASUS"){
		fetch(`https://nw-dlcdnet.asus.com/plugin/productIcons/${clientObj.name}.png`)
			.then(response => {
				if (response.status === 200) {
					response.blob().then(blob => {
						const reader = new FileReader();
						reader.readAsDataURL(blob);
						reader.onloadend = function () {
							$("#card_client_image").empty();
							$('#card_client_image').append($('<img>').addClass('clientIcon_no_hover').attr('src',reader.result));
							card_client_variable.userIconBase64 = reader.result;
							card_client_variable.userUploadFlag = true;
						}
					});
				}
			})
			.catch(error => {
				console.error('Error:', error);
				useTypeIcon(clientObj);
			});
	}else{
		useTypeIcon(clientObj);
	}
}

function oui_query_card(mac) {
	var queryStr = mac.replace(/\:/g, "").splice(6,6,"");
	if(mac != document.getElementById("card_client_macaddr_field").value) //avoid click two device quickly
		oui_query_card(document.getElementById("card_client_macaddr_field").value);
	else{
		$.getJSON("/ajax/ouiDB.json", function(data){
			if(data != "" && data[queryStr] != undefined){
        if(document.getElementById("edit_client_block") == null) return true;
				var vendor_name = data[queryStr].trim();
				document.getElementById("card_client_manufacturer_field").value = vendor_name;
				document.getElementById("card_client_manufacturer_field").title = "";
				if(vendor_name.length > 38) {
					document.getElementById("card_client_manufacturer_field").value = vendor_name.substring(0, 36) + "..";
					document.getElementById("card_client_manufacturer_field").title = vendor_name;
				}
			}
		});
	}
}

/*
 * elem 
 * speed, default is 20, optional
 * opacity, default is 100, range is 0~100, optional
 */
function fadeIn(elem, speed, opacity){
	if(elem != null) {
		var setOpacity = function(ev, v) {
			ev.filters ? ev.style.filter = 'alpha(opacity=' + v + ')' : ev.style.opacity = v / 100;
		};
		speed = speed || 20;
		opacity = opacity || 100;

		//initial element display and set opacity is 0
		elem.style.display = "block";
		setOpacity(elem, 0);

		var val = 0;
		//loop add 5 the opacity value
		(function(){
			setOpacity(elem, val);
			val += 5;
			if (val <= opacity) {
				setTimeout(arguments.callee, speed);
			}
		})();
	}
}

/*
 * elem 
 * speed, default is 20, optional
 * opacity, default is 0, range is 0~100, optional
 */
function fadeOut(elem, speed, opacity) {
	if(elem != null) {
		var setOpacity = function(ev, v) {
			ev.filters ? ev.style.filter = 'alpha(opacity=' + v + ')' : ev.style.opacity = v / 100;
		};
		speed = speed || 20;
		opacity = opacity || 0;

		var val = 100;

		(function(){
			setOpacity(elem, val);
			val -= 5;
			if (val >= opacity) {
				setTimeout(arguments.callee, speed);
			}
			else if (val < 0) {
				elem.style.display = "none";
			}
		})();
	}
}

var slideFlag = false;
function slideDown(objnmae, speed) {
	var obj = document.getElementById(objnmae);
	var mySpeed = speed || 300;
	var intervals = mySpeed / 30; // we are using 30 ms intervals
	var holder = document.createElement('div');
	var parent = obj.parentNode;
	holder.setAttribute('style', 'height: 0px; overflow:hidden');
	parent.insertBefore(holder, obj);
	parent.removeChild(obj);
	holder.appendChild(obj);
	obj.style.display = obj.getAttribute("data-original-display") || "";
	var height = obj.offsetHeight;
	var sepHeight = height / intervals;
	var timer = setInterval(function() {
		var holderHeight = holder.offsetHeight;
		if (holderHeight + sepHeight < height) {
			holder.style.height = (holderHeight + sepHeight) + 'px';
		} 
		else {
			// clean up
			holder.removeChild(obj);
			parent.insertBefore(obj, holder);
			parent.removeChild(holder);
			clearInterval(timer);
			slideFlag = false;
		}
	}, 30);
}

function slideUp(objnmae, speed) {
	var obj = document.getElementById(objnmae);
	var mySpeed = speed || 300;
	var intervals = mySpeed / 30; // we are using 30 ms intervals
	var height = obj.offsetHeight;
	var holder = document.createElement('div');
	var parent = obj.parentNode;
	holder.setAttribute('style', 'height: ' + height + 'px; overflow:hidden');
	parent.insertBefore(holder, obj);
	parent.removeChild(obj);
	holder.appendChild(obj);
	var originalDisplay = (obj.style.display !== 'none') ? obj.style.display : '';
	obj.setAttribute("data-original-display", originalDisplay);
	var sepHeight = height / intervals;
	var timer = setInterval(function() {
		var holderHeight = holder.offsetHeight;
		if (holderHeight - sepHeight > 0) {
			holder.style.height = (holderHeight - sepHeight) + 'px';
		}
		else {
			// clean up
			obj.style.display = 'none';
			holder.removeChild(obj);
			parent.insertBefore(obj, holder);
			parent.removeChild(holder);
			clearInterval(timer);
			slideFlag = false;
		}
	}, 30);
}

function registerIframeClick(iframeName, action) {
	var iframe = document.getElementById(iframeName) || top.document.getElementById(iframeName);
	if(iframe != null) {
		var iframeDoc = iframe.contentDocument || iframe.contentWindow.document;

		if (typeof iframeDoc.addEventListener != "undefined") {
			iframeDoc.addEventListener("click", action, false);
		}
		else if (typeof iframeDoc.attachEvent != "undefined") {
			iframeDoc.attachEvent ("onclick", action);
		}
	}
}

function removeIframeClick(iframeName, action) {
	var iframe = document.getElementById(iframeName) || top.document.getElementById(iframeName);
	if(iframe != null) {
		var iframeDoc = iframe.contentDocument || iframe.contentWindow.document;

		if (typeof iframeDoc.removeEventListener != "undefined") {
			iframeDoc.removeEventListener("click", action, false);
		}
		else if (typeof iframeDoc.detachEvent != "undefined") {
			iframeDoc.detachEvent ("onclick", action);
		}
	}
}

var all_list = new Array();//All
var sorter = {
	"indexFlag" : 3 , // default sort is by IP
	"all_index" : 3,
	"all_display" : true,
	"sortingMethod" : "increase",
	"num_increase" : function(a, b) {
		if(sorter.indexFlag == 3) { //IP
			var a_num = 0, b_num = 0;
			a_num = inet_network(a[sorter.indexFlag]);
			b_num = inet_network(b[sorter.indexFlag]);
			return parseInt(a_num) - parseInt(b_num);
		}
		else if(sorter.indexFlag == 5 || sorter.indexFlag == 6 || sorter.indexFlag == 7) { //Interface, Tx, Rx
			var a_num = 0, b_num = 0;
			a_num = (a[sorter.indexFlag] == "") ? 0 : a[sorter.indexFlag];
			b_num = (b[sorter.indexFlag] == "") ? 0 : b[sorter.indexFlag];
			return parseInt(a_num) - parseInt(b_num);
		}
		else if(sorter.indexFlag == 8) { // Time string in (h)hh:mm:ss format
			var a_num = 0, b_num = 0;
			a_num = a[sorter.indexFlag].replace(/:/g, "");
			b_num = b[sorter.indexFlag].replace(/:/g, "");
			return parseInt(a_num) - parseInt(b_num);
		}
		else if(sorter.indexFlag == 8){//Access time
			var a_num = 0, b_num = 0;
			a_num = sorter.convert_time_to_num(a[sorter.indexFlag]);
			b_num = sorter.convert_time_to_num(b[sorter.indexFlag]);
			return parseInt(a_num) - parseInt(b_num);
		}
		else {
			return parseInt(a[sorter.indexFlag]) - parseInt(b[sorter.indexFlag]);
		}
	},
	"num_decrease" : function(a, b) {
		if(sorter.indexFlag == 3) { //IP
			var a_num = 0, b_num = 0;
			a_num = inet_network(a[sorter.indexFlag]);
			b_num = inet_network(b[sorter.indexFlag]);
			return parseInt(b_num) - parseInt(a_num);
		}
		else if(sorter.indexFlag == 5 || sorter.indexFlag == 6 || sorter.indexFlag == 7) { //Interface, Tx, Rx
			var a_num = 0, b_num = 0;
			a_num = (a[sorter.indexFlag] == "") ? 0 : a[sorter.indexFlag];
			b_num = (b[sorter.indexFlag] == "") ? 0 : b[sorter.indexFlag];
			return parseInt(b_num) - parseInt(a_num);
		}
		else if(sorter.indexFlag == 8) { // Time string in (h)hh:mm:ss format
			var a_num = 0, b_num = 0;
			a_num = a[sorter.indexFlag].replace(/:/g, "");
			b_num = b[sorter.indexFlag].replace(/:/g, "");
			return parseInt(b_num) - parseInt(a_num);
		}
		else if(sorter.indexFlag == 8){//Access time
			var a_num = 0, b_num = 0;
			a_num = sorter.convert_time_to_num(a[sorter.indexFlag]);
			b_num = sorter.convert_time_to_num(b[sorter.indexFlag]);
			return parseInt(b_num) - parseInt(a_num);
		}
		else {
			return parseInt(b[sorter.indexFlag]) - parseInt(a[sorter.indexFlag]);
		}
	},
	"str_increase" : function(a, b) {
		if(a[sorter.indexFlag].toUpperCase() == b[sorter.indexFlag].toUpperCase()) return 0;
		else if(a[sorter.indexFlag].toUpperCase() > b[sorter.indexFlag].toUpperCase()) return 1;
		else return -1;
	},
	"str_decrease" : function(a, b) {
		if(a[sorter.indexFlag].toUpperCase() == b[sorter.indexFlag].toUpperCase()) return 0;
		else if(a[sorter.indexFlag].toUpperCase() > b[sorter.indexFlag].toUpperCase()) return -1;
		else return 1;
	},
	"addBorder" : function(obj) {
		let objIndex = obj.cellIndex;
		const clickItem = obj.parentNode.id.split("_")[1];
		let sorterLastIndex = 0;
		let sorterClickIndex = objIndex;

		switch (clickItem) {
			case "all" :
				if (sorter.all_index == sorterClickIndex) {
					sorter.sortingMethod = (sorter.sortingMethod == "increase") ? "decrease" : "increase";
				}
				else {
					sorter.sortingMethod = "increase";
				}
				sorterLastIndex = sorter.all_index;
				sorter.all_index = sorterClickIndex;
				break;
			case "sc" :
				if (sorter[""+clickItem+"_index"] == sorterClickIndex) {
					sorter["sortingMethod_"+clickItem+""] = (sorter["sortingMethod_"+clickItem+""] == "increase") ? "decrease" : "increase";
				}
				else {
					sorter["sortingMethod_"+clickItem+""] = "increase";
				}
				sorterLastIndex = sorter[""+clickItem+"_index"];
				sorter[""+clickItem+"_index"] = sorterClickIndex;
				break;
			default :
				let interface_item = "";
				if(clickItem.substr(0,3) == "sdn")
						interface_item = "sdn";
				else if(clickItem.substr(0,2) == "gn")
						interface_item = "gn";
				else if(clickItem.substr(0,2) == "wl")
						interface_item = "wl";

				if(interface_item != ""){
					sorterLastIndex = sorter[""+clickItem+"_index"];
					sorter[""+clickItem+"_index"] = sorterClickIndex;
					sorter["sortingMethod_"+clickItem+""] = (sorter["sortingMethod_"+clickItem+""] == "increase") ? "decrease" : "increase";
				}
				break
		}
	},
	"drawBorder" : function(_arrayName) {
		const clickItem = _arrayName.split("_")[0];
		let clickIndex = 3;
		let clickSortingMethod = "increase";
		switch (clickItem) {
			case "all" :
				clickIndex = sorter.all_index;
				clickSortingMethod = sorter.sortingMethod;
				break;
			case "wired" :
			case "sc" :
				clickIndex = sorter[""+clickItem+"_index"];
				clickSortingMethod = sorter["sortingMethod_"+clickItem+""];
				break;
			default :
				let interface_idx = "";
				if(clickItem.substr(0,3) == "sdn")
					interface_idx = _arrayName.substr(0,4);
				else if(clickItem.substr(0,2) == "gn")
					interface_idx = _arrayName.substr(0,3);
				else if(clickItem.substr(0,2) == "wl")
					interface_idx = _arrayName.substr(0,3);
				if(interface_idx != ""){
					clickIndex = sorter[""+interface_idx+"_index"];
					clickSortingMethod = sorter["sortingMethod_"+interface_idx+""];
				}
				break;
		}

		const tableHeader = document.querySelectorAll(`#tr_${clickItem}_list_title th`)[clickIndex];

		if (clickSortingMethod === "increase") {
			tableHeader.classList.add('active-top');
		} else {
			tableHeader.classList.add('active-down');
		}

		const sort_text_div = document.createElement('div');
		sort_text_div.textContent = tableHeader.textContent;
		tableHeader.textContent = '';
		tableHeader.appendChild(sort_text_div);
	},
	"doSorter" : function(_flag, _Method, _arrayName) {	
		// update variables
		sorter.indexFlag = _flag;
		// doSorter
		if(clienlistViewMode == "All") {
			eval(""+_arrayName+".sort(sorter."+_Method+"_"+sorter.sortingMethod+");");
		}
		else if(clienlistViewMode == "ByInterface") {
			if(_arrayName == "wired_list")
				eval(""+_arrayName+".sort(sorter."+_Method+"_"+sorter.sortingMethod_wired+");");
			else if(_arrayName.substr(0,2) == "sc")
				eval(""+_arrayName+".sort(sorter."+_Method+"_"+sorter.sortingMethod_sc+");");
			else if(_arrayName.substr(0,2) == "wl")
				eval("wl_list['"+_arrayName.substr(0,3)+"'].sort(sorter."+_Method+"_"+sorter["sortingMethod_"+_arrayName.substr(0,3)+""]+");");
			else if(isSupport("amas")){
				if(isSupport("mtlancfg") && _arrayName.substr(0,3) == "sdn")
					eval("sdn_list['"+_arrayName.substr(0,4)+"'].sort(sorter."+_Method+"_"+sorter["sortingMethod_"+_arrayName.substr(0,4)+""]+");");
				else if(_arrayName.substr(0,2) == "gn")
					eval("gn_list['"+_arrayName.substr(0,3)+"'].sort(sorter."+_Method+"_"+sorter["sortingMethod_"+_arrayName.substr(0,3)+""]+");");
			}
		}
		drawClientListBlock(_arrayName);
		sorter.drawBorder(_arrayName);
	},
	"convert_time_to_num" : function(_time) {
		var num = 0;
		var array = _time.split(":");
		var hour = parseInt(array[0], 10) * 60 * 60;
		if(isNaN(hour))
			hour = 0;
		var min = parseInt(array[1], 10) * 60;
		if(isNaN(min))
			min = 0;
		var sec = parseInt(array[2], 10);
		if(isNaN(sec))
			sec = 0;

		num = hour + min + sec;
		return num;
	}
}
var wired_list = new Array();
var wl_list = [];
var sdn_rl_for_clientlist = [];
if(smart_connect_version != ""){
	var sc_list = [];
}
if(isSupport("amas")){
	if(isSupport("mtlancfg")){
		var sdn_list = [];
		init_sdn_all_list_client();
	}
	else
		var gn_list = [];
}
function init_clientlist_listview_array(){
	wired_list = [];
	wl_list = [];
	for(var index in isWL_map){
		if(index == "0"){
			sorter["wired_index"] = 3;
			sorter["wired_display"] = true;
			sorter["ssortingMethod_wired"] = "increase";
		}
		else{
			wl_list["wl" + index + ""] = new Array();
			sorter["wl" + index + "_index"] = 3;
			sorter["wl" + index + "_display"] = true;
			sorter["sortingMethod_wl" + index + ""] = "increase";
		}
	}
	if(smart_connect_version != ""){
		sorter["sc_index"] = 3;
		sorter["sc_display"] = true;
		sorter["sortingMethod_sc"] = "increase";
	}
	if(isSupport("amas")){
			if(isSupport("mtlancfg")){
				sdn_list = [];
				$.each(sdn_rl_for_clientlist, function(index, sdn_all_rl){
						if(sdn_all_rl.sdn_rl.idx == "0")
							return true;
						sdn_list["sdn" + sdn_all_rl.sdn_rl.idx + ""] = new Array();
						sorter["sdn" + sdn_all_rl.sdn_rl.idx + "_index"] = 3;
						sorter["sdn" + sdn_all_rl.sdn_rl.idx + "_display"] = true;
						sorter["sortingMethod_sdn" + sdn_all_rl.sdn_rl.idx + ""] = "increase";
				});
			}
			else{
				gn_list = [];
				for(var i=1; i<isSupport("mssid_count")+1; i++){
					gn_list["gn" + i + ""] = new Array();
					sorter["gn" + i + "_index"] = 3;
					sorter["gn" + i + "_display"] = true;
					sorter["sortingMethod_gn" + i + ""] = "increase";
				}
			}
	}
}

function init_sdn_all_list_client(){
	var sdn_rl_attr = function(){
		this.idx = "0";
		this.sdn_name = "";
		this.apg_idx = "0";
	};
	var apg_rl_attr = function(){
		this.apg_idx = "";
		this.ssid = "";
	};
	sdn_rl_for_clientlist = [];

	var sdn_all_rl_info = httpApi.nvramCharToAscii(["sdn_rl"], true);
	var sdn_rl = decodeURIComponent(sdn_all_rl_info.sdn_rl);
	var each_sdn_rl = sdn_rl.split("<");
	$.each(each_sdn_rl, function(index, value){
		if(value != ""){
			var sdn_all_rl = {sdn_rl:{}, apg_rl:{}};
			var profile_data = value.split(">");
			var sdn_rl_profile = set_sdn_profile(profile_data);
			sdn_all_rl.sdn_rl = sdn_rl_profile;

			var apg_rl_list = get_apg_rl_list(sdn_rl_profile.apg_idx);
			var specific_apg = apg_rl_list.filter(function(item, index, array){
				return (item.apg_idx == sdn_rl_profile.apg_idx);
			})[0];
			if(specific_apg != undefined){
				sdn_all_rl.apg_rl = specific_apg;
			}
			sdn_rl_for_clientlist.push(sdn_all_rl);
		}
	});

	function set_sdn_profile(profile_data){
		var sdn_profile = JSON.parse(JSON.stringify(new sdn_rl_attr()));
		sdn_profile.idx = profile_data[0];
		sdn_profile.sdn_name = profile_data[1];
		sdn_profile.apg_idx = profile_data[5];
		return sdn_profile;
	}
	function get_apg_rl_list(_apg_idx){
		var apg_rl_list = [];
		if(parseInt(_apg_idx) > 0){
			var apg_profile = new apg_rl_attr();
			var apg_info = httpApi.nvramCharToAscii(["apg" + _apg_idx + "_ssid"], true);
			apg_profile.apg_idx = _apg_idx.toString();
			apg_profile.ssid = decodeURIComponent(apg_info["apg" + _apg_idx + "_ssid"]);
			apg_rl_list.push(JSON.parse(JSON.stringify(apg_profile)));
		}
		return apg_rl_list;
	}
}

var edit_client_name_flag = false;
var clientlist_view_hide_flag = false;
function hide_clientlist_view_block() {
	if(clientlist_view_hide_flag) {
		fadeOut(document.getElementById("clientlist_viewlist_content"), 10, 0);
		document.body.onclick = null;
		document.body.onresize = null;
		clientListViewMacUploadIcon = [];
		removeIframeClick("statusframe", hide_clientlist_view_block);
	}
	clientlist_view_hide_flag = true;
}
function show_clientlist_view_block() {
	clientlist_view_hide_flag = false;
}

function closeClientListView() {
	fadeOut(document.getElementById("clientlist_viewlist_content"), 10, 0);
}

var clienlistViewMode = "All";
function changeClientListViewMode() {
	if(clienlistViewMode == "All")
		clienlistViewMode = "ByInterface";
	else
		clienlistViewMode = "All";

	create_clientlist_listview();
	sorterClientList();
	sorter.all_display = true;
	$.each(isWL_map, function(index, value){
		if(index == "0")
			sorter["wired_display"] = true;
		else
			sorter["wl" + index + "_display"] = true;
	});
	if(smart_connect_version != ""){
		sorter["sc_display"] = true;
	}
	if(isSupport("amas")){
		if(isSupport("mtlancfg")){
			$.each(sdn_rl_for_clientlist, function(index, sdn_all_rl){
					if(sdn_all_rl.sdn_rl.idx == "0")
						return true;
					sorter["sdn" + sdn_all_rl.sdn_rl.idx + "_display"] = true;
			});
		}
		else{
			for(var i=1; i<isSupport("mssid_count")+1; i++)
				sorter["gn" + i + "_display"] = true;
		}
	}
}

var interval_clientlist_listview_update = null;
function pop_clientlist_listview(theme) {
	init_clientlist_listview_array();
	if(document.getElementById("clientlist_viewlist_content") != null) {
		removeElement(document.getElementById("clientlist_viewlist_content"));
	}

	var divObj = document.createElement("div");
	divObj.setAttribute("id","clientlist_viewlist_content");
	divObj.className = `clientlist_viewlist ${theme}`;
	document.body.appendChild(divObj);
	fadeIn(document.getElementById("clientlist_viewlist_content"));
	cal_panel_block_clientList("clientlist_viewlist_content", 0.045);

	if(document.getElementById("view_clientlist_form") != null) {
		removeElement(document.getElementById("view_clientlist_form"));
	}
	var formObj = document.createElement("form");
	formObj.method = "POST";
	formObj.setAttribute("id","view_clientlist_form");
	formObj.setAttribute("name","view_clientlist_form");
	formObj.action = "/start_apply2.htm";
	formObj.target = "hidden_frame";
	var formHtml = "";
	formHtml += "<input type='hidden' name='modified' value='0'>";
	formHtml += "<input type='hidden' name='flag' value='background'>";
	formHtml += "<input type='hidden' name='action_mode' value='apply'>";
	formHtml += "<input type='hidden' name='action_script' value='saveNvram'>";
	formHtml += "<input type='hidden' name='action_wait' value='1'>";
	formHtml += "<input type='hidden' name='custom_clientlist' value=''>";
	formObj.innerHTML = formHtml;
	document.body.appendChild(formObj);

	clientlist_view_hide_flag = false;

	create_clientlist_listview();
	setTimeout(function(){httpApi.nvramSet({"action_mode": "update_client_list"});}, 5000);//avoiding no data when open the view list
	clearInterval(interval_clientlist_listview_update);
	interval_clientlist_listview_update = setInterval(function(){
		if(document.getElementById("clientlist_viewlist_content").style.display != "none")
			httpApi.nvramSet({"action_mode": "update_client_list"});
		else
			clearInterval(interval_clientlist_listview_update);
	}, 1000*60*3);
	setTimeout("sorterClientList();updateClientListView();", 500);

	registerIframeClick("statusframe", hide_clientlist_view_block);
}

function exportClientListLog() {
	var data = [["Internet access state", "Device Type", "Client Name", "Client IP address", "IP Method", "Clients MAC Address", "Interface", "Tx Rate", "Rx Rate", "Access time"]];
	var csv_items = data[0].length;
	var ipStateExport = new Array();
	ipStateExport["Static"] =  "Static IP";
	ipStateExport["DHCP"] =  "Automatic IP";
	ipStateExport["Manual"] =  "MAC-IP Binding";
	var setArray = function(array) {
		for(var i = 0; i < array.length; i += 1) {
			var tempArray = new Array(csv_items).fill("-");
			tempArray[0] = (array[i][0] == 1) ? "Allow Internet access" : "Block Internet access";
			tempArray[1] = `"${array[i][1].replace(",", "").replace(/[=@]/g, " ")}"`;
			tempArray[2] = `"${array[i][2].replace(/[=@]/g, " ")}"`;
			tempArray[3] = array[i][3];
			tempArray[4] = ipStateExport[clientList[array[i][4]].ipMethod];
			tempArray[5] = array[i][4];
			if(!(isSwMode('mb') || isSwMode('ew'))) {
				var if_name = "";
				if(array[i][9] == 0){
					if_name = "Wired";
				}
				else{
					if_name = isWL_map[array[i][9]]["text"].replace("G", " GHz");
				}
				tempArray[6] = if_name;
				tempArray[7] = (array[i][6] == "") ? "-" : array[i][6];
				tempArray[8] = (array[i][7] == "") ? "-" : array[i][7];
				tempArray[9] = (array[i][9] == 0) ? "-" : (array[i][8] == "" ? "00:00:00" : array[i][8]);
			}
			else {
				tempArray[6] = "Wired";
			}
			data.push(tempArray);
		}
	};
	switch (clienlistViewMode) {
		case "All" :
			setArray(all_list);
			break;
		case "ByInterface" :
			$.each(isWL_map, function(index, value){
				if(index == "0")
					setArray(wired_list);
				else
					setArray(wl_list["wl"+index+""]);
			});
			if(isSupport("amas")){
				if(isSupport("mtlancfg")){
					$.each(sdn_rl_for_clientlist, function(index, sdn_all_rl){
							if(sdn_all_rl.sdn_rl.idx == "0")
								return true;
							setArray(sdn_list["sdn" + sdn_all_rl.sdn_rl.idx + ""]);
					});
				}
				else{
					for(var i=1; i<isSupport("mssid_count")+1; i++)
						setArray(gn_list["gn"+i+""]);
				}
			}
			break;
	}
	var csvContent = '';
	data.forEach(function (infoArray, index) {
		dataString = infoArray.join(',');
		csvContent += index < data.length ? dataString + '\n' : dataString;
	});

	var download = function(content, fileName, mimeType) {
		var a = document.createElement('a');
		mimeType = mimeType || 'application/octet-stream';

		if (navigator.msSaveBlob) { // IE10
			return navigator.msSaveBlob(new Blob([content], { type: mimeType }), fileName);
		} 
		else if ('download' in a) { //html5 A[download]
			a.href = 'data:' + mimeType + ',' + encodeURIComponent(content);
			a.setAttribute('download', fileName);
			document.getElementById("clientlist_viewlist_content").appendChild(a);
			setTimeout(function() {
				a.click();
				document.getElementById("clientlist_viewlist_content").removeChild(a);
			}, 66);
			return true;
		} 
		else { //do iframe dataURL download (old ch+FF):
			var f = document.createElement('iframe');
			document.getElementById("clientlist_viewlist_content").appendChild(f);
			f.src = 'data:' + mimeType + ',' + encodeURIComponent(content);

			setTimeout(function() {
				document.getElementById("clientlist_viewlist_content").removeChild(f);
				}, 333);
			return true;
		}
	};

	download(csvContent, 'ClientList.csv', 'data:text/csv;charset=utf-8');
}

function sorterClientList() {
	//initial sort ip
	var indexMapType = ["", "", "str", "num", "str", "num", "num", "num", "num"];
	switch (clienlistViewMode) {
		case "All" :
			sorter.doSorter(sorter.all_index, indexMapType[sorter.all_index], 'all_list');
			break;
		case "ByInterface" :
			$.each(isWL_map, function(index, value){
				if(index == "0")
					sorter.doSorter(sorter.wired_index, indexMapType[sorter.wired_index], 'wired_list');
				else{
					if($("#clientlist_wl" + index + "_list_Block").length > 0)
						sorter.doSorter(sorter["wl" + index + "_index"], indexMapType[sorter["wl" + index + "_index"]], "wl" + index + "_list");
				}
			});
			if(smart_connect_version != ""){
				if($("#clientlist_sc_list_Block").length > 0)
					sorter.doSorter(sorter["sc_index"], indexMapType[sorter["sc_index"]], "sc_list");
			}
			if(isSupport("amas")){
				if(isSupport("mtlancfg")){
					$.each(sdn_rl_for_clientlist, function(index, sdn_all_rl){
							if(sdn_all_rl.sdn_rl.idx == "0")
								return true;
							sorter.doSorter(sorter["sdn"+sdn_all_rl.sdn_rl.idx+"_index"], indexMapType[sorter["sdn"+sdn_all_rl.sdn_rl.idx+"_index"]], 'sdn'+sdn_all_rl.sdn_rl.idx+'_list');
					});
				}
				else{
					for(var i=1; i<isSupport("mssid_count")+1; i++)
						sorter.doSorter(sorter["gn"+i+"_index"], indexMapType[sorter["gn"+i+"_index"]], 'gn'+i+'_list');
				}
			}
			break;
	}
}

const create_clientlist_card = (title, id) => {
	const listDiv = document.getElementById(`collapse_${id}_list`);
	let isShow = !listDiv;
	if (!!listDiv) {
		isShow = listDiv.classList.contains('show');
	}

	const cardCode = `
	<div class='card client_list'>
		<header class='card-header' data-bs-toggle='collapse' aria-controls='collapse_${id}_list' aria-expanded='${isShow?'true':'false'}'>
			<div>
				<span class='title'>${title}</span>
				<i class='arrow_icon ${isShow?'arrow_up':'arrow_down'}'></i>
			</div>
		</header>
		<div id='collapse_${id}_list' class='collapse ${isShow?'show':''}' style="display: ${isShow?'':'none'}">
			<div id='clientlist_${id}_list_Block'></div>
		</div>
	</div>`;

	return cardCode;
};
function create_clientlist_listview() {
	all_list = [];
	$.each(isWL_map, function(index, value){
		if(index == "0")
			wired_list = [];
		else
			wl_list["wl"+index+""] = [];
	});
	if(smart_connect_version != ""){
		sc_list = [];
	}
	if(isSupport("amas")){
		if(isSupport("mtlancfg")){
			$.each(sdn_rl_for_clientlist, function(index, sdn_all_rl){
					if(sdn_all_rl.sdn_rl.idx == "0")
						return true;
					sdn_list["sdn"+sdn_all_rl.sdn_rl.idx+""] = [];
			});
		}
		else{
			for(var i=1; i<isSupport("mssid_count")+1; i++)
				gn_list["gn"+i+""] = [];
		}
	}

	var divObj = document.createElement("div");
	divObj.setAttribute("id","clientlist_viewlist_block");

	var code = "";

	const drawSwitchMode = (mode) => {
		const isAllMode = mode === "All";
		const networkLabel = isSupport("mtlancfg") ? "<#Network#>" : "<#wan_interface#>";

		return `
        <div class='switch_block_main'>
            <div class='switch_block'>
                <div class='segmented_picker_label'><#Statistic_show_type#></div>
            </div>
            <div class='segmented_picker'>
                <div class='segmented_picker_option ${isAllMode ? 'active' : ''}' style='cursor:pointer' onclick='changeClientListViewMode();'>
                    <div class='block_filter_name'><#All#></div>
                </div>
                <div class='segmented_picker_option ${isAllMode ? '' : 'active'}' style='cursor:pointer' onclick='changeClientListViewMode();'>
                    <div class='block_filter_name'>${networkLabel}</div>
                </div>
            </div>
        </div>`;
	};

	if(!(isSwMode('mb') || isSwMode('ew')))
		code += drawSwitchMode(clienlistViewMode);

	code += `<div class="cards">`;

	switch (clienlistViewMode) {
		case "All" :
			code += create_clientlist_card("<#Clientlist_All_List#>", "all");
			break;
		case "ByInterface" :

			code += create_clientlist_card("<#tm_wired#>", "wired");

			var wl_map = {"2.4 GHz": "1",  "5 GHz": "2", "5 GHz-1": "2", "5 GHz-2": "3", "6 GHz": "4", "6 GHz-1": "4", "6 GHz-2": "5"};
			var smart_connect_x = httpApi.nvramGet(["smart_connect_x"]).smart_connect_x;

			if(!isSupport("noWiFi")){
				if(smart_connect_version != "" && smart_connect_x == "1"){
					var wl_ssid_parm = "wl" +  get_wl_unit_by_band("2G") + "_ssid";
					var tr_title = htmlEnDeCode.htmlEncode(decodeURIComponent(httpApi.nvramCharToAscii([wl_ssid_parm])[wl_ssid_parm]));
					code += create_clientlist_card(tr_title, "sc");
				}
				else{
					for(var i = 0; i < wl_nband_title.length; i += 1) {
						var tr_title = wl_nband_title[i];
						if(isSupport("amas") && isSupport("mtlancfg")){
							var wl_if = "";
							switch(wl_map[wl_nband_title[i]]){
								case "1":
									wl_if = "2G";
									break;
								case "2":
									wl_if = "5G";
									break;
								case "3":
									wl_if = "5G2";
									break;
								case "4":
									wl_if = "6G";
									break;
								case "5":
									wl_if = "6G2";
									break;
							}
							if(wl_if != ""){
								const wl_unit = get_wl_unit_by_band(wl_if);
								const cur_wlc_band = (()=>{
									if(typeof wlc_band == "undefined")
										return '<% nvram_get("wlc_band"); %>';
									else
										return wlc_band;
								})();
								var wl_ssid_parm = `wl${wl_unit}_ssid`;
								if(isSwMode("re")){
									if(concurrep_support){
										wl_ssid_parm = `wl${wl_unit}.1_ssid`;
									}
									else{
										if(cur_wlc_band == wl_unit)
											wl_ssid_parm = `wl${wl_unit}.1_ssid`;
									}
								}

								tr_title = htmlEnDeCode.htmlEncode(decodeURIComponent(httpApi.nvramCharToAscii([wl_ssid_parm])[wl_ssid_parm])) + " (" + tr_title + ")";
							}
						}

						code += create_clientlist_card(tr_title, "wl" + wl_map[wl_nband_title[i]]);

					}
				}
			}

			if(isSupport("amas")){
				if(isSupport("mtlancfg")){
					$.each(sdn_rl_for_clientlist, function(index, sdn_all_rl){
						if(sdn_all_rl.sdn_rl.idx == "0")
							return true;
						var sdn_idx = sdn_all_rl.sdn_rl.idx;

						code += create_clientlist_card(sdn_all_rl.apg_rl.ssid, "sdn" + sdn_idx);

					});
				}
				else{
					for(var i=1; i<isSupport("mssid_count")+1; i++){
						code += create_clientlist_card("<#Guest_Network#> - " + i, "gn" + i);
					}
				}
			}
			break;
	}

	code += `</div>`;

	if(!top.isIE8)
		code += `<div class='div-export'><div class='btn-export' onclick='exportClientListLog();'><#btn_Export#></div></div>`;

	code += "</td></tr></tbody>";
	code += "</table>";

	if(document.getElementById("clientlist_viewlist_block") != null) {
		removeElement(document.getElementById("clientlist_viewlist_block"));
	}

	divObj.innerHTML = code;
	document.getElementById("clientlist_viewlist_content").appendChild(divObj);

	//register event to detect mouse click
	document.body.onclick = function() {hide_clientlist_view_block();}
	document.body.onresize = function() {
		if(document.getElementById("clientlist_viewlist_content") !== null) {
			if(document.getElementById("clientlist_viewlist_content").style.display == "block") {
				cal_panel_block_clientList("clientlist_viewlist_content", 0.045);
			}
		}
	}	

	document.getElementById("clientlist_viewlist_content").onclick = function() {show_clientlist_view_block();}

	//copy clientList to each sort array
	genClientList();
	for(let i = 0; i < clientList.length; i += 1) {
		if(clientList[clientList[i]].isOnline) {
			let tempArray = [
				clientList[clientList[i]].internetState,
				clientList[clientList[i]].vendor || clientList[clientList[i]].vendor || `Loading manufacturer..`,
				clientList[clientList[i]].nickName || clientList[clientList[i]].name,
				clientList[clientList[i]].ip,
				clientList[clientList[i]].mac,
				clientList[clientList[i]].rssi,
				clientList[clientList[i]].curTx,
				clientList[clientList[i]].curRx,
				clientList[clientList[i]].wlConnectTime,
				clientList[clientList[i]].isWL,
				clientList[clientList[i]].vendor,
				clientList[clientList[i]].type,
				clientList[clientList[i]].macRepeat,
				clientList[clientList[i]].isGN,
				clientList[clientList[i]].sdn_idx,
				clientList[clientList[i]].isUserUplaodImg,
				clientList[clientList[i]].ip6,
				clientList[clientList[i]].ip6_prefix,
				clientList[clientList[i]].mlo,
				clientList[clientList[i]].isASUS
			];

			switch (clienlistViewMode) {
				case "All" :
					all_list.push(tempArray);
					break;
				case "ByInterface" :
					if(isSupport("amas")){
						if(isSupport("mtlancfg")){
								if(clientList[clientList[i]].sdn_idx > 0)
									sdn_list["sdn"+clientList[clientList[i]].sdn_idx+""].push(tempArray);
								else if(clientList[clientList[i]].isWL == 0)
									wired_list.push(tempArray);
								else{
									wl_list["wl"+clientList[clientList[i]].isWL+""].push(tempArray);
									if(smart_connect_version != ""){
										sc_list.push(tempArray);
									}
								}
						}
						else{
							if(clientList[clientList[i]].isWL != 0 && clientList[clientList[i]].isGN != "")
								gn_list["gn"+clientList[clientList[i]].isGN+""].push(tempArray);
							else if(clientList[clientList[i]].isWL == 0)
								wired_list.push(tempArray);
							else{
								wl_list["wl"+clientList[clientList[i]].isWL+""].push(tempArray);
								if(smart_connect_version != ""){
									sc_list.push(tempArray);
								}
							}
						}
					}
					else{
						if(clientList[clientList[i]].isWL == 0)
							wired_list.push(tempArray);
						else{
							wl_list["wl"+clientList[clientList[i]].isWL+""].push(tempArray);
							if(smart_connect_version != ""){
								sc_list.push(tempArray);
							}
						}
					}
					break;
			}
		}
	}

	if(clienlistViewMode == "All") {
		if(!sorter.all_display) {
			document.getElementById("clientlist_all_list_Block").style.display = "none";
			document.getElementById("all_expander").innerHTML = "[ <#Clientlist_Show#> ]";
		}
	}
	else {
		$.each(isWL_map, function(index, value){
			if(index == "0"){
				if(!sorter.wired_display){
					document.getElementById("clientlist_wired_list_Block").style.display = "none";
					document.getElementById("wired_expander").innerHTML = "[ <#Clientlist_Show#> ]";
				}
			}
			else{
				if(!sorter["wl"+index+"_display"]){
					document.getElementById("clientlist_wl"+index+"_list_Block").style.display = "none";
					document.getElementById("wl"+index+"_expander").innerHTML = "[ <#Clientlist_Show#> ]";
				}
			}
		});
		if(smart_connect_version != ""){
			if(!sorter["sc_display"]){
				document.getElementById("clientlist_sc_list_Block").style.display = "none";
				document.getElementById("sc_expander").innerHTML = "[ <#Clientlist_Show#> ]";
			}
		}
		if(isSupport("amas")){
			if(isSupport("mtlancfg")){
				$.each(sdn_rl_for_clientlist, function(index, sdn_all_rl){
						if(sdn_all_rl.sdn_rl.idx == "0")
							return true;
						if(!sorter["sdn"+sdn_all_rl.sdn_rl.idx+"_display"]){
							document.getElementById("clientlist_sdn"+sdn_all_rl.sdn_rl.idx+"_list_Block").style.display = "none";
							document.getElementById("sdn"+sdn_all_rl.sdn_rl.idx+"_expander").innerHTML = "[ <#Clientlist_Show#> ]";
						}
				});
			}
			else{
				for(var i=1; i<isSupport("mssid_count")+1; i++){
					if(!sorter["gn"+i+"_display"]){
						document.getElementById("clientlist_gn"+i+"_list_Block").style.display = "none";
						document.getElementById("gn"+i+"_expander").innerHTML = "[ <#Clientlist_Show#> ]";
					}
				}
			}
		}
	}
}

function showClientInfoMore(e){
	$(e).hide();
	$(e).closest("tr").find(".client_info_more").show();
}

function closeClientInfoMore(e){
	$(e).closest("tr").find(".client_info_icon_more").show();
	$(e).closest(".client_info_more").hide();
}

var clientListViewMacUploadIcon = new Array();


const clickClientlistCollapse = (e) => {
	e.stopPropagation();
	e.preventDefault();
	const headerElement = e.target.closest('.card-header');
	if (headerElement.getAttribute('aria-expanded') == "true") {
		$(`.collapse[id='${headerElement.getAttribute('aria-controls')}']`).slideUp();
		$(`.collapse[id='${headerElement.getAttribute('aria-controls')}']`).removeClass("show");
		headerElement.setAttribute("aria-expanded", "false");
	} else {
		$(`.collapse[id='${headerElement.getAttribute('aria-controls')}']`).slideDown();
		$(`.collapse[id='${headerElement.getAttribute('aria-controls')}']`).toggleClass("show");
		headerElement.setAttribute("aria-expanded", "true");
	}

	if ($(headerElement).find("i").hasClass("arrow_up")) {
		$(headerElement).find("i").removeClass("arrow_up")
		$(headerElement).find("i").toggleClass("arrow_down");
	} else {
		$(headerElement).find("i").removeClass("arrow_down")
		$(headerElement).find("i").toggleClass("arrow_up");
	}
}

function addEvents(objID) {
	document.querySelector(`header[data-bs-toggle="collapse"][aria-controls="collapse_${objID}"]`).removeEventListener("click", clickClientlistCollapse);
	document.querySelector(`header[data-bs-toggle="collapse"][aria-controls="collapse_${objID}"]`).addEventListener("click", clickClientlistCollapse);
}

function drawClientListBlock(objID) {
	var sortArray = "";
	switch (objID) {
		case "all_list" :
			sortArray = all_list;
			break;
		case "wired_list" :
			sortArray = wired_list;
			break;
	}
	if (sortArray == "" && objID.substr(0, 2) == "wl")
		sortArray = wl_list[objID.substr(0, 3)];
	if (sortArray == "" && smart_connect_version != "") {
		if (objID.substr(0, 2) == "sc")
			sortArray = sc_list;
	}
	if (sortArray == "" && isSupport("amas")) {
		if (isSupport("mtlancfg")) {
			if (objID.substr(0, 3) == "sdn")
				sortArray = sdn_list[objID.substr(0, 4)];
		} else {
			if (objID.substr(0, 2) == "gn")
				sortArray = gn_list[objID.substr(0, 3)];
		}
	}
	const listViewProfile = function (_profile) {
		if (_profile == null)
			_profile = new Array(20).fill("");

		this.internetState = _profile[0];
		this.deviceTypeName = _profile[1];
		this.name = _profile[2];
		this.ip = _profile[3];
		this.mac = _profile[4];
		this.rssi = _profile[5];
		this.curTx = _profile[6];
		this.curRx = _profile[7];
		this.wlConnectTime = _profile[8];
		this.isWL = _profile[9];
		this.vendor = _profile[10];
		this.type = _profile[11];
		this.macRepeat = _profile[12];
		this.isGN = _profile[13];
		this.sdn_idx = _profile[14];
		this.isUserUplaodImg = _profile[15];
		this.ip6 = _profile[16];
		this.ip6_prefix = _profile[17];
		this.mlo = _profile[18];
		this.isASUS = _profile[19];
	}

	if (document.getElementById("clientlist_" + objID + "_Block") != null) {
		if (document.getElementById("tb_" + objID) != null) {
			removeElement(document.getElementById("tb_" + objID));
		}
		const obj_width = ["7%", "7%", "25%", "20%", "15%", "8%", "8%", "10%"];
		const wl_colspan = obj_width.length;
		let clientListCode = "";
		//user icon
		let listView_userIconBase64 = "NoIcon";

		clientListCode += `<table width='100%' cellspacing='0' cellpadding='0' align='center' class='list_table' id='tb_${objID}'>`;

		clientListCode += `
                <thead class='client-list-sm'>
                    <tr id='tr_${objID}_title' class='table-header'>
                        <th width=${obj_width[0]} onclick='sorter.addBorder(this);sorter.doSorter(9, "num", "${objID}");' style='cursor:pointer;'><#wan_interface#></th>
                        <th width=${obj_width[1]}><#Client_Icon#></th>
                        <th width=${obj_width[2]} class='client-list-sm text-left' onclick='sorter.addBorder(this);sorter.doSorter(2, "str", "${objID}");' style='cursor:pointer;'><#Client_Name#></th>
                        <th width=${obj_width[3]} class='client-list-xl text-left' onclick='sorter.addBorder(this);sorter.doSorter(3, "num", "${objID}");' style='cursor:pointer;'><#vpn_client_ip#></th>
                        <th width=${obj_width[4]} class='client-list-xl' onclick='sorter.addBorder(this);sorter.doSorter(4, "str", "${objID}");' style='cursor:pointer;'><#ParentalCtrl_hwaddr#></th>
                        <th width=${obj_width[5]} onclick='sorter.addBorder(this);sorter.doSorter(6, "num", "${objID}");' style='cursor:pointer;' title='The transmission rates of your wireless device'>Tx</th>
                        <th width=${obj_width[6]} onclick='sorter.addBorder(this);sorter.doSorter(7, "num", "${objID}");' style='cursor:pointer;' title='The receive rates of your wireless device'>Rx</th>
                        <th width=${obj_width[7]} onclick='sorter.addBorder(this);sorter.doSorter(8, "num", "${objID}");' style='cursor:pointer;'><#Access_Time#></th>
                    </tr>
                </thead>`;

		clientListCode += `<tbody>`;

		if (sortArray.length == 0) {
			clientListCode += `<tr id='tr_${objID}'><td class='hintColor' colspan='${wl_colspan}'><#IPConnection_VSList_Norule#></td></tr>`;
		} else {
			clientlist_sort = new Array();
			for (var i = 0; i < sortArray.length; i += 1) {
				clientlist_sort.push(new listViewProfile(sortArray[i]));
			}

			for (var j = 0; j < clientlist_sort.length; j += 1) {
				clientListCode += "<tr>";

				let internetStateCss = "";
				let internetStateTip = "";
				if (clientlist_sort[j].internetState) {
					internetStateCss = "bg-active";
					internetStateTip = "Allow Internet access";
				} else {
					internetStateCss = "bg-block";
					internetStateTip = "Block Internet access";
				}

				//Interface
				let clientInterfaceCode = '';
				if (!(isSwMode('mb') || isSwMode('ew'))) {
					let rssi_t = 0;
					if (clientlist_sort[j].isWL == "0")
						rssi_t = "wired";
					else
						rssi_t = client_convRSSI(clientlist_sort[j].rssi);
					let radioIcon_css = "radio-icon";
					if ((clientlist_sort[j].isGN != "" && clientlist_sort[j].isGN != undefined) || (isSupport("mtlancfg") && clientlist_sort[j].sdn_idx > 0)) {
						radioIcon_css += " GN";
					}
					clientInterfaceCode += `<div class='interface_container'><div class='${radioIcon_css} radio-${rssi_t}'></div>`;
					if (clientlist_sort[j].isWL != 0 || (isSupport("mtlancfg") && clientlist_sort[j].sdn_idx > 0)) {
						let band_text = isWL_map[clientlist_sort[j].isWL]["text"];
						if (isSupport("mlo") && (clientlist_sort[j].mlo == "1")) band_text = `MLO`;
						clientInterfaceCode += `<div class='band_block'>${band_text}</div>`;
					}
					clientInterfaceCode += "</div>";
					clientListCode += `<td class='client-list-sm' align='center'>${clientInterfaceCode}</td>`;
				}

				//Icon
				let clientIconCode = '';

				if (isSupport("usericon")) {
					if (clientListViewMacUploadIcon[clientlist_sort[j].mac] == undefined) {
						const clientMac = clientlist_sort[j].mac.replace(/\:/g, "");
						listView_userIconBase64 = getUploadIcon(clientMac);
						clientListViewMacUploadIcon[clientlist_sort[j].mac] = listView_userIconBase64;
					} else {
						listView_userIconBase64 = clientListViewMacUploadIcon[clientlist_sort[j].mac];
					}
				}

				clientIconCode += "<div class='client_icon'>";

				if (clientlist_sort[j].macRepeat > 1) {
					clientIconCode += `<div class="clientlist_circle" onmouseover="return overlib('${clientlist_sort[j].macRepeat} clients are connecting to <% nvram_get("productid"); %> through this device.');" onmouseout="nd();">
							<div>clientlist_sort[j].macRepeat</div>
						</div>`;
				}

				if (listView_userIconBase64 != "NoIcon") {
					clientIconCode += "<div title='" + clientlist_sort[j].deviceTypeName + "'>";
					if (clientlist_sort[j].isUserUplaodImg) {
						clientIconCode += '<img class="imgUserIcon" src="' + listView_userIconBase64 + '">';
					} else {
						clientIconCode += '<div class="imgUserIcon"><i class="type" style="--svg:url(' + listView_userIconBase64 + ')"></i></div>';
					}
					clientIconCode += "</div>";
				} else if (clientlist_sort[j].type != "0" || clientlist_sort[j].vendor == "") {
					var icon_type = "type" + clientlist_sort[j].type;
					clientIconCode += "<div style='cursor:default;' class='clientIcon_no_hover' title='" + clientlist_sort[j].deviceTypeName + "'><i class='" + icon_type + "'></i>";
					if (clientlist_sort[j].type == "36")
						clientIconCode += "<div class='flash'></div>";
					clientIconCode += "</div>";
				} else if (clientlist_sort[j].vendor != "") {
					var vendorIconClassName = getVendorIconClassName(clientlist_sort[j].vendor.toLowerCase());
					if (vendorIconClassName != "" && !isSupport("sfp4m")) {
						clientIconCode += "<div style='height:42px;width:42px;background-size:100%;cursor:default;' class='vendorIcon_no_hover " + vendorIconClassName + "' title='" + clientlist_sort[j].deviceTypeName + "'></div>";
					} else {
						var icon_type = "type" + clientlist_sort[j].type;
						clientIconCode += "<div style='height:40px;width:40px;cursor:default;' class='clientIcon_no_hover' title='" + clientlist_sort[j].deviceTypeName + "'><i class='" + icon_type + "'></i></div>";
					}
				}
				clientIconCode += `<span class='rounded-circle internet_status ${internetStateCss}' title='${internetStateTip}'></span></div>`;
				clientListCode += `<td class='IE8HACK client-list-sm' align='center'>`;
				clientListCode += clientIconCode;
				clientListCode += "</td>";


				let txRate = "-";
				let rxRate = "-";
				let accessTime = "-";
				if (isSupport("stainfo") && !(isSwMode('mb') || isSwMode('ew'))) {
					if (clientlist_sort[j].isWL != 0) {
						txRate = (clientlist_sort[j].curTx == "") ? "-" : clientlist_sort[j].curTx;
						rxRate = (clientlist_sort[j].curRx == "") ? "-" : clientlist_sort[j].curRx;
					} else {
						txRate = "-";
						rxRate = "-";
					}
					if (clientlist_sort[j].wlConnectTime == "00:00:00" || clientlist_sort[j].wlConnectTime == "") {
						accessTime = "-";
					} else {
						accessTime = clientlist_sort[j].wlConnectTime;
					}
				}


				//Width < 960
				var clientNameEnCode = htmlEnDeCode.htmlEncode(clientlist_sort[j].name);
				clientListCode += `
                    <td class='client-list-lg'>
					    <div class='client_info'>
					        <div class='client_info_main'>
					            <div class='client_info_text'>
					                <div class='client-name'>
					                    <div class='clientName' onclick='editClientName(this);'>${clientNameEnCode}</div>
					                    <input type='text' value='${clientNameEnCode}' class='input-client-name' maxlength='32' onblur='saveClientName(this, ${clientlist_sort[j].type}, "${clientlist_sort[j].mac}");'>
                                    </div>
                                    <div class='client-ip'>`;
				clientListCode += (clientList[clientlist_sort[j].mac].isWebServer) ? `
					<a class='link' href='http://${clientlist_sort[j].ip}' target='_blank'>${clientlist_sort[j].ip}</a>`
					: `
					<div class="client-ip-group">
						<div>${clientlist_sort[j].ip}</div>
						<div style="font-size: 0.75em;" title="<#IPv6_wan_addr#>">${clientlist_sort[j].ip6_prefix}</div>
						<div style="font-size: 0.75em;" title="WAN IPv6 Link-Local">${clientlist_sort[j].ip6}</div>
					</div>`;
				if ('<% nvram_get("sw_mode"); %>' == "1") {
					clientListCode += `<span class="ipMethodTag" onmouseover="return overlib('${ipState[clientList[clientlist_sort[j].mac].ipMethod]}')" onmouseout="nd();">${clientList[clientlist_sort[j].mac].ipMethod}</span>`
				}
				clientListCode += `</div>`;
				clientListCode += `<div class='client-mac'>${clientlist_sort[j].mac}</div>`;
				clientListCode += `</div>`;
				clientListCode += `<div class='client_info_right'>${clientInterfaceCode}${clientIconCode}</div>`;
				clientListCode += `</div>`;
				clientListCode += `
					<div class='client_info_bottom'>
						<div class='client_info_detail_block'>
							<div class='client_info_detail_title'>TX</div>
							<div>${txRate} Mbps</div>
						</div>
						<div class='client_info_detail_block'>
							<div class='client_info_detail_title'>RX</div>
							<div>${rxRate} Mbps</div>
						</div>
						<div class='client_info_detail_block'>
							<div class='client_info_detail_title'><#Access_Time#></div>
							<div>${accessTime}</div>
						</div>
						<div class='client_info_icon_more' onclick='showClientInfoMore(this)'></div>
					</div>`;
				clientListCode += `</div></div>`;
				clientListCode += `
					<div class='client_info_more' style='display: none;'>
						<div class='client_info_more_detail'>
							<div class='client_info_detail_block'><div class='client_info_detail_title'>TX</div>${txRate} Mbps</div><div class='client_info_detail_block'><div class='client_info_detail_title'>RX</div>${rxRate} Mbps</div><div class='client_info_detail_block'><div class='client_info_detail_title'><#Access_Time#></div>${accessTime}</div></div><div class='client_info_more_close' onclick='closeClientInfoMore(this)'></div></div></td>`;

				//Width >= 960
				clientListCode += `<td class='client-list-xl text-left'>
					<div class='clientName' onclick='editClientName(this);'>${clientNameEnCode}</div>
					<input type='text' value='${clientNameEnCode}' class='input-client-name' maxlength='32' onblur='saveClientName(this, ${clientlist_sort[j].type}, "${clientlist_sort[j].mac}");'>
					</td>`;
				clientListCode += `
					<td class='client-list-xl text-left'>
						<div class="client-ip">`;
				clientListCode += (clientList[clientlist_sort[j].mac].isWebServer)
					? `
					<a class='link' href='http://${clientlist_sort[j].ip}' target='_blank'>
						<div class="client-ip-group">
							<div>${clientlist_sort[j].ip}</div>
							<div style="font-size: 0.75em;" title="<#IPv6_wan_addr#>">${clientlist_sort[j].ip6_prefix}</div>
							<div style="font-size: 0.75em;" title="WAN IPv6 Link-Local">${clientlist_sort[j].ip6}</div>
						</div>
					</a>` : `
					<div class="client-ip-group">
						<div>${clientlist_sort[j].ip}</div>
						<div style="font-size: 0.75em;" title="<#IPv6_wan_addr#>">${clientlist_sort[j].ip6_prefix}</div>
						<div style="font-size: 0.75em;" title="WAN IPv6 Link-Local">${clientlist_sort[j].ip6}</div>
					</div>`;
				if ('<% nvram_get("sw_mode"); %>' == "1") {
					clientListCode += `<span class="ipMethodTag" onmouseover="return overlib('${ipState[clientList[clientlist_sort[j].mac].ipMethod]}')" onmouseout="nd();">${clientList[clientlist_sort[j].mac].ipMethod}</span>`
				}

				clientListCode += `</div></td>`;
				clientListCode += `
					<td class='client-list-xl'>${clientlist_sort[j].mac}</td>
					<td class='client-list-sm'>${txRate}</td>
					<td class='client-list-sm'>${rxRate}</td>
					<td class='client-list-sm'>${accessTime}</td>`;
				clientListCode += "</tr>";
			}
		}
		clientListCode += `</tbody>`;
		clientListCode += "</table>";
		document.getElementById("clientlist_" + objID + "_Block").innerHTML = clientListCode;
		addEvents(objID);
	}
}

function showHideContent(objnmae, thisObj) {
	if(!slideFlag) {
		var state = document.getElementById(objnmae).style.display;
		var clickItem = objnmae.split("_")[1];
		if(state == "none") {
			if(clienlistViewMode == "All")
				sorter.all_display = true;
			else {
				if(clickItem == "wired")
					sorter.wired_display = true;
				else
					sorter["" + clickItem + "_display"] = true;
			}
			slideFlag = true;
			slideDown(objnmae, 200);
			thisObj.innerHTML = "[ <#Clientlist_Hide#> ]";
		}
		else {
			if(clienlistViewMode == "All")
				sorter.all_display = false;
			else {
				if(clickItem == "wired")
					sorter.wired_display = false;
				else
					sorter["" + clickItem + "_display"] = false;
			}
			slideFlag = true;
			slideUp(objnmae, 200);
			thisObj.innerHTML = "[ <#Clientlist_Show#> ]";
		}
	}
}

var updateClientListView_timer = null;
function updateClientListView(){
	$.ajax({
		url: '/update_clients.asp',
		dataType: 'script', 
		error: function(xhr) {
			setTimeout("updateClientListView();", 1000);
		},
		success: function(response){
			if(document.getElementById("clientlist_viewlist_content").style.display != "none") {
				if((isJsonChanged(originData, originDataTmp) || originData.fromNetworkmapd == "") && !edit_client_name_flag && !slideFlag){
					create_clientlist_listview();
					sorterClientList();
					if(parent.show_client_status != undefined)
						parent.show_client_status(totalClientNum.online);
				}
				clearTimeout(updateClientListView_timer);
				updateClientListView_timer = setTimeout("updateClientListView();", 3000);
			}
			else
				clearTimeout(updateClientListView_timer);
		}
	});
}

function cal_panel_block_clientList(obj, multiple) {
	var isMobile = function() {
		var tmo_support = ('<% nvram_get("rc_support"); %>'.search("tmo") == -1) ? false : true;
		if(!tmo_support)
			return false;
		
		if(	navigator.userAgent.match(/iPhone/i)	|| 
			navigator.userAgent.match(/iPod/i)		||
			navigator.userAgent.match(/iPad/i)		||
			(navigator.userAgent.match(/Android/i) && (navigator.userAgent.match(/Mobile/i) || navigator.userAgent.match(/Tablet/i))) ||
			(navigator.userAgent.match(/Opera/i) && (navigator.userAgent.match(/Mobi/i) || navigator.userAgent.match(/Mini/i))) ||	// Opera mobile or Opera Mini
			navigator.userAgent.match(/IEMobile/i)	||	// IE Mobile
			navigator.userAgent.match(/BlackBerry/i)	//BlackBerry
		 ) {
			return true;
		}
		else {
			return false;
		}
	};
	var blockmarginLeft;
	if (window.innerWidth) {
		winWidth = window.innerWidth;
	}
	else if ((document.body) && (document.body.clientWidth)) {
		winWidth = document.body.clientWidth;
	}

	if (document.documentElement  && document.documentElement.clientHeight && document.documentElement.clientWidth) {
		winWidth = document.documentElement.clientWidth;
	}

	if(winWidth > 1050) {
		winPadding = (winWidth - 1050) / 2;
		winWidth = 1105;
		blockmarginLeft = (winWidth * multiple) + winPadding;
	}
	else if(winWidth <= 1050) {
		if(isMobile()) {
			if(document.body.scrollLeft < 50) {
				blockmarginLeft= (winWidth) * multiple + document.body.scrollLeft;
			}
			else if(document.body.scrollLeft >320) {
				blockmarginLeft = 320;
			}
			else {
				blockmarginLeft = document.body.scrollLeft;
			}	
		}
		else {
			blockmarginLeft = (winWidth) * multiple + document.body.scrollLeft;	
		}
	}

	document.getElementById(obj).style.marginLeft = blockmarginLeft + "px";
}

function getFilePath(file) {
	var currentPath = file;
	var pathLength = location.pathname.split("/").length - 1;
	for(var i = pathLength; i > 1; i -= 1) {
	if(i == 1)
		currentPath = ".." + file;
	else
		currentPath = "../" + file;
	}
	return currentPath
};

function editClientName(e) {
	$(e).hide();
	$(e).siblings('input').show();
	$(e).siblings('input').focus();
	edit_client_name_flag = true;
}
var view_custom_name = decodeURIComponent('<% nvram_char_to_ascii("", "custom_clientlist"); %>').replace(/&#62/g, ">").replace(/&#60/g, "<");
function saveClientName(e, type, mac) {
	const client_name_obj = $(e);
	client_name_obj.closest("tr").find("input").val(client_name_obj.val().trim());
	if(client_name_obj.val().length == 0){
		alert("<#File_Pop_content_alert_desc1#>");
		window.setTimeout(function () {
			client_name_obj.focus();
			client_name_obj.select();
			client_name_obj.val("");
		}, 10);
		return false;
	}
	else if(client_name_obj.val().indexOf(">") != -1 || client_name_obj.val().indexOf("<") != -1){
		alert("<#JS_validstr2#> '<', '>'");
		window.setTimeout(function () {
			client_name_obj.focus();
			client_name_obj.select();
			client_name_obj.val("");
		}, 10);
		return false;
	}

	if(isSupport("utf8_ssid")){
		var len = validator.lengthInUtf8(client_name_obj.val());
		if(len > 32){
			alert("Username cannot be greater than 32 characters.");/* untranslated */
			window.setTimeout(function () {
				client_name_obj.focus();
				client_name_obj.select();
				client_name_obj.val("");
			}, 10);
			return false;
		}
	}
	else if(!validator.haveFullWidthChar(client_name_obj)) {
		alert('<#JS_validchar#>');
		window.setTimeout(function () {
			client_name_obj.focus();
			client_name_obj.select();
			client_name_obj.val("");
		}, 10);
		return false;
	}

	client_name_obj.closest("tr").find(".clientName").html(client_name_obj.val().trim());
	client_name_obj.siblings(".clientName").show();
	client_name_obj.hide();

	edit_client_name_flag = false;

	var originalCustomListArray = new Array();
	var onEditClient = new Array();
	originalCustomListArray = view_custom_name.split('<');

	onEditClient[0] = client_name_obj.val();
	onEditClient[1] = mac.toUpperCase();
	onEditClient[2] = 0;
	onEditClient[3] = type;
	onEditClient[4] = "";
	onEditClient[5] = "";

	for(var i = 0; i < originalCustomListArray.length; i += 1) {
		if(originalCustomListArray[i].split('>')[1] != undefined) {
			if(originalCustomListArray[i].split('>')[1].toUpperCase() == onEditClient[1].toUpperCase()){
				onEditClient[4] = originalCustomListArray[i].split('>')[4]; // set back callback for ROG device
				onEditClient[5] = originalCustomListArray[i].split('>')[5]; // set back keeparp for ROG device
				var app_group_tag = originalCustomListArray[i].split('>')[6]; // for app group tag
				if(typeof app_group_tag != "undefined")	onEditClient[6] = app_group_tag;
				var app_age_tag = originalCustomListArray[i].split('>')[7]; // for app age tag
				if(typeof app_age_tag != "undefined")	onEditClient[7] = app_age_tag;
				originalCustomListArray.splice(i, 1); // remove the selected client from original list
			}
		}
	}
	originalCustomListArray.push(onEditClient.join('>'));
	view_custom_name = originalCustomListArray.join('<');
	document.view_clientlist_form.custom_clientlist.value = view_custom_name;
	document.view_clientlist_form.submit();
}

function removeClient(_mac, _controlObj, _controlPanel) {
	if (!e) 
		var e = window.event; 
	e.cancelBubble = true;
	if (e.stopPropagation)
		e.stopPropagation();

	if(document.getElementById("remove_nmpclientlist_form") != null) {
		removeElement(document.getElementById("remove_nmpclientlist_form"));
	}
	var formHTML = "";
	var formObj = document.createElement("form");
	document.body.appendChild(formObj);
	formObj.method = "POST";
	formObj.setAttribute("id","remove_nmpclientlist_form");
	formObj.setAttribute("name","remove_nmpclientlist_form");
	formObj.action = "/deleteOfflineClient.cgi";
	formObj.target = "hidden_frame";

	formHTML += '<input type="hidden" name="modified" value="0">';
	formHTML += '<input type="hidden" name="flag" value="">';
	formHTML += '<input type="hidden" name="action_mode" value="">';
	formHTML += '<input type="hidden" name="action_script" value="">';
	formHTML += '<input type="hidden" name="action_wait" value="1">';
	formHTML += '<input type="hidden" name="delete_offline_client" value="">';
	formObj.innerHTML = formHTML;

	document.remove_nmpclientlist_form.delete_offline_client.value = _mac;

	//remove the client Element
	if(document.getElementById(_mac) != null) {
		removeElement(document.getElementById(_mac));
	}

	//remove offline title and content
	if(document.getElementById(_controlPanel).childNodes.length == "0") {
		if(document.getElementById(_controlPanel) != null) {
			removeElement(document.getElementById(_controlPanel));
		}
		if(document.getElementById(_controlObj) != null) {
			removeElement(document.getElementById(_controlObj));
		}
	}
	document.remove_nmpclientlist_form.submit();
}
function expand_hide_Client(_obj, _controlObj) {
	if(!slideFlag) {
		var display_state = document.getElementById(_controlObj).style.display;
		if(display_state == "none") {
			slideFlag = true;
			slideDown(_controlObj, 100);
			document.getElementById(_obj).innerText = "<#Offline_client_hide#>";
		}
		else {
			slideFlag = true;
			slideUp(_controlObj, 100);
			document.getElementById(_obj).innerText = "<#Offline_client_show#>";
		}
	}
}

function control_dropdown_client_block(_containerID, _pullArrowID, _evt) {
	_evt.stopPropagation(); //cancel bubbling
	var element = _evt.target || _evt.srcElement;
	if(element.id == "") {
		if(document.getElementById(_containerID) != null && document.getElementById(_pullArrowID) != null) {
			var container_state = document.getElementById(_containerID).style.display;
			var pullArrow_state = document.getElementById(_pullArrowID).src;
			if(container_state == "block") {
				document.getElementById(_containerID).style.display = "none";
				document.getElementById(_pullArrowID).src = "/images/unfold_more.svg";
			}
		}
	}
}

//_callBackFunParam = mac>ip>..., _interfaceMode = all(wired, wll), wired, wl, _clientState = all, online, offline
function showDropdownClientList(_callBackFun, _callBackFunParam, _interfaceMode, _containerID, _pullArrowID, _clientState) {
	document.body.addEventListener("click", function(_evt) {control_dropdown_client_block(_containerID, _pullArrowID, _evt);})
	if(clientList.length == 0){
		setTimeout(function() {
			genClientList();
			showDropdownClientList(_callBackFun, _callBackFunParam, _interfaceMode, _containerID, _pullArrowID);
		}, 500);
		return false;
	}

	var htmlCode = "";
	htmlCode += "<div id='" + _containerID + "_clientlist_online'></div>";
	htmlCode += "<div id='" + _containerID + "_clientlist_dropdown_expand' class='clientlist_dropdown_expand' onclick='expand_hide_Client(\"" + _containerID + "_clientlist_dropdown_expand\", \"" + _containerID + "_clientlist_offline\");' onmouseover='over_var=1;' onmouseout='over_var=0;'><#Offline_client_show#></div>";
	htmlCode += "<div id='" + _containerID + "_clientlist_offline'></div>";
	document.getElementById(_containerID).innerHTML = htmlCode;

	var param = _callBackFunParam.split(">");
	var clientMAC = "";
	var clientIP = "";
	var getClientValue = function(_attribute, _clienyObj) {
		var attribute_value = "";
		switch(_attribute) {
			case "mac" :
				attribute_value = _clienyObj.mac;
				break;
			case "ip" :
				if(clientObj.ip != "offline") {
					attribute_value = _clienyObj.ip;
				}
				break;
			case "name" :
				attribute_value = (clientObj.nickName == "") ? clientObj.name : clientObj.nickName;
				break;
			default :
				attribute_value = _attribute;
				break;
		}
		return attribute_value;
	};

	var genClientItem = function(_state) {
		var code = "";
		var clientName = (clientObj.nickName == "") ? clientObj.name : clientObj.nickName;
		
		code += '<a id=' + clientList[i] + ' title=' + clientList[i] + '>';
		if(_state == "online")
			code += '<div onclick="' + _callBackFun + '(\'';
		else if(_state == "offline")
			code += '<div style="color:#A0A0A0" onclick="' + _callBackFun + '(\'';
		for(var j = 0; j < param.length; j += 1) {
			if(j == 0) {
				code += getClientValue(param[j], clientObj);
			}
			else {
				code += '\', \'';
				code += getClientValue(param[j], clientObj);
			}
		}
		code += '\');">';
		code += '<strong>';
		if(clientName.length > 32) {
			code += clientName.substring(0, 30) + "..";
		}
		else {
			code += clientName;
		}
		code += '</strong>';
		if(_state == "offline")
			code += '<strong title="Remove this client" style="float:right;margin-right:5px;cursor:pointer;" onclick="removeClient(\'' + clientObj.mac + '\', \'' + _containerID  + '_clientlist_dropdown_expand\', \'' + _containerID  + '_clientlist_offline\')">×</strong>';
		code += '</div><!--[if lte IE 6.5]><iframe class="hackiframe2"></iframe><![endif]--></a>';
		return code;
	};

	for(var i = 0; i < clientList.length; i +=1 ) {
		var clientObj = clientList[clientList[i]];
		if(clientObj.amesh_isRe)
			continue;
		switch(_clientState) {
			case "all" :
				if(_interfaceMode == "wl" && (clientList[clientList[i]].isWL == 0)) {
					continue;
				}
				if(_interfaceMode == "wired" && (clientList[clientList[i]].isWL != 0)) {
					continue;
				}
				if(clientObj.isOnline) {
					document.getElementById("" + _containerID + "_clientlist_online").innerHTML += genClientItem("online");
				}
				else if(clientObj.from == "nmpClient") {
					document.getElementById("" + _containerID + "_clientlist_offline").innerHTML += genClientItem("offline");
				}
				break;
			case "online" :
				if(_interfaceMode == "wl" && (clientList[clientList[i]].isWL == 0)) {
					continue;
				}
				if(_interfaceMode == "wired" && (clientList[clientList[i]].isWL != 0)) {
					continue;
				}
				if(clientObj.isOnline) {
					document.getElementById("" + _containerID + "_clientlist_online").innerHTML += genClientItem("online");
				}
				break;
			case "offline" :
				if(_interfaceMode == "wl" && (clientList[clientList[i]].isWL == 0)) {
					continue;
				}
				if(_interfaceMode == "wired" && (clientList[clientList[i]].isWL != 0)) {
					continue;
				}
				if(clientObj.from == "nmpClient") {
					document.getElementById("" + _containerID + "_clientlist_offline").innerHTML += genClientItem("offline");
				}
				break;
		}		
	}
	
	if(document.getElementById("" + _containerID + "_clientlist_offline").childNodes.length == "0") {
		if(document.getElementById("" + _containerID + "_clientlist_dropdown_expand") != null) {
			removeElement(document.getElementById("" + _containerID + "_clientlist_dropdown_expand"));
		}
		if(document.getElementById("" + _containerID + "_clientlist_offline") != null) {
			removeElement(document.getElementById("" + _containerID + "_clientlist_offline"));
		}
	}
	else {
		if(document.getElementById("" + _containerID + "_clientlist_dropdown_expand").innerText == "<#Offline_client_show#>") {
			document.getElementById("" + _containerID + "_clientlist_offline").style.display = "none";
		}
		else {
			document.getElementById("" + _containerID + "_clientlist_offline").style.display = "";
		}
	}
	if(document.getElementById("" + _containerID + "_clientlist_online").childNodes.length == "0") {
		if(document.getElementById("" + _containerID + "_clientlist_online") != null) {
			removeElement(document.getElementById("" + _containerID + "_clientlist_online"));
		}
	}

	if(document.getElementById(_containerID).childNodes.length == "0"){
		if(document.getElementById(_pullArrowID) != null)
			document.getElementById(_pullArrowID).style.display = "none";
	}
	else{
		if(document.getElementById(_pullArrowID) != null)
			document.getElementById(_pullArrowID).style.display = "";
	}
}

function redirectTimeScheduling(_mac) {
	window.localStorage.setItem("time_scheduling_mac", _mac, 1);
	location.href = "ParentalControl.asp" ;
}

var custom_icon_list_api = {
	"paramObj": {
		"container":"",
		"source":"",//local or cloud
		"db":"",//custom_icon_list_api.db or cloud response data
		"select_icon_callBack":"",//call back fun.
		"upload_callBack":""//call back fun.
	},
	"db": {
		"electronic_product": {
			"title": "Electronic Devices",
			"translation": "#CLIENTLIST_ELECTRONIC_DEVICES",
			"list": [
				{
					"num": 0,
					"title": "Device",
					"translation": "#CLIENTLIST_DEVICE"
				},
				{
					"num": 34,
					"title": "Desktop",
					"translation": "#CLIENTLIST_DESKTOP"
				},
				{
					"num": 14,
					"title": "iMac",
					"translation": ""
				},
				{
					"num": 1,
					"title": "Windows Desktop",
					"translation": "#CLIENTLIST_WINDOWS_DESKTOP"
				},
				{
					"num": 72,
					"title": "Linux Desktop",
					"translation": "#CLIENTLIST_LINUX_DESKTOP"
				},
				{
					"num": 22,
					"title": "Linux Device",
					"translation": "#CLIENTLIST_LINUX_DEVICE"
				},
				{
					"num": 33,
					"title": "Smartphone",
					"translation": "#CLIENTLIST_SMARTPHONE"
				},
				{
					"num": 10,
					"title": "iPhone",
					"translation": "#CLIENTLIST_IPHONE"
				},
				{
					"num": 28,
					"title": "ASUS smartphone",
					"translation": "#CLIENTLIST_ASUS_SMARTPHONE"
				},
				{
					"num": 19,
					"title": "Windows Phone",
					"translation": "#CLIENTLIST_WINDOWS_PHONE"
				},
				{
					"num": 9,
					"title": "Android Phone",
					"translation": "#CLIENTLIST_ANDROID_PHONE"
				},
				{
					"num": 73,
					"title": "Pad",
					"translation": "#CLIENTLIST_PAD"
				},
				{
					"num": 21,
					"title": "iPad",
					"translation": "#CLIENTLIST_IPAD"
				},
				{
					"num": 29,
					"title": "ASUS Pad",
					"translation": "#CLIENTLIST_ASUS_PAD"
				},
				{
					"num": 20,
					"title": "Android Tablet",
					"translation": "#CLIENTLIST_ANDROID_TABLET"
				},
				{
					"num": 2,
					"title": "Router",
					"translation": "#ROUTER"
				},
				{
					"num": 24,
					"title": "Repeater",
					"translation": "#CLIENTLIST_REPEATER"
				},
				{
					"num": 4,
					"title": "NAS/Server",
					"translation": "#CLIENTLIST_NAS_SERVER"
				},
				{
					"num": 37,
					"title": "Notebook",
					"translation": "#CLIENTLIST_NB"
				},
				{
					"num": 35,
					"title": "Windows NB",
					"translation": "#CLIENTLIST_WINDOWS_NB"
				},
				{
					"num": 6,
					"title": "Macbook",
					"translation": "#CLIENTLIST_MACBOOK"
				},
				{
					"num": 38,
					"title": "ASUS Notebook",
					"translation": "#Clientlist_ASUS_NB"
				},
				{
					"num": 39,
					"title": "SD Card",
					"translation": "#CLIENTLIST_SD_CARD"
				},
				{
					"num": 40,
					"title": "USB",
					"translation": ""
				},
				{
					"num": 18,
					"title": "Printer",
					"translation": "#PRINTER"
				},
				{
					"num": 41,
					"title": "Camera",
					"translation": ""
				},
				{
					"num": 15,
					"title": "ROG Device",
					"translation": "#CLIENTLIST_ROG_DEVICE"
				},
				{
					"num": 25,
					"title": "Kindle",
					"translation": ""
				},
				{
					"num": 26,
					"title": "Scanner",
					"translation": "#CLIENTLIST_SCANNER"
				},
				{
					"num": 32,
					"title": "Apple Device",
					"translation": "#CLIENTLIST_APPLE_DEVICE"
				},
				{
					"num": 31,
					"title": "Android Device",
					"translation": "#CLIENTLIST_ANDROID_DEVICE"
				},
				{
					"num": 30,
					"title": "Windows Device",
					"translation": "#CLIENTLIST_WINDOWS_DEVICE"
				},
				{
					"num": 42,
					"title": "ASUS Device",
					"translation": "#CLIENTLIST_ASUS_DEVICE"
				}
			]
		},
		"media_and_entertainment": {
			"title": "Media and Entertainment",
			"translation": "#CLIENTLIST_MEDIA_ENTERTAINMENT",
			"list": [
				{
					"num": 43,
					"title": "Google Home",
					"translation": ""
				},
				{
					"num": 44,
					"title": "Amazon Alexa",
					"translation": ""
				},
				{
					"num": 45,
					"title": "Apple HomePod",
					"translation": ""
				},
				{
					"num": 11,
					"title": "Apple TV",
					"translation": ""
				},
				{
					"num": 46,
					"title": "DLINA",
					"translation": ""
				},
				{
					"num": 47,
					"title": "Home Cinema",
					"translation": ""
				},
				{
					"num": 48,
					"title": "Wireless Headphone",
					"translation": ""
				},
				{
					"num": 7,
					"title": "Game Console",
					"translation": ""
				},
				{
					"num": 12,
					"title": "Set-top Box",
					"translation": ""
				},
				{
					"num": 27,
					"title": "Chromecast",
					"translation": ""
				},
				{
					"num": 74,
					"title": "PlayStation",
					"translation": ""
				},
				{
					"num": 75,
					"title": "PlayStation 4",
					"translation": ""
				},
				{
					"num": 76,
					"title": "XBox",
					"translation": ""
				},
				{
					"num": 77,
					"title": "Xbox One",
					"translation": ""
				}
			]
		},
		"electronic_equipment": {
			"title": "Electronic equipment",
			"translation": "#CLIENTLIST_ELECTRONIC_EQUIPMENT",
			"list": [
				{
					"num": 49,
					"title": "Air Conditioner",
					"translation": "#CLIENTLIST_AIR_CONDITIONER"
				},
				{
					"num": 50,
					"title": "Refrigerator",
					"translation": "#CLIENTLIST_REFRIGERATOR"
				},
				{
					"num": 51,
					"title": "Weight scale",
					"translation": "#CLIENTLIST_WEIGHT_SCALE"
				},
				{
					"num": 52,
					"title": "Electric pot",
					"translation": "#CLIENTLIST_ELECTRIC_POT"
				},
				{
					"num": 53,
					"title": "Microwave oven",
					"translation": "#CLIENTLIST_MICROWAVE_OVEN"
				},
				{
					"num": 54,
					"title": "Air Purifier",
					"translation": "#CLIENTLIST_AIR_PURIFIER"
				},
				{
					"num": 55,
					"title": "Fan",
					"translation": "#CLIENTLIST_FAN"
				},
				{
					"num": 56,
					"title": "Dehumidifier",
					"translation": "#CLIENTLIST_DEHUMIDIFIER"
				},
				{
					"num": 57,
					"title": "Washing Machine",
					"translation": "#CLIENTLIST_WASHING_MACHINE"
				},
				{
					"num": 58,
					"title": "Water Heater",
					"translation": "#CLIENTLIST_WATER_HEATER"
				},
				{
					"num": 59,
					"title": "Electric Kettle",
					"translation": "#CLIENTLIST_ELECTRIC_KETTLE"
				},
				{
					"num": 60,
					"title": "Smart Bulb",
					"translation": "#CLIENTLIST_SMART_BULB"
				},
				{
					"num": 23,
					"title": "Smart TV",
					"translation": "#CLIENTLIST_SMART_TV"
				},
				{
					"num": 61,
					"title": "Cleaning Robot",
					"translation": "#CLIENTLIST_CLEANING_ROBOT"
				}
			]
		},
		"security": {
			"title": "Security",
			"translation": "#CLIENTLIST_SECURITY",
			"list": [
				{
					"num": 62,
					"title": "Seismograph",
					"translation": "#CLIENTLIST_SEISMOGRAPH"
				},
				{
					"num": 63,
					"title": "Smart lock",
					"translation": "#CLIENTLIST_SMART_LOCK"
				},
				{
					"num": 5,
					"title": "IP Cam",
					"translation": "#IPCAM"
				}
			]
		},
		"warables": {
			"title": "Wearables",
			"translation": "#CLIENTLIST_WEARABLES",
			"list": [
				{
					"num": 64,
					"title": "Smart Bracelet",
					"translation": "#CLIENTLIST_SMART_BRACELET"
				},
				{
					"num": 65,
					"title": "Watch",
					"translation": "#CLIENTLIST_WATCH"
				}
			]
		},
		"others": {
			"title": "Others",
			"translation": "#OTHERS",
			"list": [
				{
					"num": 66,
					"title": "Wall Switch",
					"translation": "#CLIENTLIST_WALL_SWITCH"
				},
				{
					"num": 67,
					"title": "Smart Door Lock",
					"translation": "#CLIENTLIST_WINDOW_SENSOR"
				},
				{
					"num": 68,
					"title": "Temperature Humidity Sensor",
					"translation": "#CLIENTLIST_TEMPERATURE_HUMIDITY_SENSOR"
				},
				{
					"num": 69,
					"title": "Body Sensor",
					"translation": "#CLIENTLIST_BODY_SENSOR"
				},
				{
					"num": 70,
					"title": "Smart plug",
					"translation": "#CLIENTLIST_SMART_PLUG"
				},
				{
					"num": 71,
					"title": "Robot",
					"translation": "#CLIENTLIST_ROBOT"
				}
			]
		}
	},
	gen_component : function(_paramObj){
		var custom_icon_db_translation_mapping = [
			{tag:"#CLIENTLIST_ELECTRONIC_DEVICES",text:"<#Clientlist_Electronic_Devices#>"},
			{tag:"#CLIENTLIST_DESKTOP",text:"<#Clientlist_Desktop#>"},
			{tag:"#CLIENTLIST_WINDOWS_DESKTOP",text:"<#Clientlist_Windows_Desktop#>"},
			{tag:"#CLIENTLIST_LINUX_DESKTOP",text:"<#Clientlist_Linux_Desktop#>"},
			{tag:"#CLIENTLIST_LINUX_DEVICE",text:"<#Clientlist_Linux_Device#>"},
			{tag:"#CLIENTLIST_SMARTPHONE",text:"<#Clientlist_Smartphone#>"},
			{tag:"#CLIENTLIST_IPHONE",text:"<#Clientlist_iPhone#>"},
			{tag:"#CLIENTLIST_ASUS_SMARTPHONE",text:"<#Clientlist_ASUS_smartphone#>"},
			{tag:"#CLIENTLIST_WINDOWS_PHONE",text:"<#Clientlist_Windows_Phone#>"},
			{tag:"#CLIENTLIST_ANDROID_PHONE",text:"<#Clientlist_Android_Phone#>"},
			{tag:"#CLIENTLIST_PAD",text:"<#Clientlist_Pad#>"},
			{tag:"#CLIENTLIST_IPAD",text:"<#Clientlist_iPad#>"},
			{tag:"#CLIENTLIST_ASUS_PAD",text:"<#Clientlist_ASUS_pad#>"},
			{tag:"#CLIENTLIST_ANDROID_TABLET",text:"<#Clientlist_Android_Tablet#>"},
			{tag:"#CLIENTLIST_REPEATER",text:"<#Clientlist_Repeater#>"},
			{tag:"#CLIENTLIST_NAS_SERVER",text:"<#Clientlist_NAS_Server#>"},
			{tag:"#CLIENTLIST_NB",text:"<#Clientlist_NB#>"},
			{tag:"#CLIENTLIST_WINDOWS_NB",text:"<#Clientlist_Windows_NB#>"},
			{tag:"#CLIENTLIST_MACBOOK",text:"<#Clientlist_Macbook#>"},
			{tag:"#Clientlist_ASUS_NB",text:"<#Clientlist_ASUS_NB#>"},
			{tag:"#CLIENTLIST_SD_CARD",text:"<#Clientlist_SD_Card#>"},
			{tag:"#CLIENTLIST_ROG_DEVICE",text:"<#Clientlist_ROG_Device#>"},
			{tag:"#CLIENTLIST_SCANNER",text:"<#Clientlist_Scanner#>"},
			{tag:"#CLIENTLIST_APPLE_DEVICE",text:"<#Clientlist_Apple_Device#>"},
			{tag:"#CLIENTLIST_ANDROID_DEVICE",text:"<#Clientlist_Android_Device#>"},
			{tag:"#CLIENTLIST_WINDOWS_DEVICE",text:"<#Clientlist_Windows_Device#>"},
			{tag:"#CLIENTLIST_ASUS_DEVICE",text:"<#Clientlist_ASUS_Device#>"},
			{tag:"#CLIENTLIST_MEDIA_ENTERTAINMENT",text:"<#Clientlist_Media_Entertainment#>"},
			{tag:"#CLIENTLIST_ELECTRONIC_EQUIPMENT",text:"<#Clientlist_Electronic_Equipment#>"},
			{tag:"#CLIENTLIST_AIR_CONDITIONER",text:"<#Clientlist_Air_Conditioner#>"},
			{tag:"#CLIENTLIST_REFRIGERATOR",text:"<#Clientlist_Refrigerator#>"},
			{tag:"#CLIENTLIST_WEIGHT_SCALE",text:"<#Clientlist_Weight_Scale#>"},
			{tag:"#CLIENTLIST_ELECTRIC_POT",text:"<#Clientlist_Electric_Pot#>"},
			{tag:"#CLIENTLIST_MICROWAVE_OVEN",text:"<#Clientlist_Microwave_Oven#>"},
			{tag:"#CLIENTLIST_AIR_PURIFIER",text:"<#Clientlist_Air_Purifier#>"},
			{tag:"#CLIENTLIST_FAN",text:"<#Clientlist_Fan#>"},
			{tag:"#CLIENTLIST_DEHUMIDIFIER",text:"<#Clientlist_Dehumidifier#>"},
			{tag:"#CLIENTLIST_WASHING_MACHINE",text:"<#Clientlist_Washing_Machine#>"},
			{tag:"#CLIENTLIST_WATER_HEATER",text:"<#Clientlist_Water_Heater#>"},
			{tag:"#CLIENTLIST_ELECTRIC_KETTLE",text:"<#Clientlist_Electric_Kettle#>"},
			{tag:"#CLIENTLIST_SMART_BULB",text:"<#Clientlist_Smart_Bulb#>"},
			{tag:"#CLIENTLIST_SMART_TV",text:"<#Clientlist_Smart_TV#>"},
			{tag:"#CLIENTLIST_CLEANING_ROBOT",text:"<#Clientlist_Cleaning_Robot#>"},
			{tag:"#CLIENTLIST_SECURITY",text:"<#Clientlist_Security#>"},
			{tag:"#CLIENTLIST_SEISMOGRAPH",text:"<#Clientlist_Seismograph#>"},
			{tag:"#CLIENTLIST_SMART_LOCK",text:"<#Clientlist_Smart_lock#>"},
			{tag:"#CLIENTLIST_WEARABLES",text:"<#Clientlist_Wearables#>"},
			{tag:"#CLIENTLIST_SMART_BRACELET",text:"<#Clientlist_Smart_bracelet#>"},
			{tag:"#CLIENTLIST_WATCH",text:"<#Clientlist_Watch#>"},
			{tag:"#CLIENTLIST_WALL_SWITCH",text:"<#Clientlist_Wall_switch#>"},
			{tag:"#CLIENTLIST_WINDOW_SENSOR",text:"<#Clientlist_Window_sensor#>"},
			{tag:"#CLIENTLIST_TEMPERATURE_HUMIDITY_SENSOR",text:"<#Clientlist_Temperature_humidity_sensor#>"},
			{tag:"#CLIENTLIST_BODY_SENSOR",text:"<#Clientlist_Body_sensor#>"},
			{tag:"#CLIENTLIST_SMART_PLUG",text:"<#Clientlist_Smart_plug#>"},
			{tag:"#CLIENTLIST_ROBOT",text:"<#Clientlist_Robot#>"},
			{tag:"#PRINTER",text:"<#Clientlist_Printer#>"},
			{tag:"#ROUTER",text:"<#Device_type_02_RT#>"},
			{tag:"#NAS",text:"<#Device_type_04_NS#>"},
			{tag:"#IPCAM",text:"<#Device_type_05_IC#>"}
		];
		var $container = _paramObj.container;
		var json_data = custom_icon_list_api.db;
		if(_paramObj.source == "cloud")
			json_data = _paramObj.db;
		$.each(json_data, function( index, value ) {
			var category = value;
			if(category.list != undefined){
				var $category_obj = "";
				if($container.find("#" + index + "").length == 0){
					$category_obj = $("<div>").addClass("custom_icon_category").attr("id",index);
					if(_paramObj.source == "cloud"){
						if($container.find(".custom_icon_category.upload").length > 0)
							$category_obj.insertBefore($container.find(".custom_icon_category.upload"));
						else
							$category_obj.appendTo($container);
					}
					else
						$category_obj.appendTo($container);
					var $title_obj = $("<div>").addClass("category_title");
					$title_obj.appendTo($category_obj);
					if(category.title != "")
						$title_obj.html(category.title);
					if(category.translation != "") {
						var specific_translation = custom_icon_db_translation_mapping.filter(function(item, index, _array){
							return (item.tag == category.translation);
						})[0];
						if(specific_translation != undefined)
							$title_obj.html(specific_translation.text);
					}
				}
				else
					$category_obj = $container.find("#" + index + "");

				if(category.list.length > 0){
					var total_count = category.list.length;
					var maxi_count = 7;

					var row_idx = $category_obj.children(".category_content").last().attr("row_idx");
					if(row_idx == undefined)
						row_idx = 0;

					$.each(category.list, function( index, value ){
						var $content_obj = "";
						if($category_obj.find(".category_content[row_idx=" + row_idx + "]").length == 0){
							$content_obj = $("<div>").addClass("category_content").attr("row_idx", row_idx);
						}
						else{
							$content_obj = $category_obj.find(".category_content[row_idx=" + row_idx + "]");
							if($content_obj.children().length >= maxi_count){
								row_idx++;
								$content_obj = $("<div>").addClass("category_content").attr("row_idx", row_idx);
							}
						}

						var icon = value;
						var src = "";
						if(_paramObj.source == "cloud"){
							if(icon.src != undefined && icon.src != "")
								src = icon.src
							else
								return;
						}
						if($category_obj.children(".category_content").children(".type" + icon.num + "").length > 0)
							return;

						var $item_obj = $("<div>").append($("<i>").addClass("type" + icon.num + ""));
						if(icon.title != "")
							$item_obj.attr("title", icon.title);
						if(icon.translation != ""){
							var specific_translation = custom_icon_db_translation_mapping.filter(function(item, index, _array){
								return (item.tag == icon.translation);
							})[0];
							if(specific_translation != undefined){
								$item_obj.attr("title", specific_translation.text);
							}
						}
						if(_paramObj.source == "cloud" && src != ""){
							$item_obj.find('i').get(0).style.setProperty("--svg", "url(" + src + ")");
						}

						$content_obj.appendTo($category_obj);
						$item_obj.appendTo($content_obj);
						$item_obj.unbind("click");
						$item_obj.click(function(e){
							e = e || event;
							if(_paramObj.select_icon_callBack)
								_paramObj.select_icon_callBack($(this));
						});
					});
				}
			}
		});

		if((_paramObj.source == "local") && isSupport("usericon") && !isSupport("sfp4m")) {
			$upload_category = $("<div>").addClass("custom_icon_category upload");
			$upload_category.appendTo($container);
			$upload_category.append($("<div>").addClass("category_title").html("<#option_upload#>"));
			var $upload_icon = $("<div>").addClass("client_upload_div").html("+");
			var $input_file = $("<input/>").attr({"type":"file", "title":"Upload client icon"}).addClass("client_upload_file");/* untranslated */
			$input_file.appendTo($upload_icon);
			$upload_category.append($("<div>").addClass("category_content").append($upload_icon));
			$input_file.on("change", function(){if(_paramObj.upload_callBack) _paramObj.upload_callBack($(this));});
		}
	}
}

/* Exported from device-map/clients.asp */

function retOverLibStr(client){
	var overlibStr = "<p><#MAC_Address#>:</p>" + client.mac.toUpperCase();

	if(client.ssid)
		overlibStr += "<p>SSID:</p>" + client.ssid.replace(/"/g, '&quot;');
	if(client.isLogin)
		overlibStr += "<p>Logged In User:</p>YES";
	if(client.isPrinter)
		overlibStr += "<p><#Device_service_Printer#></p>YES";
	if(client.isITunes)
		overlibStr += "<p><#Device_service_iTune#></p>YES";
	if(client.isWL > 0){
		overlibStr += "<p><#Wireless_Radio#>:</p>" + isWL_map[client.isWL]["text"].replace("G", " GHz") + " (" + client.rssi + " dBm)";
		if(stainfo_support) {
			overlibStr += "<p>Tx Rate:</p>" + ((client.curTx != "") ? client.curTx : "-");
			overlibStr += "<p>Rx Rate:</p>" + ((client.curRx != "") ? client.curRx : "-");
			overlibStr += "<p><#Access_Time#>:</p>" + client.wlConnectTime;
		}
	}
	return overlibStr;
}

function ajaxCallJsonp(target){    
    var data = $j.getJSON(target, {format: "json"});

    data.success(function(msg){
    	parent.retObj = msg;
		parent.db("Success!");
    });

    data.error(function(msg){
		parent.db("Error on fetch data!")
    });
}

function oui_query_full_vendor(mac){
	if(clientList[mac] != undefined && clientList[mac].vendor != "") {
		setTimeout(function(){
			var overlibStrTmp = retOverLibStr(clientList[mac]);
			overlibStrTmp += "<p><span>.....................................</span></p><p style='margin-top:5px'><#Manufacturer#> :</p>";
			overlibStrTmp += clientList[mac].vendor;
			return overlib(overlibStrTmp);
		}, 1);
	}
	else {
			var queryStr = mac.replace(/\:/g, "").splice(6,6,"");
			if (clientList[mac] != undefined)
				var overlibStrTmp = retOverLibStr(clientList[mac]);
			else
				var overlibStrTmp = "<p><#MAC_Address#>:</p>" + mac.toUpperCase();
			$.getJSON("/ajax/ouiDB.json", function(data){
				if(data != "" && data[queryStr] != undefined){
					if(overlib.isOut) return nd();
					var vendor_name = data[queryStr].trim();
					overlibStrTmp += "<p><span>.....................................</span></p><p style='margin-top:5px'><#Manufacturer#> :</p>";
					overlibStrTmp += vendor_name;
					return overlib(overlibStrTmp);
				}
			});
	}
}

function clientFromIP(ip) {
	for(var i=0; i<clientList.length;i++){
		var clientObj = clientList[clientList[i]];
		if(clientObj.ip == ip) return clientObj;
	}
	return 0;
}

/* End exported functions */

