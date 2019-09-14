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
<title><#Web_Title#> - <#menu5_2_1#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="other.css">
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/validator.js"></script>
<script>
var nt_current_page = 1;
var nt_each_page_num = 6;
var sort = 0;	// 0:date new, 1:date old, 3:priority high 4:priority low

function initial(){
	show_menu();
	document.getElementById("nt_action_webapp").checked = ('<% nvram_get("nc_web_app_enable"); %>' == "1");
	document.getElementById("nt_action_email").checked = ('<% nvram_get("nc_mail_enable"); %>' == "1");
	show_nt_event_list(nt_current_page, sort);
}

function applyRule(){

}

function update_setting_type(){
		setTimeout(function(){notification.updateNTDB_Status();}, 500);
		setTimeout(function(){notification.update_nc_setting_conf();}, 500);
		setTimeout(function(){update_nt_action();}, 500);
		setTimeout(function(){show_nt_event_list(nt_current_page, sort);}, 1000);
		document.getElementById("change_priority_sort").className = "noti_event_down";
		document.getElementById("change_data_sort").className = "noti_event_down";
}

function update_nt_action(){
	require(['/require/modules/makeRequest.js'], function(makeRequest) {
		makeRequest.start('/appGet.cgi?hook=nvram_get(nc_web_app_enable)%3bnvram_get(nc_mail_enable)', update_nt_action_check, update_nt_action);
	});
}

function update_nt_action_check(xhr){
	if(xhr.responseText.search("Main_Login.asp") !== -1) top.location.href = "/";
	
	var response = JSON.parse(xhr.responseText);
	document.form.nc_web_app_enable.value = response.nc_web_app_enable;
	document.form.nc_mail_enable.value = response.nc_mail_enable;
	document.getElementById("nt_action_webapp").checked = (response.nc_web_app_enable == "1");
	document.getElementById("nt_action_email").checked = (response.nc_mail_enable == "1");
}

