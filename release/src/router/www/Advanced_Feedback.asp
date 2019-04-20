<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>

<!--Advanced_Feedback.asp-->

<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title><#Web_Title#> - <#menu_feedback#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="merlin.js"></script>
<script language="JavaScript" type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/oauth.js"></script>
<script type="text/javascript" src="js/httpApi.js"></script>
<style>
.noUSBHint, .storeUSBHint {
	color: #FC0;
	margin-left: 10px;
	display: none;
}
.dblog_service_item {
	margin-right: 5px;
}
.dblog_enabled_status {
	display: none;
}
.dblog_stop_text {
	cursor: pointer;
	text-decoration: underline;
}
</style>
<script>
var usb_status_last_time = false;
var orig_page = '<% get_parameter("origPage"); %>';
var fb_trans_id = '<% generate_trans_id(); %>';
var default_provider = '<% get_parameter("provider"); %>';
var reload_data = parseInt('<% get_parameter("reload"); %>');
var dblog_trans_id = '<% generate_trans_id(); %>';
var fb_total_size;
function initial(){
	show_menu();
	if(dsl_support){
		change_dsl_diag_enable(0);
		document.getElementById("fb_desc1").style.display = "";
		inputCtrl(document.form.fb_ptype, 0);
		inputCtrl(document.form.fb_pdesc, 0);
		
	}
	else{
		document.getElementById("fb_desc0").style.display = "";
		inputCtrl(document.form.fb_ISP, 0);
		inputCtrl(document.form.fb_Subscribed_Info, 0);
		document.form.attach_syslog_id.checked = true;
		document.form.attach_cfgfile_id.checked = true;
		document.form.attach_iptables.checked = false;
		document.form.attach_modemlog.checked = true;
		document.form.attach_wlanlog_id.checked = true;
		document.getElementById("attach_iptables_span").style.display = "none";		
		inputCtrl(document.form.dslx_diag_enable[0], 0);
		inputCtrl(document.form.dslx_diag_enable[1], 0);
		inputCtrl(document.form.dslx_diag_duration, 0);
		inputCtrl(document.form.fb_availability, 0);
		
		gen_ptype_list(orig_page);
		Reload_pdesc(document.form.fb_ptype,orig_page);
	}		

	if(modem_support == -1 || nomodem_support){
		document.form.attach_modemlog.checked = false;
		document.getElementById("attach_modem_span").style.display = "none";
	}

	//Renjie: do not check WAN connection
	//setTimeout("check_wan_state();", 300);

	if(dblog_support)
		init_diag_feature();
	else {
		$(".dblog_support_class").remove();
	}

	var fb_email_provider = '<% nvram_get("fb_email_provider"); %>';
	if(fb_email_provider=="" && default_provider!=""){
		document.form.fb_email_provider.value = default_provider;	
	}
	else{
		document.form.fb_email_provider.value = fb_email_provider;
	}
	change_fb_email_provider();

	if(reload_data==1){
		document.form.fb_country.value = decodeURIComponent('<% nvram_char_to_ascii("", "fb_country"); %>');
		document.form.fb_ptype.value = decodeURIComponent('<% nvram_char_to_ascii("", "fb_ptype"); %>');
		Reload_pdesc(document.form.fb_ptype);
		document.form.fb_pdesc.value = decodeURIComponent('<% nvram_char_to_ascii("", "fb_pdesc"); %>');
		document.form.fb_comment.value = decodeURIComponent('<% nvram_char_to_ascii("", "fb_comment"); %>');
	}

	$("#oauth_google_btn").click(
		function() {
			oauth.google(onGoogleLogin);
		}
	);

	httpApi.nvramGetAsync({
		data: ["preferred_lang"],
		success: function(resp){
			var preferredLang = resp.preferred_lang;
			lang_str = (preferredLang == "EN" || preferredLang == "SL") ? "" : (preferredLang.toLowerCase() + '/');

			if(preferredLang == "CN")
				url = "https://www.asus.com.cn/Terms_of_Use_Notice_Privacy_Policy/Privacy_Policy";
			else{
				if(preferredLang == "SV")
					lang_str = "se/";
				else if(preferredLang == "UK")
					lang_str = "ua-ua/";
				else if(preferredLang == "MS")
					lang_str = "my/";
				else if(preferredLang == "DA")
					lang_str = "dk/";

				url = "https://www.asus.com/" + lang_str +"Terms_of_Use_Notice_Privacy_Policy/Privacy_Policy";
			}

			$("#eula_content").find($("a")).attr({
				"href": url
			})
		}
	})
}

