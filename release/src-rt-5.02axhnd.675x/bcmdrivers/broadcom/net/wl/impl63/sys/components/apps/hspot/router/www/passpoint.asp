<!--
Copyright (C) 2020, Broadcom. All Rights Reserved.

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

<<Broadcom-WL-IPTag/Open:>>
$ID$-->

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html lang="en">
<head>
<title>Broadcom Home Gateway Reference Design: SSID</title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<link rel="stylesheet" type="text/css" href="style.css" media="screen">
<script language="JavaScript" type="text/javascript" src="overlib.js"></script>
<script language="JavaScript" type="text/javascript">

var eapauthWindow;
var realmindex = -1;
var osupwindow;
var osupindex = -1

function wl_recalc()
{
	var ure_disable = "<% nvram_get("ure_disable"); %>";
	var wme_enabled = "<% nvram_get("wl_wme"); %>";
	var mode = "<% nvram_get("wl_mode"); %>";
	var wds = "<% nvram_get("wl_wds"); %>";
	var mac_restrict_mode = "<% nvram_get("wl_macmode"); %>";

	document.forms[0].wl_mode.disabled = 1;
	if (ure_disable == "0") {
		document.forms[0].wl_bridge.disabled = 1;
	}

	if (wme_enabled != "on") {
		document.forms[0].wl_wme_bss_disable.disabled = 1;
	}
	if (mode == "sta") {
		document.forms[0].wl_wmf_bss_enable.disabled = 1;
		document.forms[0].wl_bss_maxassoc.disabled = 1;
	}
	if(mode == "ap") {
		document.forms[0].wl_mcast_regen_bss_enable.disabled = 1;
	}
	/* grey out DWDS tab if not ap or sta and if lazy wds. */
	if (mode != "ap" && mode != "sta" && mode != "psr" && mode != "psta" || wds != "")
		document.forms[0].wl_dwds.disabled = 1;

	if(mac_restrict_mode == "disabled") {
		document.forms[0].wl_probresp_mf.disabled = 1;
	}
	recalc_ANQPParamsCtrls();
}

//Function for populating modify OSU List popup
function onmodify_osup(index)
{
	switch(Number(index))
	{
	case 0:
		<% print_popup_osup(0, 0, 2); %>
	break;

	case 1:
		<% print_popup_osup(1, 0, 2); %>
	break;

	case 2:
		<% print_popup_osup(2, 0, 2); %>
	break;

	case 3:
		<% print_popup_osup(3, 0, 2); %>
	break;
	}
	osupwindow = window.open("", "", "toolbar=no,scrollbars=yes,width=660,height=510,modal=yes");
	osupwindow.document.write(osupdata);
	osupindex = index;
}

//Function for deleting osulist provider
function ondelete_osup(index)
{
	document.getElementById("wl_osu_frndname_" + index).value = "";
	document.getElementById("wl_osu_uri_" + index).value = "";
	document.getElementById("wl_osu_nai_" + index).value = "";
	document.getElementById("wl_osu_method_" + index).value = "";
	document.getElementById("wl_osu_icons_" + index).value = "";
	document.getElementById("wl_osu_frndname_hid_" + index).value = "";
	document.getElementById("wl_osu_icons_hid_" + index).value = "";
	document.getElementById("wl_osu_servdesc_" + index).value = "";
}

//Function for closing modify OSU List popup
function onclose_osup()
{
	osupwindow.close();
}

//Function for updating main window from modify OSU List popup
function onsave_osup()
{
	var friendly_name = "", fnameid = "", langid = "", fname_val = "";
	var osu_server = "", osu_method = "", osumethodid = "", osu_methodval = "";
	var osu_nai = "", osu_servdes = "", osuserid = "", osuservlangid = "";
	var osu_icon = "", osuiconid = "", osuiconval = "";
	var idx = 0, canBeClosed = 1;

	var elem = osupwindow.document.getElementById("errmsg");
	elem.style.fontSize = 13 +"px";
	elem.style.color = "Red";

	/* For 2 columns of OSU friendly name and Service Description */
	for(idx; idx < 2; idx++) {
		fnameid = "fname_" + idx;
		langid = "lang_" + idx;
		var frndlyname = osupwindow.document.getElementById(fnameid).value;
		var langcode = osupwindow.document.getElementById(langid).value;
		if(frndlyname != "" && langcode != "") {
			if(friendly_name == '') {
				friendly_name = friendly_name + frndlyname + '!' + langcode;
				fname_val = frndlyname;
			} else {
				friendly_name =  friendly_name + '|' + frndlyname + '!' + langcode;
			}
		} else if((frndlyname == "" && langcode != "") || (frndlyname != "" && langcode == "")) {
			elem.innerHTML = "Enter valid OSU Friendly Name and Language Code";
			return;
		}

		osuserid = "svrdesc_" + idx;
		osuservlangid = "svrdesclang_" + idx;

		var osusvrdesc = osupwindow.document.getElementById(osuserid).value;
		var osusvrlang = osupwindow.document.getElementById(osuservlangid).value;

		if(osusvrdesc != "" && osusvrlang != "") {
			if(osu_servdes == ''){
				osu_servdes = osu_servdes + osusvrdesc + '!' + osusvrlang;
			} else {
				osu_servdes = osu_servdes + '|' + osusvrdesc + '!' + osusvrlang;
			}
		} else if((osusvrdesc == "" && osusvrlang != "") || (osusvrdesc != "" && osusvrlang == "")) {
			elem.innerHTML = "Enter valid OSU Service Description and Language Code";
			return;
		}
	}

	osu_server = osupwindow.document.getElementById("osuserverid").value;
	osumethodid = osupwindow.document.getElementById("osumethodid");
	osu_method = osumethodid.options[osumethodid.selectedIndex].value;

	if(osu_method == '0')
		osu_methodval = "OMA-DM";
	else
		osu_methodval = "SOAP-XML";

	osu_nai = osupwindow.document.getElementById("osunai").value;
	/* For 3 columns of OSU Icon and metadata */
	for(idx = 0; idx < 3; idx++) {
		osuiconid = "iconsel_" + idx;
		var ctl = osupwindow.document.getElementById(osuiconid);
		if(ctl.value != '-1') {
			if(osu_icon == '') {
				osu_icon = osu_icon + ctl.options[ctl.selectedIndex].text;
				osuiconval = ctl.options[ctl.selectedIndex].text;
			} else {
				osu_icon = osu_icon + "+" + ctl.options[ctl.selectedIndex].text;
			}
		}
	}

	if((friendly_name != '') && (osu_server != '') && (osu_method != '') && (osu_icon != '')) {
		var id = "wl_osu_frndname_"+ osupindex;
		var osufnd = document.getElementById(id);
		osufnd.value = fname_val;

		id = "";
		id = "wl_osu_uri_"+ osupindex;
		var osuri = document.getElementById(id);
		osuri.value = osu_server;

		id = "";
		id = "wl_osu_nai_"+ osupindex;
		var osuni = document.getElementById(id);
		osuni.value = osu_nai;

		id = "";
		id = "wl_osu_method_"+ osupindex;
		var osumth = document.getElementById(id);
		osumth.value = osu_methodval;
		var osuicn = document.getElementById("wl_osu_icons_"+ osupindex);
		osuicn.value = osuiconval;

		var env1 = document.getElementById("wl_osu_frndname_hid_"+ osupindex);
		env1.value = friendly_name;
		var env2 = document.getElementById("wl_osu_icons_hid_"+ osupindex);
		env2.value = osu_icon;
		var env3 = document.getElementById("wl_osu_servdesc_"+ osupindex);
		env3.value = osu_servdes;
	} else {
		if(friendly_name == '') {
			elem.innerHTML = "Enter valid OSU Friendly Name and Language Code";
		} else if(osu_server == '') {
			elem.innerHTML = "Enter valid OSU Server URI";
		} else if(osu_icon == '') {
			elem.innerHTML = "Select any OSU Icon";
		}
		return;
	}

	osupwindow.close();
}

