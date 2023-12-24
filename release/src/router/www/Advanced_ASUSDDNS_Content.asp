<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title><#Web_Title#> - <#menu5_3_6#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<script type="text/javascript" src="/js/jquery.js"></script>
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" language="JavaScript" src="/help.js"></script>
<script type="text/javascript" language="JavaScript" src="/validator.js"></script>
<script type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script type="text/javascript" src="/form.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/asus_eula.js"></script>
<style type="text/css">
.contentM_upload{
	position:absolute;
	-webkit-border-radius: 5px;
	-moz-border-radius: 5px;
	border-radius: 5px;
	z-index:500;
	background-color:#2B373B;
	display:none;
	margin-left: 30%;
	top: 570px;
	width:650px;
}

.Upload_item{
	font-family: Arial, Helvetica, sans-serif;
    font-size: 13px;
    font-weight: bolder;
    color: #FFFFFF;
    margin-left: 15px;
    margin-bottom: 15px;
    margin-top: 15px;
}

.Upload_file{
	background-color:#2B373B;
	color:#FC0;
	*color:#000;
	border:0px;
}

.cert_status_title{
	width: 20%;
}

.cert_status_val{
	width: 76%;
	padding-left: 10px;
}

.popup_container{
    width: 70%;
}

.feature_desc_container .title{
	font-weight: 600;
	font-size: 16px;
	letter-spacing: 0.2px;
	text-transform: uppercase;
	color: rgb(97 173 255);
}
.feature_desc_container .desc{
	font-weight: 500;
	font-size: 15px;
	color: rgb(255, 255, 255);
	line-height: 24px;
	margin-top: 12px;
}
.blur_effect{
	-webkit-filter: blur(4px); /* Chrome, Opera */
	-moz-filter: blur(4px);
	-ms-filter: blur(4px);
	filter: blur(4px);
}
.popup_container{
	font-family: Arial, Helvetica, sans-serif, "Microsoft JhengHei";
	display: none;
	height: auto;
	top: 4%;
	width: 70%;
	max-width: 540px;
	position: absolute;
	margin: auto;
	z-index: 300;
	left: 0;
	right: 0;
	align-items: center;
	line-height: 180%;
	-webkit-tap-highlight-color: transparent;
	-webkit-touch-callout: none;
	-webkit-user-select: none;
	-khtml-user-select: none;
	-moz-user-select: none;
	-ms-user-select: none;
	user-select: none;
	font-size: 14px;
}
.popup_container.fit_width{
	max-width: 90%;
	width: 90%;
}
.popup_container.full_width{
	max-width: 100%;
	width: 100%;
}
.popup_container.popup_element_second{
	z-index: 500;
}
.popup_container.popup_customize_alert{
	z-index: 700;
	top: 12%;
}
.popup_container .setting_content_container{
	margin-bottom: 24px;
}
.popup_container .popup_title_container > .close_btn,
.popup_container .popup_title_container > .del_btn{
	display: flex;
	align-items: center;
	justify-content: center;
	color: #FFFFFF;
	width: 24px;
	height: 24px;
	cursor: pointer;
	background: grey;
	border-radius: 50%;
}
.popup_container .popup_title_container{
	display: flex;
	justify-content: space-between;
    align-items: center;
	position: relative;
	margin-bottom: 12px;
}
.popup_container .popup_title_container .title{
	min-height: 40px;
	height: auto;
	color: hsl(0deg 0% 100%);
	font-size: 24px;
	font-weight: 600;
	letter-spacing: -0.2px;
	line-height: 40px;
	padding-left: 12px;
}
.popup_container .popup_content_container{
	position: relative;
	background: linear-gradient(0deg, rgba(255, 255, 255, 0.16), rgba(255, 255, 255, 0.16)), linear-gradient(0deg, rgba(0, 0, 0, 0.8), rgba(0, 0, 0, 0.8)), #1067A8;
	border: 1px solid rgba(255, 255, 255, 0.02);
	box-shadow: 0px 2px 4px 0px rgb(0 0 0 / 20%), 0px 1px 4px 0px rgb(60 60 60 / 30%);
	border-radius: 8px;
	padding: 5%;
}
.popup_container.mobile_view .popup_content_container{
	box-shadow: initial;
	background: rgb(5 41 88 / 30%);
}
.popup_container > div{
	width: 100%;
}
.popup_container .popup_content_container.profile_setting{
	padding: initial;
}
.popup_container .popup_content_container .profile_setting_item,
.popup_container .popup_content_container .profile_setting_two_item{
	background: rgba(0, 0, 0, 0.1);
	min-width: initial;
	height: 88px;
}
.popup_container .popup_content_container .profile_setting_two_item{
	height: auto;
}
.popup_container .popup_content_container .category_slide_title{
	background: rgba(0, 0, 0, 0.1);
}
.popup_container .popup_content_container .category_slide_title.expand{
	background: rgba(0, 133, 255, 0.1);
}
.customize_alert .action_btn_container,
.del_profile .action_btn_container{
	display: flex;
	justify-content: flex-end;
	margin-top: 30px;
	position: relative;
}
.customize_alert .action_btn_container .btn,
.del_profile .action_btn_container .btn{
	width: 117px;
	height: 38px;
	line-height: 40px;
	font-size: 14px;
	font-weight: 500;
	text-align: center;
	letter-spacing: 0.1px;
	border-radius: 8px;
	margin-left: 16px;
	cursor: pointer;
}

.icon_switch{
	border-radius: 50px;
	width: 40px;
	height: 20px;
	position: relative;
	-webkit-transition: all 0.6s;
	transition: all 0.6s;
	cursor: pointer;
}
.icon_switch.off{
	background: rgba(127, 143, 164, 1);
}
.icon_switch.on{
	background: rgba(16, 185, 129, 1);
}
.icon_switch:before{
	border-radius: 50%;
	display: block;
	position: absolute;
	content: "";
	height: 12px;
	width: 12px;
	left: 4px;
	top: 4px;
	-webkit-transition: 0.6s;
	transition: all 0.6s;
	-webkit-transition-delay: 0.01s;
	transition-delay: 0.01s;
	background: rgb(48 61 67);
}
.icon_switch.on:before{
	-webkit-transform: translateX(20px);
	-ms-transform: translateX(20px);
	transform: translateX(20px);
}

.tooltip {
  position: relative;
  display: inline-block;
}

.tooltip .tooltiptext {
  display: none;
  width: 60px;
  background-color: black;
  color: #fff;
  text-align: center;
  border-radius: 6px;
  padding: 5px;
  position: absolute;
  z-index: 1;
  bottom: 150%;
  left: 50%;
  margin-left: -40px;
}

.tooltip .tooltiptext::after {
  content: "";
  position: absolute;
  top: 100%;
  left: 50%;
  margin-left: -5px;
  border-width: 5px;
  border-style: solid;
  border-color: black transparent transparent transparent;
}

.tooltip .tooltiptextdown {
  display: none;
  width: 20vw;
  word-break: break-all;
  background-color: rgba(22,27,33,0.75);
  color: #fff;
  border-radius: 6px;
  padding: 5px;
  position: absolute;
  z-index: 1;
  top: 150%;
  left: -19vw;
}

.tooltip .tooltiptextdown::after {
  content: "";
  position: absolute;
  bottom: 100%;
  left: 95%;
  margin-left: -5px;
  border-width: 5px;
  border-style: solid;
  border-color:  transparent transparent #161b21ee transparent;
}

.tooltip:hover .tooltiptextdown {
  display: block;
}

.icon_switch{
  background: #808080;
}

.icon_switch.on{
  background:rgb(30, 162, 255);
}

.icon_switch:before {
  background: #f7f7f7;
}

.icon_help,
.icon-clone{
    background: #47A2FF!important;
}
.icon_help:hover,
.icon-clone:hover{
    background: #0b5ed7!important;
}


</style>

