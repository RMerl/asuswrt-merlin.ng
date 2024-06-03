/*
1. How to use
var schedule = new schedule();
schedule.Get_UI(), Get jquery html obj.
schedule.Get_Value(), Get Json array data.
schedule.Get_Value_AccessTime, Get Json array data, ex. {start_timestamp: 'T11667897340Z', end_timestamp: 'T01667904480Z'}

2. Input data structure, need json arry obj.
ex.
var dataString = '[{"type":"M","enable":1,"weekday":62,"start_hour":17,"start_min":0,"end_hour":21,"end_min":0},{"type":"M","enable":1,"weekday":65,"start_hour":16,"start_min":0,"end_hour":22,"end_min":0}]';
var dataJSON = jQuery.parseJSON(dataString);
var schedule = new schedule({data:dataJSON});

3. config setting
a. data_max: number.
b. schedule_type: W is offline, M is online.
c. others_rule_num: Other rule number at same feature.
d. alternate_days: true/false, start time 22:00 ~ end time 08:00 / start time 09:00 ~ end time 21:00.
e. draggable: true/false, overview time can draggable or not.
f. change_schedule_mode: true/false, supoort change W or M.
g. time_interval: true/false, support start and end time, or start time only.
h. icon_switch_callback/icon_trash_callback/btn_save_callback/btn_cancel_callback/AccTime_quickset_callback/: for call back func.
i. mode: 1:Schedule, 2:Access Time
j. Schedule_suppot/AccTime_support: mode support.
ex.
var schedule = new schedule({data_max:30, schedule_type:"W", alternate_days:false});
var schedule = new schedule({AccTime_support:true, AccTime_data:{start_timestamp: 'T11667894694Z', end_timestamp: 'T01667894754Z'}, mode:1/2});

4. schedule_handle_data API
a. data structure
"timeset_attr": function(){
	this.type = "W";
	this.enable = 1;
	this.weekday = 0;
	this.start_hour = 0;
	this.start_min = 0;
	this.end_hour = 24;
	this.end_min = 0;
}
b. string_to_json_array: string to Json array
ex.
schedule_handle_data.string_to_json_array("M13E17002100<M14116002200");
c. json_array_to_string: Json array to string
ex.
schedule_handle_data.json_array_to_string(schedule.Get_Value());

5. accesstime_handle_data API
a. data structure
"timeset_attr": function(){
	this.start_timestamp = "";
	this.end_timestamp = "";
}
b. string_to_json_array: string to Json array
ex.
accesstime_handle_data.string_to_json_array("T11667894694Z,T01667894754Z");
-->{start_timestamp: 'T11667894694Z', end_timestamp: 'T01667894754Z'}
c. json_array_to_string: Json array to string
ex.
accesstime_handle_data.json_array_to_string(schedule.Get_Value_AccessTime());
-->T11667894694Z,T01667894754Z
*/
if(typeof JS_timeObj != "object"){
	var JS_timeObj = new Date();
}
if(typeof stringSafeGet != "function"){
	function stringSafeGet(str){
		return str.replace(new RegExp("&#39;", 'g'), "'");
	}
}
var str_Scheduled = stringSafeGet("<#Time_Scheduled#>");
var str_Scheduled_one_time = stringSafeGet("<#Time_Scheduled_one_time#>");
var schedule = function(config_set){
	var _config = {
		data_max: 64,
		schedule_type: "M",
		others_rule_num: 0,
		alternate_days: true,
		draggable: false,
		change_schedule_mode: false,
		time_interval: true,
		rule_id: 100,
		icon_switch_callback: null,
		icon_trash_callback: null,
		show_timeset_viewport_callback: null,
		btn_save_callback: null,
		btn_cancel_callback: null,
		mode: 1,//1:Schedule, 2:Access Time
		Schedule_suppot: true,
		AccTime_support: false,
		AccTime_quickset_callback: null
	};
	var _data = {
		day_struct: [],
		cla_struct: [],
		accesstime_struct: []
	};
	var _mode_mapping = [
		{"bitwise":0, "text":str_Scheduled, "type":"schedule"},
		{"bitwise":1, "text":str_Scheduled_one_time, "type":"access_time"},
	];
	$.each(_mode_mapping, function(index, item){
		item["value"] = Math.pow(2, parseInt(item.bitwise));
	});
	var _weekday_mapping = [
		{"bitwise":0, "text":"Sunday"},
		{"bitwise":1, "text":"Monday"},
		{"bitwise":2, "text":"Tuesday"},
		{"bitwise":3, "text":"Wednesday"},
		{"bitwise":4, "text":"Thursday"},
		{"bitwise":5, "text":"Friday"},
		{"bitwise":6, "text":"Saturday"}
	];
	var _accesstime_mapping = [
		{"value":1800, "title":"30 <#Minute#>", "text":"30 mins"},
		{"value":3600, "title":"1 <#Hour#>", "text":"1 hr(s)"},
		{"value":7200, "title":"2 <#Hour#>", "text":"2 hr(s)"},
		{"value":14400, "title":"4 <#Hour#>", "text":"4 hr(s)"},
		{"value":21600, "title":"6 <#Hour#>", "text":"6 hr(s)"}
	];
	var _time_unit_parm = {
		"text":{"hour":"<#Hour#>", "minute":"<#Minute#>"},
		"interval":{"hour":1, "minute":15},
		"end_time":{"hour":23, "minute":60},
		"val":{
			"hour":{"start":8, "end":12},
			"minute":{"start":0, "end":0}
		},
		"AccTime_interval":{"hour":5, "minute":15},
		"AccTime_end_time":{"hour":100, "minute":60},
		"AccTime_val":{
			"hour":12,
			"minute":0
		}
	};
	var _current_weekday = (function(){
		var result = {"text":"", "weekday":""};
		var abbr_weekday = JS_timeObj.toString().substring(0,3).toUpperCase();
		var specific_weekday = _weekday_mapping.filter(function(item, index, _array){
			return (item.text.substr(0,3).toUpperCase() == abbr_weekday);
		})[0];
		result.text = specific_weekday.text;
		result.weekday = Math.pow(2, parseInt(specific_weekday.bitwise));
		return result;
	})();
	var timeset_attr = function(){
		this.id = "";
		this.type = "M";
		this.enable = 1;
		this.weekday = 0;
		this.start_hour = 0;
		this.start_min = 0;
		this.end_hour = 24;
		this.end_min = 0;
		this.title = "";
	};
	var accesstime_attr = function(){
		this.type = "T";
		this.start_flag = "1";
		this.start_timestamp = "";
		this.end_flag = "0";
		this.end_timestamp = "";
		this.seconds = 0;
	};

	init_data_obj();

	if(config_set != undefined){
		if(config_set.hasOwnProperty("data_max") && !isNaN(parseInt(config_set.data_max))){
			_config.data_max = config_set.data_max;
		}
		if(config_set.hasOwnProperty("schedule_type") && typeof config_set.schedule_type == "string"){
			var type = config_set.schedule_type.toLocaleUpperCase();
			if(type == "W" || type == "M")
				_config.schedule_type = type;
		}
		if(config_set.hasOwnProperty("alternate_days") && typeof config_set.alternate_days == "boolean"){
			_config.alternate_days = config_set.alternate_days;
		}
		if(config_set.hasOwnProperty("draggable") && typeof config_set.draggable == "boolean"){
			_config.draggable = config_set.draggable;
		}
		if(config_set.hasOwnProperty("change_schedule_mode") && typeof config_set.change_schedule_mode == "boolean"){
			_config.change_schedule_mode = config_set.change_schedule_mode;
		}
		if(config_set.hasOwnProperty("time_interval") && typeof config_set.time_interval == "boolean"){
			_config.time_interval = config_set.time_interval;
		}
		if(config_set.hasOwnProperty("data")){
			if(Array.isArray(config_set.data)){
				$.each(config_set.data, function(index, value){
					var timeset_data = JSON.parse(JSON.stringify(new timeset_attr()));
					$.extend(true, timeset_data, value);
					timeset_data.id = _config.rule_id.toString() + num_add_left_pad(((parseInt(timeset_data.weekday)).toString(16).toUpperCase()), 2) +
								num_add_left_pad(timeset_data.start_hour, 2) + num_add_left_pad(timeset_data.start_min, 2) +
								num_add_left_pad(timeset_data.end_hour, 2) + num_add_left_pad(timeset_data.end_min, 2);
						switch(timeset_data.weekday){
							case 127 ://Daily
								timeset_data.title = "Daily";
								_data.cla_struct["Daily"].push(timeset_data);
							break;
							case 62 ://Weekdays
								timeset_data.title = "Weekdays";
								_data.cla_struct["Weekdays"].push(timeset_data);
							break;
							case 65 ://Weekend
								timeset_data.title = "Weekend";
								_data.cla_struct["Weekend"].push(timeset_data);
							break;
							default ://Other
								timeset_data.title = get_week_title(timeset_data.weekday);
								_data.cla_struct["Other"].push(timeset_data);
							break;
						}
						_config.rule_id++;
				});
			}
		}
		if(config_set.hasOwnProperty("icon_switch_callback") && typeof config_set.icon_switch_callback == "function"){
			_config.icon_switch_callback = config_set.icon_switch_callback;
		}
		if(config_set.hasOwnProperty("icon_trash_callback") && typeof config_set.icon_trash_callback == "function"){
			_config.icon_trash_callback = config_set.icon_trash_callback;
		}
		if(config_set.hasOwnProperty("btn_save_callback") && typeof config_set.btn_save_callback == "function"){
			_config.btn_save_callback = config_set.btn_save_callback;
		}
		if(config_set.hasOwnProperty("btn_cancel_callback") && typeof config_set.btn_cancel_callback == "function"){
			_config.btn_cancel_callback = config_set.btn_cancel_callback;
		}
		if(config_set.hasOwnProperty("show_timeset_viewport_callback") && typeof config_set.show_timeset_viewport_callback == "function"){
			_config.show_timeset_viewport_callback = config_set.show_timeset_viewport_callback;
		}
		if(config_set.hasOwnProperty("Schedule_suppot") && typeof config_set.Schedule_suppot == "boolean"){
			_config.Schedule_suppot = config_set.Schedule_suppot;
		}
		if(config_set.hasOwnProperty("AccTime_support") && typeof config_set.AccTime_support == "boolean"){
			_config.AccTime_support = config_set.AccTime_support;
		}
		if(_config.Schedule_suppot && _config.AccTime_support)
			_config.mode = 1;
		else if(!_config.Schedule_suppot && _config.AccTime_support)
			_config.mode = 2;
		if(config_set.hasOwnProperty("mode") && (typeof config_set.mode == "string" ||  typeof config_set.mode == "number")){
			var mode = parseInt(config_set.mode);
			if(mode === 1 || mode === 2)
				_config.mode = mode;
		}
		if(config_set.hasOwnProperty("AccTime_data")){
			if(Array.isArray(config_set.AccTime_data)){
				$.each(config_set.AccTime_data, function(index, value){
					if(value.start_timestamp != undefined && value.end_timestamp){
						if(value.start_timestamp.length == 13 && value.end_timestamp.length == 13){
							var accesstime_data = JSON.parse(JSON.stringify(new accesstime_attr()));
							accesstime_data.start_flag = value.start_timestamp.substr(1,1).toUpperCase();
							accesstime_data.start_timestamp = parseInt(value.start_timestamp.substr(2,10));
							accesstime_data.end_flag = value.end_timestamp.substr(1,1).toUpperCase();
							accesstime_data.end_timestamp = parseInt(value.end_timestamp.substr(2,10));
							accesstime_data.seconds = accesstime_data.end_timestamp - accesstime_data.start_timestamp;
							_data.accesstime_struct.push(accesstime_data);
						}
					}
				});
			}
		}
		if(config_set.hasOwnProperty("AccTime_quickset_callback") && typeof config_set.AccTime_quickset_callback == "function"){
			_config.AccTime_quickset_callback = config_set.AccTime_quickset_callback;
		}
	}
	this.Get_UI = function(){
		var $container = $("<div>").addClass("schedule_ui").css({"width":"100%", "height":"100%"});
		if(_config.Schedule_suppot && _config.AccTime_support){
			$container.append(Get_Schedule_Mode_Container());
		}
		if(_config.Schedule_suppot){
			var $mode_schedule = $("<div>").attr({"data-viewport":"schedule"}).appendTo($container);
			var $schedule_main = $("<div>").attr({"data-viewport":"schedule_main"}).appendTo($mode_schedule);
			$schedule_main.append(Get_Header_Container())
			$schedule_main.append(Get_Content_Container());
			var $schedule_timeset = $("<div>").attr({"data-viewport":"schedule_timeset"}).appendTo($mode_schedule);
			$schedule_timeset.unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				reset_combination_select_status($(this));
			}).append(Get_TimeSet_Container()).hide();
			var rule_count = get_rule_count();
			if(rule_count == 0){
				$schedule_main.find(".schedule_header_container .btn_control_bg").children().filter(".add_icon, .collapse_icon").hide();
			}
		}
		if(_config.AccTime_support){
			var $mode_access_time = $("<div>").attr({"data-viewport":"access_time"}).appendTo($container);
			$mode_access_time.append(Get_AccessTime_Container());
		}
		if(_config.Schedule_suppot && _config.AccTime_support){
			$container.find("[data-container=mode]")
				.find(".radio_container, .icon_radio").removeClass("clicked")
				.filter("[value=" + _config.mode + "]").click();
		}
		if(httpApi.nvramGet(["ntp_ready"]).ntp_ready != "1"){
			$("<div>").addClass("warning_desc")
				.html("* " + stringSafeGet("<#General_x_SystemTime_syncNTP2#>") + "")
				.appendTo($container);
		}
		return $container;
	};
	this.Get_Value = function(){
		var data_arr = [];
		Object.keys(_data.cla_struct).forEach(function(key) {
			$.each(_data.cla_struct[key], function(index, value){
				var timeset_data = JSON.parse(JSON.stringify(new schedule_handle_data.timeset_attr()));
				timeset_data.type = value.type;
				timeset_data.enable = value.enable;
				timeset_data.weekday = value.weekday;
				timeset_data.start_hour = value.start_hour;
				timeset_data.start_min = value.start_min;
				timeset_data.end_hour = value.end_hour;
				timeset_data.end_min = value.end_min;
				data_arr.push(timeset_data);
			});
		});
		return data_arr;
	};
	this.Get_Value_AccessTime = function(_delay){
		var delay_time = 0;
		if(_delay != undefined && _delay != "" && !isNaN(_delay))
		 delay_time = _delay;

		var data_arr = [];
		var utctimestamsp = parseInt(httpApi.hookGet("utctimestamp", true));
		var cur_time = (isNaN(utctimestamsp)) ? 0 : utctimestamsp;
		$.each(_data.accesstime_struct, function(index, item){
			if(index == 0){
				var accesstime_data = JSON.parse(JSON.stringify(new accesstime_handle_data.timeset_attr()));
				accesstime_data.start_timestamp = item.type + item.start_flag + (cur_time + delay_time) + "Z";
				accesstime_data.end_timestamp = item.type + item.end_flag + (cur_time + item.seconds + delay_time) + "Z";
				data_arr.push(accesstime_data);
			}
		});
		return data_arr;
	};
	this.Get_Value_Mode = function(){
		if(_config.mode != 1 && _config.mode != 2)
			return 1;
		else
			return _config.mode;
	}

	function init_data_obj(){
		init_data_day_struct();
		init_data_cla_struct();
	}
	function init_data_day_struct(){
		_data.day_struct = [];
		$.each(_weekday_mapping, function(index, value){
			var weekday_obj = value;
			_data.day_struct["w_" + weekday_obj.bitwise] = [];//initial data obj
		});
	}
	function init_data_cla_struct(){
		_data.cla_struct = [];
		_data.cla_struct["Daily"] = [];
		_data.cla_struct["Weekdays"] = [];
		_data.cla_struct["Weekend"] = [];
		_data.cla_struct["Other"] = [];
	}
	function Get_Header_Container(){
		var $container = $("<div>").addClass("schedule_header_container");

		var $title_bg = $("<div>").addClass("title_bg").appendTo($container);
		var title_text = "<#weekSche_Online_Sche#>";

		if(_config.change_schedule_mode){
			$title_bg.addClass("change_mode");
			if(_config.schedule_type == "W"){
				$title_bg.addClass("offline");
				title_text = "<#weekSche_Offtime_Sche#>";
			}
			else if(_config.schedule_type == "M"){
				$title_bg.addClass("online");
				title_text = "<#weekSche_Online_Sche#>";
			}
			$title_bg.unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
			});
		}
		else{
			if(_config.schedule_type == "W"){
				title_text = "<#weekSche_Offtime_Sche#>";
			}
			else if(_config.schedule_type == "M"){
				title_text = "<#weekSche_Online_Sche#>";
			}
		}

		$("<div>").addClass("title").html(title_text).appendTo($title_bg);
		if(_config.change_schedule_mode)
			$("<div>").addClass("change_mode_icon").appendTo($title_bg);

		var $btn_control_bg = $("<div>").addClass("btn_control_bg").appendTo($container);
		$("<div>")
			.addClass("btn_icon_bg add_icon listview_mode")
			.attr("title", "Add a new rule")/* untranslated */
			.unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				if(check_rule_full()){
					/*
					popupHint.init();
					var hint = "<#weekSche_MAX_Num#>".replace("#MAXNUM", weekScheduleApi.data_max);
					hint += "<br>";
					hint += "<#weekSche_MAX_Del_Hint#>";
					popupHint.set_text(hint);
					popupHint.set_btn_ok();
					popupHint.hide_btn("cancel");
					*/
					return;
				}
				show_timeset_viewport($(this));
				set_current_weekday($(this));
			}).appendTo($btn_control_bg);

		$("<div>")
			.addClass("btn_icon_bg overview_icon listview_mode")
			.attr("title", "Switch View")/* untranslated */
			.unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				var $schedule_ui_obj = $(this).closest(".schedule_ui");
				$schedule_ui_obj.find(".listview_mode").hide();
				$schedule_ui_obj.find(".overview_mode").show();
				/*
				init_data_day_struct();
				transform_cla_struct_to_day_struct();
				if($schedule_ui_obj.find(".overview_mode").css("display") == "block"){
					weekScheduleApi.set_offtime_bg();
					if(weekScheduleApi.support_draggable)
						weekScheduleApi.set_offtime_bg_resizable_draggable();
				}
				*/
			}).appendTo($btn_control_bg).hide();

		$("<div>")
			.addClass("btn_icon_bg listview_icon overview_mode")
			.attr("title", "Switch View")/* untranslated */
			.unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				var $schedule_ui_obj = $(this).closest(".schedule_ui");
				$schedule_ui_obj.find(".listview_mode").show();
				$schedule_ui_obj.find(".overview_mode").hide();
				/*
				var rule_count = get_rule_count();
				$schedule_ui_obj.find(".offtime_schedule_content_bg").empty();
				if(rule_count == 0)
					$schedule_ui_obj.find(".offtime_schedule_content_bg").append(weekScheduleApi.PC_get_no_data_component());
				else
					$schedule_ui_obj.find(".offtime_schedule_content_bg").append(weekScheduleApi.PC_get_list_period_component());
				*/
			}).appendTo($btn_control_bg).hide();

		$("<div>")
			.addClass("btn_icon_bg collapse_icon expand listview_mode")
			.attr("title", "Expand Collapse")/* untranslated */
			.unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				$(this).toggleClass("expand");
				var $schedule_ui_obj = $(this).closest(".schedule_ui");
				if($(this).hasClass("expand"))
					$schedule_ui_obj.find("[view_mode=list]").slideDown();
				else
					$schedule_ui_obj.find("[view_mode=list]").slideUp();
			}).appendTo($btn_control_bg);

		return $container;
	}
	function Get_Content_Container(){
		var $container = $("<div>").addClass("schedule_content_container");
		var $list_mode_content = $("<div>").attr({"view_mode":"list"}).appendTo($container);
		$list_mode_content.append(Get_List_Mode_Container());
		return $container;
	}
	function Get_List_Mode_Container(){
		var rule_count = get_rule_count();
		if(rule_count == 0)
			return Get_Add_New_Component();
		else
			return Get_List_Period_Container();
	}
	function Get_Add_New_Component(){
		var $component = $("<div>").addClass("add_new_component").unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation()
			show_timeset_viewport($(this));
			set_current_weekday($(this));
		});
		$("<div>").addClass("add_new_text").html("<#vpnc_step1#>").appendTo($component);
		return $component;
	}
	function Get_List_Period_Container(){
		sort_data_cla_struct();
		var $container = $("<div>");

		Object.keys(_data.cla_struct).forEach(function(key){
			var week_group = key;
			var $list_period_group_bg = $("<div>").attr("id", "list_period_" + week_group + "").appendTo($container);
			$.each(_data.cla_struct[week_group], function(index, value){
				var timeset_data = value;
				var $list_period_bg = $("<div>").addClass("list_period_bg").attr({"id":timeset_data.id, "week_group":week_group}).appendTo($list_period_group_bg);

				//var $list_period_title = $("<div>").addClass("list_period_title").appendTo($list_period_bg);
				var $list_period_title = $("<div>").addClass("list_period_title");
				var display_title = "";
				if(week_group == "Other")
					display_title = value.title
						.replace("MON", "<#date_Mon_itemdesc#>").replace("TUE", "<#date_Tue_itemdesc#>").replace("WED", "<#date_Wed_itemdesc#>")
						.replace("THU", "<#date_Thu_itemdesc#>").replace("FRI", "<#date_Fri_itemdesc#>").replace("SAT", "<#date_Sat_itemdesc#>")
						.replace("SUN", "<#date_Sun_itemdesc#>");
				else
					display_title = week_group.replace("Daily", "<#weekSche_Everyday#>").replace("Weekdays", "<#weekSche_Weekdays#>").replace("Weekend", "<#weekSche_Weekend#>");
				$list_period_title.html(display_title);

				var $list_period_content = $("<div>").addClass("list_period_content").appendTo($list_period_bg);
				if(timeset_data.enable)
					$list_period_content.addClass("on");
				else
					$list_period_content.addClass("off");

				$list_period_content.unbind("click").click(function(e){
					e = e || event;
					e.stopPropagation();
					show_timeset_viewport($(this));
					update_sel_timeset_data($(this), timeset_data);
				});

				var $time_set_container = $("<div>").addClass("time_set_container").appendTo($list_period_content);
				var $time_set_title = $("<div>").addClass("time_set_title").appendTo($time_set_container);
				$time_set_title.html(display_title);
				var $time_set_content = $("<div>").addClass("time_set_content").appendTo($time_set_container);

				var time_set = num_add_left_pad(timeset_data.start_hour, 2) + ":" + num_add_left_pad(timeset_data.start_min, 2);
				if(_config.time_interval){
					time_set += " - ";
					time_set += num_add_left_pad(timeset_data.end_hour, 2) + ":" + num_add_left_pad(timeset_data.end_min, 2);
				}
				$time_set_content.html(time_set);

				var $time_switch_bg = $("<div>").addClass("time_switch_bg").appendTo($list_period_content);
				var $period_switch = $("<div>");
				$period_switch.appendTo($time_switch_bg);
				$period_switch.addClass("icon_switch");
				if(timeset_data.enable)
					$period_switch.addClass("on");
				else
					$period_switch.addClass("off");
				$period_switch.unbind("click");
				$period_switch.click(function(e){
					e = e || event;
					e.stopPropagation();
					$(this).toggleClass("off  on");
					$list_period_content.toggleClass("off  on");
					if($(this).hasClass("icon_switch on"))
						timeset_data.enable = 1;
					else
						timeset_data.enable = 0;

					if(_config.icon_switch_callback != null){
						_config.icon_switch_callback();
					}
				});

				var $time_trash_bg = $("<div>").addClass("time_trash_bg").appendTo($list_period_content);
				$("<div>")
					.addClass("icon_trash")
					.unbind("click").click(function(e){
						e = e || event;
						e.stopPropagation();
						var delete_id = $(this).parents(".list_period_bg").attr("id");
						var delete_week_group = $(this).parents(".list_period_bg").attr("week_group");
						delete_array_by_property(_data.cla_struct[delete_week_group], "id", delete_id);
						var rule_count = get_rule_count();
						if(rule_count == 0){
							$(this).closest("[data-viewport=schedule_main]").find(".schedule_header_container .btn_control_bg").children().filter(".add_icon, .collapse_icon").hide();
							$(this).closest(".schedule_content_container").find("[view_mode=list]").empty().append(Get_Add_New_Component());
						}
						else{
							$(this).parents(".list_period_bg").remove();
						}
						if(_config.icon_trash_callback != null){
							_config.icon_trash_callback();
						}
					}).appendTo($time_trash_bg);
			});
		});

		return $container;
	}
	function Get_TimeSet_Container(){
		var $container = $("<div>").addClass("schedule_timeset_container").attr({"data-timeset-id":"new"});

		var $timeset_cntr = $("<div>").addClass("timeset_container").appendTo($container);

		function Get_Hour_Minute_Component(_type){
			var $container = $("<div>").addClass("hr_mins_container");
			var title_text = "";
			if(_type == "start")
				title_text = "<#weekSche_Start_Time#>";
			else
				title_text = "<#weekSche_End_Time#>";

			$("<div>").addClass("hr_mins_title").html(title_text).appendTo($container);

			var $hr_mins_content = $("<div>").addClass("hr_mins_content").appendTo($container);
			function Get_Time_Unit_Component(_type, _unit){
				var $time_unit_comp = $("<div>").addClass("time_unit_component");
				var $time_unit_title = $("<div>").addClass("time_unit_title").html(_time_unit_parm["text"][_unit]).appendTo($time_unit_comp);

				var $time_unit_content = $("<div>").addClass("time_unit_content").appendTo($time_unit_comp);
				var $combi_input_select = $("<div>").addClass("combination_input_select").appendTo($time_unit_content);
				$combi_input_select.attr({"data-control":"activate"})

				var input_obj_value = num_add_left_pad(_time_unit_parm["val"][_unit][_type], 2);
				$("<input/>")
					.addClass("combi_input " + _type + " " + _unit + "")
					.val(input_obj_value)
					.attr({"type" : "text", "maxlength" : 2, "autocomplete" : "off", "autocorrect" : "off", "autocapitalize" : "off"})
					.keypress(function(){
						reset_combination_select_status($(this).closest(".timeset_container"));
						return validator.isNumber(this,event);
					})
					.blur(function(){
						var value = $.trim($(this).val());
						$(this).val(value);
						var $top_timeset_obj = $(this).closest(".timeset_container");
						reset_combination_select_status($top_timeset_obj);
						function show_valid_hint(_hint, _inputObj){
							$top_timeset_obj.find(".combi_input").attr("disabled", true);
							$top_timeset_obj.find(".combination_input_select").attr({"data-control":"deactivate"});

							$top_timeset_obj.find(".valid_hint_component").html(_hint).show();
							$(_inputObj).attr("disabled", false);
							$(_inputObj).closest(".combination_input_select").attr({"data-control":"activate"}).addClass("focus");
						}

						if(value.length == 0){
							show_valid_hint("* <#JS_fieldblank#>", $(this));
							return false;
						}
						if(isNaN(value) || (_unit == "hour" && (parseInt(value) > 24)) || (_unit == "minute" && (parseInt(value) > 59))){
							show_valid_hint("* <#FirewallConfig_URLActiveTime_itemhint2#>", $(this));
							return false;
						}

						var start_hour = parseInt($top_timeset_obj.find(".combi_input.start.hour").val());
						var start_min = parseInt($top_timeset_obj.find(".combi_input.start.minute").val());
						var end_hour = parseInt($top_timeset_obj.find(".combi_input.end.hour").val());
						var end_min = parseInt($top_timeset_obj.find(".combi_input.end.minute").val());
						if(start_hour == 24){
							start_hour = "00";
							$top_timeset_obj.find(".combi_input.start.hour").val(start_hour);
						}
						if(start_hour == 0 && start_min == 0 && end_hour == 0 && end_min == 0){
							end_hour = "24";
							$top_timeset_obj.find(".combi_input.end.hour").val(end_hour);
						}
						if((end_hour*60+end_min) > 1440){//1440=24:00
							end_hour = "00";
							$top_timeset_obj.find(".combi_input.end.hour").val(end_hour);
						}

						if(!_config.alternate_days){
							if((start_hour*60+start_min) > (end_hour*60+end_min)){
								show_valid_hint("* <#FirewallConfig_URLActiveTime_itemhint#>", $(this));
								return false;
							}
							if((end_hour*60+end_min) > 1440){//24:00
								show_valid_hint("* <#weekSche_End_Time#> : <#weekSche_format_incorrect#>", $(this));
								return false;
							}
							if((start_hour*60+start_min) == (end_hour*60+end_min)){
								show_valid_hint("* Please enter different start and end time.", $(this));/* untranslated */
								return false;
							}
						}

						$top_timeset_obj.find(".valid_hint_component").hide().empty();
						$top_timeset_obj.find(".combi_input").attr("disabled", false);
						$top_timeset_obj.find(".combination_input_select").attr({"data-control":"activate"}).removeClass("focus");

						$(this).val(num_add_left_pad($(this).val(), 2));
					})
					.appendTo($combi_input_select);

				$("<div>")
					.addClass("combi_sel")
					.unbind("click").click(function(e){
						e = e || event;
						e.stopPropagation();
						var $combi_input_select
						var status = $(this).closest(".combination_input_select").attr("data-control");
						if(status == "activate"){
							var $combi_sel_list_obj = $(this).siblings(".combi_sel_list");
							$combi_sel_list_obj.animate({scrollTop: 0});
							if($combi_sel_list_obj.hasClass("combi_sel_list_hide")){//click first time or not click again
								var $top_timeset_obj = $(this).closest(".timeset_container");
								$top_timeset_obj.find(".combi_sel_list").each(function(index, item){
									if(!($(this).hasClass("combi_sel_list_hide")))
										$(this).toggleClass("combi_sel_list_hide");
								});
								$top_timeset_obj.find(".combi_sel").each(function(index, item){
									if($(this).hasClass("arrow_active"))
										$(this).toggleClass("arrow_active");
								});
								$combi_sel_list_obj.toggleClass("combi_sel_list_hide");
							}
							else{//click again
								$combi_sel_list_obj.toggleClass("combi_sel_list_hide");
							}
							$(this).toggleClass("arrow_active");
						}
					})
					.appendTo($combi_input_select);

				var $combi_sel_list = $("<div>").addClass("combi_sel_list combi_sel_list_hide").appendTo($combi_input_select);
				var end_time = _time_unit_parm["end_time"][_unit];
				var interval =  _time_unit_parm["interval"][_unit];
				for(var idx = 0; idx <= end_time; idx += interval) {
					var optionText = num_add_left_pad(idx, 2);
					var optionValue = idx;
					if(_unit == "minute"){
						if(idx == 60){
							optionText = "59";
							optionValue = 59;
						}
					}
					$("<div>")
						.addClass("combi_sel_option")
						.attr("value", optionValue)
						.html(optionText)
						.unbind("click").click(function(e){
							e = e || event;
							e.stopPropagation();
							var $combi_input_sel_obj = $(this).closest(".combination_input_select");
							var $combi_input_obj = $combi_input_sel_obj.find(":input");
							var sel_val = $(this).attr("value");
							var input_val = num_add_left_pad(sel_val, 2);
							$combi_input_obj.val(input_val);
							$combi_input_sel_obj.find(".combi_sel_list").toggleClass("combi_sel_list_hide");
							var $combi_sel_obj = $combi_input_sel_obj.find(".combi_sel");
							$combi_sel_obj.toggleClass("arrow_active");
							if($combi_input_sel_obj.hasClass("focus")){
								$combi_input_sel_obj.toggleClass("focus");
								var $top_timeset_obj = $(this).closest(".timeset_container");
								$top_timeset_obj.find(".valid_hint_component").hide().empty();
								$top_timeset_obj.find(".combi_input").attr("disabled", false);
								$top_timeset_obj.find(".combination_input_select").attr({"data-control":"activate"}).removeClass("focus");
							}
							$combi_input_obj.blur();
						}).appendTo($combi_sel_list);
				}
				return $time_unit_comp
			}
			$hr_mins_content.append(Get_Time_Unit_Component(_type, "hour"));
			$hr_mins_content.append(Get_Time_Unit_Component(_type, "minute"));

			return $container;
		}
		Get_Hour_Minute_Component("start").appendTo($timeset_cntr);
		if(_config.time_interval)
			Get_Hour_Minute_Component("end").appendTo($timeset_cntr);

		$("<div>").addClass("valid_hint_component").attr({"data-valid_hint":"timeset"}).hide().appendTo($timeset_cntr);

		if(_config.time_interval){
			var $all_day_com = $("<div>").addClass("all_day_component").appendTo($timeset_cntr);
			$("<div>").addClass("cb_icon").unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				var $top_timeset_obj = $(this).closest(".timeset_container");
				reset_combination_select_status($top_timeset_obj);
				$(this).toggleClass("clicked");
				if($(this).hasClass("clicked")){
					$top_timeset_obj.find(".combi_input.start.hour").val(num_add_left_pad(0, 2));
					$top_timeset_obj.find(".combi_input.start.minute").val(num_add_left_pad(0, 2));
					$top_timeset_obj.find(".combi_input.end.hour").val(num_add_left_pad(24, 2));
					$top_timeset_obj.find(".combi_input.end.minute").val(num_add_left_pad(00, 2));
					$top_timeset_obj.find(".valid_hint_component").hide().empty();
					$top_timeset_obj.find(".combi_input").attr("disabled", true);
					$top_timeset_obj.find(".combination_input_select").attr({"data-control":"deactivate"}).removeClass("focus");
				}
				else{
					$top_timeset_obj.find(".combi_input").attr("disabled", false);
					$top_timeset_obj.find(".combination_input_select").attr({"data-control":"activate"}).removeClass("focus");
				}
			}).appendTo($all_day_com);
			$("<div>").addClass("cb_text").html("<#weekSche_All_Day#>").appendTo($all_day_com);
		}

		var $weekdayset_cntr = $("<div>").addClass("weekdayset_container").appendTo($container);
		$("<div>").addClass("title").html("<#weekSche_Schedule#>").appendTo($weekdayset_cntr);

		var $weekdayset_comp = $("<div>").addClass("weekdayset_component").appendTo($weekdayset_cntr);
		$.each(_weekday_mapping, function(index, value){
			var weekday_obj = value;
			var abbreviation_weekday = weekday_obj.text.substr(0,3).toUpperCase();
			$("<div>")
				.attr({"weekday":index})
				.html(abbreviation_weekday)
				.unbind("click").click(function(e){
					e = e || event;
					e.stopPropagation();
					reset_combination_select_status($(this).closest(".schedule_timeset_container"));
					$(this).toggleClass("clicked");
					var click_weekday_list = $(this).closest(".weekdayset_component").children("div.clicked");
					var total_weekday = 0;
					click_weekday_list.each(function(i, obj) {
						total_weekday += Math.pow(2, parseInt($(obj).attr("weekday")));
					});
					if(total_weekday == 0)
						$(this).closest(".weekdayset_container").find(".valid_hint_component").html("* <#JS_fieldblank#>").show();
					else
						$(this).closest(".weekdayset_container").find(".valid_hint_component").hide().empty();
				})
				.appendTo($weekdayset_comp);
		});
		$("<div>").addClass("valid_hint_component").attr({"data-valid_hint":"weekdayset"}).hide().appendTo($weekdayset_cntr);

		var $action_cntr = $("<div>").addClass("sche_action_container").appendTo($container);
		$("<div>").addClass("sche_btn_cancel").html("<#CTL_Cancel#>").unbind("click").click(function(e){
			var $top_schedule_ui = $(this).closest(".schedule_ui");
			$top_schedule_ui.find("[data-viewport=schedule_main]").show();
			$top_schedule_ui.find("[data-viewport=schedule_timeset]").hide();
			if(_config.btn_cancel_callback != null){
				_config.btn_cancel_callback();
			}
		}).appendTo($action_cntr);
		$("<div>").addClass("sche_btn_save").html("<#CTL_onlysave#>").unbind("click").click(function(e){
			var $top_schedule_ui = $(this).closest(".schedule_ui");
			var deactivate_len = $top_schedule_ui.find(".combination_input_select[data-control=deactivate]").length;
			var legal_flag = (deactivate_len == 4 || deactivate_len == 0) ? true : false;//4 is mean have click all day, 0 means format not error.
			var start_hour = parseInt($top_schedule_ui.find(".combi_input.start.hour").val());
			var start_min = parseInt($top_schedule_ui.find(".combi_input.start.minute").val());
			var end_hour = parseInt($top_schedule_ui.find(".combi_input.end.hour").val());
			var end_min = parseInt($top_schedule_ui.find(".combi_input.end.minute").val());
			if(isNaN(start_hour) || isNaN(start_min))
				legal_flag = false;
			if(_config.time_interval){
				if(isNaN(end_hour) || isNaN(end_min))
					legal_flag = false;
			}
			else{
				end_hour = end_min = 0;
			}

			if(start_hour > 24 || end_hour > 24 || start_min > 59 || end_min > 59)
				legal_flag = false;

			if(start_hour == 24){
				start_hour = 0;
			}
			if(start_hour == 0 && start_min == 0 && end_hour == 0 && end_min == 0){
				end_hour = 24;
			}
			if((end_hour*60+end_min) > 1440){//1440=24:00
				end_hour = 0;
			}
			if(_config.time_interval){
				if(!_config.alternate_days){
					var start_num = start_hour*60 + start_min;
					var end_num = end_hour*60 + end_min;
					if(start_num >= end_num)
						legal_flag = false;
					if(end_num > 1440)//24:00
						legal_flag = false;
				}
			}

			if(!legal_flag){
				$top_schedule_ui.find(".timeset_container .valid_hint_component").html("* <#weekSche_format_incorrect#> <#weekSche_check_and_retype#>").show();
				return;
			}
			else
				$top_schedule_ui.find(".timeset_container .valid_hint_component").hide().empty();

			var click_weekday_list = $top_schedule_ui.find("[data-viewport=schedule_timeset] .weekdayset_component > div.clicked");
			var total_weekday = 0;
			click_weekday_list.each(function(i, obj) {
				total_weekday += Math.pow(2, parseInt($(obj).attr("weekday")));
			});
			if(total_weekday == 0){
				legal_flag = false;
				$top_schedule_ui.find(".weekdayset_container .valid_hint_component").html("* <#JS_fieldblank#>").show();
				return;
			}
			else
				$top_schedule_ui.find(".weekdayset_container .valid_hint_component").hide().empty();

			if(legal_flag){
				var timeset_id = $top_schedule_ui.find(".schedule_timeset_container").attr("data-timeset-id");
				var timeset_data = {};
				if(timeset_id == "new"){
					timeset_data = JSON.parse(JSON.stringify(new timeset_attr()));
					timeset_data.id = _config.rule_id.toString() + num_add_left_pad(((parseInt(total_weekday)).toString(16).toUpperCase()), 2) +
								num_add_left_pad(start_hour, 2) + num_add_left_pad(start_min, 2) +
								num_add_left_pad(end_hour, 2) + num_add_left_pad(end_min, 2);
					timeset_data.enable = 1;
					timeset_data.type = _config.schedule_type;
					_config.rule_id++;
				}
				else{
					timeset_data = get_specific_timeset_data(timeset_id);
					delete_data_cla_struct(timeset_id);
				}
				timeset_data.weekday = total_weekday;
				timeset_data.start_hour = start_hour;
				timeset_data.start_min = start_min;
				timeset_data.end_hour = end_hour;
				timeset_data.end_min = end_min;

				switch(total_weekday){
					case 127 ://Daily
						timeset_data.title = "Daily";
						_data.cla_struct["Daily"].push(timeset_data);
					break;
					case 62 ://Weekdays
						timeset_data.title = "Weekdays";
						_data.cla_struct["Weekdays"].push(timeset_data);
					break;
					case 65 ://Weekend
						timeset_data.title = "Weekend";
						_data.cla_struct["Weekend"].push(timeset_data);
					break;
					default ://Other
						timeset_data.title = get_week_title(total_weekday);
						_data.cla_struct["Other"].push(timeset_data);
					break;
				}
				$top_schedule_ui.find(".schedule_content_container [view_mode=list]").empty().append(Get_List_Period_Container());
				$top_schedule_ui.find("[data-viewport=schedule_main]").show();
				$top_schedule_ui.find("[data-viewport=schedule_main]").find(".schedule_header_container .btn_control_bg").children().filter(".add_icon, .collapse_icon").show();
				$top_schedule_ui.find("[data-viewport=schedule_timeset]").hide();
			}
			if(_config.btn_save_callback != null){
				_config.btn_save_callback();
			}
		}).appendTo($action_cntr);

		return $container;
	}
	function Get_AccessTime_Container(){
		var $container = $("<div>").addClass("accesstime_container");
		var $quickset_cntr = $("<div>").addClass("quickset_container").appendTo($container);
		var $quickset_comp = $("<div>").addClass("quickset_component").appendTo($quickset_cntr);
		$.each(_accesstime_mapping, function(index, value){
			var quick_time = value;
			$("<div>").attr({"title":quick_time.title, "value":quick_time.value}).append($("<span>").html(quick_time.text)).appendTo($quickset_comp);
		});

		var custom_default = (_time_unit_parm["AccTime_val"]["hour"]*60*60) + (_time_unit_parm["AccTime_val"]["minute"]*60);
		$("<div>").attr({"title":"<#Custom#>", "data-component":"custom", "value":custom_default}).html("<span><#Custom#></span>").addClass("custom").appendTo($quickset_comp);
		$quickset_comp.children().unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			var $top_accesstime_obj = $(this).closest(".accesstime_container");
			$top_accesstime_obj.find(".quickset_component").children().removeClass("clicked").filter($(this)).addClass("clicked");
			$top_accesstime_obj.find("[data-container=timeset_cntr]").hide();
			if(_data.accesstime_struct[0] == undefined){
				var accesstime_data = JSON.parse(JSON.stringify(new accesstime_attr()));
				_data.accesstime_struct.push(accesstime_data);
			}
			_data.accesstime_struct[0].seconds = parseInt($(this).attr("value"));

			if($(this).attr("data-component") == "custom")
				$top_accesstime_obj.find("[data-container=timeset_cntr]").show();

			if(_config.AccTime_quickset_callback != null){
				_config.AccTime_quickset_callback();
			}
		});

		var $timeset_cntr = $("<div>").attr({"data-container":"timeset_cntr"}).addClass("timeset_container").appendTo($container).hide();
		Get_Hour_Minute_Component().appendTo($timeset_cntr);
		function Get_Hour_Minute_Component(){
			var $container = $("<div>").addClass("hr_mins_container");
			var $hr_mins_content = $("<div>").addClass("hr_mins_content").appendTo($container);
			$hr_mins_content.append(Get_Time_Unit_Component("hour"));
			$hr_mins_content.append(Get_Time_Unit_Component("minute"));
			return $container;

			function Get_Time_Unit_Component(_unit){
				var $time_unit_comp = $("<div>").addClass("time_unit_component");
				var $time_unit_title = $("<div>").addClass("time_unit_title").html(_time_unit_parm["text"][_unit]).appendTo($time_unit_comp);

				var $time_unit_content = $("<div>").addClass("time_unit_content").appendTo($time_unit_comp);
				var $combi_input_select = $("<div>").addClass("combination_input_select").appendTo($time_unit_content);
				$combi_input_select.attr({"data-control":"activate"})

				var input_obj_value = num_add_left_pad(_time_unit_parm["AccTime_val"][_unit], 2);
				$("<input/>")
					.addClass("combi_input " + _unit + "")
					.val(input_obj_value)
					.attr({"type" : "text", "maxlength" : 2, "autocomplete" : "off", "autocorrect" : "off", "autocapitalize" : "off"})
					.keypress(function(){
						reset_combination_select_status($(this).closest(".timeset_container"));
						return validator.isNumber(this,event);
					})
					.blur(function(){
						var value = $.trim($(this).val());
						$(this).val(value);
						var $top_timeset_obj = $(this).closest(".timeset_container");
						reset_combination_select_status($top_timeset_obj);
						function show_valid_hint(_hint, _inputObj){
							$top_timeset_obj.find(".combi_input").attr("disabled", true);
							$top_timeset_obj.find(".combination_input_select").attr({"data-control":"deactivate"});

							$top_timeset_obj.find(".valid_hint_component").html(_hint).show();
							$(_inputObj).attr("disabled", false);
							$(_inputObj).closest(".combination_input_select").attr({"data-control":"activate"}).addClass("focus");
						}
						if(value.length == 0){
							show_valid_hint("* <#JS_fieldblank#>", $(this));
							return false;
						}
						if(isNaN(value) || (_unit == "minute" && (parseInt(value) > 59))){
							show_valid_hint("* <#FirewallConfig_URLActiveTime_itemhint2#>", $(this));
							return false;
						}
						var hour = parseInt($top_timeset_obj.find(".combi_input.hour").val());
						var min = parseInt($top_timeset_obj.find(".combi_input.minute").val());
						if((hour == 0 && min < 5)){
							show_valid_hint("* <#FirewallConfig_URLActiveTime_itemhint2#>", $top_timeset_obj.find(".combi_input.minute"));
							return false;
						}
						$(this).val(num_add_left_pad($(this).val(), 2));
						$top_timeset_obj.find(".valid_hint_component").hide().empty();
						$top_timeset_obj.find(".combi_input").attr("disabled", false);
						$top_timeset_obj.find(".combination_input_select").attr({"data-control":"activate"}).removeClass("focus");

						if(_data.accesstime_struct[0] == undefined){
							var accesstime_data = JSON.parse(JSON.stringify(new accesstime_attr()));
							_data.accesstime_struct.push(accesstime_data);
						}
						var start_hour = parseInt($top_timeset_obj.find(".combi_input.hour").val());
						var start_min = parseInt($top_timeset_obj.find(".combi_input.minute").val());
						_data.accesstime_struct[0].seconds = (start_hour*60*60) + (start_min*60);
						if(_config.AccTime_quickset_callback != null){
							_config.AccTime_quickset_callback();
						}
					})
					.appendTo($combi_input_select);

				$("<div>")
					.addClass("combi_sel")
					.unbind("click").click(function(e){
						e = e || event;
						e.stopPropagation();
						var $combi_input_select
						var status = $(this).closest(".combination_input_select").attr("data-control");
						if(status == "activate"){
							var $combi_sel_list_obj = $(this).siblings(".combi_sel_list");
							$combi_sel_list_obj.animate({scrollTop: 0});
							if($combi_sel_list_obj.hasClass("combi_sel_list_hide")){//click first time or not click again
								var $top_timeset_obj = $(this).closest(".timeset_container");
								$top_timeset_obj.find(".combi_sel_list").each(function(index, item){
									if(!($(this).hasClass("combi_sel_list_hide")))
										$(this).toggleClass("combi_sel_list_hide");
								});
								$top_timeset_obj.find(".combi_sel").each(function(index, item){
									if($(this).hasClass("arrow_active"))
										$(this).toggleClass("arrow_active");
								});
								$combi_sel_list_obj.toggleClass("combi_sel_list_hide");
							}
							else{//click again
								$combi_sel_list_obj.toggleClass("combi_sel_list_hide");
							}
							$(this).toggleClass("arrow_active");
						}
					})
					.appendTo($combi_input_select);

				var $combi_sel_list = $("<div>").addClass("combi_sel_list combi_sel_list_hide").appendTo($combi_input_select);
				var end_time = _time_unit_parm["AccTime_end_time"][_unit];
				var interval =  _time_unit_parm["AccTime_interval"][_unit];
				for(var idx = 0; idx <= end_time; idx += interval) {
					var optionText = num_add_left_pad(idx, 2);
					var optionValue = idx;
					if(_unit == "hour"){
						if(idx == 100){
							optionText = "99";
							optionValue = 99;
						}
					}
					if(_unit == "minute"){
						if(idx == 60){
							optionText = "59";
							optionValue = 59;
						}
					}
					$("<div>")
						.addClass("combi_sel_option")
						.attr("value", optionValue)
						.html(optionText)
						.unbind("click").click(function(e){
							e = e || event;
							e.stopPropagation();
							var $combi_input_sel_obj = $(this).closest(".combination_input_select");
							var $combi_input_obj = $combi_input_sel_obj.find(":input");
							var sel_val = $(this).attr("value");
							var input_val = num_add_left_pad(sel_val, 2);
							$combi_input_obj.val(input_val);
							$combi_input_sel_obj.find(".combi_sel_list").toggleClass("combi_sel_list_hide");
							var $combi_sel_obj = $combi_input_sel_obj.find(".combi_sel");
							$combi_sel_obj.toggleClass("arrow_active");
							if($combi_input_sel_obj.hasClass("focus")){
								$combi_input_sel_obj.toggleClass("focus");
								var $top_timeset_obj = $(this).closest(".timeset_container");
								$top_timeset_obj.find(".valid_hint_component").hide().empty();
								$top_timeset_obj.find(".combi_input").attr("disabled", false);
								$top_timeset_obj.find(".combination_input_select").attr({"data-control":"activate"}).removeClass("focus");
							}
							$combi_input_obj.blur();
						}).appendTo($combi_sel_list);
				}
				return $time_unit_comp
			}
		}

		$("<div>").addClass("valid_hint_component").attr({"data-valid_hint":"timeset"}).hide().appendTo($timeset_cntr);

		$.each(_data.accesstime_struct, function(index, item){
			if(index == 0){
				$quickset_comp.children().removeClass("clicked");
				var $sel_quickset_comp = $quickset_comp.find("[value="+item.seconds+"]");
				if($sel_quickset_comp.length > 0){
					$sel_quickset_comp.click();
				}
				else{
					if(item.seconds == 0)
						item.seconds = custom_default;
					$quickset_comp.find("[data-component=custom]").attr({"value":item.seconds}).click();
					var custom_time = secondsToHMS(item.seconds);
					$timeset_cntr.find(".combi_input.hour").val(num_add_left_pad(custom_time.hours, 2));
					$timeset_cntr.find(".combi_input.minute").val(num_add_left_pad(custom_time.minutes, 2));

					function secondsToHMS(sec){
						var sec = Number(sec);
						var h = Math.floor(sec / (60 * 60));
						var m = Math.floor(sec % (60 * 60) / 60);
						var s = Math.floor(sec % (60 * 60) % 60);
						var result = {"hours": 0, "minutes": 0, "seconds": 0};
						result.hours = h > 0 ? h : 0;
						result.minutes = m > 0 ? m : 0;
						result.seconds = s > 0 ? s : 0;
						return result;
					}
				}
			}
		});

		if(_data.accesstime_struct.length == 0){
			$quickset_comp.children().removeClass("clicked").filter("[value=7200]").click();
		}

		return $container;
	}
	function Get_Schedule_Mode_Container(){
		var $container = $("<div>").addClass("radio_options_container").attr({"data-container":"mode"});
		$.each(_mode_mapping, function(index, item){
			var $radio_container = $("<div>").addClass("radio_container").attr({"value":item.value}).appendTo($container)
				.unbind("click").click(function(e){
					e = e || event;
					e.stopPropagation();
					var mode = $(this).attr("value");
					_config.mode = parseInt(mode);
					var $top_schedule_ui = $(this).closest(".schedule_ui");
					$top_schedule_ui.find("[data-viewport=schedule], [data-viewport=access_time]").hide();
					if(mode == "1")
						$top_schedule_ui.find("[data-viewport=schedule]").show();
					else if(mode == "2")
						$top_schedule_ui.find("[data-viewport=access_time]").show();
					var $radio_options_cntr = $(this).closest(".radio_options_container");
					$radio_options_cntr.find(".radio_container").removeClass("clicked").filter($(this)).addClass("clicked");
					$radio_options_cntr.find(".icon_radio").removeClass("clicked").filter($(this).find(".icon_radio")).addClass("clicked");
					if(_config.show_timeset_viewport_callback != null){
						_config.show_timeset_viewport_callback();
					}
				});
			$("<div>").addClass("icon_radio").appendTo($radio_container);
			$("<div>").html(htmlEnDeCode.htmlEncode(item.text)).appendTo($radio_container);
		});

		return $container;
	}
	function check_rule_full(){
		var current_data_num = 0;
		Object.keys(_data.cla_struct).forEach(function(key){
			current_data_num += _data.cla_struct[key].length;
		});
		var total_rule_num = _config.others_rule_num + current_data_num;
		if(total_rule_num + 1 > _config.data_max)
			return true;
		else
			return false;
	}
	function delete_data_cla_struct(_id){
		for (var prop in _data.cla_struct){
			if(!_data.cla_struct[prop].forEach) continue;

			delete_array_by_property(_data.cla_struct[prop], "id", _id);
		}
	}
	function delete_array_by_property(array, property, value){
		$.each(array, function(index, element){
			if(element[property] == value){
				array.splice(index, 1);
				return false;
			}
		});
	}
	function get_rule_count(){
		var count = 0;
		Object.keys(_data.cla_struct).forEach(function(key){
			count += _data.cla_struct[key].length;
		});
		return count;
	}
	function get_specific_timeset_data(_id){
		var timeset_data = {};
		for (var prop in _data.cla_struct){
			if(!_data.cla_struct[prop].forEach) continue;

			_data.cla_struct[prop].forEach(function(element){
				if(element.id == _id){
					timeset_data = JSON.parse(JSON.stringify(element));
				}
			})
		}
		return timeset_data;
	}
	function get_week_title(_weekNum){
		var weekNum = parseInt(_weekNum);
		var weekTitle = "";
		$.each(_weekday_mapping, function(index, value){
			var weekday_obj = value;
			if(weekNum >> weekday_obj.bitwise & 1){
				if(weekTitle != "")
					weekTitle += " / "
				weekTitle += weekday_obj.text.substr(0,3).toUpperCase();
			}
		});
		return weekTitle
	}
	function init_timeset_data(_obj){
		reset_combination_select_status(_obj);
		$(_obj).find(".schedule_timeset_container").attr({"data-timeset-id":"new"});
		$(_obj).find(".combi_input.start.hour").val(num_add_left_pad(_time_unit_parm["val"]["hour"]["start"], 2));
		$(_obj).find(".combi_input.start.minute").val(num_add_left_pad(_time_unit_parm["val"]["minute"]["start"], 2));
		$(_obj).find(".combi_input.end.hour").val(num_add_left_pad(_time_unit_parm["val"]["hour"]["end"], 2));
		$(_obj).find(".combi_input.end.minute").val(num_add_left_pad(_time_unit_parm["val"]["minute"]["end"], 2));
		$(_obj).find(".valid_hint_component").hide().empty();
		$(_obj).find(".combi_input").attr("disabled", false);
		$(_obj).find(".combination_input_select").attr({"data-control":"activate"}).removeClass("focus");
		$(_obj).find(".all_day_component .cb_icon").removeClass("clicked");
		$(_obj).find(".weekdayset_component > div").removeClass("clicked");
	}
	function reset_combination_select_status(_obj){
		$(_obj).find(".combi_sel_list").each(function(index, item){
			if(!($(this).hasClass("combi_sel_list_hide")))
				$(this).toggleClass("combi_sel_list_hide");
		});
		$(_obj).find(".combi_sel").each(function(index, item){
			if($(this).hasClass("arrow_active"))
				$(this).toggleClass("arrow_active");
		});
	}
	function set_current_weekday(_obj){
		var $top_schedule_ui = $(_obj).closest(".schedule_ui");
		var weekday_list = $top_schedule_ui.find("[data-viewport=schedule_timeset] .weekdayset_component > div");
		weekday_list.each(function(i, obj){
			var weekday_bitwise = parseInt($(obj).attr("weekday"));
			if(_current_weekday.weekday >> weekday_bitwise & 1)
				$(obj).addClass("clicked");
		});
	}
	function show_timeset_viewport(_obj){
		var $top_schedule_ui = $(_obj).closest(".schedule_ui");
		$top_schedule_ui.find("[data-viewport=schedule_main]").hide();
		$top_schedule_ui.find("[data-viewport=schedule_timeset]").show();
		init_timeset_data($top_schedule_ui.find("[data-viewport=schedule_timeset]"));
		if(_config.show_timeset_viewport_callback != null){
			_config.show_timeset_viewport_callback();
		}
	}
	function sort_data_cla_struct(){
		var sortWeekdayAndTime = function(a,b){
			var w1 = parseInt(a.weekday);
			var w2 = parseInt(b.weekday);
			var s1 = a.start_hour*60 + a.start_min;
			var s2 = b.start_hour*60 + b.start_min;
			var e1 = a.end_hour*60 + a.end_min;
			var e2 = b.end_hour*60 + b.end_min;
			if (w1 < w2) return -1;
			if (w1 > w2) return 1;
			if (s1 < s2) return -1;
			if (s1 > s2) return 1;
			if (e1 < e2) return -1;
			if (e1 > e2) return 1;
			return 0;
		};
		if(_data.cla_struct["Daily"].length > 0)
			_data.cla_struct["Daily"].sort(sortWeekdayAndTime);
		if(_data.cla_struct["Weekdays"].length > 0)
			_data.cla_struct["Weekdays"].sort(sortWeekdayAndTime);
		if(_data.cla_struct["Weekend"].length > 0)
			_data.cla_struct["Weekend"].sort(sortWeekdayAndTime);
		if(_data.cla_struct["Other"].length > 0)
			_data.cla_struct["Other"].sort(sortWeekdayAndTime);
	}
	function transform_cla_struct_to_day_struct(){
		Object.keys(_data.cla_struct).forEach(function(key) {
			$.each(_data.cla_struct[key], function(index, value){
				var timeset_data = value;
				var weekNum = timeset_data.weekday;
				var alternate_days_data = "";

				var start_num = timeset_data.start_hour*60 + timeset_data.start_min;
				var end_num = timeset_data.end_hour*60 + timeset_data.end_min;
				if(start_num >= end_num){
					alternate_days_data = JSON.parse(JSON.stringify(timeset_data));
					alternate_days_data.start_hour = 0;
					alternate_days_data.start_min = 0;
					alternate_days_data.end_hour = timeset_data.end_hour;
					alternate_days_data.end_min = timeset_data.end_min;
					timeset_data.end_hour = 24;
					timeset_data.end_min = 0;
				}
				$.each(_weekday_mapping, function(index, value){
					var weekday_obj = value;
					if(weekNum >> weekday_obj.bitwise & 1){
						timeset_data.weekday = parseInt(weekday_obj.bitwise);
						_data.day_struct["w_" + timeset_data.weekday + ""].push(JSON.parse(JSON.stringify(timeset_data)));
						if(typeof alternate_days_data == "object"){
							var alternate_days_weekday = (((timeset_data.weekday + 1) > 6) ? 0 : (timeset_data.weekday + 1));
							alternate_days_data.weekday = alternate_days_weekday;
							_data.day_struct["w_" + alternate_days_weekday + ""].push(JSON.parse(JSON.stringify(alternate_days_data)));
						}
					}
				});
			});
		});
	}
	function transform_cla_struct_to_string(){
		var result = "";
		Object.keys(_data.cla_struct).forEach(function(key) {
			$.each(_data.cla_struct[key], function(index, value){
				var timeset_data = value;
				var enable = timeset_data.enable;
				var weekday = num_add_left_pad(((parseInt(timeset_data.weekday)).toString(16).toUpperCase()), 2);
				var start_hour = num_add_left_pad(timeset_data.start_hour, 2);
				var start_min = num_add_left_pad(timeset_data.start_min, 2);
				var end_hour = num_add_left_pad(timeset_data.end_hour, 2);
				var end_min = num_add_left_pad(timeset_data.end_min, 2);
				if(result != "")
					result += "<";
				result += _config.schedule_type;
				result += enable;
				result += weekday;
				result += start_hour;
				result += start_min;
				result += end_hour;
				result += end_min;
			});
		});
		return result;
	}
	function update_sel_timeset_data(_obj, _timeset_data){
		var $top_schedule_ui = $(_obj).closest(".schedule_ui");
		$top_schedule_ui.find(".schedule_timeset_container").attr({"data-timeset-id":_timeset_data.id});

		$top_schedule_ui.find(".combi_input.start.hour").val(num_add_left_pad(_timeset_data.start_hour, 2));
		$top_schedule_ui.find(".combi_input.start.minute").val(num_add_left_pad(_timeset_data.start_min, 2));
		$top_schedule_ui.find(".combi_input.end.hour").val(num_add_left_pad(_timeset_data.end_hour, 2));
		$top_schedule_ui.find(".combi_input.end.minute").val(num_add_left_pad(_timeset_data.end_min, 2));

		var weekday_list = $top_schedule_ui.find("[data-viewport=schedule_timeset] .weekdayset_component > div");
		weekday_list.each(function(i, obj){
			var weekday_bitwise = parseInt($(obj).attr("weekday"));
			if(_timeset_data.weekday >> weekday_bitwise & 1)
				$(obj).addClass("clicked");
		});
	}
};
var schedule_handle_data = {
	"timeset_attr": function(){
		this.type = "W";
		this.enable = 1;
		this.weekday = 0;
		this.start_hour = 0;
		this.start_min = 0;
		this.end_hour = 24;
		this.end_min = 0;
	},
	"string_to_json_array": function(_dataString){
		var result = [];
		if(_dataString != ""){
			var splitData = _dataString.split("<");
			$.each(splitData, function(index, value){
				if(value.length == 12){
					var timeset_data = JSON.parse(JSON.stringify(new schedule_handle_data.timeset_attr()));
					timeset_data.type = value.substr(0,1).toUpperCase();
					timeset_data.enable = parseInt(value.substr(1,1));
					timeset_data.weekday = parseInt(value.substr(2,2), 16);
					timeset_data.start_hour = parseInt(value.substr(4,2));
					timeset_data.start_min = parseInt(value.substr(6,2));
					timeset_data.end_hour = parseInt(value.substr(8,2));
					timeset_data.end_min = parseInt(value.substr(10,2));
					result.push(timeset_data);
				}
			});
		}
		return result;
	},
	"json_array_to_string": function(_jsonArr){
		var result = "";
		$.each(_jsonArr, function(index, value){
			var timeset_data = value;
			if(result != "")
				result += "<";
			result += timeset_data.type.toString().toUpperCase();
			result += timeset_data.enable;
			result += num_add_left_pad(((parseInt(timeset_data.weekday)).toString(16).toUpperCase()), 2);
			result += num_add_left_pad(timeset_data.start_hour, 2);
			result += num_add_left_pad(timeset_data.start_min, 2);
			result += num_add_left_pad(timeset_data.end_hour, 2);
			result += num_add_left_pad(timeset_data.end_min, 2);
		});
		return result;
	}
};
var accesstime_handle_data = {
	"timeset_attr": function(){
		this.start_timestamp = "";
		this.end_timestamp = "";
	},
	"string_to_json_array": function(_dataString){
		var result = [];
		if(_dataString != ""){
			var splitData = _dataString.split("<");
			$.each(splitData, function(index, accesstime){
				var splitAccTime = accesstime.split(",");
				if(splitAccTime[0] != undefined && splitAccTime[1] != undefined){
					if(splitAccTime[0].length == 13 && splitAccTime[1].length == 13){
						var timeset_data = JSON.parse(JSON.stringify(new accesstime_handle_data.timeset_attr()));
						timeset_data.start_timestamp = splitAccTime[0];
						timeset_data.end_timestamp = splitAccTime[1];
						result.push(timeset_data);
					}
				}
			});
		}
		return result;
	},
	"json_array_to_string": function(_jsonArr){
		var result = "";
		$.each(_jsonArr, function(index, value){
			var timeset_data = value;
			if(result != "")
				result += "<";
			result += timeset_data.start_timestamp
			result += ",";
			result += timeset_data.end_timestamp;
		});
		return result;
	}
};
function num_add_left_pad(_num, _len){
	if(_len == undefined) _len = 2;
	return ("0".repeat(_len) + _num).slice(-_len);
}
