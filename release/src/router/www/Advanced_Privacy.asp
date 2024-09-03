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
<title><#Web_Title#> - <#menu_privacy#></title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<script type="text/javascript" src="/js/jquery.js"></script>
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/form.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script type="text/javascript" src="/js/asus_policy.js"></script>
<style>

.eula_withdraw{
	border: 1px solid #A9A9A9;
	font-family: Arial, Helvetica, sans-serif;
	color: #FFF;
	margin: auto 10px 20px 10px;
}

.eula_withdraw_title{
	font-weight: bold;
	font-size: 14px;
	margin: 5px 10px auto 10px;
}

.eula_withdraw_content{
	margin-left: 10px;
	margin-right: 10px;
}
</style>
<script>

let eula_status = {
	"EULA": "0",
	"PP": "0",
	"TM_EULA": "0"
}

eula_status = httpApi.nvramGet(["ASUS_NEW_EULA", "TM_EULA"], true);
eula_status.EULA = eula_status.ASUS_NEW_EULA;
httpApi.privateEula.get().then(data=>{
    eula_status.PP = data.ASUS_PP;
});

var link_internet = httpApi.nvramGet(["link_internet"], true).link_internet;

var services_array = {
	"without_alexa_ifttt" : "Account Binding, DDNS",
	"without_ifttt" : "Account Binding, DDNS, Google Assistant, Alexa™",
	"within_alexa_ifttt" : "For Account Binding, DDNS, Google Assistant, Alexa™, IFTTT™"
}
var services_show = "<#ASUS_eula_withdraw0#>";
function initial(){
    var url1 = "";
    var url2 = "";
	show_menu();

	url1 = "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Policy&lang="+ui_lang+"&kw=tm_ppp&num=";
	$("#tm_eula_url").attr("href",url1);    // #TM_privacy_policy#
	url2 = "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Policy&lang="+ui_lang+"&kw=tm_ka&num=";
	$("#tm_disclosure_url").attr("href",url2);  // #TM_data_collection#

	// if(eula_status.EULA == "1"){
	// 	if(!alexa_support && !ifttt_support){
	// 		services_show = services_show.replace('%1$@', services_array.without_alexa_ifttt);
	// 		document.getElementById("asus_eula_title").innerHTML = services_show;
	// 		document.getElementById("alexa_ifttt").style.display = "none";
	// 	}
	// 	else if(alexa_support && !ifttt_support){
	// 		services_show = services_show.replace('%1$@', services_array.without_ifttt);
	// 		document.getElementById("asus_eula_title").innerHTML = services_show;
	// 		document.getElementById("alexa_ifttt").innerHTML = "<#ASUS_eula_withdraw_desc2_Alexa#>";
	// 	}
	// 	else{
	// 		services_show = services_show.replace('%1$@', services_array.within_alexa_ifttt);
	// 		document.getElementById("asus_eula_title").innerHTML = services_show;
	// 		document.getElementById("alexa_ifttt").innerHTML = "<#ASUS_eula_withdraw_desc2_AlexaIFTTT#>";
	// 	}
	// }

	if(eula_status.TM_EULA == "1"){
		document.getElementById("tm_eula").style.display = "";
	}
	else{
		document.getElementById("tm_eula").style.display = "none";
	}

	if(eula_status.PP >= "1"){
        $("#pp").find('.btn_subusage').attr({'onclick':`show_policy_withdraw('PP')`,'value':`<#withdraw_str#>`});
        $("#pp").find('.eula_withdraw_content').append($('<div>').html(`<div><#ASUS_eula_withdraw_content1#> <#ASUS_eula_withdraw_content2#></div>`));
        $("#pp").find('.eula_withdraw_content').append($('<div>').html(`<a style="cursor: pointer; color: #006ce1; text-decoration: underline;" onclick="show_policy('PP')"><#ASUS_PP_Title#></a>`));
        $("#pp").find('.eula_withdraw_content').append($('<div>').html(`<div style="margin-top: 1em;"><#ASUS_eula_withdraw_content3#></div>`));
	} else {
        $("#pp").find('.eula_withdraw_content').append($('<div>').html(`<div><#ASUS_PP_Info#></div>`));
		$("#pp").find('.btn_subusage').attr({'onclick':`show_policy('PP')`,'value':`<#Reading#>`});
    }

	setTimeout(update_link_status, 1000);
}

function update_link_status(){
	link_internet = httpApi.nvramGet(["link_internet"], true).link_internet;
}