<script>
<% wanlink(); %>
var ddns_hostname_x_t = '<% nvram_get("ddns_hostname_x"); %>';
var ddns_server_x_t = '<% nvram_get("ddns_server_x"); %>';
var ddns_updated_t = '<% nvram_get("ddns_updated"); %>';
var wans_mode ='<% nvram_get("wans_mode"); %>';
var no_phddns = isSupport("no_phddns");
var ddns_replace_status = '<% nvram_get("ddns_replace_status"); %>';

var ddns_return_code = '<% nvram_get_ddns("LANHostConfig","ddns_return_code"); %>';
var ddns_return_code_chk = '<% nvram_get("ddns_return_code_chk"); %>';
var ddns_old_name = '<% nvram_get("ddns_hostname_old"); %>';
var ddns_server_x = '<% nvram_get("ddns_server_x"); %>';
var ddns_enable_x = '<% nvram_get("ddns_enable_x"); %>';
var http_enable = '<% nvram_get("http_enable"); %>';

var httpd_cert_info = [<% httpd_cert_info(); %>][0];
var orig_le_enable = '<% nvram_get("le_enable"); %>';
var le_state = '<% nvram_get("le_state"); %>';
var httpd_restart = 0;
var ddnsStatus = getDDNSState(ddns_return_code_chk, ddns_hostname_x_t, ddns_old_name);
var deregister_fail = 0;
var cur_wan_ipaddr = wanlink_ipaddr();
var inadyn = isSupport("inadyn");

var le_sbstate_t = '<% nvram_get("le_sbstate_t"); %>';
var le_auxstate_t = '<% nvram_get("le_auxstate_t"); %>';
var le_re_ddns = '<% nvram_get("le_re_ddns"); %>';
var faq_href = "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang="+ui_lang+"&kw=&num=105";
var oauth_auth_status = httpApi.nvramGet(["oauth_auth_status"], true).oauth_auth_status;
var aae_ddnsinfo = httpApi.nvramGet(["aae_ddnsinfo"], true).aae_ddnsinfo;
var ipv6_service = httpApi.nvramGet(["ipv6_service"], true).ipv6_service;
var asusddns_token_state = httpApi.nvramGet(["asusddns_token_state"], true).asusddns_token_state;
var ddns_accournt_remove_note = stringSafeGet("<#asusddns_rm_account_hint#>");

function init(){
	show_menu();
	document.getElementById("faq").href=faq_href;
	ddns_load_body();
	update_ddns_wan_unit_option();

	if(no_phddns){
		for(var i = 0; i < document.form.ddns_server_x.length; i++){
			if(document.form.ddns_server_x.options[i].value == "WWW.ORAY.COM"){
				document.form.ddns_server_x.remove(i);
				break;
			}
		}
	}

	setTimeout(show_warning_message, 1000);

	ASUS_EULA.config(applyRule, refreshpage);
	if(ddns_enable_x == "1" && ddns_server_x.indexOf("WWW.ASUS.COM") != -1){
		ASUS_EULA.check('asus');
	}

	if(oauth_auth_status == "2"){
		if(aae_ddnsinfo == "ns1.asuscomm.com" && ddns_hostname_x_t.indexOf(".asuscomm.com") != "-1" && ddns_replace_status == "1")
			$("#ddns_server_x option[value='WWW.ASUS.COM.CN']").remove();
		else if(aae_ddnsinfo == "ns1.asuscomm.cn" && ddns_hostname_x_t.indexOf(".asuscomm.cn") != "-1" && ddns_replace_status == "1")
			$("#ddns_server_x option[value='WWW.ASUS.COM']").remove();
	}

    $('#ddns_security_info_1').attr({'style':'text-decoration: underline','href':'https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang='+ui_lang+'&kw=&num=171'});
    $('#ddns_security_info_2').attr({'style':'text-decoration: underline','href':'https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang='+ui_lang+'&kw=&num=107'});
}

function update_ddns_wan_unit_option(){
	var wans_dualwan_array = '<% nvram_get("wans_dualwan"); %>'.split(" ");
	var wans_mode = '<%nvram_get("wans_mode");%>';
	if(document.form.ddns_enable_x.value == "0" || !dualWAN_support || wans_mode != "lb" || wans_dualwan_array.indexOf("none") != -1){
		document.getElementById("ddns_wan_unit_th").style.display = "none";
		document.getElementById("ddns_wan_unit_td").style.display = "none";
	}else{
		document.getElementById("ddns_wan_unit_th").style.display = "";
		document.getElementById("ddns_wan_unit_td").style.display = "";
	}
}

var MAX_RETRY_NUM = 5;
var external_ip_retry_cnt = MAX_RETRY_NUM;

function show_warning_message(){
	if(realip_support && (based_modelid == "BRT-AC828" || wans_mode != "lb")){
		if(realip_state != "2" && external_ip_retry_cnt > 0){
			if( external_ip_retry_cnt == MAX_RETRY_NUM )
				get_real_ip();
			else
				setTimeout("get_real_ip();", 3000);
		}
		else if(realip_state != "2"){
			if(cur_wan_ipaddr == "0.0.0.0" || validator.isPrivateIP(cur_wan_ipaddr))
				showhide("wan_ip_hide2", 1);
			else
				showhide("wan_ip_hide2", 0);
		}
		else{
			if(!external_ip)
				showhide("wan_ip_hide2", 1);
			else
				showhide("wan_ip_hide2", 0);
		}
	}
	else if(cur_wan_ipaddr == "0.0.0.0" || validator.isPrivateIP(cur_wan_ipaddr))
		showhide("wan_ip_hide2", 1);
}

function get_real_ip(){
	$.ajax({
		url: 'get_real_ip.asp',
		dataType: 'script',
		error: function(xhr){
			get_real_ip();
		},
		success: function(response){
			external_ip_retry_cnt--;
			show_warning_message();
		}
	});
}

function submitForm(){
	if(letsencrypt_support){
		if(document.form.ddns_enable_x.value == "1" && $("input[name='le_enable']:checked").val() == "1"){
			document.form.action_wait.value = "10";
			document.form.action_script.value = "restart_ddns_le;prepare_cert";
		}
		else if(http_enable != "0" && ($("input[name='le_enable']:checked").val() != orig_le_enable || httpd_restart == 1 )){
			document.form.action_wait.value = "10";
			if(orig_le_enable == "1")
				document.form.action_script.value = "prepare_cert;restart_webdav;restart_ddns_le";
			else
				document.form.action_script.value += ";prepare_cert;restart_webdav";

		}
		if (('<% nvram_get("enable_ftp"); %>' == "1") && ('<% nvram_get("ftp_tls"); %>' == "1")) {
			document.form.action_script.value += ";prepare_cert;restart_ftpd";
		}
	}

	document.form.submit();
	showLoading();
	setTimeout('location.reload();', 5000);
}

