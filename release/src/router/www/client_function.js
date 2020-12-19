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

/* ouiDB lookup code */
var ouiClientListArray = new Array();
ouiClientListArray = Session.get("ouiDB");
if(ouiClientListArray == undefined) {
	ouiClientListArray = [];
	//Download OUI DB
	setTimeout(function() {
		var ouiBDjs = document.createElement("script");
		ouiBDjs.type = "application/javascript";
		ouiBDjs.src = "/js/ouiDB.js";
		window.document.body.appendChild(ouiBDjs);
	}, 1000);
}

function updateManufacturer(_ouiDBArray) {
	ouiClientListArray = [];
	ouiClientListArray = _ouiDBArray;
	Session.set("ouiDB", _ouiDBArray);
}

/* End ouiDB lookup code */

var ipState = new Array();
ipState["Static"] = "<#BOP_ctype_title5#>";
ipState["DHCP"] = "<#BOP_ctype_title1#>";
ipState["Manual"] = "<#Clientlist_IPMAC_Binding#>";
ipState["OffLine"] = "<#Clientlist_OffLine_Hint#>";

var venderArrayRE = /(adobe|amazon|apple|asus|belkin|bizlink|buffalo|dell|d-link|fujitsu|google|hon hai|htc|huawei|ibm|lenovo|nec|microsoft|panasonic|pioneer|ralink|samsung|sony|synology|toshiba|tp-link|vmware)/;

var networkmap_fullscan = '<% nvram_get("networkmap_fullscan"); %>';

var originDataTmp;
var originData = {
	fromNetworkmapd : [<% get_clientlist(); %>],
	nmpClient : [<% get_clientlist_from_json_database(); %>], //Record the client connected to the router before.
	init: true
}

var totalClientNum = {
	online: 0,
	wireless: 0,
	wired: 0,
	wireless_ifnames: []
}

var AiMeshTotalClientNum = [];

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
	this.ROG = false;
}

var clientList = new Array(0);
function genClientList(){
	clientList = [];
	totalClientNum.online = 0;
	totalClientNum.wired = 0;
	totalClientNum.wireless = 0;
	AiMeshTotalClientNum = [];
	for(var i=0; i<wl_nband_title.length; i++) totalClientNum.wireless_ifnames[i] = 0;
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

			if(!downsize_4m_support) {
				clientList[thisClientMacAddr].type = thisClient.type;
				clientList[thisClientMacAddr].defaultType = thisClient.defaultType;
			}
			
			clientList[thisClientMacAddr].ip = thisClient.ip;
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
			if(isSupport("amas"))
				clientList[thisClientMacAddr].isGN = ((thisClient.isGN != "") ? parseInt(thisClient.isGN) : "");
			if(amesh_support && isSupport("dualband") && clientList[thisClientMacAddr].isWL == 3)
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
			clientList[thisClientMacAddr].qosLevel = thisClient.qosLevel;
			clientList[thisClientMacAddr].wtfast = parseInt(thisClient.wtfast);
			clientList[thisClientMacAddr].internetMode = thisClient.internetMode;
			clientList[thisClientMacAddr].internetState = thisClient.internetState;
			if(stainfo_support) {
				clientList[thisClientMacAddr].curTx = (thisClient.curTx == "") ? "": thisClient.curTx;
				clientList[thisClientMacAddr].curRx = (thisClient.curRx == "") ? "": thisClient.curRx;
				clientList[thisClientMacAddr].wlConnectTime = thisClient.wlConnectTime;
			}

			if(amesh_support) {
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
				var thisClientNickName = (typeof thisClient.nickName == "undefined") ? "" : (thisClient.nickName.trim() == "") ? "" : thisClient.nickName.trim();
				var thisClientReNode = (typeof thisClient.amesh_isRe == "undefined") ? false : ((thisClient.amesh_isRe == "1") ? true : false);

				clientList.push(thisClientMacAddr);
				clientList[thisClientMacAddr] = new setClientAttr();
				clientList[thisClientMacAddr].from = thisClient.from;
				if(!downsize_4m_support) {
					clientList[thisClientMacAddr].type = thisClientType;
					clientList[thisClientMacAddr].defaultType = thisClientDefaultType;
				}
				clientList[thisClientMacAddr].mac = thisClientMacAddr;
				clientList[thisClientMacAddr].name = thisClientName;
				clientList[thisClientMacAddr].nickName = thisClientNickName;
				clientList[thisClientMacAddr].vendor = thisClient.vendor.trim();
				if(amesh_support) {
					clientList[thisClientMacAddr].amesh_isRe = thisClientReNode;
					if(isSupport("force_roaming") && isSupport("sta_ap_bind")) {
						clientList[thisClientMacAddr].amesh_bind_mac = (typeof thisClient.amesh_bind_mac == "undefined") ? "" : thisClient.amesh_bind_mac;
						clientList[thisClientMacAddr].amesh_bind_band = (typeof thisClient.amesh_bind_band == "undefined") ? "0" : thisClient.amesh_bind_band;
					}
				}

				clientList[thisClientMacAddr].ROG = (thisClient.ROG == "1");
				nmpCount++;
			}
			else if(!clientList[thisClientMacAddr].isOnline) {
				clientList[thisClientMacAddr].from = thisClient.from;
				nmpCount++;
			}
		}
	}

	//initial Gateway client
	if(amesh_support) {
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
	$.ajax({
		url: '/ajax_uploadicon.asp?clientmac=' + clientMac,
		async: false,
		dataType: 'script',
		error: function(xhr){
			setTimeout("getUploadIcon('" + clientMac + "');", 1000);
		},
		success: function(response){
			result = htmlEnDeCode.htmlEncode(upload_icon);
		}
	});
	return result
}

function getUploadIconCount() {
	var count = 0;
	$.ajax({
		url: '/ajax_uploadicon.asp',
		async: false,
		dataType: 'script',
		error: function(xhr){
			setTimeout("getUploadIconCount();", 1000);
		},
		success: function(response){
			count = upload_icon_count;
		}
	});
	return count
}

function getUploadIconList() {
	var list = "";
	$.ajax({
		url: '/ajax_uploadicon.asp',
		async: false,
		dataType: 'script',
		error: function(xhr){
			setTimeout("getUploadIconList();", 1000);
		},
		success: function(response){
			list = upload_icon_list;
		}
	});
	return list
}


