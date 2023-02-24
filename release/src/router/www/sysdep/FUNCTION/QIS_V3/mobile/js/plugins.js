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

jQuery.fn.showPasswordScore = function(id){
	var postfix = (id == undefined)? "": ("_"+id);
	$("<div>")
		.attr("id", "scorebarBorder"+postfix)
		.attr("title", "<#LANHostConfig_x_Password_itemSecur#>")
		.addClass("scorebarBorder")
		.appendTo(this.parent());

	$("<div>")
		.attr("id", "scorebar"+postfix)
		.addClass("scorebar")
		.appendTo($("#scorebarBorder"+postfix));
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

function getScArray(mode){
	var smartConnectTable = {
		"SC_2g5g": {
			"wlArray": (function(){
				if(isSupport("quadband"))
					return [{"title":"2.4 GHz / 5 GHz-1 / 5 GHz-2", "ifname":get_wl_unit_by_band("2G")}, {"title":"6 GHz", "ifname":get_wl_unit_by_band("6G")}];
				else
					return [{"title":"2.4 GHz / 5 GHz", "ifname":"0"}, {"title":"6 GHz", "ifname":"2"}];
			})(),
			"value": (function(){
				if(isSupport("quadband"))
					return 1;
				else
					return 3;
			})()
		},
		"SC_5g5g": {
			"wlArray": [{"title":"2.4 GHz", "ifname":"0"}, {"title":"5 GHz", "ifname":"1"}],
			"value": 2
		},
		"SC_dwb_mode": {
			"wlArray": [{"title":"2.4 GHz / 5 GHz-1", "ifname":"0"}, {"title":"5 GHz-2", "ifname":"2"}],
			"value": 1
		},
		"SC_all": {
			"wlArray": [{"title":"", "ifname":"0"}],
			"value": 1
		}
	}

	return smartConnectTable[mode] ||  smartConnectTable["SC_all"];
}

function getAllWlArray(){
	if(isSupport("quadband")){
		var wlArrayRet = [			
			{"title":"2.4 GHz", "ifname":get_wl_unit_by_band("2G"), "suffix": ""},
			{"title":"5 GHz-1", "ifname":get_wl_unit_by_band("5G"), "suffix": "_5G-1"},
			{"title":"5 GHz-2", "ifname":get_wl_unit_by_band("5G2"), "suffix": "_5G-2"},
			{"title":"6 GHz", "ifname":get_wl_unit_by_band("6G"), "suffix": "_6G"},
		];
		if(isSwMode('RP') || isSwMode('MB')){
			wlArrayRet = [				
				{ title: '2.4 GHz', ifname: get_wl_unit_by_band('2G'), suffix: '' },				
				{ title: '5 GHz-1', ifname: get_wl_unit_by_band('5G'), suffix: '_5G-1' },
				{ title: '5 GHz-2', ifname: get_wl_unit_by_band('5G2'), suffix: '_5G-2' },
				{ title: '6 GHz', ifname: get_wl_unit_by_band('6G'), suffix: '_6G' },
			];
		}
		document.querySelector('label[for="wireless_checkbox"]').innerHTML = 'Separate 2.4 GHz, 5 GHz-1, 5 GHz-2 and 6 GHz';
	}
	else{
		var wlArrayRet = [{"title":"2.4 GHz", "ifname":get_wl_unit_by_band('2G'), "suffix": ""}];

		if(isSupport("triband")){
			if(isSupport('wifi6e')){
				document.querySelector('label[for="wireless_checkbox"]').innerHTML = '<#qis_wireless_setting_separate1#>';
				wlArrayRet.push({"title":"5 GHz", "ifname":get_wl_unit_by_band('5G'), "suffix": "_5G"});
				wlArrayRet.push({"title":"6 GHz", "ifname":get_wl_unit_by_band('6G'), "suffix": "_6G"});
			}
			else{
				wlArrayRet.push({"title":"5 GHz-1", "ifname":get_wl_unit_by_band('5G'), "suffix": "_5G-1"});
				wlArrayRet.push({"title":"5 GHz-2", "ifname":get_wl_unit_by_band('5G2'), "suffix": "_5G-2"});
			}

			if(isSupport("prelink") && isSupport("amas_bdl")){
				if(!isSupport("prelink_mssid")){
					if(isSwMode("RT") || isSwMode("AP")){
						var prelink_unit = isSupport("prelink_unit");
						if(prelink_unit >= 0)
							wlArrayRet.splice(prelink_unit, 1);
					}
				}
			}
		}
		else if(isSupport("dualband") || isSupport('5G')){
			wlArrayRet.push({"title":"5 GHz", "ifname":get_wl_unit_by_band('5G'), "suffix": "_5G"})
		}

		if(isSupport('wigig')){
			wlArrayRet.push({"title":"60 GHz", "ifname":get_wl_unit_by_band('60G'), "suffix": "_60G"});
		}
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
				else if(based_modelid === 'GT-AXE16000'){
					return (ch >= 36 && ch <= 64) ? {name: "5GHz-1", unit: 0} : {name: "5GHz-2", unit: 1};
				}
				else
					return {name: "5GHz", unit: 1};
			}
		}

		if(_profile == null || _profile.length == 0)
			_profile = ["", "", "", "", "", "", "", "", "", ""];

		this.band = (function(){
			if(isSupport('wifi6e') && _profile[0] === '6G'){
				return "6GHz";
				//return {name: "6GHz", unit: 2};
			}
			else if(isSupport('wifi6e') && _profile[0] === '5G'){
				return "5GHz";
			}
			else{
				return getBandWidthName(_profile[2]).name;
			}
			
		})();
		this.unit = (function(){
			if(based_modelid === 'GT-AXE16000'){
				if(_profile[0] === '6G'){
					return 2;
				}
				else if(_profile[0] === '2G'){
					return 3;
				}
				else{	// 5 GHz-1 / 5 GHz-2
					return getBandWidthName(_profile[2]).unit;
				}
			}
			else if(isSupport('wifi6e') && _profile[0] === '6G'){
				return 2;
				//return {name: "6GHz", unit: 2};
			}
			else if(isSupport('wifi6e') && _profile[0] === '5G'){
				return 1;
			}
			else{
				return getBandWidthName(_profile[2]).unit;
			}		
		})();
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

function getAiMeshOnboardinglist(_onboardingList){
	var jsonArray = [];
	var profile = function(){
		this.name = "";
		this.ui_model_name = "";
		this.signal = "";
		this.rssi = "";
		this.source = "";
		this.mac = "";
		this.pap_mac = "";
		this.id = "";
		this.tcode = "";
		this.type = "";
		this.cobrand = "";
		this.misc_info = {};
	};
	var convRSSI = function(val) {
		var result = 1;
		val = parseInt(val);
		if(val >= -50) result = 4;
		else if(val >= -80) result = Math.ceil((24 + ((val + 80) * 26)/10)/25);
		else if(val >= -90) result = Math.ceil((((val + 90) * 26)/10)/25);
		else return 1;

		if(result == 0) result = 1;
		return result;
	};

	Object.keys(_onboardingList).forEach(function(key) {
		var papMac = key;
		var newReMacArray = _onboardingList[papMac];
		Object.keys(newReMacArray).forEach(function(key) {
			var newReMac = key;
			var node_info  = new profile();
			node_info.name = newReMacArray[newReMac].model_name;
			node_info.ui_model_name = newReMacArray[newReMac].ui_model_name;
			node_info.signal = convRSSI(newReMacArray[newReMac].rssi);
			node_info.rssi = newReMacArray[newReMac].rssi;
			node_info.source = newReMacArray[newReMac].source;
			node_info.mac = newReMac;
			node_info.pap_mac = papMac;
			node_info.id = newReMac.replace(/:/g, "");
			node_info.tcode = newReMacArray[newReMac].tcode;
			node_info.type = newReMacArray[newReMac].type;
			node_info.cobrand = httpApi.aimesh_get_misc_info(newReMacArray[newReMac]).cobrand;
			node_info.misc_info = newReMacArray[newReMac].misc_info;
			jsonArray.push(node_info);
		});
	});

	return jsonArray;
}

function getProcessPercentage(_start, _current, _timeout, _percentage){
	var percentage = 0;
	var interval = parseInt(_current) - parseInt(_start);
	var denominator = parseInt(_timeout) / parseInt(_percentage);
	percentage = Math.round( interval / denominator );
	if(percentage > 100)
		percentage = 100;
	return percentage;
}

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

function checkPasswd($obj){
	var targetObj = $(this);
	targetObj.toggleClass("icon_eye_close").toggleClass("icon_eye_open");

	$.each( targetObj.attr("for").split(" "), function(i, val){
		$("#"+val).prop("type", (function(){return targetObj.hasClass("icon_eye_close") ? "password" : "text";}()));
	});
}

String.prototype.strReverse = function() {
	var newstring = "";
	for (var s=0; s < this.length; s++) {
		newstring = this.charAt(s) + newstring;
	}
	return newstring;
};

function chkPass(flag, pwd, idx) {
	var orig_pwd = "";
	var postfix = (idx == undefined)? "": ("_" + idx);
	var oScorebar = document.getElementById("scorebar"+postfix);

	if(flag == "httpd" && (isSku("KR") || isSku("SG") || isSku("AA"))){
		oScorebar.style.display = "none";
		return;
	}

	// Simultaneous variable declaration and value assignment aren't supported in IE apparently
	// so I'm forced to assign the same value individually per var to support a crappy browser *sigh*
	var nScore=0, nLength=0, nAlphaUC=0, nAlphaLC=0, nNumber=0, nSymbol=0, nMidChar=0, nRequirements=0, nAlphasOnly=0, nNumbersOnly=0, nUnqChar=0, nRepChar=0, nRepInc=0, nConsecAlphaUC=0, nConsecAlphaLC=0, nConsecNumber=0, nConsecSymbol=0, nConsecCharType=0, nSeqAlpha=0, nSeqNumber=0, nSeqSymbol=0, nSeqChar=0, nReqChar=0, nMultConsecCharType=0;
	var nMultRepChar=1, nMultConsecSymbol=1;
	var nMultMidChar=2, nMultRequirements=2, nMultConsecAlphaUC=2, nMultConsecAlphaLC=2, nMultConsecNumber=2;
	var nReqCharType=3, nMultAlphaUC=3, nMultAlphaLC=3, nMultSeqAlpha=3, nMultSeqNumber=3, nMultSeqSymbol=3;
	var nMultLength=4, nMultNumber=4;
	var nMultSymbol=6;
	var nTmpAlphaUC="", nTmpAlphaLC="", nTmpNumber="", nTmpSymbol="";
	var sAlphaUC="0", sAlphaLC="0", sNumber="0", sSymbol="0", sMidChar="0", sRequirements="0", sAlphasOnly="0", sNumbersOnly="0", sRepChar="0", sConsecAlphaUC="0", sConsecAlphaLC="0", sConsecNumber="0", sSeqAlpha="0", sSeqNumber="0", sSeqSymbol="0";
	var sAlphas = "abcdefghijklmnopqrstuvwxyz";
	var sNumerics = "01234567890";
	var sSymbols = "~!@#$%^&*()_+";
	var sComplexity = "";
	var nMinPwdLen = 8;
	if (document.all) { var nd = 0; } else { var nd = 1; }
	if (pwd) {
		nScore = parseInt(pwd.length * nMultLength);
		nLength = pwd.length;
		var arrPwd = pwd.replace(/\s+/g,"").split(/\s*/);
		var arrPwdLen = arrPwd.length;

		/* Main calculation for strength:
				Loop through password to check for Symbol, Numeric, Lowercase and Uppercase pattern matches */
		for (var a=0; a < arrPwdLen; a++) {
			if (arrPwd[a].match(/[A-Z]/g)) {
				if (nTmpAlphaUC !== "") { if ((nTmpAlphaUC + 1) == a) { nConsecAlphaUC++; nConsecCharType++; } }
				nTmpAlphaUC = a;
				nAlphaUC++;
			}
			else if (arrPwd[a].match(/[a-z]/g)) {
				if (nTmpAlphaLC !== "") { if ((nTmpAlphaLC + 1) == a) { nConsecAlphaLC++; nConsecCharType++; } }
				nTmpAlphaLC = a;
				nAlphaLC++;
			}
			else if (arrPwd[a].match(/[0-9]/g)) {
				if (a > 0 && a < (arrPwdLen - 1)) { nMidChar++; }
				if (nTmpNumber !== "") { if ((nTmpNumber + 1) == a) { nConsecNumber++; nConsecCharType++; } }
				nTmpNumber = a;
				nNumber++;
			}
			else if (arrPwd[a].match(/[^a-zA-Z0-9_]/g)) {
				if (a > 0 && a < (arrPwdLen - 1)) { nMidChar++; }
				if (nTmpSymbol !== "") { if ((nTmpSymbol + 1) == a) { nConsecSymbol++; nConsecCharType++; } }
				nTmpSymbol = a;
				nSymbol++;
			}
			/* Internal loop through password to check for repeat characters */
			var bCharExists = false;
			for (var b=0; b < arrPwdLen; b++) {
				if (arrPwd[a] == arrPwd[b] && a != b) { /* repeat character exists */
					bCharExists = true;
					/*
					Calculate icrement deduction based on proximity to identical characters
					Deduction is incremented each time a new match is discovered
					Deduction amount is based on total password length divided by the
					difference of distance between currently selected match
					*/
					nRepInc += Math.abs(arrPwdLen/(b-a));
				}
			}
			if (bCharExists) {
				nRepChar++;
				nUnqChar = arrPwdLen-nRepChar;
				nRepInc = (nUnqChar) ? Math.ceil(nRepInc/nUnqChar) : Math.ceil(nRepInc);
			}
		}

		/* Check for sequential alpha string patterns (forward and reverse) */
		for (var s=0; s < 23; s++) {
			var sFwd = sAlphas.substring(s,parseInt(s+3));
			var sRev = sFwd.strReverse();
			if (pwd.toLowerCase().indexOf(sFwd) != -1 || pwd.toLowerCase().indexOf(sRev) != -1) { nSeqAlpha++; nSeqChar++;}
		}

		/* Check for sequential numeric string patterns (forward and reverse) */
		for (var s=0; s < 8; s++) {
			var sFwd = sNumerics.substring(s,parseInt(s+3));
			var sRev = sFwd.strReverse();
			if (pwd.toLowerCase().indexOf(sFwd) != -1 || pwd.toLowerCase().indexOf(sRev) != -1) { nSeqNumber++; nSeqChar++;}
		}

		/* Check for sequential symbol string patterns (forward and reverse) */
		for (var s=0; s < 8; s++) {
			var sFwd = sSymbols.substring(s,parseInt(s+3));
			var sRev = sFwd.strReverse();
			if (pwd.toLowerCase().indexOf(sFwd) != -1 || pwd.toLowerCase().indexOf(sRev) != -1) { nSeqSymbol++; nSeqChar++;}
		}

		/* Modify overall score value based on usage vs requirements */

		/* General point assignment */
		if (nAlphaUC > 0 && nAlphaUC < nLength) {
			nScore = parseInt(nScore + ((nLength - nAlphaUC) * 2));
			sAlphaUC = "+ " + parseInt((nLength - nAlphaUC) * 2);
		}
		if (nAlphaLC > 0 && nAlphaLC < nLength) {
			nScore = parseInt(nScore + ((nLength - nAlphaLC) * 2));
			sAlphaLC = "+ " + parseInt((nLength - nAlphaLC) * 2);
		}
		if (nNumber > 0 && nNumber < nLength) {
			nScore = parseInt(nScore + (nNumber * nMultNumber));
			sNumber = "+ " + parseInt(nNumber * nMultNumber);
		}
		if (nSymbol > 0) {
			nScore = parseInt(nScore + (nSymbol * nMultSymbol));
			sSymbol = "+ " + parseInt(nSymbol * nMultSymbol);
		}
		if (nMidChar > 0) {
			nScore = parseInt(nScore + (nMidChar * nMultMidChar));
			sMidChar = "+ " + parseInt(nMidChar * nMultMidChar);
		}

		/* Point deductions for poor practices */
		if ((nAlphaLC > 0 || nAlphaUC > 0) && nSymbol === 0 && nNumber === 0) {  // Only Letters
			nScore = parseInt(nScore - nLength);
			nAlphasOnly = nLength;
			sAlphasOnly = "- " + nLength;
		}
		if (nAlphaLC === 0 && nAlphaUC === 0 && nSymbol === 0 && nNumber > 0) {  // Only Numbers
			nScore = parseInt(nScore - nLength);
			nNumbersOnly = nLength;
			sNumbersOnly = "- " + nLength;
		}
		if (nRepChar > 0) {  // Same character exists more than once
			nScore = parseInt(nScore - nRepInc);
			sRepChar = "- " + nRepInc;
		}
		if (nConsecAlphaUC > 0) {  // Consecutive Uppercase Letters exist
			nScore = parseInt(nScore - (nConsecAlphaUC * nMultConsecAlphaUC));
			sConsecAlphaUC = "- " + parseInt(nConsecAlphaUC * nMultConsecAlphaUC);
		}
		if (nConsecAlphaLC > 0) {  // Consecutive Lowercase Letters exist
			nScore = parseInt(nScore - (nConsecAlphaLC * nMultConsecAlphaLC));
			sConsecAlphaLC = "- " + parseInt(nConsecAlphaLC * nMultConsecAlphaLC);
		}
		if (nConsecNumber > 0) {  // Consecutive Numbers exist
			nScore = parseInt(nScore - (nConsecNumber * nMultConsecNumber));
			sConsecNumber = "- " + parseInt(nConsecNumber * nMultConsecNumber);
		}
		if (nSeqAlpha > 0) {  // Sequential alpha strings exist (3 characters or more)
			nScore = parseInt(nScore - (nSeqAlpha * nMultSeqAlpha));
			sSeqAlpha = "- " + parseInt(nSeqAlpha * nMultSeqAlpha);
		}
		if (nSeqNumber > 0) {  // Sequential numeric strings exist (3 characters or more)
			nScore = parseInt(nScore - (nSeqNumber * nMultSeqNumber));
			sSeqNumber = "- " + parseInt(nSeqNumber * nMultSeqNumber);
		}
		if (nSeqSymbol > 0) {  // Sequential symbol strings exist (3 characters or more)
			nScore = parseInt(nScore - (nSeqSymbol * nMultSeqSymbol));
			sSeqSymbol = "- " + parseInt(nSeqSymbol * nMultSeqSymbol);
		}

		/* Determine complexity based on overall score */
		if (nScore > 100) { nScore = 100; } else if (nScore < 0) { nScore = 0; }
		if (nScore >= 0 && nScore < 20) { sComplexity = "<#AiProtection_scan_rDanger#>"; }
		else if (nScore >= 20 && nScore < 40) { sComplexity = "<#PASS_score1#>"; }
		else if (nScore >= 40 && nScore < 60) { sComplexity = "<#PASS_score2#>"; }
		else if (nScore >= 60 && nScore < 80) { sComplexity = "<#PASS_score3#>"; }
		else if (nScore >= 80 && nScore <= 100) { sComplexity = "<#PASS_score4#>"; }

		/* Display updated score criteria to client */
		$('#scorebarBorder'+postfix).css("display", "block");
		oScorebar.style.backgroundPosition = parseInt(nScore) + "%";
		oScorebar.innerHTML = sComplexity;
	}
	else{
		chkPass("", " ", idx);
	}
}

function check_password_length(obj){

	var password = obj.value;
	var httpPassInput = $("#http_passwd");

        if(isSku("KR") || isSku("SG") || isSku("AA")){     /* MODELDEP by Territory Code */
		httpPassInput.showTextHint("");
		return;
	}

	if(password.length > systemVariable.maxPasswordLen){
		httpPassInput.showTextHint("<#JS_max_password#>");
		obj.focus();
	}
	else if(password.length > 0 && password.length < 5){
		httpPassInput.showTextHint("<#JS_short_password#> <#JS_password_length#>");
		obj.focus();
	}
	else{
		httpPassInput.showTextHint("");
	}
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

function check_dwb_ssid(){
	var confilct_flag = false;
	var dwb_band = httpApi.nvramGet(["dwb_band"]).dwb_band;
	var $dwbObj = $("#wireless_ssid_" + dwb_band + "");
	if($dwbObj.length == "1"){
		var backhaul_ssid = $dwbObj.val();
		if(backhaul_ssid != ""){
			var confilct_flag = false;
			$(".wlInput").each(function(index, item) {
				var item_id = $(item).attr("id");
				if(item_id.indexOf("wireless_ssid_") >= 0 && item_id != $dwbObj.attr("id")){
					if($(item).val() == backhaul_ssid){
						var band_info = getAllWlArray().filter(function(item, index, array){return (item.ifname == dwb_band);})[0];
						var band_text = (band_info != undefined) ? band_info.title : "";
						var hint = "Use a different Network Name (SSID) for your #WIRELESSBAND band.".replace("#WIRELESSBAND", band_text);
						$dwbObj.showTextHint(hint);
						confilct_flag = true;
						return false;
					}
				}
			});
		}
	}
	return confilct_flag;
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

function showDiableDHCPclientID(clientid_enable){
	if(clientid_enable.checked) {
		$('#wan_clientid').val("")
						  .attr('disabled', true)
						  .hide();
	}
	else {
		$('#wan_clientid').val("")
		                  .attr('disabled', false)
		                  .show();
	}
}

function set_state_info(state){
	        switch(state) {
                case "down":
                        $("#LED_state").html("<#adsl_link_sts_itemname#> : Link down (DSL LED Off)<br>");
                        $("#LED_state").show();
                        break;
                case "wait":
                case "wait for init":
                        $("#LED_state").html("<#adsl_link_sts_itemname#> : Wait for init (DSL LED Flashing)<br>");
                        $("#LED_state").show();
                        break;
                case "init":
                case "initializing":
                        $("#LED_state").html("<#adsl_link_sts_itemname#> : Initializing (DSL LED Flashing)<br>");
                        $("#LED_state").show();
                        break;
                case "up":
                        $("#LED_state").html("<#adsl_link_sts_itemname#> : Link up (DSL LED On)<br>");
                        $("#LED_state").show();
                        break;
                default:
                        $("#LED_state").hide();
                        break;
        	}
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

var Get_Component_AiMeshOnboarding_List = function(nodeInfo) {
	var nodeContainer = $("<div>").attr({"id" : nodeInfo.id}).addClass("apListContainer apProfile");
	var nodeDiv = $("<div>").addClass("apListDiv");
	nodeContainer.append(nodeDiv);
	var model_info = {"model_name": nodeInfo.name, "tcode": nodeInfo.tcode, "cobrand": nodeInfo.cobrand, "icon_model_name": ""};
	var cloudModelName = httpApi.transformCloudModelName(model_info);

	var model_icon_container = $("<div>").addClass("ap_icon_container middle");
	nodeDiv.append(model_icon_container);

	var model_icon = $("<div>").addClass("aimesh_icon").attr("model_name", cloudModelName);
	if(systemVariable.modelCloudIcon[cloudModelName])
		model_icon.css("background-image", "url(" + systemVariable.modelCloudIcon[cloudModelName] + ")");
	model_icon.appendTo(model_icon_container);

	var node_name_container = $("<div>").addClass("ap_ssid");
	var node_name = $("<div>").html(handle_ui_model_name(nodeInfo.name, nodeInfo.ui_model_name));
	var labelMac = nodeInfo.mac;
	httpApi.getAiMeshLabelMac(nodeInfo.name, nodeInfo.mac,
		function(_callBackMac){
			labelMac = _callBackMac;
		}
	);
	var node_mac = $("<div>").addClass("aimesh_mac").html(labelMac);
	node_name.appendTo(node_name_container);
	node_mac.appendTo(node_name_container);

	nodeDiv.append(node_name_container);
	node_name_container.hover(function(){
		$(this).addClass("ap_ssid_hover");
		$(this).find(".aimesh_mac").addClass("ap_ssid_hover");
	}, function(){
		$(this).removeClass("ap_ssid_hover")
		$(this).find(".aimesh_mac").removeClass("ap_ssid_hover");
	})

	var band_icon_container = $("<div>").addClass("ap_icon_container middle");
	nodeDiv.append(band_icon_container);
	var band_icon = $("<div>");
	if(nodeInfo.source == "2"){
		if(nodeInfo.type != undefined && nodeInfo.type == "65536")
			band_icon.addClass("aimesh_band_icon icon_plc");
		else
			band_icon.addClass("aimesh_band_icon icon_wired");
	}
	else
		band_icon.addClass("icon_wifi_" + nodeInfo.signal + " aimesh_band_icon");
	band_icon.appendTo(band_icon_container);

	var ap_narrow_container = $("<div>").addClass("ap_narrow_container");
	nodeDiv.append(ap_narrow_container);

	var ap_narrow = $("<div>").addClass("icon_arrow_right ap_narrow");
	ap_narrow.appendTo(ap_narrow_container);

	return nodeContainer;
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
		if(systemVariable.isDefault && !systemVariable.keepDefpsk){
			wirelessAP["wl" + wl.ifname + "_ssid"] = "";
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
					console.log(isSupport('wifi6e'));
					
					if(isSupport('quadband')){
						ssid_tmp = ssid_tmp.slice(0,26) + "_RPT5G";
					}
					else{
						ssid_tmp = ssid_tmp.slice(0,28) + "_RPT";
					}

					break;
				case 1 :
					
					if(isSupport('quadband')){
						ssid_tmp = ssid_tmp.slice(0,26) + "_RPT5G2";
					}
					else{
						ssid_tmp = ssid_tmp.slice(0,26) + "_RPT5G";
					}

					break;
				case 2 :
					
					if(isSupport('quadband')
					|| isSupport('triband') && isSupport('wifi6e')){
						ssid_tmp = ssid_tmp.slice(0,26) + "_RP6G";
					}
					else{
						ssid_tmp = ssid_tmp.slice(0,25) + "_RPT5G2";
					}	
					
					break;
				case 3 :
					ssid_tmp = ssid_tmp.slice(0,25) + "_RPT";
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
					"maxlength": "33",
					"class": "textInput wlInput",
					"autocomplete": "off",
					"autocorrect": "off",
					"autocapitalize": "off",
					"spellcheck": "false",
					"data-role": "none",
					"data-clear-btn": "true"
				})
				.keyup(function(e){
					if($(e.currentTarget).attr("id").indexOf(get_wl_unit_by_band("2G")) == -1){
						$("#wireless_sync_checkbox").enableCheckBox(false);
					}

					if(e.keyCode == 13){
						$(".wlInput")[idx*2+1].focus();
					}

					validator.ssidCheck($("#"+this.id));
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
					if($(e.currentTarget).attr("id").indexOf(get_wl_unit_by_band("2G")) == -1){
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
					chkPass("WiFi", this.value, wl.ifname);
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
	var iptvSettings = httpApi.hookGet("get_iptvSettings");
	var isp_profiles = iptvSettings.isp_profiles;
	var original_switch_wantag = httpApi.nvramGet(["switch_wantag"]).switch_wantag;
	var text = "";
	var selected = false;
	var found = false;

	systemVariable.ispProfiles = isp_profiles;
	systemVariable.ispPortDefinitions = iptvSettings.port_definitions;
	$.each(isp_profiles, function(i, isp_profile) {
		text = isp_profile.profile_name;

		if(text == "none")
			text = "<#wl_securitylevel_0#>";
		else if(text == "manual")
			text = "<#Manual_Setting_btn#>";

		if(isp_profile.switch_wantag == original_switch_wantag){
			selected = true;
			found = true;
		}
		else
			selected = false;

		isp_select
			.append($("<option></option>")
			.attr("value",isp_profile.switch_wantag)
			.text(text)
			.attr("selected", selected));
	});

	if(!found){
		if(original_switch_wantag != ""){
			isp_select.val("manual");
		}
		else
			isp_select.val("none");
	}

	return isp_select;
}

function getCloudProfiles(){
	$.getJSON("https://nw-dlcdnet.asus.com/plugin/js/iptv_profile.json",
		function(data){
			Object.keys(data).forEach(function(profile_name) {
				var newProfile = {};

				systemVariable.cloudIspProfiles.push(data[profile_name]);
				newProfile.profile_name = profile_name;
				if(data[profile_name].iptv_port == "IPTV_PORT")
					newProfile.iptv_port = systemVariable.ispPortDefinitions.IPTV_PORT;
				else if(data[profile_name].iptv_port == "MSTB_PORT")
					newProfile.iptv_port = systemVariable.ispPortDefinitions.MSTB_PORT;
				else
					newProfile.iptv_port = "";

				if(data[profile_name].voip_port == "VOIP_PORT")
					newProfile.voip_port = systemVariable.ispPortDefinitions.VOIP_PORT;
				else
					newProfile.voip_port = "";

				if(data[profile_name].bridge_port == "IPTV_PORT")
					newProfile.bridge_port = systemVariable.ispPortDefinitions.IPTV_PORT;
				else if(data[profile_name].bridge_port == "VOIP_PORT")
					newProfile.bridge_port = systemVariable.ispPortDefinitions.VOIP_PORT;
				else
					newProfile.bridge_port = "";
				newProfile.iptv_config = data[profile_name].iptv_config;
				newProfile.voip_config = data[profile_name].voip_config;
				newProfile.switch_wantag = data[profile_name].switch_wantag;
				newProfile.switch_stb_x = data[profile_name].switch_stb_x;
				newProfile.mr_enable_x = data[profile_name].mr_enable_x;
				newProfile.emf_enable = data[profile_name].emf_enable;
				systemVariable.ispProfiles.push(newProfile);
				systemVariable.ispProfiles.sort(function(a, b){
									if((a.switch_wantag < b.switch_wantag) || a.switch_wantag == "none" || b.switch_wantag == "manual")
										return -1;
									if( a.switch_wantag > b.switch_wantag )
										return 1;

									return 0;
								});
				$("#switchWanTagContainer").empty();
				$("#switchWanTagContainer").append(Get_Component_ISPSelect);
				$("#switch_wantag").attr("class", "optionInput");
			});
		}
	);
}

function is_cloud_profile(isp){
	var found = false;
	$.each(systemVariable.cloudIspProfiles, function(i, cloud_isp_profile) {
		if(cloud_isp_profile.switch_wantag == isp){
			found = true;
			return false;
		}
	});

	return found;
}

function get_cloud_settings(isp){
	var cloud_profile = {};
	$.each(systemVariable.cloudIspProfiles, function(i, cloud_isp_profile) {
		console.log("cloud_isp_profile.switch_wantag = "+cloud_isp_profile.switch_wantag);
		if(cloud_isp_profile.switch_wantag == isp){
			cloud_profile = cloud_isp_profile;
			return false;
		}
	});

	return cloud_profile;
}

function isEmpty(obj)
{
	for (var name in obj){
		return false;
	}

	return true;
};

function installPages(flag){
	switch(flag){
		case "amasAddNodePages":
			$("#amassearch_page").load("/mobile/pages/amassearch_page.html");
			$("#amasonboarding_page").load("/mobile/pages/amasonboarding_page.html");
		break;
	}
}

function handleSysDep(){
	var isNoWAN = (isSupport("dsl"))?(httpApi.detDSLwanGetRet().wanType == 'NOWAN'):(httpApi.detwanGetRet().wanType == 'NOWAN');
	var amas_bdl_num = parseInt(httpApi.nvramGet(["amas_bdl"]).amas_bdl);

	$(".amasSupport").toggle(isSupport("amas"));
	$(".noAmasSupport").toggle(!isSupport("amas"));
	$(".tosSupport").toggle(systemVariable.isDefault && isSupport("QISBWDPI"));
	$(".repeaterSupport").toggle(isSupport("repeater"));
	$(".pstaSupport").toggle(isSupport("psta"));
	$(".dualbandSupport").toggle(isSupport("dualband") || isSupport("triband") || isSupport("quadband"));
	$(".v6plus").toggle(isSupport("s46"));
	$(".ocnvc").toggle(isSupport("s46") && isSupport("ocnvc"));
	$(".vpnClient").toggle(isSupport("VPNCLIENT"));
	$(".iptv").toggle(isSupport("IPTV"));
	$(".defaultSupport").toggle(systemVariable.isDefault);
	$(".configuredSupport").toggle(!systemVariable.isDefault);
	//$(".forceUpgrade").toggle(isSupport("fupgrade")); 
	$(".routerSupport").toggle(!isSupport("noRouter"));
	$(".apSupport").toggle(!isSupport("noAP"));

	if(systemVariable.forceChangePw){
		systemVariable.forceChangePw = false;
		systemVariable.forceChangePwInTheEnd = true;
	}

	if(!isNoWAN || !isSupport("amasNode")) $(".amasNoWAN").remove();
	if(!isSupport("amas")) $(".amasSupport").remove();
	if(isSupport("amas") && isSupport("amas_bdl")){
		$("#amassearch_page").load("/mobile/pages/amassearch_page.html");
		$("#amasonboarding_page").load("/mobile/pages/amasonboarding_page.html");
	}

	if(isSupport("prelink") && amas_bdl_num > 2){
		if(amas_bdl_num == 3)
			$("#product_location").attr("src", "/images/product_location_3pack.png")
	}

	if((isGundam() || isKimetsu() || isEva()) && !$('.GD-head').length){
		if(isGundam()){
			$('head').append('<link rel="stylesheet" type="text/css" href="/css/gundam.css">');
		}
		else if(isKimetsu()){
			$('head').append('<link rel="stylesheet" type="text/css" href="/css/kimetsu.css">');
		}
		else if(isEva()){
			$('head').append('<link rel="stylesheet" type="text/css" href="/css/eva.css">');
			$('body').addClass("body_eva");
		}
		
		$("#summary_page").append($("<div>").attr({"id": "gdContainer", "class": "gundam-footer-field"}).hide())
		$("#summary_page").append($("<div>").attr({"id": "gd-logo", "class": "icon_logo-1"}).show())
		$("#gdContainer").html('');
		if(systemVariable.productid == 'GT-AX11000'){
			$("#gdContainer").append($("<div>").attr({"class": "GD-head-1"}))
		}
		else{
			$("#gdContainer").append($("<div>").attr({"class": "GD-head"}))
		}
		
		$("#gdContainer").append($("<div>").attr({"class": "GD-wait"}).append($("<div>").attr({"id": "GD-status"}).html("Installing..")))
		$("#gdContainer").append($("<div>").attr({"class": "GD-footer-left"}))
		$("#gdContainer").append($("<div>").attr({"class": "GD-footer-extend"}))
		$("#gdContainer").append($("<div>").attr({"class": "GD-footer-center"}))
		$("#gdContainer").append($("<div>").attr({"class": "GD-footer-extend eva-footer-extend_right"}))
		$("#gdContainer").append($("<div>").attr({"class": "GD-footer-right"}))
		$("#gdContainer").append($("<div>").attr({"class": "GD-law-label"}))
		// systemVariable.imgContainer = [];

		if(isGundam()){
			var gdImagesList = ["-head", "_bg", "_bg_bottom01", "_bg_bottom02", "_bg_bottom03", "_bg_bottom04", "_lawlabel", "-logo", "-waiting"]
			gdImagesList.forEach(function(imageUrl){
				var tmpObj = new Image();
				tmpObj.src = "/images/gundam" + imageUrl + ".png";
				// systemVariable.imgContainer.push(tmpObj)
			})
		}
		else if(isKimetsu()){
			var gdImagesList = ["-head", "_bg", "_bg_bottom02", "_lawlabel", "-logo", "-waiting"]
			gdImagesList.forEach(function(imageUrl){
				var tmpObj = new Image();
				tmpObj.src = "/images/kimetsu_no_yaiba" + imageUrl + ".png";
				// systemVariable.imgContainer.push(tmpObj)
			})
		}
		else if(isEva()){
			var gdImagesList = ["-head", "_bg", "_bg_bottom01", "_bg_bottom02", "_bg_bottom03", "_bg_bottom04", "_lawlabel", "-logo", "-waiting"]
			gdImagesList.forEach(function(imageUrl){
				var tmpObj = new Image();
				tmpObj.src = "/images/eva" + imageUrl + ".png";
				// systemVariable.imgContainer.push(tmpObj)
			})
		}
		
	}
}

function handleModelIcon() {
	var modelInfo = httpApi.nvramGet(["territory_code", "productid", "odmpid", "color", "rc_support", "CoBrand"], true);
	var ttc = modelInfo.territory_code;
	var CoBrand = modelInfo.CoBrand;
	var isGundam = (CoBrand == 1 || ttc.search('GD') == '0');
	var isKimetsu = (CoBrand == 2);
	var isEVA = (CoBrand == 3);
	var based_modelid = modelInfo.productid;
	var odmpid = modelInfo.odmpid;
	var color = modelInfo.color.toUpperCase();
	var odm_support = isSupport("odm");
	if(isSupport("cobrand_change")){
		$('#ModelPid_img').css('background-image', 'url("/images/Model_product.png")');
	}
	else{
		$('#ModelPid_img').css('background-image', 'url(' + function() {
			var LinkCheck = function(url) {
				var http = new XMLHttpRequest();
				http.open('HEAD', url, false);
				http.send();
				return http.status!="404";
			};

			var update_color = function() {
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
			var MP_png_path = default_png_path;

			if(update_color().length > 0) {
				MP_png_path = "/images/Model_product_"+ update_color() +".png";
			}
			else if(odmpid.length > 0 && odmpid != based_modelid) {
				if(odmpid == "RT-AC66U_B1" || odmpid == "RT-AC1750_B1" || odmpid == "RT-N66U_C1" || odmpid == "RT-AC1900U" || odmpid == "RT-AC67U")
					MP_png_path = "/images/RT-AC66U_V2/Model_product.png";
				else if(odmpid == "RP-AC1900")
					MP_png_path = "/images/RP-AC1900/Model_product.png";
				else if(odmpid == "RT-AX86S")
					MP_png_path = "/images/Model_product_rt-ax86s.png";
			}

			if(isGundam){
				MP_png_path = "/images/Model_product_GD.png";
			}
			else if(isKimetsu){
				MP_png_path = "/images/Model_product_KNY.png";
			}
			else if(isEVA){
				MP_png_path = "/images/Model_product_EVA.png";
			}

			if(odm_support){
				MP_png_path = "/images/Model_product_COD.png";
			}

			if(LinkCheck(MP_png_path))
				return MP_png_path;
			else
				return default_png_path;

		}() + ')');
	}

	if(odmpid == 'RT-AX86S'){
		$("#resetModem").removeClass('unplug').addClass("unplug-ax86s");
		$("#noWanPic").attr("src", "/images/WANunplug_rt-ax86s.png");
		$("#noWanEth").attr("src", "/images/WANunplug_eth_rt-ax86s.png");
		$("#noWanUsb").attr("src", "/images/WANunplug_usb_rt-ax86s.png");
	}
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

function handle_ui_model_name(_model_name, _ui_model_name){
	var result = "";
	if(_ui_model_name == undefined || _ui_model_name == "")
		result = _model_name;
	else
		result = _ui_model_name;
	return result;
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
			if(authentication === 'WPA2-Personal'){
				qisPostData["wlc" + unit + "_auth_mode"] = "psk2";
			}
			
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
		else if(authentication == "WPA3-Personal" && encryption == "AES"){
			qisPostData["wlc" + unit + "_auth_mode"] = "sae";
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

function setupFronthaulNetwork(_smart_connect){
	if(isSupport("FRONTHAUL_NETWORK")){
		var dwb_mode = httpApi.nvramGet(["dwb_mode"]).dwb_mode;
		postDataModel.insert(fronthaulNetworkObj);

		switch(parseInt(_smart_connect)){
			case 0:
			case 2:
			case 3:
				qisPostData.fh_ap_enabled = "0";
				break;
			case 1:
				qisPostData.fh_ap_enabled = httpApi.nvramDefaultGet(["fh_ap_enabled"]).fh_ap_enabled;
				break;
		}

		switch(parseInt(qisPostData.fh_ap_enabled)){
			case 0:
				if(isSupport("amas_bdl")){
					qisPostData.acs_unii4 = "1";
				}
				else{
					qisPostData.acs_unii4 = "0";
				}

				break;
			case 1:
				qisPostData.acs_unii4 = "0";
				break;
			case 2:
				qisPostData.acs_unii4 = "0";
				break;
		}

		if(parseInt(_smart_connect) == 0) delete qisPostData.acs_unii4;
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
	var current_webs_chg_sku = (httpApi.nvramGet(["webs_chg_sku"], true).webs_chg_sku=="1")? true:false;
	var current_webs_SG_mode = (httpApi.nvramGet(["webs_SG_mode"], true).webs_SG_mode=="1")? true:false;

	if(isWANChanged()){
		actionScript.push("restart_wan_if " + systemVariable.ethWanIf);
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

	if(qisPostData.hasOwnProperty("ipv6_service")){
		actionScript.push("restart_net");	
	}

	if(
		qisPostData.hasOwnProperty("wl0_ssid") || 
		qisPostData.hasOwnProperty("wl0.1_ssid") || 
		qisPostData.hasOwnProperty("wl0_11ax") || 
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

	if(isSwModeChanged() && isSwMode("RT")){
		return "restart_all";
	}

	if(isSupport("2p5G_LWAN") || isSupport("10G_LWAN") || isSupport("10GS_LWAN") || Object.keys(systemVariable.eth_wan_list).length > 1){
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

	if(current_webs_chg_sku){
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
			Object.keys(obj).every(function(key){
				if(qisPostData.hasOwnProperty(key)){
					return false;
				}
				else if(obj[key] === "")
					queryArray.push(key);
				else
					qisPostData[key] = obj[key];

				return true;
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

	if(Object.keys(systemVariable.eth_wan_list).length > 1){
		Object.keys(systemVariable.eth_wan_list).forEach(function(eth_wan){
			var wan_obj = systemVariable.eth_wan_list[eth_wan];
			if(wan_obj.hasOwnProperty("extra_settings")){
				var extra_settings = wan_obj.extra_settings;
				$.each(extra_settings, function(key) {
					if(qisPostData.hasOwnProperty(key)){
						if(qisPostData[key] != systemVariable[key]){
							isChanged = true;
						}
					}
				});
			}
		});
	}
	return isChanged;
};

var isPage = function(page){
	return $("#" + page).is(":visible");
}

var isSupport = function(_ptn){
	var ui_support = JSON.parse(JSON.stringify(httpApi.hookGet("get_ui_support")));
	var modelInfo = httpApi.nvramGet(["productid", "odmpid"]);
	var based_modelid = modelInfo.productid;
	var odmpid = modelInfo.odmpid;
	var matchingResult = false;
	var amas_bdlkey = httpApi.nvramGet(["amas_bdlkey"]).amas_bdlkey;

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
			matchingResult = (isSku("US") || isSku("CA") || isSku("TW") || isSku("CN") || isSku("CT") || isSku("GD") || isSku("TC")) ? false : true;
			break;
		case "IPTV":
			matchingResult = (isSku("US") || isSku("CN") || isSku("CT") || isSku("GD") || isSku("TC") || isSku("CA")|| isSku("U2") || isSku("TW")) ? false : true;
			break;
		case "ENABLE_IPv6":
			matchingResult = (isSku("CN") || isSku("GD") || isSku("TC")) ? true : false;
			break;
		case "SMARTCONNECT":
			matchingResult = (ui_support["smart_connect"] == "1" || ui_support["smart_connect"] == "2" || ui_support["bandstr"] == "1") ? true : false;
			break;
		case "GUNDAM_UI":
			matchingResult = ((isGundam() || isKimetsu() || isEva()) && $(".desktop_left_field").is(":visible")) ? true : false;
			break;
		case "amas_bdl":
			matchingResult = (ui_support["amas_bdl"] >= 1 && amas_bdlkey.length != 0) ? true : false;
			break;
		case "FRONTHAUL_NETWORK":
			matchingResult = ui_support["amas_fronthaul_network"] || false;
			break;
		case "boostkey": // force remove boostkey from qis
			matchingResult = false;
			break;
		case "MB_mode_concurrep":
			if(isSwMode("MB") && (ui_support["concurrep"] == 1) && odmpid != "RP-AC1900" && based_modelid != 'RP-AX56' && based_modelid != 'RP-AX58')
				matchingResult = true;
			else
				matchingResult = false;
			break;
		default:
			matchingResult = ((ui_support[_ptn] > 0) || (systemVariable.productid.search(_ptn) !== -1)) ? true : false;
			break;
	}

	return matchingResult;
}

var getSupportNum = function(_ptn){
	var ui_support = httpApi.hookGet("get_ui_support");
	return ui_support[_ptn];
}

var isGundam = function(){
	return (isSku("GD") || systemVariable.CoBrand == "1");
}
var isKimetsu = function(){
	return (systemVariable.CoBrand == "2");
}
var isEva = function(){
	return (systemVariable.CoBrand == "3");
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
	var CheckTime = "";
	var TimeDiff = "";
	if(!systemVariable.linkInternet){
		setTimeout(arguments.callee, 1000);
	}
	else{
		var NowTime = Math.ceil(Date.now() / 1000);
		httpApi.nvramSet({"action_mode":"apply", "webs_update_trigger":"QIS", "rc_service":"start_webs_update"}, function(){
			setTimeout(function(){

				var fwInfo = httpApi.nvramGet(["webs_state_update", "webs_state_info", "webs_state_flag", "webs_state_level", "webs_update_ts", "apps_sq"], true);
				CheckTime = (fwInfo.webs_update_ts == "")? 0:fwInfo.webs_update_ts.split("&#62")[0];
				TimeDiff = NowTime-CheckTime;
				
				if(fwInfo.webs_state_flag == "1" || fwInfo.webs_state_flag == "2"){
					systemVariable.isNewFw = fwInfo.webs_state_flag;
					systemVariable.newFwVersion = fwInfo.webs_state_info;
					systemVariable.forceLevel = fwInfo.webs_state_level;
				}

				if(TimeDiff > 1800 || fwInfo.webs_state_update != "1"){
					setTimeout(arguments.callee, 1000);
				}
				else{
					httpApi.log("fwInfo", JSON.stringify(fwInfo), systemVariable.qisSession)
				}
			}, 2000);
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

	if(str.charAt(0) == '"'){
		testResult.isError = true;
		testResult.errReason = '<#JS_validstr1#> ["]';
                return testResult;
	}
	else if(str.charAt(str.length - 1) == '"'){
		testResult.isError = true;
		testResult.errReason = '<#JS_validstr3#> ["]';
		return testResult;
	}
	else{
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
	}
};

validator.KRSkuPwd = function(str){
	var testResult = {
		'isError': false,
		'errReason': ''
	}

	if( !/[A-Za-z]/.test(str) || !/[0-9]/.test(str) || str.length < 10
		|| !/[\!\"\#\$\%\&\'\(\)\*\+\,\-\.\/\:\;\<\=\>\?\@\[\\\]\^\_\`\{\|\}\~]/.test(str)
		|| /([A-Za-z0-9\!\"\#\$\%\&\'\(\)\*\+\,\-\.\/\:\;\<\=\>\?\@\[\\\]\^\_\`\{\|\}\~])\1/.test(str)
	){
		testResult.isError = true;
		testResult.errReason = "<#JS_validLoginPWD#>";
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

var transformWLCObj = function(){
	var wlcUnit = systemVariable.multiPAP.wlcOrder[0];
	Object.keys(qisPostData).forEach(function(key){
		qisPostData[key.replace("wlc" + wlcUnit, "wlc")] = qisPostData[key];
	});

	postDataModel.remove(wlcMultiObj["wlc" + wlcUnit]);
};
var copyWLCObj_wlc1ToWlc2 = function(){
	var wlcPostData = wlcMultiObj.wlc2;
	$.each(wlcPostData, function(item){wlcPostData[item] = qisPostData[item.replace("2", "1")];});
	postDataModel.insert(wlcPostData);
	qisPostData.wlc2_band = 2;
};

var transformWLToGuest = function(){
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

var handleWirelessClientSSID = function(_wlArray, _autoStr){
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

var isAllPAPSet = function(){
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

var genPAPList = function(papList, filterBand){
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

var genWLBandOption = function(){
	$("#wlc_band_manual").find("option").remove();
	if($("#wlc_band_manual").has("option").length == 0){
		var wlArray = Get_Value_Available_WL_Band();
		wlArray.forEach(function(band){
			$("#wlc_band_manual").append($("<option>").val(band).html(getAllWlArray()[band].title));
		});
	}
};

var handleWLWepOption = function(authMode){
	if(authMode == "open"){
		$("#wlc_wep_manual option[value='0']").show();
		$("#wlc_wep_manual option[value='0']").prop("selected", true).change();
	}
	else if(authMode == "shared"){
		$("#wlc_wep_manual option[value='0']").hide();
		$("#wlc_wep_manual option[value='1']").prop("selected", true).change();
	}
};

var handleWLAuthModeItem = function(){
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
	else if(auth_mode == "sae"){
		$("#manual_pap_setup-crypto").show();
		$("#manual_pap_setup-key").show();
		$("#wlc_crypto_manual option[value='tkip']").remove();
	}
};

var clearIntervalStatus = function(){
	if(systemVariable.interval_status != false){
		clearInterval(systemVariable.interval_status);
		systemVariable.interval_status = false;
	}
};

function site2site_handle_wlSet(){
	var wl_text_mapping = [{"idx":"wl_2G", "text":"2.4 GHz"}, {"idx":"wl_5G", "text":"5 GHz"}];
	var select_none_num = 0;
	var hide_wl_none = false;
	var cur_wl_idx = "";
	$.each(systemVariable.site2site_wl, function(wl_idx, wl_item){
		if(cur_wl_idx == ""){
			if(!wl_item.confirm){
				cur_wl_idx = wl_idx;
			}
		}

		if(cur_wl_idx != wl_idx){
			if(wl_item.confirm){
				if(wl_item.select == "wl_none")
					select_none_num++;
			}
		}
	});
	if(cur_wl_idx == ""){
		goTo.site2site_Finish();
		return;
	}
	if(Object.keys(systemVariable.site2site_wl).length == (select_none_num + 1))
		hide_wl_none = true;

	var specific_wl_text = wl_text_mapping.filter(function(item, index, array){
		return (item.idx == cur_wl_idx);
	})[0];
	if(specific_wl_text != undefined){
		$("#site2site_wlSet #s2s_wlSet_desc").html(specific_wl_text.text + " <#Guest_Network#>");
	}
	else{
		$("#site2site_wlSet #s2s_wlSet_desc").html("<#Guest_Network#>");
	}

	var $wl_objs = $("#site2site_wlSet").find("[data-container=wl_new], [data-container=wl_ori], [data-container=wl_none]")
		.show().unbind("click").click(function(){
			var type = $(this).attr("data-container");
			systemVariable.site2site_wl[cur_wl_idx].select = type;
			systemVariable.site2site_wl[cur_wl_idx].confirm = true;
			var all_confirm = true;
			$.each(systemVariable.site2site_wl, function(wl_idx, wl_item){
				if(!wl_item.confirm){
					all_confirm = false
					return false;
				}
			});
			if(all_confirm){
				goTo.site2site_Finish();
			}
			else{
				goTo.site2site_wlSet();
			}
		});
	var $wl_new_obj = $wl_objs.filter("[data-container=wl_new]");
	var $wl_ori_obj = $wl_objs.filter("[data-container=wl_ori]");
	var $wl_none_obj = $wl_objs.filter("[data-container=wl_none]");

	$wl_new_obj.find(".ap_ssid").html(htmlEnDeCode.htmlEncode(systemVariable.site2site_wl[cur_wl_idx].ssid_new));
	$wl_ori_obj.find(".ap_ssid").html(htmlEnDeCode.htmlEncode(systemVariable.site2site_wl[cur_wl_idx].ssid_ori));
	if(systemVariable.site2site_wl[cur_wl_idx].ssid_ori == ""){
		$wl_ori_obj.hide();
		$("#site2site_wlSet #s2s_wlSet_hint").hide();
	}
	else{
		$("#site2site_wlSet #s2s_wlSet_hint").show();
	}
	if(hide_wl_none)
		$wl_none_obj.hide();

	$("#site2site_wlSet .ap_ssid").unbind("mouseenter mouseleave").hover(function(){
		$(this).addClass("ap_ssid_hover")
	}, function(){
		$(this).removeClass("ap_ssid_hover")
	});
}
