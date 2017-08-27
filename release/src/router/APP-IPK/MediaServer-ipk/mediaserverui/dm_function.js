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
var isLogShow = false;      	//if taskLog is not show,isLogShow=false else isLogShow=true
//var old_taskType= "";       	//old selected task's type 
var taskLogShowType = 0;    	//"0"is General /"1" is transfer/ "2" is File /"3" is Log
var sorted_by = "All";          //used for show desire tasks 
var old_sorted_by = "All";
//var old_sorted_by1 = "All";         //used for rember the old desire tasks
var old_sorted_by2 = "All";
//var taskid = "";
var WH_INT = 0;


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

/*function Ajax_Get_sort_inphase(){
	ajaxRequest = false;
	var t;
	old_sorted_by1 = sorted_by;
	var url = "ms_print_status.cgi" + "?action_mode=" +sorted_by+"&t="+Math.random();
	$j.ajax({url: url,
			async: false,
			success: function(data){showSortList(data)},
			complete: function(XMLHttpRequest, textStatus){ajaxRequest = true;}
			});
	
	}
*/

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

function showPanel() {
	 WH_INT = setInterval("getWH();",1000);
   	 $j("#DM_mask").fadeIn(1000);
     $j("#panel_add").show(1000);
	 //$j("#FTP_usb_dm_url").attr("value","ftp://");
	 var task_code = '<span style="margin-right:15px;margin-left:15px;"><b>'+multiLanguage_array[multi_INT][18]+'</b></span>\n';
	 task_code += '<input type="file" id="open_usb_dm_url" value="" name="filename" /><br /><br />\n';
	 $("open_usb_dm_url_div").innerHTML = task_code;
	 $j("#HTTP_usb_dm_url").attr("value","");
	 //$j("#open_usb_dm_url").attr("value","");
	 //alert(document.getElementById("open_usb_dm_url").value);
	 //$j("#magnet_usb_dm_url").attr("value","magnet:?");
	 //icon_on("icon_"+_item);
}

