<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title></title>
<link href="/NM_style.css" rel="stylesheet" type="text/css" />
<link href="/form_style.css" rel="stylesheet" type="text/css" />
<link href="/js/table/table.css" rel="stylesheet" type="text/css" >
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/js/table/table.js"></script>
<script type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>
<script>
if(parent.location.pathname.search("index") === -1) top.location.href = "../"+'<% networkmap_page(); %>';

function initial(){
	if((parent.sw_mode == 2 || parent.sw_mode == 4) && '<% nvram_get("wlc_band"); %>' == '<% nvram_get("wl_unit"); %>')
		document.form.wl_subunit.value = 1;
	else
		document.form.wl_subunit.value = -1;

	if(parent.lyra_hide_support){
		document.getElementById("t0").style.display = "";
		document.getElementById("span0").innerHTML = "<#tm_wireless#>";
	}
	else{
		if(parent.band5g_support){
			document.getElementById("t0").style.display = "";
			document.getElementById("t1").style.display = "";

			if(parent.wl_info.band5g_2_support)
				tab_reset(0);

			if(parent.wl_info.band60g_support)
				tab_reset(0);
			
			// disallow to use the other band as a wireless AP
			if(parent.sw_mode == 4 && !localAP_support){
				for(var x=0; x<parent.wl_info.wl_if_total;x++){
					if(x != '<% nvram_get("wlc_band"); %>')
						document.getElementById('t'+parseInt(x)).style.display = 'none';
				}
			}
		}
		else{
			document.getElementById("t0").style.display = "";
		}
	}

	if(parent.wlc_express == '1' && parent.sw_mode == '2'){
		document.getElementById("t0").style.display = "none";
	}
	else if(parent.wlc_express == '2' && parent.sw_mode == '2'){
		document.getElementById("t1").style.display = "none";
	}

	var table_height = document.getElementById("rt_table").clientHeight;
	if(table_height != "0" || table_height != "")
		set_NM_height(table_height);
	else {
		document.body.style.overflow = "hidden";
		var errorCount = 0;
		var readyStateCheckInterval = setInterval(function() {
			table_height = document.getElementById("rt_table").clientHeight;
			if (table_height != "0" || table_height != "") {
				clearInterval(readyStateCheckInterval);
				set_NM_height(table_height);
			}
			else {
				if(errorCount > 5) {
					clearInterval(readyStateCheckInterval);
					table_height = parent.document.getElementById("NM_table").style.height;
					set_NM_height(table_height);
				}
				errorCount++;
			}
		}, 10);
	}
}

function tabclickhandler(wl_unit){
	if(wl_unit == "status"){
		location.href = "router_status.asp";
	}
	else if(wl_unit == "compatibility"){
		location.href = "compatibility.asp";
	}
	else{
		if((parent.sw_mode == 2 || parent.sw_mode == 4) && '<% nvram_get("wlc_band"); %>' == wl_unit)
			document.form.wl_subunit.value = 1;
		else
			document.form.wl_subunit.value = -1;

		if(parent.wlc_express != '0' && parent.wlc_express != '')
			document.form.wl_subunit.value = 1;

		document.form.wl_unit.value = wl_unit;
		document.form.current_page.value = "device-map/router.asp?time=" + Math.round(new Date().getTime()/1000);
		FormActions("/apply.cgi", "change_wl_unit", "", "");
		document.form.target = "hidden_frame";
		document.form.submit();
	}
}