//Function for Icon selection change in modify OSU List
function onchange_icon(iconid)
{
	var selectediconid = osupwindow.document.getElementById("iconsel_" + iconid);
	var selectediconval = selectediconid.options[selectediconid.selectedIndex].value;
	switch(Number(selectediconval))
	{
	case -1:
		<% icon_change(-1); %>
	break;

	case 0:
		<% icon_change(0); %>
	break;

	case 1:
		<% icon_change(1); %>
	break;

	case 2:
		<% icon_change(2); %>
	break;

	case 3:
		<% icon_change(3); %>
	break;

	case 4:
		<% icon_change(4); %>
	break;

	case 5:
		<% icon_change(5); %>
	break;

	case 6:
		<% icon_change(6); %>
	break;

	case 7:
		<% icon_change(7); %>
	break;

	case 8:
		<% icon_change(8); %>
	break;

	case 9:
		<% icon_change(9); %>
	break;

	case 10:
		<% icon_change(10); %>
	break;

	case 11:
		<% icon_change(11); %>
	break;

	case 12:
		<% icon_change(12); %>
	break;

	case 13:
		<% icon_change(13); %>
	break;

	case 14:
		<% icon_change(14); %>
	break;
	}
	osupwindow.document.getElementById("iwidth_" + iconid).value = width;
	osupwindow.document.getElementById("iheight_" + iconid).value = height;
	osupwindow.document.getElementById("itype_" + iconid).value = mimetype;
	osupwindow.document.getElementById("ilangcode_" + iconid).value = langcode;
}

//Function for populating modify Realm List popup
function onmodify_eapauth(index)
{
	switch(Number(index))
	{
	case 0:
		<% print_popup_realm(0, 0, 3); %>
	break;

	case 1:
		<% print_popup_realm(1, 0, 3); %>
	break;

	case 2:
		<% print_popup_realm(2, 0, 3); %>
	break;

	case 3:
		<% print_popup_realm(3, 0, 3); %>
	break;

	case 4:
		<% print_popup_realm(4, 0, 3); %>
	break;

	case 5:
		<% print_popup_realm(5, 0, 3); %>
	break;
	}
	eapauthWindow = window.open("", "", "toolbar=no,scrollbars=yes,width=600,height=500,modal=yes");
	eapauthWindow.document.write(test);
	realmindex = index;
}

//Function for deleting modify Realm List popup
function ondelete_eapauth(index)
{
	document.getElementById("wl_realmlist_"+ index +"_0").value = "";
	document.getElementById("wl_realmlist_"+ index +"_2").value = "";
	document.getElementById("wl_realmlist_"+ index +"_5").value = "";
}

//Function for closing modify Realm List popup
function onclose_eapauth()
{
	eapauthWindow.close();
}