function getVenderIconClassName(venderName) {
	var vender_class_name = "";
	var match_data = venderName.match(venderArrayRE);
	if(Boolean(match_data) && match_data[0] != undefined) {
		vender_class_name = match_data[0];
		if(vender_class_name == "hon hai")
			vender_class_name = "honhai";
	}
	else {
		vender_class_name = "";
	}
	return vender_class_name;
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
		else if(temp_clickedObj.className.search("venderIcon") != -1) {
			temp_clickedObj.className = temp_clickedObj.className.replace("venderIcon_clicked","venderIcon");
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
	if(!downsize_4m_support)
		code += '<div id="card_client_preview_icon" class="client_preview_icon" title="Change client icon" onclick="card_show_custom_image();">';
	else
		code += '<div id="card_client_preview_icon" class="client_preview_icon" title="Change client icon">';
	code += '<div id="card_client_image" style="width:85px;height:85px;margin:0 auto;cursor:pointer;"></div>';
	code += '<canvas id="card_canvas_user_icon" class="client_canvasUserIcon" width="85px" height="85px"></canvas>';
	code += '</div>';
	if(!downsize_4m_support) {
		code += '<div class="changeClientIcon">';
		code += '<span title="Change to default client icon" onclick="card_setDefaultIcon();"><#CTL_Default#></span>';
		code += '<span id="card_changeIconTitle" title="Change client icon" style="margin-left:10px;" onclick="card_show_custom_image();"><#CTL_Change#></span>';
		code += '</div>';
	}
	code += '</td>';

	code += '<td style="vertical-align:top;text-align:center;">';
	code += '<div class="clientTitle">';
	code += '<#Clientlist_name#>';
	code += '</div>';
	code += '<div  class="clientTitle" style="margin-top:10px;">';
	code += 'IP';
	code += '</div>';
	code += '<div  class="clientTitle" style="margin-top:10px;">';
	code += 'MAC';
	code += '</div>';
	code += '<div  class="clientTitle" style="margin-top:10px;">';
	code += '<#Clientlist_device#>';
	code += '</div>';
	code += '</td>';

	code += '<td style="vertical-align:top;width:280px;">';

	code += '<div>';
	code += '<input id="card_client_name" name="card_client_name" type="text" value="" class="input_32_table" maxlength="32" style="width:275px;">';
	code += '</div>';
	code += '<div style="margin-top:10px;">';
	code += '<input id="card_client_ipaddr_field_orig" type="hidden" value="" disabled="">';
	code += '<input id="card_client_ipaddr_field" type="text" value="" class="input_32_table client_input_text_disabled" disabled>';
	code += '</div>';
	code += '<div style="margin-top:10px;">';
	code += '<input id="card_client_macaddr_field" type="text" value="" class="input_32_table client_input_text_disabled" disabled>';
	code += '</div>';
	code += '<div style="margin-top:10px;">';
	code += '<input id="card_client_manufacturer_field" type="text" value="Loading manufacturer.." class="input_32_table client_input_text_disabled" disabled>';
	code += '</div>';
	code += '</td>';
	code += '</tr>';
	//device icon and device info. end

	//device icon list start
	code += '<tr>';
	code += '<td colspan="3">';
	code += '<div id="card_custom_image" class="client_icon_list" style="display:none;">';
	code += '<table width="99%;" id="tbCardClientListIcon" border="1" align="center" cellpadding="4" cellspacing="0">';
	code += '</table>';
	code += '</td>';
	code += '</tr>';
	//device icon list end

	//adv setting start
	if(adv_setting) {
		var block_internet_hint = "Enable this button to block this device to access internet.";/* untranslated */
		var time_scheduling_hint = "Time Scheduling allows you to set the time limit for a client's network usage.";/* untranslated */
		var ip_binding_hint = "Enable this button to bind specific IP with MAC Address of this device.";/* untranslated */
		var internetTimeScheduling_title = (bwdpi_support) ? "<#Time_Scheduling#>" : "<#Parental_Control#>";
		code += '<tr>';
		code += '<td colspan="3">';
		code += '<div class="clientList_line"></div>';
		code += '<div style="height:33px;width:100%;margin:5px 0;">';
		code += '<div style="width:65%;float:left;line-height:33px;">';
		code += "<span onmouseover='return overlib(\"" + htmlEnDeCode.htmlEncode(block_internet_hint) +"\");' onmouseout='return nd();'><#Clientlist_block_internet#></span>";
		code += '</div>';
		code += '<div class="left" style="cursor:pointer;float:right;" id="card_radio_BlockInternet_enable"></div>';
		code += '</div>';
		code += '<div class="clientList_line"></div>';
		code += '<div style="height:33px;width:100%;margin:5px 0;">';
		code += '<div style="width:65%;float:left;line-height:33px;">';
		code += "<span onmouseover='return overlib(\"" + htmlEnDeCode.htmlEncode(time_scheduling_hint) + "\");' onmouseout='return nd();'>" + internetTimeScheduling_title + "</span>";
		code += '</div>';
		code += '<div align="center" class="left" style="cursor:pointer;float:right;" id="card_radio_TimeScheduling_enable"></div>';
		code += '<div id="card_internetTimeScheduling" class="internetTimeEdit" style="float:right;margin-right:10px;" title="<#Time_Scheduling#>"></div>';
		code += '</div>';
		code += '<div class="clientList_line"></div>';
		code += '<div style="height:33px;width:100%;margin:5px 0;">';
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
	code += '<input class="button_gen" type="button" style="margin-right:5px;" onclick="card_closeClientListView();" value="<#CTL_Cancel#>">';
	code += '<input id="card_client_confirm" class="button_gen" type="button" value="<#CTL_apply#>">';
	code += '<img id="card_client_loadingIcon" style="margin-left:5px;display:none;" src="/images/InternetScan.gif">';
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
		else if(temp_clickedObj.className.search("venderIcon") != -1) {
			temp_clickedObj.className = temp_clickedObj.className.replace("venderIcon_clicked","venderIcon");
			temp_clickedObj.className = temp_clickedObj.className.replace(" card_clicked", "");
		}
		temp_clickedObj = null;
	}
	temp_clickedObj = obj;
	if(obj.className.search("clientIcon") != -1) {
		obj.className = obj.className.replace("clientIcon","clientIcon_clicked");
		obj.className = obj.className  + " card_clicked";
	}
	else if(obj.className.search("venderIcon") != -1) {
		obj.className = obj.className.replace("venderIcon","venderIcon_clicked");
		obj.className = obj.className  + " card_clicked";
	}
	
	fadeIn(document.getElementById("edit_client_block"));
	if(panel_block_top_value)
		adjust_panel_block_top("edit_client_block", panel_block_top_value);
	document.body.onclick = function() {hide_edit_client_block();}
	document.getElementById("edit_client_block").onclick = function() {show_edit_client_block();}

	//build device icon list start
	var clientListIconArray = [["Windows device", "1"], ["Router", "2"], ["NAS/Server", "4"], ["IP Cam", "5"], ["Macbook", "6"], ["Game Console", "7"], ["Android Phone", "9"], ["iPhone", "10"], 
	["Apple TV", "11"], ["Set-top Box", "12"], ["iMac", "14"], ["ROG", "15"], ["Printer", "18"], ["Windows Phone", "19"], ["Android Tablet", "20"], ["iPad", "21"], ["Linux Device", "22"], 
	["Smart TV", "23"], ["Repeater", "24"], ["Kindle", "25"], ["Scanner", "26"], ["Chromecast", "27"], ["ASUS smartphone", "28"], ["ASUS Pad", "29"], ["Windows", "30"], ["Android", "31"], ["Mac OS", "32"],
	["Windows laptop", "35"]];

	var eachColCount = 7;
	var colCount = parseInt(clientListIconArray.length / eachColCount);
	if(usericon_support)
		colCount++;

	for(var rowIndex = 0; rowIndex < colCount; rowIndex += 1) {
		var row = document.getElementById("tbCardClientListIcon").insertRow(rowIndex);
		row.id = "trCardClientListIcon" + rowIndex;
		for(var colIndex = 0; colIndex < eachColCount; colIndex += 1) {
			var cell = row.insertCell(-1);
			cell.id = "tbCardClientListIcon" + (colIndex + (rowIndex * 7));
			var idx = (colIndex + (rowIndex * eachColCount));
			if(clientListIconArray[colIndex + (rowIndex * eachColCount)] != undefined) {
				cell.innerHTML = '<div class="type' + clientListIconArray[idx][1] + '" onclick="select_image(this.className,\''+clientInfo.vendor.replace("'","\\'")+'\');"  title="' + clientListIconArray[idx][0] + '"></div>';
			}
			else {
				if(usericon_support) {
					if(document.getElementById("tdCardUserIcon") == null) {
						cell.id = "tdCardUserIcon";
						cell.className = "client_icon_list_td";
						cell.innerHTML = '<div id="divCardUserIcon" class="client_upload_div" style="display:none;">+' +
						'<input type="file" name="cardUploadIcon" id="cardUploadIcon" class="client_upload_file" onchange="previewCardUploadIcon(this);" title="Upload client icon" /></div>';/*untranslated*/
						break;
					}
				}
			}
		}
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
			if(stainfo_support) {
				if(clientInfo.curTx != "")
					connectModeTip += "Tx Rate: " + clientInfo.curTx + "\n"; /*untranslated*/
				if(clientInfo.curRx != "")
					connectModeTip += "Rx Rate: " + clientInfo.curRx + "\n"; /*untranslated*/
				connectModeTip += "<#Access_Time#>: " + clientInfo.wlConnectTime + "";
			}
		}

		if(sw_mode != 4){
			var radioIcon_css = "radioIcon";
			if(clientInfo.isGN != "" && clientInfo.isGN != undefined)
				radioIcon_css += " GN";
			clientIconHtml += '<div class="' + radioIcon_css + ' radio_' + rssi_t +'" title="' + connectModeTip + '"></div>';
			if(clientInfo.isWL != 0) {
				var bandClass = (navigator.userAgent.toUpperCase().match(/CHROME\/([\d.]+)/)) ? "band_txt_chrome" : "band_txt";
				clientIconHtml += '<div class="band_block"><span class="' + bandClass + '" style="color:#000000;">' + wl_nband_title[clientInfo.isWL-1].replace("Hz", "").replace(/\s*/g,"") + '</span></div>';
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
	//device title info. end

	//device icon and device info. start
	document.getElementById("card_client_ipaddr_field_orig").value = clientInfo.ip;
	document.getElementById("card_client_ipaddr_field").value = clientInfo.ip;
	document.getElementById("card_client_macaddr_field").value = clientInfo.mac;
	select_image("type" + parseInt(clientInfo.type), clientInfo.vendor);
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
		$("#edit_client_block #card_client_ipaddr_field").css("width", "275px");
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
			switch (mode) {
				case "ipBinding" :
					$('#edit_client_block #card_radio_IPBinding_enable').iphoneSwitch(state,
						function(){
							if(card_client_variable.manual_dhcp_list[mac] == undefined) {
								if(manual_dhcp_list_num == 64) {
									if(confirm("The max limit is 64 rule. Please check your client list on DHCP server.")) { /*untranslated*/
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
								if(client_MULTIFILTER_num == 16) {
									if(confirm("The max limit is 16 clients. Please check your client list on time scheduling.")) { /*untranslated*/
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
								if(client_MULTIFILTER_num == 16) {
									if(confirm("The max limit is 16 clients. Please check your client list on time scheduling.")) { /*untranslated*/
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
	if(usericon_support) {
		//2.check browswer support File Reader and Canvas or not.
		if(isSupportFileReader() && isSupportCanvas()) {
			document.getElementById("divCardUserIcon").style.display = "";
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
							var mimeType = img.src.split(",")[0].split(":")[1].split(";")[0];
							var canvas = document.getElementById("card_canvas_user_icon");
							var ctx = canvas.getContext("2d");
							ctx.clearRect(0,0,85,85);
							document.getElementById("card_client_image").style.display = "none";
							document.getElementById("card_canvas_user_icon").style.display = "";
							setTimeout(function() {
								ctx.drawImage(img, 0, 0, 85, 85);
								var dataURL = canvas.toDataURL(mimeType);
								card_client_variable.userIconBase64 = dataURL;
							}, 100); //for firefox FPS(Frames per Second) issue need delay
						};
						reader.readAsDataURL(file);
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
function previewCardUploadIcon(imageObj) {
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
		if (!checkImageExtension(imageObj.value)) {
			alert("<#Setting_upload_hint#>");
			imageObj.focus();
		}
		else {
			//2.Re-drow image
			var fileReader = new FileReader(); 
			fileReader.onload = function (fileReader) {
				var img = document.createElement("img");
				img.src = fileReader.target.result;
				var mimeType = img.src.split(",")[0].split(":")[1].split(";")[0];
				var canvas = document.getElementById("card_canvas_user_icon");
				var ctx = canvas.getContext("2d");
				ctx.clearRect(0,0,85,85);
				document.getElementById("card_client_image").style.display = "none";
				document.getElementById("card_canvas_user_icon").style.display = "";
				setTimeout(function() {
					ctx.drawImage(img, 0, 0, 85, 85);
					var dataURL = canvas.toDataURL(mimeType);
					card_client_variable.userIconBase64 = dataURL;
				}, 100); //for firefox FPS(Frames per Second) issue need delay
			}
			fileReader.readAsDataURL(imageObj.files[0]);
			userIconHideFlag = true;
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
				else if(ip_num <= getSubnet('<% nvram_get("lan_ipaddr"); %>', '<% nvram_get("lan_netmask"); %>', "head") ||
					 ip_num >= getSubnet('<% nvram_get("lan_ipaddr"); %>', '<% nvram_get("lan_netmask"); %>', "end")){
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

		if(utf8_ssid_support){
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
		if(document.getElementById('card_client_image').className.search("venderIcon") != -1) {
			clientTypeNum = "0";
		}
		else {
			clientTypeNum = document.getElementById("card_client_image").className.replace("clientIcon_no_hover type", "");
			if(clientTypeNum == "0_viewMode") {
				clientTypeNum = "0";
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
							document.card_clientlist_form.MULTIFILTER_ENABLE.value += "1";
						else if(card_client_variable.blockInternetFlag)
							document.card_clientlist_form.MULTIFILTER_ENABLE.value += "2";
						document.card_clientlist_form.MULTIFILTER_MAC.value += clientMac;
						document.card_clientlist_form.MULTIFILTER_DEVICENAME.value += clientName;
						if(isSupport("PC_SCHED_V3"))
							document.card_clientlist_form.MULTIFILTER_MACFILTER_DAYTIME_V2.value += "W03E21000700<W04122000800";
						else
							document.card_clientlist_form.MULTIFILTER_MACFILTER_DAYTIME.value += "<";
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
		if(usericon_support) {
			document.card_clientlist_form.custom_usericon.disabled = false;
			var clientMac = document.getElementById("card_client_macaddr_field").value.replace(/\:/g, "");
			if(card_client_variable.userIconBase64 != "NoIcon") {
				document.card_clientlist_form.custom_usericon.value = clientMac + ">" + card_client_variable.userIconBase64;
			}
			else {
				document.card_clientlist_form.custom_usericon.value = clientMac + ">noupload";
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
								case "DNSFilter" :
									showDropdownClientList('setclientmac', 'name>mac', 'all', 'ClientList_Block_PC', 'pull_arrow', 'all');
									show_dnsfilter_list();
									break;
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
	var defaultType = "0";
	var defaultDpiVender = "";
	if(clientList[mac] != undefined) {
		defaultType = clientList[mac].defaultType;
		defaultDpiVender = clientList[mac].vendor;
	}
	select_image("type" + defaultType, defaultDpiVender);
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
	confirmFlag = confirm("The client icon over upload limting, please remove at least one client icon then try to upload again."); /*untranslated*/
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
		code +='<tr><td style="color:#FFCC00;" colspan="4"><#IPConnection_VSList_Norule#></td></tr>';
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
		code +='<tr><td style="color:#FFCC00;" colspan="4"><#IPConnection_VSList_Norule#></td></tr>';
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

function select_image(type,  vender) {
	var sequence = type.substring(4,type.length);
	document.getElementById("card_client_image").style.display = "none";
	document.getElementById("card_canvas_user_icon").style.display = "none";
	var icon_type = type;
	if(type == "type0") {
		icon_type = "type0_viewMode";
	}
	document.getElementById("card_client_image").style.backgroundSize = "";
	document.getElementById("card_client_image").className = "clientIcon_no_hover "+ icon_type;
	if(vender != "" && type == "type0" && !downsize_4m_support) {
		var venderIconClassName = getVenderIconClassName(vender.toLowerCase());
		if(venderIconClassName != "") {
			document.getElementById("card_client_image").className = "venderIcon_no_hover "+ venderIconClassName;
			document.getElementById("card_client_image").style.backgroundSize = "180%";
		}
	}

	var userImageFlag = false;
	if(!card_client_variable.firstTimeOpenBlock) {
		if(usericon_support) {
			var clientMac = document.getElementById('card_client_macaddr_field').value.replace(/\:/g, "");
			card_client_variable.userIconBase64 = getUploadIcon(clientMac);
			if(card_client_variable.userIconBase64 != "NoIcon") {
				var img = document.createElement("img");
				img.src = card_client_variable.userIconBase64;
				var canvas = document.getElementById("card_canvas_user_icon");
				var ctx = canvas.getContext("2d");
				ctx.clearRect(0,0,85,85);
				document.getElementById("card_client_image").style.display = "none";
				document.getElementById("card_canvas_user_icon").style.display = "";
				ctx.drawImage(img, 0, 0, 85, 85);
				userImageFlag = true;
			}
		}
	}

	if(!userImageFlag) {
		card_client_variable.userIconBase64 = "NoIcon";
		document.getElementById("card_client_image").style.display = "";
	}
}

 function oui_query_card(mac) {
	var queryStr = mac.replace(/\:/g, "").splice(6,6,"");

	if(mac != document.getElementById("card_client_macaddr_field").value) //avoid click two device quickly
		oui_query_card(document.getElementById("card_client_macaddr_field").value);
	else{
		$.getJSON("http://nw-dlcdnet.asus.com/plugin/js/ouiDB.json", function(data){
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
var wired_list = new Array();
var wl1_list = new Array();//2.4G
var wl2_list = new Array();//5G
var wl3_list = new Array();//5G-2

var sorter = {
	"indexFlag" : 3 , // default sort is by IP
	"all_index" : 3,
	"all_display" : true,
	"wired_index" : 3,
	"wired_display" : true,
	"wl1_index" : 3,
	"wl1_display" : true,
	"wl2_index" : 3,
	"wl2_display" : true,
	"wl3_index" : 3,
	"wl3_display" : true,
	"sortingMethod" : "increase", 
	"sortingMethod_wired" : "increase", 
	"sortingMethod_wl1" : "increase", 
	"sortingMethod_wl2" : "increase", 
	"sortingMethod_wl3" : "increase", 

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
		else {
			return parseInt(a[sorter.indexFlag]) - parseInt(b[sorter.indexFlag]);
		}
	},
	"num_decrease" : function(a, b) {
		var a_num = 0, b_num = 0;
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
		var objIndex = obj;
		var clickItem = obj.parentNode.id.split("_")[1];
		var sorterLastIndex = 0;
		var sorterClickIndex = 0;
		while( (objIndex = objIndex.previousSibling) != null ) 
			sorterClickIndex++;

		switch (clickItem) {
			case "all" :
				sorterLastIndex = sorter.all_index;
				sorter.all_index = sorterClickIndex;
				sorter.sortingMethod = (sorter.sortingMethod == "increase") ? "decrease" : "increase";
				break;
			case "wired" :
				sorterLastIndex = sorter.wired_index;
				sorter.wired_index = sorterClickIndex;
				sorter.sortingMethod_wired = (sorter.sortingMethod_wired == "increase") ? "decrease" : "increase";
				break;
			case "wl1" :
				sorterLastIndex = sorter.wl1_index;
				sorter.wl1_index = sorterClickIndex;
				sorter.sortingMethod_wl1 = (sorter.sortingMethod_wl1 == "increase") ? "decrease" : "increase";
				break;
			case "wl2" :
				sorterLastIndex = sorter.wl2_index;
				sorter.wl2_index = sorterClickIndex;
				sorter.sortingMethod_wl2 = (sorter.sortingMethod_wl2 == "increase") ? "decrease" : "increase";
				break;
			case "wl3" :
				sorterLastIndex = sorter.wl3_index;
				sorter.wl3_index = sorterClickIndex;
				sorter.sortingMethod_wl3 = (sorter.sortingMethod_wl3 == "increase") ? "decrease" : "increase";
				break;	
		}
		if(isSupport("amas") && clickItem.substr(0,2) == "gn"){
			sorterLastIndex = sorter[""+clickItem+"_index"];
			sorter[""+clickItem+"_index"] = sorterClickIndex;
			sorter["sortingMethod_"+clickItem+""] = (sorter["sortingMethod_"+clickItem+""] == "increase") ? "decrease" : "increase";
		}
		obj.parentNode.childNodes[sorterLastIndex].style.borderTop = '1px solid #222';
		obj.parentNode.childNodes[sorterLastIndex].style.borderBottom = '1px solid #222';	
	},
	"drawBorder" : function(_arrayName) {
		var clickItem = _arrayName.split("_")[0];
		var clickIndex = 2;
		var clickSortingMethod = "increase";
		switch (clickItem) {
			case "all" :
				clickIndex = sorter.all_index;
				clickSortingMethod = sorter.sortingMethod;
				break;
			case "wired" :
				clickIndex = sorter.wired_index;
				clickSortingMethod = sorter.sortingMethod_wired;
				break;
			case "wl1" :
				clickIndex = sorter.wl1_index;
				clickSortingMethod = sorter.sortingMethod_wl1;
				break;
			case "wl2" :
				clickIndex = sorter.wl2_index;
				clickSortingMethod = sorter.sortingMethod_wl2;
				break;
			case "wl3" :
				clickIndex = sorter.wl3_index;
				clickSortingMethod = sorter.sortingMethod_wl3;
				break;
		}
		if(isSupport("amas") && _arrayName.substr(0,2) == "gn"){
			clickIndex = sorter[""+_arrayName.substr(0,3)+"_index"];
			clickSortingMethod = sorter["sortingMethod_"+_arrayName.substr(0,3)+""];
		}
		var borderTopCss = "";
		var borderBottomCss = "";
		if(getBrowser_info().ie != undefined || getBrowser_info().ie != null) {
			borderTopCss = "3px solid #FC0";
			borderBottomCss = "1px solid #FC0";
		}
		else if(getBrowser_info().firefox != undefined || getBrowser_info().firefox != null) {
			borderTopCss = "2px solid #FC0";
			borderBottomCss = "1px solid #FC0";
		}
		else {
			borderTopCss = "2px solid #FC0";
			borderBottomCss = "2px solid #FC0";
		}
		if(clickSortingMethod == "increase") {
			document.getElementById("tr_"+clickItem+"_title").childNodes[clickIndex].style.borderTop = borderTopCss;
			document.getElementById("tr_"+clickItem+"_title").childNodes[clickIndex].style.borderBottom = '1px solid #222';
		}
		else {
			document.getElementById("tr_"+clickItem+"_title").childNodes[clickIndex].style.borderTop = '1px solid #222';
			document.getElementById("tr_"+clickItem+"_title").childNodes[clickIndex].style.borderBottom = borderBottomCss;
		}
	},
	"doSorter" : function(_flag, _Method, _arrayName) {	
		// update variables
		sorter.indexFlag = _flag;
		
		// doSorter
		if(clienlistViewMode == "All") {
			eval(""+_arrayName+".sort(sorter."+_Method+"_"+sorter.sortingMethod+");");
		}
		else if(clienlistViewMode == "ByInterface") {
			switch (_arrayName) {
				case "wired_list" :
					eval(""+_arrayName+".sort(sorter."+_Method+"_"+sorter.sortingMethod_wired+");");
					break;
				case "wl1_list" :
					eval(""+_arrayName+".sort(sorter."+_Method+"_"+sorter.sortingMethod_wl1+");");
					break;
				case "wl2_list" :
					eval(""+_arrayName+".sort(sorter."+_Method+"_"+sorter.sortingMethod_wl2+");");
					break;
				case "wl3_list" :
					eval(""+_arrayName+".sort(sorter."+_Method+"_"+sorter.sortingMethod_wl3+");");
					break;
			}
			if(isSupport("amas") && _arrayName.substr(0,2) == "gn"){
				eval("gn_list['"+_arrayName.substr(0,3)+"'].sort(sorter."+_Method+"_"+sorter["sortingMethod_"+_arrayName.substr(0,3)+""]+");");
			}
		}
		drawClientListBlock(_arrayName);
		sorter.drawBorder(_arrayName);
	}
}

if(isSupport("amas")){
	var gn_list = [];
	for(var i=1; i<multissid_count+1; i++){
		gn_list["gn" + i + ""] = new Array();
		sorter["gn" + i + "_index"] = 3;
		sorter["gn" + i + "_display"] = true;
		sorter["sortingMethod_gn" + i + ""] = "increase";
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
	sorter.wired_display = true;
	sorter.wl1_display = true;
	sorter.wl2_display = true;
	sorter.wl3_display = true;
	if(isSupport("amas")){
		for(var i=1; i<multissid_count+1; i++)
			sorter["gn" + i + "_display"] = true;
	}
}

var interval_clientlist_listview_update = null;
function pop_clientlist_listview() {
	if(document.getElementById("clientlist_viewlist_content") != null) {
		removeElement(document.getElementById("clientlist_viewlist_content"));
	}

	var divObj = document.createElement("div");
	divObj.setAttribute("id","clientlist_viewlist_content");
	divObj.className = "clientlist_viewlist";
	divObj.setAttribute("onselectstart","return false");
	document.body.appendChild(divObj);
	fadeIn(document.getElementById("clientlist_viewlist_content"));
	cal_panel_block_clientList("clientlist_viewlist_content", 0.045);

	if(document.getElementById("view_clientlist_form") != null) {
		removeElement(document.getElementById("view_clientlist_form"));
	}
	var formObj = document.createElement("form");
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
	setTimeout(function(){parent.document.networkmapdRefresh.submit();}, 5000);//avoiding no data when open the view list
	clearInterval(interval_clientlist_listview_update);
	interval_clientlist_listview_update = setInterval(function(){
		if(document.getElementById("clientlist_viewlist_content").style.display != "none")
			parent.document.networkmapdRefresh.submit();
		else
			clearInterval(interval_clientlist_listview_update);
	}, 1000*60*3);
	setTimeout("sorterClientList();updateClientListView();", 500);

	registerIframeClick("statusframe", hide_clientlist_view_block);
}

function exportClientListLog() {
	var data = [["Internet access state", "Device Type", "Client Name", "Client IP address", "IP Method", "Clients MAC Address", "Interface", "Tx Rate", "Rx Rate", "Access time"]];
	var tempArray = new Array();
	var ipStateExport = new Array();
	ipStateExport["Static"] =  "Static IP";
	ipStateExport["DHCP"] =  "Automatic IP";
	ipStateExport["Manual"] =  "MAC-IP Binding";
	var setArray = function(array) {
		for(var i = 0; i < array.length; i += 1) {
			tempArray = [];
			tempArray[0] = (array[i][0] == 1) ? "Allow Internet access" : "Block Internet access";
			tempArray[1] = array[i][1].replace(",", "");
			tempArray[2] = array[i][2];
			tempArray[3] = array[i][3];
			tempArray[4] = ipStateExport[clientList[array[i][4]].ipMethod];
			tempArray[5] = array[i][4];
			if(!(isSwMode('mb') || isSwMode('ew'))) {
				var if_name = "";
				if(array[i][9] == 0)
					if_name = "Wired";
				else{
					if(isSupport("amas") && array[i][13] != "")
						if_name =  wl_nband_title[array[i][9] - 1] + " Guest Network - " +  array[i][13];
					else
						if_name = wl_nband_title[array[i][9] - 1];
				}
				tempArray[6] = if_name;
				tempArray[7] = (array[i][6] == "") ? "-" : array[i][6];
				tempArray[8] = (array[i][7] == "") ? "-" : array[i][7];
				tempArray[9] = (array[i][9] == 0) ? "-" : array[i][8];
			}
			else {
				tempArray[6] = "Wired";
				tempArray[7] = "-";
				tempArray[8] = "-";
				tempArray[9] = "-";
			}
			data.push(tempArray);
		}
	};
	switch (clienlistViewMode) {
		case "All" :
			setArray(all_list);
			break;
		case "ByInterface" :
			setArray(wired_list);
			setArray(wl1_list);
			setArray(wl2_list);
			setArray(wl3_list);
			if(isSupport("amas")){
				for(var i=1; i<multissid_count+1; i++)
					setArray(gn_list["gn"+i+""]);
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
			sorter.doSorter(sorter.wired_index, indexMapType[sorter.wired_index], 'wired_list');
			if(wl_info.band2g_support)
				sorter.doSorter(sorter.wl1_index, indexMapType[sorter.wl1_index], 'wl1_list');
			if(wl_info.band5g_support)
				sorter.doSorter(sorter.wl2_index, indexMapType[sorter.wl2_index], 'wl2_list');
			if(wl_info.band5g_2_support || wl_info.band6g_support)
				sorter.doSorter(sorter.wl3_index, indexMapType[sorter.wl3_index], 'wl3_list');
			if(isSupport("amas")){
				for(var i=1; i<multissid_count+1; i++)
					sorter.doSorter(sorter["gn"+i+"_index"], indexMapType[sorter["gn"+i+"_index"]], 'gn'+i+'_list');
			}
			break;
	}
}

function create_clientlist_listview() {
	all_list = [];
	wired_list = [];
	wl1_list = [];
	wl2_list = [];
	wl3_list = [];
	if(isSupport("amas")){
		for(var i=1; i<multissid_count+1; i++)
			gn_list["gn"+i+""] = [];
	}

	if(document.getElementById("clientlist_viewlist_block") != null) {
		removeElement(document.getElementById("clientlist_viewlist_block"));
	}

	var divObj = document.createElement("div");
	divObj.setAttribute("id","clientlist_viewlist_block");

	var obj_width_map = [["15%", "20%", "25%", "20%", "20%"],["10%", "10%", "30%", "20%", "20%", "10%"],["6%", "6%", "27%", "20%", "15%", "6%", "6%", "6%", "8%"]];
	if(top.isIE8) obj_width_map = [["", "", "40%", "40%", "20%"],["", "", "40%", "30%", "20%", "10%"],["", "", "33%", "26%", "15%", "6%", "6%", "6%", "8%"]];
	var obj_width = stainfo_support ? obj_width_map[2] : obj_width_map[1];
	var wl_colspan = stainfo_support ? 9 : 6;

	var code = "";

	var drawSwitchMode = function(mode) {
		var drawSwitchModeHtml = "";

		drawSwitchModeHtml += "<div style='margin-top:15px;margin-left:15px;float:left;'>";

		if(mode == "All") {
			drawSwitchModeHtml += "<div class='block_filter_pressed clientlist_All'>";
			drawSwitchModeHtml += "<div class='block_filter_name' style='color:#93A9B1;'><#All#></div>";
			drawSwitchModeHtml += "</div>";
			drawSwitchModeHtml += "<div class='block_filter clientlist_ByInterface' style='cursor:pointer'>";
			drawSwitchModeHtml += "<div class='block_filter_name' onclick='changeClientListViewMode();'><#wan_interface#></div>";
			drawSwitchModeHtml += "</div>";
		}
		else {							
			drawSwitchModeHtml += "<div class='block_filter clientlist_All' style='cursor:pointer'>";
			drawSwitchModeHtml += "<div class='block_filter_name' onclick='changeClientListViewMode();'><#All#></div>";
			drawSwitchModeHtml += "</div>";
			drawSwitchModeHtml += "<div class='block_filter_pressed clientlist_ByInterface'>";
			drawSwitchModeHtml += "<div class='block_filter_name' style='color:#93A9B1;'><#wan_interface#></div>";
			drawSwitchModeHtml += "</div>";
		}
		drawSwitchModeHtml += "</div>";
		return drawSwitchModeHtml;
	};

	if(!(isSwMode('mb') || isSwMode('ew')))
		code += drawSwitchMode(clienlistViewMode);
	code += "<div style='float:right;'><img src='/images/button-close.gif' style='width:30px;cursor:pointer' onclick='closeClientListView();'></div>";
	code += "<table border='0' align='center' cellpadding='0' cellspacing='0' style='width:100%;padding:0 15px 15px 15px;'><tbody><tr><td>";

	switch (clienlistViewMode) {
		case "All" :
			code += "<table width='100%' border='1' align='center' cellpadding='0' cellspacing='0' class='FormTable_table' style='margin-top:15px;'>";
			code += "<thead><tr height='28px'><td id='td_all_list_title' colspan='" + wl_colspan + "'><#Clientlist_All_List#>";
			code += "<a id='all_expander'class='clientlist_expander' onclick='showHideContent(\"clientlist_all_list_Block\", this);'>[ <#Clientlist_Hide#> ]</a>";
			code += "</td></tr></thead>";
			code += "<tr id='tr_all_title' height='40px'>";
			code += "<th class='IE8HACK' width=" + obj_width[0] + "><#Internet#></th>";
			code += "<th class='IE8HACK' width=" + obj_width[1] + "><#Client_Icon#></th>";
			code += "<th width=" + obj_width[2] + " onclick='sorter.addBorder(this);sorter.doSorter(2, \"str\", \"all_list\");' style='cursor:pointer;'><#ParentalCtrl_username#></th>";
			code += "<th width=" + obj_width[3] + " onclick='sorter.addBorder(this);sorter.doSorter(3, \"num\", \"all_list\");' style='cursor:pointer;'><#vpn_client_ip#></th>";
			code += "<th width=" + obj_width[4] + " onclick='sorter.addBorder(this);sorter.doSorter(4, \"str\", \"all_list\");' style='cursor:pointer;'><#ParentalCtrl_hwaddr#></th>";
			if(!(isSwMode('mb') || isSwMode('ew')))
				code += "<th width=" + obj_width[5] + " onclick='sorter.addBorder(this);sorter.doSorter(5, \"num\", \"all_list\");' style='cursor:pointer;'><#wan_interface#></th>";
			if(stainfo_support && !(isSwMode('mb') || isSwMode('ew'))) {
				code += "<th width=" + obj_width[6] + " onclick='sorter.addBorder(this);sorter.doSorter(6, \"num\", \"all_list\");' style='cursor:pointer;' title='The transmission rates of your wireless device'>Tx Rate (Mbps)</th>";/*untranslated*/
				code += "<th width=" + obj_width[7] + " onclick='sorter.addBorder(this);sorter.doSorter(7, \"num\", \"all_list\");' style='cursor:pointer;' title='The receive rates of your wireless device'>Rx Rate (Mbps)</th>";/*untranslated*/
				code += "<th width=" + obj_width[8] + " onclick='sorter.addBorder(this);sorter.doSorter(8, \"str\", \"all_list\");' style='cursor:pointer;'><#Access_Time#></th>";
			}
			code += "</tr>";
			code += "</table>";
			code += "<div id='clientlist_all_list_Block'></div>";
			break;
		case "ByInterface" :
			code += "<table width='100%' border='1' align='center' cellpadding='0' cellspacing='0' class='FormTable_table' style='margin-top:15px;'>";
			code += "<thead><tr height='28px'><td colspan='" + wl_colspan + "'><#tm_wired#>";
			code += "<a id='wired_expander' class='clientlist_expander' onclick='showHideContent(\"clientlist_wired_list_Block\", this);'>[ <#Clientlist_Hide#> ]</a>";
			code += "</td></tr></thead>";
			code += "<tr id='tr_wired_title' height='40px'>";
			code += "<th class='IE8HACK' width=" + obj_width[0] + "><#Internet#></th>";
			code += "<th class='IE8HACK' width=" + obj_width[1] + "><#Client_Icon#></th>";
			code += "<th width=" + obj_width[2] + " onclick='sorter.addBorder(this);sorter.doSorter(2, \"str\", \"wired_list\");' style='cursor:pointer;'><#ParentalCtrl_username#></th>";
			code += "<th width=" + obj_width[3] + " onclick='sorter.addBorder(this);sorter.doSorter(3, \"num\", \"wired_list\");' style='cursor:pointer;'><#vpn_client_ip#></th>";
			code += "<th width=" + obj_width[4] + " onclick='sorter.addBorder(this);sorter.doSorter(4, \"str\", \"wired_list\");' style='cursor:pointer;'><#ParentalCtrl_hwaddr#></th>";
			if(!(isSwMode('mb') || isSwMode('ew')))
				code += "<th width=" + obj_width[5] + " ><#wan_interface#></th>";
			if(stainfo_support && !(isSwMode('mb') || isSwMode('ew'))) {
				code += "<th width=" + obj_width[6] + " title='The transmission rates of your wireless device'>Tx Rate (Mbps)</th>";/*untranslated*/
				code += "<th width=" + obj_width[7] + " title='The receive rates of your wireless device'>Rx Rate (Mbps)</th>";/*untranslated*/
				code += "<th width=" + obj_width[8] + "><#Access_Time#></th>";
			}
			code += "</tr>";
			code += "</table>";
			code += "<div id='clientlist_wired_list_Block'></div>";
	
			var wl_map = {"2.4 GHz": "1",  "5 GHz": "2", "5 GHz-1": "2", "5 GHz-2": "3", "6 GHz": "3"};
			obj_width = stainfo_support ? obj_width_map[2] : obj_width_map[1];
			for(var i = 0; i < wl_nband_title.length; i += 1) {
				code += "<table width='100%' border='1' align='center' cellpadding='0' cellspacing='0' class='FormTable_table' style='margin-top:15px;'>";
				code += "<thead><tr height='23px'><td colspan='" + wl_colspan + "'>" + wl_nband_title[i];
				code += "<a id='wl" + wl_map[wl_nband_title[i]] + "_expander' class='clientlist_expander' onclick='showHideContent(\"clientlist_wl" + wl_map[wl_nband_title[i]] + "_list_Block\", this);'>[ <#Clientlist_Hide#> ]</a>";
				code += "</td></tr></thead>";
				code += "<tr id='tr_wl" + wl_map[wl_nband_title[i]] + "_title' height='40px'>";
				code += "<th class='IE8HACK' width=" + obj_width[0] + "><#Internet#></th>";
				code += "<th class='IE8HACK' width=" + obj_width[1] + "><#Client_Icon#></th>";
				code += "<th width=" + obj_width[2] + " onclick='sorter.addBorder(this);sorter.doSorter(2, \"str\", \"wl"+wl_map[wl_nband_title[i]]+"_list\");' style='cursor:pointer;'><#ParentalCtrl_username#></th>";
				code += "<th width=" + obj_width[3] + " onclick='sorter.addBorder(this);sorter.doSorter(3, \"num\", \"wl"+wl_map[wl_nband_title[i]]+"_list\");' style='cursor:pointer;'><#vpn_client_ip#></th>";
				code += "<th width=" + obj_width[4] + " onclick='sorter.addBorder(this);sorter.doSorter(4, \"str\", \"wl"+wl_map[wl_nband_title[i]]+"_list\");' style='cursor:pointer;'><#ParentalCtrl_hwaddr#></th>";
				if(!(isSwMode('mb') || isSwMode('ew')))
					code += "<th width=" + obj_width[5] + " onclick='sorter.addBorder(this);sorter.doSorter(5, \"num\", \"wl"+wl_map[wl_nband_title[i]]+"_list\");' style='cursor:pointer;'><#wan_interface#></th>";
				if(stainfo_support && !(isSwMode('mb') || isSwMode('ew'))) {
					code += "<th width=" + obj_width[6] + " onclick='sorter.addBorder(this);sorter.doSorter(6, \"num\", \"wl"+wl_map[wl_nband_title[i]]+"_list\");' style='cursor:pointer;' title='The transmission rates of your wireless device'>Tx Rate (Mbps)</th>";/*untranslated*/
					code += "<th width=" + obj_width[7] + " onclick='sorter.addBorder(this);sorter.doSorter(7, \"num\", \"wl"+wl_map[wl_nband_title[i]]+"_list\");' style='cursor:pointer;' title='The receive rates of your wireless device'>Rx Rate (Mbps)</th>";/*untranslated*/
					code += "<th width=" + obj_width[8] + " onclick='sorter.addBorder(this);sorter.doSorter(8, \"str\", \"wl"+wl_map[wl_nband_title[i]]+"_list\");' style='cursor:pointer;'><#Access_Time#></th>";
				}
				code += "</tr>";
				code += "</table>";
				code += "<div id='clientlist_wl" + wl_map[wl_nband_title[i]] + "_list_Block'></div>";
			}
			if(isSupport("amas")){
				for(var i=1; i<multissid_count+1; i++){
					code += "<table width='100%' border='1' align='center' cellpadding='0' cellspacing='0' class='FormTable_table' style='margin-top:15px;'>";
					code += "<thead><tr height='23px'><td colspan='" + wl_colspan + "'><#Guest_Network#> - " + i;
					code += "<a id='gn" + i + "_expander' class='clientlist_expander' onclick='showHideContent(\"clientlist_gn" + i + "_list_Block\", this);'>[ <#Clientlist_Hide#> ]</a>";
					code += "</td></tr></thead>";
					code += "<tr id='tr_gn" + i + "_title' height='40px'>";
					code += "<th class='IE8HACK' width=" + obj_width[0] + "><#Internet#></th>";
					code += "<th class='IE8HACK' width=" + obj_width[1] + "><#Client_Icon#></th>";
					code += "<th width=" + obj_width[2] + " onclick='sorter.addBorder(this);sorter.doSorter(2, \"str\", \"gn"+i+"_list\");' style='cursor:pointer;'><#ParentalCtrl_username#></th>";
					code += "<th width=" + obj_width[3] + " onclick='sorter.addBorder(this);sorter.doSorter(3, \"num\", \"gn"+i+"_list\");' style='cursor:pointer;'><#vpn_client_ip#></th>";
					code += "<th width=" + obj_width[4] + " onclick='sorter.addBorder(this);sorter.doSorter(4, \"str\", \"gn"+i+"_list\");' style='cursor:pointer;'><#ParentalCtrl_hwaddr#></th>";
					if(!(isSwMode('mb') || isSwMode('ew')))
						code += "<th width=" + obj_width[5] + " onclick='sorter.addBorder(this);sorter.doSorter(5, \"num\", \"gn"+i+"_list\");' style='cursor:pointer;'><#wan_interface#></th>";
					if(stainfo_support && !(isSwMode('mb') || isSwMode('ew'))) {
						code += "<th width=" + obj_width[6] + " onclick='sorter.addBorder(this);sorter.doSorter(6, \"num\", \"gn"+i+"_list\");' style='cursor:pointer;' title='The transmission rates of your wireless device'>Tx Rate (Mbps)</th>";/*untranslated*/
						code += "<th width=" + obj_width[7] + " onclick='sorter.addBorder(this);sorter.doSorter(7, \"num\", \"gn"+i+"_list\");' style='cursor:pointer;' title='The receive rates of your wireless device'>Rx Rate (Mbps)</th>";/*untranslated*/
						code += "<th width=" + obj_width[8] + " onclick='sorter.addBorder(this);sorter.doSorter(8, \"str\", \"gn"+i+"_list\");' style='cursor:pointer;'><#Access_Time#></th>";
					}
					code += "</tr>";
					code += "</table>";
					code += "<div id='clientlist_gn" + i + "_list_Block'></div>";
				}
			}
			break;
	}

	if(!top.isIE8)
		code += "<div style='text-align:center;margin-top:15px;'><input  type='button' class='button_gen' onclick='exportClientListLog();' value='<#btn_Export#>'></div>";
	
	code += "</td></tr></tbody>";
	code += "</table>";

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
	for(var i = 0; i < clientList.length; i += 1) {
		if(clientList[clientList[i]].isOnline) {
			var deviceTypeName = "Loading manufacturer..";
			if((clientList[clientList[i]].vendor != "" && clientList[clientList[i]].vendor != undefined)) { //Oui Vendor name
				deviceTypeName = clientList[clientList[i]].vendor;
			}		
			if((clientList[clientList[i]].dpiDevice != "" && clientList[clientList[i]].dpiDevice != undefined)) { //BWDPI device
				deviceTypeName = clientList[clientList[i]].dpiDevice;
			}
			var clientName = (clientList[clientList[i]].nickName == "") ? clientList[clientList[i]].name : clientList[clientList[i]].nickName;
			var tempArray = [clientList[clientList[i]].internetState, deviceTypeName, clientName, clientList[clientList[i]].ip, 
							clientList[clientList[i]].mac, clientList[clientList[i]].rssi, clientList[clientList[i]].curTx, clientList[clientList[i]].curRx, 
							clientList[clientList[i]].wlConnectTime, clientList[clientList[i]].isWL, clientList[clientList[i]].vendor, clientList[clientList[i]].type, 
							clientList[clientList[i]].macRepeat, clientList[clientList[i]].isGN];
			switch (clienlistViewMode) {
				case "All" :
					all_list.push(tempArray);
					break;
				case "ByInterface" :
					if(isSupport("amas") && clientList[clientList[i]].isWL != 0 && clientList[clientList[i]].isGN != "")
						gn_list["gn"+clientList[clientList[i]].isGN+""].push(tempArray);
					else{
						switch (clientList[clientList[i]].isWL) {
							case 0:
								wired_list.push(tempArray);
								break;
							case 1:
								wl1_list.push(tempArray);
								break;
							case 2:
								wl2_list.push(tempArray);
								break;
							case 3:
								wl3_list.push(tempArray);
								break;
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
		if(!sorter.wired_display) {
			document.getElementById("clientlist_wired_list_Block").style.display = "none";
			document.getElementById("wired_expander").innerHTML = "[ <#Clientlist_Show#> ]";
		}
		if(!sorter.wl1_display) {
			document.getElementById("clientlist_wl1_list_Block").style.display = "none";
			document.getElementById("wl1_expander").innerHTML = "[ <#Clientlist_Show#> ]";
		}
		if(!sorter.wl2_display) {
			document.getElementById("clientlist_wl2_list_Block").style.display = "none";
			document.getElementById("wl2_expander").innerHTML = "[ <#Clientlist_Show#> ]";
		}
		if(!sorter.wl3_display) {
			document.getElementById("clientlist_wl3_list_Block").style.display = "none";
			document.getElementById("wl3_expander").innerHTML = "[ <#Clientlist_Show#> ]";
		}
		if(isSupport("amas")){
			for(var i=1; i<multissid_count+1; i++){
				if(!sorter["gn"+i+"_display"]){
					document.getElementById("clientlist_gn"+i+"_list_Block").style.display = "none";
					document.getElementById("gn"+i+"_expander").innerHTML = "[ <#Clientlist_Show#> ]";
				}
			}
		}
	}
}

var clientListViewMacUploadIcon = new Array();
function drawClientListBlock(objID) {
	var sortArray = "";
	switch(objID) {
		case "all_list" :
			sortArray = all_list;
			break;
		case "wired_list" :
			sortArray = wired_list;
			break;
		case "wl1_list" :
			sortArray = wl1_list;
			break;
		case "wl2_list" :
			sortArray = wl2_list;
			break;	
		case "wl3_list" :
			sortArray = wl3_list;
			break;
	}
	if(sortArray == "" && isSupport("amas")){
		if(objID.substr(0,2) == "gn")
			sortArray = gn_list[objID.substr(0,3)];
	}
	var listViewProfile = function(_profile) {
		if(_profile == null)
			_profile = ["", "", "", "", "", "", "", "", "", "", "", "", "", ""];
		
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
		this.vender = _profile[10];
		this.type = _profile[11];
		this.macRepeat = _profile[12];
		this.isGN = _profile[13];
	}

	if(document.getElementById("clientlist_" + objID + "_Block") != null) {
		if(document.getElementById("tb_" + objID) != null) {
			removeElement(document.getElementById("tb_" + objID));
		}
		var obj_width_map = [["15%", "20%", "25%", "20%", "20%"],["10%", "10%", "30%", "20%", "20%", "10%"],["6%", "6%", "27%", "20%", "15%", "6%", "6%", "6%", "8%"]];
		if(top.isIE8) obj_width_map = [["", "", "40%", "40%", "20%"],["", "", "40%", "30%", "20%", "10%"],["", "", "33%", "26%", "15%", "6%", "6%", "6%", "8%"]];
		//var obj_width = (objID == "wired_list") ? obj_width_map[0] : ((stainfo_support) ? obj_width_map[2] : obj_width_map[1]);
		var obj_width = (stainfo_support) ? obj_width_map[2] : obj_width_map[1];
		var wl_colspan = stainfo_support ? 9 : 6;
		var clientListCode = "";
		//user icon
		var listView_userIconBase64 = "NoIcon";

		clientListCode += "<table width='100%' cellspacing='0' cellpadding='0' align='center' class='list_table' id='tb_" + objID + "'>";
		if(sortArray.length == 0) {
			clientListCode += "<tr id='tr_" + objID + "'><td style='color:#FFCC00;' colspan='" + wl_colspan + "'><#IPConnection_VSList_Norule#></td></tr>";
		}
		else {
			clientlist_sort = new Array();
			for(var i = 0; i < sortArray.length; i += 1) {
				clientlist_sort.push(new listViewProfile(sortArray[i]));
			}

			for(var j = 0; j < clientlist_sort.length; j += 1) {
				clientListCode += "<tr height='48px'>";

				if(usericon_support) {
					if(clientListViewMacUploadIcon[clientlist_sort[j].mac] == undefined) {
						var clientMac = clientlist_sort[j].mac.replace(/\:/g, "");
						listView_userIconBase64 = getUploadIcon(clientMac);
						clientListViewMacUploadIcon[clientlist_sort[j].mac] = listView_userIconBase64;
					}
					else {
						listView_userIconBase64 = clientListViewMacUploadIcon[clientlist_sort[j].mac];
					}
				}
			
				var internetStateCss = "";
				var internetStateTip = "";
				if(clientlist_sort[j].internetState) {
					internetStateCss = "internetAllow" ;
					internetStateTip = "Allow Internet access";
				}
				else {
					internetStateCss = "internetBlock";
					internetStateTip = "Block Internet access";
				}

				clientListCode += "<td class='IE8HACK' width='" + obj_width[0] + "' align='center'>";
				clientListCode += "<div class=" + internetStateCss + " title=\"" + internetStateTip + "\"></div>";
				clientListCode += "</td>";

				clientListCode += "<td class='IE8HACK' width='" + obj_width[1] + "' align='center'>";
				// display how many clients that hide behind a repeater.
				if(clientlist_sort[j].macRepeat > 1){
					clientListCode += '<div class="clientlist_circle"';
					clientListCode += 'onmouseover="return overlib(\''+clientlist_sort[j].macRepeat+' clients are connecting to <% nvram_get("productid"); %> through this device.\');"';
					clientListCode += 'onmouseout="nd();"><div>';
					clientListCode += clientlist_sort[j].macRepeat;
					clientListCode += '</div></div>';
				}

				if(listView_userIconBase64 != "NoIcon") {
					clientListCode += "<div style='height:42px;width:42px;' title='"+ clientlist_sort[j].deviceTypeName +"'>";
					clientListCode += "<img class='imgUserIcon_viewlist' src=" + listView_userIconBase64 + "";
					clientListCode += ">";
					clientListCode += "</div>";
				}
				else if( clientlist_sort[j].type != "0" || clientlist_sort[j].vender == "") {
					var icon_type = "type" + clientlist_sort[j].type;
					if(clientlist_sort[j].type == "0") {
						icon_type = "type0_viewMode";
					}
					clientListCode += "<div style='height:40px;width:40px;cursor:default;' class='clientIcon_no_hover " + icon_type + "' title='"+ clientlist_sort[j].deviceTypeName +"'>";
					if(clientlist_sort[j].type == "36")
						clientListCode += "<div class='flash'></div>";
					clientListCode += "</div>";
				}
				else if(clientlist_sort[j].vender != "" ) {
					var venderIconClassName = getVenderIconClassName(clientlist_sort[j].vender.toLowerCase());
					if(venderIconClassName != "" && !downsize_4m_support) {
						clientListCode += "<div style='height:42px;width:42px;background-size:77px;cursor:default;' class='venderIcon_no_hover " + venderIconClassName + "' title='"+ clientlist_sort[j].deviceTypeName +"'></div>";
					}
					else {
						var icon_type = "type" + clientlist_sort[j].type;
						if(clientlist_sort[j].type == "0") {
							icon_type = "type0_viewMode";
						}
						clientListCode += "<div style='height:40px;width:40px;cursor:default;' class='clientIcon_no_hover " + icon_type + "' title='"+ clientlist_sort[j].deviceTypeName +"'></div>";
					}				
				}
				clientListCode += "</td>";

				clientListCode += "<td style='word-wrap:break-word; word-break:break-all;' width='" + obj_width[2] + "'>";
				var clientNameEnCode = htmlEnDeCode.htmlEncode(decodeURIComponent(clientlist_sort[j].name));
				clientListCode += "<div id='div_clientName_"+objID+"_"+j+"' class='viewclientlist_clientName_edit' onclick='editClientName(\""+objID+"_"+j+"\");'>"+clientNameEnCode+"</div>";
				clientListCode += "<input id='client_name_"+objID+"_"+j+"' type='text' value='"+clientNameEnCode+"' class='input_25_table' maxlength='32' style='width:95%;margin-left:0px;display:none;' onblur='saveClientName(\""+objID+"_"+j+"\", "+clientlist_sort[j].type+", \"" + clientlist_sort[j].mac + "\");'>";
				clientListCode += "</td>";
				var ipStyle = ('<% nvram_get("sw_mode"); %>' == "1") ? "line-height:16px;text-align:left;padding-left:10px;" : "line-height:16px;text-align:center;";
				clientListCode += "<td width='" + obj_width[3] + "' style='" + ipStyle + "'>";
				clientListCode += (clientList[clientlist_sort[j].mac].isWebServer) ? "<a class='link' href='http://"+clientlist_sort[j].ip+"' target='_blank'>"+clientlist_sort[j].ip+"</a>" : clientlist_sort[j].ip;
				if('<% nvram_get("sw_mode"); %>' == "1") {
					clientListCode += '<span style="float:right;margin-top:-3px;margin-right:5px;" class="ipMethodTag" onmouseover="return overlib(\''
					clientListCode += ipState[clientList[clientlist_sort[j].mac].ipMethod];
					clientListCode += '\')" onmouseout="nd();">';
					clientListCode += clientList[clientlist_sort[j].mac].ipMethod + '</span>';
				}

				clientListCode += "</td>";
				clientListCode += "<td width='" + obj_width[4] + "'>"+clientlist_sort[j].mac+"</td>";
				if(!(isSwMode('mb') || isSwMode('ew'))) {
					var rssi_t = 0;
					if(clientlist_sort[j].isWL == "0")
						rssi_t = "wired";
					else
						rssi_t = client_convRSSI(clientlist_sort[j].rssi);
					var radioIcon_css = "radioIcon";
					if(clientlist_sort[j].isGN != "" && clientlist_sort[j].isGN != undefined)
						radioIcon_css += " GN";
					clientListCode += "<td width='" + obj_width[5] + "' align='center'><div style='height:28px;width:28px'><div class='" +  radioIcon_css + " radio_" + rssi_t + "'></div>";
					if(clientlist_sort[j].isWL != 0) {
						var bandClass = (navigator.userAgent.toUpperCase().match(/CHROME\/([\d.]+)/)) ? "band_txt_chrome" : "band_txt";
						clientListCode += "<div class='band_block'><span class='" + bandClass + "'>" + wl_nband_title[clientlist_sort[j].isWL-1].replace("Hz", "").replace(/\s*/g,"") + "</span></div>";
					}
					clientListCode += "</div></td>";
				}
				if(stainfo_support && !(isSwMode('mb') || isSwMode('ew'))) {
					var txRate = "";
					var rxRate = "";
					if(clientlist_sort[j].isWL != 0) {
						txRate = (clientlist_sort[j].curTx == "") ? "-" : clientlist_sort[j].curTx;
						rxRate = (clientlist_sort[j].curRx == "") ? "-" : clientlist_sort[j].curRx;
					}
					else {
						txRate = "-";
						rxRate = "-";
					}
					clientListCode += "<td width='" + obj_width[6] + "'>" + txRate + "</td>";
					clientListCode += "<td width='" + obj_width[7] + "'>" + rxRate + "</td>";
					clientListCode += "<td width='" + obj_width[8] + "'>"+((clientlist_sort[j].wlConnectTime == "00:00:00") ? "-" : clientlist_sort[j].wlConnectTime)+"</td>";
				}
				clientListCode += "</tr>";
			}
		}
		clientListCode += "</table>";
		document.getElementById("clientlist_" + objID + "_Block").innerHTML = clientListCode;
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
				switch (clickItem) {
					case "wired" :
						sorter.wired_display = true;
						break;
					case "wl1" :
						sorter.wl1_display = true;
						break;
					case "wl2" :
						sorter.wl2_display = true;
						break;
					case "wl3" :
						sorter.wl3_display = true;
						break;
				}
			}
			slideFlag = true;
			slideDown(objnmae, 200);
			thisObj.innerHTML = "[ <#Clientlist_Hide#> ]";
		}
		else {
			if(clienlistViewMode == "All")
				sorter.all_display = false;
			else {
				switch (clickItem) {
					case "wired" :
						sorter.wired_display = false;
						break;
					case "wl1" :
						sorter.wl1_display = false;
						break;
					case "wl2" :
						sorter.wl2_display = false;
						break;
					case "wl3" :
						sorter.wl3_display = false;
						break;
				}
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

function editClientName(index) {
	document.getElementById("div_clientName_"+index).style.display = "none";
	document.getElementById("client_name_"+index).style.display = "";
	document.getElementById("client_name_"+index).focus();
	edit_client_name_flag = true;
}
var view_custom_name = decodeURIComponent('<% nvram_char_to_ascii("", "custom_clientlist"); %>').replace(/&#62/g, ">").replace(/&#60/g, "<");
function saveClientName(index, type, mac) {
	document.getElementById("client_name_"+index).value = document.getElementById("client_name_"+index).value.trim();
	var client_name_obj = document.getElementById("client_name_"+index);
	if(client_name_obj.value.length == 0){
		alert("<#File_Pop_content_alert_desc1#>");
		window.setTimeout(function () { 
			client_name_obj.focus();
			client_name_obj.select();
			client_name_obj.value = "";
		}, 10);
		return false;
	}
	else if(client_name_obj.value.indexOf(">") != -1 || client_name_obj.value.indexOf("<") != -1){
		alert("<#JS_validstr2#> '<', '>'");
		window.setTimeout(function () { 
			client_name_obj.focus();
			client_name_obj.select();
			client_name_obj.value = "";
		}, 10);
		return false;
	}

	if(utf8_ssid_support){
		var len = validator.lengthInUtf8(client_name_obj.value);
		if(len > 32){
			alert("Username cannot be greater than 32 characters.");/* untranslated */
			window.setTimeout(function () {
				client_name_obj.focus();
				client_name_obj.select();
				client_name_obj.value = "";
			}, 10);
			return false;
		}
	}
	else if(!validator.haveFullWidthChar(client_name_obj)) {
		alert('<#JS_validchar#>');
		window.setTimeout(function () { 
			client_name_obj.focus();
			client_name_obj.select();
			client_name_obj.value = "";
		}, 10);
		return false;
	}

	document.getElementById("div_clientName_"+index).style.display = "";
	client_name_obj.style.display = "none";
	edit_client_name_flag = false;
	
	var originalCustomListArray = new Array();
	var onEditClient = new Array();
	originalCustomListArray = view_custom_name.split('<');
	
	onEditClient[0] = client_name_obj.value;
	onEditClient[1] = mac.toUpperCase();
	onEditClient[2] = 0;
	onEditClient[3] = type;
	onEditClient[4] = "";
	onEditClient[5] = "";
	document.getElementById("div_clientName_"+index).innerHTML = htmlEnDeCode.htmlEncode(document.getElementById("client_name_"+index).value.trim());

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
				document.getElementById(_pullArrowID).src = "/images/arrow-down.gif";
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
				attribute_value = (clientObj.nickName == "") ? clientObj.name.replace(/'/g, "\\'") : clientObj.nickName.replace(/'/g, "\\'");
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

	if(document.getElementById(_containerID).childNodes.length == "0")
		document.getElementById(_pullArrowID).style.display = "none";
	else
		document.getElementById(_pullArrowID).style.display = "";
}

function redirectTimeScheduling(_mac) {
	cookie.set("time_scheduling_mac", _mac, 1);
	location.href = "ParentalControl.asp" ;
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
		overlibStr += "<p><#Wireless_Radio#>:</p>" + wl_nband_title[client.isWL-1] + " (" + client.rssi + " dBm)";
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
	if(clientList[mac].vendor != "") {
		setTimeout(function(){
			var overlibStrTmp = retOverLibStr(clientList[mac]);
			overlibStrTmp += "<p><span>.....................................</span></p><p style='margin-top:5px'><#Manufacturer#> :</p>";
			overlibStrTmp += clientList[mac].vendor;
			return overlib(overlibStrTmp);
		}, 1);
	}
	else {
		if('<% nvram_get("x_Setting"); %>' == '1' && wanConnectStatus && clientList[mac].internetState) {
			var queryStr = mac.replace(/\:/g, "").splice(6,6,"");
			var overlibStrTmp = retOverLibStr(clientList[mac]);
			$.getJSON("http://nw-dlcdnet.asus.com/plugin/js/ouiDB.json", function(data){
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
}

function clientFromIP(ip) {
	for(var i=0; i<clientList.length;i++){
		var clientObj = clientList[clientList[i]];
		if(clientObj.ip == ip) return clientObj;
	}
	return 0;
}

/* End exported functions */