function hidePanel() {
	clearInterval(WH_INT);
	$j("#DM_mask").fadeOut('fast');
	$j("#panel_add").hide('fast');
	//$j("#User_name").attr("value","");
	//$j("#Password").attr("value","");
	//$j("#FTP_usb_dm_url").attr("value","");
	$j("#HTTP_usb_dm_url").attr("value","");
	var task_code = '<div id="open_usb_dm_url_div" style="display:block; float:left;"></div>\n';
	$j(task_code).replaceAll("#open_usb_dm_url_div");
	//$j("#open_usb_dm_url").attr("value","");
	//$j("#magnet_usb_dm_url").attr("value","");
	//FtpUrl = "";
	//save_user_pass = "";
	}

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
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3],"Now is not within the set time !");
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
	if(document.getElementById("open_usb_dm_url").value == "" && document.getElementById("HTTP_usb_dm_url").value == "")
		{
			Ext.MessageBox.alert(multiLanguage_array[multi_INT][3],multiLanguage_array[multi_INT][6]);
			}
	//http and ftp add task
	else if(document.getElementById("open_usb_dm_url").value == "" && document.getElementById("HTTP_usb_dm_url").value != ""){
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
		hidePanel();
		/*if (downloadtype == 1){
			url += "?action_mode=" + action_mode;
			url += "&download_type=" + downloadtype; 
			url += "&usb_dm_url=" + encodeURIComponent(usb_dm_url)+"&t="+Math.random();
			$j.get(url,function(data){response_dm_add(data);});
			}
		else if(downloadtype == 2){
			url += "?action_mode=" + action_mode;
			url += "&download_type=" + downloadtype; 
			url += "&usb_dm_url=" + encodeURIComponent(usb_dm_url)+"&t="+Math.random();
			$j.get(url,function(data){response_dm_add(data);});
			}
		else if(downloadtype == 3){
			alert("bt");
			url += "?action_mode=" + action_mode;
			url += "&download_type=" + downloadtype; 
			url += "&usb_dm_url=" + encodeURIComponent(usb_dm_url)+"&t="+Math.random();
			$j.get(url,function(data){response_dm_add(data);});
			}
		else if(downloadtype == 0){
			Ext.MessageBox.alert("Notice!","Please enter the right URL!");
			}*/
		}
		
		//bt and nzb add task
		else if (document.getElementById("open_usb_dm_url").value != ""){
			var usb_dm_url = $j("#open_usb_dm_url").attr("value");
			//var url = "ms_uploadbt.cgi";
			var downloadtype = check_open_url(usb_dm_url); //http is 1, ftp is 2, BT is 3 ,NZB is 4 ,false is 0
			if (downloadtype == 3){
			//url += "?download_type=" + downloadtype; 
			//url += "&usb_dm_url=" + encodeURIComponent(usb_dm_url);
			//$j.get(url,function(data){hidePanel();response_dm_add(data);});
			
			$j("#open_a_file_form").submit();    //缺少回调函数   添加任务table时需要
			hidePanel();
			}
			else if(downloadtype == 4){
			//url += "?download_type=" + downloadtype; 
			//url += "&usb_dm_url=" + encodeURIComponent(usb_dm_url);
			//$j.get(url,function(data){hidePanel();response_dm_add(data);});
			
			$j("#open_a_file_form").submit();     //缺少回调函数   添加任务table时需要
			hidePanel();
			}
			else if(downloadtype == 0){
			Ext.MessageBox.alert(multiLanguage_array[multi_INT][3],multiLanguage_array[multi_INT][7]);
			}
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

/*function saveFtpUrl(){
	FtpUrl = $j("#FTP_usb_dm_url").attr("value");
	}
function getFtpUrl(){
	var user = $j("#User_name").attr("value");
	var pass = $j("#Password").attr("value");
	var user_pass = "";
	var len1 = FtpUrl.length;
	var foot = "";
	var url = "";
	
	if(user != "" && pass != "")
	user_pass = user + ":" + pass + "@";
	else if(user != "" && pass == "")
	user_pass = user + "@";
	else
	user_pass = "";
	var len2 =save_user_pass.length;
		foot = FtpUrl.substr(6+len2,len1-6-len2);
		url = "ftp://" + user_pass + foot;
		$j("#FTP_usb_dm_url").attr("value",url);
		save_user_pass = user_pass;

}
*/

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
		download = dm_array[i][7];
		upload = dm_array[i][8];
		peers = dm_array[i][9];
	progressTxt = (parseFloat(size)*progress).toFixed(2) + size.substr(size.length-3,3);
	var j = dm_array[i][0];
	var task_code = '';
	var task_code2 = '';
	if(!isLogShow){
	task_code +='<table id="'+j+'" class="taskLongWidth" style="border-bottom:#000 solid 1px; table-layout:fixed;" onclick="selectedTask(this.id);">\n';}
	else
	{
		task_code +='<table id="'+j+'" class="taskShortWidth" style="border-bottom:#000 solid 1px; table-layout:fixed;" onclick="selectedTask(this.id);">\n';
		}
		task_code +='<tr><td colspan="9" style="word-wrap:break-word;word-break:break-all;overflow:hidden;"><span style="font-weight:bold; font-size:16px;" id="filename'+j+'">'+filename+'</span>\n';
		task_code +='<a href="javascript:Show_Log();"><img id="showicon'+j+'" src="images/icon/Ino2Icon.png" width="15" height="15" onmousedown="changeshowicon(this.id);" onmouseup="returnshowicon(this.id);" title="Detail information" /></a></td></tr>\n';
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
		task_code +='<a id="showUP_n5'+j+'" href="javascript:show_diskfull_info();" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>Diskfull</span></a>\n';
		task_code +='<span id="status'+j+'">'+status+'</span></td>\n';
		if(status!="notbegin"){
		if(type!="HTTP"&&type!="FTP"){
		task_code +='<td id="peerstd'+j+'" colspan="2"><span id="peers'+j+'">'+peers+'</span> peers</td>\n';
		}
		else{
		task_code +='<td colspan="2"></td>\n';
		}
		task_code +='<td id="downloadtd'+j+'" align="right" colspan="2" id="downloadplace'+j+'"><img width="13" height="13" src="images/icon/DownArrowIcon.png" /><span id="download'+j+'">'+download+'</span></td>\n';
		if(type!="HTTP"&&type!="FTP"&&type!="NZB"){
		task_code +='<td colspan="2" id="uploadtd'+j+'"><img width="13" height="13" src="images/icon/UpArrowIcon.png" /><span id="upload'+j+'">'+upload+'</span></td>\n';
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
	var task_code = '<div style="display:none;"></div>';
	$j(task_code).replaceAll("#"+dm_tmp[0]);
	for(var i=0; i<ProgressBar_array.length; i++){
		if(dm_tmp[0] == ProgressBar_array[i].stateId.substring(4))
		ProgressBar_array.splice(i,1);
		}
	dm_num = dm_array.length;
	$j("#transfers").html(dm_num);
	if(isLogShow)
	select_taskType();
	if(dm_num == 0){
		$j("#taskLog_show").hide();
		isLogShow = false;
		}
	updateCtrlIcon();
	ajaxRequest = true;
	}
	
function response_dm_cancel(data){
	if(data.search(/ACK_SUCESS/)>=0)
	{
		//pidname = "";	
		//alert("ACK_SUCESS");
		Ajax_Get_DM_Status_inphase();
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
		Ajax_Get_DM_Status();
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
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3],"File size is larger than the space left on the partition");
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



//update taskLogIcon NZB and BT will have 4 icons
function update_taskLogIcon(){
	if(dm[5] == "BT" || dm[5] == "NZB"){
		//$j("#inspector_tab_peers").children("a").attr("href","javascript:select_taskType_links()");
		//document.getElementById("TrackerInformationIcon").disabled = false;
		$j("#inspector_tab_peers").show();
		//$j("#inspector_tab_files").children("a").attr("href","javascript:select_taskType_included()");
		}
	else{
		//$j("#inspector_tab_peers").children("a").removeAttr("href");
		//document.getElementById("TrackerInformationIcon").disabled = true;
		$j("#inspector_tab_peers").hide();
		//$j("#inspector_tab_files").children("a").removeAttr("href");
		//$j("#inspector_tab_peers").removeClass("selected");
		//$j("#inspector_tab_files").removeClass("selected");
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
	var Diskfull = "Diskfull"
	//var com = false;
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
			if(dm_array[i][4] == Paused || dm_array[i][4] == Diskfull){
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
			$j("#icon_del").children("a").attr("href","javascript:DM_Ctrl('cancel');");
			if(dm[4] == Downloading || dm[4] == Seeding){
				// the selected task's status is download
				$j("#icon_pause").children("a").attr("href","javascript:DM_Ctrl('paused');");
				$j("#icon_resume").children("a").removeAttr("href");
				}
				// the selected task's status is pause || error || stoped
			else if(dm[4] == Paused || dm[4] == Error || dm[4] == Stop || dm[4] == Diskfull){
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
	
function update_BT_general(){
	if(document.getElementById("taskLog_Pieces") && $j("#taskLog_Pieces").html()!= dm_Log[1])
			$j("#taskLog_Pieces").html(dm_Log[1]);
	if(document.getElementById("taskLog_Hash") && $j("#taskLog_Hash").html()!= dm_Log[2])
			$j("#taskLog_Hash").html(dm_Log[2]);
	if(document.getElementById("taskLog_Secure") && $j("#taskLog_Secure").html()!= dm_Log[3])
			$j("#taskLog_Secure").html(dm_Log[3]);
	if(document.getElementById("taskLog_Comment") && $j("#taskLog_Comment").html()!= dm_Log[4])
			$j("#taskLog_Comment").html(dm_Log[4]);
	if(document.getElementById("taskLog_Creator") && $j("#taskLog_Creator").html()!= dm_Log[5])
			$j("#taskLog_Creator").html(dm_Log[5]);
	if(document.getElementById("taskLog_Date") && $j("#taskLog_Date").html()!= dm_Log[6])
			$j("#taskLog_Date").html(dm_Log[6]);
	if(document.getElementById("taskLog_Download_Dir") && $j("#taskLog_Download_Dir").html()!= dm_Log[7])
			$j("#taskLog_Download_Dir").html(dm_Log[7]);
	}

function update_FHN_general(){
	if(document.getElementById("taskLog_FHN_Destination") && $j("#taskLog_FHN_Destination").html()!= dm_Log[0])
			$j("#taskLog_FHN_Destination").html(dm_Log[0]);
	if(document.getElementById("taskLog_FHN_Filesize") && $j("#taskLog_FHN_Filesize").html()!= dm_Log[1])
			$j("#taskLog_FHN_Filesize").html(dm_Log[1]);
	if(document.getElementById("taskLog_FHN_Createdtime") && $j("#taskLog_FHN_Createdtime").html()!= dm_Log[2])
			$j("#taskLog_FHN_Createdtime").html(dm_Log[2]);
	if(document.getElementById("taskLog_FHN_URL") && $j("#taskLog_FHN_URL").html()!= dm_Log[3])
			$j("#taskLog_FHN_URL").html(dm_Log[3]);
	//if(document.getElementById("taskLog_FHN_Username") && $j("#taskLog_FHN_Username").html()!= dm_Log[4])
			//$j("#taskLog_FHN_Username").html(dm_Log[4]);
	}
	
function update_BT_transfer(){
		if(document.getElementById("taskLog_Progress") && $j("#taskLog_Progress").html()!= dm_Log[0])
			$j("#taskLog_Progress").html(dm_Log[0]);
		if(document.getElementById("taskLog_Size") && $j("#taskLog_Size").html()!= dm_Log[1])
			$j("#taskLog_Size").html(dm_Log[1]);
		if(document.getElementById("taskLog_Status") && $j("#taskLog_Status").html()!= dm_Log[2])
			$j("#taskLog_Status").html(dm_Log[2]);	
		if(document.getElementById("taskLog_Availability") && $j("#taskLog_Availability").html()!= dm_Log[3])
			$j("#taskLog_Availability").html(dm_Log[3]);	
		if(document.getElementById("taskLog_Downloaded") && $j("#taskLog_Downloaded").html()!= dm_Log[4])
			$j("#taskLog_Downloaded").html(dm_Log[4]);
		if(document.getElementById("taskLog_Uploaded") && $j("#taskLog_Uploaded").html()!= dm_Log[5])
			$j("#taskLog_Uploaded").html(dm_Log[5]);	
		if(document.getElementById("taskLog_DL_Speed") && $j("#taskLog_DL_Speed").html()!= dm_Log[6])
			$j("#taskLog_DL_Speed").html(dm_Log[6]);	
		if(document.getElementById("taskLog_UL_Speed") && $j("#taskLog_UL_Speed").html()!= dm_Log[7])
			$j("#taskLog_UL_Speed").html(dm_Log[7]);
		if(document.getElementById("taskLog_Error") && $j("#taskLog_Error").html()!= dm_Log[8])
			$j("#taskLog_Error").html(dm_Log[8]);
		if(document.getElementById("taskLog_Ratio") && $j("#taskLog_Ratio").html()!= dm_Log[9])
			$j("#taskLog_Ratio").html(dm_Log[9]);
		if(document.getElementById("taskLog_UL_To") && $j("#taskLog_UL_To").html()!= dm_Log[10])
			$j("#taskLog_UL_To").html(dm_Log[10]);
		if(document.getElementById("taskLog_DL_From") && $j("#taskLog_DL_From").html()!= dm_Log[11])
			$j("#taskLog_DL_From").html(dm_Log[11]);
		
	}
	
function update_FHN_transfer(){
		if(document.getElementById("taskLog_FHN_Status") && $j("#taskLog_FHN_Status").html()!= dm_Log[0])
			$j("#taskLog_FHN_Status").html(dm_Log[0]);
		if(document.getElementById("taskLog_FHN_Transferred") && $j("#taskLog_FHN_Transferred").html()!= dm_Log[1])
			$j("#taskLog_FHN_Transferred").html(dm_Log[1]);
		if(document.getElementById("taskLog_FHN_Progress") && $j("#taskLog_FHN_Progress").html()!= dm_Log[2])
			$j("#taskLog_FHN_Progress").html(dm_Log[2]);
		if(document.getElementById("taskLog_FHN_Speed") && $j("#taskLog_FHN_Speed").html()!= dm_Log[3])
			$j("#taskLog_FHN_Speed").html(dm_Log[3]);
		if(document.getElementById("taskLog_FHN_StartTime") && $j("#taskLog_FHN_StartTime").html()!= dm_Log[4])
			$j("#taskLog_FHN_StartTime").html(dm_Log[4]);
		if(document.getElementById("taskLog_FHN_TimeLeft") && $j("#taskLog_FHN_TimeLeft").html()!= dm_Log[5])
			$j("#taskLog_FHN_TimeLeft").html(dm_Log[5]);
	}
	
function update_BT_links(){
	//假设links的数目不会变化
	for(var i=0; i<dm_Log.length; i++){
		if(!document.getElementById("taskLog_linkName"+i))
		create_linksInfo();
		if(document.getElementById("taskLog_linkName"+i) && $j("#taskLog_linkName"+i).html()!= dm_Log[i][0])
			$j("#taskLog_linkName"+i).html(dm_Log[i][0]);
		if(document.getElementById("taskLog_lastAnnounce"+i) && $j("#taskLog_lastAnnounce"+i).html()!= dm_Log[i][1])
			$j("#taskLog_lastAnnounce"+i).html(dm_Log[i][1]);
		if(document.getElementById("taskLog_Seeders"+i) && $j("#taskLog_Seeders"+i).html()!= dm_Log[i][2])
			$j("#taskLog_Seeders"+i).html(dm_Log[i][2]);
		if(document.getElementById("taskLog_Leechers"+i) && $j("#taskLog_Leechers"+i).html()!= dm_Log[i][3])
			$j("#taskLog_Leechers"+i).html(dm_Log[i][3]);
		if(document.getElementById("taskLog_Downloads"+i) && $j("#taskLog_Downloads"+i).html()!= dm_Log[i][4])
			$j("#taskLog_Downloads"+i).html(dm_Log[i][4]);
		if(document.getElementById("taskLog_lastScrape"+i) && $j("#taskLog_lastScrape"+i).html()!= dm_Log[i][5])
			$j("#taskLog_lastScrape"+i).html(dm_Log[i][5]);
		}
	}

function update_BT_included(){
	for(var i=0; i<dm_Log.length; i++){
		if(!document.getElementById("BTincludedFile"+i))
		create_includedInfo();
		if(document.getElementById("BTincludedFile"+i) && $j("#BTincludedFile"+i).html()!= dm_Log[i][0])
			$j("#BTincludedFile"+i).html(dm_Log[i][0]);
		if(document.getElementById("BTincludedFile_progress"+i) && $j("#BTincludedFile_progress"+i).html()!= dm_Log[i][1])
			$j("#BTincludedFile_progress"+i).html(dm_Log[i][1]);
		if(document.getElementById("BTincludedFile_downloaded"+i) && $j("#BTincludedFile_downloaded"+i).html()!= dm_Log[i][2])
			$j("#BTincludedFile_downloaded"+i).html(dm_Log[i][2]);
		if(document.getElementById("BTincludedFile_size"+i) && $j("#BTincludedFile_size"+i).html()!= dm_Log[i][3])
			$j("#BTincludedFile_size"+i).html(dm_Log[i][3]);
		}
	}

function update_NZB_Log(){
	for(var i=0; i<dm_Log.length; i++){
		if(!document.getElementById("NZBLogName"+i))
		create_includedInfo();
		if(document.getElementById("NZBLogName"+i) && $j("#NZBLogName"+i).html()!= dm_Log[i][0])
			$j("#NZBLogName"+i).html(dm_Log[i][0]);
		if(document.getElementById("NZBLogSize"+i) && $j("#NZBLogSize"+i).html()!= dm_Log[i][2])
			$j("#NZBLogSize"+i).html(dm_Log[i][2]);
		if(document.getElementById("NZBLogProgress"+i) && $j("#NZBLogProgress"+i).html()!= dm_Log[i][1])
			$j("#NZBLogProgress"+i).html(dm_Log[i][1]);
		}
	}
	
function show_dm_Log(data){
	dm_Log.length = 0;
	if(data != null && data != ""){
		if(taskLogShowType == 0 || taskLogShowType ==1)
	eval("dm_Log=" + data );
		else if(taskLogShowType == 2 || taskLogShowType ==3)
	eval("dm_Log=[" + data +"]");
	if(taskLogShowType == 0 && (dm[5] == "BT" || dm[5] == "ED2K")){
		update_BT_general();
		}
	else if(taskLogShowType == 0 && (dm[5] == "HTTP" || dm[5] == "NZB" ||dm[5] == "FTP")){
		update_FHN_general();
		}
	else if(taskLogShowType == 1 && (dm[5] == "BT" || dm[5] == "ED2K")){
		update_BT_transfer();
		}
	else if(taskLogShowType == 1 && (dm[5] == "HTTP" || dm[5] == "NZB" ||dm[5] == "FTP")){
		update_FHN_transfer();
		}
	else if(taskLogShowType == 2 && dm[5] == "BT"){
		update_BT_links();
		}
	else if(taskLogShowType == 3 && dm[5] == "BT"){
		update_BT_included();
		}
	else if(taskLogShowType ==3 && dm[5] == "NZB"){
		update_NZB_Log();
		}
	else if(taskLogShowType ==2 && dm[5] == "NZB"){
		create_NZBFileInfo();
		}
	}
	//else
	//Get_dm_Log();
}



//dm_Log is the selected task's desired taskLogInfo
function Get_dm_Log(){
	if(dm.length && isLogShow){
		dm_Log.length = 0;
		var t;
		var url = "ms_print_status.cgi";
		var action_mode = "show_single_task";
		url += "?action_mode=" + action_mode;
		url += "&task_id=" +dm[0];
		ajaxRequest = false;
		if(taskLogShowType == 0){
			url += "&logTab=" +taskLogShowType+"&download_type="+taskLogShowType+"&t="+Math.random();
			//$j.get(url,function(data){show_dm_Log(data);});
			$j.ajax({
					url: url,
					async: true,
					success: function(data){show_dm_Log(data)},
					complete: function(XMLHttpRequest, textStatus){ajaxRequest = true;}
					//error: function(XMLHttpRequest, textStatus, errorThrown){Update_dm_Log();}
					})
			}
		else if(taskLogShowType == 1){
			url += "&logTab=" +taskLogShowType+"&download_type="+taskLogShowType+"&t="+Math.random();
			//$j.get(url,function(data){show_dm_Log(data);});
			$j.ajax({
					url: url,
					async: true,
					success: function(data){show_dm_Log(data)},
					complete: function(XMLHttpRequest, textStatus){ajaxRequest = true;}
					//error: function(XMLHttpRequest, textStatus, errorThrown){Update_dm_Log();}
					})
			}
		else if(taskLogShowType == 2){
			url += "&logTab=" +taskLogShowType+"&download_type="+taskLogShowType+"&t="+Math.random();
			//$j.get(url,function(data){show_dm_Log(data);});
			$j.ajax({
					url: url,
					async: true,
					success: function(data){show_dm_Log(data)},
					complete: function(XMLHttpRequest, textStatus){ajaxRequest = true;}
					//error: function(XMLHttpRequest, textStatus, errorThrown){Update_dm_Log();}
					})
			}
		else if(taskLogShowType == 3){
			url += "&logTab=" +taskLogShowType+"&download_type="+taskLogShowType+"&t="+Math.random();
			//$j.get(url,function(data){show_dm_Log(data);});
			$j.ajax({
					url: url,
					async: true,
					success: function(data){show_dm_Log(data)},
					complete: function(XMLHttpRequest, textStatus){ajaxRequest = true;}
					//error: function(XMLHttpRequest, textStatus, errorThrown){Update_dm_Log();}
					})
			}
			}
	}
	
function Update_dm_Log(){
	if(isLogShow && dm.length && ajaxRequest){
		dm_Log.length = 0;
		var t;
		var url = "ms_print_status.cgi";
		var action_mode = "show_single_task";
		url += "?action_mode=" + action_mode;
		url += "&task_id=" +dm[0];
		if(taskLogShowType == 0){
			url += "&logTab=" +taskLogShowType+"&download_type="+taskLogShowType+"&t="+Math.random();
			$j.get(url,function(data){show_dm_Log(data);});
			}
		else if(taskLogShowType == 1){
			url += "&logTab=" +taskLogShowType+"&download_type="+taskLogShowType+"&t="+Math.random();
			$j.get(url,function(data){show_dm_Log(data);});
			}
		else if(taskLogShowType == 2 && dm[4] == "BT"){
			url += "&logTab=" +taskLogShowType+"&download_type="+taskLogShowType+"&t="+Math.random();
			$j.get(url,function(data){show_dm_Log(data);});
			}
		else if(taskLogShowType == 3){
			url += "&logTab=" +taskLogShowType+"&download_type="+taskLogShowType+"&t="+Math.random();
			$j.get(url,function(data){show_dm_Log(data);});
			}
		}
	}
	
function select_taskType(){
			if(taskLogShowType == 0){
				select_taskType_general();
				//old_taskType = dm[5];
				}
			else if(taskLogShowType == 1){
				select_taskType_transfer();
				//old_taskType = dm[5];
				}
			else if(taskLogShowType == 2){
				select_taskType_links();
				//old_taskType = dm[5];
				}
			//else if(taskLogShowType == 3){
				//select_taskType_included();
				//old_taskType = dm[5];
				//}
	
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
		select_taskType();
		update_taskLogIcon();
		update_taskLogFilename();
}



//create taskLogFilename in the taskLog
function create_taskLogFilename(){
	
	var taskLog_code = "";
	
	taskLog_code += "<table style='width:246px;table-layout:fixed;'>\n";
	taskLog_code += "<tr><td id='taskLog_filename' valign='middle' style='font-size:20px;'>No Select File</td></tr></table>\n";
	
	$("taskLogFilename").innerHTML = taskLog_code;
	}
//update taskLogFilename in the taskLog
function update_taskLogFilename(){
	if(document.getElementById("taskLog_filename") && $j("#taskLog_filename").html()!= dm[1])
			$j("#taskLog_filename").html(dm[1]);
	}


//different task type will have different taskInfo 
function select_taskType_general(){
	taskLogShowType = 0;
	//$j(".inspector_tab").removeClass("selected");
	//$j("#inspector_tab_info").addClass("selected");
	if (dm[5] == "BT" || dm[5] == "ED2K")
	create_fileInfo();
	else if(dm[5] == "NZB")
	create_NZBGeneralInfo();
	else
	create_GeneralInfo();
	Get_dm_Log();
	}
//create BT fileInfo in the taskLog table for taskLogIcon 1
function create_fileInfo(){
	
	var taskLog_code = "";
	
	taskLog_code += "<table style='width:241px; table-layout:fixed;'>\n";
	taskLog_code += "<tr><td style='font-size:15px; font-weight:bold;' colspan='4'>\n";
	taskLog_code += "<span>"+multiLanguage_array[multi_INT][36]+"</span>\n";
	taskLog_code += "</td></tr>\n";
	if(dm[5] != "ED2K"){
	taskLog_code += "<tr><td><span style='margin-left:5px;'>"+multiLanguage_array[multi_INT][37]+"</span>\n";
	taskLog_code += "</td><td colspan='3'><span id='taskLog_Pieces'>-</span></td></tr>\n";
	}
	taskLog_code += "<tr><td valign='top'><span style='margin-left:5px;'>"+multiLanguage_array[multi_INT][38]+"</span>\n";
	taskLog_code += "</td><td colspan='3' style='word-wrap:break-word;overflow:hidden;'><span id='taskLog_Hash'>-</span></td></tr>\n";
	taskLog_code += "<tr><td><span style='margin-left:5px;'>"+multiLanguage_array[multi_INT][39]+"</span>\n";
	taskLog_code += "</td><td colspan='3'><span id='taskLog_Secure'>-</span></td></tr>\n";
	taskLog_code += "<tr><td colspan='2'><span style='margin-left:5px;'>"+multiLanguage_array[multi_INT][40]+"</span>\n";
	taskLog_code += "</td><td colspan='2'><span  id='taskLog_Comment'>-</span></td></tr>\n";
	if(dm[5] != "ED2K"){
	taskLog_code += "<tr><td style='border-bottom:1px #000 solid;' colspan='4'></td></tr>\n"
	taskLog_code += "<tr><td style='font-size:15px; font-weight:bold;' colspan='4'>\n";
	taskLog_code += "<span>"+multiLanguage_array[multi_INT][41]+"</span>\n";
	taskLog_code += "</td></tr>\n";
	taskLog_code += "<tr><td><span style='margin-left:5px;'>"+multiLanguage_array[multi_INT][42]+"</span>\n";
	taskLog_code += "</td><td colspan='3'><span id='taskLog_Creator'>-</span></td></tr>\n";
	taskLog_code += "<tr><td><span style='margin-left:5px;'>"+multiLanguage_array[multi_INT][43]+"</span>\n";
	taskLog_code += "</td><td colspan='3'><span id='taskLog_Date'>-</span></td></tr>\n";
	}
	taskLog_code += "<tr><td style='border-bottom:1px #000 solid;' colspan='4'></td></tr>\n"
	taskLog_code += "<tr><td style='font-size:15px; font-weight:bold;' colspan='4'>\n";
	taskLog_code += "<span>"+multiLanguage_array[multi_INT][44]+"</span>\n";
	taskLog_code += "</td></tr>\n";
	taskLog_code += "<tr><td colspan='2' valign='top'><span style='margin-left:5px;'>"+multiLanguage_array[multi_INT][45]+"</span>\n";
	taskLog_code += "</td><td colspan='2' style='word-wrap:break-word;overflow:hidden;'><span id='taskLog_Download_Dir'>-</span></td></tr>\n";
	taskLog_code += "<tr><td></td><td></td><td></td><td></td></tr></table>\n";
	$("taskLog").innerHTML = taskLog_code;
	}


//create HTTP FTP GeneralInfo in the taskLog table for taskLogIcon 1
function create_GeneralInfo(){
	var taskLog_code = "";
	
	taskLog_code += "<table style='width:241px; table-layout:fixed;'>\n";
	taskLog_code += "<tr><td colspan='2' valign='top'>\n"
	taskLog_code += "<span style='margin-left:5px;'>"+multiLanguage_array[multi_INT][46]+"</span>\n"
	taskLog_code += "</td><td colspan='3' style='word-break:break-all;' id='taskLog_FHN_Destination'>-</td></tr>\n"
	taskLog_code += "<tr><td valign='top' colspan='2'>\n";
	taskLog_code += "<span style='margin-left:5px;'>"+multiLanguage_array[multi_INT][47]+"</span>\n"
	taskLog_code += "</td><td colspan='3'><span id='taskLog_FHN_Createdtime'>-</span></td></tr>\n"
	taskLog_code += "<tr><td colspan='2'>\n";
	taskLog_code += "<span style='margin-left:5px;'>"+multiLanguage_array[multi_INT][48]+"</span>\n";
	taskLog_code += "</td><td colspan='3' id='taskLog_FHN_Filesize'>-</td></tr>\n"
	taskLog_code += "<tr><td valign='top' style='word-break:break-all;overflow:hidden;'>\n";
	taskLog_code += "<span style='margin-left:5px;'>"+multiLanguage_array[multi_INT][49]+"</span>\n";
	taskLog_code += "</td><td id='taskLog_FHN_URL' colspan='4'>-</td></tr>\n"
	taskLog_code += "<tr><td><td><td><td><td></td></td></td></td></td></tr></table>\n"
	
	$("taskLog").innerHTML = taskLog_code;
	}
//create NZB GeneralInfo in the taskLog table for taskLogIcon 1
function create_NZBGeneralInfo(){
	var taskLog_code = "";
	
	taskLog_code += "<table style='width:241px; table-layout:fixed;'>\n";
	taskLog_code += "<tr><td colspan='2' valign='top'>\n";
	taskLog_code += "<span style='margin-left:5px;'>"+multiLanguage_array[multi_INT][46]+"</span>\n";
	taskLog_code += "</td><td id='taskLog_FHN_Destination' colspan='3' style='word-wrap:break-word;overflow:hidden;'>-</td></tr>\n"
	taskLog_code += "<tr><td colspan='2'>\n"
	taskLog_code += "<span style='margin-left:5px;'>"+multiLanguage_array[multi_INT][48]+"</span>\n";
	taskLog_code += "</td><td id='taskLog_FHN_Filesize' colspan='3' style='word-wrap:break-word;overflow:hidden;'>-</td></tr>\n"
	taskLog_code += "<tr><td colspan='2' valign='top'>\n";
	taskLog_code += "<span style='margin-left:5px;'>"+multiLanguage_array[multi_INT][47]+"</span>\n";
	taskLog_code += "</td><td colsapn='3'><span id='taskLog_FHN_Createdtime'>-</span></td></tr>\n"
	taskLog_code += "<tr><td><td><td><td><td></td></td></td></td></td></tr></table>\n";
	
	$("taskLog").innerHTML = taskLog_code;
	}
	
//different task type will have different taskInfo 
function select_taskType_transfer(){
	taskLogShowType = 1;
	//$j(".inspector_tab").removeClass("selected");
	//$j("#inspector_tab_activity").addClass("selected");
	if (dm[5] == "BT" || dm[5] == "ED2K")
	create_downloadInfo();
	else
	create_TransferInfo();
	Get_dm_Log();
	}
// create BT downloadInfo in the taskLog table for taskLogIcon 2
function create_downloadInfo(){
	var taskLog_code = "";
	
	taskLog_code += "<table style='width:241px;table-layout:fixed;'>\n";
	
	taskLog_code += "<tr><td style='font-size:15px; font-weight:bold;'>\n";
	taskLog_code += "<span>"+multiLanguage_array[multi_INT][50]+"</span>\n";
	taskLog_code += "</td></tr>";
	taskLog_code += "<tr><td><span style='margin-left:5px;'>"+multiLanguage_array[multi_INT][51]+"</span>\n";
	taskLog_code += "<span style='margin-left:19px;' id='taskLog_Progress'>-</span></td></tr>\n";
	taskLog_code += "<tr><td>\n";
	taskLog_code += "<span style='margin-left:5px;'>"+multiLanguage_array[multi_INT][52]+"</span>\n";
	taskLog_code += "<span style='margin-left:45px;' id='taskLog_Size'>-</span></td></tr>\n";
	taskLog_code += "<tr><td>\n";
	taskLog_code += "<span style='margin-left:5px;'>"+multiLanguage_array[multi_INT][53]+"</span>\n";
	taskLog_code += "<span style='margin-left:33px;' id='taskLog_Status'>-</span></td></tr>\n";
	// availability no update
	taskLog_code += "<tr><td>\n";
	taskLog_code += "<span style='margin-left:5px;'>"+multiLanguage_array[multi_INT][54]+"</span>\n";
	taskLog_code += "<span style='margin-left:10px;' id='taskLog_Availability'>-</span></td></tr>\n";
	taskLog_code += "<tr><td>\n";
	taskLog_code += "<span style='margin-left:5px;'>"+multiLanguage_array[multi_INT][55]+"</span>\n";
	taskLog_code += "<span id='taskLog_Downloaded'>-</span></td></tr>\n";
	taskLog_code += "<tr><td>\n";
	taskLog_code += "<span style='margin-left:5px;'>"+multiLanguage_array[multi_INT][56]+"</span>\n";
	taskLog_code += "<span style='margin-left:17px;' id='taskLog_Uploaded'>-</span></td></tr>\n";
	taskLog_code += "<tr><td>\n";
	taskLog_code += "<span style='margin-left:5px;'>"+multiLanguage_array[multi_INT][57]+"</span>\n";
	taskLog_code += "<span style='margin-left:14px;' id='taskLog_DL_Speed'>-</span></td></tr>\n";
	taskLog_code += "<tr><td>\n";
	taskLog_code += "<span style='margin-left:5px;'>"+multiLanguage_array[multi_INT][58]+"</span>\n";
	taskLog_code += "<span style='margin-left:14px;' id='taskLog_UL_Speed'>-</span></td></tr>\n";
	taskLog_code += "<tr><td>\n";
	taskLog_code += "<span style='margin-left:5px;'>"+multiLanguage_array[multi_INT][59]+"</span>\n";
	taskLog_code += "<span style='margin-left:45px;' id='taskLog_Error'>-</span></td></tr>\n";
	// ratio no update
	taskLog_code += "<tr><td>\n";
	taskLog_code += "<span style='margin-left:5px;'>"+multiLanguage_array[multi_INT][60]+"</span>\n";
	taskLog_code += "<span style='margin-left:46px;' id='taskLog_Ratio'>-</span></td></tr>\n";
	taskLog_code += "<tr><td style='font-size:15px; font-weight:bold; border-top:#000 solid 1px;'>\n";
	taskLog_code += "<span>"+multiLanguage_array[multi_INT][61]+"</span>\n";
	taskLog_code += "</td></tr>\n";
	//UL To and DL From no update
	taskLog_code += "<tr><td>\n";
	taskLog_code += "<span style='margin-left:5px;'>"+multiLanguage_array[multi_INT][62]+"</span>\n";
	taskLog_code += "<span style='margin-left:40px;' id='taskLog_UL_To'>-</span></td></tr>\n";
	taskLog_code += "<tr><td>\n";
	taskLog_code += "<span style='margin-left:5px;'>"+multiLanguage_array[multi_INT][63]+"</span>\n";
	taskLog_code += "<span style='margin-left:23px;' id='taskLog_DL_From'>-</span></td></tr></table>\n";
	$("taskLog").innerHTML = taskLog_code;
	}
	
//create NZB HTTP FTP TransferInfo in the taskLog table for taskLogIcon 2
function create_TransferInfo(){
	//var progress = (parseFloat(dm_Log[2]))*100+"%";
	var taskLog_code ="";
	
	taskLog_code += "<table style='width:241px; table-layout:fixed;'>\n";
	taskLog_code += "<tr><td>\n";
	taskLog_code += "<span style='margin-left:5px;'>"+multiLanguage_array[multi_INT][53]+"</span>\n";
	taskLog_code += "<span id='taskLog_FHN_Status' style='margin-left:8px;'>-</span></td></tr>\n";
	taskLog_code += "<tr><td>\n";
	taskLog_code += "<span style='margin-left:5px;'>"+multiLanguage_array[multi_INT][55]+"</span>\n";
	taskLog_code += "<span id='taskLog_FHN_Transferred' style='margin-left:8px;'>-</span></td></tr>\n";
	taskLog_code += "<tr><td>\n";
	taskLog_code += "<span style='margin-left:5px;'>"+multiLanguage_array[multi_INT][51]+"</span>\n";
	taskLog_code += "<span id='taskLog_FHN_Progress' style='margin-left:8px;'>-</span></td></tr>\n";
	taskLog_code += "<tr><td>\n";
	taskLog_code += "<span style='margin-left:5px;'>"+multiLanguage_array[multi_INT][64]+"</span>\n";
	taskLog_code += "<span id='taskLog_FHN_Speed' style='margin-left:8px;'>-</span></td></tr>\n";
	taskLog_code += "<tr><td>\n";
	taskLog_code += "<span style='margin-left:5px;'>"+multiLanguage_array[multi_INT][65]+"</span>\n";
	taskLog_code += "<span id='taskLog_FHN_StartTime' style='margin-left:8px;'>-</span></td></tr>\n";
	taskLog_code += "<tr><td>\n";
	taskLog_code += "<span style='margin-left:5px;'>"+multiLanguage_array[multi_INT][66]+"</span>\n";
	taskLog_code += "<span id='taskLog_FHN_TimeLeft' style='margin-left:8px;'>-</span></td></tr></table>\n";
	$("taskLog").innerHTML = taskLog_code;
	}


//different task type will have different taskInfo 
function select_taskType_links(){
	taskLogShowType = 2;
	//$j(".inspector_tab").removeClass("selected");
	//$j("#inspector_tab_peers").addClass("selected");
	Get_dm_Log();
	if (dm[5] == "NZB")
	create_NZBFileInfo();
	else if(dm[5] == "BT")
	create_linksInfo();
	else{
		$("taskLog").innerHTML = "";
		}
	}	
//create NZB FileInfo in the taskLog table for taskLogIcon 3
function create_NZBFileInfo(){
	var taskLog_code = "";
	taskLog_code += "<table style='width:229px; table-layout:fixed;'>\n";
	taskLog_code += "<tr style='font-weight:bold; font-size:18px;'><th>\n";
	taskLog_code += "<span>"+multiLanguage_array[multi_INT][67]+"</span>\n";
	taskLog_code += "</th></tr>\n"
	for(var i = 0; i < dm_Log.length; i++){
		taskLog_code += "<tr><td style='word-wrap:break-word;overflow:hidden;'>"+dm_Log[i][0]+"</td></tr>\n";
		}
	taskLog_code += "<tr><td align='right'><input type='button' class='button_gen' value='Refresh' onclick='get_NZBfile_Log();' /></tr></td>\n"
	taskLog_code += "</table>\n";
	$("taskLog").innerHTML = taskLog_code;
	}
function get_NZBfile_Log(){
		dm_Log.length = 0;
		var url = "ms_print_status.cgi";
		var action_mode = "show_single_task";
		url += "?action_mode=" + action_mode;
		url += "&task_id=" +dm[0];
		url += "&logTab=" +taskLogShowType+"&download_type="+taskLogShowType+"&t="+Math.random();
		$j.get(url,function(data){show_dm_Log(data);});
		
	}
//create BT linksInfo in the taskLog table for taskLogIcon 3
function create_linksInfo(){
	
	var taskLog_code = "";
	for(var i=0; i<dm_Log.length; i++)
	{
	taskLog_code += "<table style='width:229px; table-layout:fixed;' border='0' cellspacing='0'>\n";
	taskLog_code += "<tr><td style='font-size:15px; font-weight:bold; width:128px;'>\n";
	taskLog_code += "<span>"+multiLanguage_array[multi_INT][68]+"</span>\n";
	taskLog_code += "</span>"+(i+1)+"<span></td></tr>\n";
	taskLog_code += "<tbody style='background-color:#999;'><tr>\n"
	taskLog_code += "<td style='word-wrap:break-word;overflow:hidden;'><span style='margin-left:7px;' id='taskLog_linkName"+i+" '>"+dm_Log[i][0]+"</span></td></tr>\n";
	taskLog_code += "<tr><td style='word-wrap:break-word;overflow:hidden;'><span style='margin-left:7px;' id='taskLog_lastAnnounce"+i+"'>"+dm_Log[i][1]+"<span></td></tr>\n";
	taskLog_code += "<tr><td style='word-wrap:break-word;overflow:hidden;'><span style='margin-left:7px;' id='taskLog_lastScrape"+i+"'>"+dm_Log[i][5]+"<span></td></tr>\n";
	taskLog_code += "<tr><td style='word-wrap:break-word;overflow:hidden;'><span style='margin-left:7px;' id='taskLog_Seeders"+i+"'>"+dm_Log[i][2]+"</span></td></tr>\n";
	taskLog_code += "<tr><td style='word-wrap:break-word;overflow:hidden;'><span style='margin-left:7px;' id='taskLog_Leechers"+i+"'>"+dm_Log[i][3]+"</span></td></tr>\n";
	taskLog_code += "<tr><td style='word-wrap:break-word;overflow:hidden;'><span style='margin-left:7px;' id='taskLog_Downloads"+i+"'>"+dm_Log[i][4]+"</span></td></tr>\n";
	taskLog_code += "</tbody></table>\n";
	}
	
	$("taskLog").innerHTML = taskLog_code;
	}

//different task type will have different taskInfo 
function select_taskType_included(){
	taskLogShowType = 3;
	//$j(".inspector_tab").removeClass("selected");
	//$j("#inspector_tab_files").addClass("selected");
	Get_dm_Log();
	if (dm[5] == "NZB")
	create_LogInfo();
	else if(dm[5] == "BT")
	create_includedInfo();
	else{
		$("taskLog").innerHTML = "";
		}
	}	
//create BT includedInfo in the taskLog table 4
function create_includedInfo(){
	var taskLog_code = "";
	taskLog_code += "<table style='width:229px; table-layout:fixed;' border='0' cellspacing='0'>\n";
	for(var i = 0; i<dm_Log.length; i++){
		//taskLog_code += "<tr><td rowspan='2' valign='middle' align='right'><input type='checkbox' name='BTincluded' value="+i+" /></td>\n";
		taskLog_code += "<tr><td style='width:7px;'></td><td style='word-wrap:break-word;overflow:hidden;'><span id='BTincludedFile"+i+"'>"+dm_Log[i][0]+"</span></td></tr>\n";
		taskLog_code += "<tr><td style='width:7px;'></td><td style='word-wrap:break-word;overflow:hidden;'>progress:<span style='margin-left:10px;' id='BTincludedFile_progress"+i+"'>"+dm_Log[i][1]+"</span></br>\n";
		taskLog_code += "<span id='BTincludedFile_downloaded"+i+"'>"+dm_Log[i][2]+"</span> of\n";
		taskLog_code += "<span id='BTincludedFile_size"+i+"'>"+dm_Log[i][3]+"</span> completed</br></br></td></tr>\n"
	}
	taskLog_code += "</table>\n";
	$("taskLog").innerHTML = taskLog_code;
	}
//create NZB LogInfo in the taskLog table 4
function create_LogInfo(){
	var taskLog_code ="";
	taskLog_code += "<table style='width:229px; table-layout:fixed;' border='0' cellspacing='0'>\n";
	for(var i = 0; i<dm_Log.length; i++){
		taskLog_code += "<tr><td>&nbsp;Name:<span style='margin-left:7px;word-break:break-all;' id='NZBLogName"+i+"'>"+dm_Log[i][0]+"</span><br />\n";
		taskLog_code += "&nbsp;Progress:<span style='margin-left:7px;' id='NZBLogProgress"+i+"'>"+dm_Log[i][1]+"</span><br />\n";
		taskLog_code += "&nbsp;Size:<span style='margin-left:7px;' id='NZBLogSize"+i+"'>"+dm_Log[i][2]+"</span></td></tr>";
		}
	taskLog_code += "</table>\n";
	$("taskLog").innerHTML = taskLog_code;
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

//双击显示logInfo
/*function db_Show_Log(){
	var dmid = dm[0];
	if(!isLogShow){
		    //alert("first"+old_dmid+isLogShow+old_dmid_tmp);
			$j("#"+dmid).siblings().removeClass("taskLongWidth");
			$j("#"+dmid).removeClass("taskLongWidth");
			$j("#"+dmid).siblings().addClass("taskShortWidth");
			$j("#"+dmid).addClass("taskShortWidth");
			$j("#taskLog_show").show();
			for(var i=0; i<dm_array.length; i++){
				var j=dm_array[i][0];
				if(document.getElementById("pbid" + j) )   //还需要添加判断条件--看Progress是否有变化
				{
					update_progress(ProgressBar_array[i],dm_array[i][2]);
				}
				}
			isLogShow = true;
	}
	}*/
	
//show and hide the logInfo
function Show_Log(){
	var dmid = dm[0];
	if(!isLogShow){
		    //alert("first"+old_dmid+isLogShow+old_dmid_tmp);
			$j("#"+dmid).siblings().removeClass("taskLongWidth");
			$j("#"+dmid).removeClass("taskLongWidth");
			$j("#"+dmid).siblings().addClass("taskShortWidth");
			$j("#"+dmid).addClass("taskShortWidth");
			$j("#taskLog_show").show();
			isLogShow = true;
			Get_dm_Log();
			create_progress();
			for(var i=0; i<dm_array.length; i++){
				var j=dm_array[i][0];
				if(document.getElementById("pbid" + j) )   //还需要添加判断条件--看Progress是否有变化
				{
					update_progress(ProgressBar_array[i],dm_array[i][2]);
				}
				}
	}
	else{
		if(old_dmid == old_dmid_tmp){
			//alert("second"+old_dmid+isLogShow+old_dmid_tmp);
			$j("#taskLog_show").hide();
			$j("#"+dmid).siblings().removeClass("taskShortWidth");
			$j("#"+dmid).removeClass("taskShortWidth");
			$j("#"+dmid).siblings().addClass("taskLongWidth");
			$j("#"+dmid).addClass("taskLongWidth");
			isLogShow = false;
			create_progress();
			for(var i=0; i<dm_array.length; i++){
				var j=dm_array[i][0];
				if(document.getElementById("pbid" + j) )   //还需要添加判断条件--看Progress是否有变化
				{
					update_progress(ProgressBar_array[i],dm_array[i][2]);
				}
				}	
		}
		else{
			//alert("third"+old_dmid+isLogShow+old_dmid_tmp);
			$j("#"+dmid).siblings().removeClass("taskLongWidth");
			$j("#"+dmid).removeClass("taskLongWidth");
			$j("#"+dmid).siblings().addClass("taskShortWidth");
			$j("#"+dmid).addClass("taskShortWidth");
			$j("#taskLog_show").show();
			isLogShow = true;
			create_progress();
			for(var i=0; i<dm_array.length; i++){
				var j=dm_array[i][0];
				if(document.getElementById("pbid" + j) )   //还需要添加判断条件--看Progress是否有变化
				{
					update_progress(ProgressBar_array[i],dm_array[i][2]);
				}
				}
		}
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
	
function show_diskfull_info(){
	Ext.MessageBox.alert(multiLanguage_array[multi_INT][3],"The disk is full,Please delete something in your disk then press the \"resume\" button!");
	}


function create_task(){
	//taskid = "";
	$j("#taskLog_show").hide();
	ProgressBar_array.length = 0;
	dm.length = 0;
	old_dmid = "";
	old_dmid_tmp = "";
	//old_taskType = "";
	isLogShow = false;
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
		download = dm_array[i][7];
		upload = dm_array[i][8];
		peers = dm_array[i][9];
		progressTxt = (parseFloat(size)*progress).toFixed(2) + size.substr(size.length-3,3);
		task_code +='<table id="'+j+'" class="taskLongWidth" style="border-bottom:#000 solid 1px; table-layout:fixed;" onclick="selectedTask(this.id);">\n';
		task_code +='<tr><td colspan="9" style="word-wrap:break-word;word-break:break-all;overflow:hidden;"><span style="font-weight:bold; font-size:16px;" id="filename'+j+'">'+filename+'</span>\n';
		task_code +='<a href="javascript:Show_Log();"><img id="showicon'+j+'" src="images/icon/Ino2Icon.png" width="15" height="15" onmousedown="changeshowicon(this.id);" onmouseup="returnshowicon(this.id);" title="Detail information" /></a></td></tr>\n';
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
		task_code +='<a id="showUP_n5'+j+'" href="javascript:show_diskfull_info();" style="display:none;"><img src="images/statusIcon/warning.png" alt="" /><span>Diskfull</span></a>\n';
		task_code +='<span id="status'+j+'">'+status+'</span></td>\n';
		if(status!="notbegin"){
		if(type!="HTTP"&&type!="FTP"){
		task_code +='<td colspan="2" id="peerstd'+j+'"><span id="peers'+j+'">'+peers+'</span> peers</td>\n';
		}
		else{
		task_code +='<td colspan="2"></td>\n';
		}
		task_code +='<td id="downloadtd'+j+'" align="right" colspan="2" id="downloadplace'+j+'"><img width="13" height="13" src="images/icon/DownArrowIcon.png" /><span id="download'+j+'">'+download+'</span></td>\n';
		if(type!="HTTP"&&type!="FTP"&&type!="NZB"){
		task_code +='<td colspan="2" id="uploadtd'+j+'"><img width="13" height="13" src="images/icon/UpArrowIcon.png" /><span id="upload'+j+'">'+upload+'</span></td>\n';
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
	$("TaskMain").innerHTML = task_code;
	$j("#transfers").html(dm_array.length);
		create_progress();
		
		//clear taskLog everytime wehn task num changed
		create_taskLogFilename();
		//select_taskType_general();
		updateCtrlIcon();
	
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
		 $j("#showUP_n5"+j).hide();
		 $j("#status" + j).html("");}
		else if((dm_array[i][4] == "ConnectFail" || dm_array[i][4] == "SoketRecvFail") && document.getElementById("showUP_n1" + j))
		{$j("#showUP_n1"+j).show();
		$j("#showUP"+j).hide();
		$j("#showUP_n2"+j).hide();
		$j("#showUP_n3"+j).hide();
		$j("#showUP_n4"+j).hide();
		$j("#showUP_n5"+j).hide();
		$j("#status" + j).html("");}
		else if(dm_array[i][4] == "SSLFail" && document.getElementById("showUP_n2" + j))
		{$j("#showUP_n2"+j).show();
		$j("#showUP"+j).hide();
		$j("#showUP_n1"+j).hide();
		$j("#showUP_n3"+j).hide();
		$j("#showUP_n4"+j).hide();
		$j("#showUP_n5"+j).hide();
		$j("#status" + j).html("");}
		else if(dm_array[i][4] == "AccountFail" && document.getElementById("showUP_n3" + j))
		{$j("#showUP_n3"+j).show();
		$j("#showUP"+j).hide();
		$j("#showUP_n1"+j).hide();
		$j("#showUP_n2"+j).hide();
		$j("#showUP_n4"+j).hide();
		$j("#showUP_n5"+j).hide();
		$j("#status" + j).html("");}
		else if(dm_array[i][4] == "NotCompleted" && document.getElementById("showUP_n4" + j)){
		$j("#showUP_n4"+j).show();
		$j("#showUP"+j).hide();
		$j("#showUP_n1"+j).hide();
		$j("#showUP_n2"+j).hide();
		$j("#showUP_n3"+j).hide();
		$j("#showUP_n5"+j).hide();
		$j("#status" + j).html("");
			}
		else if(dm_array[i][4] == "Diskfull" && document.getElementById("showUP_n5" + j)){
		$j("#showUP_n5"+j).show();
		$j("#showUP_n4"+j).hide();
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
		$j("#showUP_n5"+j).hide();
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
			if(dm_array[i][4] == "warning" || dm_array[i][4] == "ConnectFail" || dm_array[i][4] == "SoketRecvFail" || dm_array[i][4] == "SSLFail" || dm_array[i][4] == "AccountFail" || dm_array[i][4] == "NotCompleted" || dm_array[i][4] == "Diskfull")
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
		if(document.getElementById("download" + j) && $j("#download" + j).html()!= dm_array[i][7])
			$j("#download" + j).html(dm_array[i][7]);
		//upload
		if(document.getElementById("upload" + j) && $j("#upload" + j).html()!= dm_array[i][8])
			$j("#upload" + j).html(dm_array[i][8]);
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
			if(isLogShow)
			select_taskType();
			if(dm_num == 0){
				$j("#taskLog_show").hide();
				isLogShow = false;
				}
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
	eval("dm_array_tmp = [" + data + "]")
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

/*function get_Refresh_time(){
	var url = "ms_apply.cgi";
	var action_mode = "initial";
	var type = "General";
	url += "?action_mode=" + action_mode + "&download_type=" +type+ "&t=" +Math.random();
	$j.ajax({
			url:url,
			success:function(data){initial_Refresh_time(data)},
			error:function(XMLHttpRequest, textStatus, errorThrown){error_Refresh_time()}
			});
	}*/

function initial_Refresh_time(data){
	var initial_array = new Array();
	eval("initial_array="+data);
	//document.getElementById("homeAddress").href = "http://"+initial_array[10];
	if(location.host.split(":")[0] != initial_array[10]){
		if(initial_array[12] == 1){
				document.getElementById("homeAddress").href = "http://" + location.host.split(":")[0] + ":" + initial_array[11] + "/APP_Installation.asp";
				document.getElementById("helpAddress").href = "http://"+ location.host.split(":")[0] +":8081/help.asp";
				}
		else if(initial_array[12] == 0){
			document.getElementById("helpAddress").href = "http://"+ location.host.split(":")[0] +":8081/help.asp";
			document.getElementById("icon_home").title = "Open \"Enable Web Access from WAN\" From Router!";
			}
		if(document.getElementById("handToPhone"))
		document.getElementById("handToPhone").href = "http://"+ location.host.split(":")[0] +":8081/task_hand.asp";
	}
	else{
			document.getElementById("homeAddress").href = "http://" + location.host.split(":")[0] + "/APP_Installation.asp";
			document.getElementById("helpAddress").href = "http://"+ location.host +"/help.asp";
			if(document.getElementById("handToPhone"))
				document.getElementById("handToPhone").href = "http://"+ location.host +"/task_hand.asp";
	}
	
	//$j("#homeAddress").attr("href",initial_array[10]);
	if(initial_array.length)
	Refresh_time = parseInt(initial_array[7])*1000;
	setInterval("Ajax_Get_DM_Status();Update_dm_Log();",Refresh_time);
	}
/*function error_Refresh_time(){
	setInterval("Ajax_Get_DM_Status();Update_dm_Log();",Refresh_time);
	}*/
function UtilityFlag(){
	var flag;
	flag = location.search.split("flag=")[1];
		if(flag == 73)
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3],"File size is larger than the space left on the partition");
		else
		Ext.MessageBox.alert(multiLanguage_array[multi_INT][3],multiLanguage_array[multi_INT][flag]);
	}


function showTask(){
	Ajax_Get_DM_Status();
	}