//Function for updating main UI from modify Realm List popup
function onsave_eapauth()
{
	var eap_id, auth_id, auth_param_id;
	var eaptext = '', eaptextencoding = '';
	var authIdType;
	for(eap_idx = 0; eap_idx < 4; eap_idx++)
	{
		eap_id = "eap_method_" + eap_idx;
		auth_id = "auth_id_" + eap_idx;
		auth_param_id = "auth_param_" + eap_idx;

		var eapobj = eapauthWindow.document.getElementById(eap_id);
		var selectedeap = eapobj.options[eapobj.selectedIndex].value;

		var authidobj = eapauthWindow.document.getElementById(auth_id);
		var selectedauthidobj = authidobj.options[authidobj.selectedIndex].value;

		var authprmobj = eapauthWindow.document.getElementById(auth_param_id);
		var selectedauthprmobj = authprmobj.options[authprmobj.selectedIndex].value;

		if(selectedeap == '255' || selectedauthprmobj == '-1')
			continue;
		else
		{
			if((eap_idx > 0) && (eaptext.length > 1))
			{
				eaptext = eaptext +";";
				eaptextencoding = eaptextencoding + ";";
			}

			authIdType = AuthIDtoStr(selectedauthidobj,false);
			eaptext = eaptext + EaptoStr(selectedeap) + "=" + AuthIDtoStr(selectedauthidobj,true) + "," + AuthParamtoStr(selectedauthprmobj, authIdType);
			eaptextencoding = eaptextencoding + selectedeap + "=" + selectedauthidobj + "," + selectedauthprmobj;
		}

		for(auth_idx = 1; auth_idx < 3; auth_idx++)
		{
			auth_id = "auth_id_"  + eap_idx + "_" + auth_idx;
			auth_param_id = "auth_param_" + eap_idx + "_" + auth_idx;

			authidobj = eapauthWindow.document.getElementById(auth_id);
			selectedauthidobj = authidobj.options[authidobj.selectedIndex].value;

			authprmobj = eapauthWindow.document.getElementById(auth_param_id);
			selectedauthprmobj = authprmobj.options[authprmobj.selectedIndex].value;
			if(selectedauthprmobj == '-1')
				continue;
			else
			{
				authIdType = AuthIDtoStr(selectedauthidobj,false);
				eaptext = eaptext + "#" + AuthIDtoStr(selectedauthidobj,true) + ',' + AuthParamtoStr(selectedauthprmobj,authIdType);
				eaptextencoding = eaptextencoding + "#" + selectedauthidobj + "," + selectedauthprmobj;
			}
		}
	}
	var eapinfofield = document.getElementById("wl_realmlist_"+ realmindex +"_2");
	eapinfofield.value = eaptext;
	var eapcode = document.getElementById("wl_realmlist_"+ realmindex +"_5");
	eapcode.value = eaptextencoding;
	eapauthWindow.close();
}

//Function for mapping eap numbers to eap-values
function EaptoStr(eapval)
{
	var eap;
	switch(Number(eapval))
	{
	case 13:
	eap = "EAP-TLS";
	break;

	case 17:
	eap = "EAP-LEAP";
	break;

	case 18:
	eap = "EAP-SIM";
	break;

	case 21:
	eap = "EAP-TTLS";
	break;

	case 23:
	eap = "EAP-AKA";
	break;

	case 25:
	eap = "EAP-PEAP";
	break;

	case 43:
	eap = "EAP-FAST";
	break;

	case 47:
	eap = "EAP-PSK";
	break;

	case 50:
	eap = "EAP-AKAP";
	break;

	case 254:
	eap = "EAP-EXPANDED";
	break;
	}
	return eap;
}

//Function for mapping auth id numbers to auth id -values
function AuthIDtoStr(authid,togglemapping)
{
	var authid_val;
	var authidtype;
	switch(Number(authid))
	{
	case 2:
	authid_val = "NonEAPInner";
	authidtype = 1;
	break;

	case 3:
	authid_val = "InnerAuthEAP";
	authidtype = 2;
	break;

	case 5:
	authid_val = "Credential";
	authidtype = 3;
	break;

	case 6:
	authid_val = "TunnledEAPCred";
	authidtype = 4;
	break;
	}
	return (togglemapping == true ? authid_val : authidtype);
}

//Function for mapping auth param numbers to auth param-values
function AuthParamtoStr(authprmid, authidtype)
{
	var authprm;

	if(authidtype == 1) {
		switch(Number(authprmid))
		{
		case 1:
		authprm = "PAP";
		break;

		case 2:
		authprm = "CHAP";
		break;

		case 3:
		authprm = "MSCHAP";
		break;

		case 4:
		authprm = "MSCHAPV2";
		break;
		}
	} else if(authidtype == 2) {
		switch(Number(authprmid))
		{
		case 13:
		authprm = "EAP-TLS";
		break;

		case 17:
		authprm = "LEAP";
		break;

		case 18:
		authprm = "EAP-SIM";
		break;

		case 21:
		authprm = "EAP-TTLS";
		break;

		case 23:
		authprm = "EAP-AKA";
		break;

		case 25:
		authprm = "PEAP";
		break;

		case 43:
		authprm = "EAP-FAST";
		break;
		}
	} else if(authidtype == 3 || authidtype == 4) {
		switch(Number(authprmid))
		{
		case 1:
		authprm = "SIM";
		break;

		case 2:
		authprm = "USIM";
		break;

		case 3:
		authprm = "NFC";
		break;

		case 4:
		authprm = "HARDTOKEN";
		break;

		case 5:
		authprm = "SOFTTOKEN";
		break;

		case 6:
		authprm = "CERTIFICATE";
		break;

		case 7:
		authprm = "USERNAME_PASSWORD";
		break;

		case 8:
		authprm = "RESERVED";
		break;

		case 9:
		authprm = "ANNONYMOUS";
		break;

		case 10:
		authprm = "VENDOR_SPECIFIC";
		break;
		}
	}
	return authprm;
}

//Function for Auth Id selection change in modify Realm List
function onchange_authid(authidName,authidopt)
{
	var id = ( (authidopt == 1000) ? ("auth_id_" + authidName) : ("auth_id_" + authidName + "_" + authidopt) );
	var selectedauthid = eapauthWindow.document.getElementById(id);
	var selectedauthidval = selectedauthid.options[selectedauthid.selectedIndex].value;
	var index = Number(selectedauthidval);
	switch(index)
	{
	case 2:
		<% authid_change(0); %>
	break;
	case 3:
		<% authid_change(1); %>
	break;
	case 5:
		<% authid_change(2); %>
	break;
	case 6:
		<% authid_change(3); %>
	break;
	}

	var authparamid = ( (authidopt == 1000) ? ("auth_param_" + authidName) : ("auth_param_" + authidName + "_" + authidopt) );
	var authparam  = eapauthWindow.document.getElementById(authparamid);

	authparam.length = authTypes.length;

	for(i=0;i<authTypes.length;i++)
	{
		authparam[i] = new Option(authTypes[i], authTypes[i]);
		authparam[i].value = authTypesCode[i];
	}

	authparam[0].selected = true;
}

