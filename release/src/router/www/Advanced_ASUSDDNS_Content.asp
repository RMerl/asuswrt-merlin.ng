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
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" language="JavaScript" src="/help.js"></script>
<script type="text/javascript" language="JavaScript" src="/validator.js"></script>
<script type="text/javaScript" src="/js/jquery.js"></script>
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
</style>

<script>
<% wanlink(); %>
var ddns_hostname_x_t = '<% nvram_get("ddns_hostname_x"); %>';
var ddns_server_x_t = '<% nvram_get("ddns_server_x"); %>';
var ddns_updated_t = '<% nvram_get("ddns_updated"); %>';
var wans_mode ='<% nvram_get("wans_mode"); %>';
var no_phddns = isSupport("no_phddns");

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

function init(){
	show_menu();
	httpApi.faqURL("1034294", function(url){document.getElementById("faq").href=url;});
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
	if(ddns_enable_x == "1" && ddns_server_x == "WWW.ASUS.COM"){
		ASUS_EULA.check('asus');
	}
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
		if(document.form.https_crt_cn.value != '<% nvram_get("https_crt_cn"); %>'){
			document.form.https_crt_gen.value = "1";
		}
		if($("input[name='ddns_enable_x']:checked").val() == "1" && $("input[name='le_enable']:checked").val() == "1"){
			document.form.action_wait.value = "10";
			document.form.action_script.value = "restart_ddns_le";
		}
		else if(http_enable != "0" && ($("input[name='le_enable']:checked").val() != orig_le_enable || document.form.https_crt_gen.value == "1" || httpd_restart == 1 )){
			document.form.action_wait.value = "10";
			if(orig_le_enable == "1")
				document.form.action_script.value = "restart_httpd;restart_webdav;restart_ddns_le";
			else
				document.form.action_script.value += ";restart_httpd;restart_webdav";

		}
		if (('<% nvram_get("enable_ftp"); %>' == "1") && ('<% nvram_get("ftp_tls"); %>' == "1")) {
			document.form.action_script.value += ";restart_ftpd";
		}
	}

	document.form.submit();
	showLoading();
}