function check_update(){
    var ddns_ipaddr_t = '<% nvram_get("ddns_ipaddr"); %>';
		ddns_ipaddr_t = ddns_ipaddr_t.replace(/&#10/g,"");

    if (document.form.ddns_enable_x.value == "1" &&
        (wanlink_ipaddr() == ddns_ipaddr_t) &&
        (ddns_server_x_t == document.form.ddns_server_x.value) &&
        (ddns_hostname_x_t == document.form.ddns_hostname_x.value) &&
			ddns_updated_t == '1'){
			force_update();
    }else{
		submitForm();
    }
}

function force_update() {
    var r = confirm("<#LANHostConfig_x_DDNS_update_confirm#>");
	if(r == false)
		return false

	submitForm();
}

function show_ipv6update_setting(){
	if(ipv6_service != "disabled")
		showhide("ddns_ipv6update_tr", 1);
	else
		showhide("ddns_ipv6update_tr", 0);
}

function show_deregister_btn(){
	$("#deregister_btn").css("display", "inline");
	if(asusddns_token_state == "1"){
		$("#deregister_btn").click(function(){
			alert(ddns_accournt_remove_note);
		});
	}
	else{
		$("#deregister_btn").click(function(){
			if(orig_le_enable != "0"){
				var confirm_msg = stringSafeGet("<#LANHostConfig_x_DDNS_alarm_deregister#>");
				if(!confirm(confirm_msg)){
					return false;
				}
			}

			showLoading();
			asuscomm_deregister();
		});
	}
}

function ddns_load_body(){
    if(ddns_enable_x == 1){
        inputCtrl(document.form.ddns_server_x, 1);
        document.getElementById('ddns_hostname_tr').style.display = "";
        if(ddns_server_x.indexOf("WWW.ASUS.COM") != -1 || ddns_server_x == ""){
            document.form.ddns_hostname_x.parentNode.style.display = "none";
            document.form.DDNSName.parentNode.style.display = "";
			if(ddns_server_x.indexOf(".CN") != -1)
				$("#domain_text").text(".asuscomm.cn");
			else
				$("#domain_text").text(".asuscomm.com");
            var ddns_hostname_title = ddns_hostname_x_t.substring(0, ddns_hostname_x_t.indexOf($("#domain_text").text()));
            if(ddns_hostname_x_t != '' && ddns_hostname_title)
                document.getElementById("DDNSName").value = ddns_hostname_title;
            else
                document.getElementById("DDNSName").value = "<#asusddns_inputhint#>";
        }
        else{
            document.form.ddns_hostname_x.parentNode.style.display = "";
            document.form.DDNSName.parentNode.style.display = "none";
            inputCtrl(document.form.ddns_username_x, 1);
            inputCtrl(document.form.ddns_passwd_x, 1);
            if(ddns_hostname_x_t != '')
                document.getElementById("ddns_hostname_x").value = ddns_hostname_x_t;
            else
                document.getElementById("ddns_hostname_x").value = "<#asusddns_inputhint#>";
        }
	inputCtrl(document.form.ddns_refresh_x, 1);
		show_ipv6update_setting();
        change_ddns_setting(document.form.ddns_server_x.value);

	    if(document.form.ddns_server_x.value == "WWW.ORAY.COM"){
		    if(ddns_updated_t == "1"){
				document.getElementById("ddns_hostname_info_tr").style.display = "";
				document.getElementById("ddns_hostname_x_value").innerHTML = ddns_hostname_x_t;
			}
		}
    }
    else{
        inputCtrl(document.form.ddns_server_x, 0);
        document.getElementById('ddns_hostname_tr').style.display = "none";
        document.getElementById("ddns_hostname_info_tr").style.display = "none";
        inputCtrl(document.form.ddns_username_x, 0);
        inputCtrl(document.form.ddns_passwd_x, 0);
        document.form.ddns_wildcard_x[0].disabled= 1;
        document.form.ddns_wildcard_x[1].disabled= 1;
	inputCtrl(document.form.ddns_refresh_x, 0);
        showhide("wildcard_field",0);
    }

    if (HTTPS_support) {
        show_cert_settings(1);
		change_cert_method(orig_le_enable);
		show_cert_details();
	} else {
        show_cert_settings(0);
	}

    hideLoading();

	if(ddns_enable_x == "1" && !deregister_fail)
	{
		var ddnsHint = getDDNSState(ddns_return_code, ddns_hostname_x_t, ddns_old_name);

		if(ddnsHint != "" && le_re_ddns != "1"){
			document.getElementById("ddns_result").innerHTML = ddnsHint;
			document.getElementById('ddns_result_tr').style.display = "";
		}

		if((ddns_return_code.indexOf('200')!=-1 || ddns_return_code.indexOf('220')!=-1 || ddns_return_code == 'register,230') ||
		   (ddns_return_code_chk.indexOf('200')!=-1 || ddns_return_code_chk.indexOf('220')!=-1 || ddns_return_code_chk == 'register,230')){
			showhide("wan_ip_hide2", 0);
		}
		else{
			if((ddns_return_code == "ddns_query" || ddns_return_code_chk == "Time-out" || ddns_return_code_chk == "connect_fail" || ddns_return_code_chk.indexOf('-1') != -1) && le_re_ddns != "1")
				checkDDNSReturnCode_noRefresh();
		}
	}
}

function get_cert_info(){
	$.ajax({
		url: '/ajax_certinfo.asp',
		dataType: 'script',
		error: function(xhr){
			setTimeout("get_cert_info();", 1000);
		},
		success: function(response){
			show_cert_details();
	   }
	});
}

function apply_eula_check(){
	if(document.form.ddns_enable_x.value == "1" && document.form.ddns_server_x.value.indexOf("WWW.ASUS.COM") != -1){
		if(!ASUS_EULA.check("asus")) return false;
	}

	applyRule();
}

function applyRule(){
	if(validForm()){
		if(document.form.ddns_enable_x.value == "1" && (document.form.ddns_server_x.value.indexOf("WWW.ASUS.COM") != -1)){
			document.form.ddns_hostname_x.value = document.form.DDNSName.value+$("#domain_text").text();
		}

		if (document.form.le_enable.value != orig_le_enable && document.form.le_enable.value == "0") {
			alert(`<#DDNS_Install_Root_Cert_Desc2#>`);
		}

		if(document.form.ddns_hostname_x.value != ddns_hostname_x_t){
			$('<input>').attr({
				type: 'hidden',
				name: "ddns_replace_status",
				value: "0"
			}).appendTo('#ruleForm');
		}


		check_update();
	}
}

function validForm(){
	if(document.form.ddns_enable_x.value == "1"){		//ddns enable
		if(document.form.ddns_server_x.value.indexOf("WWW.ASUS.COM") != -1){		//WWW.ASUS.COM	or WWW.ASUS.COM.CN
			if(document.form.DDNSName.value == ""){
				alert("<#LANHostConfig_x_DDNS_alarm_14#>");
				document.form.DDNSName.focus();
				document.form.DDNSName.select();
				return false;
			}else{
				if(!validate_ddns_hostname(document.form.DDNSName)){
					document.form.DDNSName.focus();
					document.form.DDNSName.select();
					return false;
				}

				if(letsencrypt_support){
					if( document.form.le_enable[0].checked == true && document.form.letsEncryptTerm_check.checked != true){
						if(!confirm("<#LANHostConfig_x_DDNSLetsEncrypt_Disagree#>")){
							document.form.letsEncryptTerm_check.focus();
							return false;
						}
						else{
							document.form.le_enable[2].checked = true;
						}
					}
				}

				return true;
			}
		}else{
			if(!validator.numberRange(document.form.ddns_refresh_x, 0, 365))
				return false;

			if(document.form.ddns_server_x.value != "WWW.ORAY.COM" && document.form.ddns_hostname_x.value == ""){
				alert("<#LANHostConfig_x_DDNS_alarm_14#>");
				document.form.ddns_hostname_x.focus();
				document.form.ddns_hostname_x.select();
				return false;
			}else if(!validator.string(document.form.ddns_hostname_x)){
				return false;
			}

			if(document.form.ddns_server_x.value != "CUSTOM"){             // Not CUSTOM
				if(document.form.ddns_server_x.value != "DNS.HE.NET" && document.form.ddns_username_x.value == ""){
					alert("<#QKSet_account_nameblank#>");
					document.form.ddns_username_x.focus();
					document.form.ddns_username_x.select();
					return false;
				}else if(!validator.string(document.form.ddns_username_x)){
					return false;
				}

				if(document.form.ddns_passwd_x.value == ""){
					alert("<#File_Pop_content_alert_desc6#>");
					document.form.ddns_passwd_x.focus();
					document.form.ddns_passwd_x.select();
					return false;
				}else if(!validator.string(document.form.ddns_passwd_x)){
					return false;
				}
			}

			if(document.form.ddns_regular_period.value < 30){
				alert("<#period_time_validation#> : 30");
				document.form.ddns_regular_period.focus();
				document.form.ddns_regular_period.select();
				return false;
			}

			return true;
		}
	}
	else
		return true;
}

function checkDDNSReturnCode(){
    $.ajax({
    	url: '/ajax_ddnscode.asp',
    	dataType: 'script',
    	error: function(xhr){
      		checkDDNSReturnCode();
    	},
    	success: function(response){
            if(ddns_return_code == 'ddns_query')
        	    setTimeout("checkDDNSReturnCode();", 500);
            else
                refreshpage();
       }
   });
}

function checkDDNSReturnCode_noRefresh(){
	$.ajax({
		url: '/ajax_ddnscode.asp',
		dataType: 'script',
		error: function(xhr){
			checkDDNSReturnCode_noRefresh();
		},
		success: function(response){
			var ddnsHint = getDDNSState(ddns_return_code, ddns_hostname_x_t, ddns_old_name);

			if(ddns_return_code == 'ddns_query')
				setTimeout("checkDDNSReturnCode_noRefresh();", 500);
			else if(ddns_return_code_chk == 'Time-out' || ddns_return_code_chk == 'connect_fail' || ddns_return_code_chk.indexOf('-1') != -1)
				setTimeout("checkDDNSReturnCode_noRefresh();", 3000);

			if(ddnsHint != ""){
				document.getElementById("ddns_result").innerHTML = ddnsHint;
				document.getElementById('ddns_result_tr').style.display = "";
			}

			if((ddns_return_code.indexOf('200')!=-1 || ddns_return_code.indexOf('220')!=-1 || ddns_return_code == 'register,230') ||
			   (ddns_return_code_chk.indexOf('200')!=-1 || ddns_return_code_chk.indexOf('220')!=-1 || ddns_return_code_chk == 'register,230')){
				showhide("wan_ip_hide2", 0);
				if(ddns_server_x == "WWW.ASUS.COM"){
					showhide("wan_ip_hide3", 1);
					document.getElementById("ddns_status").innerHTML = "<#Status_Active#>";
					if(inadyn)
						show_deregister_btn();
				}
			}
			else{
				if(ddns_server_x == "WWW.ASUS.COM"){
					document.getElementById("ddns_status").innerHTML = "<#Status_Inactive#>";
					if(ddnsStatus != "")
						$("#ddns_status_detail").css("display", "inline");
				}
			}
		}
	});
}

function validate_ddns_hostname(o){
	dot=0;
	s=o.value;

	if(s == ""){
		show_alert_block("<#QKSet_account_nameblank#>");
		return false;
	}

	var unvalid_start=new RegExp("^[0-9].*", "gi");
	if(unvalid_start.test(s) ){
		show_alert_block("<#LANHostConfig_x_DDNS_alarm_7#>");
		return false;
	}

	if (!validator.string(o)){
		return false;
	}

	for(i=0;i<s.length;i++){
		c = s.charCodeAt(i);
		if (c==46){
			dot++;
			if(dot>0){
				show_alert_block("<#LANHostConfig_x_DDNS_alarm_7#>");
				return false;
			}
		}

		if (!validator.hostNameChar(c)){
			show_alert_block("<#LANHostConfig_x_DDNS_alarm_13#> '" + s.charAt(i) +"' !");
			return false;
		}
	}

	return true;
}

function show_alert_block(alert_str){
	document.getElementById("alert_block").style.display = "block";
	showtext(document.getElementById("alert_str"), alert_str);
}

function cleandef(){
	if(document.form.DDNSName.value == "<#asusddns_inputhint#>")
		document.form.DDNSName.value = "";
}

function onSubmitApply(s){
	if(s == "hostname_check"){
		showLoading();
		if(!validate_ddns_hostname(document.form.ddns_hostname_x)){
			hideLoading();
			return false;
		}
	}

	document.form.action_mode.value = "Update";
	document.form.action_script.value = s;
	return true;
}

function change_ddns_setting(v){
	document.getElementById("ddns_result_tr").style.display = "none";
	document.getElementById("ddns_status_tr").style.display = "none";
	if(inadyn)
		$("#deregister_btn").css("display", "none");
	if (v.indexOf("WWW.ASUS.COM") != -1){
			document.getElementById("ddns_hostname_info_tr").style.display = "none";
			document.getElementById("ddns_hostname_tr").style.display="";
			document.form.ddns_hostname_x.parentNode.style.display = "none";
			document.form.DDNSName.parentNode.style.display = "";
			if(v.indexOf(".CN") != -1)
				$("#domain_text").text(".asuscomm.cn");
			else
				$("#domain_text").text(".asuscomm.com");
			var ddns_hostname_title = ddns_hostname_x_t.substring(0, ddns_hostname_x_t.indexOf($("#domain_text").text()));
			if(ddns_hostname_x_t != '' && ddns_hostname_title)
					document.getElementById("DDNSName").value = ddns_hostname_title;
			else
					document.getElementById("DDNSName").value = "<#asusddns_inputhint#>";

			inputCtrl(document.form.ddns_username_x, 0);
			inputCtrl(document.form.ddns_passwd_x, 0);
			document.form.ddns_wildcard_x[0].disabled= 1;
			document.form.ddns_wildcard_x[1].disabled= 1;
			showhide("link", 0);
			showhide("linkToHome", 0);
			showhide("wildcard_field",0);
			document.form.ddns_regular_check.value = 0;
			showhide("check_ddns_field", 0);
			inputCtrl(document.form.ddns_regular_period, 0);
			showhide("customnote", 0);
			showhide("need_custom_scripts", 0);
			document.getElementById("ddns_status_tr").style.display = "";

			if(ddns_enable_x == "1" && ddns_server_x_t.indexOf("WWW.ASUS.COM") != -1 &&
				(ddns_return_code_chk.indexOf('200')!=-1 || ddns_return_code_chk.indexOf('220')!=-1 || ddns_return_code_chk == 'register,230')){
				document.getElementById("ddns_status").innerHTML = "<#Status_Active#>";
				if(inadyn)
					show_deregister_btn();
			}
			else
				document.getElementById("ddns_status").innerHTML = "<#Status_Inactive#>";
	}
	else if (v == "CUSTOM"){
			document.form.ddns_hostname_x.parentNode.style.display = "";
			document.form.DDNSName.parentNode.style.display = "none";
			inputCtrl(document.form.ddns_username_x, 0);
			inputCtrl(document.form.ddns_passwd_x, 0);
			document.form.ddns_wildcard_x[0].disabled= 1;
			document.form.ddns_wildcard_x[1].disabled= 1;
			showhide("customnote", 1);
			showhide("link", 0);
			showhide("linkToHome", 0);
			showhide("wildcard_field",0);
			showhide("check_ddns_field", 0);
			if (('<% nvram_get("jffs2_on"); %>' != '1') || ('<% nvram_get("jffs2_scripts"); %>' != '1'))
				showhide("need_custom_scripts", 1);
			else
			showhide("need_custom_scripts", 0);
	}
	else if( v == "WWW.ORAY.COM"){
		document.getElementById("ddns_hostname_tr").style.display="none";
		inputCtrl(document.form.ddns_username_x, 1);
		inputCtrl(document.form.ddns_passwd_x, 1);
		document.form.ddns_wildcard_x[0].disabled= 1;
		document.form.ddns_wildcard_x[1].disabled= 1;
		showhide("link", 1);
		showhide("linkToHome", 0);
		showhide("wildcard_field",0);
		document.form.ddns_regular_check.value = 0;
		showhide("check_ddns_field", 0);
		inputCtrl(document.form.ddns_regular_period, 0);
	}
	else{
			document.getElementById("ddns_hostname_info_tr").style.display = "none";
			document.getElementById("ddns_hostname_tr").style.display="";
			document.form.ddns_hostname_x.parentNode.style.display = "";
			document.form.DDNSName.parentNode.style.display = "none";
			if(v == "DNS.HE.NET")
				inputCtrl(document.form.ddns_username_x, 0);
			else
				inputCtrl(document.form.ddns_username_x, 1);
			inputCtrl(document.form.ddns_passwd_x, 1);
			var disable_wild = 0;
			if(v == "WWW.TUNNELBROKER.NET" || v == "DNS.HE.NET" || v == "WWW.SELFHOST.DE" || v == "DOMAINS.GOOGLE.COM")
				var disable_wild = 1;
			else
				var disable_wild = 0;
			document.form.ddns_wildcard_x[0].disabled= disable_wild;
			document.form.ddns_wildcard_x[1].disabled= disable_wild;
			if(v == "WWW.ZONEEDIT.COM" || v == "DOMAINS.GOOGLE.COM" || v == "WWW.NAMECHEAP.COM"){
				showhide("link", 0);
				showhide("linkToHome", 1);
			}
			else{
				showhide("link", 1);
				showhide("linkToHome", 0);
			}

			showhide("wildcard_field",!disable_wild);
			showhide("check_ddns_field", 1);
			showhide("customnote", 0);
			showhide("need_custom_scripts", 0);
			if(document.form.ddns_regular_check.value == 0)
				inputCtrl(document.form.ddns_regular_period, 0);
			else
				inputCtrl(document.form.ddns_regular_period, 1);
	}

	var default_hostname_label = "<a class=\"hintstyle\" href=\"javascript:void(0);\" onClick=\"openHint(5,13);\"><#LANHostConfig_x_DDNSHostNames_itemname#></a>";
	if(v == "WWW.NAMECHEAP.COM") {
		document.getElementById("ddns_username_th").innerHTML = "Domain Name";
		document.getElementById("ddns_password_th").innerHTML = "<#LANHostConfig_x_DDNSPassword_itemname#>";
		document.getElementById("ddns_hostname_th").innerHTML = default_hostname_label;
	}
	else if(v == "FREEDNS.AFRAID.ORG") {
		document.getElementById("ddns_username_th").innerHTML = "Username";
		document.getElementById("ddns_password_th").innerHTML = "<#PPPConnection_Password_itemname#>";
		document.getElementById("ddns_hostname_th").innerHTML = default_hostname_label;
	}
	else if (v == "WWW.TUNNELBROKER.NET") {
		document.getElementById("ddns_username_th").innerHTML = "Account Name";
		document.getElementById("ddns_password_th").innerHTML = "Update Key";
		document.getElementById("ddns_hostname_th").innerHTML = "Tunnel ID";
	}
	else {
		document.getElementById("ddns_username_th").innerHTML = "<#LANHostConfig_x_DDNSUserName_itemname#>";
		document.getElementById("ddns_password_th").innerHTML = "<#LANHostConfig_x_DDNSPassword_itemname#>";
		document.getElementById("ddns_hostname_th").innerHTML = default_hostname_label;
	}

	if(letsencrypt_support){
		document.getElementById("le_crypt").style.display = "";
	}
}

function change_cert_method(cert_method){
	var html_code = "";
	if(letsencrypt_support){
		if(cert_method === undefined){
			//Let's encrypt default on
			if(ddns_server_x_t != document.form.ddns_server_x.value && orig_le_enable == "0")
				cert_method = "0";
			else
				cert_method = orig_le_enable;

			switch(cert_method){
				case "0":
					document.form.le_enable[2].checked = true;
					break;
				case "1":
					document.form.le_enable[0].checked = true;
					break;
				case "2":
					document.form.le_enable[1].checked = true;
					break;
			}
		}

		switch(cert_method){
			case "0":
				document.getElementById("cert_desc").style.display = "none";
				document.getElementById("cert_act").style.display = "none";
				document.getElementById("CAcert_details").style.display = "";
				document.getElementById("cert_details").style.display = "";

				break;

			case "1":
				document.getElementById("cert_desc").style.display = "";
				document.getElementById("le_desc").innerHTML = "<#LANHostConfig_x_DDNSLetsEncrypt_desc#>";
				html_code = '<div style="margin-top:5px;"><input type="checkbox" name="letsEncryptTerm_check" checked>';
				html_code += "<#DDNS_https_cert_LetsEncrypt_agree#>";
				html_code += '<a href="https://letsencrypt.org/documents/LE-SA-v1.2-November-15-2017.pdf" target="_blank" style="margin-left: 5px; color:#FFF; text-decoration: underline;">Term of Service</a>'
				html_code += "</div>";
				document.getElementById("cert_act").innerHTML = html_code;

				if(orig_le_enable != "1")
					document.getElementById("cert_act").style.display = "";
				else
					document.getElementById("cert_act").style.display = "none";

				document.getElementById("CAcert_details").style.display = "none";
				document.getElementById("cert_details").style.display = "";

				if(orig_le_enable == "1")
					document.form.letsEncryptTerm_check.checked = true;

				break;

			case "2":
				document.getElementById("cert_desc").style.display = "none";
				html_code += '<div style="display:table-cell"><input class="button_gen" onclick="open_upload_window();" type="button" value="<#CTL_upload#>"/><img id="loadingicon" style="margin-left:5px;display:none;" src="/images/InternetScan.gif"></div>';
				document.getElementById("cert_act").innerHTML = html_code;
				document.getElementById("cert_act").style.display = "";
				document.getElementById("CAcert_details").style.display = "";
				document.getElementById("cert_details").style.display = "";

				break;
		}
	}
}

function open_upload_window(){
	$("#upload_cert_window").fadeIn(300);
}

function hide_upload_window(){
	$("#upload_cert_window").fadeOut(300);
}

function show_cert_details(){
	if(httpd_cert_info.issueTo != "" && httpd_cert_info.issueBy != "" && httpd_cert_info.expire != ""){
		if(orig_le_enable == "1" && le_state == "0"){
			if(httpd_cert_info.issueBy.indexOf("Let's Encrypt") == -1)
				document.getElementById("cert_status").innerHTML = "<#vpn_openvpn_KC_Authorizing#>";
			else
				document.getElementById("cert_status").innerHTML = "<#upgrade_processing#>";
			setTimeout("get_cert_info();", 1000);
		}
		else{
			document.getElementById("cert_status").innerHTML = "<#Status_Active#>";
			document.getElementById("CAissueTo").innerHTML = httpd_cert_info.CAissueTo;
			document.getElementById("CAissueBy").innerHTML = httpd_cert_info.CAissueBy;
			document.getElementById("CAexpireOn").innerHTML = httpd_cert_info.CAexpire;
			document.getElementById("issueTo").innerHTML = httpd_cert_info.issueTo;
			document.getElementById("issueBy").innerHTML = httpd_cert_info.issueBy;
			document.getElementById("expireOn").innerHTML = httpd_cert_info.expire;
		}
	}
	else{
		if(le_auxstate_t == "5" && le_sbstate_t == "7"){
			var ddnsHint = "<#DDNS_Auth_Fail_Hint#>";
			$("#cert_status").text(ddnsHint);
			$("#cert_status").css("color", "#FFCC00")
		}
		else{
			document.getElementById("cert_status").innerHTML = "Unknown or processing...";
			setTimeout("get_cert_info();", 1000);
		}
	}
}

function check_filename(){
	var key_file = document.upload_form.file_key.value;
	var cert_file = document.upload_form.file_cert.value;
	var key_subname = key_file.substring(key_file.lastIndexOf('.') + 1);
	var cert_subname = cert_file.substring(cert_file.lastIndexOf('.') + 1);

	if(key_subname != 'pem' && key_subname != 'key'){
		alert("<#DDNS_https_key_alert#>");
		document.upload_form.file_key.value = "";
		document.upload_form.file_key.focus();
		return false;
	}

	if(cert_subname != 'pem' && cert_subname != 'crt' && cert_subname != 'cer'){
		alert("<#DDNS_https_cert_alert#>");
		document.upload_form.file_cert.value = "";
		document.upload_form.file_cert.focus();
		return false;
	}

	return true;
}

function upload_cert_key(){
	if(check_filename()){
		document.upload_form.submit();
		hide_upload_window();
		show_cert_details();
		httpd_restart = 1;
	}
}

function save_cert_key(){
	location.href = "cert_key.tar";
}

function show_ddns_status_detail() {
	var ddnsHint = getDDNSState(ddns_return_code_chk, ddns_hostname_x_t, ddns_old_name);

	if(ddnsHint != ""){
		overlib(ddnsHint);
	}
}

function asuscomm_deregister(){
	$.ajax({
		url: "/unreg_ASUSDDNS.cgi",

		success: function( response ) {
			check_unregister_result();
		}
	});
}

function clean_ddns(){
	$.ajax({
		url: "/clean_ddns.cgi",

		success: function( response ) {
			setTimeout(function(){
			alert("<#LANHostConfig_x_DDNS_alarm_16#>");
			refreshpage();
			}, 2000);
		}
	});
}

var max_retry_count = 6;
var retry_count = 0;
function check_unregister_result(){
	var asusddns_reg_result = httpApi.nvramGet(["asusddns_reg_result"], true).asusddns_reg_result;
	var action_type = asusddns_reg_result.slice(0, asusddns_reg_result.indexOf(','));
	var return_status = "";
	var timeout = 0;

	if(action_type != "unregister" && retry_count < max_retry_count){
		setTimeout(check_unregister_result, 500);
		retry_count++;
	}
	else if(action_type == "unregister"){
		return_status = asusddns_reg_result.slice(asusddns_reg_result.indexOf(',') + 1);
	}
	else if(retry_count == max_retry_count){
		timeout = 1;
	}

	if(timeout || return_status != ""){
		if(return_status == "200"){
			clean_ddns();
		}
		else{
			hideLoading();
			alert("<#LANHostConfig_x_DDNS_alarm_18#>");
			deregister_fail = 1;
			retry_count = 0;
		}
	}

}

function show_feature_desc(){
	$(".container").addClass("blur_effect");
	if($(".popup_container.popup_element").css("display") == "flex"){
		$(".popup_container.popup_element").addClass("blur_effect");
	}
	$(".popup_element_second").css("display", "flex");
	$(".popup_container.popup_element_second").empty();
	$(".popup_container.popup_element_second").append(Get_Component_Feature_Desc());

    function close_popup(){
    	$(".popup_element_second").hide();
    	$(".container, .qis_container").removeClass("blur_effect");
    	$(".popup_container.popup_element").removeClass().addClass("popup_container popup_element").empty();
    }

	function Get_Component_Feature_Desc(){
		var $container = $("<div>");
		var $popup_title_container = $("<div>").addClass("popup_title_container");
		$popup_title_container.appendTo($container);
		$("<div>").addClass("title").html("<#NewFeatureAbout#>").appendTo($popup_title_container);
		var $close_btn = $("<div>").addClass("close_btn").html('&times;');
		$close_btn.appendTo($popup_title_container);
		$close_btn.unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
            close_popup();
		});

		var $popup_content_container = $("<div>").addClass("popup_content_container");
		$popup_content_container.appendTo($container);

		var $feature_desc_cntr = $("<div>").addClass("feature_desc_container").appendTo($popup_content_container);
		$("<div>").addClass("title").html("FAQ").appendTo($feature_desc_cntr);
		$("<div>").addClass("desc").html(
			'<div class="text-list"><div class="icon-circle-mask"><i class="icon-comments"></i></div><a target="_blank" href="https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang='+ui_lang+'&kw=&num=168"><#LANHostConfig_x_DDNSEnable_faq1#></a></div>'+
            '<div class="text-list"><div class="icon-circle-mask"><i class="icon-comments"></i></div><a target="_blank" href="https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang='+ui_lang+'&kw=&num=169"><#LANHostConfig_x_DDNSEnable_faq2#></a></div>'+
            '<div class="text-list"><div class="icon-circle-mask"><i class="icon-comments"></i></div><a target="_blank" href="https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang='+ui_lang+'&kw=&num=170"><#LANHostConfig_x_DDNSEnable_faq3#></a></div>'
		).appendTo($feature_desc_cntr);/* untranslated */

		return $container;
	}
}