//functions to populate venue type on venue group change event.
function onchange_vanuegrp()
{
	var selectedVanueGroup = document.forms[0].wl_venuegrp[document.forms[0].wl_venuegrp.selectedIndex].value;
	var indx = Number(selectedVanueGroup);

	switch(indx){
	case 0:
	<% vanuegrp_change(0); %>
	break;

	case 1:
	<% vanuegrp_change(1); %>
	break;

	case 2:
	<% vanuegrp_change(2); %>
	break;

	case 3:
	<% vanuegrp_change(3); %>
	break;

	case 4:
	<% vanuegrp_change(4); %>
	break;

	case 5:
	<% vanuegrp_change(5); %>
	break;

	case 6:
	<% vanuegrp_change(6); %>
	break;

	case 7:
	<% vanuegrp_change(7); %>
	break;

	case 8:
	<% vanuegrp_change(8); %>
	break;

	case 9:
	<% vanuegrp_change(9); %>
	break;

	case 10:
	<% vanuegrp_change(10); %>
	break;

	case 11:
	<% vanuegrp_change(11); %>
	break;

	case 12:
	<% vanuegrp_change(12);%>
	break;
	}

	document.forms[0].wl_venuetype.length = groupTypes.length;
	for(i=0;i<groupTypes.length;i++) {
		document.forms[0].wl_venuetype[i] = new Option(groupTypes[i], groupTypes[i]);
		document.forms[0].wl_venuetype[i].value = groupTypesCode[i];
	}

	document.forms[0].wl_venuetype[0].selected = true;
}

//functions for disable/enable conytrols based on wl_u11en flag
function recalc_ANQPParamsCtrls()
{
	if(document.forms[0].wl_u11en[document.forms[0].wl_u11en.selectedIndex].text == "Disabled"
		|| document.forms[0].wl_u11en.disabled == 1)
		update_ANQPParamsCtrls(true);
	else
		update_ANQPParamsCtrls(false);
}

function update_ANQPParamsCtrls(result)
{
	document.forms[0].wl_iwint.disabled = result == true ? 1 : 0;
	document.forms[0].wl_iwasra.disabled = result == true ? 1 : 0;
	document.forms[0].wl_iwnettype.disabled = result == true ? 1 : 0;
	document.forms[0].wl_hessid.disabled = result == true ? 1 : 0;
	document.forms[0].wl_ipv4addr.disabled = result == true ? 1 : 0;
	document.forms[0].wl_ipv6addr.disabled = result == true ? 1 : 0;
	EnableCtrlByID("p2pieparams","select",result);
	EnableCtrlByID("netauthlist","select",result);
	EnableCtrlByID("netauthlist","input",result);
	recalc_netauthlist();
	EnableCtrlByID("venueinfolist","select",result);
	EnableCtrlByID("venueinfolist","input",result);
	EnableCtrlByID("ouilist","input",result);
	EnableCtrlByID("3gpplist","input",result);
	EnableCtrlByID("domainlist","input",result);
	EnableCtrlByID("realmlist","input",result);
	EnableCtrlByID("realmlist","select",result);
	document.forms[0].wl_hs2en.disabled = result == true ? 1 : 0;
	recalc_HSParamsCtrls();
}

//function to call onchange_netauthtype for all items
function recalc_netauthlist()
{
	for(i=0; i<4; i++){
		onchange_netauthtype(i);
	}
}

//functions for disable/enable conytrols based on wl_hs2en flag
function recalc_HSParamsCtrls()
{
	if(document.forms[0].wl_hs2en[document.forms[0].wl_hs2en.selectedIndex].text == "Disabled"
		|| document.forms[0].wl_hs2en.disabled == 1)
		update_HSParamsCtrls(true);
	else
		update_HSParamsCtrls(false);
}

function update_HSParamsCtrls(result)
{
	document.forms[0].wl_hs2cap.disabled = result == true ? 1 : 0;
	document.forms[0].wl_dgaf.disabled = result == true ? 1 : 0;
	document.forms[0].wl_proxyarp.disabled = result == true ? 1 : 0;
	EnableCtrlByID("operatingclass","select",result);
	EnableCtrlByID("anonymousnai","input",result);
	EnableCtrlByID("wanmatrics","select",result);
	EnableCtrlByID("wanmatrics","input",result);
	EnableCtrlByID("qosmapie","input",result);
	EnableCtrlByID("oplist","input",result);
	EnableCtrlByID("naihrqlist","select",result);
	EnableCtrlByID("naihrqlist","input",result);
	EnableCtrlByID("concaplistid","input",result);
	EnableCtrlByID("concaplistid","select",result);
	EnableCtrlByID("osuplistid","input",result)
}

function EnableCtrlByID(id,tag,result)
{
	var elements = document.getElementById(id).getElementsByTagName(tag);
	for(i=0;i<elements.length;i++)
		elements[i].disabled = result == true ? 1 : 0;
}

//functions to enable/disable wl_netauthlist control on change event.
function onchange_netauthtype(idx)
{
	var elem = document.getElementById("wl_netauthlist_" + idx + "_0");
	var selectedelem = elem.options[elem.selectedIndex].value;
	if(selectedelem == "httpred" || selectedelem == "dnsred") {
		document.getElementById("wl_netauthlist_" + idx + "_1").removeAttribute("readOnly", "");
	} else {
		document.getElementById("wl_netauthlist_" + idx + "_1").setAttribute("readOnly", "");
		document.getElementById("wl_netauthlist_" + idx + "_1").value = "";
	}
}

function reset_passpoint()
{
	setTimeout(function(){recalc_ANQPParamsCtrls();},200);
}

var xmlhttp;

function handler()
{
	if (xmlhttp.readyState == 4) {
		document.getElementById("iconform").submit();
	}
}

