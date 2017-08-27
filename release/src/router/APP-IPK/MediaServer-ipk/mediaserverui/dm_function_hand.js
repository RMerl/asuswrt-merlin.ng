var Refresh_time = 5000; 
var dm_array = new Array();     //all tasks
var dm_array_tmp = new Array();   //the tasks saved in this array 
var cancel_array = new Array();
var ProgressBar_array = new Array();  //all progressBars
var dm = new Array();       	//the selected task
var dm_tmp = new Array();       //saved selected task for remove
var dm_Log = new Array();   	//the selected task's desired logInfo
var dm_num = 0;             	//old task nums
var MAX_NAMELEN = 1024;      	//max url len
//var FtpUrl = "";            	//for add ftp task
//var save_user_pass ="";     	//for add ftp task
var ajaxRequest = true;    	//for circle ajaxRequest
//var app_ready =false;
//var grid;
var old_dmid = "";          	//old selected task's dmid
var old_dmid_tmp = "";      	//save old selected task's dmid
//var old_taskType= "";       	//old selected task's type
var sorted_by = "All";          //used for show desire tasks 
var old_sorted_by = "All";
//var old_sorted_by1 = "All";         //used for rember the old desire tasks
var old_sorted_by2 = "All";
//var taskid = "";
var WH_INT = 0;
var TASK_INT = 0;


function showSortList()
{
	if(sorted_by == "All"){
		dm_array.length = 0;
		dm_array = dm_array_tmp.concat();
		create_task();
		dm_num = dm_array.length;
		old_sorted_by = sorted_by;
		update_task();
		}
	else{
		var array = new Array();
		dm_array.length = 0;
		dm_array = dm_array_tmp.concat();
		for(var i =0;i<dm_array.length;i++){
			if(dm_array[i][4] == sorted_by){
				array.push(dm_array[i]);
				}
			}
		dm_array.length = 0;
		dm_array = array.concat();
		create_task();
		dm_num = dm_array.length;
		old_sorted_by = sorted_by;
		update_task();
		}
}

function showTask_select(type){
	switch(type){
		case 0:	if($j("#All_tasks").attr("class") == "noselected"){
						$j("#taskClass").children("span").removeClass();
						$j("#All_tasks").siblings().addClass("noselected");
						$j("#All_tasks").addClass("selected2");
						sorted_by = "All";
						showSortList();
					}
					break;
		case 1:	if($j("#Downloading_tasks").attr("class") == "noselected"){
								$j("#taskClass").children("span").removeClass();
								$j("#Downloading_tasks").siblings().addClass("noselected");
								$j("#Downloading_tasks").addClass("selected2");
								sorted_by = "Downloading";
								showSortList();
								//Ajax_Get_DM_Status();   here  need to try on
							}
							break;
		case 2:	if($j("#Seeding_tasks").attr("class") == "noselected"){
							$j("#taskClass").children("span").removeClass();
							$j("#Seeding_tasks").siblings().addClass("noselected");
							$j("#Seeding_tasks").addClass("selected2");
							sorted_by = "Seeding";
							showSortList();
						}
						break;
		case 3:	if($j("#Paused_tasks").attr("class") == "noselected"){
							$j("#taskClass").children("span").removeClass();
							$j("#Paused_tasks").siblings().addClass("noselected");
							$j("#Paused_tasks").addClass("selected2");
							sorted_by = "Paused";
							showSortList();
						}
						break;
		case 4: if($j("#Finished_tasks").attr("class") == "noselected"){
							 $j("#taskClass").children("span").removeClass();
							 $j("#Finished_tasks").siblings().addClass("noselected");
							 $j("#Finished_tasks").addClass("selected2");
							 sorted_by = "Finished";
							 showSortList();
						 }
						break;
		}
	
	}

function getWH(){
	var winWidth;
	var winHeight;
	winWidth = document.documentElement.scrollWidth;
	if(document.documentElement.clientHeight > document.documentElement.scrollHeight)
	winHeight = document.documentElement.clientHeight;
	else
	winHeight = document.documentElement.scrollHeight;
	$("DM_mask").style.width = winWidth+"px";
	$("DM_mask").style.height = winHeight+"px";
	}

/*function showPanel() {
	 WH_INT = setInterval("getWH();",1000);
   	 $j("#DM_mask").fadeIn(1000);
     $j("#panel_add").show(1000);
	 var task_code = '<span style="margin-right:15px;margin-left:15px;"><b>'+multiLanguage_array[multi_INT][18]+'</b></span>\n';
	 task_code += '<input type="file" id="open_usb_dm_url" value="" name="filename" /><br /><br />\n';
	 $("open_usb_dm_url_div").innerHTML = task_code;
	 $j("#HTTP_usb_dm_url").attr("value","");
}

function hidePanel() {
	clearInterval(WH_INT);
	$j("#DM_mask").fadeOut('fast');
	$j("#panel_add").hide('fast');
	$j("#HTTP_usb_dm_url").attr("value","");
	var task_code = '<div id="open_usb_dm_url_div" style="display:block; float:left;"></div>\n';
	$j(task_code).replaceAll("#open_usb_dm_url_div");
	}*/

//deal with dm_ctrl_status responsed
function response_dm_ctrl(data){
	//updateCtrlIcon();
	if(data.search(/ACK_SUCESS/)>=0)
	{
		return;
	}
	else if(data.search(/ACK_FAIL/)>=0)
	{
		return;
	}
	else if(data.search(/ACK_LIMIT/)>=0)
	{
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3],"Time limit!");
		return;
	}
	
	}
	
function response_dm_clear(data){
	if(data.search(/ACK_SUCESS/)>=0){
		var task_code = '<div style="display:none;"></div>';
		for(var i=0; i<dm_array.length; i++){
			if(dm_array[i][4] == "Seeding" || dm_array[i][4] == "Finished"){
				$j(task_code).replaceAll("#"+dm_array[i][0]);
				for(var j=0; j<ProgressBar_array.length; j++){
					if(dm_array[i][0] == ProgressBar_array[j].stateId.substring(4))
					ProgressBar_array.splice(j,1);
					}
				}
			}
	}
}

