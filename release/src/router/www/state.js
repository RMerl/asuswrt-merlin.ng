document.write('<script type="text/javascript" src="/require/require.min.js"></script>');
document.write('<script type="text/javascript" src="/js/support_site.js"></script>');
document.write('<script type="text/javascript" src="/notification.js"></script>');
document.write('<link rel="stylesheet" type="text/css" href="/notification.css"></link>');

/* String splice function */
String.prototype.splice = function( idx, rem, s ) {
    return (this.slice(0,idx) + s + this.slice(idx + Math.abs(rem)));
};

/* String repeat function */
String.prototype.repeat = function(times) {
   return (new Array(times + 1)).join(this);
};

/* String replace all function */
function replaceAll(txt, replace, with_this) {
   return txt.replace(new RegExp(replace, 'g'),with_this);
}

/* Internet Explorer lacks this array method */
if (!('indexOf' in Array.prototype)) {
	Array.prototype.indexOf = function(find, i) {
		if(i===undefined) i=0;
		if(i<0) i+= this.length;
		if(i<0) i=0;
		for(var n=this.length; i<n; i++){
			if (i in this && this[i]===find)
				return i;
		}
		return -1;
	};
}

/* add Array.prototype.forEach() in IE8 */
if(typeof Array.prototype.forEach != 'function'){
	Array.prototype.forEach = function(callback){
		for(var i = 0; i < this.length; i++){
			callback.apply(this, [this[i], i, this]);
		}
	};
}

if (!Array.prototype.some) {
	Array.prototype.some = function(fun) {
		'use strict';
		if(this == null) {throw new TypeError('Array.prototype.some called on null or undefined');}
		if(typeof fun !== 'function') {throw new TypeError();}

		var t = Object(this);
		var len = t.length >>> 0;
		var thisArg = arguments.length >= 2 ? arguments[1] : void 0;

		for (var i = 0; i < len; i++) {
			if (i in t && fun.call(thisArg, t[i], i, t)){return true;}
		}

		return false;
	};
}

String.prototype.toArray = function(){
	var ret = eval(this.toString());
	if(Object.prototype.toString.apply(ret) === '[object Array]')
		return ret;
	return [];
}

// for compatibility jQuery trim on IE
String.prototype.trim = function() {
    return this.replace(/^\s+|\s+$/g, '');
}

String.prototype.shorter = function(len){
	var replaceWith = "...";

	if(this.length > len)
		return this.substring(0, len-replaceWith.length)+replaceWith;
	else
		return this.toString();
}

Array.prototype.getIndexByValue = function(value){
	var index = -1;
	for(var i=0; i<this.length; i++){
		if (this[i] == value){
			index = i;
			break;
		}
	}
	return index;
}

Array.prototype.getRowIndexByValue2D = function(value, col){
	for(var i=0; i<this.length; i++){
		if(typeof(col) != "undefined" && col >= 0) {
			if(this[i][col] == value)
				return i;
		}
		else {
			for(var j=0; j<this[i].length; j++) {
				if(this[i][j] == value)
					return i;
			}
		}
	}
	return -1;
}

Array.prototype.getIndexByValue = function(value){
	var index = -1;
	for(var i=0; i<this.length; i++){
		if (this[i] == value){
			index = i;
			break;
		}
	}
	return index;
}

Array.prototype.getIndexByValue2D = function(value){
	for(var i=0; i<this.length; i++){
		if(this[i].getIndexByValue(value) != -1){
			return [i, this[i].getIndexByValue(value)]; // return [1-D_index, 2-D_index];
		}
	}
	return -1;
}



Array.prototype.del = function(n){
　if(n < 0)
　　return this;
　else
　　return this.slice(0,n).concat(this.slice(n+1,this.length));
}

var cookie = {
	set: function(key, value, days) {
		document.cookie = key + '=' + value + '; expires=' +
			(new Date(new Date().getTime() + ((days ? days : 14) * 86400000))).toUTCString() + '; path=/';
	},
	get: function(key) {
		var r = ('; ' + document.cookie + ';').match(key + '=(.*?);');
		return r ? r[1] : null;
	},
	unset: function(key) {
		document.cookie = key + '=; expires=' +
			(new Date(1)).toUTCString() + '; path=/';
	}
};

var Session = Session || (function(){
 	var win = window.top || window;
	try{
		var store = (win.name ? JSON.parse(win.name) : {});
		function Save() {
			win.name = JSON.stringify(store);
		};

		if (window.addEventListener) window.addEventListener("unload", Save, false);
		else if (window.attachEvent) window.attachEvent("onunload", Save);
		else window.onunload = Save;

		return {
			set: function(name, value) {
				store[name] = value;
			},
			get: function(name) {
				return (store[name] ? store[name] : undefined);
			},
			clear: function() { store = {}; },
			dump: function() { return JSON.stringify(store); }
		};
	}
	catch(e){
		win.name = ""; /* reset cache */
		return {
			set: function(){},
			get: function(){},
			clear: function(){},
			dump: function(){}
		};
	}
})();

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

var sw_mode = '<% nvram_get("sw_mode"); %>';
var wlc_band = '<% nvram_get("wlc_band"); %>';
/*Media Bridge mode
Broadcom: sw_mode = 3 & wlc_psta = 1, sw_mode = 3 & wlc_psta = 3
MTK/QCA: sw_mode = 2 & wlc_psta = 1
*/ 
if(((sw_mode == 2 || sw_mode == 3) && '<% nvram_get("wlc_psta"); %>' == 1) || (sw_mode == 3 && '<% nvram_get("wlc_psta"); %>' == 3))
	sw_mode = 4;
//for Broadcom New Repeater mode reference by Media Bridge mode
var new_repeater = false;	
if(sw_mode == 3 && '<% nvram_get("wlc_psta"); %>' == 2){
	sw_mode = 2;
	new_repeater = true;
}
var wlc_express = '<% nvram_get("wlc_express"); %>';
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

var current_url = location.pathname.substring(location.pathname.lastIndexOf('/') + 1) || "<% rel_index_page(); %>";
var productid = '<#Web_Title2#>';
var based_modelid = '<% nvram_get("productid"); %>';
var odmpid = '<% nvram_get("odmpid"); %>';
var support_site_modelid = (odmpid == "")? based_modelid : odmpid;
var hw_ver = '<% nvram_get("hardware_version"); %>';
var bl_version = '<% nvram_get("bl_version"); %>';
var uptimeStr = "<% uptime(); %>";
var timezone = uptimeStr.substring(26,31);
var boottime = parseInt(uptimeStr.substring(32,42));
var uptimeStr_update = "<% uptime(); %>";
var boottime_update = parseInt(uptimeStr_update.substring(32,42));
var newformat_systime = uptimeStr.substring(8,11) + " " + uptimeStr.substring(5,7) + " " + uptimeStr.substring(17,25) + " " + uptimeStr.substring(12,16);  //Ex format: Jun 23 10:33:31 2008
var systime_millsec = Date.parse(newformat_systime); // millsec from system
var JS_timeObj = new Date(); // 1970.1.1
var test_page = 0;
var testEventID = "";
var httpd_dir = "/cifs1"
var svc_ready = '<% nvram_get("svc_ready"); %>';
var qos_enable_flag = ('<% nvram_get("qos_enable"); %>' == 1) ? true : false;
var bwdpi_app_rulelist = "<% nvram_get("bwdpi_app_rulelist"); %>".replace(/&#60/g, "<");
var qos_type_flag = "<% nvram_get("qos_type"); %>";
var exist_firmver="<% nvram_get("firmver"); %>";
var exist_extendno = '<% nvram_get("extendno"); %>';

//territory_code sku
function in_territory_code(_ptn){
        return (ttc.search(_ptn) == -1) ? false : true;
}
var ttc = '<% nvram_get("territory_code"); %>';
var is_KR_sku = in_territory_code("KR");
var is_CN = in_territory_code("CN");
var is_TW_sku = in_territory_code("TW");
var is_US_sku = in_territory_code("US");
var is_UA_sku = in_territory_code("UA");

//wireless
var wl_nband_title = [];
var wl_nband_array = "<% wl_nband_info(); %>".toArray();
var band2g_count = 0;
var band5g_count = 0;
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
	else if(wl_nband_array[j] == '6'){
		band60g_count++;
		wl_nband_title.push("60 GHz" + ((band60g_count > 1) ? ("-" + band60g_count) : ""));
	}
}
if(wl_nband_title.indexOf("2.4 GHz-2") > 0) wl_nband_title[wl_nband_title.indexOf("2.4 GHz")] = "2.4 GHz-1";
if(wl_nband_title.indexOf("5 GHz-2") > 0) wl_nband_title[wl_nband_title.indexOf("5 GHz")] = "5 GHz-1";
if(wl_nband_title.indexOf("60 GHz-2") > 0) wl_nband_title[wl_nband_title.indexOf("60 GHz")] = "60 GHz-1";

var wl_info = {
	band2g_support:(function(){
				if(band2g_count > 0)
					return true;
				else
					return false;
			})(),
	band5g_support:(function(){
				if(band5g_count > 0)
					return true;
				else
					return false;
			})(),
	band5g_2_support:(function(){
				if(band5g_count == 2)
					return true;
				else
					return false;
			})(),
	band60g_support:(function(){
				if(band60g_count > 0)
					return true;
				else
					return false;
			})(),
	band2g_total:band2g_count,
	band5g_total:band5g_count,
	band60g_total:band60g_count,
	wl_if_total:(function(){
					var count = 0;
					for (var idx in wl_nband_array) {
						if (wl_nband_array.hasOwnProperty(idx)) {
							if(wl_nband_array[idx] != "")
								count++;
						}
					}
					return count;
			})()
};
//wireless end

function uiSupport(_ptn){
	var ui_support = [<% get_ui_support(); %>][0];

	if(ui_support[_ptn])
		return ui_support[_ptn];
	else
		return false;
}

// parsing rc_support
var rc_support = '<% nvram_get("rc_support"); %>';
function isSupport(_ptn){
	if(_ptn == "rog"){
		var hasRogClient = false;
		decodeURIComponent('<% nvram_char_to_ascii("", "custom_clientlist"); %>').replace(/&#62/g, ">").replace(/&#60/g, "<").split('<').forEach(function(element, index){
			if(element.split('>')[4] != "" && typeof element.split('>')[4] != "undefined" && rc_support.search(_ptn) != -1) hasRogClient = true;
		});
		return hasRogClient;
	}
	else if(_ptn == "mssid"){
		var wl_vifnames = '<% nvram_get("wl0_vifnames"); %>';
		var multissid = rc_support.search("mssid");
		if(sw_mode == 2 || sw_mode == 4)
			multissid = -1;
		if(multissid != -1) {
		//MODELDEP : RT-AD7200, special case when wireless change to 60GHz(No Multiple SSID), will casue multissid_support not correct.
			if(based_modelid == "RT-AD7200") {
				wl_vifnames = '<% nvram_get("wl0_vifnames"); %>';
			}
			multissid = wl_vifnames.split(" ").length;
		}
		return multissid;
	}
	else if(_ptn == "aicloudipk"){
		//MODELDEP : DSL-N55U、DSL-N55U-B、RT-N56U
		if(based_modelid == "DSL-N55U" || based_modelid == "DSL-N55U-B" || based_modelid == "RT-N56U")
			return true;
		else
			return false;
	}
	else if(_ptn == "nwtool"){
		return true;
	}
	else if(_ptn == "11AC"){
		if(based_modelid == "RT-N600")		//UK , remove 80MHz(11ac) for MODELDEP: RT-N600
			return false;
		if(Rawifi_support || Qcawifi_support)
			return (rc_support.search(_ptn) == -1) ? false : true;
		else
			return ('<% nvram_get("wl1_phytype"); %>' == 'v' ? true : false)
	}
	else if(_ptn == "wlopmode"){
		return ('<% nvram_get("wlopmode"); %>' == 7 ? true : false)
	}
	else if(_ptn == "wl_mfp"){
		return ('<% nvram_get("wl_mfp"); %>' == "" ? false: true)
	}
	else if(_ptn == "localap"){
		return (sw_mode == 4) ? false : true;
	}
	else if(_ptn == "concurrep"){
		return (based_modelid.search("RP-") != -1) ? true : false;
	}
	else if(_ptn == "usbPortMax"){
		var usbX_index = rc_support.indexOf("usbX");
		return (usbX_index != -1) ? rc_support.charAt(usbX_index+4) : 0;
	}
	else if(_ptn == "usbX"){
		return (rc_support.search("usbX") != -1) ? true : false;
	}
	else if(_ptn == "yadns"){
		return (rc_support.search("yadns_hideqis") != -1 || rc_support.search("yadns") != -1) ? true : false;
	}
	else if(_ptn == "hnd"){
		var machine_name = '<% get_machine_name(); %>';
		return (machine_name.search("aarch64") != -1)? true : false;
	}
	else if(_ptn == "rrsut"){
		return (sw_mode == 1 && (rc_support.search(_ptn) != -1))? true : false;
	}
	else if(_ptn == "traffic_analyzer"){
		if(!bwdpi_support) return false;

		if(( based_modelid == "BRT-AC828" || based_modelid == "GT-AC5300" || based_modelid == "GT-AC9600" || based_modelid == "RT-AC5300" || based_modelid == "RT-AC3200" || based_modelid == "RT-AC3100" || 
			based_modelid == "RT-AC88U" || based_modelid == "RT-AC87U" || based_modelid == "RT-AC86U" || based_modelid == "AC2900" || based_modelid == "RT-AC85U" ||
			based_modelid == "RT-AC65U" || based_modelid == "4G-AC68U" || based_modelid == "DSL-AC68U" || based_modelid == "RT-AC68U" || based_modelid == "RT-AC68R" || 
			based_modelid == "RT-AC68W" || based_modelid == "RT-AC68RW" || based_modelid == "RT-AC1900P" || 
			based_modelid == "RT-AC67U" || based_modelid == "RT-AC56U" || based_modelid == "RT-AC56R" || based_modelid == "BLUECAVE")
		){
			return true;
		}

		return false;
	}
	else if(_ptn == "meoVoda"){
		var meoVoda_support_models = ["RT-N14U", "BRT-AC828", "RT-AD7200", "RT-AC5300", "GT-AC5300", "RT-AC3200", "RT-AC3100", "RT-AC1200G+", "RT-AC88U", "RT-AC86U", "AC2900", "RT-AC87U", 
							  "RT-AC68U", "RT-AC68A", "4G-AC68U", "RT-AC66U", "RT-AC56U", "RT-AC51U", "RT-N66U", "RT-N18U", "BLUECAVE", "RT-N800HP"];
		return (meoVoda_support_models.indexOf(based_modelid) >= 0)? true : false;
	}
	else if(_ptn == "movistarTriple"){
		var movistarTriple_support_models = ["BRT-AC828", "RT-AC5300", "GT-AC5300", "RT-AC3200", "RT-AC3100", "RT-AC1200G+", "RT-AC88U", "RT-AC87U", "RT-AC68U", 
									 "RT-AC68A", "4G-AC68U", "RT-AC66U", "RT-AC56U", "RT-AC56S", "RT-AC51U", "RT-N66U", "RT-N18U", "BLUECAVE", "RT-N800HP"];
		return (movistarTriple_support_models.indexOf(based_modelid) >= 0)? true : false;
	}
	else if(_ptn == "dpi_mals" || _ptn == "dpi_vp" || _ptn == "dpi_cc" || _ptn == "adaptive_qos" || _ptn == "webs_filter" || _ptn == "apps_filter" || _ptn == "web_history" || _ptn == "bandwidth_monitor"){
		return uiSupport(_ptn);
	}
	else
		return (rc_support.split(" ").indexOf(_ptn) == -1) ? false : true;
}

var igd2_support = isSupport("igd2");
var nfsd_support = isSupport("nfsd");
var dnsfilter_support = isSupport("dnsfilter");
var spirit_logo_support = isSupport("spirit");
var wifilogo_support = isSupport("WIFI_LOGO"); 
var new_wifi_cert_support = isSupport("wifi2017"); 
var band2g_support = isSupport("2.4G"); 
var band5g_support = isSupport("5G");
var band5g2_support = isSupport("5G-2");
var band60g_support = isSupport("wigig");
var live_update_support = isSupport("update"); 
var no_update_support = isSupport("noupdate");
var cooler_support = isSupport("fanctrl");
var power_support = isSupport("pwrctrl");
var repeater_support = isSupport("repeater");
var concurrep_support = isSupport("concurrep");
var psta_support = isSupport("psta");
var wisp_support = isSupport("wisp");
var wl6_support = isSupport("wl6");
var no_finiwl_support = isSupport("no_finiwl");
var Bcmwifi_support = isSupport("bcmwifi");
var Rawifi_support = isSupport("rawifi");
var Qcawifi_support = isSupport("qcawifi");
var Rtkwifi_support = isSupport("rtkwifi");
var lantiq_support = isSupport("lantiq");
var wifi_logo_support = isSupport("wifilogo");
var vht80_80_support = isSupport("vht80_80");
var vht160_support = isSupport("vht160");
var dfs_US_support = isSupport("dfs");
var SwitchCtrl_support = isSupport("switchctrl");
var dsl_support = isSupport("dsl");
var vdsl_support = isSupport("vdsl");
var dualWAN_support = isSupport("dualwan");
var noWAN_support = isSupport("nowan");
var mtwancfg_support = isSupport("mtwancfg");
var ruisp_support = isSupport("ruisp");
var ssh_support = isSupport("ssh");
var snmp_support = isSupport("snmp");
var multissid_support = isSupport("mssid");
var no5gmssid_support = isSupport("no5gmssid");
var wifi_hw_sw_support = isSupport("wifi_hw_sw");
var wifi_tog_btn_support = isSupport("wifi_tog_btn");
var default_psk_support = isSupport("defpsk");
var location_list_support = isSupport("loclist");
var cfg_wps_btn_support = isSupport("cfg_wps_btn");
var usb_support = isSupport("usbX");
var usbPortMax = isSupport("usbPortMax");
var printer_support = isSupport("printer"); 
var noprinter_support = isSupport("noprinter");
var appbase_support = isSupport("appbase");
var appnet_support = isSupport("appnet");
var media_support = isSupport("media");
var noiTunes_support = isSupport("noitunes");
var nomedia_support = isSupport("nomedia");
var noftp_support = isSupport("noftp");
var noaidisk_support = isSupport("noaidisk");
var cloudsync_support = isSupport("cloudsync");
var nocloudsync_support = isSupport("nocloudsync");
var aicloudipk_support = isSupport("aicloudipk");
var yadns_hideqis = isSupport("yadns_hideqis");
var yadns_support = isSupport("yadns");
var manualstb_support = isSupport("manual_stb"); 
var wps_multiband_support = isSupport("wps_multiband");
var modem_support = isSupport("modem"); 
var nomodem_support = isSupport("nomodem");
var IPv6_support = isSupport("ipv6"); 
var IPv6_Passthrough_support = isSupport("ipv6pt"); 
var ParentalCtrl2_support = isSupport("PARENTAL2");
var pptpd_support = isSupport("pptpd"); 
var openvpnd_support = isSupport("openvpnd"); 
var vpnc_support = isSupport("vpnc");
var WebDav_support = isSupport("webdav"); 
var HTTPS_support = isSupport("HTTPS"); 
var nodm_support = isSupport("nodm"); 
var wimax_support = isSupport("wimax");
var downsize_4m_support = isSupport("sfp4m");
var downsize_8m_support = isSupport("sfp8m");
var hwmodeSwitch_support = isSupport("swmode_switch");
var diskUtility_support = isSupport("diskutility");
var networkTool_support = isSupport("nwtool");
var band5g_11ac_support = isSupport("11AC");
var no_vht_support = isSupport("no_vht");	//Hide 11AC/80MHz from GUI
var optimizeXbox_support = isSupport("optimize_xbox");
var spectrum_support = isSupport("spectrum");
var mediareview_support = isSupport("wlopmode");
var userRSSI_support = isSupport("user_low_rssi");
var timemachine_support = isSupport("timemachine");
var kyivstar_support = isSupport("kyivstar");
var email_support = isSupport("email");
var feedback_support = isSupport("feedback");
var swisscom_support = isSupport("swisscom");
var tmo_support = isSupport("tmo");
var atf_support = isSupport("atf");
var pwrsave_support = isSupport("pwrsave");
var wl_mfp_support = isSupport("wl_mfp");	// For Protected Management Frames, ARM platform
var bwdpi_support = isSupport("bwdpi");
var ipsec_srv_support = isSupport("ipsec_srv");
var ipsec_cli_support = isSupport("ipsec_cli");
//var traffic_analyzer_support = isSupport("traffic_analyzer");
var traffic_analyzer_support = bwdpi_support;
var traffic_limiter_support = isSupport("traffic_limiter");
var force_upgrade_support = isSupport("fupgrade");