function SendIconDeleteRequest(param)
{
	var url = "iconupload.cgi" + "?" + param;
	if (window.XMLHttpRequest) {
		/* code for IE7+, Firefox, Chrome, Opera, Safari */
		xmlhttp=new XMLHttpRequest();
	}
	else {
	/* code for IE6, IE5 */
		xmlhttp=new ActiveXObject("Microsoft.XMLHTTP");
	}
	xmlhttp.open("GET", url, false);
	xmlhttp.onreadystatechange = handler;
	xmlhttp.send();
}

function onsubmit_iconinfo()
{
	var idx = 0, element;
	var outstring = "";
	for(idx = 10; idx < 15; idx++) {
		element = document.getElementById("icon_del_" + idx);
		if(element != null && element.checked == true){
		 if(outstring.length != 0)
			outstring = outstring + ";";
		 outstring = outstring +document.getElementById("icon_" + idx).value;
		}
	}
	if(outstring.length != 0)
		SendIconDeleteRequest(outstring);
	else
		document.getElementById("iconform").submit();
}

/* For each check-box need to check the valid count. */
function validate_max_3_oui()
{
	var count = 0;
	if(document.forms[0].wl_ouilist_0_1.checked)
		count++;

	if(document.forms[0].wl_ouilist_1_1.checked)
		count++;

	if(document.forms[0].wl_ouilist_2_1.checked)
		count++;

	if(document.forms[0].wl_ouilist_3_1.checked)
		count++;

	if(document.forms[0].wl_ouilist_4_1.checked)
		count++;

	if(document.forms[0].wl_ouilist_5_1.checked)
		count++;

	document.getElementById("wl_ouilist_bc").value = count;
}
</script>
</head>
<body onload="wl_recalc()">
<div id="overDiv" style="position:absolute; visibility:hidden; z-index:1000;"></div>
<input type=hidden name="wl_mode_changed" value=0>
<input type=hidden name="wl_ure_changed" value=0>

<table id="page_header" border="0" cellpadding="0" cellspacing="0" width="100%" bgcolor="#cc0000">
	<% asp_list(); %>
</table>

<table border="0" cellpadding="0" cellspacing="0" width="100%">
	<tr class="page_title">
		<td colspan="2" class="edge"><img border="0" src="blur_new.jpg" alt=""></td>
	</tr>
	<tr>
		<td class="page_title"><img border="0" src="logo_new.gif" alt=""></td>
		<td width="100%" valign="top">
		<br>
		<span class="title">Passpoint</span><br>
		<span class="subtitle">This page allows you to configure the Passpoint parameters for each Virtual/Physical interface.</span>
		</td>
	</tr>
</table>

<form method="post" action="passpoint.asp" onreset="reset_passpoint()">
<input type="hidden" name="page" value="passpoint.asp">
<p>
<table border="0" cellpadding="0" cellspacing="0">
	<tr>
		<th width="310"
		onMouseOver="return overlib('Selects which wireless interface to configure.', LEFT);"
		onMouseOut="return nd();">
		Wireless Interface:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td>
		<select name="wl_unit" onChange="submit();">
		<% wl_list("INCLUDE_SSID" , "INCLUDE_VIFS"); %>
		</select>
		</td>
		<td>
		<button type="submit" name="action" value="Select">Select</button>
		</td>
	</tr>
</table>
<p>
<table border="0" cellpadding="0" cellspacing="0">
	<tr>
		<th width="310">
		ANQP Elements:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td><B>802.11u ANQP Parameters</B></td>
	</tr>

	<tr>
		<th width="310"
		onMouseOver="return overlib('Enables or disables 802.11u Interworking Capability for this SSID .', LEFT);"
		onMouseOut="return nd();">
		<input type="hidden" name="wl_hsflag">
		802.11u Status:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td>
		<select name="wl_u11en" onChange="recalc_ANQPParamsCtrls();">
		<option value="0" <% nvram_match_bitflag("wl_hsflag", 2, "0", "selected"); %>>Disabled</option>
		<option value="1" <% nvram_match_bitflag("wl_hsflag", 2, "1", "selected"); %>>Enabled</option>
		</select>
		</td>
	</tr>

	<tr>
		<th width="310"
		onMouseOver="return overlib('Enables or disables Internet Access Capability for this SSID .', LEFT);"
		onMouseOut="return nd();">
		Internet Access:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td>
		<select name="wl_iwint">
		<option value="0" <% nvram_match_bitflag("wl_hsflag", 3, "0", "selected"); %>>Disabled</option>
		<option value="1" <% nvram_match_bitflag("wl_hsflag", 3, "1", "selected"); %>>Enabled</option>
		</select>
		</td>
	</tr>

	<tr>
		<th width="310"
		onMouseOver="return overlib('Enables or disables ASRA field for this SSID .', LEFT);"
		onMouseOut="return nd();">
		Additional Step Required for Access (ASRA):&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td>
		<select name="wl_iwasra">
		<option value="0" <% nvram_match_bitflag("wl_hsflag", 4, "0", "selected"); %>>Disabled</option>
		<option value="1" <% nvram_match_bitflag("wl_hsflag", 4, "1", "selected"); %>>Enabled</option>
		</select>
		</td>
	</tr>
	<tr>
		<th width="310"
		onMouseOver="return overlib('Set Network Access Type to the BSS supported by the interface."
		onMouseOut="return nd();">
		Network Access Type:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td>
		<select name="wl_iwnettype">
		<option value="0" <%  nvram_match("wl_iwnettype", "0",  "selected"); %>>Private Network</option>
		<option value="1" <%  nvram_match("wl_iwnettype", "1",  "selected"); %>>Private Network with Guest Access</option>
		<option value="2" <%  nvram_match("wl_iwnettype", "2",  "selected"); %>>Chargable Public Network</option>
		<option value="3" <%  nvram_match("wl_iwnettype", "3",  "selected"); %>>Free Public Network</option>
		<option value="4" <%  nvram_match("wl_iwnettype", "4",  "selected"); %>>Emergency Services Only Network</option>
		<option value="5" <%  nvram_match("wl_iwnettype", "5",  "selected"); %>>Personal Device Network</option>
		<option value="14" <% nvram_match("wl_iwnettype", "14", "selected"); %>>Test or Experimental</option>
		<option value="15" <% nvram_match("wl_iwnettype", "15", "selected"); %>>Wildcard</option>
		</select>
		</td>
	</tr>

	<tr>
		<th width="310"
		onMouseOver="return overlib('Sets Interworking HESSID of this network.', LEFT);"
		onMouseOut="return nd();">
		Interworking HESSID:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td><input name="wl_hessid" value="<% nvram_get("wl_hessid"); %>" size="17" maxlength="17"></td>
	</tr>