function copyDdnsInputValue(e) {
    let text = '';
    if($('#ddnsname_input').css('display') !== 'none'){
        text = $('input[name=ddns_hostname_x]').val();
    }else if($('#asusddnsname_input').css('display') !== 'none'){
        text = $('input[name=DDNSName]').val()+$('#domain_text').html();
    }

    if (window.isSecureContext && navigator.clipboard) {
        navigator.clipboard.writeText(text);
    } else {
        const textArea = document.createElement("textarea");
        textArea.value = text;
        document.body.appendChild(textArea);
        textArea.select();
        try {
            document.execCommand('copy')
        } catch (err) {
            console.error('Unable to copy to clipboard', err)
        }
        document.body.removeChild(textArea)
    }
    let span = $("<span>").addClass("tooltiptext").html($(e).data('title'));
    $(e).parent().append(span);
    span.show().fadeOut(1500, function() { $(this).remove(); });
}

function showDescTooltip(e){
    let ddnsname = '';
    if($('#ddnsname_input').css('display') !== 'none'){
        ddnsname = $('input[name=ddns_hostname_x]').val();
    }else if($('#asusddnsname_input').css('display') !== 'none'){
        ddnsname = $('input[name=DDNSName]').val()+$('#domain_text').html();
    }

    let span = $("<span>").addClass("tooltiptextdown");
    if($(e).parent().find('.tooltiptextdown').length == 0){
        $(e).parent().append(span);
    }
    $(e).parent().find('.tooltiptextdown').html('You can use “'+ddnsname+'” to connect to the home network and manage it when you’re out.');
}