function dm_ctrl_status(status){
	var url = "ms_apply.cgi";
	var action_mode = "DM_CTRL";
	var dm_ctrl ="";
	var download_type ="";
	var t;
	switch(status){
		case "pause_all":
							dm_ctrl="pause_all";
							url += "?action_mode="+action_mode;
							url +="&dm_ctrl="+dm_ctrl;
							url +="&download_type=ALL&t="+Math.random();
							$j.get(url,function(data){response_dm_ctrl(data);});
							break;
		case "start_all":
							dm_ctrl="start_all";
							url += "?action_mode="+action_mode;
							url +="&dm_ctrl="+dm_ctrl;
							url +="&download_type=ALL&t="+Math.random();
							$j.get(url,function(data){response_dm_ctrl(data);});
							break;
		case "clear":
							dm_ctrl="clear";
							url += "?action_mode="+action_mode;
							url +="&dm_ctrl="+dm_ctrl;
							url += "&task_id="+dm[0];
							url +="&download_type="+dm[5]+"&t="+Math.random();
							$j.get(url,function(data){response_dm_clear(data);});
							break;
		case "cancel":
							dm_ctrl="cancel";
							url += "?action_mode="+action_mode;
							url +="&dm_ctrl="+dm_ctrl;
							url += "&task_id="+dm_tmp[0];
							url +="&download_type="+dm_tmp[5]+"&t="+Math.random();
							dm.length = 0;
							$j.get(url,function(data){response_dm_cancel(data);});
							break;
		case "paused":
							dm_ctrl="paused";
							url += "?action_mode="+action_mode;
							url +="&dm_ctrl="+dm_ctrl;
							url += "&task_id="+dm[0];
							url +="&download_type="+dm[5]+"&t="+Math.random();
							$j.get(url,function(data){response_dm_ctrl(data);});
							break;
		case "start":
							dm_ctrl="start";
							url += "?action_mode="+action_mode;
							url +="&dm_ctrl="+dm_ctrl;
							url += "&task_id="+dm[0];
							url +="&download_type="+dm[5]+"&t="+Math.random();
							$j.get(url,function(data){response_dm_ctrl(data);});
							break;
		}
	}

function DM_Ctrl(action){
	switch(action){
		case "pause_all":
							if(bt_initialing("pause_all"))
							return;
							dm_ctrl_status("pause_all");
							break;
		case "start_all":
							if(bt_initialing("start_all"))
							return;
							dm_ctrl_status("start_all");
							break;
		case "clear":
						if(bt_initialing("clear"))
						return;
						dm_ctrl_status("clear");
						break;
		case "cancel":
						if(bt_initialing("cancel"))
						return;
						dm_tmp = dm;
						if(confirm(multiLanguage_array[multi_INT][15]))
						dm_ctrl_status("cancel");
						break;
		case "paused":
						if(bt_initialing("paused"))
						return;
						dm_ctrl_status("paused");
						break;
		case "start":
						if(bt_initialing("start"))
						return;
						dm_ctrl_status("start");
						break;
		}
	
	}

function bt_initialing(status){
	var BT = "BT";
	var Initialing = "Initialing";
	var Finished = "Finished";
	var Seeding = "Seeding";
	switch(status){
		case "pause_all":
		case "start_all":for(var i=0; i < dm_array.length; i++){     //select all task
							if(dm_array[i][5] == BT && dm_array[i][4] ==Initialing){
								Ext.MessageBox.alert(multiLanguage_array[multi_INT][3],multiLanguage_array[multi_INT][0]+"\n"+multiLanguage_array[multi_INT][1]);
								return 1;
								}
							
						};
						return 0;
						break;
		
		case "cancel":
		case "paused":
		case "start":if(dm[5] == BT && dm[4] == Initialing){
							Ext.MessageBox.alert(multiLanguage_array[multi_INT][3],multiLanguage_array[multi_INT][0]+"\n"+multiLanguage_array[multi_INT][1]);
							return 1;
						}
						return 0;
						break;
		case "clear":for(var i= 0; i<dm_array.length;i++){
							if(dm_array[i][4] == Seeding || dm_array[i][4] == Finished){
								return 0;
								}
						}
						Ext.MessageBox.alert(multiLanguage_array[multi_INT][3],multiLanguage_array[multi_INT][2]);
						return 1;
						break;
		}
	
	}

// check http and ftp url
function check_enter_url(url){
	var download_type = 0;      //http is 1, ftp is 2, BT is 3 ,NZB is 4 ,false is 0
	
	if(!url || url.length==0)
	{



		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3],multiLanguage_array[multi_INT][4]);
		return -1;
	}
	else if(url.length > MAX_NAMELEN)
	{
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3],multiLanguage_array[multi_INT][5]);
		return -1;
	}
	else{
			url = url.toLowerCase();
			var http=/^http(.*)/
			var ftp=/^ftp(.*)/
			var bt=/(.*)\.torrent/
			var magnet=/^magnet:\?(.+)/
			if(http.test(url)){
				download_type = 1;
				}
			else if(ftp.test(url)){
				download_type = 2;	
				}
			else if(magnet.test(url)){
				download_type = 3;
				}
			else if(bt.test(url)){
				download_type = 3;
				}
			
		}
		switch(download_type){
			case 1:var httpreg=/http(s)?:\/\/[A-Za-z0-9]+\.[A-Za-z0-9]+[\/=\?%\-&_~`@[\]\':+!]*([^<>\"\"])*$/				
					if (httpreg.test(url))    
			{
				if(bt.test(url)){
					download_type = 3;
					return download_type;
					}
					else
				return download_type;
			}else
			return 0;
			break;
			case 2:/*if($j("#User_name").attr("value")=="")
				{
					var ftpreg=/ftp:\/\/[A-Za-z0-9]+\.[A-Za-z0-9]+[\/=\?%\-&_~`@[\]\':+!]*([^<>\"\"])*$/
					}
			else if($j("#User_name").attr("value")!="" && $j("#Password").attr("value")!="")
				{
					var ftpreg=/ftp:\/\/[0-9a-z_!~*'().&=+$%-]+:+[0-9a-z_!~*'().&=+$%-]+@+[A-Za-z0-9]+\.[A-Za-z0-9]+[\/=\?%\-&_~`@[\]\':+!]*([^<>\"\"])*$/
				}
			else if($j("#User_name").attr("value")!="" && $j("#Password").attr("value")=="")
				{
					var ftpreg=/ftp:\/\/[0-9a-z_!~*'().&=+$%-]+@+[A-Za-z0-9]+\.[A-Za-z0-9]+[\/=\?%\-&_~`@[\]\':+!]*([^<>\"\"])*$/
					}
				else{}
			if (ftpreg.test(url))		
			{
				if(bt.test(url)){
					download_type = 3;
					return download_type;
					}
					else*/
				return download_type;
			/*}else
			return 0;
			break;*/
			case 3: return download_type;
					break;
			default: return 0;break;
			
			}
	}

// check bt and nzb url
function check_open_url(url){
	var download_type = 0;  //http is 1, ftp is 2, BT is 3 ,NZB is 4 ,false is 0
	if(!url || url.length==0)
	{
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3],multiLanguage_array[multi_INT][4]);
		return -1;
	}
	else if(url.length > MAX_NAMELEN)
	{
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3],multiLanguage_array[multi_INT][5]);
		return -1;
	}
	else{
		url = url.toLowerCase();
		var bt=/(.*)\.torrent/
		var nzb=/(.*)\.nzb/
		if(bt.test(url)){
			download_type = 3;
			}
		else if(nzb.test(url)){
			download_type = 4;	
			}
		}
		switch(download_type){
			case 3:	var is_torrent_file = url.search(/\.torrent$/);
					is_torrent_file = is_torrent_file >=0 ? true : false;
					if(is_torrent_file){
						return download_type;
						}else
					return 0;
					break;
			case 4: var is_nzb_file = url.search(/\.nzb$/);
					is_nzb_file = is_nzb_file >=0 ? true : false;
					if(is_nzb_file){
						return download_type;
						}else
					return 0;
					break;
			default: return 0;break;
			}
	}

