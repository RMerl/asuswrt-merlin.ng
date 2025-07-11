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
				if(isSupport("quadband")){
					if(get_wl_unit_by_band("5G2") != ""){
						return [
							{"title":"2.4 GHz / 5 GHz-1 / 5 GHz-2", "ifname":get_wl_unit_by_band("2G")}, 
							{"title":"6 GHz", "ifname":get_wl_unit_by_band("6G")}
						];
					}
					else{
						return [
							{"title":"2.4 GHz / 5 GHz", "ifname":get_wl_unit_by_band("2G")}, 
							{"title":"6 GHz-1", "ifname":get_wl_unit_by_band("6G1")},
							{"title":"6 GHz-2", "ifname":get_wl_unit_by_band("6G2")}
						];
					}
				}
				else
					return [{"title":"2.4 GHz / 5 GHz", "ifname":get_wl_unit_by_band("2G")}, {"title":"6 GHz", "ifname":get_wl_unit_by_band("6G")}];
			})(),
			"value": (function(){
				if(isSupport("smart_connect_v2"))
					return 1;
				else
					return 3;
			})()
		},
		"SC_5g5g": {
			"wlArray": [{"title":"2.4 GHz", "ifname":get_wl_unit_by_band("2G")}, {"title":"5 GHz", "ifname":get_wl_unit_by_band("5G")}],
			"value": 2
		},
		"SC_dwb_mode": {
			"wlArray": [{"title":"2.4 GHz / 5 GHz-1", "ifname":get_wl_unit_by_band("2G")}, {"title":"5 GHz-2", "ifname": get_wl_unit_by_band("5G2")}],
			"value": 1
		},
		"SC_all": {
			"wlArray": [{"title":"", "ifname": get_wl_unit_by_band("2G")}],
			"value": 1
		}
	}

	return smartConnectTable[mode] ||  smartConnectTable["SC_all"];
}

function getSelifValue(mode){
	var val = 0;

	if(get_wl_unit_by_band("2G") != "") val += 1;
	if(get_wl_unit_by_band("5G1") != "") val += 2;
	if(get_wl_unit_by_band("5G2") != "") val += 4;

	if(mode == "SC_all"){
		if(get_wl_unit_by_band("6G1") != "") val += 8;
		if(get_wl_unit_by_band("6G2") != "") val += 16;
	}
	else if(mode == "SC_5g5g"){
		if(get_wl_unit_by_band("2G") != "") val -= 1;
	}

	return val;
}

function getAllWlArray() {
	var bands = httpApi.nvramGet(["wlnband_list"]).wlnband_list.split("&#60");
    let wlArrayRet = [];
    let wlSeperateStr = '';
    let allBandName = [];

    let bandMapping = {
        '2g1': {"title": "2.4 GHz", "suffix": ""},
        '5g1': bands.includes('5g2') ? {"title": "5 GHz-1", "suffix": "_5G-1"} : {"title": "5 GHz", "suffix": "_5G"},
        '5g2': {"title": "5 GHz-2", "suffix": "_5G-2"},
        '6g1': bands.includes('6g2') ? {"title": "6 GHz-1", "suffix": "_6G-1"} : {"title": "6 GHz", "suffix": "_6G"},
        '6g2': {"title": "6 GHz-2", "suffix": "_6G-2"},
        '60g': {"title": "60 GHz", "suffix": "_60G"}
    };

    bands.sort(function(a, b) {
        var freqA = parseFloat(a);
        var freqB = parseFloat(b);
        return freqA - freqB;
    });

    bands.forEach(bandKey => {
        if (bandMapping[bandKey]) {
            wlArrayRet.push({
                "title": bandMapping[bandKey].title,
                "ifname": get_wl_unit_by_band(bandKey.toUpperCase()),
                "suffix": bandMapping[bandKey].suffix
            });
        }
    });

    wlArrayRet.forEach(function(unit){allBandName.push(unit.title);});
    document.querySelector('label[for="wireless_checkbox"]').innerHTML = `Separate ${allBandName.join(', ')}`;

    // remove dwb_band
    if (isSupport("amas_bdl")) {
        if (!isSupport("prelink_mssid")) {
            if (isSwMode("RT") || isSwMode("AP")) {
                let prelink_unit = isSupport("prelink_unit");
                if (prelink_unit >= 0)
                    wlArrayRet.splice(prelink_unit, 1);
            }
        }
    }

    return wlArrayRet;
}