</table>

<p>
<table id="p2pieparams" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<th width="310">
		P2P IE:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td><B>P2P IE Parameters</B></td>
	</tr>

	<tr>
		<th width="310"
		onMouseOver="return overlib('Enables or disables P2P IE Capability for this SSID .', LEFT);"
		onMouseOut="return nd();">
		P2P IE Status:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td>
		<select name="wl_p2pie" onChange="recalc_ANQPParamsCtrls();">
		<option value="0" <% nvram_match_bitflag("wl_hsflag", 7, "0", "selected"); %>>Disabled</option>
		<option value="1" <% nvram_match_bitflag("wl_hsflag", 7, "1", "selected"); %>>Enabled</option>
		</select>
		</td>
	</tr>

	<tr>
		<th width="310"
		onMouseOver="return overlib('Enables or disables P2P Cross Capability for this SSID .', LEFT);"
		onMouseOut="return nd();">
		P2P Cross Status:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td>
		<select name="wl_p2pcross" onChange="recalc_ANQPParamsCtrls();">
		<option value="0" <% nvram_match_bitflag("wl_hsflag", 8, "0", "selected"); %>>Disabled</option>
		<option value="1" <% nvram_match_bitflag("wl_hsflag", 8, "1", "selected"); %>>Enabled</option>
		</select>
		</td>
	</tr>
</table>

<p>
<table border="0" cellpadding="0" cellspacing="0">
	<tr>
		<th width="310"
		onMouseOver="return overlib('Set IP Address Type Availability Information to the BSS supported by the interface.', LEFT);"
		onMouseOut="return nd();">
		IP Address Type Availability Information:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td></td>
	</tr>
	<tr>
		<th width="310"
		onMouseOver="return overlib('Set IPv4 Address Type Availability Information to the BSS supported by the interface.', LEFT);"
		onMouseOut="return nd();">
		IPv4:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td>
		<select name="wl_ipv4addr">
		<option value="0" <%  nvram_match("wl_ipv4addr", "0", "selected"); %>>Not Available</option>
		<option value="1" <%  nvram_match("wl_ipv4addr", "1", "selected"); %>>Public</option>
		<option value="2" <%  nvram_match("wl_ipv4addr", "2", "selected"); %>>Port Restricted</option>
		<option value="3" <%  nvram_match("wl_ipv4addr", "3", "selected"); %>>Single NATed Private</option>
		<option value="4" <%  nvram_match("wl_ipv4addr", "4", "selected"); %>>Double NATed Private</option>
		<option value="5" <%  nvram_match("wl_ipv4addr", "5", "selected"); %>>Port Restricted,Single NATed Private</option>
		<option value="6" <%  nvram_match("wl_ipv4addr", "6", "selected"); %>>Port Restricted,Double NATed Private</option>
		<option value="7" <%  nvram_match("wl_ipv4addr", "7", "selected"); %>>Unknown Availability</option>
		</select>
		</td>
	</tr>
	<tr>
		<th width="310"
		onMouseOver="return overlib('Set IPv6 Address Type Availability Information to the BSS supported by the interface.', LEFT);"
		onMouseOut="return nd();">
		IPv6:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td>
		<select name="wl_ipv6addr">
		<option value="0" <%  nvram_match("wl_ipv6addr", "0", "selected"); %>>Not Available</option>
		<option value="1" <%  nvram_match("wl_ipv6addr", "1", "selected"); %>>Available</option>
		<option value="2" <%  nvram_match("wl_ipv6addr", "2", "selected"); %>>Unknown Availability</option>
		</select>
		</td>
	</tr>
	<tr>
</table>

<p>
<table id="netauthlist" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<th width="310" valign="top" rowspan="1"
		onMouseOver="return overlib('Set Network Authentication Type List to the BSS supported by the interface.', LEFT);"
		onMouseOut="return nd();">
		<input type="hidden" name="wl_netauthlist" value="4">
		Network Authentication Type List:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td class="label">Auth Type</td>
		<td></td>
		<td class="label">Redirect URL</td>
	</tr>
	<% print_wl_netauthlist(0, 3); %>
</table>

<p>
<table id="realmlist" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<th width="310" valign="top" rowspan="1"
		onMouseOver="return overlib('Set NAI Realm List to the BSS supported by the interface.', LEFT);"
		onMouseOut="return nd();">
		<input type="hidden" name="wl_realmlist" value="6">
		Realm List:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td class="label">Realm Name</td>
		<td></td>
		<td class="label">Encoding</td>
		<td></td>
		<td class="label">Eap and Auth Information</td>
		<td></td>
		<td class="label"></td>
	</tr>
	<% print_wl_realmlist(0, 5); %>
</table>

<p>
<table id="venueinfolist" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<th width="310"
		onMouseOver="return overlib('Set Venue Information to the BSS supported by the interface.', LEFT);"
		onMouseOut="return nd();">
		Venue Information:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td></td>
	</tr>
	<% print_wl_venuegrp_type(); %>

	<tr>
		<th width="310" valign="top" rowspan="1"
		onMouseOver="return overlib('Set Venue Name List to the BSS supported by the interface.', LEFT);"
		onMouseOut="return nd();">
		<input type="hidden" name="wl_venuelist" value="2">
		Venue Name List:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td class="label">Venue Name</td>
		<td></td>
		<td class="label">Language Code</td>
	</tr>
	<% print_wl_venuelist(0, 1); %>