function dm_add_status(){
	var action_mode = "";                //action_mode
	//var file_type = $j("#filetype").attr("value");          //file is 0, folder is 1
	var t;
	if(document.getElementById("HTTP_usb_dm_url").value == "")
		{
			Ext.MessageBox.alert(multiLanguage_array[multi_INT][3],multiLanguage_array[multi_INT][6]);
			}
	//http and ftp add task
	else{
		var usb_dm_url =""
		//if(status == "add_FTP")
		//usb_dm_url += $j("#FTP_usb_dm_url").attr("value");
		//else if(status == "add_HTTP")
		usb_dm_url += $j("#HTTP_usb_dm_url").attr("value");
		usb_dm_url = usb_dm_url.replace(/\n/g,'');
		//else if(status == "add_magnet")
		//usb_dm_url += $j("#magnet_usb_dm_url").attr("value");
		var url = "ms_apply.cgi";
		var downloadtype = 5; //http is 1, ftp is 2, BT is 3 ,NZB is 4 ,false is 0
		action_mode = "DM_ADD";
		url += "?action_mode=" + action_mode;
		url += "&download_type=" + downloadtype;
		url += "&again=no";
		url += "&usb_dm_url=" + encodeURIComponent(usb_dm_url)+"&t="+Math.random();
		$j.get(url,function(data){response_dm_add(data);});
		}
	
	}
	
function Ftp_dm_add_status(){
	var http=/^http:\/\/(.*)/
	var ftp=/^ftp:\/\/(.*)/
	var head=/^(.*):\/\//
	var user = $j("#User_name").attr("value");
	var pass = $j("#Password").attr("value");
	var user_pass = "";
	var foot = "";
	var furl = dm[1];
	var fid = dm[0];
	furl_s = furl.toLowerCase();
	var len = furl.length;
	if(user != "" && pass != "")
	user_pass = user + ":" + pass + "@";
	else if(user != "" && pass == "")
	user_pass = user + "@";
	else
	user_pass = "";
	if(ftp.test(furl_s)){
		foot = furl.substr(6,len-6);
		furl = "ftp://" + user_pass + foot;
	}
	else if(http.test(furl_s)){
		foot = furl.substr(7,len-7);
		furl = "http://" + user_pass + foot;
		}
	else{
		if(head.test(furl_s)){
					 var a = furl_s.indexOf("://",0);
					 var he = furl.substr(0,a+3);
					 foot = furl.substr(a+3,len-a-3);
					 furl = he + user_pass + foot;
					 }
		
		else
		furl = user_pass + furl;
		}
		var url = "ms_apply.cgi";
		var downloadtype = 5; //http is 1, ftp is 2, BT is 3 ,NZB is 4 ,false is 0
		action_mode = "DM_ADD";
		url += "?action_mode=" + action_mode;
		url += "&download_type=" + downloadtype; 
		url += "&again=yes";
		url += "&fid=" + fid;
		url += "&usb_dm_url=" + encodeURIComponent(furl)+"&t="+Math.random();
		$j.get(url,function(data){response_dm_add(data);});
		hideUnamePsw();
	}

function add_progress(i,progress){
			
			var j = dm_array[i][0];
			var ProgressBar = "ProgressBar" + j;
			var pbid = "pbid" + j;
			ProgressBar = new Ext.ProgressBar({
			autoWidth:true,
			text:'working....',
			//width:400,
			renderTo:pbid,
			stateId:pbid
			});
			//alert(ProgressBar_array.length+"    "+dm_num)
			if(ProgressBar_array.length == (dm_num-1)){
				ProgressBar_array.push(ProgressBar);
				}
			else{
				var ProgressBar_array1 = new Array();
				for(var x=0;x < ProgressBar_array.length;x++){
				if(ProgressBar_array[x].stateId != ProgressBar.stateId)
				ProgressBar_array1.push(ProgressBar_array[x]);
				else
				ProgressBar_array1.push(ProgressBar);
				}
			ProgressBar_array = ProgressBar_array1;
			}
			var ProgressBar_array2 = new Array();
			for(var m=0; m<dm_array.length; m++){
				for(var n=0; n<ProgressBar_array.length; n++){
					if(dm_array[m][0] == ProgressBar_array[n].stateId.substring(4))
					ProgressBar_array2.push(ProgressBar_array[n]);
					}
				}
			ProgressBar_array.length = 0;
			ProgressBar_array = ProgressBar_array2;
			update_progress(ProgressBar,progress);
	}