function check_wan_state(){
	
	if(sw_mode != 3 && document.getElementById("connect_status").className == "connectstatusoff"){
		document.getElementById("fb_desc_disconnect").style.display = "";
		document.form.fb_country.disabled = "true";
		document.form.fb_email.disabled = "true";
		document.form.fb_serviceno.disabled = "true";
		document.form.attach_syslog.disabled = "true";
		document.form.attach_cfgfile.disabled = "true";
		document.form.attach_modemlog.disabled = "true";
		document.form.attach_wlanlog.disabled = "true";
		document.form.fb_comment.disabled = "true";
		document.form.btn_send.disabled = "true";
		if(dsl_support){
			document.form.fb_ISP.disabled = "true";
			document.form.fb_Subscribed_Info.disabled = "true";
			document.form.attach_iptables.disabled = "true";
			document.form.dslx_diag_enable[0].disabled = "true";
			document.form.dslx_diag_enable[1].disabled = "true";
			document.form.dslx_diag_duration.disabled = "true";
			document.form.fb_availability.disabled = "true";
			
		}
		else{
			document.form.fb_ptype.disabled = "true";
			document.form.fb_pdesc.disabled = "true";
		}		
	}
	else{
		document.getElementById("fb_desc_disconnect").style.display = "none";
		document.form.fb_country.disabled = "";
		document.form.fb_email.disabled = "";
		document.form.fb_serviceno.disabled = "";
		document.form.attach_syslog.disabled = "";
		document.form.attach_modemlog.disabled = "";
		document.form.attach_wlanlog.disabled = "";
		document.form.attach_cfgfile.disabled = "";
		document.form.fb_comment.disabled = "";
		document.form.btn_send.disabled = "";
		if(dsl_support){
			document.form.fb_ISP.disabled = "";
			document.form.fb_Subscribed_Info.disabled = "";
			document.form.attach_iptables.disabled = "";
			document.form.dslx_diag_enable[0].disabled = "";
			document.form.dslx_diag_enable[1].disabled = "";
			document.form.dslx_diag_duration.disabled = "";
			document.form.fb_availability.disabled = "";
			
		}
		else{
			document.form.fb_ptype.disabled = "";
			document.form.fb_pdesc.disabled = "";
		}
	}		
		
	setTimeout("check_wan_state();", 3000);
}

function gen_ptype_list(url){
	ptypelist = new Array();
	ptypelist.push(["<#Select_menu_default#> ...", "No_selected"]);
	ptypelist.push(["<#feedback_setting_problem#>", "Setting_Problem"]);	
	ptypelist.push(["<#feedback_conn_problem#>", "Connection_or_Speed_Problem"]);
	ptypelist.push(["<#feedback_compat_problem#>", "Compatibility_Problem"]);
	ptypelist.push(["<#feedback_translation#>", "Translated_Suggestion"]);
	ptypelist.push(["<#Adaptive_Others#>", "Other_Problem"]);
	free_options(document.form.fb_ptype);
	document.form.fb_ptype.options.length = ptypelist.length;
	for(var i = 0; i < ptypelist.length; i++){
		document.form.fb_ptype.options[i] = new Option(ptypelist[i][0], ptypelist[i][1]);
	}	
	if(url)		//with url : Setting Problem
		document.form.fb_ptype.options[1].selected = "1";
}