function show_nt_event_list(nt_page, sort){
	
	nt_current_page = nt_page;

	var first_line = false;
	var nt_current_page_start = (nt_current_page - 1)*6;
	var nt_event_tmp = NTDB_info.get_nt_db;

	var txt='';

	if(sort == 2 || sort == 3){
		for(var j=0; j<nt_event_tmp.length; j++){
			nt_event_tmp[j].event_type = notification.find_nc_setting_type(nt_event_tmp[j].event_id.trim());
			if(nt_event_tmp[j].event_type == notification.TYPE_OF_IMPORTANT) nt_event_tmp[j].event_type = notification.TYPE_OF_TOTAL;
		}
	}

	if(nt_event_tmp.length == 0){
		txt += '<div style="width:100%;display:table;height:160px;">';
		txt += '<div class="noti_mail"></div>';
		txt += '<div class="notiDiv_nonews_word">No News!</div>';
		txt += '</div>';
		setTimeout("show_nt_event_list(nt_current_page);", 100);
	}else{
		switch(sort){
			case 0:
				nt_event_tmp.sort(sort_by('tstamp', false, parseInt));
				break;
			case 1:
				nt_event_tmp.sort(sort_by('tstamp', true, parseInt));
				break;
			case 2:
				nt_event_tmp.sort(sort_by('event_type', false, parseInt));
				break;
			case 3:
				nt_event_tmp.sort(sort_by('event_type', true, parseInt));
				break;
			default:
				nt_event_tmp.sort(sort_by('tstamp', true, parseInt));
				break;
		}		
	}
	txt += '<div>';

	for(var i=nt_current_page_start; i<nt_event_tmp.length; i++){

		if(i > nt_current_page_start + nt_each_page_num -1)	break;	//show select page in the end

		if(first_line == true)
			txt += '<div style="padding-top:6px"><div style="background-color:#AAAAAA;height:1px;width:95%;margin-left:7px;"></div></div>';
		else{
			txt += '<div style="padding-top:0px"></div>';
			first_line = true;
		}

		var ntdb_obj = nt_event_tmp[i];
		txt += '<div style="padding-bottom: 3px;">';
			txt += '<div style="display:table-cell;padding:6px;width: 84px;"><div>'+notification.noti_event_time_date(ntdb_obj.tstamp,"date")+'</div><div>'+notification.noti_event_time_date(ntdb_obj.tstamp,"clock")+'</div></div>';
		
  			txt += '<div style="display:table-cell;height:35px;width:115px;vertical-align:middle;" title="Change type">';
  				txt += '<div id="noti_event_type_'+i+'" style="cursor:pointer;margin:0px auto;height:25px;width:40px;padding-top:20px;" onclick="notification.noti_type_select(this.id)">';
	  			if(notification.find_nc_setting_type(ntdb_obj.event_id.trim()) == notification.TYPE_OF_IMPORTANT)
	  				txt += '<div id="noti_circle_t" class="noti_type_important_small">'
	  			else if(notification.find_nc_setting_type(ntdb_obj.event_id.trim()) == notification.TYPE_OF_TIPS) 	//tips
	  				txt += '<div id="noti_circle_t" class="noti_type_tips_small">'
	  			else if(notification.find_nc_setting_type(ntdb_obj.event_id.trim()) == notification.TYPE_OF_TURN_OFF) 	//turn off
	  				txt += '<div id="noti_circle_t" class="noti_type_turnoff_small">';
	  			txt += '</div>';
	  			//txt += '<div id="noti_event_type1_'+i+'" style="cursor:pointer;position:absolute;background-repeat:no-repeat;background-position:6px 6px;height:37px;width:38px;background-size: 26px 25px;';
	  			//txt += '"></div>';
	  			//txt += 'background-image: url(/images/New_ui/nt_center/nt_icon'+ NTDB_content[ntdb_obj.event_id.trim()].icon +'.svg);"></div>';
	  			txt += '</div>';
	  			txt += '<div id="noti_event_type_'+i+'_option" style="display:none;">';
	  			txt += '<div class="noti_type_content_triangle"></div>';
				txt += '<div class="noti_type_content_select_t">';
				//txt += '<div style="font-size:16px;color:#AAAAAA;text-align:center;padding-top:8px;">Notice Type</div>';
				//txt += '<div style="padding-top:2px"><div style="background-color:#333333;height:1px;width:95%;margin:0px auto;opacity:0.2;"></div></div>';
				txt += '<div style="height:30px;cursor: pointer;" onclick="notification.change_type('+ntdb_obj.event_id+',2,update_setting_type)"><div style="display:table-cell;"><div class="noti_type_select_important">';
				if(notification.find_nc_setting_type(ntdb_obj.event_id.trim()) == notification.TYPE_OF_IMPORTANT){
					txt += '<div class="noti_type_select_check"></div>';
				}
				txt += '</div></div><div class="noti_type_word" title="The notification related to system and safety will be defined as important, and you will receive them via mobile notification system once installing ASUS router app.">Important</div></div>';
				txt += '<div style="padding-bottom:3px;height:30px;cursor: pointer;" onclick="notification.change_type('+ntdb_obj.event_id+',3,update_setting_type)"><div style="display: table-cell;"><div class="noti_type_select_tips">';
				if(notification.find_nc_setting_type(ntdb_obj.event_id.trim()) == notification.TYPE_OF_TIPS){
					txt += '<div class="noti_type_select_check"></div>';
				}
				txt += '</div></div><div class="noti_type_word" title="General notification related to new function introduce or application suggestion."><#QIS_finish_Tips#></div></div>';
				txt += '<div style="padding-bottom:3px;height:30px;cursor: pointer;" onclick="notification.change_type('+ntdb_obj.event_id+',1,update_setting_type)"><div style="display: table-cell;"><div class="noti_type_select_turnoff">';
				if(notification.find_nc_setting_type(ntdb_obj.event_id.trim()) == notification.TYPE_OF_TURN_OFF){
					txt += '<div class="noti_type_select_check"></div>';
				}
				txt += '</div></div><div class="noti_type_word" title="Set notice as \“turn off\” to disable the event display on new notifications list.">Turn Off</div></div>';				
				txt += '</div>'
  			txt += '</div>';
  			txt += '</div>';

  			txt += '<div style="display:table-cell;padding-left:28px;width:400px;">';
  				txt += '<div class="notiDiv_content_item">'+NTDB_content[ntdb_obj.event_id.trim()].item+'</div>';
	  			txt += '<div>';

				txt += '<span class="notiDiv_content_desc">';
				if(ntdb_obj.event_id.trim() == "10010" || ntdb_obj.event_id.trim() == "10011"){
					if(clientList[ntdb_obj.msg.macaddr]){
					 	txt += clientList[ntdb_obj.msg.macaddr].name +' ( '+clientList[ntdb_obj.msg.macaddr].ip +' )';
					}else{
						txt += ntdb_obj.msg.macaddr +' ( offline )';
					}
					txt += NTDB_content[ntdb_obj.event_id.trim()].contents;
				}else if(ntdb_obj.event_id.trim() == "10012" || ntdb_obj.event_id.trim() == "10013"){
				 	txt += NTDB_content[ntdb_obj.event_id.trim()].contents;
					txt += ntdb_obj.msg.fw_ver;
				  	txt += NTDB_content[ntdb_obj.event_id.trim()].contents_1;
				}else if(ntdb_obj.event_id.trim() == "30002" || ntdb_obj.event_id.trim() == "30003"){
				  	txt += NTDB_content[ntdb_obj.event_id.trim()].contents;
				  	txt += ntdb_obj.msg.CName;
				  	txt += NTDB_content[ntdb_obj.event_id.trim()].contents_1;
				}else if(ntdb_obj.event_id.trim() == "20001"){
				  	txt += ntdb_obj.msg.IP;
				  	txt += NTDB_content[ntdb_obj.event_id.trim()].contents;
				  }else if(ntdb_obj.event_id.trim() == "30008"){
				  	txt += ntdb_obj.msg.CName;
				  	txt += NTDB_content[ntdb_obj.event_id.trim()].contents;
				}else{
				  	txt += NTDB_content[ntdb_obj.event_id.trim()].contents;
				}
				txt += '</span></div>';
  			txt += '</div>';

  			txt += '<div style="display:table-cell;padding-left: 45px;">';

			txt += '</div>';

		txt += '</div>';
		//break;
	}
	txt += '</div>';

	document.getElementById("nt_event_list_content").innerHTML = txt;

	show_Message_Page_Num();
}