function changeshowicon(iconid){
	$j("#"+iconid).attr("src","images/icon/Ino2Icon_click.png");
	}
function returnshowicon(iconid){
	$j("#"+iconid).attr("src","images/icon/Ino2Icon.png");
	}
function changeInfoIcon(icon,iconid){
	$j("#"+iconid).attr("src",icon);
	}
function returnInfoIcon(icon,iconid){
	$j("#"+iconid).attr("src",icon);
	}

function add_task(i){
	var filename,progress,size,status,type,elased,download,upload,peers;//error;
	var progressTxt;
		filename = dm_array[i][1];
		progress = dm_array[i][2];
		size = dm_array[i][3];
		status = dm_array[i][4];
		type = dm_array[i][5];
		download = parseInt(dm_array[i][7]);
		upload = parseInt(dm_array[i][8]);
		peers = dm_array[i][9];
	progressTxt = (parseFloat(size)*progress).toFixed(2) + size.substr(size.length-3,3);
	var j = dm_array[i][0];
	var task_code = '';
	var task_code2 = '';
	task_code +='<table id="'+j+'" class="taskLongWidth" style="border-bottom:#000 solid 1px; table-layout:fixed;" onclick="selectedTask(this.id);">\n';
		task_code +='<tr><td colspan="9" style="overflow:hidden;"><div class="zxx_text_overflow_2" style="font-weight:bold; font-size:14px;" id="filename'+j+'">'+filename+'</div></td></tr>\n';
		task_code +='<tr><td colspan="2"><span style="margin-right:30px;" id="type'+j+'">'+type+'</span></td>\n';
		if(status!="notbegin"){
		task_code +='<td colspan="7" id="progresstd'+j+'"><span id="progress'+j+'">'+progressTxt+'</span> of <span id="size'+j+'">'+size+'</span> downloaded</td></tr>\n';
		}
		task_code +='<tr><td colspan="8" id="pbid'+j+'"></td>\n';
		task_code +='<td style="width:20px;"><img id="status_icon'+j+'" src="images/statusIcon/'+status+'.gif" alt="" /></td></tr>\n';
		task_code +='<tr><td colspan="3"><a id="showUP'+j+'" href="javascript:showUnamePsw();" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>'+multiLanguage_array[multi_INT][69]+'</span></a>\n';
		task_code +='<a id="showUP_n1'+j+'" href="http://'+location.host+'/Setting_NZB.asp" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>'+multiLanguage_array[multi_INT][70]+'</span></a>\n';
		task_code +='<a id="showUP_n2'+j+'" href="http://'+location.host+'/Setting_NZB.asp" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>'+multiLanguage_array[multi_INT][71]+'</span></a>\n';
		task_code +='<a id="showUP_n3'+j+'" href="http://'+location.host+'/Setting_NZB.asp" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>'+multiLanguage_array[multi_INT][72]+'</span></a>\n';
		task_code +='<a id="showUP_n4'+j+'" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span> The file\'s data blocks are not complete</span></a>\n';
		task_code +='<span id="status'+j+'">'+status+'</span></td>\n';
		if(status!="notbegin"){
		if(type!="HTTP"&&type!="FTP"){
		task_code +='<td id="peerstd'+j+'" colspan="2"><span id="peers'+j+'">'+peers+'</span> peers</td>\n';
		}
		else{
		task_code +='<td colspan="2"></td>\n';
		}
		task_code +='<td id="downloadtd'+j+'" align="right" colspan="2" id="downloadplace'+j+'"><img width="11" height="11" src="images/icon/DownArrowIcon.png" /><span id="download'+j+'">'+download+'</span> KBps</td>\n';
		if(type!="HTTP"&&type!="FTP"&&type!="NZB"){
		task_code +='<td colspan="2" id="uploadtd'+j+'"><img width="11" height="11" src="images/icon/UpArrowIcon.png" /><span id="upload'+j+'">'+upload+'</span> KBps</td>\n';
		}
		else{
		task_code +='<td colspan="2"></td>\n';
		}
		}
		task_code +='</tr>\n';
		task_code +='<tr><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td></tr>\n';
		task_code +='</table>\n';
		task_code2 +='<div id="div_for_addtask" style="display:none;"></div>\n';
		//alert(dm_array[dm_num-1][0]);
		$j(task_code).replaceAll("#div_for_addtask");
		$j("#transfers").html(dm_array.length);
		dm_num = dm_array.length;
		add_progress(i,progress);
		$j("#"+j).after(task_code2);
		updateCtrlIcon();
		//clear taskLog everytime wehn task num changed
		//create_taskLogFilename();
	
	}


function cancel_task(data){
	eval("dm_array = [" + data + "]");
	eval("dm_array_tmp = [" + data + "]");
		create_task();
		dm_num = dm_array.length;
		//old_sorted_by = sorted_by;
		update_task();
		ajaxRequest = true;
	}
	
function response_dm_cancel(data){
	if(data.search(/ACK_SUCESS/)>=0)
	{
		//pidname = "";	
		//alert("ACK_SUCESS");
		Ajax_Get_DM_Status_inphase();
		TASK_INT = setInterval("Ajax_Get_DM_Status();",Refresh_time);
		return;
	}
	else if(data.search(/ACK_FAIL/)>=0)
	{
		//pidname = "";
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3],multiLanguage_array[multi_INT][8]);
		//alert("ACK_FAIL");
		return;
	}
	}