function Reload_pdesc(obj, url){
	//alert(obj.value+" ; "+url);
	free_options(document.form.fb_pdesc);
	var ptype = obj.value;
	desclist = new Array();
	url_group = new Array();
	desclist.push(["<#Select_menu_default#> ...","No_selected"]);
	url_group.push(["select"]);//false value
	if(ptype == "Setting_Problem"){
		desclist.push(["<#QIS#>","QIS"]);
		url_group.push(["QIS"]);

		desclist.push(["<#menu1#>","Network Map"]);
		url_group.push(["index"]);

		desclist.push(["<#Guest_Network#>","Guest Network"]);
		url_group.push(["Guest_network"]);

		desclist.push(["<#AiProtection_title#>","AiProtection"]);
		url_group.push(["AiProtection"]);

		desclist.push(["<#Adaptive_QoS#>","Adaptive QoS"]);	//5
		url_group.push(["AdaptiveQoS"]);

		desclist.push(["<#EzQoS_type_traditional#>","Traditional QoS"]);
		url_group.push(["AiProtection"]);

		desclist.push(["<#Menu_TrafficManager#>","<#Traffic_Analyzer#>"]);
		url_group.push(["TrafficMonitor"]);

		desclist.push(["<#Parental_Control#>","Parental Ctrl"]);
		url_group.push(["ParentalControl"]);

		desclist.push(["<#Menu_usb_application#>","USB Application"]);
		url_group.push(["APP_", "AiDisk", "aidisk", "mediaserver", "PrinterServer", "TimeMachine"]);

		desclist.push(["AiCloud","AiCloud"]);	//10
		url_group.push(["cloud"]);

		desclist.push(["<#menu5_1#>","Wireless"]);
		url_group.push(["ACL", "WAdvanced", "Wireless", "WMode", "WSecurity", "WWPS"]);

		desclist.push(["<#menu5_3#>","WAN"]);
		url_group.push(["WAN_", "PortTrigger", "VirtualServer", "Exposed", "NATPassThrough"]);

		desclist.push(["<#dualwan#>","Dual WAN"]);
		url_group.push(["WANPort"]);

		desclist.push(["<#menu5_2#>","LAN"]);
		url_group.push(["LAN", "DHCP", "GWStaticRoute", "IPTV", "SwitchCtrl"]);

		desclist.push(["<#menu5_4_4#>","USB dongle"]);	//15
		url_group.push(["Modem"]);

		desclist.push(["<#DM_title#>","DM"]);
		url_group.push(["DownloadMaster"]);//false value

		desclist.push(["<#menu5_3_6#>","DDNS"]);
		url_group.push(["DDNS"]);

		desclist.push(["IPv6","IPv6"]);
		url_group.push(["IPv6"]);

		desclist.push(["VPN","VPN"]);
		url_group.push(["VPN"]);

		desclist.push(["<#menu5_5#>","Firewall"]);	//20
		url_group.push(["Firewall", "KeywordFilter", "URLFilter"]);

		desclist.push(["<#menu5_6#>","Administration"]);
		url_group.push(["OperationMode", "System", "SettingBackup"]);

		desclist.push(["<#System_Log#>","System Log"]);
		url_group.push(["VPN"]);

		desclist.push(["<#Network_Tools#>","Network Tools"]);
		url_group.push(["Status_"]);

		desclist.push(["Rescue Mode","Rescue"]);
		url_group.push(["Rescue"]);//false value

		desclist.push(["<#feedback_compat_wond#>","Other Devices"]);	//25
		url_group.push(["Other_Device"]);//false value

		desclist.push(["Cannot access firmware page","Fail to access"]);
		url_group.push(["GUI"]);//false value

		desclist.push(["<#menu5_6_3#>","FW update"]);
		url_group.push(["FirmwareUpgrade"]);

	}
	else if(ptype == "Connection_or_Speed_Problem"){
		
		desclist.push(["<#feedback_conn_swls#>","Wireless speed"]);
		desclist.push(["<#feedback_conn_sws#>","Wired speed"]);
		desclist.push(["<#feedback_conn_ucp#>","Unstable connection"]);
		desclist.push(["<#feedback_conn_rra#>","Router reboot"]);
		
	}
	else if(ptype == "Compatibility_Problem"){
		
		desclist.push(["<#feedback_compat_wm#>","modem"]);
		desclist.push(["<#feedback_compat_wor#>","other router"]);
		desclist.push(["<#feedback_compat_oooa#>","OS or Application"]);
		desclist.push(["<#feedback_compat_wp#>","printer"]);
		desclist.push(["<#feedback_compat_wum#>","USB modem"]);
		desclist.push(["<#feedback_compat_wehd#>","external hardware disk"]);
		desclist.push(["<#feedback_compat_wond#>","network devices"]);

	}
	else if(ptype == "Translated_Suggestion"){
		
		desclist.splice(0,1);
		desclist.push(["<#feedback_translation_ts#>","Translation"]);		
	}
	else{	//Other_Problem
		
		desclist.splice(0,1);		
		desclist.push(["<#Adaptive_Others#>","others"]);
	}

	document.form.fb_pdesc.options.length = desclist.length;
	if(obj.value == "Setting_Problem" && url){
		for(var i = 0; i < url_group.length; i++){	
			document.form.fb_pdesc.options[i] = new Option(desclist[i][0], desclist[i][1]);
			//with url : Find pdesc in Setting Problem
			for(var j = 0; j < url_group[i].length; j++){
				if(url.search(url_group[i][j]) >= 0){					
					document.form.fb_pdesc.options[i].selected = "1";
				}
			}		
		}
	}
	else{
		for(var i = 0; i < desclist.length; i++){
			document.form.fb_pdesc.options[i] = new Option(desclist[i][0], desclist[i][1]);
		}	
	}
}

function updateUSBStatus(){	
	if(allUsbStatus.search("storage") == "-1"){			
		document.getElementById("storage_ready").style.display = "none";
		document.getElementById("be_lack_storage").style.display = "";
	}
	else{		
		document.getElementById("storage_ready").style.display = "";
		document.getElementById("be_lack_storage").style.display = "none";
	}
}

function redirect(){
	document.location.href = "Feedback_Info.asp";
}

function applyRule(){
	if(!document.form.eula_checkbox.checked){
		alert('<#feedback_eula_notice#>');
		return false;
	}

		/*if(document.form.feedbackresponse.value == "3"){
				alert("Feedback report daily maximum(10) send limit reached.");
				return false;
		}*/
		if(document.form.attach_syslog.checked == true)
			document.form.PM_attach_syslog.value = 1;
		else
			document.form.PM_attach_syslog.value = 0;
		if(document.form.attach_cfgfile.checked == true)
			document.form.PM_attach_cfgfile.value = 1;
		else
			document.form.PM_attach_cfgfile.value = 0;
		if(document.form.attach_modemlog.checked == true)
			document.form.PM_attach_modemlog.value = 1;
		else
			document.form.PM_attach_modemlog.value = 0;
		if(document.form.attach_wlanlog.checked == true)
			document.form.PM_attach_wlanlog.value = 1;
		else
			document.form.PM_attach_wlanlog.value = 0;
		if(dsl_support){
			if(document.form.attach_iptables.checked == true)
				document.form.PM_attach_iptables.value = 1;
			else
				document.form.PM_attach_iptables.value = 0;
		}	
                
		if(document.form.fb_email.value == ""){
			if(!confirm("<#feedback_email_confirm#>")){
				document.form.fb_email.focus();
				return false;
			}
		}
		else{	//validate email
			if(!isEmail(document.form.fb_email.value)){
				alert("<#feedback_email_alert#>");    					
				document.form.fb_email.focus();
				return false;
			}
		}

		if(fb_trans_id != "")
		{
			document.form.fb_transid.value = fb_trans_id;
		}

		//check Diagnostic
		if(dblog_support) {
			document.form.dblog_tousb.disabled = true;
			document.form.dblog_service.disabled = true;
			document.form.dblog_duration.disabled = true;
			document.form.dblog_transid.disabled = true;
			var dblog_enable_status = httpApi.nvramGet(["dblog_enable"], true).dblog_enable;
			var dblog_enable = getRadioValue($('form[name="form"]').children().find('input[name=dblog_enable]'));
			if(dblog_enable_status == "0" && dblog_enable == "1") {
				var service_list_checked = $("input:checkbox[name=dblog_service_list]:checked").map(function() {
					return $(this).val();
				}).get();
				var dblog_service = 0;
				if(service_list_checked.length == 0) {
					alert("<#feedback_debug_log_noSelected#>");
					return false;
				}
				for(var idx in service_list_checked){
					if(service_list_checked.hasOwnProperty(idx)) {
						dblog_service += parseInt(service_list_checked[idx]);
					}
				}
				document.form.dblog_tousb.disabled = false;
				document.form.dblog_tousb.value = "0";
				if(usb_support) {
					if(allUsbStatus.search("storage") != "-1") {
						if($("input[name=dblog_tousb_cb]").prop("checked")) {
							document.form.dblog_tousb.value = "1";
						}
					}
				}
				document.form.dblog_service.disabled = false;
				document.form.dblog_service.value = dblog_service;
				document.form.dblog_duration.disabled = false;
				document.form.dblog_transid.disabled = false;
				if(dblog_trans_id != "")
					document.form.dblog_transid.value = dblog_trans_id;
			}
		}

		if(document.form.PM_attach_wlanlog.value == "1")
			httpApi.update_wlanlog();

		document.form.fb_browserInfo.value = navigator.userAgent;
		if(dsl_support){
			if(document.form.dslx_diag_enable[0].checked == true){
				document.form.action_wait.value="120";
				showLoading(120);
			}else	
				showLoading(60);
		}
		else{
			startLogPrep();
		}
		document.form.submit();
}