var adBlock_support = isSupport("adBlock");
var keyGuard_support = isSupport("keyGuard");
var rog_support = isSupport("rog");
var smart_connect_support = isSupport("smart_connect");
if(based_modelid == "BLUECAVE"){
	smart_connect_support = isSupport("bandstr");
}

var rrsut_support = isSupport("rrsut");
var gobi_support = isSupport("gobi");
var findasus_support = isSupport("findasus");
var usericon_support = isSupport("usericon");
var localAP_support = isSupport("localap");
var ntfs_sparse_support = isSupport("sparse");
var tr069_support = isSupport("tr069");
var tor_support = isSupport("tor");
var stainfo_support = isSupport("stainfo");
var dhcp_override_support = isSupport("dhcp_override");
var disnwmd_support = isSupport("disable_nwmd");
var wtfast_support = isSupport("wtfast");
var powerline_support = isSupport("plc");
var reboot_schedule_support = isSupport("reboot_schedule");
var captivePortal_support = isSupport("captivePortal");
var cp_freewifi_support = isSupport("cp_freewifi");
var cp_advanced_support = isSupport("cp_advanced");
var fbwifi_support = isSupport("fbwifi");
var noiptv_support = isSupport("noiptv");
var nz_isp_support = isSupport("nz_isp");
var app_support = isSupport("app");
var letsencrypt_support = isSupport("letsencrypt");
var pm_support = isSupport("permission_management");
var hive_hide_support = isSupport("hive_hide");
var wifiproxy_support = isSupport("wifiproxy");
var lyra_hide_support = isSupport("lyra_hide");
var hdspindown_support = isSupport("hdspindown");
if ("<% nvram_get("amas_force"); %>" == "1") {
	var amesh_support = uiSupport("amas");
	var cfg_sync_support = isSupport("cfg_sync");
} else {
	var amesh_support = false;
	var cfg_sync_support = false;
}
var ifttt_support = isSupport("ifttt");
var alexa_support = isSupport("alexa");
var hnd_support = isSupport("hnd");
var tagged_based_vlan = isSupport("tagged_based_vlan");
var vpn_fusion_support = isSupport("vpn_fusion");
var meoVoda_support = isSupport("meoVoda");
var movistarTriple_support = isSupport("movistarTriple");

var QISWIZARD = "QIS_wizard.htm";

var wl_version = "<% nvram_get("wl_version"); %>";
var sdk_version_array = new Array();
sdk_version_array = wl_version.split(".");
var sdk_9 = sdk_version_array[0] == 9 ? true : false;
var sdk_7 = sdk_version_array[0] == 7 ? true : false;
var sdk_5 = sdk_version_array[0] == 5 ? true : false;
var bcm_mumimo_support = isSupport("mumimo");		//Broadcom MU-MIMOs
var nt_center_support = isSupport("nt_center");
var dblog_support = isSupport("dblog");

if(nt_center_support)
	document.write('<script type="text/javascript" src="/client_function.js"></script>');

if(live_update_support){
	if((exist_extendno.indexOf("beta") != -1) || (exist_extendno.indexOf("alpha") != -1))
		var current_firmware_path = 1;
	else
		var current_firmware_path = 0;	
}	
else{
	var current_firmware_path = 0;
}	

// Todo: Support repeater mode
/*if(isMobile() && sw_mode != 2 && !dsl_support)
	QISWIZARD = "MobileQIS_Login.asp";*/

//T-Mobile, force redirect to Mobile QIS page if client is mobile devices
if(tmo_support && isMobile()){	
	if(location.pathname != "/MobileQIS_Login.asp")
		location.href = "MobileQIS_Login.asp";
}
	
if(isMobile() && (based_modelid == "RT-AC88U" || based_modelid == "RT-AC86U" || based_modelid == "AC2900" || based_modelid == "RT-AC3100" || based_modelid == "RT-AC5300" || based_modelid == "GT-AC5300" || based_modelid == "GT-AC9600"))
	QISWIZARD = "QIS_wizard_m.htm";	

var stopFlag = 0;

var gn_array_2g = <% wl_get_guestnetwork("0"); %>;
var gn_array_5g = <% wl_get_guestnetwork("1"); %>;
var gn_array_5g_2 = <% wl_get_guestnetwork("2"); %>;
var gn_array_60g = <% wl_get_guestnetwork("3"); %>;
if(lyra_hide_support){
	gn_array_2g.splice(1, gn_array_2g.length-1);
	gn_array_5g = [];
	gn_array_5g_2 = [];
}

//notification value
var notice_pw_is_default = '<% check_pw(); %>';
if(notice_pw_is_default == 1 && window.location.pathname.toUpperCase().search("QIS_") < 0)	//force to change http_passwd / http_username & except QIS settings
		location.href = 'Main_Password.asp?nextPage=' + window.location.pathname.substring(1 ,window.location.pathname.length);

var allUsbStatus = "";
var allUsbStatusTmp = "";
var allUsbStatusArray = '<% show_usb_path(); %>'.toArray();


var wan_line_state = "<% nvram_get("dsltmp_adslsyncsts"); %>";
var wan_diag_state = "<% nvram_get("dslx_diag_state"); %>";
var wlan0_radio_flag = "<% nvram_get("wl0_radio"); %>";
var wlan1_radio_flag = "<% nvram_get("wl1_radio"); %>";
var wlan2_radio_flag = "<% nvram_get("wl2_radio"); %>";

var diag_dblog_enable = "<% nvram_get("dblog_enable"); %>";
var diag_dblog_remaining = "<% nvram_get("dblog_remaining"); %>";

//for high power model
var auto_channel = '<% nvram_get("AUTO_CHANNEL"); %>';
var is_high_power = auto_channel ? true : false;

function change_wl_unit_status(_unit){
	// if(sw_mode == 2 || sw_mode == 4) return false;
	
	document.titleForm.wl_unit.disabled = false;
	document.titleForm.wl_unit.value = _unit;

	if(sw_mode == 2 && concurrep_support){
		document.titleForm.wl_subunit.disabled = false;
		document.titleForm.wl_subunit.value = 1;
	}

	if(document.titleForm.current_page.value == "")
		document.titleForm.current_page.value = "Advanced_Wireless_Content.asp";
	if(document.titleForm.next_page.value == "")
		document.titleForm.next_page.value = "Advanced_Wireless_Content.asp";

	document.titleForm.action_mode.value = "change_wl_unit";
	document.titleForm.action = "apply.cgi";
	document.titleForm.target = "";
	document.titleForm.submit();
}

var dsltmp_transmode_orig = '<% nvram_get("dsltmp_transmode"); %>';
var wans_dualwan_orig = '<% nvram_get("wans_dualwan"); %>';
var wans_dualwan_array = new Array();
wans_dualwan_array = wans_dualwan_orig.split(" ");
var usb_index = wans_dualwan_array.getIndexByValue("usb");
var dsl_index = wans_dualwan_array.getIndexByValue("dsl");
var active_wan_unit = '<% get_wan_unit(); %>';
var wan0_enable = '<% nvram_get("wan0_enable"); %>';
var wan1_enable = '<% nvram_get("wan1_enable"); %>';
var dualwan_enabled = (dualWAN_support && wans_dualwan_orig.search("none") == -1) ? 1 : 0;

var realip_support = isSupport("realip");
var realip_state = "";
var realip_ip = "";
var external_ip = 0;

if(lyra_hide_support){
	var Android_app_link = "https://play.google.com/store/apps/details?id=com.asus.hive";
	var IOS_app_link = "https://itunes.apple.com/us/app/asus-lyra/id1171291475";
}else{
	var Android_app_link = "https://play.google.com/store/apps/details?id=com.asus.aihome";
	var IOS_app_link = "https://itunes.apple.com/tw/app/asus-router/id1033794044";
}

var banner_code, menu_code="", menu1_code="", menu2_code="", tab_code="", footer_code;
function show_banner(L3){// L3 = The third Level of Menu
	var banner_code = "";

	// creat a hidden iframe to cache offline page
//	if(get_protocol() == "http:")
//		banner_code +='<iframe width="0" height="0" frameborder="0" scrolling="no" src="/manifest.asp"></iframe>';

	banner_code +='<form method="post" name="titleForm" id="titleForm" action="/start_apply.htm" target="hidden_frame">\n';
	banner_code +='<input type="hidden" name="next_page" value="">\n';
	banner_code +='<input type="hidden" name="current_page" value="">\n';
	banner_code +='<input type="hidden" name="action_mode" value="apply">\n';
	banner_code +='<input type="hidden" name="action_script" value="">\n';
	banner_code +='<input type="hidden" name="action_wait" value="5">\n';
	banner_code +='<input type="hidden" name="wl_unit" value="" disabled>\n';
	banner_code +='<input type="hidden" name="wl_subunit" value="-1" disabled>\n';
	banner_code +='<input type="hidden" name="preferred_lang" value="<% nvram_get("preferred_lang"); %>">\n';
	banner_code +='<input type="hidden" name="flag" value="">\n';
	banner_code +='<input type="hidden" name="wan_unit" value="" disabled>\n';
	if(gobi_support && (usb_index != -1) && (sim_state != "")){
		banner_code +='<input type="hidden" name="sim_order" value="" disabled>\n';
	}
	banner_code +='</form>\n';

	banner_code +='<form method="post" name="diskForm_title" action="/device-map/safely_remove_disk.asp" target="hidden_frame">\n';
	banner_code +='<input type="hidden" name="disk" value="">\n';
	banner_code +='</form>\n';
	
	banner_code +='<form method="post" name="noti_ftp" action="/aidisk/switch_share_mode.asp" target="hidden_frame">\n';
	banner_code +='<input type="hidden" name="protocol" value="ftp">\n';
	banner_code +='<input type="hidden" name="mode" value="account">\n';
	banner_code +='</form>\n';
	
	banner_code +='<form method="post" name="noti_samba" action="/aidisk/switch_share_mode.asp" target="hidden_frame">\n';
	banner_code +='<input type="hidden" name="protocol" value="cifs">\n';
	banner_code +='<input type="hidden" name="mode" value="account">\n';
	banner_code +='</form>\n';
	
	banner_code +='<form method="post" name="noti_experience_Feedback" action="/start_apply.htm" target="hidden_frame">\n';
	banner_code +='<input type="hidden" name="next_page" value="">\n';
	banner_code +='<input type="hidden" name="current_page" value="">\n';
	banner_code +='<input type="hidden" name="action_mode" value="apply">\n';
	banner_code +='<input type="hidden" name="action_script" value="">\n';
	banner_code +='<input type="hidden" name="action_wait" value="">\n';	
	banner_code +='<input type="hidden" name="fb_experience" value="1">\n';
	banner_code +='</form>\n';

	banner_code +='<form method="post" name="noti_notif_hint" action="/start_apply.htm" target="hidden_frame">\n';
	banner_code +='<input type="hidden" name="next_page" value="">\n';
	banner_code +='<input type="hidden" name="current_page" value="">\n';
	banner_code +='<input type="hidden" name="action_mode" value="apply">\n';
	banner_code +='<input type="hidden" name="action_script" value="">\n';
	banner_code +='<input type="hidden" name="action_wait" value="">\n';    
	banner_code +='<input type="hidden" name="noti_notif_Flag" value="0">\n';
	banner_code +='</form>\n'; 

	banner_code +='<form method="post" name="internetForm_title" action="/start_apply2.htm" target="hidden_frame">\n';
	banner_code +='<input type="hidden" name="current_page" value="<% abs_index_page(); %>">\n';
	banner_code +='<input type="hidden" name="next_page" value="<% abs_index_page(); %>">\n';
	banner_code +='<input type="hidden" name="action_mode" value="apply">\n';
	banner_code +='<input type="hidden" name="action_script" value="restart_wan_if">\n';
	banner_code +='<input type="hidden" name="action_wait" value="5">\n';
	banner_code +='<input type="hidden" name="wan_enable" value="<% nvram_get("wan_enable"); %>">\n';
	banner_code +='<input type="hidden" name="wan_unit" value="<% get_wan_unit(); %>" >\n';
	banner_code +='<input type="hidden" name="modem_enable" value="<% nvram_get("modem_enable"); %>">\n';
	banner_code +='<input type="hidden" name="dslx_link_enable" value="<% nvram_get("dslx_link_enable"); %>">\n';
	banner_code +='</form>\n';

	banner_code +='<form method="post" name="rebootForm" action="apply.cgi" target="hidden_frame">\n';
	banner_code +='<input type="hidden" name="action_mode" value="reboot">\n';
	banner_code +='<input type="hidden" name="action_script" value="">\n';
	banner_code +='<input type="hidden" name="action_wait" value="<% get_default_reboot_time(); %>">\n';
	banner_code +='</form>\n';
	
	banner_code +='<form method="post" name="canceldiagForm" action="apply.cgi" target="hidden_frame">\n';
	banner_code +='<input type="hidden" name="action_mode" value="apply">\n';
	banner_code +='<input type="hidden" name="rc_service" value="stop_dsl_diag">\n';
	banner_code +='<input type="hidden" name="action_wait" value="">\n';	
	banner_code +='</form>\n';

	if(dblog_support) {
		banner_code +='<form method="post" name="canceldblogForm" action="apply.cgi" target="hidden_frame">\n';
		banner_code +='<input type="hidden" name="action_mode" value="apply">\n';
		banner_code +='<input type="hidden" name="rc_service" value="stop_dblog">\n';
		banner_code +='<input type="hidden" name="action_wait" value="">\n';
		banner_code +='<input type="hidden" name="dblog_enable" value="0">\n';
		banner_code +='</form>\n';
	}

	//banner_code +='<form method="post" name="wan_form" action="/start_apply.htm" target="hidden_frame">\n';
	//banner_code +='<input type="hidden" name="next_page" value="<% abs_index_page(); %>">\n';
	//banner_code +='<input type="hidden" name="current_page" value="<% abs_index_page(); %>">\n';
	//banner_code +='<input type="hidden" name="action_mode" value="apply">\n';
	//banner_code +='<input type="hidden" name="action_script" value="restart_wan_if">\n';
	//banner_code +='<input type="hidden" name="action_wait" value="5">\n';    
	//banner_code +='<input type="hidden" name="wan_unit" value="<% nvram_get("wan_unit"); %>">\n';    
	//banner_code +='<input type="hidden" name="wan_pppoe_username" value="<% nvram_get("wan_pppoe_username"); %>">\n';    
	//banner_code +='</form>\n'; 
	
	if(bwdpi_support){
		banner_code +='<form method="post" name="qosDisableForm" action="/start_apply.htm" target="hidden_frame">\n';
		banner_code +='<input type="hidden" name="current_pgae" value="QoS_EZQoS.asp">\n';
		banner_code +='<input type="hidden" name="next_page" value="QoS_EZQoS.asp">\n';
		banner_code +='<input type="hidden" name="action_mode" value="apply">\n';
		banner_code +='<input type="hidden" name="action_script" value="reboot">\n';
		banner_code +='<input type="hidden" name="action_wait" value="<% get_default_reboot_time(); %>">\n';	
		banner_code +='<input type="hidden" name="qos_enable" value="0">\n';
		banner_code +='</form>\n';
	}	
	
	if(based_modelid == "GT-AC5300" || based_modelid == "GT-AC9600"){
		banner_code +='<div class="banner1" align="center"><img src="images/New_ui/logo_ROG.png" width="358" height="86" align="left">\n';
		banner_code +='<div style="margin-top:13px;margin-left:-90px;*margin-top:0px;*margin-left:0px;" align="center"><span id="modelName_top" onclick="this.focus();" class="modelName_top"><#Web_Title2#></span></div>';
		// logout
		banner_code +='<a href="javascript:logout();"><div style="margin:13px 0 0 160px;*width:136px;background:url(\'images/New_ui/btn_logout.png\') no-repeat;background-size:cover;width:132px;height:34px;float:left;" align="center"><div style="margin:8px 0 0 15px;"><#t1Logout#></div></div></a>\n';		
	}
	else if(spirit_logo_support){
		banner_code +='<div class="banner1" align="center"><img src="images/New_ui/asus_spirit_title.png" width="214" height="31" align="left" style="margin-top:13px;margin-left:30px;">\n';
	}
	else{
		banner_code +='<div class="banner1" align="center"><img src="images/New_ui/asustitle.png" width="218" height="54" align="left">\n';
		banner_code +='<div style="margin-top:13px;margin-left:-90px;*margin-top:0px;*margin-left:0px;" align="center"><span id="modelName_top" onclick="this.focus();" class="modelName_top"><#Web_Title2#></span></div>';
		banner_code +='<div style="margin-left:25px;width:160px;height:52px;margin-top:0px;float:left;" align="left"><span><a href="https://asuswrt.lostrealm.ca/" target="_blank"><img src="images/merlin-logo.png" style="border: 0;"></span></div>';

		// logout, reboot
		banner_code +='<a href="javascript:logout();"><div style="margin-top:13px;margin-left:25px; *width:136px;" class="titlebtn" align="center"><span><#t1Logout#></span></div></a>\n';
		banner_code +='<a href="javascript:reboot();"><div style="margin-top:13px;margin-left:0px;*width:136px;" class="titlebtn" align="center"><span><#BTN_REBOOT#></span></div></a>\n';
	}

	// language
	banner_code +='<ul class="navigation">';
	banner_code +='<% shown_language_css(); %>';
	banner_code +='</ul>';

	banner_code +='</div>\n';
	banner_code +='<table width="998" border="0" align="center" cellpadding="0" cellspacing="0" class="statusBar" style="margin:auto;">\n';
	banner_code +='<tr>\n';
	if(based_modelid == "GT-AC5300" || based_modelid == "GT-AC9600"){
		banner_code +='<td class="minup_bg" height="179" valign="top"><table width="764" border="0" cellpadding="0" cellspacing="0" height="35px" style="margin-left:230px;">\n';
	}else{
		banner_code +='<td class="minup_bg" height="179" valign="top"><table width="764" border="0" cellpadding="0" cellspacing="0" height="35px" style="margin-left:230px;">\n';
	}
	banner_code +='<tbody><tr>\n';
 	banner_code +='<td valign="center" class="titledown" width="auto">';

	// dsl does not support operation mode
	if (!dsl_support && based_modelid != "AC2900" && !lyra_hide_support) {	//MODELDEP: AC2900(RT-AC86U)
		banner_code +='<span style="font-family:Verdana, Arial, Helvetica, sans-serif;"><#menu5_6_1_title#>:</sapn><span class="title_link" style="text-decoration: none;" id="op_link"><a href="/Advanced_OperationMode_Content.asp" style="color:white"><span id="sw_mode_span" style="text-decoration: underline;"></span></a></span>\n';
	}
	if(lyra_hide_support)
		banner_code +='<span style="font-family:Verdana, Arial, Helvetica, sans-serif;"><#General_x_FirmwareVersion_itemname#></sapn><span id="firmver" class="title_link"></span></a>\n';
	else
		banner_code +='<span style="font-family:Verdana, Arial, Helvetica, sans-serif;"><#General_x_FirmwareVersion_itemname#></sapn><a href="/Advanced_FirmwareUpgrade_Content.asp" style="color:white;"><span id="firmver" class="title_link"></span></a>\n';
	banner_code +='<span style="font-family:Verdana, Arial, Helvetica, sans-serif;" id="ssidTitle">SSID:</sapn>';
	if(lyra_hide_support)
		banner_code +='<span id="elliptic_ssid_2g" class="title_link"></span>';
	else
		banner_code +='<span onclick="change_wl_unit_status(0)" id="elliptic_ssid_2g" class="title_link"></span>';
	banner_code +='<span onclick="change_wl_unit_status(1)" id="elliptic_ssid_5g" class="title_link"></span>\n';
	if(wl_info.band5g_2_support)
		banner_code +='<span onclick="change_wl_unit_status(2)" id="elliptic_ssid_5g_2" class="title_link"></span>\n';
	banner_code +='</td>\n';

	if(dblog_support)
		banner_code +='<td id="dblog_diag_status_td"><div id="dblog_diag_status" class="dblogdiag"></div></td>\n';

	if(nt_center_support)
		banner_code +='<td width="30" id="notification_status_td"><div><div id="noti_event_count" style="display:none"><div id="noti_event_num"></div></div><div id="notification_status"></div></div><div id="notification_desc"></div></td>\n';
	else
		banner_code +='<td width="30" id="notification_status1" class="notificationOn"><div id="notification_status" class="notificationOn"></div><div id="notification_desc" class=""></div></td>\n';
	
	if(bwdpi_support && qos_enable_flag && qos_type_flag == "1")
		banner_code +='<td width="30"><div id="bwdpi_status" class=""></div></td>\n';	
		
//	if(wifi_hw_sw_support && !downsize_8m_support && !downsize_4m_support){
			banner_code +='<td width="30"><div id="wifi_hw_sw_status" class="wifihwswstatusoff"></div></td>\n';
//	}

	if(cooler_support)
		banner_code +='<td width="30"><div id="cooler_status" class="" style="display:none;"></div></td>\n';
	
	if(multissid_support != -1)
		banner_code +='<td width="30"><div id="guestnetwork_status" style="width:30px;" class="guestnetworkstatusoff"></div></td>\n';

	if(dsl_support)
		banner_code +='<td width="30"><div id="adsl_line_status" class="linestatusdown"></div></td>\n';

	if(sw_mode != 3)
		banner_code +='<td width="30"><div id="connect_status" class="connectstatusoff"></div></td>\n';

	if(usb_support)
		banner_code +='<td width="30"><div id="usb_status"></div></td>\n';
	
	if(based_modelid == "GT-AC5300" || based_modelid == "GT-AC9600"){
		banner_code +='<td><div id="reboot_status" class="reboot_status" onclick="reboot();"></div></td>\n';
	}

	if(printer_support && !noprinter_support)
		banner_code +='<td width="30" style="display:none"><div id="printer_status" class="printstatusoff"></div></td>\n';

	/* Cherry Cho added in 2014/8/22. */
	if(((modem_support && hadPlugged("modem") && !nomodem_support) || gobi_support) && (usb_index != -1) && (sim_state != "")){
		banner_code +='<td width="30"><div id="sim_status" class="simnone"></div></td>\n';
	}	

	if(gobi_support && (usb_index != -1) && (sim_state != "")){
		banner_code +='<td width="30"><div id="simsignal" class="simsignalno"><div class="img_wrap"><div id="signalsys" class="signalsysimg"></div></div></div></td>\n';
		if(roaming == "1")
			banner_code +='<td width="30"><div id="simroaming_status" class="simroamingoff"></div></td>\n';		
	}
	
	banner_code +='<td width="17"></td>\n';
	banner_code +='</tr></tbody></table></td></tr></table>\n';

	/* Traffic Limit Warning*/
	if(gobi_support && (usb_index != -1) && (sim_state != "")){	
		var setCookie = 0;
		traffic_warning_cookie = cookie.get(keystr);
		if(traffic_warning_cookie == null){
			setCookie = 1;
		}
		else{
			var cookie_year = traffic_warning_cookie.substring(0,4);
			var indexOfcolon = traffic_warning_cookie.indexOf(':');
			var cookie_month = traffic_warning_cookie.substring(5, indexOfcolon);

			if(cookie_year == date_year && cookie_month == date_month)
				traffic_warning_flag = parseInt(traffic_warning_cookie.substring(indexOfcolon + 1));
			else
				setCookie = 1;
		}

		if(setCookie){
			set_traffic_show("1");
		}

		banner_code +='<div id="mobile_traffic_warning" class="traffic_warning" style="display: none; opacity:0;">\n';
		banner_code +='<div style="text-align:right"><img src="/images/button-close.gif" style="width:30px;cursor:pointer" onclick="slow_hide_warning();"></div>';
		banner_code +='<div style="margin:0px 30px 10px;">';
		banner_code += "<#Mobile_limit_warning#>";
		banner_code +='</div>';
		banner_code +="<span><input style=\"margin-left:30px;\" type=\"checkbox\" name=\"stop_show_chk\" id=\"stop_show_chk\"><#Mobile_stop_warning#></input></span>";
		banner_code +='<div style="margin-top:20px;padding-bottom:10px;width:100%;text-align:center;">';
		banner_code +='<input id="changeButton" class="button_gen" type="button" value="Change" onclick="setTrafficLimit();">';
		banner_code +='</div>';
		banner_code +='</div>';

		var show_flag = '<% get_parameter("show"); %>';
		if(show_flag != "0" && traffic_warning_flag && (modem_bytes_data_limit > 0)){
			var data_usage = rx_bytes + tx_bytes;
			if( data_usage >= modem_bytes_data_limit)
				setTimeout("show_traffic_warning();", 600);
		}
	}

	document.getElementById("TopBanner").innerHTML = banner_code;
	
	show_loading_obj();
	show_top_status();
	updateStatus();

	if(app_support && !isIE8){
		document.body.addEventListener('click', show_app_table, false);
	}

	if(lyra_hide_support){
		document.getElementById("elliptic_ssid_5g").style.display = "none";
		if(wl_info.band5g_2_support)
			document.getElementById('elliptic_ssid_5g_2').style.display = "none";
		document.getElementById('elliptic_ssid_2g').style.textDecoration = "none";
		document.getElementById('elliptic_ssid_2g').style.cursor = "auto";
		document.getElementById('firmver').style.textDecoration = "none";
		document.getElementById('firmver').style.cursor = "auto";
	}
}