function response_dm_add(data){
	//hidePanel();
	if(data.search(/ACK_SUCESS/)>=0)
	{
		//pidname = "";	
		//alert("ACK_SUCESS");
		//create_task();
		//dm_num = dm_array.length;
		//old_sorted_by = sorted_by;
		//update_task();
		Ajax_Get_DM_Status();
		TASK_INT = setInterval("Ajax_Get_DM_Status();",Refresh_time);
		//Ajax_Get_DM_Status();
		//add_task();
		return;
	}
	else if(data.search(/ACK_FAIL/)>=0)
	{
		//pidname = "";
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3],multiLanguage_array[multi_INT][9]);
		//alert("ACK_FAIL");
		return;
	}
	else if(data.search(/BT_EXIST/)>=0)
	{
		//pidname = "";
		//hidePanel("add");
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3],multiLanguage_array[multi_INT][10]);
		return;
	}
	else if(data.search(/LIGHT_FULL/)>=0)
	{
		//pidname = "";
		//hidePanel("add");
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3],multiLanguage_array[multi_INT][11]);
		return;
	}
	else if(data.search(/HEAVY_FULL/)>=0)
	{
		//pidname = "";
		//hidePanel("add");
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3],multiLanguage_array[multi_INT][12]);
		return;
	}
	else if(data.search(/NNTP_FULL/)>=0)
	{
		//pidname = "";
		//hidePanel("add");
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3],multiLanguage_array[multi_INT][13]);
		return;
	}
	else if(data.search(/TOTAL_FULL/)>=0)
	{
		//pidname = "";
		//hidePanel("add");
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3],multiLanguage_array[multi_INT][14]);
		return;
	}
	else if(data.search(/DISK_FULL/)>=0)
	{
		//pidname = "";
		//hidePanel("add");
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3],"File size is larger then the space left on the partition");
		return;
	}
	}


function Ajax_Get_DM_Status_inphase(){
	ajaxRequest = false;
	var t;
	var url = "ms_print_status.cgi" + "?action_mode=" +sorted_by+"&t="+Math.random();
	$j.ajax({url: url,
			async: false,
			success: function(data){cancel_task(data)},
			complete: function(XMLHttpRequest, textStatus){;}
			});
	
	}

function Ajax_Get_DM_Status()
{
	if(ajaxRequest){
	var t;
	old_sorted_by2 = sorted_by;
	var url = "ms_print_status.cgi" + "?action_mode=All&t="+Math.random();
	$j.get(url,function(data){showDMList(data);});
	}	
}


//when dm_array change,or taskid change, or status change,update icon
function updateCtrlIcon(){
	var Finished = "Finished";
	var Seeding = "Seeding";
	var Downloading = "Downloading";
	var Paused = "Paused";
	var Error = "Error";
	var Stop = "Stop";
	//var com = false;
	$j("#icon_add").children("a").attr("href","javascript:create_AddTask();");
	if(dm_array.length >0 ){// have tasks
		var a = true;
		var b = true;
		var c = true;
		for(var i=0; i<dm_array.length; i++){//have Downloading task
			if(dm_array[i][4] == Downloading || dm_array[i][4] == Seeding){
				$j("#icon_pause_all").children("a").attr("href","javascript:DM_Ctrl('pause_all');");
				a = false;
				break;
			}
		}
		for(var i=0; i<dm_array.length; i++){//have Paused task
			if(dm_array[i][4] == Paused){
				$j("#icon_resume_all").children("a").attr("href","javascript:DM_Ctrl('start_all');");
				b = false;
				break;
			}
		}
		// have completed tasks
		for(var i=0; i<dm_array.length; i++){
			if(dm_array[i][4] == Seeding || dm_array[i][4] == Finished){
				$j("#icon_clear").children("a").attr("href","javascript:DM_Ctrl('clear');");
				c = false;
				break;
				}
			}
		if(a)
		$j("#icon_pause_all").children("a").removeAttr("href");
		if(b)
		$j("#icon_resume_all").children("a").removeAttr("href");
		if(c)
		$j("#icon_clear").children("a").removeAttr("href");
		
		/*else
		$j("#icon_clear").children("a").removeAttr("href");
		*/
		if(dm.length != 0){
			// have selected task
			$j("#icon_del").children("a").attr("href","javascript:create_CancelTask();");
			if(dm[4] == Downloading || dm[4] == Seeding){
				// the selected task's status is download
				$j("#icon_pause").children("a").attr("href","javascript:DM_Ctrl('paused');");
				$j("#icon_resume").children("a").removeAttr("href");
				}
				// the selected task's status is pause || error || stoped
			else if(dm[4] == Paused || dm[4] == Error || dm[4] == Stop){
				$j("#icon_resume").children("a").attr("href","javascript:DM_Ctrl('start');");
				$j("#icon_pause").children("a").removeAttr("href");
				}
			else{
				$j("#icon_pause").children("a").removeAttr("href");
				$j("#icon_resume").children("a").removeAttr("href");
				}
			}
		else{
				$j("#icon_del").children("a").removeAttr("href");
				$j("#icon_pause").children("a").removeAttr("href");
				$j("#icon_resume").children("a").removeAttr("href");
				}
		}
		else
		{
			$j("#icon_clear").children("a").removeAttr("href");
			$j("#icon_del").children("a").removeAttr("href");
			$j("#icon_pause").children("a").removeAttr("href");
			$j("#icon_resume").children("a").removeAttr("href");
			$j("#icon_pause_all").children("a").removeAttr("href");
			$j("#icon_resume_all").children("a").removeAttr("href");
			}
	
	}

//dm is the selected task
function selectedTask(id){
	for(var i=0;i<dm_array.length;i++){
		if (id==dm_array[i][0]){
			dm = dm_array[i];
			break;
		}
		}
		change_bgcolor();
		updateCtrlIcon();
}


//task that have be selected will have a different bgcolor
function change_bgcolor(){
	var dmid = dm[0];
	if (old_dmid != ""){
		if (dmid != old_dmid){
			document.getElementById(dmid).bgColor="#333";
			$j("#"+old_dmid).removeAttr("bgColor");	
			}
		}
	else{
		document.getElementById(dmid).bgColor="#333";
		}	
		old_dmid_tmp = old_dmid;
		old_dmid = dmid;
	}


function create_progress(){
		ProgressBar_array.length = 0;
		var task_code;
		for(var i=0 ; i<dm_array.length ; i++){
			var j = dm_array[i][0];
			task_code = "<td colspan='8' id='pbid"+j+"'></td>";
			$j(task_code).replaceAll("#pbid"+j);
			}
		for(var i=0 ; i<dm_array.length ; i++){
			
			var j = dm_array[i][0];
			var ProgressBar = "ProgressBar" + j;
			var pbid = "pbid" + j;
			ProgressBar = new Ext.ProgressBar({
			autoWidth:true,
			text:'working....',
			//width:400,
			renderTo:pbid,
			stateId:pbid
			});
			ProgressBar_array[i] = ProgressBar;
		}
}