function isEmail(strE) {
	if (strE.search(/^[A-Za-z0-9]+((-[A-Za-z0-9]+)|(\.[A-Za-z0-9]+)|(_[A-Za-z0-9]+))*\@[A-Za-z0-9]+((\.|-)[A-Za-z0-9]+)*\.[A-Za-z]+$/) != -1)
		return true;
	else
		return false;
}

function textCounter(field, cnt, upper) {
        if (field.value.length > upper)
                field.value = field.value.substring(0, upper);
        else
                cnt.value = upper - field.value.length;
}

function change_dsl_diag_enable(value) {
	if(value) {
		if(allUsbStatus.search("storage") == "-1"){
			alert("USB disk required in order to store the debug log, please plug-in a USB disk to <#Web_Title2#> and Enable DSL Line Diagnostic again.");/*untranslated*/
			document.form.dslx_diag_enable[1].checked = true;
			return;
		}
		else{
			alert("<#feedback_capturing_note1#> <#feedback_capturing_note_DSL#>");
		}
		showhide("dslx_diag_duration",1);
	}
	else {
		showhide("dslx_diag_duration",0);
	}
}
function init_diag_feature() {
	var dblog_enable = '<% nvram_get("dblog_enable"); %>';
	var dblog_remaining = parseInt('<% nvram_get("dblog_remaining"); %>');
	setRadioValue($('form[name="form"]').children().find('input[name=dblog_enable]'), dblog_enable);

	if(dblog_enable == "1" && dblog_remaining > 0) {
		$(".dblog_disabled_status").find("input, textarea, button, select").attr("disabled", true);
		$(".dblog_disabled_status").css("display", "none");
		$(".dblog_enabled_status").css("display", "inline");

		var transformTime = function(_sec) {
			var days = Math.floor(dblog_remaining / 60 / 60 / 24);
			var hours = Math.floor(dblog_remaining / 60 / 60 % 24);
			var minutes = Math.floor(dblog_remaining / 60 % 60);
			var seconds = Math.floor(dblog_remaining % 60);
			var remaining_time_str = "<#mssid_time_remaining#> : ";

			if(dblog_remaining == 0) {
				remaining_time_str += "0" + " " + "(Prepare data...)";	//Untranslated
				return remaining_time_str;
			}

			if(days)
				remaining_time_str += days + " <#Day#> ";
			if(hours)
				remaining_time_str += hours + " <#Hour#> ";
			if(minutes)
				remaining_time_str += minutes + " <#Minute#> ";
			if(seconds)
				remaining_time_str += seconds + " <#Second#> ";
			return remaining_time_str;
		};
		$(".dblog_remaining_text").html(transformTime(dblog_remaining));
		var transformTimeInterval = setInterval(function(){
			if(dblog_remaining == 0 || isNaN(dblog_remaining))
				clearInterval(transformTimeInterval);
			else {
				dblog_remaining--;
				$(".dblog_remaining_text").html(transformTime(dblog_remaining));
			}
		}, 1000);

		var dblog_service = parseInt('<% nvram_get("dblog_service"); %>');
		var dblog_service_mapping = ["", "Wi-Fi", "<#DM_title#>", "<#UPnPMediaServer#>", "AiMesh"];
		var dblog_service_text = "";
		for(var i = 1; dblog_service != 0 && i <= 4; i++) {
			if(dblog_service & 1) {
				if(dblog_service_text != "")
					dblog_service_text += ", " + dblog_service_mapping[i];
				else
					dblog_service_text += dblog_service_mapping[i];
			}
			dblog_service = dblog_service >> 1;
		}
		$(".dblog_service_text").html(dblog_service_text);
	}
	else {
		$(".dblog_item_tr").css("display", "none");
		$(".dblog_disabled_status").find("input, textarea, button, select").attr("disabled", false);
		$("input[name=dblog_tousb_cb]").prop("checked", false);
		if(usb_support) {
			var usb_exist = (('<% show_usb_path(); %>').search("storage") != "-1") ? true : false;
			if(usb_exist) {
				$(".noUSBHint").css("display", "none");
				$(".storeUSBHint").css("display", "inline");
				var dblog_tousb = '<% nvram_get("dblog_tousb"); %>';
				$("input[name=dblog_tousb_cb]").prop("checked", ((dblog_tousb == "1") ? true : false));
				usb_status_last_time = true;
			}
			else {
				$(".noUSBHint").css("display", "inline");
				$(".storeUSBHint").css("display", "none");
				usb_status_last_time = false;
			}
		}
		else {
			$(".noUSBHint").css("display", "none");
			$(".storeUSBHint").css("display", "none");
		}
		$("input[name=dblog_service_list_all]").prop("checked", false);
		$("input[name=dblog_service_list]").prop("checked", false);
		diag_tune_service_option();
		diag_create_duration_option();
	}
}
function diag_change_dblog_status() {
	var dblog_enable = getRadioValue($('form[name="form"]').children().find('input[name=dblog_enable]'));
	if(dblog_enable == "1") {
		$(".dblog_item_tr").css("display", "");
		if(usb_support) {
			if(allUsbStatus.search("storage") == "-1")
				alert("<#feedback_capturing_note#>");
			else {
				if($("input[name=dblog_tousb_cb]").prop("checked"))
					alert("<#feedback_capturing_note1#>");
				else
					alert("<#feedback_capturing_note#>");
			}
		}
		else
			alert("<#feedback_capturing_note#>");
	}
	else {
		$(".dblog_item_tr").css("display", "none");
	}
}
function diag_control_usb_status() {
	var dblog_enable = '<% nvram_get("dblog_enable"); %>';
	if(dblog_enable == "0") {
		if(usb_support) {
			var usb_status_current = (allUsbStatus.search("storage") != "-1") ? true : false;
			if(usb_status_current != usb_status_last_time) {
				$(".noUSBHint").css("display", "none");
				$(".storeUSBHint").css("display", "none");
				$("input[name=dblog_tousb_cb]").prop("checked", false);
				if(usb_status_current) {
					$(".noUSBHint").css("display", "none");
					$(".storeUSBHint").css("display", "inline");
					var dblog_tousb = '<% nvram_get("dblog_tousb"); %>';
					$("input[name=dblog_tousb_cb]").prop("checked", ((dblog_tousb == "1") ? true : false));
					usb_status_last_time = true;
				}
				else {
					$(".noUSBHint").css("display", "inline");
					$(".storeUSBHint").css("display", "none");
					usb_status_last_time = false;
				}
				diag_tune_service_option();
				diag_create_duration_option();
			}
		}
	}
}
function diag_change_storeUSB() {
	diag_tune_service_option();
	if($("input[name=dblog_service_list_all]").prop("checked")) {
		$("input[name=dblog_service_list]").prop("checked", true);
	}
	diag_create_duration_option();
}
function diag_create_duration_option() {
	$("select[name=dblog_duration]").empty();
	var hour_to_sec = function(_hours) {
		var sec = 0;
		sec = _hours*60*60;
		return sec;
	};
	var selectOption = "";
	if(usb_support && $("input[name=dblog_tousb_cb]").prop("checked")) {
		selectOption = { "12 <#Hour#>" : hour_to_sec(12), "1 <#Day#>" : hour_to_sec(24), "2 <#Day#>" : hour_to_sec(48), "3 <#Day#>" : hour_to_sec(72) };
	}
	else {
		selectOption = { "6 <#Hour#>" : hour_to_sec(6), "12 <#Hour#>" : hour_to_sec(12), "24 <#Hour#>" : hour_to_sec(24) };
	}

	$.each(selectOption, function(item, value) {
		$("select[name=dblog_duration]")
			.append($("<option></option>")
			.attr("value",value)
			.text(item));
	});
}
function diag_change_service_list_all() {
	if($("input[name=dblog_service_list_all]").prop("checked")) {
		$("input[name=dblog_service_list]").prop("checked", true);
	}
	else {
		$("input[name=dblog_service_list]").prop("checked", false);
	}
}
function diag_change_service_list() {
	var service_list_all_option_count = $("input:checkbox[name=dblog_service_list]").length;
	var service_list_click_option_count = $("input:checkbox[name=dblog_service_list]:checked").length;
	if(service_list_all_option_count == service_list_click_option_count)
		$("input[name=dblog_service_list_all]").prop("checked", true);
	else
		$("input[name=dblog_service_list_all]").prop("checked", false);
}
function diag_tune_service_option() {
	var gen_service_option = function(_value, _text, _class) {
		var $labelHtml = $("<label>");
		$labelHtml.addClass("dblog_service_item");
		$labelHtml.addClass(_class);

		var $inputHtml = $('<input/>');
		$inputHtml.attr({"type" : "checkbox"});
		$inputHtml.attr({"name" : "dblog_service_list"});
		$inputHtml.val(_value);
		$inputHtml.click(function() {
			diag_change_service_list();
		});
		$labelHtml.append($inputHtml);
		$labelHtml.append(_text);

		return $labelHtml;
	};
	if(amesh_support && (isSwMode("rt") || isSwMode("ap"))) {
		if($(".dblog_service_item.AiMesh").length == 0)
			$(".dblog_service_item.all").after(gen_service_option(8, "AiMesh", "AiMesh"));
	}

	if(usb_support) {
		if($(".dblog_service_item.noUSB").length > 0)
			$(".dblog_service_item.noUSB").remove();
		if($("input[name=dblog_tousb_cb]").prop("checked")) {
			if(media_support)
				$(".dblog_service_item.all").after(gen_service_option(4, "<#UPnPMediaServer#>", "noUSB"));
			if(!nodm_support)
				$(".dblog_service_item.all").after(gen_service_option(2, "<#DM_title#>", "noUSB"));
		}
	}

	if($(".dblog_service_item.wifi").length == 0)
		$(".dblog_service_item.all").after(gen_service_option(1, "Wi-Fi", "wifi"));
}
function dblog_stop() {
	showLoading(3);
	document.stop_dblog_form.submit();
}