function show_app_table(evt){
	
	var target = document.getElementById("app_link_table");
	var evt_target = evt.target || evt.srcElement;	//evt.target for Firefox patched
	
	if(evt_target.id == "app_icon" || evt_target.id == "cancel_app"){
		if(target.style.display == "none"){
			target.style.display = "";		
		}
		else{
			target.style.display = "none";
		}
		
	}
	else if(evt_target.offsetParent == null){
		if(target.style.display == ""){
			target.style.display = "none";
		}			
		
	}
	else if((evt_target.id != "null" && evt_target.id == "app_link_table") || (evt_target.offsetParent.id != "null" && evt_target.offsetParent.id) == "app_link_table"){	
		return true;

	}
	else{
		if(target.style.display == ""){
			target.style.display = "none";
		}
		
	}
}

function set_traffic_show(flag){ //0:hide 1:show
	traffic_warning_cookie = date_year + '.' + date_month + ':' + flag;
	cookie.set(keystr, traffic_warning_cookie, 1000);
	traffic_warning_flag = parseInt(flag);
}

var opacity = 0;
var inc = 1/50;
function slow_show_warning(){
	document.getElementById("mobile_traffic_warning").style.display = "";
	opacity = opacity + inc;
	document.getElementById("mobile_traffic_warning").style.opacity = opacity;
	if(document.getElementById("mobile_traffic_warning").style.opacity < 1)
		setTimeout("slow_show_warning();", 1);
}

function slow_hide_warning(){
	document.getElementById("mobile_traffic_warning").style.display = "none";
	opacity = document.getElementById("mobile_traffic_warning").style.opacity;
	if(opacity == 1 && document.getElementById("stop_show_chk").checked == true){
		set_traffic_show("0");
	}

	opacity = opacity - inc;
	document.getElementById("mobile_traffic_warning").style.opacity = opacity;
	if(document.getElementById("mobile_traffic_warning").style.opacity > 0)
		setTimeout("slow_hide_warning();", 1);
}

var clickListener = function(event){
	var traffic_waring_element = document.getElementById("mobile_traffic_warning");

	if(event.target.id != 'mobile_traffic_warning' && !traffic_waring_element.contains(event.target))
    	hide_traffic_warning();
};

function show_traffic_warning(){
	var statusframe= document.getElementById("statusframe");
	var statusframe_content;

	slow_show_warning();
	document.addEventListener('click', clickListener, false);
	if(statusframe){
		statusframe_content = statusframe.contentWindow.document;
		statusframe_content.addEventListener('click', clickListener, false);
	}
}

function hide_traffic_warning(){
	var statusframe= document.getElementById("statusframe");
	var statusframe_content;

	slow_hide_warning();
	// disable event listener
	document.removeEventListener('click', clickListener, false);
	if(statusframe){
		statusframe_content = statusframe.contentWindow.document;
		statusframe_content.removeEventListener('click', clickListener, false);
	}

}

/*
//for changing CHT PPPoE to PPPoE staitc IP
function change_cht_pppoe_static(){
	var wan_pppoe_username = '<% nvram_get("wan_pppoe_username"); %>';
	var cht_pppoe = wan_pppoe_username.split("@");
	var temp = "";
	temp = cht_pppoe[0] + "@ip." + cht_pppoe[1];
	
	document.wan_form.wan_pppoe_username.value = temp;
	document.wan_form.submit();
}*/

function get_helplink(){
	var href_lang = get_supportsite_lang();
	var model_name_supportsite = supportsite_model(support_site_modelid, hw_ver);  // @ /js/support_site.js

	var getlink="https://www.asus.com"+href_lang+"Networking/" +model_name_supportsite+ "/HelpDesk_Download/";       
	return getlink;
}

function Block_chars(obj, keywordArray){
	// bolck ascii code 32~126 first
	var invalid_char = "";		
	for(var i = 0; i < obj.value.length; ++i){
		if(obj.value.charCodeAt(i) < '32' || obj.value.charCodeAt(i) > '126'){
			invalid_char += obj.value.charAt(i);
		}
	}
	if(invalid_char != ""){
		alert('<#JS_validstr2#>" '+ invalid_char +'" !');
		obj.focus();
		return false;
	}

	// check if char in the specified array
	if(obj.value){
		for(var i=0; i<keywordArray.length; i++){
			if( obj.value.indexOf(keywordArray[i]) >= 0){						
				alert(keywordArray+ " <#JS_invalid_chars#>");
				obj.focus();
				return false;
			}	
		}
	}
	
	return true;
}

function submitenter(myfield,e)
{
	var keycode;
	if (window.event) keycode = window.event.keyCode;
	else if (e) keycode = e.which;
	else return true;
	if (keycode == 13){
		search_supportsite();
		return false;
	}
	else
		return true;
}

function show_menu(){
	var wan_pppoe_username = decodeURIComponent('<% nvram_char_to_ascii("", "wan0_pppoe_username"); %>');
	var cht_pppoe = wan_pppoe_username.split("@");
	is_CHT_pppoe = (cht_pppoe[1] == "hinet.net") ? true : false;
	is_CHT_pppoe_static = (cht_pppoe[1] == "ip.hinet.net") ? true : false;

	show_banner();
	show_footer();
	show_selected_language();
	autoFocus('<% get_parameter("af"); %>');

	try{
		showMenuTree(Session.get("menuList"), Session.get("menuExclude"));
	}
	catch(e){
	}

	require(['/require/modules/menuTree.js'], function(menuTree){
		menuList = menuTree.list;

		menuExclude = {
			menus: menuTree.exclude.menus(),
			tabs: menuTree.exclude.tabs()
		};

		Session.set("menuList", menuList);
		Session.set("menuExclude", menuExclude);
		showMenuTree(menuList, menuExclude);
	});

	if(nt_center_support){
		notification.update_NT_Center();
	}

	notification.run();
	browser_compatibility();
}

var goToPage = function(menu, tab, obj){
	cookie.set("clickedItem_tab", tab);
	location.href = obj.title;
}

var orig_NM_table_height;
function showMenuTree(menuList, menuExclude){
	var clickedItem = {menu:2, tab:0};

	var getCode = function(flag){
		var getMenuCode = function(){
			var menu_code = '<div style="margin-top:-172px">';
			for(var i=0; i<menuList.length; i++){
				var curMenu = menuList[i];
				var firstEntry = -1;

				curMenu.tab.some(function(tab, idx){
					if(menuExclude.tabs.indexOf(tab.url) === -1){
						firstEntry = idx;
						return true;
					}
				})

				if(firstEntry === -1) continue;
				if(menuExclude.menus.indexOf(curMenu.index) !== -1) continue;

				if(curMenu.index === "menu_Split"){
					menu_code += '<div class="';
					menu_code += curMenu.index;
					menu_code += '"><table width="192px" height="30px"><tbody><tr><td>';
					menu_code += curMenu.menuName;
					menu_code += '</td></tr></tbody></table></div>';
				}
				else{
					if(curMenu.tab[firstEntry].url !== "NULL"){
						if(curMenu.index == "menu_QIS")
							menu_code += '<div class="menu menu_QIS_title';
						else
							menu_code += '<div class="menu';
						
						//-------Fine tune Menu icon start----------
						if(dualWAN_support && based_modelid != "BRT-AC828" && current_url.indexOf("Advanced_Modem_Content") == 0){
							menu_code += (i == clickedItem_menuWAN) ? ' menuClicked' : '';	//show WAN menu css as class 'menuClicked'
						}
						else{
							menu_code += (i == clickedItem.menu) ? ' menuClicked' : '';	
						}
						//-------Fine tune Menu icon end----------

						menu_code += '" onclick="goToPage('; 
						menu_code += i; 
						menu_code += ', '
						menu_code += firstEntry;
						menu_code += ', this);" title="';
						menu_code += curMenu.tab[firstEntry].url;
						menu_code += '" id="';
						menu_code += curMenu.tab[firstEntry].url.split(".")[0];
						menu_code += '_menu"><table><tr><td><div class="menu_Icon ';
						if(based_modelid == "RT-AC85U" && curMenu.index == 'menu_QoS')	//MODELDEP : RT-AC85U change icon
							menu_code += 'menu_BandwidthMonitor';
						else
							menu_code += curMenu.index;
						menu_code += '"></div></td><td class="menu_Desc">';
						if(curMenu.index == 'menu_Alexa_IFTTT' && (!alexa_support || !ifttt_support)){
							if(alexa_support)
								menu_code += "Amazon Alexa";
							else
								menu_code += "IFTTT";
						}
						else
						menu_code += curMenu.menuName;
						menu_code += '</td></tr></table></div>\n';
					}
				}
			}
			menu_code += "</div>";
			return menu_code;
		}

		var getTabCode = function(){
			var tab_code = "", tabCounter = 0;
			if(dualWAN_support && based_modelid != "BRT-AC828" && current_url.indexOf("Advanced_Modem_Content") == 0){
				clickedItem.menu = clickedItem_menuWAN;			//show tab group as WAN while dualWAN_support
			}
			for(var j=0; j<menuList[clickedItem.menu].tab.length; j++){
				var curTab = menuList[clickedItem.menu].tab[j];

				if(curTab.tabName === "__HIDE__" || curTab.tabName === "__INHERIT__") continue;
				if(menuExclude.tabs.indexOf(curTab.url) !== -1) continue;

				tab_code += '<td><div class="';
	
				//-----Fine tune tab icon start-----------				
				if( dsl_support && (current_url.indexOf("Advanced_DSL_Content") == 0 || current_url.indexOf("Advanced_VDSL_Content") == 0 || current_url.indexOf("Advanced_WAN_Content") == 0)){
					tab_code += (j == 1) ? 'tabClicked' : 'tab';	//show 1st tab css as class 'tabClicked'
				}
				else if((based_modelid != "RT-AC65U" && based_modelid != "BRT-AC828") && (current_url.indexOf("AiProtection_WebProtector") == 0 || current_url.indexOf("ParentalControl") == 0)){
					//Should remove these MODELDEP issue after we have dpi_support supported to display bwdpi related pages.
					tab_code += (j == 5) ? 'tabClicked' : 'tab';	//show 1st tab css as class 'tabClicked'
				}
				else if(dualWAN_support && based_modelid != "BRT-AC828" && current_url.indexOf("Advanced_Modem_Content") == 0){
					tab_code += (j == 0 || j == 3) ? 'tabClicked' : 'tab';	// Show fist tab css as class 'tabClicked'
				}
				else if(dualWAN_support && based_modelid != "BRT-AC828" && current_url.indexOf("Advanced_Modem_Content") == 0) {
					tab_code += (j == 0 || j == 3) ? 'tabClicked' : 'tab';	// Show fist tab css as class 'tabClicked'
				}
				else{
					tab_code += (j == clickedItem.tab) ? 'tabClicked' : 'tab';
				}
				//-----Fine tune tab icon end-----------

				tab_code += '" onclick="goToPage('; 
				tab_code += clickedItem.menu; 
				tab_code += ', ';
				tab_code += j;
				tab_code += ', this);" title="';
				tab_code += curTab.url;
				tab_code += '" id="';
				tab_code += curTab.url.split(".")[0];
				tab_code += '_tab"><span>';
				tab_code += curTab.tabName; 
				tab_code += '</span></div></td>';
				tabCounter ++;
			}

			var tab_container = "";
			if(menuList[clickedItem.menu].tab[clickedItem.tab].tabName !== "__HIDE__" && tabCounter > 1){
				tab_container += '<div><table>';
				tab_container += tab_code;
				tab_container += '</table></div>';
			}
			return tab_container;
		}

		switch(flag){
			case "menus":
				return getMenuCode();
			break;
			case "tabs":
				return getTabCode();
			break;
		}
	}

	//Find clickedItem.menu for WAN
	var clickedItem_menuWAN
	for(var z=0; z<menuList.length; z++){
		var curMenuWAN = menuList[z];
		for(var y=0; y<curMenuWAN.tab.length; y++){
			if("Advanced_WAN_Content.asp" === curMenuWAN.tab[y].url){
				clickedItem_menuWAN = z;
			}
		}
	}

	// set the index of current page to clickedItem
	for(var i=0; i<menuList.length; i++){
		var curMenu = menuList[i];
		for(var j=0; j<curMenu.tab.length; j++){
			if(current_url === curMenu.tab[j].url){
				clickedItem.menu = i;

				if(curMenu.tab[j].url.indexOf("Advanced_MobileBroadband_Content") == 0 && dualwan_enabled){
					curMenu.tab[j].tabName = "__INHERIT__";
				}

				if(curMenu.tab[j].tabName !== "__INHERIT__"){
					clickedItem.tab = j;
				}
				else{
					clickedItem.tab = (curMenu.tab[cookie.get("clickedItem_tab")]) ? parseInt(cookie.get("clickedItem_tab")) : 0;						
				}
				cookie.set("clickedItem_tab", clickedItem.tab);
				break;
			}
		}
	}

	document.getElementById("mainMenu").innerHTML = getCode("menus");
	document.getElementById("tabMenu").innerHTML = getCode("tabs");

	var getTableHeight = function(){
		var tab = document.getElementById("tabMenu").clientHeight;
		var main = document.getElementById("mainMenu").clientHeight;
		var factor = 15;
		return (main - tab - factor);
	}

	var getStyle = function(oElm, strCssRule){
		var strValue = "";
		if(document.defaultView && document.defaultView.getComputedStyle){
			strValue = document.defaultView.getComputedStyle(oElm, "").getPropertyValue(strCssRule);
		}
		else if(oElm.currentStyle){
			strCssRule = strCssRule.replace(/\-(\w)/g, function (strMatch, p1){
				return p1.toUpperCase();
			});
			strValue = oElm.currentStyle[strCssRule];
		}
		return strValue;
	}

	var tableHeight = getTableHeight();
	// general page
	if(document.getElementById("FormTitle")){
		var CONTENT_PADDING = parseInt(getStyle(document.getElementById("FormTitle"), "padding-top")) + parseInt(getStyle(document.getElementById("FormTitle"), "padding-bottom"));
		if(current_url.indexOf("Advanced_AiDisk_ftp") != 0 && current_url.indexOf("Advanced_AiDisk_samba") != 0){
			if(current_url.indexOf("GameDashboard") != -1){
				document.getElementById("FormTitle").style.height = "908px";
			}
			else{
				document.getElementById("FormTitle").style.height = tableHeight - CONTENT_PADDING + "px";
			}
		}
	}
	// index.asp
	else if(document.getElementById("NM_table")){
		if(usbPortMax == 3) tableHeight = 1060;
		tableHeight = (tableHeight < 930) ? 930 : tableHeight;
		var NM_TABLE_PADDING = parseInt(getStyle(document.getElementById("NM_table"), "padding-top")) + parseInt(getStyle(document.getElementById("NM_table"), "padding-bottom"));
		document.getElementById("NM_table").style.height = (tableHeight - NM_TABLE_PADDING) + "px";
		orig_NM_table_height = tableHeight - NM_TABLE_PADDING;
	}

	if(document.titleForm.preferred_lang.value == "JP"){
		var els = document.getElementsByClassName('menu_Desc');
		for (var i=0; i<els.length; i++) {
			els[i].style.wordBreak = "break-all";
		}
	}	
}