function show_Message_Page_Num(){

	if(NTDB_info.get_nt_db.length == 0)
		setTimeout("show_Message_Page_Num();", 500);

	var max_page_num = Math.ceil(NTDB_info.get_nt_db.length/nt_each_page_num);

	var cur_max_page_num;
	var max_page_list_num = 9;

	if(max_page_num > max_page_list_num){
		if(nt_current_page > 5){
			if(nt_current_page > max_page_num-4)
				cur_max_page_num = max_page_num;
			else
				cur_max_page_num = nt_current_page+4;
		}
		else
			cur_max_page_num = max_page_list_num;
	}
	else
		cur_max_page_num = max_page_num;

	var forward_page_num = (nt_current_page != 1)?nt_current_page-1:nt_current_page;
	var last_page_num = (nt_current_page != cur_max_page_num)?nt_current_page+1:nt_current_page;

	var txt = '';
	txt += '<div class="noti_change_page">';
	txt += '<span id="noti_begin_page" class="noti_begin_end_page" onclick="show_nt_event_list(1)">1</span>';
	txt += '<span id="noti_change_page_forward" class="noti_change_page_forward" onclick="show_nt_event_list('+forward_page_num+')"></span>';

	if(max_page_num < max_page_list_num){
		for(var i=1; i<cur_max_page_num+1; i++){
			if( i == nt_current_page)
				txt += '<span class="noti_change_page_num">'+i+'</span>';
			else
				txt += '<span style="cursor: pointer;" onclick="show_nt_event_list('+i+')">'+i+'</span>';
		}		
	}else{
		if(nt_current_page < Math.ceil(max_page_list_num/2)+1){
			for(var i=1; i<cur_max_page_num+1; i++){
				if( i == nt_current_page)
					txt += '<span class="noti_change_page_num">'+i+'</span>';
				else
					txt += '<span style="cursor: pointer;" onclick="show_nt_event_list('+i+')">'+i+'</span>';
			}
		}else if(nt_current_page > Math.ceil(max_page_list_num/2) && nt_current_page < max_page_num-3){
			for(var i=nt_current_page-4; i<nt_current_page+5; i++){
				if( i == nt_current_page)
					txt += '<span class="noti_change_page_num">'+i+'</span>';
				else
					txt += '<span style="cursor: pointer;" onclick="show_nt_event_list('+i+')">'+i+'</span>';
			}
		}else if(nt_current_page > max_page_num-4){
			for(var i=max_page_num-8; i<max_page_num+1; i++){
				if( i == nt_current_page)
					txt += '<span class="noti_change_page_num">'+i+'</span>';
				else
					txt += '<span style="cursor: pointer;" onclick="show_nt_event_list('+i+')">'+i+'</span>';
			}			
		}
	}	
	txt += '<span id="noti_change_page_last" class="noti_change_page_last" onclick="show_nt_event_list('+last_page_num+')"></span>';
	txt += '<span id="noti_last_page" class="noti_begin_end_page" onclick="show_nt_event_list('+max_page_num+')">'+max_page_num+'</span>';
	txt += '</div>';
	document.getElementById("change_message_page_list").innerHTML = txt;

	if(nt_current_page == 1)
		document.getElementById("noti_change_page_forward").style.display = "none";

	if(nt_current_page < Math.ceil(max_page_list_num/2)+1)
		document.getElementById("noti_begin_page").style.display = "none";

	if(max_page_num == nt_current_page)
		document.getElementById("noti_change_page_last").style.display = "none";

	if(nt_current_page > (max_page_num - Math.floor(max_page_list_num/2)-1))
		document.getElementById("noti_last_page").style.display = "none";
}