</table>

<p>
<table id="ouilist" border="0" cellpadding="0" cellspacing="0">
	<tr>
	<th width="310" valign="top" rowspan="1"
		onMouseOver="return overlib('Set Roaming Consortium List to the BSS supported by the interface.', LEFT);"
		onMouseOut="return nd();">
		<input type="hidden" name="wl_ouilist" value="6">
		Roaming Consortium List:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td class="label">OUI Name</td>
		<td class="label">Is Beacon</td>
	</tr>
	<% print_wl_ouilist(0, 5); %>
</table>

<p>
<table id="3gpplist" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<th width="310" valign="top" rowspan="1"
		onMouseOver="return overlib('Set 3GPP Cellular Network Information List to the BSS supported by the interface.', LEFT);"
		onMouseOut="return nd();">
		<input type="hidden" name="wl_3gpplist" value="6">
		3GPP Cellular Network Information List:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td class="label">Country Code</td>
		<td></td>
		<td class="label">Network Code</td>
	</tr>
	<% print_wl_3gpplist(0, 5); %>
</table>

<p>
<table id="domainlist" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<th width="310" valign="top" rowspan="8"
		onMouseOver="return overlib('Set the List of Domain Names to the BSS supported by the interface.', LEFT);"
		onMouseOut="return nd();">
		<input type="hidden" name="wl_domainlist" value="8">
		Domain Name List:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td><input name="wl_domainlist0" value="<% nvram_list("wl_domainlist", 0); %>" size="32" maxlength="32"></td>
		<td><input name="wl_domainlist1" value="<% nvram_list("wl_domainlist", 1); %>" size="32" maxlength="32"></td>
	</tr>
	<tr>
		<td>&nbsp;&nbsp;</td>
		<td><input name="wl_domainlist2" value="<% nvram_list("wl_domainlist", 2); %>" size="32" maxlength="32"></td>
		<td><input name="wl_domainlist3" value="<% nvram_list("wl_domainlist", 3); %>" size="32" maxlength="32"></td>
	</tr>
	<tr>
		<td>&nbsp;&nbsp;</td>
		<td><input name="wl_domainlist4" value="<% nvram_list("wl_domainlist", 4); %>" size="32" maxlength="32"></td>
		<td><input name="wl_domainlist5" value="<% nvram_list("wl_domainlist", 5); %>" size="32" maxlength="32"></td>
	</tr>
	<tr>
		<td>&nbsp;&nbsp;</td>
		<td><input name="wl_domainlist6" value="<% nvram_list("wl_domainlist", 6); %>" size="32" maxlength="32"></td>
		<td><input name="wl_domainlist7" value="<% nvram_list("wl_domainlist", 7); %>" size="32" maxlength="32"></td>
	</tr>
</table>

<!--
#ifdef __CONFIG_HSPOT__
-->
<p>
<table border="0" cellpadding="0" cellspacing="0">
	<tr>
		<th width="310">
		ANQP Elements:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td><B>Passpoint ANQP Parameters</B></td>
	</tr>

	<tr>
		<th width="310"
		onMouseOver="return overlib('Enables or disables Passpoint Capability for this SSID .', LEFT);"
		onMouseOut="return nd();">
		Passpoint Status:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td>
		<select name="wl_hs2en" onChange="recalc_HSParamsCtrls();">
		<option value="0" <% nvram_match_bitflag("wl_hsflag", 0, "0", "selected"); %>>Disabled</option>
		<option value="1" <% nvram_match_bitflag("wl_hsflag", 0, "1", "selected"); %>>Enabled</option>
		</select>
		</td>
	</tr>

	<tr>
		<th width="310"
		onMouseOver="return overlib('Sets Passpoint Capability for this SSID .', LEFT);"
		onMouseOut="return nd();">
		Passpoint Capability:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td>
		<select name="wl_hs2cap">
		<option value="0" <% nvram_match("wl_hs2cap", "0", "selected"); %>>Release 1</option>
		<option value="1" <% nvram_match("wl_hs2cap", "1", "selected"); %>>Release 2</option>
		</select>
		</td>
	</tr>

	<tr>
		<th width="310"
		onMouseOver="return overlib('Enables or disables DGAF Capability for this SSID .', LEFT);"
		onMouseOut="return nd();">
		DGAF Status:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td>
		<select name="wl_dgaf" onChange="recalc_ANQPParamsCtrls();">
		<option value="0" <% nvram_match_bitflag("wl_hsflag", 10, "0", "selected"); %>>Disabled</option>
		<option value="1" <% nvram_match_bitflag("wl_hsflag", 10, "1", "selected"); %>>Enabled</option>
		</select>
		</td>
	</tr>

	<tr>
		<th width="310"
		onMouseOver="return overlib('Enables or disables Proxy ARP Capability for this SSID .', LEFT);"
		onMouseOut="return nd();">
		Proxy ARP Status:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td>
		<select name="wl_proxyarp" onChange="recalc_ANQPParamsCtrls();">
		<option value="0" <% nvram_match_bitflag("wl_hsflag", 9, "0", "selected"); %>>Disabled</option>
		<option value="1" <% nvram_match_bitflag("wl_hsflag", 9, "1", "selected"); %>>Enabled</option>
		</select>
		</td>
	</tr>
</table>

<p>
<table id="operatingclass" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<th width="310"
		onMouseOver="return overlib('Set Operating Class Indication to the BSS supported by the interface.', LEFT);"
		onMouseOut="return nd();">
		Operating Class Indication:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td>
		<select name="wl_opercls">
		<option value="1" <% nvram_match("wl_opercls", "1", "selected"); %>>Operating Class 81</option>
		<option value="2" <% nvram_match("wl_opercls", "2", "selected"); %>>Operating Class 115</option>
		<option value="3" <% nvram_match("wl_opercls", "3", "selected"); %>>Operating Class 81 & 115</option>
		</select>
		</td>
	</tr>
</table>