function showUnamePsw(){
	WH_INT = setInterval("getWH();",1000);
	$j("#User_name").attr("value","");
	$j("#Password").attr("value","");
	$j("#DM_mask").fadeIn(1000);
    $j("#UnamePsw_panel").show(1000);
	}
function hideUnamePsw(){
	clearInterval(WH_INT);
	$j("#DM_mask").fadeOut('fast');
	$j("#UnamePsw_panel").hide('fast');
	}


function create_task(){
	//taskid = "";
	
	$("handHidden").style.display = "inherit";
	ProgressBar_array.length = 0;
	dm.length = 0;
	old_dmid = "";
	old_dmid_tmp = "";
	var filename,progress,size,status,type,elased,download,upload,peers;//error;
	var progressTxt;
	
	var task_code="";
	for(var i = 0; i < dm_array.length ; i++){
		var j = dm_array[i][0];
		filename = dm_array[i][1];
		progress = dm_array[i][2];
		size = dm_array[i][3];
		status = dm_array[i][4];
		type = dm_array[i][5];
		download = parseInt(dm_array[i][7]);
		upload = parseInt(dm_array[i][8]);
		peers = dm_array[i][9];
		progressTxt = (parseFloat(size)*progress).toFixed(2) + size.substr(size.length-3,3);
		task_code +='<table id="'+j+'" class="taskLongWidth" style="border-bottom:#000 solid 1px; table-layout:fixed;" onclick="selectedTask(this.id);">\n';
		task_code +='<tr><td align="left" colspan="9" style="overflow:hidden;"><div style="font-weight:bold; font-size:14px;" class="zxx_text_overflow_2" id="filename'+j+'">'+filename+'</div></td></tr>\n';
		task_code +='<tr><td colspan="2" align="left"><span style="margin-right:30px;" id="type'+j+'">'+type+'</span></td>\n';
		if(status!="notbegin"){
		task_code +='<td align="left" colspan="7" id="progresstd'+j+'"><span id="progress'+j+'">'+progressTxt+'</span> of <span id="size'+j+'">'+size+'</span> downloaded</td></tr>\n';
		}
		task_code +='<tr><td colspan="8" id="pbid'+j+'"></td>\n';
		task_code +='<td style="width:20px;"><img id="status_icon'+j+'" src="images/statusIcon/'+status+'.gif" alt="" /></td></tr>\n';
		task_code +='<tr><td align="left" colspan="3"><a id="showUP'+j+'" href="javascript:showUnamePsw();" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>'+multiLanguage_array[multi_INT][69]+'</span></a>\n';
		task_code +='<a id="showUP_n1'+j+'" href="http://'+location.host+'/Setting_NZB.asp" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>'+multiLanguage_array[multi_INT][70]+'</span></a>\n';
		task_code +='<a id="showUP_n2'+j+'" href="http://'+location.host+'/Setting_NZB.asp" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>'+multiLanguage_array[multi_INT][71]+'</span></a>\n';
		task_code +='<a id="showUP_n3'+j+'" href="http://'+location.host+'/Setting_NZB.asp" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>'+multiLanguage_array[multi_INT][72]+'</span></a>\n';
		task_code +='<a id="showUP_n4'+j+'" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span> The file\'s data blocks are not complete</span></a>\n';
		task_code +='<span id="status'+j+'">'+status+'</span></td>\n';
		if(status!="notbegin"){
		if(type!="HTTP"&&type!="FTP"){
		task_code +='<td align="left" colspan="2" id="peerstd'+j+'"><span id="peers'+j+'">'+peers+'</span> peers</td>\n';
		}
		else{
		task_code +='<td colspan="2"></td>\n';
		}
		task_code +='<td id="downloadtd'+j+'" align="right" colspan="2" id="downloadplace'+j+'"><img width="11" height="11" src="images/icon/DownArrowIcon.png" /><span id="download'+j+'">'+download+'</span> KBps</td>\n';
		if(type!="HTTP"&&type!="FTP"&&type!="NZB"){
		task_code +='<td colspan="2" id="uploadtd'+j+'"><img width="11" height="11" src="images/icon/UpArrowIcon.png" /><span id="upload'+j+'">'+upload+'</span> KBps</td>\n';
		}
		else{
		task_code +='<td colspan="2"></td>\n';
		}
		}
		task_code +='</tr>\n';
		task_code +='<tr><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td></tr>\n';
		task_code +='</table>\n';
	}
		task_code +='<div id="div_for_addtask" style="display:none;"></div>\n';
		$("task_main_div").innerHTML = task_code;
		$j("#transfers").html(dm_array.length);
		create_progress();
		updateCtrlIcon();
	
}

function create_AddTask(){
	clearInterval(TASK_INT);
	showTask_select(0);
	$("handHidden").style.display= "none";
	var task_code = "<table align='center'><tr><td align='center'>\n";
	task_code += "<img src='images/Hand_ui/alertImg.gif' alt='' /></td</tr>\n";
	task_code += "<tr><td align='center' style='font-size:16px;'>Please Key The URL</td></tr>\n";
	task_code += "<tr><td align='center' style='font-size:16px;'><span>URL:</span><input type='text' id='HTTP_usb_dm_url' value='' /></td></tr>\n";
	task_code += "<tr><td align='center'><br /><input class='button_gen' onclick='dm_add_status();' type='button' value='Apply'/>   <input class='button_gen' onclick='Cancel_ControlTask();' type='button' value='Cancel'/></td></tr>\n";
	task_code += "</table>\n";
	$("task_main_div").innerHTML = task_code;
	$j("#icon_add").children("a").removeAttr("href");
	$j("#icon_clear").children("a").removeAttr("href");
	$j("#icon_del").children("a").removeAttr("href");
	$j("#icon_pause").children("a").removeAttr("href");
	$j("#icon_resume").children("a").removeAttr("href");
	$j("#icon_pause_all").children("a").removeAttr("href");
	$j("#icon_resume_all").children("a").removeAttr("href");
	dm_num = 0;
	}