function check_update(){
    var ddns_ipaddr_t = '<% nvram_get("ddns_ipaddr"); %>';
		ddns_ipaddr_t = ddns_ipaddr_t.replace(/&#10/g,"");

    if (document.form.ddns_enable_x[0].checked == true &&
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

function ddns_load_body(){
    if(ddns_enable_x == 1){
        inputCtrl(document.form.ddns_server_x, 1);
        document.getElementById('ddns_hostname_tr').style.display = "";
        if(ddns_server_x == "WWW.ASUS.COM" || ddns_server_x == ""){
            document.form.ddns_hostname_x.parentNode.style.display = "none";
            document.form.DDNSName.parentNode.style.display = "";
            var ddns_hostname_title = ddns_hostname_x_t.substring(0, ddns_hostname_x_t.indexOf('.asuscomm.com'));
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
	showhide("ddns_ipcheck_tr", 1);

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
	showhide("ddns_ipcheck_tr", 0);
    }

    if(letsencrypt_support){
        show_cert_settings(1);
        change_cert_method(orig_le_enable);
        show_cert_details();
    }

    hideLoading();

	if(ddns_enable_x == "1" && !deregister_fail)
	{
		var ddnsHint = getDDNSState(ddns_return_code, ddns_hostname_x_t, ddns_old_name);

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
					$("#deregister_btn").css("display", "inline");
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
	if(document.form.ddns_enable_x[0].checked == true && document.form.ddns_server_x.value == "WWW.ASUS.COM"){
		if(!ASUS_EULA.check("asus")) return false;
	}
	
	applyRule();
}

function applyRule(){
	if(validForm()){
		if(document.form.ddns_enable_x[0].checked == true && document.form.ddns_server_x.selectedIndex == 0){
			document.form.ddns_hostname_x.value = document.form.DDNSName.value+".asuscomm.com";
		}

		check_update();
	}
}

function validForm(){
	if(document.form.ddns_enable_x[0].checked){		//ddns enable
		if(document.form.ddns_server_x.selectedIndex == 0){		//WWW.ASUS.COM	
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
						if(!confirm("Disagree the term of service of Let's Encrypt will change the certificate method to \"None\". Are you sure?")){
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
				if(document.form.ddns_username_x.value == ""){
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
	else if (document.form.le_enable[0].checked == true) {
		alert("Let's Encrypt requires DDNS to be enabled.");
		return false;
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
	if (v == "WWW.ASUS.COM"){
			document.getElementById("ddns_hostname_info_tr").style.display = "none";
			document.getElementById("ddns_hostname_tr").style.display="";
			document.form.ddns_hostname_x.parentNode.style.display = "none";
			document.form.DDNSName.parentNode.style.display = "";
			var ddns_hostname_title = ddns_hostname_x_t.substring(0, ddns_hostname_x_t.indexOf('.asuscomm.com'));
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

			if(ddns_enable_x == "1" && ddns_server_x_t == "WWW.ASUS.COM" &&
				(ddns_return_code_chk.indexOf('200')!=-1 || ddns_return_code_chk.indexOf('220')!=-1 || ddns_return_code_chk == 'register,230')){
				document.getElementById("ddns_status").innerHTML = "<#Status_Active#>";
				if(inadyn)
					$("#deregister_btn").css("display", "inline");
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
			if (('<% nvram_get("jffs2_enable"); %>' != '1') || ('<% nvram_get("jffs2_scripts"); %>' != '1'))
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
			inputCtrl(document.form.ddns_username_x, 1);
			inputCtrl(document.form.ddns_passwd_x, 1);
			var disable_wild = 0;
			if(v == "WWW.TUNNELBROKER.NET" || v == "WWW.SELFHOST.DE" || v == "DOMAINS.GOOGLE.COM")
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
				cert_method = "1";
			else
				cert_method = orig_le_enable;

			switch(cert_method){
				case "0":
					document.form.le_enable[2].checked = true;
					break;
				case "1":
					if(document.form.ddns_enable_x[1].checked) {
						alert("Let's Encrypt requires DDNS to be enabled.");
						document.form.le_enable[2].checked = true;
					} else {
						document.form.le_enable[0].checked = true;
					}
					break;
				case "2":
					document.form.le_enable[1].checked = true;
					break;
			}
		}

		switch(cert_method){
			case "0":
				document.getElementById("cert_gen").style.display = "";
				document.getElementById("cert_san").style.display = "";
				document.getElementById("cert_desc").style.display = "none";
				document.getElementById("cert_act").style.display = "none";
				document.getElementById("cert_details").style.display = "";
				break;

			case "1":
				if(document.form.ddns_enable_x[1].checked) {
					alert("Let's Encrypt requires DDNS to be enabled.");
					document.form.le_enable[2].checked = true;
					return false;
				}
				document.getElementById("cert_gen").style.display = "none";
				document.getElementById("cert_san").style.display = "none";
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

					document.getElementById("cert_details").style.display = "";

				if(orig_le_enable == "1")
					document.form.letsEncryptTerm_check.checked = true;

				break;

			case "2":
				document.getElementById("cert_gen").style.display = "none";
				document.getElementById("cert_san").style.display = "none";
				document.getElementById("cert_desc").style.display = "none";
				html_code += '<div style="display:table-cell"><input class="button_gen" onclick="open_upload_window();" type="button" value="<#CTL_upload#>"/><img id="loadingicon" style="margin-left:5px;display:none;" src="/images/InternetScan.gif"></div>';
				document.getElementById("cert_act").innerHTML = html_code;
				document.getElementById("cert_act").style.display = "";
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
			document.getElementById("SAN").innerHTML = httpd_cert_info.SAN;
			document.getElementById("issueTo").innerHTML = httpd_cert_info.issueTo;
			document.getElementById("issueBy").innerHTML = httpd_cert_info.issueBy;
			document.getElementById("expireOn").innerHTML = httpd_cert_info.expire;
		}
	}
	else{
		if(orig_le_enable == "1") {
			document.getElementById("cert_status").innerHTML = "<#vpn_openvpn_KC_Authorizing#>";
			setTimeout("get_cert_info();", 1000);
		} else {
			document.getElementById("cert_status").innerHTML = "No certificate found";
		}
	}
}

function check_filename(){
	var key_file = document.upload_form.file_key.value;
	var cert_file = document.upload_form.file_cert.value;
	var key_subname = key_file.substring(key_file.lastIndexOf('.') + 1);
	var cert_subname = cert_file.substring(cert_file.lastIndexOf('.') + 1);

	if(key_subname != 'pem' && key_subname != 'key'){
		alert("Please select correct private key file.");
		document.upload_form.file_key.value = "";
		document.upload_form.file_key.focus();
		return false;
	}

	if(cert_subname != 'pem' && cert_subname != 'crt' && cert_subname != 'cer'){
		alert("Please select correct SSL certificate file.");
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
			alert("Deregistration is successful.");
			refreshpage();
		}
		else{
			hideLoading();
			alert("Fail to deregister ASUSDDNS hostname.");
			deregister_fail = 1;
			retry_count = 0;
		}
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
		  		<td bgcolor="#4D595D" valign="top"  >
		  		<div>&nbsp;</div>
		  		<div class="formfonttitle"><#menu5_3#> - <#menu5_3_6#></div>
		  		<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
		 		<div class="formfontdesc"><#LANHostConfig_x_DDNSEnable_sectiondesc#></div>
		 		<div class="formfontdesc" style="margin-top:-8px;"><#NSlookup_help#></div>
				<div class="formfontdesc" id="wan_ip_hide2" style="color:#FFCC00; display:none;">The wireless router currently uses a private WAN IP address.<p>This router may be in the multiple-NAT environment.  While using an External check might allow DDNS to reflect the correct IP address, this might still interfere with remote access services.</div>
				<div class="formfontdesc" id="wan_ip_hide3" style="color:#FFCC00; display:none;"><#LANHostConfig_x_DDNSEnable_sectiondesc3#></div>
				<div class="formfontdesc" id="lb_note" style="color:#FFCC00; display:none;"><#lb_note_ddns#></div>
				<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
	                        <thead>
	                        <tr>
	                                <td colspan="2">DDNS Service</td>
	                        </tr>
	                        </thead>
			<tr>
				<th><#LANHostConfig_x_DDNSEnable_itemname#></th>
				<td>
				<input type="radio" value="1" name="ddns_enable_x" onClick="change_cert_method();show_cert_details();return change_common_radio(this, 'LANHostConfig', 'ddns_enable_x', '1')" <% nvram_match("ddns_enable_x", "1", "checked"); %>><#checkbox_Yes#>
				<input type="radio" value="0" name="ddns_enable_x" onClick="return change_common_radio(this, 'LANHostConfig', 'ddns_enable_x', '0')" <% nvram_match("ddns_enable_x", "0", "checked"); %>><#checkbox_No#>
				</td>
			</tr>
			<tr id="ddns_status_tr" style="display:none;">
				<th>DDNS Status</th>
				<td><sapn id="ddns_status" style="color:#FFCC00"></sapn><span id="ddns_status_detail" class="notificationon" style="display: none;" onmouseover="show_ddns_status_detail();" onMouseOut="nd();"></span></td>
			</tr>
			<tr id="ddns_result_tr" style="display:none;">
				<th>DDNS Registration Result</th>
				<td id="ddns_result"></td>
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
			<tr id="ddns_ipcheck_tr">
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(50,26);">Method to retrieve WAN IP</a></th>
                                <td>
				<select name="ddns_realip_x" class="input_option">
					<option class="content_input_fd" value="0" <% nvram_match("ddns_realip_x", "0","selected"); %>>Internal</option>
					<option class="content_input_fd" value="1" <% nvram_match("ddns_realip_x", "1","selected"); %>>External</option>
				</select>
				</td>
			</tr>
			<tr>
				<th><#LANHostConfig_x_DDNSServer_itemname#></th>
				<td>
					<select name="ddns_server_x"class="input_option" onchange="change_ddns_setting(this.value); change_cert_method();">
						<option value="WWW.ASUS.COM" <% nvram_match("ddns_server_x", "WWW.ASUS.COM","selected"); %>>WWW.ASUS.COM</option>
						<option value="DOMAINS.GOOGLE.COM" <% nvram_match("ddns_server_x", "DOMAINS.GOOGLE.COM","selected"); %>>DOMAINS.GOOGLE.COM</option>
						<option value="WWW.DYNDNS.ORG" <% nvram_match("ddns_server_x", "WWW.DYNDNS.ORG","selected"); %>>WWW.DYNDNS.ORG</option>
						<option value="WWW.DYNDNS.ORG(CUSTOM)" <% nvram_match("ddns_server_x", "WWW.DYNDNS.ORG(CUSTOM)","selected"); %>>WWW.DYNDNS.ORG(CUSTOM)</option>
						<option value="WWW.DYNDNS.ORG(STATIC)" <% nvram_match("ddns_server_x", "WWW.DYNDNS.ORG(STATIC)","selected"); %>>WWW.DYNDNS.ORG(STATIC)</option>
						<option value="WWW.SELFHOST.DE" <% nvram_match("ddns_server_x", "WWW.SELFHOST.DE","selected"); %>>WWW.SELFHOST.DE</option>
						<option value="WWW.ZONEEDIT.COM" <% nvram_match("ddns_server_x", "WWW.ZONEEDIT.COM","selected"); %>>WWW.ZONEEDIT.COM</option>
						<option value="WWW.DNSOMATIC.COM" <% nvram_match("ddns_server_x", "WWW.DNSOMATIC.COM","selected"); %>>WWW.DNSOMATIC.COM</option>
						<option value="WWW.TUNNELBROKER.NET" <% nvram_match("ddns_server_x", "WWW.TUNNELBROKER.NET","selected"); %>>WWW.TUNNELBROKER.NET</option>
						<option value="WWW.NO-IP.COM" <% nvram_match("ddns_server_x", "WWW.NO-IP.COM","selected"); %>>WWW.NO-IP.COM</option>
						<option value="WWW.ORAY.COM" <% nvram_match("ddns_server_x", "WWW.ORAY.COM","selected"); %>>WWW.ORAY.COM(花生壳)</option>
						<option value="WWW.NAMECHEAP.COM" <% nvram_match("ddns_server_x", "WWW.NAMECHEAP.COM","selected"); %>>WWW.NAMECHEAP.COM</option>
						<option value="FREEDNS.AFRAID.ORG" <% nvram_match("ddns_server_x", "FREEDNS.AFRAID.ORG","selected"); %>>FREEDNS.AFRAID.ORG</option>
						<option value="CUSTOM" <% nvram_match("ddns_server_x", "CUSTOM","selected");  %>>Custom</option>
					</select>
					<input id="deregister_btn" class="button_gen" style="display: none; margin-left: 5px;" type="button" value="Deregister" onclick="showLoading();asuscomm_deregister();"/>
				<a id="link" href="javascript:openLink('x_DDNSServer')" style=" margin-left:5px; text-decoration: underline;"><#LANHostConfig_x_DDNSServer_linkname#></a>
				<a id="linkToHome" href="javascript:openLink('x_DDNSServer')" style=" margin-left:5px; text-decoration: underline;"><#ddns_home_link#></a>
				<div id="customnote" style="display:none;"><span>For the Custom DDNS you must manually create a ddns-start script that handles your custom notification.</span></div>
				<div id="need_custom_scripts" style="display:none;"><span>WARNING: you must enable both the JFFS2 partition and custom scripts support!<br>Click <a href="Advanced_System_Content.asp" style="text-decoration: underline;">HERE</a> to proceed.</span></div>
				</td>
			</tr>
			<tr id="ddns_hostname_tr">
				<th id="ddns_hostname_th"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,13);"><#LANHostConfig_x_DDNSHostNames_itemname#></a></th>
				<td>
					<div id="ddnsname_input" style="display:none;">
						<input type="text" maxlength="63" class="input_25_table" name="ddns_hostname_x" id="ddns_hostname_x" value="<% nvram_get("ddns_hostname_x"); %>" onKeyPress="return validator.isString(this, event)" autocorrect="off" autocapitalize="off">
					</div>
					<div id="asusddnsname_input" style="display:none;">
						<input type="text" maxlength="50" class="input_32_table" name="DDNSName" id="DDNSName" class="inputtext" onKeyPress="return validator.isString(this, event)" OnClick="cleandef();" autocorrect="off" autocapitalize="off">.asuscomm.com
						<div id="alert_block" style="color:#FFCC00; margin-left:5px; font-size:11px;display:none;">
								<span id="alert_str"></span>
						</div>
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
				<td><input type="password" maxlength="64" class="input_25_table" name="ddns_passwd_x" value="<% nvram_get("ddns_passwd_x"); %>" autocomplete="new-password" autocorrect="off" autocapitalize="off"></td>
			</tr>
			<tr id="wildcard_field">
				<th><#LANHostConfig_x_DDNSWildcard_itemname#></th>
				<td>
					<input type="radio" value="1" name="ddns_wildcard_x" onClick="return change_common_radio(this, 'LANHostConfig', 'ddns_wildcard_x', '1')" <% nvram_match("ddns_wildcard_x", "1", "checked"); %>><#checkbox_Yes#>
					<input type="radio" value="0" name="ddns_wildcard_x" onClick="return change_common_radio(this, 'LANHostConfig', 'ddns_wildcard_x', '0')" <% nvram_match("ddns_wildcard_x", "0", "checked"); %>><#checkbox_No#>
				</td>
			</tr>
			<tr style="display:none;">
				<th>Forced update interval (in days)</th>
				<td>
					<input type="text" maxlength="3" name="ddns_refresh_x" class="input_3_table" value="<% nvram_get("ddns_refresh_x"); %>" onKeyPress="return validator.isNumber(this,event)">
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
				<th><#LANHostConfig_x_DDNSStatus_itemname#></th>
				<td>
					<input type="hidden" maxlength="15" class="button_gen" size="12" name="" value="<% nvram_get("DDNSStatus"); %>">
					<input type="submit" maxlength="15" class="button_gen" onclick="showLoading();return onSubmitApply('ddnsclient');" size="12" name="LANHostConfig_x_DDNSStatus_button" value="<#LANHostConfig_x_DDNSStatus_buttonname#>" /></td>
			</tr>
		</table>

		<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
		<thead>
		<tr>
			<td colspan="2">Webui SSL Certificate</td>
		</tr>
		</thead>
			<tr id="https_cert" style="display:none;">
				<th><#DDNS_https_cert#></th>
				<td>
					<span id="le_crypt" style="color:#FFF;display:none;">
					<input type="radio" value="1" name="le_enable" onClick="change_cert_method(this.value);" <% nvram_match("le_enable", "1", "checked"); %>><#DDNS_https_cert_LetsEncrypt#>
					</span>
					<input type="radio" value="2" name="le_enable" onClick="change_cert_method(this.value);" <% nvram_match("le_enable", "2", "checked"); %>><#DDNS_https_cert_Import#>
					<span id="self_signed" style="color:#FFF;">
					<input type="radio" value="0" name="le_enable" onClick="change_cert_method(this.value);" <% nvram_match("le_enable", "0", "checked"); %>><#wl_securitylevel_0#>
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
			<tr id="cert_gen" style="display:none;">
				<th>Generate a new certificate</th>
				<td>
					<input type="radio" name="https_crt_gen" class="input" value="1" <% nvram_match_x("", "https_crt_gen", "1", "checked"); %>><#checkbox_Yes#>
					<input type="radio" name="https_crt_gen" class="input" value="0" <% nvram_match_x("", "https_crt_gen", "0", "checked"); %>><#checkbox_No#>
				</td>
			</tr>
			<tr id="cert_san" style="display:none;">
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(50,22)">Additional Certificate SANs</a></th>
				<td>
					<input type="text" name="https_crt_cn" value="<% nvram_get("https_crt_cn"); %>" autocomplete="off" class="input_32_table" maxlength="64" autocorrect="off" autocapitalize="off">
				</td>
			</tr>

			<tr id="cert_details" style="display:none;">
				<th><#vpn_openvpn_KC_SA#></th>
				<td>
					<div style="display:table-row;white-space: nowrap;">
						<div style="display:table-cell;white-space: nowrap;"><#Status_Str#> :</div>
						<div id="cert_status" style="display:table-cell; padding-left:10px;"></div>
					</div>
					<div style="display:table-row;white-space: nowrap;">
						<div style="display:table-cell;"><#vpn_openvpn_KC_to#> :</div>
						<div id="issueTo" style="display:table-cell; padding-left:10px;"></div>
					</div>
					<div style="display:table-row;">
						<div style="display:table-cell;">SAN :</div>
						<div id="SAN" style="display:table-cell; padding-left:10px;"></div>
					</div>
					<div style="display:table-row;white-space: nowrap;">
						<div style="display:table-cell;"><#vpn_openvpn_KC_by#> :</div>
						<div id="issueBy" style="display:table-cell; padding-left:10px;"></div>
					</div>
					<div style="display:table-row;white-space: nowrap;">
						<div style="display:table-cell;"><#vpn_openvpn_KC_expire#> :</div>
						<div id="expireOn" style="display:table-cell; padding-left:10px;"></div>
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
	<div class="formfontdesc" style="margin-left: 15px;"><#DDNS_https_cert_Import_desc#></div>
	<div class="Upload_item">
		<div style="display:table-cell; width: 45%;"><#DDNS_https_cert_PrivateKey#> :</div>
		<div style="display:table-cell;"><input type="file" name="file_key" class="input Upload_file"></div>
	</div>
	<div class="Upload_item">
		<div style="display:table-cell; width: 45%;"><#DDNS_ssl_cert#> :</div>
		<div style="display:table-cell;"><input type="file" name="file_cert" class="input Upload_file"></div>
	</div>
	<div style="color: #FC0; margin-left: 15px; margin-top: 20px">* <#DDNS_https_cert_PrivateKey_note#></div>
	<div align="center" style="margin-top:30px; padding-bottom:15px;">
		<div style="display:table-cell;"><input class="button_gen" type="button" onclick="hide_upload_window();" id="cancelBtn" value="<#CTL_Cancel#>"></div>
		<div style="display:table-cell; padding-left: 5px;"><input class="button_gen" type="button" onclick="upload_cert_key();" value="<#CTL_ok#>"></div>
	</div>
</div>
</form>
</body>
</html>