function set_NM_height(){
	var table_height = document.getElementById("rt_table").clientHeight;
	var paddingTop = parseInt(parent.$(".NM_table").css("padding-top"));
	var title_height = parseInt(parent.$("#statusTitle_NM").css("height"));
	var total = table_height + paddingTop + title_height;
	var factor = 15;
	if(parent.orig_NM_container_height < table_height){
		parent.$("#NM_table").css("height", total);
		parent.$("#statusframe").css("height", table_height + factor );
		parent.$(".NM_radius_bottom_container").css("height", table_height + factor );
	}
	else{
		parent.$("#NM_table").css("height", parent.orig_NM_table_height);
		parent.$("#statusframe").css("height",  parent.orig_NM_container_height );
		parent.$(".NM_radius_bottom_container").css("height", parent.orig_NM_container_height );
	}
}

function reset_NM_height(){
	parent.$("#NM_table").css("height", parent.orig_NM_table_height);
	parent.$("#statusframe").css("height",  parent.orig_NM_container_height );
	parent.$(".NM_radius_bottom_container").css("height", parent.orig_NM_container_height );
}

function show_footer(){
	var href_lang = get_supportsite_lang();
	if(based_modelid == "RT-N56UB1" || based_modelid == "RT-N56UB2" ||
			based_modelid == "DSL-AC68U" || based_modelid == "DSL-AC68R" || 
			based_modelid == "DSL-N55U" || based_modelid == "DSL-N55U-B" || 
			based_modelid == "RT-N11P" || based_modelid == "RT-N300")
			href_lang = "/";	//global only
			
	footer_code = '<div align="center" class="bottom-image"></div>\n';
	footer_code +='<div align="center" class="copyright"><#footer_copyright_desc#></div><br>';

	// FAQ searching bar{
	footer_code += '<div style="margin-top:-75px;margin-left:205px;"><table width="765px" border="0" align="center" cellpadding="0" cellspacing="0"><tr>';
	footer_code += '<td width="20" align="right"><div id="bottom_help_icon" style="margin-right:3px;"></div></td><td width="110" id="bottom_help_title" align="left"><#Help#> & <#Support#></td>';

	var tracing_path_Manual	= "/HelpDesk_Manual/?utm_source=asus-product&utm_medium=referral&utm_campaign=router";
	var tracing_path_Utility = "/HelpDesk_Download/?utm_source=asus-product&utm_medium=referral&utm_campaign=router";
	var model_name_supportsite = supportsite_model(support_site_modelid, hw_ver);	// @ /js/support_site.js	
			
	footer_code += "<td width=\"335\" id=\"bottom_help_link\" align=\"left\"><a style=\"font-weight: bolder;text-decoration:underline;cursor:pointer;\" href=\"https://www.asus.com"+ href_lang +"Networking/"+ model_name_supportsite + tracing_path_Manual +"\" target=\"_blank\"><#Manual#></a>&nbsp|&nbsp<a style=\"font-weight: bolder;text-decoration:underline;cursor:pointer;\" href=\"https://www.asus.com"+ href_lang +"Networking/" + model_name_supportsite + tracing_path_Utility +"\" target=\"_blank\"><#Utility#></a>";

	if(dsl_support && feedback_support){
		footer_code += '&nbsp|&nbsp<a id="fb_link" href="/Advanced_Feedback.asp" target="_self" style="font-weight: bolder;text-decoration:underline;cursor:pointer;"><#menu_feedback#></a>';
	}
	else if(feedback_support){
		var header_info = [<% get_header_info(); %>];
		var location_href = '/Advanced_Feedback.asp?origPage=' + header_info[0].current_page;
		footer_code += '&nbsp|&nbsp<a id="fb_link" href="'+location_href+'" target="_blank" style="font-weight: bolder;text-decoration:underline;cursor:pointer;"><#menu_feedback#></a>';
	}
	
	/* MODELDEP */
	if(based_modelid != "AC2900"){	//MODELDEP: AC2900(RT-AC86U)
		footer_code += '&nbsp|&nbsp<a id="registration_link" target="_blank" href="https://account.asus.com/" target="_self" style="font-weight: bolder;text-decoration:underline;cursor:pointer;"><#Product_Registration#></a>';
	}	

	//APP Link
	if(app_support){
		footer_code +='&nbsp;|&nbsp;<span id="app_icon" style="font-weight:bolder;text-decoration:underline;cursor:pointer;">App</span>';
		footer_code +='<div id="app_link_table" style="display:none;width:325px;height:360px;position:relative;top:-380px;background-color:rgb(35, 38, 41);z-index:10;margin-top:13px;margin-left:-170px;;border-radius:5px;box-shadow:3px 3px 4px #000;opacity:.95">';
		footer_code +='<div style="padding:10px;">';
		footer_code +='<div id="cancel_app" style="width:20px;height:20px;background:url(\'images/button-close.png\') no-repeat;position:absolute;right:10px;"></div>';
		footer_code +='</div>';
		//ASUS Router icon
		footer_code +='<div style="padding:10px;border-bottom:1px solid #666;">';
		footer_code +='<div style="display:table-cell;vertical-align:middle;padding-left:10px;">';
		footer_code +='<div style="width:75px;height:75px;background:url(\'images/New_ui/asus_router.png\') no-repeat;"></div>';
		footer_code +='</div>';
		footer_code +='<div style="display:table-cell;">';
		footer_code +='<div style="padding:5px 0 5px 15px;font-size:22px;">ASUS Router</div>';
		footer_code +='<div style="padding:5px 0 5px 15px;font-size:14px;color:#BDBDBD">ASUS Router is a mobile app for managing your home network.</div>';
		footer_code +='</div>';
		footer_code +='</div>';
		//Play Store
		footer_code +='<div style="padding:20px 10px;">';
		footer_code +='<div style="display:table-cell;vertical-align:middle;padding-left:10px;">';
		footer_code +='<div><img src="images/New_ui/asus_router_android_qr.png" style="width:75px;height:75px;"></div>';
		footer_code +='</div>';
		footer_code +='<div style="display:table-cell;vertical-align:middle;width:100%;text-align:center">';
		footer_code +='<div style="padding-left: 30px;"><a href="https://play.google.com/store/apps/details?id=com.asus.aihome" target="_blank"><div style="width:172px;height:60px;background:url(\'images/cloudsync/googleplay.png\') no-repeat;"></div></a></div>';
		footer_code +='</div>';	
		footer_code +='</div>';
		//AppStore
		footer_code +='<div style="padding:20px 10px;">';
		footer_code +='<div style="display:table-cell;vertical-align:middle;padding-left:10px;">';
		footer_code +='<div><img src="images/New_ui/asus_router_ios_qr.png" style="width:75px;height:75px;"></div>';
		footer_code +='</div>';
		footer_code +='<div style="display:table-cell;vertical-align:middle;width:100%;text-align:center">';
		footer_code +='<div style="padding-left: 30px;"><a href="https://itunes.apple.com/tw/app/asus-router/id1033794044" target="_blank"><div style="width:172px;height:51px;background:url(\'images/cloudsync/AppStore.png\') no-repeat;"></div></a></div>';
		footer_code +='</div>';	
		footer_code +='</div>';

		footer_code +='</div>';
	}
	// APP Link End

	footer_code += '</td>';
	footer_code += '<td width="270" id="bottom_help_FAQ" align="right" style="font-family:Arial, Helvetica, sans-serif;">FAQ&nbsp&nbsp<input type="text" id="FAQ_input" class="input_FAQ_table" maxlength="40" onKeyPress="submitenter(this,event);" autocorrect="off" autocapitalize="off"></td>';
	footer_code += '<td width="30" align="left"><div id="bottom_help_FAQ_icon" class="bottom_help_FAQ_icon" style="cursor:pointer;margin-left:3px;" target="_blank" onClick="search_supportsite();"></div></td>';
	footer_code += '</tr></table></div>\n';
	//}

	document.getElementById("footer").innerHTML = footer_code;
	flash_button();
}

function show_contactus(){
	
	var contactus_info = "";
	contactus_info += "<table border=\"0\" align=\"center\" cellpadding=\"0\" cellspacing=\"0\" style=\"width:280px;height:155px;padding:5px 10px 0px 10px;	-webkit-border-radius: 5px;-moz-border-radius: 5px;border-radius:5px;behavior: url(../PIE.htc);\">";
	contactus_info += "<tr><td>";
	contactus_info += "<div style=\"margin:5px 0px -5px 5px;font-family:Arial, Helvetica, sans-serif;font-size:14px;font-weight:bolder\">Contact us (Hotlines)</div>";
	contactus_info += "</td><td align=\"right\">";
	contactus_info += "<a href=\"javascript:close_contactus();\"><img width=\"18px\" height=\"18px\" src=\"/images/button-close2.png\" onmouseover=\"this.src='/images/button-close2.png'\" onmouseout=\"this.src='/images/button-close.png'\" border=\"0\"></a>";
	contactus_info += "</td></tr>";
	contactus_info += "<tr><td colspan=\"2\">";
	contactus_info += "<img style=\"width:90%;height:2px\" src=\"/images/New_ui/networkmap/linetwo2.png\">";
	contactus_info += "</td></tr>";
	contactus_info += "<tr><td colspan=\"2\"><div style=\"font-size:14px;margin-left:5px;text-shadow: 1px 1px 0px black;font-family: Arial, Helvetica, sans-serif;font-weight: bolder;\">Component Product Support :</div>";
	contactus_info += "<div style=\"font-size:16px;color :#FFCC00;margin-top:5px;margin-left:25px;text-shadow: 1px 1px 0px black;font-family: Arial, Helvetica, sans-serif;font-weight: bolder;margin-left:5px;\">1-812-282-2787</div>";
	contactus_info += "<br>";
	contactus_info += "<div style=\"font-size:13px;margin-left:5px;text-decoration:underline;font-family: Arial, Helvetica, sans-serif;\">Open Hour :</div>";
	contactus_info += "<div style=\"font-size:13px;margin-top:5px;margin-left:5px;font-family: Arial, Helvetica, sans-serif;font-weight: bolder;\">5:30 ~ 23:00 PST ( Monday ~ Friday )</div>";
	contactus_info += "<div style=\"font-size:13px;margin-left:5px;font-family: Arial, Helvetica, sans-serif;font-weight: bolder;\">6:00 ~ 15:00 PST ( Saturday ~ Sunday )</div>";
	contactus_info += "</td></tr>";
	contactus_info += "</table>";
	document.getElementById('contactus_block').innerHTML = contactus_info;
	document.getElementById('contactus_block').style.display = "";
	
}

function close_contactus(){
	document.getElementById('contactus_block').style.display = "none";
}

function get_supportsite_lang(obj){
	var faqLang = {
		EN : "/",
		TW : "/tw/",
		CN : ".cn/",
		BR : "/br/",
		CZ : "/cz/",
		DA : "/dk/",
		DE : "/de/",
		ES : "/es/",
		FI : "/fi/",
		FR : "/fr/",
		HU : "/hu/",
		IT : "/it/",
		JP : "/jp/",
		KR : "/kr/",
		MS : "/my/",
		NL : "/nl/",
		NO : "/no/",
		PL : "/pl/",
		RO : "/ro/",
		RU : "/ru/",
		SL : "/sk/",
		SV : "/se/",
		TH : "/th/",
		TR : "/tr/",
		UK : "/ua/"
	}	
	
	return faqLang['<% nvram_get("preferred_lang"); %>'] || faqLang["EN"];
}

function search_supportsite(){
	//	https://www.asus.com/search/results.aspx?SearchKey=%s&SearchType=FAQ&PDLineFilter=1849
	var faq_href = "https://www.asus.com";
	faq_href += get_supportsite_lang();
	faq_href += "search/results.aspx?SearchKey=";
	faq_href += (document.getElementById("FAQ_input")) ? document.getElementById("FAQ_input").value : '';
	faq_href += "&SearchType=FAQ&PDLineFilter=1849";

	window.open(faq_href);
}

var isFirefox = navigator.userAgent.search("Firefox") > -1;
var isOpera = navigator.userAgent.search("Opera") > -1;
var isIE8 = navigator.userAgent.search("MSIE 8") > -1; 
var isiOS = navigator.userAgent.search("iP") > -1; 
function browser_compatibility(){
	if(isiOS){
		var obj_inputBtn;
	
		/* language options */
		document.body.addEventListener("touchstart", mouseClick, false);

		obj_inputBtn = document.getElementsByClassName("button_gen");
		for(var i=0; i<obj_inputBtn.length; i++){
			obj_inputBtn[i].addEventListener('touchstart', function(){this.className = 'button_gen_touch';}, false);
			obj_inputBtn[i].addEventListener('touchend', function(){this.className = 'button_gen';}, false);
		}
		obj_inputBtn = document.getElementsByClassName("button_gen_long");
		for(var i=0; i<obj_inputBtn.length; i++){
			obj_inputBtn[i].addEventListener('touchstart', function(){this.className = 'button_gen_long_touch';}, false);
			obj_inputBtn[i].addEventListener('touchend', function(){this.className = 'button_gen_long';}, false);
		}
	}

	try{
		// if jQuery is available
		var $container = $("#tabMenu").parent();
		$('<div>')
			.css({"margin-top":"-140px"})
			.append($container.children())
			.appendTo($container)
	}
	catch(e){
		var container = document.getElementById('tabMenu').parentNode;
		var newDiv = document.createElement('div');	
		newDiv.style.marginTop = "-140px";
		for(var i=0; i<container.children.length; i++){
			newDiv.appendChild(container.children[i].cloneNode(true));
		}
		container.innerHTML = "";
		container.appendChild(newDiv);
	}
}	

var mouseClick = function(){
	var e = document.createEvent('MouseEvent');
	e.initEvent('click', false, false);
	document.getElementById("modelName_top").dispatchEvent(e);
}

function show_top_status(){
	if(!localAP_support){
		document.getElementById("ssidTitle").style.display = "none";
	}

	var ssid_status_2g =  decodeURIComponent('<% nvram_char_to_ascii("WLANConfig11b", "wl0_ssid"); %>');
	var ssid_status_5g =  decodeURIComponent('<% nvram_char_to_ascii("WLANConfig11b", "wl1_ssid"); %>');

	if(wl_info.band5g_2_support){
		var ssid_status_5g_2 =  decodeURIComponent('<% nvram_char_to_ascii("WLANConfig11b", "wl2_ssid"); %>');
	}

	if(!band2g_support)
		ssid_status_2g = "";

	if(!band5g_support)
		ssid_status_5g = "";

	if(!wl_info.band5g_2_support)
		ssid_status_5g_2 = "";

	if(sw_mode == 4){
		if('<% nvram_get("wlc_band"); %>' == '0'){
			document.getElementById("elliptic_ssid_2g").style.display = "none";
			document.getElementById("elliptic_ssid_5g").style.marginLeft = "";
		}
		else
			document.getElementById("elliptic_ssid_5g").style.display = "none";

		document.getElementById('elliptic_ssid_2g').style.textDecoration="none";
		document.getElementById('elliptic_ssid_2g').style.cursor="auto";
		document.getElementById('elliptic_ssid_5g').style.textDecoration="none";
		document.getElementById('elliptic_ssid_5g').style.cursor="auto";
		if(wl_info.band5g_2_support){
			document.getElementById('elliptic_ssid_5g_2').style.textDecoration="none";
			document.getElementById('elliptic_ssid_5g_2').style.cursor="auto";
		}
	}
	else if(sw_mode == 2){
		if(concurrep_support){
			ssid_status_2g =  decodeURIComponent('<% nvram_char_to_ascii("WLANConfig11b", "wl0.1_ssid"); %>');
			ssid_status_5g =  decodeURIComponent('<% nvram_char_to_ascii("WLANConfig11b", "wl1.1_ssid"); %>');
			if(wl_info.band5g_2_support)
				ssid_status_5g_2 =  decodeURIComponent('<% nvram_char_to_ascii("WLANConfig11b", "wl2.1_ssid"); %>');

			if(wlc_express == '1'){
				document.getElementById('elliptic_ssid_2g').style.display = "none";
			}
			else if(wlc_express == '2'){
				document.getElementById('elliptic_ssid_5g').style.display = "none";
			}
			else if(wlc_express == '3'){
				if(wl_info.band5g_2_support)
					document.getElementById('elliptic_ssid_5g_2').style.display = "none";
			}
		}	
		else if('<% nvram_get("wlc_band"); %>' == '0')
			ssid_status_2g =  decodeURIComponent('<% nvram_char_to_ascii("WLANConfig11b", "wl0.1_ssid"); %>');
		else if('<% nvram_get("wlc_band"); %>' == '2'){
			if(wl_info.band5g_2_support){
				ssid_status_5g_2 =  decodeURIComponent('<% nvram_char_to_ascii("WLANConfig11b", "wl2.1_ssid"); %>');
			}
		}
		else
			ssid_status_5g =  decodeURIComponent('<% nvram_char_to_ascii("WLANConfig11b", "wl1.1_ssid"); %>');

		document.getElementById('elliptic_ssid_2g').style.textDecoration="none";
		document.getElementById('elliptic_ssid_2g').style.cursor="auto";
		document.getElementById('elliptic_ssid_5g').style.textDecoration="none";
		document.getElementById('elliptic_ssid_5g').style.cursor="auto";
		if(wl_info.band5g_2_support){
			document.getElementById('elliptic_ssid_5g_2').style.textDecoration="none";
			document.getElementById('elliptic_ssid_5g_2').style.cursor="auto";	
		}
	}

	var topbanner_ssid_2g = handle_show_str(ssid_status_2g);

	if(topbanner_ssid_2g.length >18){
		document.getElementById('elliptic_ssid_2g').innerHTML = extend_display_ssid(ssid_status_2g)+"...";
	}
	else{
		document.getElementById('elliptic_ssid_2g').innerHTML = topbanner_ssid_2g;
	}

	if(!lyra_hide_support)
		document.getElementById('elliptic_ssid_2g').title = "2.4 GHz: \n"+ ssid_status_2g;

	var topbanner_ssid_5g = handle_show_str(ssid_status_5g);
	if(topbanner_ssid_5g.length >18){
		document.getElementById('elliptic_ssid_5g').innerHTML = extend_display_ssid(ssid_status_5g)+"...";
	}
	else{
		document.getElementById('elliptic_ssid_5g').innerHTML = topbanner_ssid_5g;
	}

	document.getElementById('elliptic_ssid_5g').title = "5 GHz: \n"+ ssid_status_5g;

	if(wl_info.band5g_2_support){
		var topbanner_ssid_5g_2 = handle_show_str(ssid_status_5g_2);
		if(topbanner_ssid_5g_2.length >18){
			document.getElementById('elliptic_ssid_5g_2').innerHTML = extend_display_ssid(ssid_status_5g_2)+"...";
		}
		else{
			document.getElementById('elliptic_ssid_5g_2').innerHTML = topbanner_ssid_5g_2;
		}
		document.getElementById('elliptic_ssid_5g_2').title = "5 GHz-2: \n"+ ssid_status_5g_2;
	}

	if(smart_connect_support){
		if('<% nvram_get("smart_connect_x"); %>' == '1'){
			document.getElementById('elliptic_ssid_2g').title = "Smart Connect: \n"+ ssid_status_2g;
			document.getElementById('elliptic_ssid_5g').style.display = "none";
			if(wl_info.band5g_2_support)
				document.getElementById('elliptic_ssid_5g_2').style.display = "none";
		}else if('<% nvram_get("smart_connect_x"); %>' == '2'){
			document.getElementById('elliptic_ssid_2g').title = "2.4 GHz: \n"+ ssid_status_2g;
			document.getElementById('elliptic_ssid_5g').title = "5 GHz Smart Connect: \n"+ ssid_status_5g;
			document.getElementById('elliptic_ssid_5g').style.display = "";
			if(wl_info.band5g_2_support)
				document.getElementById('elliptic_ssid_5g_2').style.display = "none";
		}else{
			document.getElementById('elliptic_ssid_2g').title = "2.4 GHz: \n"+ ssid_status_2g;
			document.getElementById('elliptic_ssid_5g').style.display = "";
			if(wl_info.band5g_2_support)
				document.getElementById('elliptic_ssid_5g_2').style.display = "";
		}
	}

	var swpjverno = '<% nvram_get("swpjverno"); %>';
	var firmver = '<% nvram_get("firmver"); %>';
	var buildno = '<% nvram_get("buildno"); %>';
	//var rcno = '<% nvram_get("rcno"); %>';
	var extendno = '<% nvram_get("extendno"); %>';
	var FWString = '';

	FWString = buildno;
	//if(rcno.length > 0)
	//	FWString += "rc"+rcno;
	if ((extendno != "") && (extendno != "0"))
		FWString += "_"+extendno;

	if(swpjverno == ''){
		if(swisscom_support)
			FWString += '_swisscom';
		showtext(document.getElementById("firmver"), FWString);
	}
	else{
		showtext(document.getElementById("firmver"), swpjverno + '_' + extendno);
	}

	// no_op_mode
	if (!dsl_support && based_modelid != "AC2900" && !lyra_hide_support){	//MODELDEP: AC2900(RT-AC86U)

		if(sw_mode == "1")  // Show operation mode in banner, Viz 2011.11
			document.getElementById("sw_mode_span").innerHTML = "<#wireless_router#>";
		else if(sw_mode == "2"){
			if(wlc_express == 1)
				document.getElementById("sw_mode_span").innerHTML = "Express Way (2.4GHz)";
			else if(wlc_express == 2)
				document.getElementById("sw_mode_span").innerHTML = "Express Way (5GHz)";
			else
				document.getElementById("sw_mode_span").innerHTML = "<#OP_RE_item#>";
		}
		else if(sw_mode == "3")
			document.getElementById("sw_mode_span").innerHTML = "<#OP_AP_item#>";
		else if(sw_mode == "4")
			document.getElementById("sw_mode_span").innerHTML = "<#OP_MB_item#>";
		else
			document.getElementById("sw_mode_span").innerHTML = "Unknown";	

		if(hwmodeSwitch_support){	
			document.getElementById("op_link").innerHTML = document.getElementById("sw_mode_span").innerHTML;	
			document.getElementById("op_link").style.cursor= "auto";			
		}
	}
}