function check_refresh_token() {
	$("#oauth_google_hint").html("");
	var oauth_google_refresh_token_ori = httpApi.nvramGet(["oauth_google_refresh_token"]).oauth_google_refresh_token;
	$.ajax({
		url: '/ajax_oauth.asp',
		dataType: 'script',
		error: function(xhr) {
			setTimeout("check_refresh_token();", 1000);
		},
		success: function(response){			
			if(oauth_google_refresh_token != "" && oauth_google_refresh_token != oauth_google_refresh_token_ori) {
				//$("#oauth_google_btn").val("Reauthenticate");/*untranslated*/
				$("#oauth_google_hint").html("Authenticated!");/*untranslated*/
				$("#oauth_google_btn").css("display", "");
				$("#oauth_google_loading").css("display", "none");
				
				window.location.reload();
			}
			else {
				if(check_refresh_token_retry > 5) {
					//$("#oauth_google_btn").val("Authenticate");/*untranslated*/
					$("#oauth_google_hint").html("<#qis_fail_desc1#>");
					$("#oauth_google_btn").css("display", "");
					$("#oauth_google_loading").css("display", "none");
				}
				else {
					setTimeout("check_refresh_token();", 2000);
				}
				check_refresh_token_retry++;
			}
		}
	});
}
var check_refresh_token_retry = 0;
function onGoogleLogin(_parm) {
	if(_parm.code != "error") {		
		check_refresh_token_retry = 0;
		$("#oauth_google_btn").css("display", "none");
		$("#oauth_google_loading").css("display", "");
		httpApi.nvramSet({
			"oauth_google_auth_code" : _parm.code,
			"fb_email_provider" : "google",
			"action_mode": "apply",
			"rc_service": "oauth_google_gen_token_email"
			}, check_refresh_token);
	}
}
function change_fb_email_provider(obj){	
	if(document.form.fb_email_provider.value=="google"){
		$("#option_google").show();

		var googleTokenStatus = httpApi.nvramGet(["oauth_google_refresh_token"]);
		if(googleTokenStatus.oauth_google_refresh_token == "") {
			//$("#oauth_google_btn").val("Authenticate");/*untranslated*/
			$("#oauth_google_hint").css("display", "none");
		}
		else {		
			//$("#oauth_google_btn").val("Reauthenticate");/*untranslated*/
			$("#oauth_google_hint").css("display", "");
			$("#oauth_google_hint").html("Authenticated!");/*untranslated*/

			var googleAuthInfo = httpApi.nvramGet(["oauth_google_user_email"]);
			document.form.fb_email.value = googleAuthInfo.oauth_google_user_email;
		}

		document.form.fb_email.readOnly = true;
	}
	else{
		$("#option_google").hide();
		document.form.fb_email.value = "";
		document.form.fb_email.readOnly = false;
	}
}

