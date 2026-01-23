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
<script type="text/javascript" src="/js/asus_policy.js?v=5"></script>
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
var link_internet = httpApi.nvramGet(["link_internet"], true).link_internet;

function initial(){
    var url1 = "";
    var url2 = "";
	show_menu();

	url1 = "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Policy&lang="+ui_lang+"&kw=tm_ppp&num=";
	$("#tm_eula_url").attr("href",url1);    // #TM_privacy_policy#
	url2 = "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Policy&lang="+ui_lang+"&kw=tm_ka&num=";
	$("#tm_disclosure_url").attr("href",url2);  // #TM_data_collection#

    PolicyStatus()
        .then(data => {
            if(isSupport("ai_support")) {
                if (data.AIBOARD_EULA > 0) {
                    document.getElementById("aiboard-eula").style.display = "";
                } else {
                    document.getElementById("aiboard-eula").style.display = "none";
                }
            }

            if (data.PP >= 1) {
                //Withdraw PP
                $("#pp").find('.btn_subusage').attr({
                    'onclick': `show_policy_withdraw('PP')`,
                    'value': `<#withdraw_str#>`
                });
                $("#pp").find('.eula_withdraw_content').append($('<div>').html(`<div><#ASUS_eula_withdraw_content1#> <#ASUS_eula_withdraw_content2#></div>`));
                $("#pp").find('.eula_withdraw_content').append($('<div>').html(`<a style="cursor: pointer; color: #006ce1; text-decoration: underline;" onclick="show_policy('PP')"><#ASUS_PP_Title#></a>`));
                $("#pp").find('.eula_withdraw_content').append($('<div>').html(`<div style="margin-top: 1em;"><#ASUS_eula_withdraw_content3#></div>`));
            } else {
                //Read PP
                $("#pp").find('.eula_withdraw_content').append($('<div>').html(`<div><#ASUS_PP_Info#></div>`));
                $("#pp").find('.btn_subusage').attr({'onclick': `show_policy('PP')`, 'value': `<#Reading#>`});
            }

            if (data.TM == "1") {
                document.getElementById("tm_eula").style.display = "";
                let tm_eula_support_str = `<#TM_eula_new_withdraw0#>`;
                let tm_eula_support = ``;
                if (based_modelid == "GT-BE19000AI" || based_modelid == "GT-BE96_AI") {
                    tm_eula_support_str = tm_eula_support_str.replace('%@', `<#AiProtection_two-way_IPS#>`);
                }
                document.getElementById("tm_eula_withdraw_support").innerHTML = tm_eula_support_str;
            } else {
                document.getElementById("tm_eula").style.display = "none";
            }
        });

	setTimeout(update_link_status, 1000);
}

function update_link_status(){
	link_internet = httpApi.nvramGet(["link_internet"], true).link_internet;
}

function hide_withdraw_sec(eula_type){
	var eula_id = '#' + eula_type + '_eula';
	$(eula_id).css("display", "none");
}

function show_policy_withdraw(policy_type){
    const policyModal = new PolicyWithdrawModalComponent({
        policy: policy_type,
		knowRiskCallback: function(){
			location.reload();
		}
    });
    policyModal.show();
}

function show_policy(policy_type) {
    const policyStatus = PolicyStatus()
		.then(data => {
			if (policy_type == 'PP') {
				const policyModal = new PolicyModalComponent({
					policy: policy_type,
					securityUpdate: true,
					websUpdate: true,
					policyStatus: data,
					agreeCallback: function () {
						location.reload();
					},
					knowRiskCallback: function () {
						location.reload();
					}
				});
				policyModal.show();
			} else if(policy_type == 'AIBOARD_EULA'){
                const policyModal = new PolicyModalComponent({
                    policy: policy_type,
                    policyStatus: data,
                    modalSize: "modal-lg",
                    agreeBtnText: "<#Acknowledge#>",
                    disagreeBtnText: "<#CTL_Decline#>",
                });
                policyModal.show();

            } else {
				const policyModal = new PolicyModalComponent({
					policy: policy_type,
					policyStatus: data,
				});
				policyModal.show();
			}
		});
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
										<div id="tm_eula_withdraw_support" class="eula_withdraw_title"><#TM_eula_withdraw0#></div>
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

                                    <div id="aiboard-eula" class="eula_withdraw" style="display:none;">
										<div class="eula_withdraw_title"><#ASUS_Aiboard_EULA_Title#></div>
										<div class="eula_withdraw_content">
											<div><#ASUS_Aiboard_EULA_Info#></div>
										</div>
										<div style="text-align: center;">
											<div style="margin: 0px auto 10px;display: flex;justify-content: center;">
												<input class="btn_subusage button_gen" onclick="show_policy('AIBOARD_EULA');" type="button" value="<#Reading#>"/>
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