function extend_display_ssid(ssid){		//"&amp;"5&; "&lt;"4< ; "&gt;"4> ; "&nbsp;"6space
		//alert(ssid.substring(0,35)+" : "+ssid.substring(0,35).lastIndexOf("&nbsp;")+" <&nbsp>");
		if(ssid.substring(0,35).lastIndexOf("&nbsp;") >=30 || ssid.substring(0,35).lastIndexOf("&nbsp;") < 25 ){
			//alert(ssid.substring(0,34)+" : "+ssid.substring(0,34).lastIndexOf("&amp;")+" <&amp>");
			if(ssid.substring(0,34).lastIndexOf("&amp;") >=30 || ssid.substring(0,34).lastIndexOf("&amp;") < 26 ){
				//alert(ssid.substring(0,33)+" : "+ssid.substring(0,33).lastIndexOf("&lt;")+" <&lt>");
				if(ssid.substring(0,33).lastIndexOf("&lt;") >=30 || ssid.substring(0,34).lastIndexOf("&lt;") < 27 ){
					//alert(ssid.substring(0,33)+" : "+ssid.substring(0,33).lastIndexOf("&gt;")+" <&gt>");
					if(ssid.substring(0,33).lastIndexOf("&gt;") >=30 || ssid.substring(0,34).lastIndexOf("&gt;") < 27 ){
						//alert(ssid.substring(0,30));
						return ssid.substring(0,30);
					}
					else{
						switch(ssid.substring(0,33).lastIndexOf("&gt;")){
							case 27:
								return ssid.substring(0,31);
								break;
							case 28:
								return ssid.substring(0,32);	
								break;
							case 29:
								return ssid.substring(0,33);		
								break;	
						}
					}
				}
				else{
					switch(ssid.substring(0,33).lastIndexOf("&lt;")){
						case 27:
							return ssid.substring(0,31);
							break;
						case 28:
							return ssid.substring(0,32);
							break;
						case 29:
							return ssid.substring(0,33);
							break;
					}			
				}
			}
			else{
				switch (ssid.substring(0,34).lastIndexOf("&amp;")){
					case 26:
						return ssid.substring(0,31);
						break;
					case 27:
						return ssid.substring(0,32);
						break;
					case 28:
						return ssid.substring(0,33);
						break;
					case 29:
						return ssid.substring(0,34);
						break;
				}
			}
		}
		else{
			switch (ssid.substring(0,35).lastIndexOf("&nbsp;")){
				case 25:			
					return ssid.substring(0,31);
					break;			
				case 26:
					return ssid.substring(0,32);
					break;
				case 27:
					return ssid.substring(0,33);
					break;
				case 28:
					return ssid.substring(0,34);
					break;
				case 29:
					return ssid.substring(0,35);
					break;					
			}
		}
}

function go_setting(page){
	if(tmo_support && isMobile()){
		location.href = "/MobileQIS_Login.asp";
	}
	else{
		location.href = page;
	}
}

function go_setting_parent(page){
		parent.location.href = page;
}

function show_time(){	
	JS_timeObj.setTime(systime_millsec); // Add millsec to it.	
	JS_timeObj3 = JS_timeObj.toString();	
	JS_timeObj3 = checkTime(JS_timeObj.getHours()) + ":" +
				  			checkTime(JS_timeObj.getMinutes()) + ":" +
				  			checkTime(JS_timeObj.getSeconds());
	document.getElementById('systemtime').innerHTML ="<a href='/Advanced_System_Content.asp'>" + JS_timeObj3 + "</a>"; 
	systime_millsec += 1000;		
	
	stime_ID = setTimeout("show_time();", 1000);
}

function checkTime(i)
{
if (i<10) 
  {i="0" + i}
  return i
}

function show_loading_obj(){
	var obj = document.getElementById("Loading");
	var code = "";
	
	code +='<table cellpadding="5" cellspacing="0" id="loadingBlock" class="loadingBlock" align="center">\n';
	code +='<tr>\n';
	code +='<td width="20%" height="80" align="center"><img src="/images/loading.gif"></td>\n';
	code +='<td><span id="proceeding_main_txt"><#Main_alert_proceeding_desc4#></span><span id="proceeding_txt" style="color:#FFFFCC;"></span></td>\n';
	code +='</tr>\n';
	code +='</table>\n';
	code +='<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->\n';
	
	obj.innerHTML = code;
}

var nav;

if(navigator.appName == 'Netscape')
	nav = true;
else{
	nav = false;
	document.onkeydown = MicrosoftEventHandler_KeyDown;
}

function MicrosoftEventHandler_KeyDown(){
	return true;
}

// display selected language in language bar, Viz modified at 2013/03/22
function show_selected_language(){
	document.getElementById('selected_lang').innerHTML = "<#selected_language#>";
}

function submit_language(obj){
	if(obj.id != document.getElementById("preferred_lang").value){
		showLoading();
		
		with(document.titleForm){
			action = "/start_apply.htm";
			
			if(location.pathname == "/")
				current_page.value = "<% abs_index_page(); %>";
			else
				current_page.value = location.pathname;
				
			preferred_lang.value = obj.id;
			//preferred_lang.value = document.getElementById("select_lang").value;
			flag.value = "set_language";
			/* For Notification Center ActMail service update language */
			action_script.value = "email_info"; 
			
			submit();
		}
	}
	else
		alert("No change LANGUAGE!");
}

function change_language(){
	if(document.getElementById("select_lang").value != document.getElementById("preferred_lang").value)
		document.getElementById("change_lang_btn").disabled = false;
	else
		document.getElementById("change_lang_btn").disabled = true;
}

function logout(){
	if(confirm('<#JS_logout#>')){
		setTimeout('location = "Logout.asp";', 1);
	}
}

function reboot(){
	if(confirm("<#Main_content_Login_Item7#>")){
		document.rebootForm.submit();
	}
}

function kb_to_gb(kilobytes){
	if(typeof(kilobytes) == "string" && kilobytes.length == 0)
		return 0;
	
	return (kilobytes*1024)/(1024*1024*1024);
}

function simpleNum(num){
	if(typeof(num) == "string" && num.length == 0)
		return 0;
	
	return parseInt(kb_to_gb(num)*1000)/1000;
}

function simpleNum2(num){
	if(typeof(num) == "string" && num.length == 0)
		return 0;
	
	return parseInt(num*1000)/1000;
}

function simpleNum3(num){
	if(typeof(num) == "string" && num.length == 0)
		return 0;
	
	return parseInt(num)/1024;
}

function getElementsByName_iefix(tag, name){
	var tagObjs = document.getElementsByTagName(tag);
	var objsName;
	var targetObjs = new Array();
	var targetObjs_length;
	
	if(!(typeof(name) == "string" && name.length > 0))
		return [];
	
	for(var i = 0, targetObjs_length = 0; i < tagObjs.length; ++i){
		objsName = tagObjs[i].getAttribute("name");
		
		if(objsName && objsName.indexOf(name) == 0){
			targetObjs[targetObjs_length] = tagObjs[i];
			++targetObjs_length;
		}
	}
	
	return targetObjs;
}

function getElementsByClassName_iefix(tag, name){
	var tagObjs = document.getElementsByTagName(tag);
	var objsName;
	var targetObjs = new Array();
	var targetObjs_length;
	
	if(!(typeof(name) == "string" && name.length > 0))
		return [];
	
	for(var i = 0, targetObjs_length = 0; i < tagObjs.length; ++i){
		if(navigator.appName == 'Netscape')
			objsName = tagObjs[i].getAttribute("class");
		else
			objsName = tagObjs[i].getAttribute("className");
		
		if(objsName == name){
			targetObjs[targetObjs_length] = tagObjs[i];
			++targetObjs_length;
		}
	}
	
	return targetObjs;
}

function showtext(obj, str){
	if(obj)
		obj.innerHTML = str;//*/
}

function showtext2(obj, str, visible){
	if(obj){
		obj.innerHTML = str;
		obj.style.display = (visible) ? "" : "none";
	}
}

function showhtmlspace(ori_str){
	var str = "", head, tail_num;
	
	head = ori_str;
	while((tail_num = head.indexOf(" ")) >= 0){
		str += head.substring(0, tail_num);
		str += "&nbsp;";
		
		head = head.substr(tail_num+1, head.length-(tail_num+1));
	}
	str += head;
	
	return str;
}

function showhtmland(ori_str){
	var str = "", head, tail_num;
	
	head = ori_str;
	while((tail_num = head.indexOf("&")) >= 0){
		str += head.substring(0, tail_num);
		str += "&amp;";
		
		head = head.substr(tail_num+1, head.length-(tail_num+1));
	}
	str += head;
	
	return str;
}

// A dummy function which just returns its argument. This was needed for localization purpose
function translate(str){
	return str;
}

function trim(val){
	val = val+'';
	for (var startIndex=0;startIndex<val.length && val.substring(startIndex,startIndex+1) == ' ';startIndex++);
	for (var endIndex=val.length-1; endIndex>startIndex && val.substring(endIndex,endIndex+1) == ' ';endIndex--);
	return val.substring(startIndex,endIndex+1);
}

function IEKey(){
	return event.keyCode;
}

function NSKey(){
	return event.which;
}

function checkDuplicateName(newname, targetArray){
	var existing_string = targetArray.join(',');
	existing_string = ","+existing_string+",";
	var newstr = ","+trim(newname)+",";
	
	var re = new RegExp(newstr, "gi");
	var matchArray = existing_string.match(re);
	
	if(matchArray != null)
		return true;
	else
		return false;
}

function alert_error_msg(error_msg){
	alert(error_msg);
	refreshpage();
}

function refreshpage(seconds){
	if(typeof(seconds) == "number")
		setTimeout("refreshpage()", seconds*1000);
	else
		location.href = location.href;
}

function hideLinkTag(){
	if(document.all){
		var tagObjs = document.all.tags("a");
		
		for(var i = 0; i < tagObjs.length; ++i)
			tagObjs(i).outerHTML = tagObjs(i).outerHTML.replace(">"," hidefocus=true>");
	}
}

function buttonOver(o){	//Lockchou 1206 modified
	o.style.color = "#FFFFFF";
	o.style.background = "url(/images/bgaibutton.gif) #ACCCE1";
	o.style.cursor = "hand";
}

function buttonOut(o){	//Lockchou 1206 modified
	o.style.color = "#000000";
	o.style.background = "url(/images/bgaibutton0.gif) #ACCCE1";
}

function flash_button(){
	if(navigator.appName.indexOf("Microsoft") < 0)
		return;
	
	var btnObj = getElementsByClassName_iefix("input", "button");
	
	for(var i = 0; i < btnObj.length; ++i){
		btnObj[i].onmouseover = function(){
				buttonOver(this);
			};
		
		btnObj[i].onmouseout = function(){
				buttonOut(this);
			};
	}
}

function no_flash_button(){
	if(navigator.appName.indexOf("Microsoft") < 0)
		return;
	
	var btnObj = getElementsByClassName_iefix("input", "button");
	
	for(var i = 0; i < btnObj.length; ++i){
		btnObj[i].onmouseover = "";
		
		btnObj[i].onmouseout = "";
	}
}

function gotoprev(formObj){
	var prev_page = formObj.prev_page.value;
	
	if(prev_page == "/")
		prev_page = "<% abs_index_page(); %>";
	
	if(prev_page.indexOf('QIS') < 0){
		formObj.action = prev_page;
		formObj.target = "_parent";
		formObj.submit();
	}
	else{
		formObj.action = prev_page;
		formObj.target = "";
		formObj.submit();
	}
}

function add_option(selectObj, str, value, selected){
	var tail = selectObj.options.length;
	
	if(typeof(str) != "undefined")
		selectObj.options[tail] = new Option(str);
	else
		selectObj.options[tail] = new Option();
	
	if(typeof(value) != "undefined")
		selectObj.options[tail].value = value;
	else
		selectObj.options[tail].value = "";
	
	if(selected == 1)
		selectObj.options[tail].selected = selected;
}

function free_options(selectObj){
	if(selectObj == null)
		return;
	
	for(var i = selectObj.options.length-1; i >= 0; --i){
  		selectObj.options[i].value = null;
		selectObj.options[i] = null;
	}
}

function getRadioValue(obj) {
	for (var i=0; i<obj.length; i++) {
		if (obj[i].checked)
			return obj[i].value;
	}
	return 0;
}

function setRadioValue(obj,val) {
	for (var i=0; i<obj.length; i++) {
		if (obj[i].value==val)
			obj[i].checked = true;
	}
}

function blocking(obj_id, show){
	var state = show?'block':'none';
	
	if(document.getElementById)
		document.getElementById(obj_id).style.display = state;
	else if(document.layers)
		document.layers[obj_id].display = state;
	else if(document.all)
		document.all[obj_id].style.display = state;
}

function inputCtrl(obj, flag){
	if(flag == 0){
		obj.disabled = true;
		if(obj.type != "select-one" && based_modelid != "GT-AC5300" && based_modelid != "GT-AC9600")
			obj.style.backgroundColor = "#CCCCCC";
		if(obj.type == "radio" || obj.type == "checkbox")
			obj.style.backgroundColor = "#475A5F";
		if(obj.type == "text" || obj.type == "password")
			if(based_modelid != "GT-AC5300" && based_modelid != "GT-AC9600")
				obj.style.backgroundImage = "url(/images/New_ui/inputbg_disable.png)";
	}
	else{
		obj.disabled = false;		
		if((obj.type == "radio" || obj.type == "checkbox") && based_modelid != "GT-AC5300" && based_modelid != "GT-AC9600") 
			obj.style.backgroundColor = "#475A5F";
		if(obj.type == "text" || obj.type == "password"){
			if(based_modelid != "GT-AC5300" && based_modelid != "GT-AC9600")
				obj.style.backgroundImage = "url(/images/New_ui/inputbg.png)";
		}
	}

	if(current_url.indexOf("Advanced_Wireless_Content") == 0
	|| current_url.indexOf("Advanced_WAN_Content") == 0
	|| current_url.indexOf("Guest_network") == 0
	|| current_url.indexOf("Advanced_WPasspoint_Content") == 0
	|| current_url.indexOf("Advanced_PerformanceTuning_Content") == 0
	|| current_url.indexOf("Advanced_Modem_Content") == 0
	|| current_url.indexOf("QIS_modem") == 0
	|| current_url.indexOf("Advanced_IPv6_Content") == 0
	|| current_url.indexOf("Advanced_WAdvanced_Content") == 0
	|| current_url.indexOf("Advanced_IPTV_Content") == 0
	|| current_url.indexOf("Advanced_WANPort_Content.asp") == 0
	|| current_url.indexOf("Advanced_ASUSDDNS_Content.asp") == 0
	|| current_url.indexOf("Advanced_DSL_Content.asp") == 0
	|| current_url.indexOf("Advanced_VDSL_Content.asp") == 0
	|| current_url.indexOf("Advanced_SwitchCtrl_Content.asp") == 0
	|| current_url.indexOf("router.asp") == 0
	|| current_url.indexOf("Advanced_MobileBroadband_Content") == 0
	|| current_url.indexOf("Advanced_Feedback") == 0
	|| current_url.indexOf("Advanced_MultiWAN_Content") == 0
	|| current_url.indexOf("Advanced_IPv61_Content") == 0
	|| current_url.indexOf("Advanced_FirmwareUpgrade_Content.asp") == 0
	){
		if(obj.type == "checkbox")
			return true;
		if(flag == 0)
			obj.parentNode.parentNode.style.display = "none";
		else
			obj.parentNode.parentNode.style.display = "";
		return true;
	}
}

function inputHideCtrl(obj, flag){
	if(obj.type == "checkbox")
		return true;
	if(flag == 0)
		obj.parentNode.parentNode.style.display = "none";
	else
		obj.parentNode.parentNode.style.display = "";
	return true;
}

function hadPlugged(deviceType){
	if(allUsbStatusArray.join().search(deviceType) != -1)
		return true;

	return false;
}

//Update current system status
var AUTOLOGOUT_MAX_MINUTE = parseInt('<% nvram_get("http_autologout"); %>') * 20;
function updateStatus(){
	if(stopFlag == 1) return false;
	if(AUTOLOGOUT_MAX_MINUTE == 1) location = "Logout.asp"; // 0:disable auto logout, 1:trigger auto logout. 

	require(['/require/modules/makeRequest.js'], function(makeRequest){
		if(AUTOLOGOUT_MAX_MINUTE != 0) AUTOLOGOUT_MAX_MINUTE--;
		makeRequest.start('/ajax_status.xml', refreshStatus, function(){stopFlag = 1;});
	});
}

var link_status;
var link_auxstatus;
var link_sbstatus;
var ddns_return_code = '<% nvram_get("ddns_return_code_chk");%>';
var ddns_updated = '<% nvram_get("ddns_updated");%>';
var vpnc_state_t = '';
var vpnc_sbstate_t = '';
var vpn_clientX_errno = '';
var vpnc_proto = '<% nvram_get("vpnc_proto");%>';
var vpnd_state;	
var vpnc_state_t1 = '';
var vpnc_state_t2 = '';
var vpnc_state_t3 = '';
var vpnc_state_t4 = '';
var vpnc_state_t5 = '';
var vpnc_errno_t1 = '';
var vpnc_errno_t2 = '';
var vpnc_errno_t3 = '';
var vpnc_errno_t4 = '';
var vpnc_errno_t5 = '';
var qtn_state_t = '';