function change_data_sort(obj){

	var data_sort;
	if(obj.className == "noti_event_down"){
		obj.className = "noti_event_up";
		data_sort = 0;
	}else{
		obj.className = "noti_event_down";
		data_sort = 1;
	}
	document.getElementById("change_priority_sort").className = "noti_event_down";
	show_nt_event_list(nt_current_page, data_sort);
	return;
}

function change_priority_sort(obj){

	var prio_sort;
	if(obj.className == "noti_event_down"){
		obj.className = "noti_event_up";
		prio_sort = 2;
	}else{
		obj.className = "noti_event_down";
		prio_sort = 3;
	}
	document.getElementById("change_data_sort").className = "noti_event_down";
	show_nt_event_list(nt_current_page, prio_sort);
	return;
}

function nt_action_click(flag){
	var nt_action;
	var nt_action_value;
	var postJson;

	if(flag == "webapp"){
		if(document.getElementById("nt_action_webapp").checked == document.form.nc_web_app_enable.value) return;
		if(document.getElementById("nt_action_webapp").checked == true)
			document.form.nc_web_app_enable.value = "1";
		else
			document.form.nc_web_app_enable.value = "0";

		nt_action = "nc_web_app_enable";
		nt_action_value = document.form.nc_web_app_enable.value;
	}else if(flag == "mail"){
		if(document.getElementById("nt_action_email").checked == document.form.nc_mail_enable.value) return;
		if(document.getElementById("nt_action_email").checked == true)
			document.form.nc_mail_enable.value = "1";
		else
			document.form.nc_mail_enable.value = "0";

		nt_action = "nc_mail_enable";
		nt_action_value = document.form.nc_mail_enable.value;
	}

	var nc_setting_conf_tmp = new Array();
	for(var i=0; i<notification.nc_setting_conf.length; i++){
		var nc_setting_array = notification.nc_setting_conf[i].split(">");
		if(nc_setting_array != ""){
			nc_setting_array[1] = 0;
			if(nc_setting_array[2] == notification.TYPE_OF_TURN_OFF){
				nc_setting_array[1] = notification.ACTION_NOTIFY_NONE;
			}else{
				if(document.form.nc_mail_enable.value == '1')
					nc_setting_array[1] += notification.ACTION_NOTIFY_EMAIL;
				if(document.form.nc_web_app_enable.value == '1'){
					nc_setting_array[1] += notification.ACTION_NOTIFY_WEBUI;
					nc_setting_array[1] += notification.ACTION_NOTIFY_APP;
				}
			}
		}
		nc_setting_conf_tmp.push(nc_setting_array.join('>'));			
	}

	notification.nc_setting_conf = nc_setting_conf_tmp;
	nc_setting_conf_tmp = nc_setting_conf_tmp.join('<');
	require(['/require/modules/makeRequest.js'], function(makeRequest) {
	makeRequest.post('/apply.cgi','nc_setting_conf='+nc_setting_conf_tmp+'&'+nt_action+'='+nt_action_value+'&rc_service=update_nc_setting_conf&action_mode=apply', update_setting_type, nt_action_click);
	});
}