function tab_reset(v){
	var tab_array1 = document.getElementsByClassName("tab_NW");
	var tab_array2 = document.getElementsByClassName("tabclick_NW");
	var tab_width = Math.floor(270/(parent.wl_info.wl_if_total+1));

	if(Bcmwifi_support && band5g_11ax_support){
		tab_width = "60";
	}

	var i = 0;
	while(i < tab_array1.length){
		tab_array1[i].style.width=tab_width+'px';
		tab_array1[i].style.display = "";
		i++;
	}
	
	if(typeof tab_array2[0] != "undefined"){	
		if (Bcmwifi_support && band5g_11ax_support) {
			tab_array2[0].style.width = '93px';
		}
		else{
			tab_array2[0].style.width = tab_width + 'px';
		}
		
		tab_array2[0].style.display = "";
	}
	
	if(v == 0){
		document.getElementById("span0").innerHTML = "2.4GHz";
		if(parent.wl_info.band5g_2_support){
			document.getElementById("span1").innerHTML = "5GHz-1";
			document.getElementById("span2").innerHTML = "5GHz-2";
		}else{
			document.getElementById("span1").innerHTML = "5GHz";
			document.getElementById("t2").style.display = "none";
		}

		if(!parent.wl_info.band60g_support){
			document.getElementById("t3").style.display = "none";
		}		
	}else if(v == 1){	//Smart Connect
		if(based_modelid == "RT-AC5300" || based_modelid == "RT-AC3200" || based_modelid == "GT-AC5300" || based_modelid == "GT-AX11000" || based_modelid == "RT-AX92U") {
			if(isSupport("triband") && dwb_info.mode) {
				document.getElementById("span0").innerHTML = "2.4GHz and 5GHz";
			}
			else
				document.getElementById("span0").innerHTML = "2.4GHz, 5GHz-1 and 5GHz-2";
		}
		else if(based_modelid == "RT-AC88U" || based_modelid == "RT-AX88U" || based_modelid == "RT-AC86U" || based_modelid == "AC2900" || based_modelid == "RT-AC3100" || based_modelid == "BLUECAVE")
			document.getElementById("span0").innerHTML = "2.4GHz and 5GHz";
		
		document.getElementById("t1").style.display = "none";
		document.getElementById("t2").style.display = "none";				
		document.getElementById("t3").style.display = "none";
		document.getElementById("t0").style.width = (tab_width*parent.wl_info.wl_if_total+10) +'px';
	}
	else if(v == 2){ //5GHz Smart Connect
		document.getElementById("span0").innerHTML = "2.4GHz";
		document.getElementById("span1").innerHTML = "5GHz-1 and 5GHz-2";
		document.getElementById("t3").style.display = "none";
		document.getElementById("t2").style.display = "none";	
		document.getElementById("t1").style.width = "143px";
		document.getElementById("span1").style.padding = "5px 4px 5px 7px";
	}
}

function applyRule(){
	document.form.submit();
}
</script>
</head>
<body class="statusbody" onload="initial();">
<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="form" action="/start_apply2.htm">
<input type="hidden" name="current_page" value="device-map/compatibility.asp">
<input type="hidden" name="next_page" value="device-map/compatibility.asp">
<input type="hidden" name="action_mode" value="apply_new">
<input type="hidden" name="action_script" value="restart_wireless">
<input type="hidden" name="action_wait" value="10">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="wl_unit" value="1">
<input type="hidden" name="wl_subunit" value="-1">
<input type="hidden" name="wl_bw_160" value='<% nvram_get("wl_bw_160"); %>'>
<input type="hidden" name="acs_dfs" value='<% nvram_get("acs_dfs"); %>'>
<input type="hidden" name="wl0_he_features" value='<% nvram_get("wl0_he_features"); %>'>
<input type="hidden" name="wl1_he_features" value='<% nvram_get("wl1_he_features"); %>'>
<input type="hidden" name="wl2_he_features" value='<% nvram_get("wl2_he_features"); %>'>
<table border="0" cellpadding="0" cellspacing="0" id="rt_table">
<tr>
	<td>		
		<table width="100px" border="0" align="left" style="margin-left:8px;" cellpadding="0" cellspacing="0">
			<td>
				<div id="t0" class="tab_NW" align="center" style="font-weight: bolder;display:none; margin-right:2px;" onclick="tabclickhandler(0)">
					<span id="span0" style="cursor:pointer;font-weight: bolder;">2.4GHz</span>
				</div>
			</td>
			<td>
				<div id="t1" class="tab_NW" align="center" style="font-weight: bolder;display:none; margin-right:2px;" onclick="tabclickhandler(1)">
					<span id="span1" style="cursor:pointer;font-weight: bolder;">5GHz</span>
				</div>
			</td>
			<td>
				<div id="t2" class="tab_NW" align="center" style="font-weight: bolder;display:none; margin-right:2px;" onclick="tabclickhandler(2)">
					<span id="span2" style="cursor:pointer;font-weight: bolder;">5GHz-2</span>
				</div>
			</td>
			<td>
				<div id="t3" class="tab_NW" align="center" style="font-weight: bolder;display:none; margin-right:2px;" onclick="tabclickhandler(3)">
					<span id="span3" style="cursor:pointer;font-weight: bolder;">60GHz</span>
				</div>
			</td>
			<td>
				<div id="t_compatibility" class="tabclick_NW" align="center" style="font-weight: bolder; margin-right:2px;" onclick="tabclickhandler('compatibility')">
					<span style="cursor:pointer;font-weight: bolder;">Compatibility</span>
				</div>
			</td>
			<td>
				<div id="t_status" class="tab_NW" align="center" style="font-weight: bolder; margin-right:2px;" onclick="tabclickhandler('status')">
					<span id="span_status" style="cursor:pointer;font-weight: bolder;"><#Status_Str#></span>
				</div>
			</td>
		</table>
	</td>