//for mobile broadband
var sim_signal = '<% nvram_get("usb_modem_act_signal"); %>';
var sim_operation = '<% nvram_get("usb_modem_act_operation"); %>';
var sim_state = '<% nvram_get("usb_modem_act_sim"); %>';
var sim_isp = '<% nvram_get("modem_isp"); %>';
var modem_act_provider = '<% nvram_get("usb_modem_act_provider"); %>';
var roaming = '<% nvram_get("modem_roaming"); %>';
var roaming_imsi = '<% nvram_get("modem_roaming_imsi"); %>';
var sim_imsi = '<% nvram_get("usb_modem_act_imsi"); %>';
var g3err_pin = '<% nvram_get("g3err_pin"); %>';
var pin_remaining_count = '<% nvram_get("usb_modem_act_auth_pin"); %>';
var usbState;
var usb_state = -1;
var usb_sbstate = -1;
var usb_auxstate = -1;	
var first_link_status = '';
var first_link_sbstatus = '';
var first_link_auxstatus = '';
var secondary_link_status = '';
var secondary_link_sbstatus = '';
var secondary_link_auxstatus = '';
var modem_bytes_data_limit = parseFloat('<% nvram_get("modem_bytes_data_limit"); %>');
var rx_bytes = parseFloat('<% nvram_get("modem_bytes_rx"); %>');
var tx_bytes = parseFloat('<% nvram_get("modem_bytes_tx"); %>');
var traffic_warning_cookie = '';
var traffic_warning_flag = '';
var keystr = 'traffic_warning_' + modem_bytes_data_limit;
var date = new Date();
var date_year = date.getFullYear();
var date_month = date.getMonth();
var modem_enable = '';
var modem_sim_order = '';
var wanConnectStatus = true;
var wlc0_ssid = '<% nvram_get("wlc0_ssid"); %>';
var wlc1_ssid = '<% nvram_get("wlc1_ssid"); %>';
var concurrent_pap = false;
var pap_flag = 0;
var pap_click_flag = 0;
if((sw_mode == "2" && wlc_express == "0")|| sw_mode == "4"){
	if(wlc0_ssid != "" && wlc1_ssid != ""){
		concurrent_pap = true;
		pap_flag = 1;
	}
}