function clear_cert_key(){
	if(confirm("You will be automatically logged out for the renewal, are you sure you want to continue?")){
		$.ajax({url: "clear_file.cgi?clear_file_name=cert.tgz"})
		showLoading();
		setTimeout(refreshpage, 1000);
        }
}

</script>
</head>

<body onload="init();" onunLoad="return unload_body();" class="bg">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_ASUSDDNS_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="restart_ddns">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="ddns_enable_x" value="<% nvram_get("ddns_enable_x"); %>">

<table class="content" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td width="17">&nbsp;</td>

		<td valign="top" width="202">
		<div  id="mainMenu"></div>
		<div  id="subMenu"></div>
		</td>

    <td valign="top">
		<div id="tabMenu" class="submenuBlock"></div>
		<!--===================================Beginning of Main Content===========================================-->
<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
	<tr>
		<td align="left" valign="top" >

		<table width="760px" border="0" cellpadding="5" cellspacing="0" class="FormTitle" id="FormTitle">
		<tbody>
			<tr>
		  		<td bgcolor="#4D595D" valign="top">
		  		<div class="container">
		  		<div class="page_title_div">
                    <div class="formfonttitle"><#menu5_3#> - <#menu5_3_6#> </div>
                    <div class="formfonttitle_help" style="position: absolute;right: 5px;top: 5px;"><i onclick="show_feature_desc()" class="icon_help"></i></div>
		  		</div>
		  		<div style="margin:10px 0 10px 5px;" class="splitLine"></div>

		 		<div class="formfontdesc formfontdesc_help_left">
					<div><#LANHostConfig_x_DDNSEnable_sectiondesc#></div>
				</div>

				<div class="formfontdesc hint-color" id="wan_ip_hide2" style="color:#FC0; display:none;"><#LANHostConfig_x_DDNSEnable_sectiondesc4#><#LANHostConfig_x_DDNSEnable_sectiondesc2#></div>
				<div class="formfontdesc hint-color" id="lb_note" style="color:#FC0; display:none;"><#lb_note_ddns#></div>
				<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
				<input type="hidden" name="wl_gmode_protection_x" value="<% nvram_get("wl_gmode_protection_x"); %>">
			<tr>
				<th><#LANHostConfig_x_DDNSEnable_itemname#></th>
				<td>
				    <div id="radio_ddns_enable"></div>
                    <script type="text/javascript">
                    $("#radio_ddns_enable").addClass("icon_switch");
                    if('<% nvram_get("ddns_enable_x"); %>'=='1')
                        $("#radio_ddns_enable").addClass("on");
                    else
                        $("#radio_ddns_enable").removeClass("on");
                    $("#radio_ddns_enable").click(function(e){
                        e = e || event;
                        e.stopPropagation();
                        $(this).toggleClass("on");
                        if($(this).hasClass("on")){
                            document.form.ddns_enable_x.value = "1";
                            change_cert_method();
                            show_cert_details();
                            change_common_radio(this, 'LANHostConfig', 'ddns_enable_x', '1');
                        }else{
                            document.form.ddns_enable_x.value = "0";
                            change_common_radio(this, 'LANHostConfig', 'ddns_enable_x', '0');
                        }
                    });
                    </script>
                    <div style="color:#FFCC00">*<#LANHostConfig_x_DDNSEnable_security_info#></div>
				</td>
			</tr>
			<tr>
				<th id="ddns_wan_unit_th"><#wan_interface#></th>
				<td id="ddns_wan_unit_td">
				<select name="ddns_wan_unit" class="input_option">
					<option class="content_input_fd" value="-1" <% nvram_match("ddns_wan_unit", "-1","selected"); %>><#Auto#></option>
					<option class="content_input_fd" value="0" <% nvram_match("ddns_wan_unit", "0","selected"); %>><#dualwan_primary#></option>
					<option class="content_input_fd" value="1"<% nvram_match("ddns_wan_unit", "1","selected"); %>><#dualwan_secondary#></option>
				</select>
				</td>
			</tr>
			<tr id="ddns_ipv6update_tr" style="display: none;">
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,18);"><#DDNS_ipv6_update#></a></th>
				<td>
					<input type="radio" name="ddns_ipv6_update" class="input" value="1" <% nvram_match("ddns_ipv6_update", "1", "checked"); %>><#checkbox_Yes#>
					<input type="radio" name="ddns_ipv6_update" class="input" value="0" <% nvram_match("ddns_ipv6_update", "0", "checked"); %>><#checkbox_No#>
				</td>
			</tr>
			<tr>
				<th><#LANHostConfig_x_DDNSServer_itemname#></th>
				<td>
					<select id="ddns_server_x" name="ddns_server_x" class="input_option" onchange="change_ddns_setting(this.value); change_cert_method();">
						<option value="WWW.ASUS.COM" <% nvram_match("ddns_server_x", "WWW.ASUS.COM","selected"); %>>WWW.ASUS.COM</option>
						<option value="WWW.ASUS.COM.CN" <% nvram_match("ddns_server_x", "WWW.ASUS.COM.CN","selected"); %>>WWW.ASUS.COM.CN</option>
						<option value="DOMAINS.GOOGLE.COM" <% nvram_match("ddns_server_x", "DOMAINS.GOOGLE.COM","selected"); %>>DOMAINS.GOOGLE.COM</option>
						<option value="WWW.DYNDNS.ORG" <% nvram_match("ddns_server_x", "WWW.DYNDNS.ORG","selected"); %>>WWW.DYNDNS.ORG</option>
						<option value="WWW.DYNDNS.ORG(CUSTOM)" <% nvram_match("ddns_server_x", "WWW.DYNDNS.ORG(CUSTOM)","selected"); %>>WWW.DYNDNS.ORG(CUSTOM)</option>
						<option value="WWW.DYNDNS.ORG(STATIC)" <% nvram_match("ddns_server_x", "WWW.DYNDNS.ORG(STATIC)","selected"); %>>WWW.DYNDNS.ORG(STATIC)</option>
						<option value="WWW.SELFHOST.DE" <% nvram_match("ddns_server_x", "WWW.SELFHOST.DE","selected"); %>>WWW.SELFHOST.DE</option>
						<option value="WWW.ZONEEDIT.COM" <% nvram_match("ddns_server_x", "WWW.ZONEEDIT.COM","selected"); %>>WWW.ZONEEDIT.COM</option>
						<option value="WWW.DNSOMATIC.COM" <% nvram_match("ddns_server_x", "WWW.DNSOMATIC.COM","selected"); %>>WWW.DNSOMATIC.COM</option>
						<option value="DNS.HE.NET" <% nvram_match("ddns_server_x", "DNS.HE.NET","selected"); %>>HE.NET</option>
						<option value="WWW.TUNNELBROKER.NET" <% nvram_match("ddns_server_x", "WWW.TUNNELBROKER.NET","selected"); %>>WWW.TUNNELBROKER.NET</option>
						<option value="WWW.NO-IP.COM" <% nvram_match("ddns_server_x", "WWW.NO-IP.COM","selected"); %>>WWW.NO-IP.COM</option>
						<option value="WWW.ORAY.COM" <% nvram_match("ddns_server_x", "WWW.ORAY.COM","selected"); %>>WWW.ORAY.COM(花生壳)</option>
						<option value="WWW.NAMECHEAP.COM" <% nvram_match("ddns_server_x", "WWW.NAMECHEAP.COM","selected"); %>>WWW.NAMECHEAP.COM</option>
						<option value="FREEDNS.AFRAID.ORG" <% nvram_match("ddns_server_x", "FREEDNS.AFRAID.ORG","selected"); %>>FREEDNS.AFRAID.ORG</option>
						<option value="CUSTOM" <% nvram_match("ddns_server_x", "CUSTOM","selected");  %>>Custom</option>
					</select>
					<input id="deregister_btn" class="button_gen" style="display: none; margin-left: 5px;" type="button" value="<#CTL_Deregister#>"/>
					<a id="link" href="javascript:openLink('x_DDNSServer')" style=" margin-left:5px; text-decoration: underline;"><#LANHostConfig_x_DDNSServer_linkname#></a>
					<a id="linkToHome" href="javascript:openLink('x_DDNSServer')" style=" margin-left:5px; text-decoration: underline;"><#ddns_home_link#></a>
					<div id="customnote" style="display:none;"><span>For the Custom DDNS you must manually create a ddns-start script that handles your custom notification.</span></div>
					<div id="need_custom_scripts" style="display:none;"><span>WARNING: you must enable both the JFFS2 partition and custom scripts support!<br>Click <a href="Advanced_System_Content.asp" style="text-decoration: underline;">HERE</a> to proceed.</span></div>
				</td>
			</tr>
			<tr id="ddns_hostname_tr">
				<th id="ddns_hostname_th"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,13);"><#LANHostConfig_x_DDNSHostNames_itemname#></a></th>
				<td>
				<div style="display: flex; align-items: center; gap:5px;">
					<div id="ddnsname_input" style="display:none;">
						<input type="text" maxlength="63" class="input_25_table" name="ddns_hostname_x" id="ddns_hostname_x" value="<% nvram_get("ddns_hostname_x"); %>" onKeyPress="return validator.isString(this, event)" autocorrect="off" autocapitalize="off">
					</div>
					<div id="asusddnsname_input" style="width:500px; display:none;">
						<input type="text" maxlength="50" class="input_32_table" name="DDNSName" id="DDNSName" class="inputtext" onKeyPress="return validator.isString(this, event)" OnClick="cleandef();" autocorrect="off" autocapitalize="off"><span id="domain_text" style="color: #FFFFFF;">.asuscomm.com</span>
						<div id="alert_block" style="color:#FFCC00; margin-left:5px; font-size:11px;display:none;">
								<span id="alert_str"></span>
						</div>
					</div>
					<div class="tooltip"><a onClick="copyDdnsInputValue(this)" onmouseover="showDescTooltip(this)" data-toggle="tooltip" data-title="Copied!"><i class="icon-clone"></i></a></div>
					</div>
				</td>
			</tr>
			<tr id="ddns_hostname_info_tr" style="display:none;">
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,13);"><#LANHostConfig_x_DDNSHostNames_itemname#></a></th>
				<td id="ddns_hostname_x_value"><% nvram_get("ddns_hostname_x"); %></td>
			</tr>
			<tr>
				<th id="ddns_username_th"><#LANHostConfig_x_DDNSUserName_itemname#></th>
				<td><input type="text" maxlength="32" class="input_25_table" name="ddns_username_x" value="<% nvram_get("ddns_username_x"); %>" onKeyPress="return validator.isString(this, event)" autocomplete="off" autocorrect="off" autocapitalize="off"></td>
			</tr>
			<tr>
				<th id="ddns_password_th"><#LANHostConfig_x_DDNSPassword_itemname#></th>
				<td><input type="password" maxlength="64" class="input_25_table" name="ddns_passwd_x" value="<% nvram_get("ddns_passwd_x"); %>" autocomplete="off" autocorrect="off" autocapitalize="off"></td>
			</tr>
			<tr id="wildcard_field">
				<th><#LANHostConfig_x_DDNSWildcard_itemname#></th>
				<td>
					<input type="radio" value="1" name="ddns_wildcard_x" onClick="return change_common_radio(this, 'LANHostConfig', 'ddns_wildcard_x', '1')" <% nvram_match("ddns_wildcard_x", "1", "checked"); %>><#checkbox_Yes#>
					<input type="radio" value="0" name="ddns_wildcard_x" onClick="return change_common_radio(this, 'LANHostConfig', 'ddns_wildcard_x', '0')" <% nvram_match("ddns_wildcard_x", "0", "checked"); %>><#checkbox_No#>
				</td>
			</tr>
			<tr id="check_ddns_field" style="display:none;">
				<th><#DDNS_verification_enable#></th>
				<td>
					<input type="radio" value="1" name="ddns_regular_check" onClick="change_ddns_setting(document.form.ddns_server_x.value);" <% nvram_match("ddns_regular_check", "1", "checked"); %>><#checkbox_Yes#>
					<input type="radio" value="0" name="ddns_regular_check" onClick="change_ddns_setting(document.form.ddns_server_x.value);" <% nvram_match("ddns_regular_check", "0", "checked"); %>><#checkbox_No#>
				</td>
			</tr>
			<tr style="display:none;">
				<th><#DDNS_verification_frequency#></th>
				<td>
					<input type="text" maxlength="5" class="input_6_table" name="ddns_regular_period" value="<% nvram_get("ddns_regular_period"); %>" autocorrect="off" autocapitalize="off"> <#Minute#>
				</td>
			</tr>
			<tr style="display:none;">
				<th>Forced update interval (in days)</th>
				<td>
					<input type="text" maxlength="3" name="ddns_refresh_x" class="input_3_table" value="<% nvram_get("ddns_refresh_x"); %>" onKeyPress="return validator.isNumber(this,event)">
				</td>
			</tr>
			<tr style="display:none;">
				<th><#LANHostConfig_x_DDNSStatus_itemname#></th>
				<td>
					<input type="hidden" maxlength="15" class="button_gen" size="12" name="" value="<% nvram_get("DDNSStatus"); %>">
				  	<input type="submit" maxlength="15" class="button_gen" onclick="showLoading();return onSubmitApply('ddnsclient');" size="12" name="LANHostConfig_x_DDNSStatus_button" value="<#LANHostConfig_x_DDNSStatus_buttonname#>" /></td>
			</tr>
			<tr id="ddns_status_tr" style="display:none;">
				<th>DDNS Status</th>
				<td><span id="ddns_status" style="color:#FFCC00"></span><span id="ddns_status_detail" class="notificationon" style="display: none;" onmouseover="show_ddns_status_detail();" onMouseOut="nd();"></span></td>
			</tr>
			<tr id="ddns_result_tr" style="display:none;">
				<th>DDNS Registration Result</th>
				<td id="ddns_result"></td>
			</tr>
			<tr id="https_cert" style="display:none;">
				<th><#DDNS_https_cert#></th>
				<td>
					<span id="le_crypt" style="color:#FFF;display:none;">
					<input type="radio" value="1" name="le_enable" onClick="change_cert_method(this.value);" <% nvram_match("le_enable", "1", "checked"); %>><#DDNS_https_cert_LetsEncrypt#>
					</span>
					<input type="radio" value="2" name="le_enable" onClick="change_cert_method(this.value);" <% nvram_match("le_enable", "2", "checked"); %>><#DDNS_https_cert_Import#>
					<span id="self_signed" style="color:#FFF;">
					<input type="radio" value="0" name="le_enable" onClick="change_cert_method(this.value);" <% nvram_match("le_enable", "0", "checked"); %>><#Auto#>
					</span>
					<div id="cert_desc" style="color:#FFCC00; margin-top: 5px;">
						<span id="le_desc"></span>
						<span id="le_faq">
							<a id="faq" href="" target="_blank" style="margin-left: 5px; color:#FFCC00; text-decoration: underline;">FAQ</a>
						</span>
					</div>
					<div id="cert_act" style="margin-top: 5px;"></div>
				</td>
			</tr>

			<tr id="CAcert_details" style="display:none;">
				<th>Root Certificate/Intermediate Certificate</th>
				<td>
					<div style="display: flex;">
						<div class="cert_status_title"><#vpn_openvpn_KC_to#> :</div>
						<div id="CAissueTo" class="cert_status_val"></div>
					</div>
					<div style="display: flex;">
						<div class="cert_status_title"><#vpn_openvpn_KC_by#> :</div>
						<div id="CAissueBy" class="cert_status_val"></div>
					</div>
					<div style="display: flex;">
						<div class="cert_status_title"><#vpn_openvpn_KC_expire#> :</div>
						<div id="CAexpireOn" class="cert_status_val"></div>
					</div>
				</td>
			</tr>
			<tr id="cert_details" style="display:none;">
				<th><#vpn_openvpn_KC_SA#></th>
				<td>
					<div style="display: flex;">
						<div class="cert_status_title"><#Status_Str#> :</div>
						<div id="cert_status" class="cert_status_val"></div>
					</div>
					<div style="display: flex;">
						<div class="cert_status_title"><#vpn_openvpn_KC_to#> :</div>
						<div id="issueTo" class="cert_status_val"></div>
					</div>
					<div style="display: flex;">
						<div class="cert_status_title"><#vpn_openvpn_KC_by#> :</div>
						<div id="issueBy" class="cert_status_val"></div>
					</div>
					<div style="display: flex;">
						<div class="cert_status_title"><#vpn_openvpn_KC_expire#> :</div>
						<div id="expireOn" class="cert_status_val"></div>
					</div>
					<div>
						<input class="button_gen" onclick="save_cert_key();" type="button" value="<#btn_Export#>" />
					</div>
				</td>
			</tr>
		</table>
				<div class="apply_gen">
					<input class="button_gen" onclick="apply_eula_check();" type="button" value="<#CTL_apply#>" />
				</div>
				</div>

                <div class="popup_container popup_element_second"></div>
			  </td>
              </tr>
            </tbody>

            </table>
	  </td>