</tr>

<tr>
	<td> 
		<div>
			<table width="96%" border="1" align="center" cellpadding="4" cellspacing="0" class="table1px" style="margin: 0px 8px;">	
			<tr>
				<td colspan="3">			
					<div style="margin:10px;">
						<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="table1px">
							<tr>
								<th style="word-break: break-word;line-height: 16px;">HE frame support</th>
								<td style="width: 170px;">
									<div align="center" class="left" style="width:94px; float:right; cursor:pointer;" id="he_enable"></div>
									<div class="iphone_switch_container" style="height:32px; width:74px; position: relative; overflow: hidden">
										<script type="text/javascript">
											var _flag = ('<% nvram_get("wl0_he_features"); %>' == 3) ? true : false;
											$('#he_enable').iphoneSwitch(_flag,
												function(){
													document.form.wl0_he_features.value = "3";
													document.form.wl1_he_features.value = "3";
													document.form.wl2_he_features.value = "3";
												},
												function(){
													document.form.wl_bw_160.value = 0;
													document.form.acs_dfs.value = 0;
													document.form.wl0_he_features.value = "0";
													document.form.wl1_he_features.value = "0";
													document.form.wl2_he_features.value = "0";
												}
											);
										</script>
									</div>
								</td>
							</tr>
						</table>
					</div>
					<div style="margin-top:5px; *margin-top:-10px;" class="line_horizontal"></div>
					<div style="margin:10px;">
						<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="table1px">
							<tr>
								<th style="word-break: break-word;line-height: 16px;">DFS band</th>
								<td style="width: 170px;">
									<div align="center" class="left" style="width:94px; float:right; cursor:pointer;" id="dfs_enable"></div>
									<div class="iphone_switch_container" style="height:32px; width:74px; position: relative; overflow: hidden">
										<script type="text/javascript">
											$('#dfs_enable').iphoneSwitch('<% nvram_get("acs_dfs"); %>',
												function(){
													document.form.acs_dfs.value = 1;					
												},
												function(){
													document.form.acs_dfs.value = 0;													
												}
											);
										</script>
									</div>
								</td>
							</tr>
						</table>
					</div>

					<div style="margin-top:5px; *margin-top:-10px;" class="line_horizontal"></div>
					<div style="margin:10px;">
						<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="table1px">
							<tr>
								<th style="word-break: break-word;line-height: 16px;">Channel Bandwidth 160 MHz</th>
								<td style="width: 170px;">
									<div align="center" class="left" style="width:94px; float:right; cursor:pointer;" id="bw_enable"></div>
									<div class="iphone_switch_container" style="height:32px; width:74px; position: relative; overflow: hidden">
										<script type="text/javascript">
											$('#bw_enable').iphoneSwitch('<% nvram_get("wl1_bw_160"); %>',
												function(){
													document.form.wl_bw_160.value = 1;
												},
												function(){
													document.form.wl_bw_160.value = 0;
												}
											);
										</script>
									</div>
								</td>
							</tr>
						</table>
					</div>
					<div style="text-align: center;">
						<input id="applySecurity" type="button" class="button_gen" value="<#CTL_apply#>" onclick="applyRule();" >
					</div>
					<div style="margin-top:5px; *margin-top:-10px;" class="line_horizontal"></div>
					<div style="width: 280px;padding: 10px 5px;">
						<div style="padding: 5px 0;line-height: 16px;">Before enabling HE frame support, DFS band, and Channel Bandwidth 160 MHz, please make sure your connected clients support
						these functions correspondingly.</div>
						<div style="padding: 5px 0;line-height: 16px;">In additiotn, please make sure your WLAN card driver is updated to the latest version. More information is provided in
						the following links.</div>
						<div style="padding: 5px 0;line-height: 16px;word-break: break-word;">Intel WLAN Driver Download: <a href="https://downloadcenter.intel.com/product/59485/Wireless-Networking" target="_blank" style="text-decoration: underline;font-weight:bold">Link</a></div>
						<div style="padding: 5px 0;line-height: 16px;">How to Update WLAN Card Driver Manually? <a href="https://www.asus.com/support/FAQ/1037422/" target="_blank" style="text-decoration: underline;font-weight:bold">Link</a></div>
					</div>
				</td>
			</tr>
			</table>
		</div>
	</td>
</tr>
</table>			
</form>
</body>
</html>