function refreshStatus(xhr){
	if(xhr.responseText.search("Main_Login.asp") !== -1) top.location.href = "<% abs_index_page(); %>";

	setTimeout(function(){updateStatus();}, 3000);	/* restart ajax */
	var devicemapXML = xhr.responseXML.getElementsByTagName("devicemap");
	var SysStatus = devicemapXML[0].getElementsByTagName("sys");
	uptimeStr_update = SysStatus[0].firstChild.nodeValue.replace("uptimeStr=", "");
	boottime_update = parseInt(uptimeStr_update.substring(32,42));
	var wanStatus = devicemapXML[0].getElementsByTagName("wan");
	link_status = wanStatus[0].firstChild.nodeValue;
	link_sbstatus = wanStatus[1].firstChild.nodeValue;
	link_auxstatus = wanStatus[2].firstChild.nodeValue;

	monoClient = wanStatus[3].firstChild.nodeValue;	
	_wlc_state = wanStatus[4].firstChild.nodeValue;
	_wlc_sbstate = wanStatus[5].firstChild.nodeValue;	
	_wlc_auth = wanStatus[6].firstChild.nodeValue;	
	wifi_hw_switch = wanStatus[7].firstChild.nodeValue;
	ddns_return_code = wanStatus[8].firstChild.nodeValue.replace("ddnsRet=", "");
	ddns_updated = wanStatus[9].firstChild.nodeValue.replace("ddnsUpdate=", "");
	wan_line_state = wanStatus[10].firstChild.nodeValue.replace("wan_line_state=", "");	
	wlan0_radio_flag = wanStatus[11].firstChild.nodeValue.replace("wlan0_radio_flag=", "");
	wlan1_radio_flag = wanStatus[12].firstChild.nodeValue.replace("wlan1_radio_flag=", "");
	wlan2_radio_flag = wanStatus[13].firstChild.nodeValue.replace("wlan2_radio_flag=", "");	
	data_rate_info_2g = wanStatus[14].firstChild.nodeValue.replace("data_rate_info_2g=", "");
	data_rate_info_5g = wanStatus[15].firstChild.nodeValue.replace("data_rate_info_5g=", "");
	data_rate_info_5g_2 = wanStatus[16].firstChild.nodeValue.replace("data_rate_info_5g_2=", "");
	wan_diag_state = wanStatus[17].firstChild.nodeValue.replace("wan_diag_state=", "");
	active_wan_unit = wanStatus[18].firstChild.nodeValue.replace("active_wan_unit=", "");
	wan0_enable = wanStatus[19].firstChild.nodeValue.replace("wan0_enable=", "");
	wan1_enable = wanStatus[20].firstChild.nodeValue.replace("wan1_enable=", "");
	wan0_realip_state = wanStatus[21].firstChild.nodeValue.replace("wan0_realip_state=", "");
	wan1_realip_state = wanStatus[22].firstChild.nodeValue.replace("wan1_realip_state=", "");
	wan0_ipaddr = wanStatus[23].firstChild.nodeValue.replace("wan0_ipaddr=", "");
	wan1_ipaddr = wanStatus[24].firstChild.nodeValue.replace("wan1_ipaddr=", "");
	wan0_realip_ip = wanStatus[25].firstChild.nodeValue.replace("wan0_realip_ip=", "");
	wan1_realip_ip = wanStatus[26].firstChild.nodeValue.replace("wan1_realip_ip=", "");
	if(concurrent_pap){
		_wlc0_state = wanStatus[27].firstChild.nodeValue;
		_wlc1_state = wanStatus[28].firstChild.nodeValue;
	}
	rssi_2g = wanStatus[29].firstChild.nodeValue.replace("rssi_2g=", "");
	rssi_5g = wanStatus[30].firstChild.nodeValue.replace("rssi_5g=", "");
	rssi_5g_2 = wanStatus[31].firstChild.nodeValue.replace("rssi_5g_2=", "");

	var vpnStatus = devicemapXML[0].getElementsByTagName("vpn");
	vpnc_proto = vpnStatus[0].firstChild.nodeValue.replace("vpnc_proto=", "");
	
	var first_wanStatus = devicemapXML[0].getElementsByTagName("first_wan");
	first_link_status = first_wanStatus[0].firstChild.nodeValue;
	first_link_sbstatus = first_wanStatus[1].firstChild.nodeValue;
	first_link_auxstatus = first_wanStatus[2].firstChild.nodeValue;
	var secondary_wanStatus = devicemapXML[0].getElementsByTagName("second_wan");
	secondary_link_status = secondary_wanStatus[0].firstChild.nodeValue;
	secondary_link_sbstatus = secondary_wanStatus[1].firstChild.nodeValue;
	secondary_link_auxstatus = secondary_wanStatus[2].firstChild.nodeValue;

	var qtn_state = devicemapXML[0].getElementsByTagName("qtn");
	qtn_state_t = qtn_state[0].firstChild.nodeValue.replace("qtn_state=", "");

	var usbStatus = devicemapXML[0].getElementsByTagName("usb");
	allUsbStatus = usbStatus[0].firstChild.nodeValue.toString();
	modem_enable = usbStatus[1].firstChild.nodeValue.replace("modem_enable=", "");

	var simState = devicemapXML[0].getElementsByTagName("sim");
	sim_state = simState[0].firstChild.nodeValue.replace("sim_state=", "");
	sim_signal = simState[1].firstChild.nodeValue.replace("sim_signal=", "");	
	sim_operation = simState[2].firstChild.nodeValue.replace("sim_operation=", "");	
	sim_isp = simState[3].firstChild.nodeValue.replace("sim_isp=", "");
	roaming = simState[4].firstChild.nodeValue.replace("roaming=", "");
	roaming_imsi = simState[5].firstChild.nodeValue.replace("roaming_imsi=", "");
	sim_imsi = simState[6].firstChild.nodeValue.replace("sim_imsi=", "");			
	g3err_pin = simState[7].firstChild.nodeValue.replace("g3err_pin=", "");		
	pin_remaining_count = simState[8].firstChild.nodeValue.replace("pin_remaining_count=", "");		
	modem_act_provider = simState[9].firstChild.nodeValue.replace("modem_act_provider=", "");
	rx_bytes = parseFloat(simState[10].firstChild.nodeValue.replace("rx_bytes=", ""));
	tx_bytes = parseFloat(simState[11].firstChild.nodeValue.replace("tx_bytes=", ""));
	modem_sim_order = parseFloat(simState[12].firstChild.nodeValue.replace("modem_sim_order=", ""));	

	var dhcpState = devicemapXML[0].getElementsByTagName("dhcp");
	dnsqmode = dhcpState[0].firstChild.nodeValue.replace("dnsqmode=", "");

	if(vpnc_support){
		vpnc_state_t1 = vpnStatus[3].firstChild.nodeValue.replace("vpn_client1_state=", "");
		vpnc_errno_t1 = vpnStatus[9].firstChild.nodeValue.replace("vpn_client1_errno=", "");
		vpnc_state_t2 = vpnStatus[4].firstChild.nodeValue.replace("vpn_client2_state=", "");
		vpnc_errno_t2 = vpnStatus[10].firstChild.nodeValue.replace("vpn_client2_errno=", "");
		vpnc_state_t3 = vpnStatus[5].firstChild.nodeValue.replace("vpn_client3_state=", "");
		vpnc_errno_t3 = vpnStatus[11].firstChild.nodeValue.replace("vpn_client3_errno=", "");
		vpnc_state_t4 = vpnStatus[6].firstChild.nodeValue.replace("vpn_client4_state=", "");
		vpnc_errno_t4 = vpnStatus[12].firstChild.nodeValue.replace("vpn_client4_errno=", "");
		vpnc_state_t5 = vpnStatus[7].firstChild.nodeValue.replace("vpn_client5_state=", "");
		vpnc_errno_t5 = vpnStatus[13].firstChild.nodeValue.replace("vpn_client5_errno=", "");
		vpnc_state_t = vpnStatus[1].firstChild.nodeValue.replace("vpnc_state_t=", "");//vpnc (pptp/l2tp)
	}
	
	vpnc_sbstate_t = vpnStatus[2].firstChild.nodeValue.replace("vpnc_sbstate_t=", "");
	if('<% nvram_get("vpn_server_unit"); %>' == 1)
		vpnd_state = vpnStatus[14].firstChild.nodeValue.replace("vpn_server1_state=", "");
	else	//unit 2
		vpnd_state = vpnStatus[15].firstChild.nodeValue.replace("vpn_server2_state=", "");

	if(dblog_support) {
		var diagStatus = devicemapXML[0].getElementsByTagName("diag");
		diag_dblog_enable = diagStatus[0].firstChild.nodeValue.replace("diag_dblog_enable=", "");
		diag_dblog_remaining = diagStatus[1].firstChild.nodeValue.replace("diag_dblog_remaining=", "");
	}

	if(realip_support){
		if(active_wan_unit == "0"){
			realip_state = wan0_realip_state;  //0: init/no act  1: can't get external IP  2: get external IP
			if(realip_state == "2"){
				realip_ip = wan0_realip_ip;
				external_ip = (realip_ip == wan0_ipaddr)? 1:0;
			}
			else{
				external_ip = -1;
			}
		}
		else if(active_wan_unit == "1"){
			realip_state = wan1_realip_state;  //0: init/no act  1: can't get external IP  2: get external IP
			if(realip_state == "2"){
				realip_ip = wan1_realip_ip;
				external_ip = (realip_ip == wan1_ipaddr)? 1:0;
			}
			else{
				external_ip = -1;
			}
		}
	}

	if(location.pathname == "/"+ QISWIZARD)
		return false;
	else if(location.pathname == "/Advanced_VPNClient_Content.asp" && !vpn_fusion_support)
		show_vpnc_rulelist();
	else if(location.pathname == "/Advanced_Feedback.asp") {
		updateUSBStatus();
		if(dblog_support)
			diag_control_usb_status();
	}
	
	//Adaptive QoS mode	
	if(bwdpi_support && qos_enable_flag && qos_type_flag == "1"){
		if(bwdpi_app_rulelist == "9,20<8<4<0,5,6,15,17<13,24<1,3,14<7,10,11,21,23<<game")
			document.getElementById("bwdpi_status").className = "bwdpistatus_game";			
		else if(bwdpi_app_rulelist == "9,20<4<0,5,6,15,17<8<13,24<1,3,14<7,10,11,21,23<<media")
			document.getElementById("bwdpi_status").className = "bwdpistatus_media";
		else if(bwdpi_app_rulelist == "9,20<13,24<4<0,5,6,15,17<8<1,3,14<7,10,11,21,23<<web")
			document.getElementById("bwdpi_status").className = "bwdpistatus_web";
		else
			document.getElementById("bwdpi_status").className = "bwdpistatus_customize";			
		
		document.getElementById("bwdpi_status").onclick = function(){openHint(24,9);}
		document.getElementById("bwdpi_status").onmouseover = function(){overHint("A");}
		document.getElementById("bwdpi_status").onmouseout = function(){nd();}
		
		if(based_modelid == "AC2900" || based_modelid == "RT-AC85U" || based_modelid == "RT-AC68A" || based_modelid == "RT-AC65U"){	//MODELDEP : Spec special fine tune
			document.getElementById("bwdpi_status").style.display = "none";
		}	
	}

	//reboot
	if(based_modelid == "GT-AC5300" || based_modelid == "GT-AC9600"){
		document.getElementById("reboot_status").onmouseover = function(){overHint(26);}
		document.getElementById("reboot_status").onmouseout = function(){nd();}
	}

	// internet
	if(sw_mode == 1){
		//Viz add 2013.04 for dsl sync status
		if(dsl_support){
				if(wan_diag_state == "1" && allUsbStatus.search("storage") >= 0){
						document.getElementById("adsl_line_status").className = "linestatusdiag";
						document.getElementById("adsl_line_status").onclick = function(){openHint(24,8);}
				}else if(wan_line_state == "up"){
						document.getElementById("adsl_line_status").className = "linestatusup";
						document.getElementById("adsl_line_status").onclick = function(){openHint(24,6);}
				}else if(wan_line_state == "wait for init"){
						document.getElementById("adsl_line_status").className = "linestatuselse";
				}else if(wan_line_state == "init" || wan_line_state == "initializing"){
						document.getElementById("adsl_line_status").className = "linestatuselse";
				}else{
						document.getElementById("adsl_line_status").className = "linestatusdown";
				}		
				document.getElementById("adsl_line_status").onmouseover = function(){overHint(9);}
				document.getElementById("adsl_line_status").onmouseout = function(){nd();}
		}

		if(dblog_support) {
			if(diag_dblog_enable == "1") {
				document.getElementById("dblog_diag_status_td").style.width = "30px";
				document.getElementById("dblog_diag_status").style.display = "block";
				document.getElementById("dblog_diag_status").className = "dblogdiag";
				document.getElementById("dblog_diag_status").onclick = function(){openHint(24,10);}
				document.getElementById("dblog_diag_status").onmouseover = function(){overHint(27);}
				document.getElementById("dblog_diag_status").onmouseout = function(){nd();}
			}
			else {
				document.getElementById("dblog_diag_status_td").style.width = "0px";
				document.getElementById("dblog_diag_status").style.display = "none";
			}
		}

		(function(){
			var wans_mode = '<%nvram_get("wans_mode");%>';
			var NM_connect_obj = function(_link_status, _link_sbstatus, _link_auxstatus, unit){
				if(dualwan_enabled && active_wan_unit != unit && (wans_mode == "fo" || wans_mode == "fb")){
					this.hint = "Cold-<#Status_Standby#>";
					this.className = "_standby";
				}
				else{
					this.hint = "<#Disconnected#>";
					this.className = "_disconnected";
				}

				if(!dualWAN_support)
					this.link = "/" + QISWIZARD + "?flag=detect";
				else{
					if(dualwan_enabled)
						this.link = "gotoWAN";
					else if(wans_dualwan_array[unit] == "usb"){
						if(gobi_support)
							this.link="/Advanced_MobileBroadband_Content.asp";
						else
							this.link="/Advanced_Modem_Content.asp";
					}
					else
						this.link="/Advanced_WAN_Content.asp";
				}

				this.hasInternet = false;

				if(_link_auxstatus == "1"){
					this.hint = "<#web_redirect_reason1#>";
					this.link = "/error_page.htm?flag=1";
					this.className = "_disconnected";
				}
				else if(_link_status == "2"){
					this.link = "";

					if(dualwan_enabled && active_wan_unit != unit && (wans_mode == "fo" || wans_mode == "fb")){	
						this.hint = "<#Standby_str#>";
						this.className = "_standby";
					}
					else{
						this.hint = "<#Connected#>";
						this.className = "_connected";
						this.hasInternet = true;
					}
				}
				else if(_link_status == "4"){
					if(_link_sbstatus == "1"){
						this.hint = "<#QKSet_Internet_Setup_fail_reason3#>";
						if(wans_dualwan_array[active_wan_unit] == "dsl" && dsltmp_transmode_orig == "ptm")
							this.link = "/Advanced_VDSL_Content.asp?af=dslx_pppoe_username";
						else if(wans_dualwan_array[active_wan_unit] == "dsl")
							this.link = "/Advanced_DSL_Content.asp?af=dslx_pppoe_username";
						else
							this.link = "/Advanced_WAN_Content.asp?af=wan_pppoe_username";
						this.className = "_error";
					}
					else if(_link_sbstatus == "2"){
						this.hint = "<#QKSet_Internet_Setup_fail_reason2#>";
						if(wans_dualwan_array[active_wan_unit] == "dsl" && dsltmp_transmode_orig == "ptm")
							this.link = "/Advanced_VDSL_Content.asp?af=dslx_pppoe_username";
						else if(wans_dualwan_array[active_wan_unit] == "dsl")
							this.link = "/Advanced_DSL_Content.asp?af=dslx_pppoe_username";
						else
							this.link = "/Advanced_WAN_Content.asp?af=wan_pppoe_username";
						this.className = "_error";
					}
					else if(_link_sbstatus == "3"){
						this.hint = "<#web_redirect_reason3_1#>";
						this.link = "/" + QISWIZARD + "?flag=detect";
						this.className = "_error";
					}
					else if(_link_sbstatus == "4"){
						this.hint = "<#web_redirect_reason6#>";
						this.link = "/error_page.htm?flag=6";
						this.className = "_error";
					}
				}
				else if(_link_status == "5"){
					this.hint = "<#web_redirect_reason5_1#>";
					if(wans_dualwan_array[active_wan_unit] == "dsl")
						this.link = "";
					else
						this.link = "/Advanced_WAN_Content.asp";
					this.className = "_error";
				}

				return this;
			}

			if(dualwan_enabled){
				var NM_connect_status = {
					primary: new NM_connect_obj(first_link_status, first_link_sbstatus, first_link_auxstatus, 0),
					secondary: new NM_connect_obj(secondary_link_status, secondary_link_sbstatus, secondary_link_auxstatus, 1)
				};

				if(document.getElementById("primary_status")){
					if(NM_connect_status.primary.link == "gotoWAN")
						document.getElementById('primary_status').innerHTML = '<a style="color:#FFF;text-decoration:underline; cursor:pointer;" onclick="goToWAN(0);">' + NM_connect_status.primary.hint + '</a>';
					else if(NM_connect_status.primary.link != "")
						document.getElementById('primary_status').innerHTML = '<a style="color:#FFF;text-decoration:underline;" href="'+ NM_connect_status.primary.link +'">' + NM_connect_status.primary.hint + '</a>';
					else
						document.getElementById('primary_status').innerHTML = NM_connect_status.primary.hint;
					document.getElementById('primary_line').className = "primary_wan" + NM_connect_status.primary.className;
				}
				if(document.getElementById("secondary_status")){
					if(NM_connect_status.secondary.link == "gotoWAN")
						document.getElementById('secondary_status').innerHTML = '<a style="color:#FFF;text-decoration:underline; cursor:pointer;" onclick="goToWAN(1);">' + NM_connect_status.secondary.hint + '</a>';
					else if(NM_connect_status.secondary.link != "")
						document.getElementById('secondary_status').innerHTML = '<a style="color:#FFF;text-decoration:underline;" href="'+ NM_connect_status.secondary.link +'">' + NM_connect_status.secondary.hint + '</a>';
					else
						document.getElementById('secondary_status').innerHTML = NM_connect_status.secondary.hint;
					document.getElementById('secondary_line').className = "secondary_wan" + NM_connect_status.secondary.className;
				}
				document.getElementById("connect_status").className = (NM_connect_status.primary.hasInternet || NM_connect_status.secondary.hasInternet) ? "connectstatuson" : "connectstatusoff";
				wanConnectStatus = NM_connect_status.primary.hasInternet || NM_connect_status.secondary.hasInternet;
			}
			else{
				var NM_connect_status = new NM_connect_obj(link_status, link_sbstatus, link_auxstatus, 0);
				if(document.getElementById("NM_connect_status")){
					if(NM_connect_status.link != "")
						document.getElementById("NM_connect_status").innerHTML = '<a style="color:#FFF;text-decoration:underline;" href="'+ NM_connect_status.link +'">' + NM_connect_status.hint + '</a>';
					else
						document.getElementById('NM_connect_status').innerHTML = NM_connect_status.hint;
					document.getElementById('single_wan').className = "single_wan" + NM_connect_status.className;
					document.getElementById("wanIP_div").style.display = NM_connect_status.hasInternet ? "" : "none";
					if(NM_connect_status.hasInternet){
						if(active_wan_unit == "0")
							document.getElementById("index_status").innerHTML = '<span style="word-break:break-all;">' + wan0_ipaddr + '</span>';
						else if(active_wan_unit == "1")
							document.getElementById("index_status").innerHTML = '<span style="word-break:break-all;">' + wan1_ipaddr + '</span>';
					}
					document.getElementById("ddnsHostName_div").style.display = NM_connect_status.hasInternet ? "" : "none";
				}
				document.getElementById("connect_status").className = NM_connect_status.hasInternet ? "connectstatuson" : "connectstatusoff";
				wanConnectStatus = NM_connect_status.hasInternet;

				if((based_modelid == "GT-AC5300" || based_modelid == "GT-AC9600") && location.pathname == "/GameDashboard.asp"){
					$("#wan_state").html(NM_connect_status.hint);
					if(NM_connect_status.hasInternet){
						$("#wan_state_icon").addClass("wan_icon_connect");
					}
					else{
						$("#wan_state_icon").addClass("wan_icon_disconnect");
					}
				}
			}
		})()

		document.getElementById("connect_status").onclick = wanConnectStatus ? function(){openHint(24,3);} : function(){return false};
		document.getElementById("connect_status").onmouseover = function(){overHint(3);}
		document.getElementById("connect_status").onmouseout = function(){nd();}
	}
	else if(sw_mode == 2 || sw_mode == 4){
		if(sw_mode == 4 || (sw_mode == 2 && new_repeater)){
			if(_wlc_auth.search("wlc_state=1") != -1 && _wlc_auth.search("wlc_state_auth=0") != -1)
				_wlc_state = "wlc_state=2";
			else
				_wlc_state = "wlc_state=0";
		}

		if(_wlc_state == "wlc_state=2"){
			document.getElementById("connect_status").className = "connectstatuson";
			document.getElementById("connect_status").onclick = function(){openHint(24,3);}
			if(location.pathname == "/" || location.pathname == "<% abs_index_page(); %>"){
				document.getElementById("NM_connect_status").innerHTML = "<#Connected#>";
				document.getElementById('single_wan').className = "single_wan_connected";
			}	
			wanConnectStatus = true;
		}
		else{
			document.getElementById("connect_status").className = "connectstatusoff";
			if(location.pathname == "/" || location.pathname == "<% abs_index_page(); %>"){
				document.getElementById("NM_connect_status").innerHTML = "<#Disconnected#>";		 
				document.getElementById('single_wan').className = "single_wan_disconnected";				
			}	
			wanConnectStatus = false;
		}
		document.getElementById("connect_status").onmouseover = function(){overHint(3);}
		document.getElementById("connect_status").onmouseout = function(){nd();}
		
		if(location.pathname == "/" || location.pathname == "<% abs_index_page(); %>" ||
		   location.pathname == "<% abs_networkmap_page(); %>"){
			if(wlc_band == 0) {	// show repeater and media bridge date rate
				var speed_info = data_rate_info_2g;
				var rssi_info = rssi_2g;
			} else if (wlc_band == 1) {
				var speed_info = data_rate_info_5g;
				var rssi_info = rssi_5g;
			} else if (wlc_band == 2) {
				var speed_info = data_rate_info_5g_2;
				var rssi_info = rssi_5g_2;
			}
			else{
				var speed_info = "";
				var rssi_info = "";				
			}

			if(concurrent_pap){
				document.getElementById('speed_info_primary').innerHTML = "Link Rate: " + data_rate_info_2g;
				document.getElementById('speed_info_secondary').innerHTML = "Link Rate: " + data_rate_info_5g;
				if(_wlc0_state == "wlc0_state=2"){
					document.getElementById('primary_line').className = "primary_wan_connected";				
				}
				else{
					document.getElementById('primary_line').className = "primary_wan_disconnected";					
				}
				
				if(_wlc1_state == "wlc1_state=2"){
					document.getElementById('secondary_line').className = "secondary_wan_connected";
				}
				else{
					document.getElementById('secondary_line').className = "secondary_wan_disconnected";
				}
			}
			else{
				document.getElementById('speed_status').innerHTML = speed_info;
				if(!Rawifi_support && !Qcawifi_support)
					document.getElementById('rssi_status').innerHTML = rssi_info;
			}	
		}	
	}
	else if(sw_mode == 3){
		if(dhcp_override_support && (location.pathname == "/" || location.pathname == "<% abs_index_page(); %>")){
			if(dnsqmode == "1")
				document.getElementById('single_wan').className = "single_wan_connected";
			else
				document.getElementById('single_wan').className = "single_wan_disconnected";
		}
	}

	// wifi hw sw status
	if(wifi_hw_sw_support && !downsize_8m_support && !downsize_4m_support){
		if(wl_info.band5g_2_support){
				if(wlan0_radio_flag == "0" && wlan1_radio_flag == "0" && wlan2_radio_flag == "0"){
						document.getElementById("wifi_hw_sw_status").className = "wifihwswstatusoff";
						document.getElementById("wifi_hw_sw_status").onclick = function(){}
				}
				else{
						document.getElementById("wifi_hw_sw_status").className = "wifihwswstatuson";
						document.getElementById("wifi_hw_sw_status").onclick = function(){}
				}
		}	
		else if(wl_info.band5g_support){
			if(wlan0_radio_flag == "0" && wlan1_radio_flag == "0"){
					document.getElementById("wifi_hw_sw_status").className = "wifihwswstatusoff";
					document.getElementById("wifi_hw_sw_status").onclick = function(){}
			}
			else{
					document.getElementById("wifi_hw_sw_status").className = "wifihwswstatuson";
					document.getElementById("wifi_hw_sw_status").onclick = function(){}
			}
		}
		else{
				if(wl_info.wlan0_radio_flag == "0"){
					document.getElementById("wifi_hw_sw_status").className = "wifihwswstatusoff";
					document.getElementById("wifi_hw_sw_status").onclick = function(){}
			}
			else{
					document.getElementById("wifi_hw_sw_status").className = "wifihwswstatuson";
					document.getElementById("wifi_hw_sw_status").onclick = function(){}
			}
		}
	}
	else	// No HW switch - reflect actual radio states
	{
		<%radio_status();%>

		if (!band5g_support)
			radio_5 = radio_2;

		if (wl_info.band5g_2_support)
			radio_5b = '<% nvram_get("wl2_radio"); %>';
		else
			radio_5b = radio_5;

		if (radio_2 && radio_5 && parseInt(radio_5b))
		{
			document.getElementById("wifi_hw_sw_status").className = "wifihwswstatuson";
			document.getElementById("wifi_hw_sw_status").onclick = function(){}
		}
		else if (radio_2 || radio_5 || parseInt(radio_5b))
		{
			document.getElementById("wifi_hw_sw_status").className = "wifihwswstatuspartial";  
			document.getElementById("wifi_hw_sw_status").onclick = function(){}
		}
		else
		{
			document.getElementById("wifi_hw_sw_status").className = "wifihwswstatusoff"; 
			document.getElementById("wifi_hw_sw_status").onclick = function(){}
		}
 		document.getElementById("wifi_hw_sw_status").onmouseover = function(){overHint(8);}
		document.getElementById("wifi_hw_sw_status").onmouseout = function(){nd();}
	}

	// usb.storage
	if(usb_support){
		if(allUsbStatus != allUsbStatusTmp && allUsbStatusTmp != ""){
			if(current_url=="<% abs_index_page(); %>"||current_url=="")
				location.href = "<% abs_index_page(); %>";
		}

	 	require(['/require/modules/diskList.js'], function(diskList){
	 		var usbDevicesList = diskList.list();
	 		var index = 0, find_nonprinter = 0, find_storage = 0, find_modem = 0;

			for(index = 0; index < usbDevicesList.length; index++){
				if(usbDevicesList[index].deviceType != "printer"){
					find_nonprinter = 1;
				}
				if(usbDevicesList[index].deviceType == "storage"){
					find_storage = 1;
				}
				if(usbDevicesList[index].deviceType == "modem"){
					find_modem = 1;
				}
				if(find_nonprinter && find_storage && find_modem)
					break;
			}

			if(find_nonprinter){
				document.getElementById("usb_status").className = "usbstatuson";		
			}
			else{
				document.getElementById("usb_status").className = "usbstatusoff";		
			}

			if(find_storage){
				document.getElementById("usb_status").onclick = function(){openHint(24,2);}				
			}
			else if(modem_support && find_modem && !nomodem_support){
				document.getElementById("usb_status").onclick = function(){openHint(24,7);}	
			}			
			else{
				document.getElementById("usb_status").onclick = function(){overHint(2);}			
			}

			document.getElementById("usb_status").onmouseover = function(){overHint(2);}
			document.getElementById("usb_status").onmouseout = function(){nd();}

			allUsbStatusTmp = allUsbStatus;
		});
	}

	// usb.printer
	if(printer_support && !noprinter_support){
		if(allUsbStatus.search("printer") == -1){
			document.getElementById("printer_status").className = "printstatusoff";
			document.getElementById("printer_status").parentNode.style.display = "none";
			document.getElementById("printer_status").onmouseover = function(){overHint(5);}
			document.getElementById("printer_status").onmouseout = function(){nd();}
		}
		else{
			document.getElementById("printer_status").className = "printstatuson";
			document.getElementById("printer_status").parentNode.style.display = "";
			document.getElementById("printer_status").onmouseover = function(){overHint(6);}
			document.getElementById("printer_status").onmouseout = function(){nd();}
			document.getElementById("printer_status").onclick = function(){openHint(24,1);}
		}
	}

	// guest network
	if(multissid_support != -1 && (gn_array_5g.length > 0 || (wl_info.band5g_2_support && gn_array_5g_2.length > 0))){
		if(based_modelid == "RT-AC87U"){	//workaround for RT-AC87U
			for(var i=0; i<gn_array_2g.length; i++){
				if(gn_array_2g[i][0] == 1){
					document.getElementById("guestnetwork_status").className = "guestnetworkstatuson";
					document.getElementById("guestnetwork_status").onclick = function(){openHint(24,4);}
					break;
				}
				else{
					document.getElementById("guestnetwork_status").className = "guestnetworkstatusoff";
					document.getElementById("guestnetwork_status").onclick = function(){overHint(4);}
				}
			}
			
			for(var i=0; i<gn_array_5g.length; i++){
				if(gn_array_2g[i][0] == 1 || gn_array_5g[i][0] == 1){
					document.getElementById("guestnetwork_status").className = "guestnetworkstatuson";
					document.getElementById("guestnetwork_status").onclick = function(){openHint(24,4);}
					break;
				}
				else{
					document.getElementById("guestnetwork_status").className = "guestnetworkstatusoff";
					document.getElementById("guestnetwork_status").onclick = function(){overHint(4);}
				}
			}
		}else{
			for(var i=0; i<gn_array_2g.length; i++){
				if(gn_array_2g[i][0] == 1 || gn_array_5g[i][0] == 1 || (wl_info.band5g_2_support && gn_array_5g_2[i][0] == 1)){
					document.getElementById("guestnetwork_status").className = "guestnetworkstatuson";
					document.getElementById("guestnetwork_status").onclick = function(){openHint(24,4);}
					break;
				}
				else{
					document.getElementById("guestnetwork_status").className = "guestnetworkstatusoff";
					document.getElementById("guestnetwork_status").onclick = function(){overHint(4);}
				}
			}
		}
		
		document.getElementById("guestnetwork_status").onmouseover = function(){overHint(4);}
		document.getElementById("guestnetwork_status").onmouseout = function(){nd();}
		
	}
	else if(multissid_support != -1 && gn_array_5g.length == 0){
		for(var i=0; i<gn_array_2g.length; i++){
			if(gn_array_2g[i][0] == 1){
				document.getElementById("guestnetwork_status").className = "guestnetworkstatuson";
				document.getElementById("guestnetwork_status").onclick = function(){openHint(24,4);}
				break;
			}
			else{
				document.getElementById("guestnetwork_status").className = "guestnetworkstatusoff";
			}
		}
		document.getElementById("guestnetwork_status").onmouseover = function(){overHint(4);}
		document.getElementById("guestnetwork_status").onmouseout = function(){nd();}
	}

	if(cooler_support){
		if(cooler == "cooler=2"){
			document.getElementById("cooler_status").className = "coolerstatusoff";
			document.getElementById("cooler_status").onclick = function(){}
		}
		else{
			document.getElementById("cooler_status").className = "coolerstatuson";
			document.getElementById("cooler_status").onclick = function(){openHint(24,5);}
		}
		document.getElementById("cooler_status").onmouseover = function(){overHint(7);}
		document.getElementById("cooler_status").onmouseout = function(){nd();}
	}

	if(gobi_support && (usb_index != -1) && (sim_state != ""))//Cherry Cho added in 2014/8/25.
	{//Change icon according to status
		if(usb_index == 0)
			usbState = devicemapXML[0].getElementsByTagName("first_wan");
		else if(usb_index == 1)
			usbState = devicemapXML[0].getElementsByTagName("second_wan");
		usb_state = usbState[0].firstChild.nodeValue;
		usb_sbstate = usbState[1].firstChild.nodeValue;
		usb_auxstate = usbState[2].firstChild.nodeValue;

		if(roaming == "1"){
			if(usb_state == 2 && usb_sbstate == 0 && usb_auxstate == 0){
				if(roaming_imsi.length > 0 && roaming_imsi != sim_imsi.substr(0, roaming_imsi.length))
					document.getElementById("simroaming_status").className = "simroamingon";				
			}
		}

		document.getElementById("simsignal").onmouseover = function(){overHint(98)};
		document.getElementById("simsignal").onmouseout = function(){nd();}
		if( sim_state == '1'){			
			switch(sim_signal)
			{
				case '0':
					document.getElementById("simsignal").className = "simsignalno";
					break;
				case '1':
					document.getElementById("simsignal").className = "simsignalmarginal";	
					break;
				case '2':
					document.getElementById("simsignal").className = "simsignalok";	
					break;
				case '3':
					document.getElementById("simsignal").className = "simsignalgood";	
					break;	
				case '4':
					document.getElementById("simsignal").className = "simsignalexcellent";	
					break;											
				case '5':
					document.getElementById("simsignal").className = "simsignalfull";	
					break;	
				default:
					document.getElementById("simsignal").className = "simsignalno";
					break;
			}

			if(parseInt(sim_signal) > 0 && (usb_state == 2 && usb_sbstate == 0 && usb_auxstate == 0)){
				switch(sim_operation)
				{
					case 'Edge':
						document.getElementById("signalsys").innerHTML  = '<img src="/images/mobile/E.png">';
						break;
					case 'GPRS':
						document.getElementById("signalsys").innerHTML = '<img src="/images/mobile/G.png">';	
						break;
					case 'WCDMA':
					case 'CDMA':
					case 'EV-DO REV 0':	
					case 'EV-DO REV A':		
					case 'EV-DO REV B':
						document.getElementById("signalsys").innerHTML = '<img src="/images/mobile/3G.png">';	
							break;	
					case 'HSDPA':										
					case 'HSUPA':
						document.getElementById("signalsys").innerHTML = '<img src="/images/mobile/H.png">';	
						break;	
					case 'HSDPA+':										
					case 'DC-HSDPA+':
						document.getElementById("signalsys").innerHTML = '<img src="/images/mobile/H+.png">';	
						break;		
					case 'LTE':
						document.getElementById("signalsys").innerHTML = '<img src="/images/mobile/LTE.png">';	
						break;		
					case 'GSM':	
					default:
						document.getElementById("signalsys").innerHTML = "";
						break;
				}
			}
		}
		else{
			document.getElementById("simsignal").className = "simsignalno";
			document.getElementById("signalsys").innerHTML = "";
		}
	}	

	if(((modem_support && hadPlugged("modem") && !nomodem_support) || gobi_support) && (usb_index != -1) && (sim_state != "")){
		document.getElementById("sim_status").onmouseover = function(){overHint(99)};
		document.getElementById("sim_status").onmouseout = function(){nd();}	
		switch(sim_state)
		{
			case '-1':
				document.getElementById("sim_status").className = "simnone";	
				break;
			case '1':
				document.getElementById("sim_status").className = "simexist";			
				break;
			case '2':
			case '4':
				document.getElementById("sim_status").className = "simlock";
				document.getElementById("sim_status").onclick = function(){openHint(24,7);}
				break;
			case '3':
			case '5':
				document.getElementById("sim_status").className = "simfail";
				document.getElementById("sim_status").onclick = function(){openHint(24,7);}	
				break;
			case '6':
			case '-2':
			case '-10':
				document.getElementById("sim_status").className = "simfail";					
				break;
			default:
				break;
		}
	}

	if(nt_center_support)
		setTimeout(function(){notification.updateNTDB_Status();}, 10000);
	else
		notification.updateNTDB_Status()
	
	if(window.frames["statusframe"] && window.frames["statusframe"].stopFlag == 1 || stopFlag == 1){
		return 0;
	}
}	

function db(obj){
	if(typeof console == 'object')
		console.log(obj);
}

function dbObj(obj){
	for(var j in obj){	
		if(j!="textContent" && j!="outerHTML" && j!="innerHTML" && j!="innerText" && j!="outerText")
			db(j+" : "+obj[j]);
	}
}

function FormActions(_Action, _ActionMode, _ActionScript, _ActionWait){
	if(_Action != "")
		document.form.action = _Action;
	if(_ActionMode != "")
		document.form.action_mode.value = _ActionMode;
	if(_ActionScript != "")
		document.form.action_script.value = _ActionScript;
	if(_ActionWait != "")
		document.form.action_wait.value = _ActionWait;
}

function change_wl_unit(){
	FormActions("apply.cgi", "change_wl_unit", "", "");
	document.form.target = "";
	document.form.submit();
}

function addNewScript(scriptName){
	var script = document.createElement("script");
	script.type = "text/javascript";
	script.src = scriptName;
	document.getElementsByTagName("head")[0].appendChild(script);
}

function addNewCSS(cssName){
	var cssNode = document.createElement('link');
	cssNode.type = 'text/css';
	cssNode.rel = 'stylesheet';
	cssNode.href = cssName;
	document.getElementsByTagName("head")[0].appendChild(cssNode);
}

function unload_body(){
}

function enableCheckChangedStatus(){
}

function disableCheckChangedStatus(){
	stopFlag = 1;
}

function get_changed_status(){
}