function getPAPList(siteSurveyAPList, filterType, filterValue) {
	var modelInfo = httpApi.nvramGet(["productid", "odmpid"]);
	var based_modelid = modelInfo.productid;
	var papList = [];

	var profile = function(_profile){
		var getBandWidthName = function(band, ch){
			if(band == "2G"){
				return {name: "2.4GHz", unit: get_wl_unit_by_band("2G")};
			}

			if(band == "5G"){
				if(get_wl_unit_by_band("5G2")){
					return (ch >= 36 && ch <= 64) ? {name: "5GHz-1", unit: get_wl_unit_by_band("5G1")} : {name: "5GHz-2", unit: get_wl_unit_by_band("5G2")};
				}
				else{
					return {name: "5GHz", unit: get_wl_unit_by_band("5G")};
				}
			}

			if(band == "6G"){
				if(get_wl_unit_by_band("6G2")){
					return (ch >= 1 && ch <= 129) ? {name: "6GHz-1", unit: get_wl_unit_by_band("6G1")} : {name: "6GHz-2", unit: get_wl_unit_by_band("6G2")};
				}
				else{
					return {name: "6GHz", unit: get_wl_unit_by_band("6G")};
				}
			}
		}

		if(_profile == null || _profile.length == 0)
			_profile = ["", "", "", "", "", "", "", "", "", ""];

		this.band = (function(){
			return getBandWidthName(_profile[0], _profile[2]).name;
		})();

		this.unit = (function(){
			return getBandWidthName(_profile[0], _profile[2]).unit;
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
		this.mlomacaddr = (_profile[10] && typeof _profile[10] === "string" && _profile[10].length > 2) ? _profile[10] : "";
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

	update_node_ui_model_name(jsonArray);
	return jsonArray;

	function update_node_ui_model_name(nodeList){
		if (!Array.isArray(nodeList)) return;
		let lang = httpApi.nvramGet(["preferred_lang"]).preferred_lang;
		for (let node of nodeList){
			let model_name = node["name"];
			let cobrand = httpApi.aimesh_get_misc_info(node).cobrand;
			const uiModelName = findUIModelName({
				"model": model_name,
				"lang": lang,
				"coBrand": cobrand,
			});
			if (uiModelName) {
				node["ui_model_name"] = uiModelName;
			}
		}

		function findUIModelName(params) {
			if (!params) {
				return "";
			}
			const { model, lang, coBrand } = params;
			if (!model) {
				return "";
			}
			if (!systemVariable.uiModelNameCloud[model]) return "";

			let defaultLang = null;
			let defaultCoBrand = null;

			for (let item of systemVariable.uiModelNameCloud[model]) {
				if (item.lang === lang && item.CoBrand === coBrand) {
					return item.uiModelName;
				}
				if (item.lang === "*" && item.CoBrand === coBrand) {
					defaultLang = item.uiModelName;
				}
				if (item.lang === lang && item.CoBrand === "*") {
					defaultCoBrand = item.uiModelName;
				}
				if (item.lang === "*" && item.CoBrand === "*") {
					defaultLang = item.uiModelName;
				}
			}

			if (defaultCoBrand) {
				return defaultCoBrand;
			}

			if (defaultLang) {
				return defaultLang;
			}

			return "";
		}
	}
}

function getProcessPercentage(_start, _current, _timeout, _percentage){
	var percentage = 0;
	var interval = parseInt(_current) - parseInt(_start);
	var denominator = parseInt(_timeout) / parseInt(_percentage);
	percentage = isNaN(Math.round( interval / denominator )) ? 0 : Math.round( interval / denominator );
	if(percentage > 100)
		percentage = 100;
	return percentage;
}

function get_wl_unit_by_band(_band){
	if(_band == undefined) return "";

	_band = (_band).toString().toUpperCase();
	const wlnband_list = `<% nvram_get("wlnband_list"); %>`.toUpperCase().split("&#60");

	const bandMap = {
		"2G": "2G1",
		"5G": "5G1",
		"6G": "6G1"
	};

	_band = bandMap[_band] || _band;

	return findWLUnit(wlnband_list, _band).toString();

	function findWLUnit(array, band) {
		const idx = $.inArray(band, array);
		return idx !== -1 ? idx : '';
	}
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

function chkPass(pwd, flag, obj, id) {
	var ttc = systemVariable.territoryCode;
	var isSku = function(_ptn){
		return (ttc.search(_ptn) == -1) ? false : true;
	}
	var orig_pwd = "";
	var postfix = (id == undefined)? "": ("_" + id);
	var oScorebarBorder = document.getElementById("scorebarBorder"+postfix);
	var oScorebar = document.getElementById("scorebar"+postfix);
	var oScore = document.getElementById("score"+postfix);

	if(obj != undefined && (typeof obj == "object")){
		oScorebarBorder = $(obj)[0];
		oScorebar = $(obj).find(".strength_color")[0];
		oScore =$(obj).find(".strength_text")[0];
	}

	if(flag == "http_passwd" && (isSku("KR") || isSku("SG") || isSku("AA") || isSupport("secure_default"))){
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
		if(typeof document.forms[0] == "undefined" || (typeof document.forms[0] != "undefined" && document.form.current_page.value != "AiProtection_HomeProtection.asp")){
			if (nScore >= 0 && nScore < 20) { sComplexity = "<#AiProtection_scan_rDanger#>"; }
			else if (nScore >= 20 && nScore < 40) { sComplexity = "<#PASS_score1#>"; }
			else if (nScore >= 40 && nScore < 60) { sComplexity = "<#PASS_score2#>"; }
			else if (nScore >= 60 && nScore < 80) { sComplexity = "<#PASS_score3#>"; }
			else if (nScore >= 80 && nScore <= 100) { sComplexity = "<#PASS_score4#>"; }
		}
		else{
			if (nScore >= 0 && nScore < 20) { sComplexity = "<a href='Advanced_Wireless_Content.asp' target='_blank'><#PASS_score0#></a>"; }
			else if (nScore >= 20 && nScore < 40) { sComplexity = "<a href='Advanced_Wireless_Content.asp' target='_blank'><#PASS_score1#></a>"; }
			else if (nScore >= 40 && nScore < 60) { sComplexity = "<a href='Advanced_Wireless_Content.asp' target='_blank'><#PASS_score2#></a>"; }
			else if (nScore >= 60 && nScore < 80) { sComplexity = "<a href='Advanced_Wireless_Content.asp' target='_blank'><#PASS_score3#></a>"; }
			else if (nScore >= 80 && nScore <= 100) { sComplexity = "<a href='Advanced_Wireless_Content.asp' target='_blank'><#PASS_score4#></a>"; }
		}

		/* Display updated score criteria to client */
		if(typeof document.forms[0] == "undefined" || (typeof document.forms[0] != "undefined" && document.form.current_page.value != "AiProtection_HomeProtection.asp")){		//for Router weakness status, Jimeing added at 2014/06/07
			oScorebarBorder.style.display = "flex";
			oScorebar.style.backgroundPosition = parseInt(nScore) + "%";
		}
		else{
			if(nScore >= 0 && nScore < 40){
				oScore.className = "status_no";
			}
			else if(nScore >= 40 && nScore <= 100){
				oScore.className = "status_yes";
			}
		}
		if(oScore == null){
			oScorebar.innerHTML = sComplexity;
		}
		else{
			oScore.innerHTML = sComplexity;
		}
	}
	else {
		/* Display default score criteria to client */
		if(flag == 'http_passwd'){
			chkPass(" ", 'http_passwd', obj, id);
		}
		else
			chkPass(" ", "", obj, id);
	}
}

function check_password_length(obj){

	var password = obj.value;
	var httpPassInput = $("#http_passwd");

        if(isSku("KR") || isSku("SG") || isSku("AA") || isSupport("secure_default")){     /* MODELDEP by Territory Code */
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
	else{
		// if the SSID of dwb is hidden.
		if(!systemVariable.isDefault){
			delete qisPostData[`wl${dwb_band}_ssid`];
			delete qisPostData[`wl${dwb_band}_auth_mode_x`];
			delete qisPostData[`wl${dwb_band}_wpa_psk`];
			delete qisPostData[`wl${dwb_band}_crypto`];
			delete qisPostData[`wl${dwb_band}_mfp`];	
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

		var ap_band = $("<div>").addClass("ap_band").html((AP.mlomacaddr == "" || !isSupport("MLO_CLIENT")) ? AP.band : "MLO");
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
		else if(nodeInfo.type != undefined && nodeInfo.type == "131072")
			band_icon.addClass("aimesh_band_icon icon_moca");
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
	return $(`<div class="gg-spinner-two-alt"></div>`);
}

var Get_Component_WirelessInput = function(wlArray){
	var container = $("<div>");

	wlArray.forEach(function(wl, idx){
		var wirelessAP = httpApi.nvramCharToAscii(["wl" + wl.ifname + "_ssid", "wl" + wl.ifname + "_wpa_psk", "wl" + wl.ifname + "_auth_mode_x"]);

		if(isSupport("sdn_mainfh") && !systemVariable.isDefault){
			const main_fh_info = get_sdn_main_fh_info();
			const band_name = get_band_by_wl_unit(wl.ifname);
			const band_value = (()=>{
				if(band_name == "2g1") return 1;
				if(band_name == "5g") return 2;
				if(band_name == "5g1") return (get_wl_unit_by_band("5g2") == "") ? 2 : 4;
				if(band_name == "5g2") return 8;
				if(band_name == "6g") return 16;
				if(band_name == "6g1") return (get_wl_unit_by_band("6g2") == "") ? 16 : 32;
				if(band_name == "6g2") return 64;
			})();

			const specific_main_fh_info = main_fh_info.find(item => item.band & band_value);
			if(specific_main_fh_info != undefined){
				wirelessAP["wl" + wl.ifname + "_ssid"] = specific_main_fh_info["ssid"];
				wirelessAP["wl" + wl.ifname + "_wpa_psk"] = specific_main_fh_info["psk"];
			}
			else{
				wirelessAP["wl" + wl.ifname + "_ssid"] = "";
				wirelessAP["wl" + wl.ifname + "_wpa_psk"] = "";
			}
		}

		// Do not use default value.
		if(systemVariable.isDefault && !systemVariable.keepDefpsk){
			wirelessAP["wl" + wl.ifname + "_ssid"] = "";
			wirelessAP["wl" + wl.ifname + "_wpa_psk"] = "";
		}

		if(systemVariable.multiPAP.wlcOrder.length > 0 && !isSwMode("WISP")){
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

			ssid_tmp = `${ssid_tmp.slice(0,26)}_RPT${get_band_by_wl_unit(wl.ifname).replace("2g", "").replace("1", "").toUpperCase()}`;
			if(systemVariable.productid.indexOf("_GO") != -1) ssid_tmp = ssid_tmp.replace("_RPT", "_GO");
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
			.append($("<div>").addClass("inputTitle").html(wl.title + " <#QIS_finish_wireless_item3#>"))
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
					chkPass(this.value, "WiFi", "", wl.ifname);
				})
				.val(decodeURIComponent(wirelessAP["wl" + wl.ifname + "_wpa_psk"]))
			)
			.appendTo(__container);

		container.append(__container);
	})

	return container;
}

var Get_Component_WirelessInput_MLO = function(wlArray){
	var container = $("<div>");
	var __container = $("<div>").addClass("wirelessBand");

	$("<div>")
		.addClass("inputTitleContainer")
		.append($("<div>").addClass("inputTitle").html(wlArray.title + " <#QIS_finish_wireless_item1#>"))
		.appendTo(__container)

	$("<div>")
		.addClass("inputContainer")
		.append($("<input>")
			.attr({
				"id": "wireless_ssid_" + wlArray.ifname,
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
				if(e.keyCode == 13){
					$(".wlInput")[idx*2+1].focus();
				}
	
				validator.ssidCheck($("#"+this.id));
			})
			.val(wlArray.ssid)
		)
		.appendTo(__container)

	$("<div>")
		.addClass("inputTitleContainer")
		.append($("<div>").addClass("inputTitle").html(wlArray.title + " <#QIS_finish_wireless_item3#>"))
		.append($("<div>").addClass("secureInput icon_eye_close").attr({"for":"wireless_key_" + wlArray.ifname}))
		.appendTo(__container)

	$("<div>")
		.addClass("inputContainer")
		.append($("<input>")
			.attr({
				"id": "wireless_key_" + wlArray.ifname,
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
				if(e.keyCode == 13){
					try{
						$(".wlInput")[idx*2+2].focus();
					}
					catch(e){
						apply.wireless();
					}
				}
				chkPass(this.value, "WiFi", "", wlArray.ifname);
			})
			.val(wlArray.psk)
		)
		.appendTo(__container);

	container.append(__container);
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
}

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
	$(".dslite_xpass").toggle(isSupport("s46") && isSupport("dslite"));
	$(".dslite_transix").toggle(isSupport("s46") && isSupport("dslite"));
	$(".v6option").toggle(isSupport("s46") && isSupport("v6option"));
	$(".vpnClient").toggle(isSupport("VPNCLIENT"));
	$(".iptv").toggle(isSupport("IPTV"));
	$(".defaultSupport").toggle(systemVariable.isDefault);
	$(".configuredSupport").toggle(!systemVariable.isDefault);
	//$(".forceUpgrade").toggle(isSupport("fupgrade")); 
	$(".routerSupport").toggle(!isSupport("noRouter"));
	$(".apSupport").toggle(!isSupport("noAP"));
	$(".defpassSupport").toggle(isSupport("defpass"));
	$(".defpskSupport").toggle(isSupport("defpsk"));
	$(".wispSupport").toggle(isSupport("wisp"));
	$(".mloSupport").toggle(isSupport("mlo"));

	if(systemVariable.forceChangePw){
		systemVariable.forceChangePw = false;
		systemVariable.forceChangePwInTheEnd = true;

		if(isSupport("BUSINESS") || isSupport("sdn_mwl")){
			systemVariable.forceChangePw = true;
			systemVariable.forceChangePwInTheEnd = false;
		}
	}

	if(!isNoWAN || !isSupport("amasNode")) $(".amasNoWAN").remove();
	if(!isSupport("amas")) $(".amasSupport").remove();
	if(isSupport("amas") && isSupport("amas_bdl")){
		$("#amassearch_page").load("/mobile/pages/amassearch_page.html");
		$("#amasonboarding_page").load("/mobile/pages/amasonboarding_page.html");
	}

	if(isSupport("amas_bdl") && amas_bdl_num > 2){
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
		
		$("#gdContainer").append($("<div>").attr({"class": "GD-wait"}).append($("<div>").attr({"id": "GD-status"}).html("<#Excute_processing#>")))
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

	if(isSupport("TS_UI")){
		$("#summary_page").append($("<div>").attr({"id": "gdContainer", "class": "gundam-footer-field"}).hide())
		$("#gdContainer").html('');
		$("#gdContainer").append($("<div>").attr({"class": "GD-wait"}).append($("<div>").attr({"id": "GD-status"}).html("<#Excute_processing#>")));
	}

	if(isSupport("mlo")){
		$('#wifi6e_legacy_hint').remove();
		$('#wifi6e_legacy_hint_summary').remove();

		$("#mlo_enable_checkbox").change(function(e){
			var curStatus = $(this).prop("checked");
	
			if(curStatus){
				if(isSwMode("RP")){
					qisPostData["mlo_rp"] = "1";
					qisPostData["mlo_mb"] = "0";
				}
				else if(isSwMode("MB")){
					qisPostData["mlo_rp"] = "0";
					qisPostData["mlo_mb"] = "1";
				}

				qisPostData["mld_enable"] = "1";
				qisPostData["wlc_dpsta"] = "2";
				qisPostData["wlc_band"] = httpApi.nvramGet(["mlo_map"]).mlo_map.replace("wl", "");
			}
			else{
				qisPostData["mlo_rp"] = "0";
				qisPostData["mlo_mb"] = "0";
				qisPostData["mld_enable"] = "0";
				qisPostData["wlc_dpsta"] = "0";
			}
		});	
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

var tz_table = new Object;
function setUpTimeZone(){
	postDataModel.insert(timeObj);
	// getDST_db();
	require(['/require/modules/timeZone.js'], function(timeZone) {
		var timeZoneObj = new timeZone.get(systemVariable.uiLanguage);
		qisPostData.time_zone = timeZoneObj.time_zone;
		qisPostData.time_zone_dst = (timeZoneObj.hasDst) ? 1 : 0;
	});
	setTimeout(function(){
		if(qisPostData.time_zone_dst){
			qisPostData.time_zone_dstoff = getTimeZoneOffset(qisPostData.time_zone);
		}
	}, 500);

}
function getDST_db(){
	$.getJSON("/ajax/tz_db.json", function(data){tz_table = data;
		$.getJSON("https://nw-dlcdnet.asus.com/plugin/js/tz_db.json", function(data){tz_table = data;});
	});
}
var getTimeZoneOffset = function(tz){
	return tz_table[tz] ? tz_table[tz] : "M3.2.0/2,M11.1.0/2";
}

function setupWLCNvram(apProfileID) {
	systemVariable.selectedAP = systemVariable.papList[apProfileID];
	var unit = systemVariable.selectedAP.unit;

	if(systemVariable.selectedAP.mlomacaddr != "" && isSupport("MLO_CLIENT")){
		$("#mlo_enable_checkbox").enableCheckBox(true);
		$("#mlo_enable_checkbox").change();
	}
	else{
		$("#mlo_enable_checkbox").enableCheckBox(false);
		$("#mlo_enable_checkbox").change();	
	}

	postDataModel.insert(wlcMultiObj["wlc" + unit]);

	var encryption = systemVariable.selectedAP.encryption;
	var authentication = systemVariable.selectedAP.authentication;
	var phyMode = systemVariable.selectedAP.wlmode;
	var wifi7Mode = httpApi.nvramGet([ `wl${unit}_11be`]);
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
		else if(authentication == "WPA3-Personal" && encryption == "AES+GCMP256"){
			qisPostData["wlc" + unit + "_auth_mode"] = "sae";
			qisPostData["wlc" + unit + "_crypto"] = "aes+gcmp256";
			qisPostData["wlc" + unit + "_wep"] = "0";
			qisPostData["wl" + unit + "_11be"] = "1";
		}
		else if(authentication == "WPA3-Personal" && encryption == "AES"){
			qisPostData["wlc" + unit + "_auth_mode"] = "sae";
			qisPostData["wlc" + unit + "_crypto"] = "aes";
			qisPostData["wlc" + unit + "_wep"] = "0";
			if(isSupport('wifi7') && wifi7Mode == '1' && phyMode == 'be'){
				qisPostData["wlc" + unit + "_crypto"] = "aes+gcmp256";
			}
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

	if(isSwModeChanged() && isSwMode("RT")){
		return "restart_all";
	}

	if(isSupport("2p5G_LWAN") || isSupport("10G_LWAN") || isSupport("10GS_LWAN") || isSupport("autowan") || Object.keys(systemVariable.eth_wan_list).length > 1){
		if(isWANLANChange())
			return "reboot";
	}

	if((qisPostData.hasOwnProperty("switch_wantag") && (qisPostData.switch_wantag != original_switch_wantag)) ||
		qisPostData.hasOwnProperty("wlc_ssid") ||
		qisPostData.hasOwnProperty("lan_proto") ||
		qisPostData.hasOwnProperty("wans_dualwan") ||
		qisPostData.hasOwnProperty("wans_mode") ||
		isSwModeChanged()
	){
		return "reboot";
	}

	if(current_webs_chg_sku){
		return "reboot";
	}

	if(isSupport("mlo") && qisPostData.hasOwnProperty("mld_enable") && isSupport("bcmwifi")){
		const wl_mlo_config = httpApi.nvramGet(["wl_mlo_config"]).wl_mlo_config;
		if(wl_mlo_config == "") return "reboot";
	}

	var restart_net = 0;
	var restart_firewall = 0;

	/*
	if(qisPostData.hasOwnProperty("ipv6_service")){
		restart_net = 1;
		actionScript.push("restart_net");
	}
	*/

	if(systemVariable.detwanResult.isIPConflict){
		actionScript.push("restart_subnet");
	}

	if(qisPostData.hasOwnProperty("wans_mt_ioport")){
		actionScript.push("restart_mtwan_profile");
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
		qisPostData.hasOwnProperty("wl0_11ax") || 
		systemVariable.isDefault || 
		isSmartConnectChanged()
	){
		actionScript.push("restart_wireless");
	}

	if(systemVariable.isDefault && isSupport("lantiq")){
		actionScript.push("stop_bluetooth_service");
	}

	if(!restart_net && qisPostData.hasOwnProperty("cfg_master")){
		actionScript.push("restart_cfgsync");
	}

	if(!restart_net && !systemVariable.restart_wan_if && isWANChanged()){
		systemVariable.restart_wan_if = true;
		actionScript.push("restart_wan_if " + systemVariable.ethWanIf);
	}

	if(!restart_net && !systemVariable.restart_wan_if && qisPostData.hasOwnProperty("wrs_protect_enable")){
		// do not push these if restart_wan_if has been pushed. these are included in restart_wan_if.
		actionScript.push("restart_firewall");
		actionScript.push("restart_wrs");
	}

	if(qisPostData.hasOwnProperty("sdn_rl")){
		const rcs_list = ["restart_wireless", "restart_sdn"];
		$.each(rcs_list, function(index, item){
			if(!(actionScript.includes(item)))
				actionScript.push(item);
		});
	}

	if(qisPostData.hasOwnProperty("sdn_rc_service")){
		var rcs_list = qisPostData.sdn_rc_service.split(";");
		$.each(rcs_list, function(index, item){
			if(!(actionScript.includes(item)))
				actionScript.push(item);
		});
		delete qisPostData["sdn_rc_service"];
	}

	if(actionScript.includes("restart_subnet")){
		actionScript = actionScript.filter(item =>
			item !== "restart_wireless" &&
			item !== "restart_cfgsync" &&
			item !== "restart_sdn"
		);
	}

	return actionScript.join(";")
}

var setRestartService = function (postData) {
	if (!postData) postData = {};
	postData.action_mode = "apply";
	postData.rc_service = getRestartService();
	if (systemVariable.isDefault) postData.cfg_pause = "3";
	if (postData.rc_service.split(",").includes("reboot")) {
		sendMessageToSiteManager();
	}
	return postData;
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

	if(isSupport("autowan") && qisPostData.hasOwnProperty("autowan_enable")){
		if(qisPostData.autowan_enable != systemVariable.originAutoWAN) isChanged = true;
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


var get_default_wan_name = function(){
	var default_wan_name = "WAN";

	if(Object.keys(systemVariable.eth_wan_list).length > 1){
		default_wan_name = systemVariable.eth_wan_list["wan"].wan_name;
	}

	return default_wan_name;
}

var useDefaultWAN = function(){
	let _use_default_wan = true;

	if(Object.keys(systemVariable.eth_wan_list).length > 1){
		$.each(systemVariable.eth_wan_list, function(key) {
			if(key == "wan"){
				let wan_obj = systemVariable.eth_wan_list[key];
				if(wan_obj.hasOwnProperty("extra_settings")){
					let extra_settings = wan_obj.extra_settings;

					$.each(extra_settings, function(key) {
						if( (qisPostData.hasOwnProperty(key) && (qisPostData[key] != extra_settings[key])) ||
							(!qisPostData.hasOwnProperty(key) && (systemVariable[key] != extra_settings[key]))
						){
							_use_default_wan = false;
							return false;
						}
					});
				}
				return false;
			}
		});
	}

	return _use_default_wan;
}

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
			matchingResult = (isSupport("noiptv") || isSku("US") || isSku("CN") || isSku("CT") || isSku("GD") || isSku("TC") || isSku("CA")|| isSku("U2") || isSku("TW")) ? false : true;
			break;
		case "ENABLE_IPv6":
			matchingResult = (isSku("CN") || isSku("GD") || isSku("TC")) ? true : false;
			break;
		case "SMARTCONNECT":
			matchingResult = (ui_support["smart_connect_v2"] == "1" || ui_support["smart_connect"] == "1" || ui_support["smart_connect"] == "2" || ui_support["bandstr"] == "1") ? true : false;
			break;
		case "GUNDAM_UI":
			matchingResult = ((isGundam() || isKimetsu() || isEva()) && $(".desktop_left_field").is(":visible")) ? true : false;
			break;
		case "amas_bdl":
			matchingResult = (isSupport("prelink") && ui_support["amas_bdl"] >= 1 && amas_bdlkey.length != 0) ? true : false;
			break;
		case "FRONTHAUL_NETWORK":
			matchingResult = ui_support["amas_fronthaul_network"] || false;
			break;
		case "boostkey": // force remove boostkey from qis
			matchingResult = false;
			break;
		case "MB_mode_concurrep":
			if(isSwMode("MB") && (ui_support["concurrep"] == 1) && odmpid != "RP-AC1900" && based_modelid != 'RP-AX56' && based_modelid != 'RP-AX58' && based_modelid != 'RP-BE58')
				matchingResult = true;
			else
				matchingResult = false;
			break;
		case "MLO_CLIENT":
			matchingResult = (ui_support["mloclient"]) ? true : false;
			break;						
		case "concurrep":
			matchingResult = ui_support["concurrep"] && qisPostData.mlo_rp != 1 && qisPostData.mlo_mb != 1;
			break;
		case "defpsk":
			matchingResult = (ui_support["defpsk"] >= 1 && (httpApi.nvram_match_x("wifi_psk","","1").wifi_psk != "1")) ? true : false;
			break;
		case "MaxRule_SDN":
			matchingResult = ui_support["MaxRule_SDN"] || false;
			break;
		case "SDN_Scenario":
			matchingResult = (
				isSupport("mtlancfg") && isSupport("BUSINESS") && !isSupport("mlo") &&
				systemVariable.isDefault && systemVariable.productid.indexOf('EBG') == -1 &&
				isSwMode("RT")
			) ? true : false;
			break;
		case "SDN_White_Theme":
			matchingResult = false;
			if(isSupport("mtlancfg") && isSupport("BUSINESS")){
				matchingResult = true;
			}
			break;
		case "apMode_detwan":
			matchingResult = (systemVariable.productid == "EBA63") ? true : false;
			break;
		case "mloSetup":
			matchingResult = (systemVariable.mloSetup && (isSwMode("RT") || isSwMode("AP"))) ? true : false;
			break;
		case "mainfhSetup":
			matchingResult = (isSupport("sdn_mwl") && (isSwMode("RT") || isSwMode("AP"))) ? true : false;
			break;
		case "SiteManager":
			matchingResult = (navigator.userAgent.match(/ASUSMultiSiteManager/) || navigator.userAgent.match(/ASUSExpertSiteManager/)) ? true : false;
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
		httpApi.nvramSet({"action_mode":"apply", "webs_update_trigger":"QIS", "rc_service":"start_webs_update", "skip_modify_flag":"1"}, function(){
			setTimeout(function(){
				var fwInfo = httpApi.nvramGet(["webs_state_update", "webs_state_info", "webs_state_flag", "webs_state_level", "webs_update_ts", "apps_sq"], true);
				CheckTime = (fwInfo.webs_update_ts == "")? 0:fwInfo.webs_update_ts.split("&#62")[0];
				TimeDiff = NowTime-CheckTime;
				
				if(fwInfo.webs_state_flag == "1" || fwInfo.webs_state_flag == "2"){
					systemVariable.isNewFw = fwInfo.webs_state_flag;
					systemVariable.newFwVersion = fwInfo.webs_state_info;
					systemVariable.forceLevel = fwInfo.webs_state_level;
				}

				systemVariable.webs_state_update = fwInfo.webs_state_update;
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

validator.valid_block_chars = function(str, keywordArray){
	var testResult = {
		'isError': false,
		'errReason': ''
	};

	// bolck ascii code 32~126 first
	var invalid_char = "";
	for(var i = 0; i < str.length; ++i){
		if(str.charCodeAt(i) < '32' || str.charCodeAt(i) > '126'){
			invalid_char += str.charAt(i);
		}
	}
	if(invalid_char != ""){
		testResult.isError = true;
		testResult.errReason = '<#JS_validstr2#>" '+ invalid_char +'" !';
		return testResult;
	}

	// check if char in the specified array
	if(str){
		for(var i=0; i<keywordArray.length; i++){
			if(str.indexOf(keywordArray[i]) >= 0){
				testResult.isError = true;
				testResult.errReason = keywordArray + " <#JS_invalid_chars#>";
				return testResult;
			}
		}
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

	if(systemVariable.multiPAP.wlcOrder.length > 1 || qisPostData["mlo_rp"] == "1"){
		var allWlArray = getAllWlArray();
		for(var i = 0; i < allWlArray.length; i += 1){
			var wlcUnit = allWlArray[i].ifname;
			transformWLIdx(wlcUnit);
		}
	}
	else{
		var wlcUnit = systemVariable.multiPAP.wlcOrder[0];
		transformWLIdx(wlcUnit);
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
			const band_name = get_band_by_wl_unit(wl.ifname);
			if(band_name == "2g1") ssid_tmp = ssid_tmp.slice(0,28) + "_RPT";
			if(band_name == "5g1") ssid_tmp = ssid_tmp.slice(0,26) + "_RPT5G";
			if(band_name == "5g2") ssid_tmp = ssid_tmp.slice(0,25) + "_RPT5G2";
			if(band_name == "6g1") ssid_tmp = ssid_tmp.slice(0,26) + "_RPT6G";
			if(band_name == "6g2") ssid_tmp = ssid_tmp.slice(0,25) + "_RPT6G2";
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

		// sort WL interface sequence
		if(wlArray[0] !== '0'){
			wlArray.push(wlArray[0]);
			wlArray.shift();
		}

		wlArray.forEach(function(band){
			$("#wlc_band_manual").append($("<option>").val(getAllWlArray()[band].ifname).html(getAllWlArray()[band].title));
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

	$("#wlc_crypto_manual option[value='aes']").hide();
	$("#wlc_crypto_manual option[value='tkip']").hide();
	$("#wlc_crypto_manual option[value='aes+gcmp256']").hide();

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

		$("#wlc_crypto_manual option[value='aes']").show();
		$("#wlc_crypto_manual option[value='tkip']").show();
		$("#wlc_crypto_manual option[value='aes+gcmp256']").hide();
	}
	else if(auth_mode == "sae"){
		$("#manual_pap_setup-crypto").show();
		$("#manual_pap_setup-key").show();
		$("#wlc_crypto_manual option[value='aes']").show();
		$("#wlc_crypto_manual option[value='tkip']").hide();
		if(isSupport("wifi7")){
			$("#wlc_crypto_manual option[value='aes+gcmp256']").show();
		}
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
function adjust_popup_container_top(_obj, _offsetHeight){
	$(_obj).css({top: ""});
	var scrollTop = window.pageYOffset || document.documentElement.scrollTop || document.body.scrollTop || 0;
	var parent_scrollTop = parent.window.pageYOffset || parent.document.documentElement.scrollTop || parent.document.body.scrollTop || 0;
	if(scrollTop == 0 && parent_scrollTop != 0)
		parent_scrollTop = parent_scrollTop - 200;
	var final_scrollTop = Math.max(scrollTop, parent_scrollTop);
	if(final_scrollTop != 0){
		$(_obj).css({top: (final_scrollTop + _offsetHeight)});
	}
}
function stringSafeGet(str){
	return str.replace(new RegExp("&#39;", 'g'), "'");
}
var str_find_st = stringSafeGet("<#HowFindST#>");
var str_HowFindPassword = stringSafeGet("<#HowFindPassword#>");
function initialize_SDN(){
	$('link').last().after('<link group="SDN_need_file" rel="stylesheet" type="text/css" href="./RWD_UI/rwd_component.css">');
	$('link').last().after('<link group="SDN_need_file" rel="stylesheet" type="text/css" href="./SDN/sdn.css">');
	$('link').last().after('<link group="SDN_need_file" rel="stylesheet" type="text/css" href="./js/weekSchedule/schedule_ui.css">');
	$('link').last().after('<link group="SDN_need_file" rel="stylesheet" type="text/css" href="./pwdmeter.css">');
	if(isSupport("SDN_White_Theme")){
		$('link').last().after('<link group="SDN_need_file" rel="stylesheet" type="text/css" href="./RWD_UI/rwd_component_WHITE.css">');
		$('link').last().after('<link group="SDN_need_file" rel="stylesheet" type="text/css" href="./SDN/sdn_WHITE.css">');
		$('link').last().after('<link group="SDN_need_file" rel="stylesheet" type="text/css" href="./js/weekSchedule/schedule_ui_WHITE.css">');
	}
	$('<script />', { type : 'text/javascript', src : "./RWD_UI/rwd_component.js", group: "SDN_need_file"}).appendTo($('head'));
	$('<script />', { type : 'text/javascript', src : "./SDN/sdn.js", group: "SDN_need_file"}).appendTo($('head'));
	$('<script />', { type : 'text/javascript', src : "./popup.js", group: "SDN_need_file"}).appendTo($('head'));
	$('<script />', { type : 'text/javascript', src : "./js/weekSchedule/schedule_ui.js", group: "SDN_need_file"}).appendTo($('head'));
	$('<script />', { type : 'text/javascript', src : "./form.js", group: "SDN_need_file"}).appendTo($('head'));
	setTimeout(function(){
		if(isSupport("captivePortal") && isSupport("cp_freewifi")){
			Get_FreeWiFi_template();
		}
		init_sdn_all_list();
	},500);
}
var interval_cfg_pause = false;
function set_cfg_pause(){
	clearInterval(interval_cfg_pause);
	if(!systemVariable.isDefault)
		return;

	postData();
	interval_cfg_pause = setInterval(function(){
		if(qisPostData.hasOwnProperty("cfg_pause")){
			if(qisPostData.cfg_pause == "0"){
				clearInterval(interval_cfg_pause);
			}
			else{
				postData();
			}
		}
		else{
			postData();
		}
	},1000*60*10);

	function postData(){
		httpApi.nvramSet({
			"action_mode" : "apply",
			"cfg_pause" : 120,
			"skip_modify_flag": "1"
		});
	}
}

function isJoinSmartConnect(wlIfIndex){
    wlIfIndex = wlIfIndex.toLowerCase();

    if(wlIfIndex == "2g") wlIfIndex = "2g1";
    if(wlIfIndex == "5g") wlIfIndex = "5g1";
    if(wlIfIndex == "6g") wlIfIndex = "6g1";

    const referenceArray = ["", "", "", "6g2", "6g1", "5g2", "5g1", "2g1"];
    var smart_connect_selif_x = httpApi.nvramGet(["smart_connect_selif_x"]).smart_connect_selif_x;

    var v2Band = (() => {
        const maxLength = 8;
        let biString = parseInt(smart_connect_selif_x).toString(2);
        let bandArray = biString.split("");
        while (bandArray.length < maxLength) {
            bandArray.unshift("0");
        }
    
        return bandArray;
    })();

    var version = isSupport("smart_connect_v2") ? "v2" : isSupport("smart_connect") || isSupport("bandstr") ? "v1" : "";
    var v1Type = httpApi.nvramGet(["smart_connect_x"]).smart_connect_x;

    if (version === "v2") {
        let index = referenceArray.findIndex((element) => element === wlIfIndex);
        return v2Band[index] === "1";
    } else {
        if (v1Type === "1") {
            return true;
        } else if (v1Type === "2") {
            if (wlIfIndex === "5g1" || wlIfIndex === "5g2") {
                return true;
            }
        } else if (v1Type === "3") {
            if (wlIfIndex === "2g1" || wlIfIndex === "5g1") {
                return true;
            }
        }

        return false;
    }
}

function get_band_by_wl_unit(wl_unit){
	var wlnband_list = httpApi.nvramGet(["wlnband_list"]).wlnband_list.split("&#60");
	return wlnband_list[wl_unit];
}
function getDutBand(){
	let bandCount = 0;
	if(get_wl_unit_by_band("2G") != "") bandCount += 1;
	if(get_wl_unit_by_band("5G") != "" && get_wl_unit_by_band("5G2") == "") bandCount += 2;
	if(get_wl_unit_by_band("5G1") != "" && get_wl_unit_by_band("5G2") != "") bandCount += 4;
	if(get_wl_unit_by_band("5G2") != "") bandCount += 8;
	if(get_wl_unit_by_band("6G") != "" && get_wl_unit_by_band("6G2") == "") bandCount += 16;
	if(get_wl_unit_by_band("6G1") != "" && get_wl_unit_by_band("6G2") != "") bandCount += 32;
	if(get_wl_unit_by_band("6G2") != "") bandCount += 64;
	return bandCount;
}
function getMloBand(){
	var result = "";
	const bandCount = getDutBand();
	const mlo_band_num_mapping = [
		{"all_band":(1+4+8+16), "mlo_band":(1+8+16)}, //2556, mlo:2/5-2/6
		{"all_band":(1+2+32+64), "mlo_band":(1+2+32)}, //2566, mlo:2/5/6-1
		{"all_band":(1+2+16), "mlo_band":(1+2+16)}, //256, mlo:2/5/6
		{"all_band":(1+4+8), "mlo_band":(1+4+8)}, //255
		{"all_band":(1+2), "mlo_band":(1+2)}, //25
	];

	for(var i=0; i<mlo_band_num_mapping.length; i++){
		if(bandCount == mlo_band_num_mapping[i].all_band){
			result = mlo_band_num_mapping[i].mlo_band;
			break;
		}
	}

	return result;
}

function get_new_sdn_mlo(){
	systemVariable.sdn_idx = 0;
	systemVariable.apg_idx = 0;
	const dutBand = (1+2+4);//2g,5g,5g1
	var cap_mac = (httpApi.hookGet('get_lan_hwaddr')) ? httpApi.hookGet('get_lan_hwaddr') : '';
	const sdn_maximum = ((isSupport("MaxRule_SDN") == "0") ? 6 : (parseInt(isSupport("MaxRule_SDN")) - 1));//default is sdn 0
	let sdn_idx_arr = Array.from({length: sdn_maximum}, (num, i) => (i+1));
	let apg_idx_arr = sdn_idx_arr.concat();
	const sdn_rl = (()=>{
		if(isSwModeChanged() || systemVariable.isDefault)
			return (httpApi.nvramDefaultGet(["sdn_rl"]).sdn_rl).replace(/&#60/g, "<").replace(/&#62/g, ">")
		else
			return decodeURIComponent(httpApi.nvramCharToAscii(["sdn_rl"]).sdn_rl);
	})();
	const each_sdn_rl = sdn_rl.split("<");
	$.each(each_sdn_rl, function(index, value){
		if(value != ""){
			const profile_data = value.split(">");
			const sdn_idx = profile_data[0];
			const apg_idx = profile_data[5];
			sdn_idx_arr = sdn_idx_arr.filter(e => e != sdn_idx);
			apg_idx_arr = apg_idx_arr.filter(e => e != apg_idx);
		}
	});
	const new_sdn_idx = sdn_idx_arr[0];
	const new_apg_idx = apg_idx_arr[0];
	let post_sdnObj = {}
	if(new_sdn_idx != undefined && new_apg_idx != undefined){
		post_sdnObj = JSON.parse(JSON.stringify(sdnObj));
		post_sdnObj.sdn_rl = sdn_rl;
		post_sdnObj.sdn_rl += `<${new_sdn_idx}>LEGACY>1>0>0>${new_apg_idx}>0>0>0>0>0>0>0>0>0>0>0>0>0>WEB>0>0`;
		post_sdnObj.apg_dut_list = `<*>${dutBand}>`;
		post_sdnObj.apg_mlo = "0";
		Object.keys(post_sdnObj).forEach(function(key){
			if(key.substring(0, 3) == "apg"){
				post_sdnObj[key.replace("apg", "apg" + new_apg_idx)] = post_sdnObj[key];
				delete post_sdnObj[key];
			}
		});
		systemVariable.sdn_idx = parseInt(new_sdn_idx);
		systemVariable.apg_idx = parseInt(new_apg_idx);
	}

	return post_sdnObj;
}

function createSDNCompatibleNetwork(){
	if(isSwModeChanged()){
		return true;
	}
	const sdn_rl = decodeURIComponent(httpApi.nvramCharToAscii(["sdn_rl"]).sdn_rl);
	const sdn_rl_default = (httpApi.nvramDefaultGet(["sdn_rl"]).sdn_rl).replace(/&#60/g, "<").replace(/&#62/g, ">");
	if (sdn_rl === sdn_rl_default) {
		return true;
	}
	
	const sdn_maximum = ((isSupport("MaxRule_SDN") == "0") ? 6 : (parseInt(isSupport("MaxRule_SDN")) - 1));//default is sdn 0
	let legacy_exist = false;
	let sdn_rl_count = 0;
	const each_sdn_rl = (()=>{
		if(systemVariable.isDefault)
			return sdn_rl_default.split("<");
		else
			return sdn_rl.split("<");
	})();

	$.each(each_sdn_rl, function(index, value){
		if(value != ""){
			let profile_data = value.split(">");
			if(profile_data[1] == "LEGACY"){
				legacy_exist = true;
			}
			if(parseInt(profile_data[0]) > 0){
				sdn_rl_count++;
			}
		}
	});
	const sdn_rl_is_full = (sdn_rl_count >= sdn_maximum);
	return (!legacy_exist && !sdn_rl_is_full);
}

function get_mainfh_dut_list(_bitwise, _flag){
	const band_bitwise = parseInt(_bitwise) || 0;
	const mlo_enable = (_flag === true) || false;
	//2G: 1, 5G: 2, 5GL: 4, 5GH: 8, 6G: 16, 6GL: 32, 6GH: 64
	let band = 0;
	if(band_bitwise & 1) band += 1;//2
	if(band_bitwise & 2) band += 2;//5
	if(band_bitwise & 4) band += 4;//51
	if(band_bitwise & 8) band += 8;//52
	if(band_bitwise & 16) band += 16;//6
	if(band_bitwise & 32) band += 32;//61
	if(band_bitwise & 64) band += 64;//62

	if((band_bitwise & 4) && !(band_bitwise & 2)) band += 2;//5g1 on, need also on 5g
	if((band_bitwise & 64) && !(band_bitwise & 16)) band += 16;//6g2 on, need also on 6g

	if(mlo_enable){
		if((band_bitwise & 2) && !(band & 8)) band += 8;//5g on, need also on 5g2
		if((band_bitwise & 16) && !(band & 32)) band += 32;//6g on, need also on 6g1

		if((band_bitwise & 8) && !(band & 2)) band += 2;//5g2 on, need also on 5g
		if((band_bitwise & 32) && !(band & 16)) band += 16;//6g1 on, need also on 6g
	}
	else{
		if((band_bitwise & 2) && !(band & 4)) band += 4;//5g on, need also on 5g1
		if((band_bitwise & 16) && !(band & 64)) band += 64;//6g on, need also on 6g2

		if((band_bitwise & 8) && (band_bitwise > 8)){//5g2 on and band count > 2
			if(!(band & 2)) band += 2;//need also on 5g
		}
		if((band_bitwise & 32) && (band_bitwise > 32)){//6g1 on and band count > 2
			if(!(band & 16)) band += 16;//need also on 6g
		}
	}

	return `<*>${band}>`;
}

function findNextSdnIndex(sdnRulelist){
	const sdn_maximum = ((isSupport("MaxRule_SDN") == "0") ? 6 : (parseInt(isSupport("MaxRule_SDN")) - 1));//default is sdn 0
	let sdn_idx_arr = Array.from({length: sdn_maximum}, (num, i) => (i+1));
	let apg_idx_arr = sdn_idx_arr.concat();
	let apm_idx_arr = sdn_idx_arr.concat();
 
	if(sdnRulelist == ""){
		if(isSwModeChanged() || systemVariable.isDefault)
			sdnRulelist = (httpApi.nvramDefaultGet(["sdn_rl"]).sdn_rl).replace(/&#60/g, "<").replace(/&#62/g, ">");
		else
			sdnRulelist = decodeURIComponent(httpApi.nvramCharToAscii(["sdn_rl"]).sdn_rl);
	}
	const sdn_rl = sdnRulelist;
	const each_sdn_rl = sdn_rl.split("<");
	$.each(each_sdn_rl, function(index, value){
		if(value != ""){
			const profile_data = value.split(">");
			const sdn_idx = profile_data[0];
			const apg_idx = profile_data[5];
			sdn_idx_arr = sdn_idx_arr.filter(e => e != sdn_idx);
			if(profile_data[1] == "MAINFH" || profile_data[1] == "MAINBH")
				apm_idx_arr = apm_idx_arr.filter(e => e != apg_idx);
			else
				apg_idx_arr = apg_idx_arr.filter(e => e != apg_idx);
		}
	});
	
	return sdn_idx_arr[0];
}

function configApmVariables(){
	// initial sdn_rl
	if(!qisPostData.hasOwnProperty("sdn_rl")){
		if(isSwModeChanged() || systemVariable.isDefault)
			qisPostData.sdn_rl = (httpApi.nvramDefaultGet(["sdn_rl"]).sdn_rl).replace(/&#60/g, "<").replace(/&#62/g, ">");
		else
			qisPostData.sdn_rl = decodeURIComponent(httpApi.nvramCharToAscii(["sdn_rl"]).sdn_rl);
	}

	// remove MAINFH
	qisPostData.sdn_rl = qisPostData.sdn_rl.split("<").filter(item => !item.includes("MAINFH")).join("<");
	for(let key in qisPostData){if(key.startsWith("apm")){delete qisPostData[key];}}

	// install MAINBH
	systemVariable.macAddr = httpApi.nvramGet(["et0macaddr"]).et0macaddr;
	let MAINBH_array = qisPostData.sdn_rl.split("<").filter(item => item.includes("MAINBH"));
	let apmBhIdx = (MAINBH_array.length > 0) ? MAINBH_array[0].split(">")[5] : 1;
	if(MAINBH_array.length == 0){
		var bhName = `${calcMD5(qisPostData.wl0_ssid)}N`.slice(0, 32);
		var bhKey = `${calcMD5(qisPostData.wl0_wpa_psk)}K`.slice(0, 32);

		qisPostData.sdn_rl += `<${findNextSdnIndex(qisPostData.sdn_rl)}>MAINBH>1>0>0>${apmBhIdx}>0>0>0>0>0>0>0>0>0>0>0>0>0>WEB>0>0`;
		qisPostData[`apm${apmBhIdx}_11be`] = 1;
		qisPostData[`apm${apmBhIdx}_ap_isolate`] = 0;
		qisPostData[`apm${apmBhIdx}_dut_list`] = `<*>127>`;
		qisPostData[`apm${apmBhIdx}_enable`] = 1;
		qisPostData[`apm${apmBhIdx}_hide_ssid`] = 1;
		qisPostData[`apm${apmBhIdx}_macmode`] = 0;
		qisPostData[`apm${apmBhIdx}_mlo`] = 1;
		qisPostData[`apm${apmBhIdx}_security`] = `<3>psk2sae>aes+gcmp256>${bhKey}>0<13>psk2sae>aes+gcmp256>${bhKey}>0<16>sae>aes+gcmp256>${bhKey}>0<96>sae>aes+gcmp256>${bhKey}>0`;
		qisPostData[`apm${apmBhIdx}_ssid`] = bhName;
		qisPostData[`apm${apmBhIdx}_timesched`] = 0;
	}

	// install MAINFH
	const is_amas_bdl = isSupport("amas_bdl");
	const dut_band_num_mapping = [
		{"all_band":(1+4+8+16), "enable_band":is_amas_bdl ? (1+4+16) : (1+4+8+16)}, //2556
		{"all_band":(1+2+32+64), "enable_band":is_amas_bdl ? (1+2+32) : (1+2+32+64)}, //2566
		{"all_band":(1+2+16), "enable_band":is_amas_bdl ? (1+2+16) : (1+2+16)}, //256
		{"all_band":(1+4+8), "enable_band":is_amas_bdl ? (1+4) : (1+4+8)}, //255
		{"all_band":(1+2), "enable_band":is_amas_bdl ? (1+2) : (1+2)}, //25
	];
	var apmIdx = 1;
	for(var i=0; i<systemVariable.wirelessBand; i++){
		if($(`#wlInputField #wireless_ssid_${i}`).length == 0) continue;
		var ssid = $(`#wireless_ssid_${i}`).val();
		var key = $(`#wireless_key_${i}`).val();
		var dutBand = $(`#wlInputField .wirelessBand`).length == 1 ? "all" : get_band_by_wl_unit(i);
		var dutList = "";
		if(dutBand == "2g1") dutList = get_mainfh_dut_list(1, 0);
		if(dutBand == "5g") dutList = get_mainfh_dut_list(2, 0);
		if(dutBand == "5g1") dutList = get_mainfh_dut_list(get_wl_unit_by_band("5g2")==""?2:4, 0);
		if(dutBand == "5g2") dutList = get_mainfh_dut_list(8, 0);
		if(dutBand == "6g") dutList = get_mainfh_dut_list(16, 0);
		if(dutBand == "6g1") dutList = get_mainfh_dut_list(get_wl_unit_by_band("6g2")==""?16:32, 0);
		if(dutBand == "6g2") dutList = get_mainfh_dut_list(64, 0);
		if(dutBand == "all"){
			const enable_band = (()=>{
				const all_band = getDutBand();
				const band_info = dut_band_num_mapping.find(item => item.all_band == all_band);
				return (band_info != undefined) ? band_info.enable_band : all_band;
			})();
			dutList = get_mainfh_dut_list(enable_band, 0);
		}
		if(apmIdx == apmBhIdx) apmIdx++;
		qisPostData.sdn_rl += `<${findNextSdnIndex(qisPostData.sdn_rl)}>MAINFH>1>0>0>${apmIdx}>0>0>0>0>0>0>0>0>0>0>0>0>0>WEB>0>0`;
		qisPostData[`apm${apmIdx}_11be`] = 1;
		qisPostData[`apm${apmIdx}_ap_isolate`] = 0;
		qisPostData[`apm${apmIdx}_dut_list`] = dutList;
		qisPostData[`apm${apmIdx}_enable`] = 1;
		qisPostData[`apm${apmIdx}_hide_ssid`] = 0;
		qisPostData[`apm${apmIdx}_macmode`] = 0;
		qisPostData[`apm${apmIdx}_mlo`] = 0;
		qisPostData[`apm${apmIdx}_security`] = `<3>psk2sae>aes>${key}>0<13>psk2sae>aes>${key}>0<16>sae>aes>${key}>0<96>sae>aes>${key}>0`;
		qisPostData[`apm${apmIdx}_ssid`] = ssid;
		qisPostData[`apm${apmIdx}_timesched`] = 0;
		apmIdx++;
	}

	// remove wlX
	for(let key in qisPostData){
		if(key.startsWith("wl") && !key.startsWith("wlc")){
			delete qisPostData[key];
		}
	}
}
function get_sdn_main_fh_info(){
	let main_fh_info = [];
	if(isSupport("mainfhSetup")){
		const mainFH = decodeURIComponent(httpApi.nvramCharToAscii(["sdn_rl"]).sdn_rl).split("<").filter(item => item.includes("MAINFH"));
		mainFH.forEach(item=>{
			if(item != ""){
				const apmIdx = item.split(">")[5];
				const apm_ssid = decodeURIComponent(httpApi.nvramCharToAscii([`apm${apmIdx}_ssid`])[`apm${apmIdx}_ssid`]);
				const apm_security = decodeURIComponent(httpApi.nvramCharToAscii([`apm${apmIdx}_security`])[`apm${apmIdx}_security`]).split("<");
				const apm_dut_list = decodeURIComponent(httpApi.nvramCharToAscii([`apm${apmIdx}_dut_list`])[`apm${apmIdx}_dut_list`]).split("<");
				let apm_auth = "open";
				let apm_psk = "";
				let apm_band = 0;
				if(apm_security[1] != undefined && apm_security[1] != ""){
					const sec_arr = apm_security[1].split(">");
					apm_auth = sec_arr[1];
					apm_psk = sec_arr[3];
				}
				if(apm_dut_list[1] != undefined && apm_dut_list[1] != ""){
					const dut_list_arr = apm_dut_list[1].split(">");
					apm_band = parseInt(dut_list_arr[1]);
				}
				main_fh_info.push({"ssid":apm_ssid, "auth":apm_auth, "psk":apm_psk, "band":apm_band});
			}
		});
	}
	if(main_fh_info.length == 0){
		main_fh_info = [{"ssid":"", "auth":"open", "psk":"", "band":""}];
	}
	return main_fh_info;
}