function create_CancelTask(){
	clearInterval(TASK_INT);
	$("handHidden").style.display = "none";
	dm_tmp = dm;
	var task_code = "<div align='center' style='width:100%;'>\n";
	task_code += "<img src='images/Hand_ui/alertImg.gif' alt='' /></div>\n";
	task_code += "<div align='center' class='zxx_text_overflow_2'>Remove "+dm_tmp[1]+"</div>\n";
	task_code += "<div align='center' style='font-size:13px; width:100%;'>Are you sure you want to remove it?</div>\n";
	task_code += "<div align='center' style='width:100%;'><br /><input class='button_gen' onclick='dm_ctrl_status(\"cancel\");' type='button' value='Apply'/> <input class='button_gen' onclick='Cancel_ControlTask();' type='button' value='Cancel'/></div>\n";
	task_code += "\n";
	$("task_main_div").innerHTML = task_code;
	$j("#icon_add").children("a").removeAttr("href");
	$j("#icon_clear").children("a").removeAttr("href");
	$j("#icon_del").children("a").removeAttr("href");
	$j("#icon_pause").children("a").removeAttr("href");
	$j("#icon_resume").children("a").removeAttr("href");
	$j("#icon_pause_all").children("a").removeAttr("href");
	$j("#icon_resume_all").children("a").removeAttr("href");
	dm_num = 0;
	}
	
function Cancel_ControlTask(){
	dm.length = 0;
	Ajax_Get_DM_Status();
	updateCtrlIcon();
	TASK_INT = setInterval("Ajax_Get_DM_Status();",Refresh_time);
	}

function update_progress(ProgressBar,percentage){
		var x = percentage*10000;
		var y = parseInt(x/100);
		var progressText = y + "%";
		ProgressBar.updateProgress(percentage,progressText);
	}

function updateDownloadplace(downloadplaceId){
	
	}

function update_task(){
	if (dm_array.length == 0)
	{
		$j("#total_uploadSpeed").html(0);
		$j("#total_downloadSpeed").html(0);
		return;
	
	}
	var totaldownloadSpeed = 0;
	var totaluploadSpeed = 0;
	var progressTxt;
	for (var i=0;i < dm_array.length;i++){
		var j = dm_array[i][0];
		if(!document.getElementById(j)){
			add_task(i);			
			}
		progressTxt = (parseFloat(dm_array[i][3])*dm_array[i][2]).toFixed(2) + dm_array[i][3].substr(dm_array[i][3].length-3,3);	
		//filename	
		if(document.getElementById("filename" + j) && $j("#filename" + j).html()!= dm_array[i][1])
			$j("#filename" + j).html(dm_array[i][1]);
		//status
		if(dm_array[i][4] == "Finished" && document.getElementById("downloadplace" + j))//根据状态如果是Finished，就隐藏掉下载速度
		$j("#downloadplace"+j).hide();
		if(dm_array[i][4] == "warning" && document.getElementById("showUP" + j))//根据状态需要用户名和密码去显示图标
		{$j("#showUP"+j).show();
		 $j("#showUP_n1"+j).hide();
		 $j("#showUP_n2"+j).hide();
		 $j("#showUP_n3"+j).hide();
		 $j("#showUP_n4"+j).hide();
		 $j("#status" + j).html("");}
		else if((dm_array[i][4] == "ConnectFail" || dm_array[i][4] == "SoketRecvFail") && document.getElementById("showUP_n1" + j))
		{$j("#showUP_n1"+j).show();
		$j("#showUP"+j).hide();
		$j("#showUP_n2"+j).hide();
		$j("#showUP_n3"+j).hide();
		$j("#showUP_n4"+j).hide();
		$j("#status" + j).html("");}
		else if(dm_array[i][4] == "SSLFail" && document.getElementById("showUP_n2" + j))
		{$j("#showUP_n2"+j).show();
		$j("#showUP"+j).hide();
		$j("#showUP_n1"+j).hide();
		$j("#showUP_n3"+j).hide();
		$j("#showUP_n4"+j).hide();
		$j("#status" + j).html("");}
		else if(dm_array[i][4] == "AccountFail" && document.getElementById("showUP_n3" + j))
		{$j("#showUP_n3"+j).show();
		$j("#showUP"+j).hide();
		$j("#showUP_n1"+j).hide();
		$j("#showUP_n2"+j).hide();
		$j("#showUP_n4"+j).hide();
		$j("#status" + j).html("");}
		else if(dm_array[i][4] == "NotCompleted" && document.getElementById("showUP_n4" + j)){
		$j("#showUP_n4"+j).show();
		$j("#showUP"+j).hide();
		$j("#showUP_n1"+j).hide();
		$j("#showUP_n2"+j).hide();
		$j("#showUP_n3"+j).hide();
		$j("#status" + j).html("");
			}
		else{
		$j("#showUP"+j).hide();
		$j("#showUP_n1"+j).hide();
		$j("#showUP_n2"+j).hide();
		$j("#showUP_n3"+j).hide();
		$j("#showUP_n4"+j).hide();
		}
		if(dm_array[i][4] == "Error" && document.getElementById("type"+j)){ //根据状态如果是Error，就隐藏掉一部分信息
			if(document.getElementById("peerstd"+j))
			$j("#peerstd"+j).hide();
			if(document.getElementById("uploadtd"+j))
			$j("#uploadtd"+j).hide();
			if(document.getElementById("downloadtd"+j))
			$j("#downloadtd"+j).hide();
			if(document.getElementById("progresstd"+j))
			$j("#progresstd"+j).hide();
			}
		if(dm_array[i][4] == "warning" && document.getElementById("type"+j)){ //根据状态如果是warning，就隐藏掉一部分信息
			if(document.getElementById("peerstd"+j))
			$j("#peerstd"+j).hide();
			if(document.getElementById("uploadtd"+j))
			$j("#uploadtd"+j).hide();
			if(document.getElementById("downloadtd"+j))
			$j("#downloadtd"+j).hide();
			if(document.getElementById("progresstd"+j))
			$j("#progresstd"+j).hide();
			}
		if(document.getElementById("status" + j) && $j("#status" + j).html()!= dm_array[i][4])
		{
			if(dm_array[i][4] == "warning" || dm_array[i][4] == "ConnectFail" || dm_array[i][4] == "SoketRecvFail" || dm_array[i][4] == "SSLFail" || dm_array[i][4] == "AccountFail" || dm_array[i][4] == "NotCompleted")
			$j("#status" + j).html("");
			else
			$j("#status" + j).html(dm_array[i][4]);   //change status
			//$j("#status_icon" +j).removeAttr("src");
			$j("#status_icon" +j).attr("src","images/statusIcon/"+dm_array[i][4]+".gif");  //chang status_icon 
			updateCtrlIcon();
			}
		//type
		if(document.getElementById("type" + j) && $j("#type" + j).html()!= dm_array[i][5])
			$j("#type" + j).html(dm_array[i][5]);
		//download
		if(document.getElementById("download" + j) && $j("#download" + j).html()!= parseInt(dm_array[i][7]))
			$j("#download" + j).html(parseInt(dm_array[i][7]));
		//upload
		if(document.getElementById("upload" + j) && $j("#upload" + j).html()!= parseInt(dm_array[i][8]))
			$j("#upload" + j).html(parseInt(dm_array[i][8]));
		//peers
		if(document.getElementById("peers" + j) && $j("#peers" + j).html()!= dm_array[i][9])
			$j("#peers" + j).html(dm_array[i][9]);
		//pgrogress
		if(document.getElementById("progress" + j) && $j("#progress" +j).html()!= progressTxt)
			$j("#progress" +j).html(progressTxt);
		//size
		if(document.getElementById("size" +j) && $j("#size" +j).html()!= dm_array[i][3])
			$j("#size" +j).html(dm_array[i][3]);
		//progressBar
		if(document.getElementById("pbid" + j) )   //还需要添加判断条件--看Progress是否有变化
		{
			//var progress = 0;
			for(var x=0;x<ProgressBar_array.length;x++)
			{
				if(ProgressBar_array[x].stateId == "pbid"+j)
				update_progress(ProgressBar_array[x],dm_array[i][2]);
				}
		}
		if(dm_array[i][7] != "")
		totaldownloadSpeed += parseFloat(dm_array[i][7]);
		if(dm_array[i][8] != "")
		totaluploadSpeed += parseFloat(dm_array[i][8]);
		}
		totaldownloadSpeed = parseInt(totaldownloadSpeed);
		totaluploadSpeed = parseInt(totaluploadSpeed);
		if(document.getElementById("total_downloadSpeed") && $j("#total_downloadSpeed").html() !=totaldownloadSpeed)
			$j("#total_downloadSpeed").html(totaldownloadSpeed);
		if(document.getElementById("total_uploadSpeed") && $j("#total_uploadSpeed").html() !=totaluploadSpeed)
			$j("#total_uploadSpeed").html(totaluploadSpeed);
		
	}
	