<p>
<table id="anonymousnai" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<th width="310"
		onMouseOver="return overlib('Sets the Anonymous NAI of this network.', LEFT);"
		onMouseOut="return nd();">
		Anonymous NAI:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td><input name="wl_anonai" value="<% nvram_get("wl_anonai"); %>" size="32" maxlength="32"></td>
	</tr>
</table>

<p>
<table id="qosmapie" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<th width="310" valign="top" rowspan="1"
		onMouseOver="return overlib('Set QoS Map Set IE's DSCP Exception and DSCP Range to the BSS supported by the interface.', LEFT);"
		onMouseOut="return nd();">
		<input type="hidden" name="wl_qosmapie" value="8">
		QoS Map Set IE:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td></td>
	</tr>

	<tr>
		<th width="310"
		onMouseOver="return overlib('Set QoS Map Set IE's DSCP Exception and DSCP Range to the BSS supported by the interface.', LEFT);"
		onMouseOut="return nd();">
		&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
	</tr>

	<% print_wl_qosmapie(0, 7); %>

</table>

<p>
<table id="wanmatrics" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<th width="310" valign="top" rowspan="1"
		onMouseOver="return overlib('Set WAN Metrics Information to the BSS supported by the interface.', LEFT);"
		onMouseOut="return nd();">
		<input type="hidden" name="wl_wanmetrics" value="1">
		WAN Metrics Information:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td class="label">Link Status</td>
		<td></td>
		<td class="label">Symmetric Link</td>
		<td></td>
		<td class="label">At Capacity</td>
		<td></td>
		<td class="label">DownLink Speed</td>
		<td></td>
		<td class="label">UpLink Speed</td>
		<td></td>
		<td class="label">DownLink Load</td>
		<td></td>
		<td class="label">UpLink Load</td>
		<td></td>
		<td class="label">Lmd</td>
	</tr>

	<% print_wl_wanmetrics(0, 0); %>

</table>

<p>
<table id="oplist" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<th width="310" valign="top" rowspan="1"
		onMouseOver="return overlib('Set Operator Friendly Name List to the BSS supported by the interface.', LEFT);"
		onMouseOut="return nd();">
		<input type="hidden" name="wl_oplist" value="4">
		Operator Friendly Name List:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td class="label">Operator Name</td>
		<td></td>
		<td class="label">Language Code</td>
	</tr>

	<% print_wl_oplist(0, 3); %>

</table>

<p>
<table id="naihrqlist" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<th width="310" valign="top" rowspan="1"
		onMouseOver="return overlib('Set NAI Home Realm Query List to the BSS supported by the interface.', LEFT);"
		onMouseOut="return nd();">
		<input type="hidden" name="wl_homeqlist" value="4">
		NAI Home Realm Query List:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td class="label">Home Realm</td>
		<td></td>
		<td class="label">Encoding</td>
	</tr>

	<% print_wl_homeqlist(0, 3); %>

</table>

<p>
<table id="concaplistid" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<th width="310" valign="top" rowspan="1"
		onMouseOver="return overlib('Set Connection Capability List to the BSS supported by the interface.', LEFT);"
		onMouseOut="return nd();">
		<input type="hidden" name="wl_concaplist" value="12">
		Connection Capability List:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td class="label">Protocol</td>
		<td></td>
		<td class="label">Port</td>
		<td></td>
		<td class="label">Status</td>
	</tr>
	<% print_wl_concaplist(0, 11); %>
</table>

<p>
<table id="osuplistid" border="0" cellpadding="0" cellspacing="0">

	<tr>
		<th width="310"
		onMouseOver="return overlib('Set OSU Provider List to the BSS supported by the interface.', LEFT);"
		onMouseOut="return nd();">
		OSU Provider List:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
	</tr>

	<tr>
		<th width="310"
		onMouseOver="return overlib('Sets OSU Provider List's SSID of this network.', LEFT);"
		onMouseOut="return nd();">
		OSU SSID:&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td><input name="wl_osu_ssid" value="<% nvram_get("wl_osu_ssid"); %>" size="17" maxlength="255"></td>
	</tr>

	<tr>
		<th width="310"
		onMouseOver="return overlib('Set OSU Provider List to the BSS supported by the interface.', LEFT);"
		onMouseOut="return nd();">
		&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
	</tr>

	<tr>
		<th width="310"
		onMouseOver="return overlib('Set OSU Provider List to the BSS supported by the interface.', LEFT);"
		onMouseOut="return nd();">
		<input type="hidden" name="wl_osu_uri" value="4">
		<input type="hidden" name="wl_osu_nai" value="4">
		<input type="hidden" name="wl_osu_method" value="4">
		<input type="hidden" name="wl_osu_frndname" value="4">
		<input type="hidden" name="wl_osu_servdesc" value="4">
		<input type="hidden" name="wl_osu_icons" value="4">
		&nbsp;&nbsp;
		</th>
		<td>&nbsp;&nbsp;</td>
		<td class="label">OSU Friendly Name</td>
		<td></td>
		<td class="label">OSU Server URI</td>
		<td></td>
		<td class="label">OSU NAI</td>
		<td></td>
		<td class="label">OSU Method</td>
		<td></td>
		<td class="label">OSU Icon</td>
		<td></td>
		<td class="label"></td>
	</tr>
	<% print_wl_osuplist(0, 3); %>
</table>

<!--
#endif // endif
-->

<p>
<table border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td width="310"></td>
		<td>&nbsp;&nbsp;</td>
		<td>
		<input type="submit" name="action" value="Apply">
		<input type="reset" name="action" value="Cancel">
		</td>
	</tr>
</table>
</form>

<form id="iconform" method="post" action="iconupload.cgi" enctype="multipart/form-data">
	<% print_iconlist(0, 14); %>
</form>

<p class="label">&#169;2001-2016 Broadcom Limited. All rights reserved. 54g and XPress are trademarks of Broadcom Limited.</p>
</body>

<script>
window.onload = function(){recalc_ANQPParamsCtrls();};
</script>
</html>
