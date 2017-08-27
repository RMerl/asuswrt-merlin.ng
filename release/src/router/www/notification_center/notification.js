var NTDB_content = {};
var NTDB_info = {"get_nt_db":""};

var nc_setting_conf_t = decodeURIComponent('<% nvram_char_to_ascii("", "nc_setting_conf"); %>').replace(/&#62/g, ">").replace(/&#60/g, "<").split('<');
var aaa;
var notification = {
	stat: "off",
	flash: "off",
	clicking: 0,
	bell_num: 0,
	important_num: 0,
	notic_num: 0,
	TYPE_OF_RSV: 0,
	TYPE_OF_TURN_OFF: 1,
	TYPE_OF_IMPORTANT: 2,
	TYPE_OF_TIPS: 3,
	TYPE_OF_TOTAL: 4,
	ACTION_NOTIFY_NONE: 0,
	ACTION_NOTIFY_WEBUI: 1,
	ACTION_NOTIFY_EMAIL: 2,
	ACTION_NOTIFY_APP: 4,
	ACTION_NOTIFY_WEEKLY: 8,
	nc_web_app_enable: 0,
	nc_mail_enable: 0,
	nc_setting_conf: decodeURIComponent('<% nvram_char_to_ascii("", "nc_setting_conf"); %>').replace(/&#62/g, ">").replace(/&#60/g, "<").split('<'),

	find_nc_setting_type:function(eventid){
		for(var i=0; i<notification.nc_setting_conf.length; i++){
			var nc_setting_array = notification.nc_setting_conf[i].split(">");
			if(nc_setting_array[0] == eventid){
				return nc_setting_array[2];
			}
		}
		return '0';
	},

	update_NT_Center: function(){
		notification.updateNTDB_Content();
		notification.update_nt_action();
		notification.updateNTDB_Status();
	},

	updateNTDB_Content: function(){
		require(['/require/modules/makeRequest.js'], function(makeRequest) {
			makeRequest.start('/nt_content.json', notification.get_nt_content, notification.updateNTDB_Content);
		});
	},

	update_nc_setting_conf: function(){
		require(['/require/modules/makeRequest.js'], function(makeRequest) {
			makeRequest.post('/appGet.cgi','hook=nvram_get(nc_setting_conf)', notification.get_nc_setting_conf, notification.update_nc_setting_conf);
		});
	},

	get_nc_setting_conf: function(xhr){
		if(xhr.responseText.search("Main_Login.asp") !== -1) top.location.href = "<% abs_index_page(); %>";
		aaa = xhr;
		var response = JSON.parse(xhr.responseText);
		notification.nc_setting_conf = decodeURIComponent(response.nc_setting_conf).replace(/&#62/g, ">").replace(/&#60/g, "<").split('<');
	},

	get_nt_content: function(xhr){
		if(xhr.responseText.search("Main_Login.asp") !== -1) top.location.href = "<% abs_index_page(); %>";

		var response = JSON.parse(xhr.responseText);
		NTDB_content = response;
	},

	updateNTDB_Status: function(){
		require(['/require/modules/makeRequest.js'], function(makeRequest) {
			makeRequest.post('/appGet.cgi','hook=get_nt_db()', notification.refresh_ntinfo, notification.updateNTDB_Status);
		});
	},

	refresh_ntinfo: function(xhr){
		if(xhr.responseText.search("Main_Login.asp") !== -1) top.location.href = "<% abs_index_page(); %>";

		var response = JSON.parse(xhr.responseText);
		NTDB_info = response;

		notification.bell_num = 0;
		notification.important_num = 0;
		notification.notic_num = 0;
		for(var i=0; i<NTDB_info.get_nt_db.length; i++){
			if(NTDB_info.get_nt_db[i].status == 1 
				|| notification.nc_web_app_enable == 0 
				|| !notification.web_display(NTDB_info.get_nt_db[i].event_id.trim())
				|| notification.find_nc_setting_type(NTDB_info.get_nt_db[i].event_id.trim()) == notification.TYPE_OF_TURN_OFF
			) continue;
				notification.bell_num++;
				if(NTDB_info.get_nt_db[i].event_type == notification.TYPE_OF_TURN_OFF)
					notification.important_num++;
				else if(NTDB_info.get_nt_db[i].event_type == notification.TYPE_OF_IMPORTANT)
					notification.notic_num++;
		}

		if(notification.bell_num > 0)
			notification.stat = "on";
		else
			notification.stat = "off";
		notification.run();
	},

	update_nt_action: function(){
		require(['/require/modules/makeRequest.js'], function(makeRequest) {
			makeRequest.post('/appGet.cgi','hook=nvram_get(nc_web_app_enable)%3bnvram_get(nc_mail_enable)', notification.update_nt_action_check, notification.update_nt_action);
		});
	},

	update_nt_action_check: function(xhr){
		if(xhr.responseText.search("Main_Login.asp") !== -1) top.location.href = "<% abs_index_page(); %>";
		
		var response = JSON.parse(xhr.responseText);
		notification.nc_web_app_enable = response.nc_web_app_enable;
		notification.nc_mail_enable = response.nc_mail_enable;
	},

	update_bell_num: function(){

		if(notification.bell_num > 0){
			document.getElementById("noti_event_count").style.display = "";
			if(notification.bell_num > 99)
				document.getElementById("noti_event_num").innerHTML = "99+";
			else
				document.getElementById("noti_event_num").innerHTML = notification.bell_num;
		}else
			document.getElementById("noti_event_count").style.display = "none";
	},

	add_clcik_event: function(){

		if(notification.clicking == 1){
			document.body.addEventListener('click',notification.noti_clickListener, false);
		}
		else{
			document.body.removeEventListener('click', notification.noti_clickListener, false);
		}
	},

	noti_clickListener: function(event){
		if((event.srcElement.offsetParent != null &&
		   event.srcElement.offsetParent.id != 'notiDiv' &&
		   event.srcElement.offsetParent.id != 'notification_status_td') &&
		   !event.srcElement.offsetParent.id.indexOf("noti_event_type_") &&
		   event.srcElement.offsetParent.id != 'noti_circle_t' || event.srcElement.offsetParent == null){
			if(notification.clicking == 1){
				document.getElementById("notification_desc").innerHTML = "";
				notification.clicking = 0;
    			document.body.removeEventListener('click', notification.noti_clickListener, false);
			}
		}
	},

	add_click_noti_type_select: function(event, obj_id){
		if(event.srcElement.offsetParent != null &&
		   document.getElementById(obj_id) != null &&
		   (event.srcElement.offsetParent.id == 'notiDiv' || event.srcElement.offsetParent.id == 'noti_circle_t') &&
		   event.srcElement.id != obj_id)
		{
			document.getElementById(obj_id+'_option').style.display = "none";
			document.body.removeEventListener('click', notification.add_click_noti_type_select, false);
		}
	},

	noti_type_select: function(obj_id){
			
			if(document.getElementById(obj_id+'_option').style.display == "none"){
				document.getElementById(obj_id+'_option').style.display = "";
				document.body.addEventListener('click',function(){var evt = event;notification.add_click_noti_type_select(evt, obj_id)}, false);
			}else
			{
				document.getElementById(obj_id+'_option').style.display = "none";
			}
	},

	change_type:function(event_id, type, call_back){
		var nc_setting_conf_tmp = new Array();
		if(notification.find_nc_setting_type(event_id) != type){
			for(var i=0; i<notification.nc_setting_conf.length; i++){
				var nc_setting_array = notification.nc_setting_conf[i].split(">");
				if(nc_setting_array[0] == event_id){
					nc_setting_array[2] = type;
				}
				nc_setting_conf_tmp.push(nc_setting_array.join('>'));
			}
			notification.nc_setting_conf = nc_setting_conf_tmp;
			nc_setting_conf_tmp = nc_setting_conf_tmp.join('<');
			require(['/require/modules/makeRequest.js'], function(makeRequest) {
			makeRequest.post('/apply.cgi','nc_setting_conf='+nc_setting_conf_tmp+'&action_mode=apply', call_back, notification.change_type);
		});
		}
	},

	show_event: function(tab){
				var txt="";
				var show_event_num = 0;
				var first_line = false;
				//var nt_event_tmp = NTDB_info.get_nt_db;
				NTDB_info.get_nt_db.sort(sort_by('tstamp', true, parseInt));
				if(NTDB_info.get_nt_db.length != 0){
					for(var i=0; i<NTDB_info.get_nt_db.length; i++){
						var ntdb_obj = NTDB_info.get_nt_db[i];

						if(ntdb_obj.status == 1
						   || notification.nc_web_app_enable == 0
						   || !notification.web_display(ntdb_obj.event_id.trim())
						   || notification.find_nc_setting_type(ntdb_obj.event_id.trim()) == notification.TYPE_OF_TURN_OFF
						   //|| (ntdb_obj.event_id.trim() == "10010" || ntdb_obj.event_id.trim() == "10011") && !clientList[ntdb_obj.msg.macaddr])
						   ) continue;

						if(show_event_num > 4) 
							break;
						else
							show_event_num++;

						if(first_line == true)
							txt += '<div style="padding-top:6px"><div style="background-color:#333333;height:1px;width:95%;margin:0px auto;"></div></div>';
						else{
							txt += '<div style="padding-top:0px"></div>';
							first_line = true;
						}
						
			  			txt += '<div id="notiDiv_event_table_'+i+'" style="width:100%;display:table;padding:10px 0px 0px 10px;height:60px;" >';

			  			txt += '<div style="display:table-cell;padding-left:4px;height:40px;width:40px;position:absolute;" title="Change type">';
				  			if(notification.find_nc_setting_type(ntdb_obj.event_id.trim()) == notification.TYPE_OF_IMPORTANT)
				  				txt += '<div id="noti_circle_t" class="noti_circle_important">'
				  			else 	//tips
				  				txt += '<div id="noti_circle_t" class="noti_circle_tips">'
				  			txt += '<div id="noti_event_type_'+i+'" onclick="notification.noti_type_select(this.id)" style="cursor:pointer;position:absolute;background-repeat:no-repeat;background-position:6px 6px;height:37px;width:38px;background-size: 26px 25px;';
				  			txt += 'background-image: url(/images/New_ui/nt_center/nt_icon'+ NTDB_content[ntdb_obj.event_id.trim()].icon +'.svg);"></div>';
				  			txt += '</div>';
				  			txt += '<div id="noti_event_type_'+i+'_option" style="display:none;">';
				  			txt += '<div class="noti_type_triangle"></div>';
							txt += '<div class="noti_type_select_t">';
							txt += '<div style="font-size:16px;color:#AAAAAA;text-align:center;padding-top:8px;">Notice Type</div>';
							txt += '<div style="padding-top:2px"><div style="background-color:#333333;height:1px;width:95%;margin:0px auto;opacity:0.2;"></div></div>';
							txt += '<div style="height:30px;cursor: pointer;" onclick="notification.change_type('+ntdb_obj.event_id+',2,notification.update_read)"><div style="display:table-cell;"><div class="noti_type_select_important">';
							if(notification.find_nc_setting_type(ntdb_obj.event_id.trim()) == notification.TYPE_OF_IMPORTANT){
								txt += '<div class="noti_type_select_check"></div>';
							}
							txt += '</div></div><div class="noti_type_word">Important</div></div>';
							txt += '<div style="padding-bottom:3px;height:30px;cursor: pointer;" onclick="notification.change_type('+ntdb_obj.event_id+',3,notification.update_read)"><div style="display: table-cell;"><div class="noti_type_select_tips">';
							if(notification.find_nc_setting_type(ntdb_obj.event_id.trim()) == notification.TYPE_OF_TIPS){
								txt += '<div class="noti_type_select_check"></div>';
							}
							txt += '</div></div><div class="noti_type_word">Tips</div></div>';
							txt += '</div>'
						txt += '</div>';
			  			txt += '</div>';

			  			txt += '<div style="width:75%;display:table-cell;padding:3px">';
				  			txt += '<div class="notiDiv_item">'+NTDB_content[ntdb_obj.event_id.trim()].item+'</div>';
				  			txt += '<div>';

				  			txt += '<span class="notiDiv_desc">';
				  			if(ntdb_obj.event_id.trim() == "10010" || ntdb_obj.event_id.trim() == "10011"){
				  				if(clientList[ntdb_obj.msg.macaddr]){
					  				txt += clientList[ntdb_obj.msg.macaddr].name;
					  				txt +=' ( '+clientList[ntdb_obj.msg.macaddr].ip +' )';
				  				}
				  				else{
				  					txt += ntdb_obj.msg.macaddr;
				  					txt +=' ( offline )';
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
				  			txt += '<div id="noti_button_'+i+'" style="padding:15px 0px 3px 0px;display:table;">';
				  				//console.log("ntdb_obj.event_id.trim() = " + ntdb_obj.event_id.trim());
				  				//console.log("NTDB_content[ntdb_obj.event_id.trim()].buttom1 = " + NTDB_content[ntdb_obj.event_id.trim()].buttom1);
				  				if(NTDB_content[ntdb_obj.event_id.trim()].buttom1 != ""){
				  					txt += '<div style="padding-right:10px;">';
					  				txt += '<div class="noti_change_button" onclick="notification.read_one('+ntdb_obj.event_id+','+ntdb_obj.tstamp+','+ntdb_obj.event_type+');';
					  					if(ntdb_obj.event_id.trim() == "10010"){
					  						txt += 'notification.acl_block(\''+ntdb_obj.msg.ifname+'\',\''+ntdb_obj.msg.macaddr+'\');';
					  					}
					  					else if(NTDB_content[ntdb_obj.event_id.trim()].callback != ""){
					  					//if(NTDB_content[ntdb_obj.event_id.trim()].callback != ""){
											txt += 'location.href=\''+NTDB_content[ntdb_obj.event_id.trim()].callback+'\'';
					  					}
				  					txt += '">'+NTDB_content[ntdb_obj.event_id.trim()].buttom1+'</div>';
				  					txt +='</div>';
			  					}
								txt += '<div class="noti_skip_button" onclick="notification.read_one('+ntdb_obj.event_id.trim()+','+ntdb_obj.tstamp+','+ntdb_obj.event_type+')">'+NTDB_content[ntdb_obj.event_id.trim()].buttom2+'</div>';
								txt += '<div style="display:table-cell;padding-left:5px;vertical-align:middle;"><img src="/images/InternetScan.gif" id="loadingIcon_'+ntdb_obj.event_id.trim()+'" style="display:none"></div>';
				  			txt +='</div>';
			  			txt += '</div>';
						txt += '<div style="width:12%;display:table-cell;text-align: center;padding-right: 8px;">';
						txt += '<div class="noti_event_timestamp">'+notification.noti_event_time(ntdb_obj.tstamp)+'</div>';
						txt += '<div class="noti_event_off" title="Turn off notice" onclick="notification.change_type('+ntdb_obj.event_id+',1,notification.update_read)"></div>';

						txt += '</div>';
			  			txt += '</div>';			  					  			
					}
				}
				if(NTDB_info.get_nt_db.length == 0 || txt ==""){
					txt += '<div style="width:100%;display:table;height:160px;">';
					txt += '<div class="noti_mail"></div>';
					txt += '<div class="notiDiv_nonews_word">No News!</div>';
					txt += '</div>';
				}
				document.getElementById("show_event_table").innerHTML = txt;
	},

	acl_block: function(ifname, macaddr){
		console.log("acl_block");
		console.log("acl_block:macaddr = " + macaddr);
		console.log("acl_block:ifname = " + ifname);
		if(ifname == "eth1")
			var nc_wl_unit = 0;
		else if(ifname == "eth2")
			var nc_wl_unit = 1;
		wl_macmode = "deny";
		var wl_maclist_x = '<% nvram_get("wl_maclist_x"); %>'.replace(/&#60/g, "<");
		console.log("acl_block:wl_maclist_x = " + wl_maclist_x);
		if(wl_maclist_x.indexOf(macaddr) != -1 || wl_maclist_x == ""){
			wl_maclist_x = wl_maclist_x + '<' + macaddr;
			console.log("acl_block:wl_maclist_x111 = " + wl_maclist_x);
		}else{
			console.log("acl_block:wl_maclist_x222 = " + wl_maclist_x);
		}

		require(['/require/modules/makeRequest.js'], function(makeRequest) {
			makeRequest.post('/apply.cgi','action_mode=apply&wl_maclist_x='+wl_maclist_x+'&wl_unit='+nc_wl_unit+'&wl_macmode='+wl_macmode+'&rc_service=restart_wireless', notification.update_read, notification.acl_block);
		});
	},

	web_display: function(eventid){
		
		if(NTDB_content[eventid] == undefined)	return false;

		var display_group = NTDB_content[eventid].group;

		if(notification.reverse_bin(parseInt(display_group, 10).toString(2))[0] == 1)
			return true;
		else
			return false;
	},


	reverse_bin: function(s){
    return s.split("").reverse().join("");
	},

	read_one: function(event_id, tstamp, event_type){
		name_id = "loadingIcon_"+event_id;
		document.getElementById(name_id).style.display = "";
		require(['/require/modules/makeRequest.js'], function(makeRequest) {
			makeRequest.post('/apply.cgi','action_mode=nt_apply&nt_action=write&nt_event='+event_id+'&tstamp='+tstamp+'&nt_status=1',notification.update_read, notification.read_one);
		});
	},

	update_read: function(){
		setTimeout(function(){notification.updateNTDB_Status();}, 500);
		setTimeout(function(){notification.update_nc_setting_conf();}, 500);
		setTimeout(function(){notification.update_nt_action();}, 500);
		setTimeout(function(){notification.show_event();}, 1000);
	},

	read_all: function(){
		require(['/require/modules/makeRequest.js'], function(makeRequest) {
			makeRequest.post('/apply.cgi','action_mode=nt_apply&nt_action=readall',notification.update_read, notification.readall);
		});
	},


	noti_event_time: function(tstamp){
		var time_string;
		var now = new Date();
		var time = new Date();
		time.setTime(tstamp*1000);
		var event_time = new Date(time);

		var diffMs = (now-event_time); // milliseconds between now & Christmas
		var diffDays = Math.round(diffMs / 86400000); // days
		var diffHrs = Math.round((diffMs % 86400000) / 3600000); // hours
		var diffMins = Math.round(((diffMs % 86400000) % 3600000) / 60000); // minutes
		if(diffDays > 0 || now.getDate() != event_time.getDate())
			time_string = ((event_time.getMonth()+1) < 10 ? "0" : "" + (event_time.getMonth()+1)) + '/' + (event_time.getDate() < 10 ? "0" : "" + event_time.getDate());
		else
			time_string = (event_time.getHours() < 10 ? "0" : "" + event_time.getHours()) + ':' + (event_time.getMinutes() < 10 ? "0" : "" + event_time.getMinutes());

		return time_string;
	},

	noti_event_time_date: function(tstamp, option){
		var time_string;
		var now = new Date();
		var time = new Date();
		time.setTime(tstamp*1000);
		var event_time = new Date(time);
		
		var year = event_time.getFullYear();
		var month = event_time.getMonth()+1;
			month = (month < 10 ? "0" : "") + month;
		var date = event_time.getDate();
			date = (date < 10 ? "0" : "") + date;
		var hour = event_time.getHours();
			hour = (hour < 10 ? "0" : "") + hour;
		var min = event_time.getMinutes();
			min = (min < 10 ? "0" : "") + min;

		if(option == "date")
			return year+' / '+month+' / '+date;
		else if(option == "clock")
			return hour + ':' + min;
	},

	notiClick: function(){
		//document.getElementById("notification_status").className = "notification_on";
		
		if(notification.clicking == 0){

			var txt = '<div id="notiDiv">';

				txt += '<div style="padding:10px 10px 0px 10px;width:94%">';
		  			txt += '<div style="vertical-align:middle;width:8%;display:table-cell;"><div class="noti_readall" onclick="notification.read_all();" title="Mark all as read"></div></div>';
		  			txt += '<div style="vertical-align:middle;width:86%;display:table-cell;"><div style="font-size:18px;color:#7A797A;text-align:center;">New Notification</div></div>';
		  			txt += '<div style="vertical-align:middle;width:8%;display:table-cell;"><div class="nt_close" title="Close" onclick="notification.close_nt_windows();"></div></div>';
	  			txt += '</div>';	//div table end			

	  			txt += '<div style="padding:6px;">';
				txt += '<div class="notiDiv_table">';
					txt += '<div style="display:table;width:100%;">';

		  			txt += '<div id="show_event_table" style="background-color:#474D4F"></div>';

					txt += '<div style="padding-top:6px;background-color:#474D4F;"><div style="background-color:#808080;height:3px;width:95%;margin:0px auto;"></div></div>';
					txt += '<div style="padding:10px;display:table;width:95%;background-color:#474D4F;-webkit-border-radius:0px 3px;-moz-border-radius:0px 3px;border-radius:0px 3px;">';
						txt += '<div style="display:table-cell;vertical-align:middle;width:90%;text-align:center;padding:7px" class="notiDiv_all_event_list"><a href="/Advanced_Notification_Content.asp"><div>All Event List</div></div>';
					txt += '</div>';

	  			txt += '</div>';	//div table end
				txt += '</div>';	//div table end
			txt += '</div>';

			document.getElementById("notification_desc").innerHTML = txt;
			notification.show_event();
			notification.clicking = 1;
		}else{
			document.getElementById("notification_desc").innerHTML = "";
			notification.clicking = 0;
		}
		notification.add_clcik_event();
	},

	close_nt_windows: function(){
		notification.notiClick();
	},

	run: function(){
		var tarObj = document.getElementById("notification_status");
		var tarObj_td = document.getElementById("notification_status_td");

		if(notification.bell_num > 0)
			notification.stat = "on";
		else
			notification.stat = "off";

		if(tarObj === null)	
			return false;		

		if(this.stat == "on"){
			tarObj.onclick = this.notiClick;
			tarObj.className = "notification_on";
			if(this.bell_num > 0){
				tarObj.className += " notification_alert";
			}
		}else if(this.stat == "off"){
			tarObj.onclick = this.notiClick;
			tarObj.className = "notification_off";
		}
		notification.update_bell_num();
	},

	reset: function(){
		this.stat = "off";
		this.flash = "off";
		this.run();
	}
}