function backstage_cancel(){
	cancel_array.length = 0;
	for(var i=0; i<dm_array_tmp.length; i++){
		var a = true;
		for(var j=0; j<dm_array.length; j++){
			if(dm_array_tmp[i][0] == dm_array[j][0]){
				a =false;
				break;
				}
			}
		if(a){
			cancel_array.push(dm_array_tmp[i]);
			}
		}
		if(cancel_array.length){
			var task_code = '<div style="display:none;"></div>';
			for(var x=0; x<cancel_array.length; x++){
				if(document.getElementById(cancel_array[x][0])){
					$j(task_code).replaceAll("#"+cancel_array[x][0]);
					if(cancel_array[x][0] == dm[0])
					dm.length = 0;
				}
				for(var m=0; m<ProgressBar_array.length; m++){
					if(cancel_array[x][0] == ProgressBar_array[m].stateId.substring(4))
					ProgressBar_array.splice(m,1);
					}
			}
			dm_num = dm_array.length;
			$j("#transfers").html(dm_num);
			updateCtrlIcon();
			ajaxRequest = true;
		}
	} 

function showDMList(data)
{
	eval("dm_array = [" + data + "]");
	//alert(dm_array[0]); 
	if(dm_array_tmp.length){
		backstage_cancel();
		}
	eval("dm_array_tmp = [" + data + "]");
	if(sorted_by == "All"){
		}
	else{
		var array = new Array();
		for(var i =0;i<dm_array.length;i++){
			if(dm_array[i][4] == sorted_by){
				array.push(dm_array[i]);
				}
			}
		dm_array = array.concat();
		if(dm_array_tmp.length){
		backstage_cancel();
		}
		}
//	var old_length = dm_array.length;
	//if (dm_num != dm_array.length && dm_num == 0)
	if (dm_num == 0 || old_sorted_by2 != old_sorted_by)
	{
		create_task();
		dm_num = dm_array.length;
		//old_sorted_by = sorted_by;
		update_task();
	}
	else
	{	
		//update_task();
		//if(dm.length){
			for(var i = 0; i <dm_array.length; i++){
				if(dm[0]==dm_array[i][0]){
				dm = dm_array[i];
				break;
				}
			}   //update dm
			update_task();
		//}
	}
//	change_bgcolor();
}

function initial_Refresh_time(data){
	var initial_array = new Array();
	eval("initial_array="+data);
	//document.getElementById("homeAddress").href = "http://"+initial_array[10];
	if(location.host.split(":")[0] != initial_array[10]){
		if(initial_array[12] == 1){
				//document.getElementById("helpAddress").href = "http://"+ location.host.split(":")[0] +":8081/help.asp";
				}
		else if(initial_array[12] == 0){
			//document.getElementById("helpAddress").href = "http://"+ location.host.split(":")[0] +":8081/help.asp";
			}
		document.getElementById("handToPC").href = "http://"+ location.host.split(":")[0] +":8081/task.asp";
	}
	else{
				//document.getElementById("helpAddress").href = "http://"+ location.host +"/help.asp";
				document.getElementById("handToPC").href = "http://"+ location.host +"/task.asp";
	}
	
	//$j("#homeAddress").attr("href",initial_array[10]);
	if(initial_array.length)
	Refresh_time = parseInt(initial_array[7])*1000;
	TASK_INT = setInterval("Ajax_Get_DM_Status();",Refresh_time);
	}

function showTask(){
	Ajax_Get_DM_Status();
	}