function hide_withdraw_sec(eula_type){
	var eula_id = '#' + eula_type + '_eula';
	$(eula_id).css("display", "none");
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

	if( return_status != "" || timeout ){
		$.ajax({
			url: "/set_ASUS_EULA.cgi",
			data:{
				"ASUS_EULA":"0"
			},
			success: function( response ) {
				hide_withdraw_sec('asus');
			}
		});
	}
}

function show_policy_withdraw(policy_type){
    const policyModal = new PolicyWithdrawModalComponent({
        policy: policy_type,
        submit_reload: 1
    });
    policyModal.show();
}

function show_policy(policy_type) {
    const applyRule = () => {
        httpApi.privateEula.set("1", function () {
            httpApi.securityUpdate.set(1);
            if (policy_status.ASUS_PP_AutoWebUpgradeDisable !== "1") {
                httpApi.nvramSet({
                    "webs_update_enable": 1,
                    "action_mode": "apply",
                    "rc_service": "saveNvram"
                },()=>{},false);
            }
        })
    }

    if (policy_type == 'PP') {
        const policyModal = new PolicyModalComponent({
            policy: policy_type,
            agreeCallback: applyRule,
            submit_reload: 1
        });
        policyModal.show();
    } else {
        const policyModal = new PolicyModalComponent({
            policy: policy_type,
            submit_reload: 1
        });
        policyModal.show();
    }
}

function withdraw_eula(eula_type){

	switch(eula_type){

		case "tm":
			if(confirm("<#withdraw_confirm#>")){
				document.getElementById('tm_withdraw_btn').style.display = "none";
				document.getElementById('tm_loadingicon').style.display = "";
				$.ajax({
					url: "/set_TM_EULA.cgi",
					data:{
						"TM_EULA":"0"
					},

					success: function( response ) {
						hide_withdraw_sec('tm');
					}
				});
			}

			break;
	}
}

</script>
</head>
<body onload="initial();" onunLoad="return unload_body();" class="bg">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="Advanced_Privacy.asp">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<table class="content" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td width="17">&nbsp;</td>
	<!--=====Beginning of Main Menu=====-->
		<td valign="top" width="202">
			<div id="mainMenu"></div>
			<div id="subMenu"></div>
		</td>
		<td valign="top">
			<div id="tabMenu" class="submenuBlock"></div>
		<!--===================================Beginning of Main Content===========================================-->
			<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
				<tr>
					<td valign="top" >
						<table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
							<tbody>
							<tr>
								<td bgcolor="#4D595D" valign="top">
									<div>&nbsp;</div>
									<div id="formfonttitle" class="formfonttitle"><#menu5_6#> - <#ASUS_Notice_Privacy#></div>
									<div style="margin:10px 0 10px 5px;" class="splitLine"></div>

									<div id="eula" class="eula_withdraw">
										<div class="eula_withdraw_title"><#ASUS_EULA_Title#> (EULA)</div>
										<div class="eula_withdraw_content">
											<div><#ASUS_EULA_Info#></div>
										</div>
										<div style="text-align: center;">
											<div style="margin: 0px auto 10px;display: flex;justify-content: center;">
												<input class="btn_subusage button_gen" onclick="show_policy('EULA');" type="button" value="<#Reading#>"/>
											</div>
										</div>
									</div>

									<div id="pp" class="eula_withdraw">
										<div class="eula_withdraw_title"><#ASUS_PP_Title#></div>
										<div class="eula_withdraw_content">
										</div>
										<div style="text-align: center;">
											<div style="margin: 0px auto 10px;display: flex;justify-content: center;">
												<input class="btn_subusage button_gen" onclick="show_policy('PP');" type="button" value="<#Reading#>"/>
											</div>
										</div>
									</div>

									<div id="tm_eula" class="eula_withdraw" style="display:none;">
										<div class="eula_withdraw_title"><#TM_eula_withdraw0#></div>
										<div class="eula_withdraw_content">
											<div><#TM_eula_withdraw_desc1#></div>
											<div style="margin-top:5px;"><#TM_eula_withdraw_desc2#>
											<div><#TM_privacy_policy#></div>
											<div><#TM_data_collection#></div>
											</div>
										</div>
										<div style="text-align:center;">
											<div style="margin: 0px auto 10px;display: flex;justify-content: center;">
												<input class="btn_subusage button_gen" id="tm_withdraw_btn" onclick="withdraw_eula('tm');" type="button" value="<#withdraw_str#>"/>
												<img id="tm_loadingicon" style="display:none;" src="/images/InternetScan.gif">
											</div>
										</div>
									</div>
								</td>
							</tr>
							</tbody>
						</table>
					</td>
				</tr>
			</table>
		<!--===================================Ending of Main Content===========================================-->
		</td>
		<td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>
</form>
<div id="footer"></div>
</body>
</html>