</form>
        </tr>
      </table>
		<!--===================================Ending of Main Content===========================================-->
	</td>

    <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>

<div id="footer"></div>
<form method="post" name="upload_form" action="upload_cert_key.cgi" target="hidden_frame" enctype="multipart/form-data">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="le_enable" value="2">
<div id="upload_cert_window"  class="contentM_upload" style="box-shadow: 1px 5px 10px #000;">
	<div class="formfonttitle" style="margin-top: 15px; margin-left: 15px;"><#DDNS_https_cert_Import#></div>
	<div class="formfontdesc" style="margin: 15px;"><#DDNS_https_cert_Import_desc#></div>
	<div class="Upload_item">
		<div style="display:table-cell; width: 45%;"><#DDNS_https_cert_PrivateKey#> :</div>
		<div style="display:table-cell;"><input type="file" name="file_key" class="input Upload_file"></div>
	</div>
	<div class="Upload_item">
		<div style="display:table-cell; width: 45%;"><#DDNS_ssl_cert#> :</div>
		<div style="display:table-cell;"><input type="file" name="file_cert" class="input Upload_file"></div>
	</div>
	<div style="color: #FC0; margin-left: 15px; margin-top: 20px">* <#DDNS_https_cert_PrivateKey_note#></div>
	<div align="center" style="margin-top:30px; padding-bottom:15px; display: flex; justify-content: center;">
		<div style=""><input class="button_gen" type="button" onclick="hide_upload_window();" id="cancelBtn" value="<#CTL_Cancel#>"></div>
		<div style=""><input class="button_gen" type="button" onclick="upload_cert_key();" id="applyBtn" value="<#CTL_ok#>"></div>
	</div>
</div>
</form>
</body>
</html>