</script>
</head>

<body onload="initial();" onunLoad="return unload_body();" class="bg">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_Notification_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="nc_web_app_enable" value="<% nvram_get("nc_web_app_enable"); %>">
<input type="hidden" name="nc_mail_enable" value="<% nvram_get("nc_mail_enable"); %>">

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
		<td align="left" valign="top">
  <table width="760px" border="0" cellpadding="5" cellspacing="0" class="FormTitle" id="FormTitle">
	<tbody>
	<tr>
	   <td bgcolor="#4D595D" valign="top">
		  <div>&nbsp;</div>
		  <div class="formfonttitle">Notification - Notification Setting</div>
      		<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
      	  	<div class="control_bg content_frame" style="width:auto;border solide: 1px #717171;border-radius:4px;height:120px;">
      	  	<div style="padding:6px;">
	      	  	<div>
	      	  		<div class="noti_bell_gray" style="display:table-cell"></div><div style="display:table-cell;font-size:14px;font-weight:bold;color:#FFFFFF;">Reminds</div>
	      	  	</div>
	      	  	<div style="font-size:12px;color:#C3D4D8;padding-left:25px">Please select the method you want to receive notifications when a system event occur.</div>
	      	  	<div>
	      	  		<div style="padding:20px 0px 0px 230px;display:table-cell">
	      	  		<span style="font-size:14px;color:#FFFFFF;"><input type="checkbox" style="width:14px; height:14px" name="nt_action_webapp" id="nt_action_webapp" onclick="nt_action_click('webapp')">Web/APP</span>
	      	  		<span style="font-size:14px;color:#FFFFFF;margin-left:70px;"><input type="checkbox" style="width:14px; height:14px" name="nt_action_email" id="nt_action_email" value="0" onclick="nt_action_click('mail')">Email</span>
	      	  		</div>
	      	  		<div style="display:table-cell;" class="noti_setting"></div>
	      	  	</div>
      	  	</div>
      	  	</div>
      	  	<!--div class="apply_gen" style="width:94%">
					<input type="button" id="applyButton" class="button_gen" value="Apply" onclick="applyRule();">
			</div-->
      	  	</div>

      	  	<div class="textarea_ssh_table content_frame" style="width:auto;border solide: 1px #717171;border-radius:4px;height:680px;margin-top:15px;">
      	  		<div style="padding:15px 6px;">
      	  			<div class="noti_message" style="display:table-cell;"></div><div style="font-size:14px;color:#FFFFFF;font-weight:bold;display:table-cell;padding-left:7px;">Event List</div>
      	  			<div style="display:"><div style="font-size:12px;color:#FFFFFF;font-weight:bold;display:table-cell;padding-left:7px;"><#diskUtility_time#></div><div id="change_data_sort" class="noti_event_down" style="display:table-cell;" onclick="change_data_sort(this);"></div><div style="display:table-cell;padding-left:58px;">Notice Type</div><div  id="change_priority_sort" class="noti_event_down" style="display:table-cell;" onclick="change_priority_sort(this)"></div><div style="display:table-cell;padding-left: 30px;">Content</div></div>
      	  			<div style="padding-top:6px"><div style="background-color:#AAAAAA;height:2px;width:95%;margin-left:7px;"></div></div>
      	  			<div id="nt_event_list_content"></div>
      	  			<div style="padding-top:6px;"><div style="background-color:#AAAAAA;height:1px;width:95%;margin-left:7px;"></div></div>
      	  			<div id="change_message_page_list" style="padding-top:10px;width:600px;margin:0px auto;text-align:center;"></div>
      	  		</div>
      	  	</div>


	  </td>
	</tr>

	</tbody>	
  </table>		
					
		</td>
	</form>					
				</tr>
			</table>				
			<!--===================================End of Main Content===========================================-->
</td>

    <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>

<div id="footer"></div>
</body>
</html>