function startLogPrep(){
	disableCheckChangedStatus();
	dr_advise();
}

var redirect_info = 0;
var showLoading_sec;
function CheckFBSize(){
	$.ajax({
		url: '/ajax_fb_size.asp',
		dataType: 'script',
		timeout: 1500,
		error: function(xhr){
				redirect_info++;
				if(redirect_info < 10){
					setTimeout("CheckFBSize();", 1000);
				}
				else{
					showLoading(35);
					setTimeout("redirect()", 35000);
				}
		},
		success: function(){
				showLoading_sec = Number(fb_total_size)/1024/1024/8;	/* 1MB for 1min */
				showLoading_sec = showLoading_sec.toFixed(1);
				showLoading_sec = showLoading_sec>1? showLoading_sec:1;
				showLoading_sec = showLoading_sec*60;   //min -> sec
				showLoading(showLoading_sec);
				setTimeout("redirect()", showLoading_sec*1000);
		}
	});
}
</script>
</head>
<body onload="initial();" onunLoad="return unload_body();">
<div id="TopBanner"></div>
<div id="hiddenMask" class="popup_bg">
<table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center">
<tr>
<td>
<div class="drword" id="drword" style="height:110px;"><#Main_alert_proceeding_desc4#> <#QKSet_detect_waitdesc1#>...
<br/>
<br/>
</div>
</td>
</tr>
</table>
</div>
<div id="Loading" class="popup_bg"></div>
<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="stop_dblog_form" class="dblog_support_class" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="current_page" value="Advanced_Feedback.asp">
<input type="hidden" name="dblog_enable" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="stop_dblog">
<input type="hidden" name="action_wait" value="3">
</form>
<form method="post" name="form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="current_page" value="Advanced_Feedback.asp">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="restart_sendmail">
<input type="hidden" name="action_wait" value="60">
<input type="hidden" name="PM_attach_syslog" value="">
<input type="hidden" name="PM_attach_cfgfile" value="">
<input type="hidden" name="PM_attach_iptables" value="">	
<input type="hidden" name="PM_attach_modemlog" value="">
<input type="hidden" name="PM_attach_wlanlog" value="">
<input type="hidden" name="feedbackresponse" value="<% nvram_get("feedbackresponse"); %>">
<input type="hidden" name="fb_experience" value="<% nvram_get("fb_experience"); %>">
<input type="hidden" name="fb_browserInfo" value="">
<input type="hidden" name="fb_transid" value="123456789ABCDEF0">
<input type="hidden" name="dblog_service" class="dblog_support_class" value="">
<input type="hidden" name="dblog_tousb" class="dblog_support_class" value="">
<input type="hidden" name="dblog_transid" class="dblog_support_class" value="0123456789ABCDEF">
<table class="content" align="center" cellpadding="0" cellspacing="0">
<tr>
<td width="17">&nbsp;</td>
<td valign="top" width="202">
<div id="mainMenu"></div>
<div id="subMenu"></div>
</td>
<td valign="top">
<div id="tabMenu" class="submenuBlock"></div>
<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
<tr>
<td align="left" valign="top">