function isMobile(){
	/*if(!tmo_support)
		return false;*/

	if(	navigator.userAgent.match(/iPhone/i) || 
		navigator.userAgent.match(/iPod/i)    ||
		navigator.userAgent.match(/iPad/i)    ||
		(navigator.userAgent.match(/Android/i) && (navigator.userAgent.match(/Mobile/i) || navigator.userAgent.match(/Tablet/i))) ||
		//(navigator.userAgent.match(/Android/i) && navigator.userAgent.match(/Mobile/i))||			//Android phone
		(navigator.userAgent.match(/Opera/i) && (navigator.userAgent.match(/Mobi/i) || navigator.userAgent.match(/Mini/i))) ||		// Opera mobile or Opera Mini
		navigator.userAgent.match(/IEMobile/i)	||		// IE Mobile
		navigator.userAgent.match(/BlackBerry/i)		//BlackBerry
	 ){
		return true;
	}
	else{
		return false;
	}
}

var stopAutoFocus;
function autoFocus(str){
	if(str == "")
		return false;

	stopAutoFocus = 0;
	if(document.form){
		for(var i = 0; i < document.form.length; i++){
			if(document.form[i].name == str){
				var sec = 600;
				var maxAF = 20;
				if(navigator.userAgent.toLowerCase().search("webkit") < 0){
					window.onclick = function(){stopAutoFocus=1;document.form[i].style.border='';}
					for(var j=0; j<maxAF; j++){
						setTimeout("if(stopAutoFocus==0)document.form["+i+"].style.border='1px solid #56B4EF';", sec*j++);
						setTimeout("if(stopAutoFocus==0)document.form["+i+"].style.border='';", sec*j);							
					}
				}
				else{
					window.onclick = function(){stopAutoFocus=1;}
					document.form[i].focus();
					for(var j=1; j<maxAF; j++){
						setTimeout("if(stopAutoFocus==0)document.form["+i+"].blur();", sec*j++);
						setTimeout("if(stopAutoFocus==0)document.form["+i+"].focus();", sec*j);
					}
				}
				break;
			}
		}
	}
}

function charToAscii(str){
	var retAscii = "";
	for(var i = 0; i < str.length; i++){
		retAscii += "%";
		retAscii += str.charCodeAt(i).toString(16).toUpperCase();
	}
	return retAscii;
}

function set_variable(_variable, _val){
	var NewInput = document.createElement("input");
	NewInput.type = "hidden";
	NewInput.name = _variable;
	NewInput.value = _val;
	document.form.appendChild(NewInput);
}

function isPortConflict(_val){
	if(_val == '<% nvram_get("http_lanport"); %>')
		return "<#portConflictHint#> HTTP LAN port.";
	else if(_val == '<% nvram_get("dm_http_port"); %>')
		return "<#portConflictHint#> Download Master.";
	else if(_val == '<% nvram_get("webdav_http_port"); %>')
		return "<#portConflictHint#> Cloud Disk.";
	else if(_val == '<% nvram_get("webdav_https_port"); %>')
		return "<#portConflictHint#> Cloud Disk.";
	else
		return false;
}

//Jieming added at 2013.05.24, to switch type of password to text or password
//for IE, need to use two input field and ID should be "xxx", "xxx_text" 
var isNotIE = (navigator.userAgent.search("MSIE") == -1); 
function switchType(obj, showText, chkBox){
	if(chkBox == undefined) chkBox = false;
	var newType = showText ? "text" : "password";
	if(isNotIE){	//Not IE
		obj.type = newType;
	}
	else {	//IE
		if(obj.type != newType)
		{
			var input2 = document.createElement('input');
			input2.mergeAttributes(obj);
			input2.type = newType;
			input2.name = obj.name;
			input2.id = obj.id? obj.id : obj.name;
			input2.value = obj.value;

			obj.parentNode.replaceChild(input2,obj);
			if(showText && input2.id && !chkBox)
				setTimeout(function(){document.getElementById(input2.id).focus()},10);
		}
	}
}

function corrected_timezone(){
	var today = new Date();
	var StrIndex;	
	if(today.toString().lastIndexOf("-") > 0)
		StrIndex = today.toString().lastIndexOf("-");
	else if(today.toString().lastIndexOf("+") > 0)
		StrIndex = today.toString().lastIndexOf("+");

	if(StrIndex > 0){
		if(timezone != today.toString().substring(StrIndex, StrIndex+5)){
			document.getElementById("timezone_hint_div").style.display = "";
			document.getElementById("timezone_hint").innerHTML = "* <#LANHostConfig_x_TimeZone_itemhint#>";
		}
		else
			return;
	}
	else
		return;	
}

String.prototype.howMany = function(val){
	var result = this.toString().match(new RegExp(val ,"g"));
	var count = (result)?result.length:0;

	return count;
}

/* convert some special character for shown string */
function handle_show_str(show_str)
{
	show_str = show_str.replace(/\&/g, "&amp;");
	show_str = show_str.replace(/\</g, "&lt;");
	show_str = show_str.replace(/\>/g, "&gt;");
	show_str = show_str.replace(/\ /g, "&nbsp;");
	return show_str;
}

function decodeURIComponentSafe(_ascii){
	try{
		return decodeURIComponent(_ascii);
	}
	catch(err){
		return _ascii;
	}
}

/*check the source IP conflict with the compare item whether it or not
CompareItem: WAN, LAN, OpenVPN PPTP, OpenVPN, VLAN LAN1~LAN8
sourceIP: User keyin IP
sourceMask: User keyin Mask
*/
function checkIPConflict(CompareItem, sourceIP, sourceMask, compareIP, compareMask)
{
	//create Constructor
	var SetIPConflictAttr = function () {
		this.state = false;
		this.ipAddr = "";
		this.mask = "";
		this.netRangeStart = "";
		this.netRangeEnd = "";
		this.netLegalRangeStart = "";
		this.netLegalRangeEnd = "";
	};

	var ipConflict = new SetIPConflictAttr();

	var calculatorNetworkSegmentRange = function (compareIP, compareMask) {
		var gatewayIPArray = compareIP.split(".");
		var netMaskArray = compareMask.split(".");
		var ipPoolStartArray  = new Array();
		var ipPoolEndArray  = new Array();
		var ipActualRange = "";
		var ipLegalRange = "";

		ipPoolStartArray[0] = (gatewayIPArray[0] & 0xFF) & (netMaskArray[0] & 0xFF);
		ipPoolStartArray[1] = (gatewayIPArray[1] & 0xFF) & (netMaskArray[1] & 0xFF);
		ipPoolStartArray[2] = (gatewayIPArray[2] & 0xFF) & (netMaskArray[2] & 0xFF);
		ipPoolStartArray[3] = (gatewayIPArray[3] & 0xFF) & (netMaskArray[3] & 0xFF);

		ipPoolEndArray[0] = (gatewayIPArray[0] & 0xFF) | (~netMaskArray[0] & 0xFF);
		ipPoolEndArray[1] = (gatewayIPArray[1] & 0xFF) | (~netMaskArray[1] & 0xFF);
		ipPoolEndArray[2] = (gatewayIPArray[2] & 0xFF) | (~netMaskArray[2] & 0xFF);
		ipPoolEndArray[3] = (gatewayIPArray[3] & 0xFF) | (~netMaskArray[3] & 0xFF);

		//actual range ex. 192.168.1.0>192.168.1.255
		ipActualRange = ipPoolStartArray[0] + "." + ipPoolStartArray[1] + "." + ipPoolStartArray[2] + "." + ipPoolStartArray[3] + ">" + 
			ipPoolEndArray[0] + "." + ipPoolEndArray[1] + "." + ipPoolEndArray[2] + "." + ipPoolEndArray[3];
		//legal range ex. 192.168.1.1>192.168.1.254
		ipLegalRange = ipPoolStartArray[0] + "." + ipPoolStartArray[1] + "." + ipPoolStartArray[2] + "." + (ipPoolStartArray[3] + 1) + ">" + 
			ipPoolEndArray[0] + "." + ipPoolEndArray[1] + "." + ipPoolEndArray[2] + "." + (ipPoolEndArray[3] - 1);

		return ipActualRange + ">" + ipLegalRange;	
	};

	var checkRangeConflict = function (sourceRangeStart, sourceRangeEnd, compareRangeStart, compareRangeEnd) {
		var sourceNetStartNum = inet_network(sourceRangeStart);
		var sourceNetEndNum = inet_network(sourceRangeEnd);

		var compareNetStartNum = inet_network(compareRangeStart);
		var compareNetEndNum = inet_network(compareRangeEnd);

		//case 1 source start in compare range, case 2 source end in compare range, case 3, compare in source range
		if( (sourceNetStartNum >= compareNetStartNum && sourceNetStartNum <= compareNetEndNum) || //case 1
			(sourceNetEndNum >= compareNetStartNum && sourceNetEndNum <= compareNetEndNum) || //case 2
			(sourceNetStartNum <= compareNetStartNum && sourceNetStartNum <= compareNetEndNum && //case 3
			sourceNetEndNum >= compareNetStartNum && sourceNetEndNum >= compareNetEndNum) ) { 
			return true;
		}
		else {
			return false;
		}
	};

	var setIPConflictValue = function (compareIP, compareMask, sourceIP, sourceMask) {
		var compareNetRangeArray = "";
		var sourceNetRangeArray = "";

		ipConflict.ipAddr = compareIP;
		ipConflict.mask = compareMask;

		compareNetRangeArray = calculatorNetworkSegmentRange(ipConflict.ipAddr, ipConflict.mask).split(">");

		ipConflict.netRangeStart = compareNetRangeArray[0];
		ipConflict.netRangeEnd = compareNetRangeArray[1];
		ipConflict.netLegalRangeStart = compareNetRangeArray[2];
		ipConflict.netLegalRangeEnd = compareNetRangeArray[3];

		sourceNetRangeArray = calculatorNetworkSegmentRange(sourceIP, sourceMask).split(">");
	
		ipConflict.state = checkRangeConflict(sourceNetRangeArray[0], sourceNetRangeArray[1], ipConflict.netRangeStart, ipConflict.netRangeEnd);
	};

	var iSourceIndex = 0;

	if(CompareItem.search("VLAN") !== -1)
	{
		iSourceIndex = parseInt(CompareItem.substring(4,5));
		CompareItem = CompareItem.substring(0,4);
	}

	if(CompareItem.search("subnet") !== -1)
	{
		iSourceIndex = parseInt(CompareItem.substring(6,7));
		CompareItem = CompareItem.substring(0,6).toUpperCase();
	}	

	switch(CompareItem)
	{
		case "WAN":
			var wanIP = wanlink_ipaddr();
			var wanMask = wanlink_netmask();
			if(wanIP != "0.0.0.0" && wanIP != "" && wanMask != "0.0.0.0" && wanMask != "") {
				setIPConflictValue(wanIP, wanMask, sourceIP, sourceMask);
			}
			break;
		case "LAN":
			setIPConflictValue('<% nvram_get("lan_ipaddr"); %>', '<% nvram_get("lan_netmask"); %>', sourceIP, sourceMask);
			break;
		case "PPTP":
			var pptpIP = '<% nvram_get("pptpd_clients"); %>';
			pptpIP = pptpIP.split("-")[0];
			setIPConflictValue(pptpIP, "255.255.255.0", sourceIP, sourceMask);
			break;
		case "OpenVPN":
			setIPConflictValue('<% nvram_get("vpn_server_sn"); %>', '<% nvram_get("vpn_server_nm"); %>', sourceIP, sourceMask);
			break;
		case "VLAN":
			var subnet_rulelist_array = decodeURIComponent("<% nvram_char_to_ascii("","subnet_rulelist"); %>");
			var subnet_rulelist_row = subnet_rulelist_array.split('<');
			var subnet_rulelist_col = subnet_rulelist_row[iSourceIndex].split('>');

			var vlanIP = subnet_rulelist_col[1];
			var vlanMask = subnet_rulelist_col[2];
			setIPConflictValue(vlanIP, vlanMask, sourceIP, sourceMask);
			break;
		case "SUBNET":
			var gatewayIP = "";
			var netMask = "";
			if(tagged_based_vlan){
				gatewayIP = compareIP;
				netMask = compareMask;
			}
			else{
				var subnet_rulelist_array = decodeURIComponent("<% nvram_char_to_ascii("","subnet_rulelist"); %>");
				var subnet_rulelist_row = subnet_rulelist_array.split('<');
				for(var i = 1; i < subnet_rulelist_row.length; i++) {
					var subnet_rulelist_col = subnet_rulelist_row[i].split('>');
					if(subnet_rulelist_col[0].substring(6, 7) == iSourceIndex){
						gatewayIP = subnet_rulelist_col[1];
						netMask = subnet_rulelist_col[2];
					}
				}
			}
			setIPConflictValue(gatewayIP, netMask, sourceIP, sourceMask);
			break;
		default:
			setIPConflictValue(compareIP, compareMask, sourceIP, sourceMask);
			break;
	}

	return ipConflict;
}

var isNewFW = function(FWVer, check_path, current_path){	//path> 0:stable, 1:beta
	if (no_update_support) return false;

	var Latest_firmver = FWVer.split("_");
	// 3004_999_2262-g260cdd9
	if(typeof Latest_firmver[0] !== "undefined" && typeof Latest_firmver[1] !== "undefined" && typeof Latest_firmver[2] !== "undefined"){
		var Latest_firm = parseInt(Latest_firmver[0]);
		var Latest_buildno = parseInt(Latest_firmver[1]);
		var Latest_extendno = parseInt(Latest_firmver[2].split("-g")[0].replace(/^[0-9]$/,"10$&").replace(/alpha/gi,"1").replace(/beta/gi,"5"));

		current_firm = parseInt('<% nvram_get("buildno"); %>'.split(".")[0].replace(/[.]/gi,""));
		current_buildno = parseInt('<% nvram_get("buildno"); %>'.split(".")[1].replace(/[.]/gi,""));
		//current_rcno = parseInt('<% nvram_get("rcno"); %>');
		current_extendno = parseInt('<% nvram_get("extendno"); %>'.split("-g")[0].replace(/^[0-9]$/,"10$&").replace(/alpha/gi,"1").replace(/beta/gi,"5"));
		if((current_firm < Latest_firm) ||
			(current_buildno < Latest_buildno && current_firm == Latest_firm) ||
			(current_extendno < Latest_extendno && current_buildno == Latest_buildno && current_firm == Latest_firm))
		{
			return true;
		}
	}
	return false;
}

function getBrowser_info(){
	var browser = {};
        var temp = navigator.userAgent.toUpperCase();

	if(temp.match(/RV:([\d.]+)\) LIKE GECKO/)){	     // for IE 11
		browser.ie = temp.match(/RV:([\d.]+)\) LIKE GECKO/)[1];
	}
	else if(temp.match(/MSIE ([\d.]+)/)){	   // for IE 10 or older
		browser.ie = temp.match(/MSIE ([\d.]+)/)[1];
	}
	else if(temp.match(/CHROME\/([\d.]+)/)){
		if(temp.match(/OPR\/([\d.]+)/)){		// for Opera 15 or newer
			browser.opera = temp.match(/OPR\/([\d.]+)/)[1];
		}
		else{
			browser.chrome = temp.match(/CHROME\/([\d.]+)/)[1];	     // for Google Chrome
		}
	}
	else if(temp.match(/FIREFOX\/([\d.]+)/)){
		browser.firefox = temp.match(/FIREFOX\/([\d.]+)/)[1];
	}
	else if(temp.match(/OPERA\/([\d.]+)/)){	 // for Opera 12 or older
		browser.opera = temp.match(/OPERA\/([\d.]+)/)[1];
	}
	else if(temp.match(/VERSION\/([\d.]+).*SAFARI/)){	       // for Safari
		browser.safari = temp.match(/VERSION\/([\d.]+).*SAFARI/)[1];
	}

	return browser;
}

function regen_band(obj_name){
	var band_desc = new Array();
	var band_value = new Array();
	current_band = '<% nvram_get("wl_unit"); %>';
	for(i=1;i<wl_info.band2g_total+1;i++){
		if(wl_info.band2g_total == 1)
			band_desc.push("2.4GHz");
		else
			band_desc.push("2.4GHz-"+i);
	}
	for(i=1;i<wl_info.band5g_total+1;i++){
		if(wl_info.band5g_total == 1)
			band_desc.push("5GHz");
		else
			band_desc.push("5GHz-"+i);	
	}
	for(i=1;i<wl_info.band60g_total+1;i++){
		if(wl_info.band60g_total == 1)
			band_desc.push("60GHz");
		else
			band_desc.push("60GHz-"+i);
	}
	for(i=0;i<wl_nband_array.length;i++){
		if (wl_nband_array[i] != 2 && wl_nband_array[i] != 1 && wl_nband_array[i] != 6)
			continue;
		band_value.push(i);
	}
	add_options_x2(obj_name, band_desc, band_value, current_band);
}

//check browser support FileReader or not
function isSupportFileReader() {
	if(typeof window.FileReader === "undefined") {
		return false;
	}
	else {
		return true;
	}
}
//check browser support canvas or not
function isSupportCanvas() {
	var elem = document.createElement("canvas");
	return !!(elem.getContext && elem.getContext('2d'));
}

//check BWDPI engine status
function check_bwdpi_engine_status() {
	var status = false;
	if(bwdpi_support) {
		var dpi_engine_status = <%bwdpi_engine_status();%>;
		if(dpi_engine_status.DpiEngine == 1)
			status = true;
	}

	return status;
}

//check dual wan status
function check_dual_wan_status() {
	var dual_wan_status = {};
	dual_wan_status.status = dualwan_enabled;
	dual_wan_status.mode = '<% nvram_get("wans_mode"); %>';
	return dual_wan_status;
}

//change dual wan mode from load balance to fail-over
function reset_wan_to_fo(_formObj, _controllerStatus) {
	var confirm_status = true;

	if(_controllerStatus == "1") {
		if(!check_bwdpi_engine_status()) {
			var dual_wan_lb_status = (check_dual_wan_status().status == "1" && check_dual_wan_status().mode == "lb") ? true : false;

			if(dual_wan_lb_status) {
				var confirm_hint = 'Dual-WAN "load balance" mode will be switched to "fail-over" while enable "AiProtection" features, Are you sure to continue?'/*untranslated*/
				var confirm_flag = confirm(confirm_hint);
				if(confirm_flag) {
					var gen_hidden_item = function(_name, _value, _formObj) {
						var input = document.createElement('input');
						input.type = 'hidden';
						input.name = _name;
						input.value = _value;
						_formObj.appendChild(input);
					};

					var removeElement = function(element) {
						element && element.parentNode && element.parentNode.removeChild(element);
					}

					if(_formObj.children.wans_mode != null) {
						removeElement(_formObj.children.wans_mode);
					}
					gen_hidden_item("wans_mode", "fo", _formObj);

					_formObj.action_script.value = "reboot";
					_formObj.action_wait.value = '<% nvram_get("reboot_time"); %>';
				
					confirm_status = true;
				}
				else {
					confirm_status = false;
				}
			}
		}
	}
	return confirm_status;
}

function get_protocol() {
	var protocol = "http:";
	if(window.location.protocol == "http:" || window.location.protocol == "https:") {
		protocol = window.location.protocol;
		return protocol;
	}

	return protocol;
}

function get_captive_portal_wl_idx(_type) {
	var _wl_idx = 0;
	var _total_guest = multissid_support;
	switch(_type) {
		case "freeWiFi" :
			_wl_idx = _total_guest;
			break;
		case "captivePortal" :
			_wl_idx = (_total_guest - 1);
			break;
		case "facebookWiFi" :
			if(cp_freewifi_support && cp_advanced_support)
				_wl_idx = (_total_guest - 2);
			else if(cp_freewifi_support || cp_advanced_support)
				_wl_idx = (_total_guest - 1);
			else
				_wl_idx = _total_guest;
			break;
	}
	return _wl_idx;
}

var sort_by = function(field, reverse, primer){

	var key = primer ?
	function(x) {return primer(x[field])} :
	function(x) {return x[field]};

	reverse = !reverse ? 1 : -1;

	return function (a, b) {
		return a = key(a), b = key(b), reverse * ((a > b) - (b > a));
	}
}