<table width="760px" border="0" cellpadding="5" cellspacing="0" class="FormTitle" id="FormTitle">
<tbody>
<tr>
<td bgcolor="#4D595D" valign="top" >
<div>&nbsp;</div>
<div class="formfonttitle"><#menu5_6#> - <#menu_feedback#></div>
<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
<div id="fb_desc0" class="formfontdesc" style="display:none;"><#Feedback_desc0#></div>
<div id="fb_desc1" class="formfontdesc" style="display:none;"><#Feedback_desc1#></div>
<div id="fb_desc_disconnect" class="formfontdesc" style="display:none;color:#FC0;"><#Feedback_desc_disconnect#> <a href="mailto:router_feedback@asus.com?Subject=<%nvram_get("productid");%>" target="_top" style="color:#FFCC00;">router_feedback@asus.com</a></div>
<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
<tr>
<th width="30%"><#feedback_country#> *</th>
<td>
	<input type="text" name="fb_country" maxlength="30" class="input_25_table" value="" autocorrect="off" autocapitalize="off">
</td>
</tr>
<tr>
<th><#feedback_isp#> *</th>
<td>
	<input type="text" name="fb_ISP" maxlength="40" class="input_25_table" value="" autocorrect="off" autocapitalize="off">
</td>
</tr>
<tr>
<th>Subscribed Plan/Service/Package *</th>	<!-- untranslated  -->
<td>
	<input type="text" name="fb_Subscribed_Info" maxlength="50" class="input_25_table" value="" autocorrect="off" autocapitalize="off">
</td>
</tr>
<tr>
	<th><#Provider#></th>
	<td>
		<select class="input_option" name="fb_email_provider" onChange="change_fb_email_provider(this);">
			<option value="">ASUS</option>
			<option value="google">Google</option>
		</select>
		<span id="option_google">
			<input id="oauth_google_btn" class="button_gen oauth_google" type="button" value="">
			<img id="oauth_google_loading" style="margin-left:5px;display:none;" src="/images/InternetScan.gif">
			<span id="oauth_google_hint" class="oauth_google" style="color:#FC0;display:none"></span>
		</span>
	</td>
	</tr>
<tr>
<th><#feedback_email#> *</th>
<td>
	<input type="text" name="fb_email" maxlength="50" class="input_25_table" value="" autocorrect="off" autocapitalize="off">	
</td>
</tr>

<tr>
<th>ASUS Service No./Case#</th>
<td>
	<input type="text" name="fb_serviceno" maxlength="50" class="input_25_table" value="" autocorrect="off" autocapitalize="off">
</td>
</tr>

<tr>
<th><#feedback_extra_info#> *</th>
<td>
	<input type="checkbox" class="input" name="attach_syslog" id="attach_syslog_id"><label for="attach_syslog_id"><#System_Log#></label>&nbsp;&nbsp;&nbsp;
	<input type="checkbox" class="input" name="attach_cfgfile" id="attach_cfgfile_id"><label for="attach_cfgfile_id"><#feedback_setting_file#></label>&nbsp;&nbsp;&nbsp;
	<span id="attach_iptables_span" style="color:#FFFFFF;"><input type="checkbox" class="input" name="attach_iptables" id="attach_iptables_id"><label for="attach_iptables_id"><#feedback_iptable_setting#></label></span>
	<span id="attach_modem_span" style="color:#FFFFFF;"><input type="checkbox" class="input" name="attach_modemlog" id="attach_modemlog_id"><label for="attach_modemlog_id"><#feedback_3G_log#></label></span>
	<input type="checkbox" class="input" name="attach_wlanlog" id="attach_wlanlog_id"><label for="attach_wlanlog_id"><#feedback_WiFi_log#></label>
</td>
</tr>

<tr>
	<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(25,11);"><#feedback_debug_log_DSL#> *</a></th>
	<td>
		<input type="radio" name="dslx_diag_enable" class="input" value="1" onclick="change_dsl_diag_enable(1);"><#checkbox_Yes#>
		<input type="radio" name="dslx_diag_enable" class="input" value="0" onclick="change_dsl_diag_enable(0);" checked><#checkbox_No#>
		<br>	
		<span id="storage_ready" style="display:none;color:#FC0">* <#USB_ready#></span>
		<span id="be_lack_storage" style="display:none;color:#FC0">* <#no_usb_found#></span>
	</td>
</tr>

<tr id="dslx_diag_duration">
	<th><#feedback_capturing_duration#> *</th>
	<td>
		<select id="" class="input_option" name="dslx_diag_duration">
			<option value="0" selected><#Auto#></option>
			<option value="3600">1 <#Hour#></option>
			<option value="18000">5 <#Hour#></option>
			<option value="43200">12 <#Hour#></option>
			<option value="86400">24 <#Hour#></option>
		</select>
	</td>
</tr>

<tr class="dblog_support_class">
	<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(34, 1);"><#feedback_debug_log#></a></th>
	<td>
		<div class="dblog_disabled_status">
			<input type='radio' name='dblog_enable' id='dblog_status_en' value="1" onclick="diag_change_dblog_status();"><label for='dblog_status_en'><#checkbox_Yes#></label>
			<input type='radio' name='dblog_enable' id='dblog_status_dis' value="0" onclick="diag_change_dblog_status();" checked><label for='dblog_status_dis'><#checkbox_No#></label>
			<label class="storeUSBHint"><input type="checkbox" name="dblog_tousb_cb" value="1" onclick="diag_change_storeUSB();" checked><#feedback_debug_log_inDisk#></label>
			<span class="noUSBHint">* <#no_usb_found#></span>
		</div>
		<div class="dblog_enabled_status">
			<span>* <#feedback_current_capturing#></span>
			<br>
			<span class="dblog_stop_text" onclick="dblog_stop();"><#feedback_cancel_capturing#></span>
		</div>
	</td>
</tr>
<tr class="dblog_item_tr dblog_support_class">
	<th><#feedback_current_log#></th>
	<td class="dblog_item_td">
		<div class="dblog_disabled_status">
			<label class="dblog_service_item all"><input type="checkbox" name="dblog_service_list_all" onclick="diag_change_service_list_all();"><#All#></label>
		</div>
		<div class="dblog_enabled_status">
			<span class="dblog_service_text"></span>
		</div>
	</td>
</tr>
<tr class="dblog_item_tr dblog_support_class">
	<th><#feedback_capturing_duration#></th>
	<td>
		<div class="dblog_disabled_status">
			<select class="input_option" name="dblog_duration"></select>
		</div>
		<div class="dblog_enabled_status">
			<span class="dblog_remaining_text"></span>
		</div>
	</td>
</tr>

<tr>
<th><#feedback_connection_type#></th>
<td>
	<select class="input_option" name="fb_availability">
		<option value="Not_selected"><#Select_menu_default#> ...</option>
		<option value="Stable_connection"><#feedback_stable#></option>
		<option value="Occasional_interruptions"><#feedback_Occasion_interrupt#></option>
		<option value="Frequent_interruptions"><#feedback_Frequent_interrupt#></option>
		<option value="Unavailable"><#feedback_usaul_unavailable#></option>
	</select>
</td>
</tr>

<tr>
<th><#feedback_problem_type#></th>
<td>
	<select class="input_option" name="fb_ptype" onChange="Reload_pdesc(this);">
		
	</select>
</td>
</tr>

<tr>
<th><#feedback_problem_desc#></th>
<td>
	<select class="input_option" name="fb_pdesc">
		
	</select>
</td>
</tr>

<tr>
	<th>
		<#feedback_comments#> *
	</th>
	<td>
		<textarea name="fb_comment" maxlength="2000" cols="55" rows="8" class="textarea_ssh_table" style="font-family:'Courier New', Courier, mono; font-size:13px;" onKeyDown="textCounter(this,document.form.msglength,2000);" onKeyUp="textCounter(this,document.form.msglength,2000)"></textarea>
		<span style="color:#FC0"><#feedback_max_counts#> : </span>
		<input type="text" class="input_6_table" name="msglength" id="msglength" maxlength="4" value="2000" autocorrect="off" autocapitalize="off" readonly>
	</td>
</tr>

<tr>
	<td colspan="2">
		<div>
			<div style="float: left;"><input type="checkbox" name="eula_checkbox"/></div>
			<div id="eula_content" style="margin-left: 20px;"><#feedback_eula#></div>
		</div>
		<input class="button_gen" style="margin-left: 305px; margin-top:5px;" name="btn_send" onclick="applyRule()" type="button" value="<#btn_send#>"/>
	</td>
</tr>

<tr>
	<td colspan="2">
		<strong><#FW_note#></strong>
		<ul>
			<li><#feedback_note4#></li>
		</ul>
	</td>
</tr>	
</table>
</td>
</tr>
</tbody>
</table>
</td>
</form>
</tr>
</table>
</td>
<td width="10" align="center" valign="top">&nbsp;</td>
</tr>
</table>
<div id="footer"></div>
</body>

<!--Advanced_Feedback.asp-->
</html>


<td width="10" align="center" valign="top">&nbsp;</td>
</tr>
</table>
<div id="footer"></div>
</body>

<!--Advanced_Feedback.asp-->
</html>

