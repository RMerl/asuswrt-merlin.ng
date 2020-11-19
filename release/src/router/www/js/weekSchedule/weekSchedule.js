var weekScheduleApi = {
	"obj_id" : "",
	"data" : [],
	"data_max" : 64,
	"weekday_mapping" : [
		{"bitwise":0, "text":"Sunday"},
		{"bitwise":1, "text":"Monday"},
		{"bitwise":2, "text":"Tuesday"},
		{"bitwise":3, "text":"Wednesday"},
		{"bitwise":4, "text":"Thursday"},
		{"bitwise":5, "text":"Friday"},
		{"bitwise":6, "text":"Saturday"}
	],
	"demarcation_hour" : 4,
	"current_time_interval" : false,
	"callback_btn_cancel": "",
	"callback_btn_apply": "",
	"offtime_attr" : function(){
		this.enable = 1;
		this.weekday = 0;
		this.start_hour = 0;
		this.start_min = 0;
		this.end_hour = 24;
		this.end_min = 0;
	},
	"init_data" : function(_jsonObj){
		$.each(weekScheduleApi.weekday_mapping, function( index, value ) {
			var weekday_obj = value;
			weekScheduleApi.data["w_" + weekday_obj.bitwise] = [];//initial data obj
		});
		Object.keys(_jsonObj).forEach(function(key) {
			var weekday_idx = key;
			Object.keys(_jsonObj[weekday_idx]).forEach(function(key) {
				if(typeof _jsonObj[weekday_idx][key] == "object"){
					var enable = parseInt(_jsonObj[weekday_idx][key].enable);
					var weekday =  parseInt(weekday_idx.replace("w_", ""));
					var start_hour = parseInt(_jsonObj[weekday_idx][key].start_hour);
					var start_min = parseInt(_jsonObj[weekday_idx][key].start_min);
					var end_hour = parseInt(_jsonObj[weekday_idx][key].end_hour);
					var end_min = parseInt(_jsonObj[weekday_idx][key].end_min);
					var offtime_obj = new weekScheduleApi.offtime_attr();
					offtime_obj.enable = enable;
					offtime_obj.weekday = weekday;
					offtime_obj.start_hour = start_hour;
					offtime_obj.start_min = start_min;
					offtime_obj.end_hour = end_hour;
					offtime_obj.end_min = end_min;
					weekScheduleApi.data[weekday_idx].push(JSON.parse(JSON.stringify(offtime_obj)));
				}
			});
		});
	},
	"init_layout" : function(_objID){
		$("#" + weekScheduleApi.obj_id + "").empty();
		weekScheduleApi.obj_id = _objID;
		$("#" + weekScheduleApi.obj_id + "").append(weekScheduleApi.overview_a_week_component());
		var abbreviation_weekday = JS_timeObj.toString().substring(0,3).toUpperCase();
		var specific_weekday = weekScheduleApi.weekday_mapping.filter(function(item, index, _array){
			return (item.text.substr(0,3).toUpperCase() == abbreviation_weekday);
		})[0];
		var current_weekday_index = specific_weekday.bitwise;
		$("#" + weekScheduleApi.obj_id + "").find(".weekday_title").eq(current_weekday_index).addClass("current_weekday");

		var demarcation_hour_array = [];
		for(var i = weekScheduleApi.demarcation_hour; i < 24; i += weekScheduleApi.demarcation_hour)
			demarcation_hour_array.push(i);
		var $all_weekday_bg = $("#" + weekScheduleApi.obj_id + "").find(".overview_a_week_bg .weekday_time_bg");
		if($all_weekday_bg.length > 0){
			$.each(demarcation_hour_array, function( index, value ) {
				$all_weekday_bg.append(weekScheduleApi.overview_demarcation_line_component(value, $all_weekday_bg.width()));//demarcation_line
			});
			$("#" + weekScheduleApi.obj_id + "").prepend(weekScheduleApi.overview_time_title_component(demarcation_hour_array, $all_weekday_bg.width()));//time title text
		}
		var $offtime_schedule_text_bg = $("<div>").addClass("offtime_schedule_text_bg");
		$offtime_schedule_text_bg.append(weekScheduleApi.get_offtime_schedule_text()).append(weekScheduleApi.get_offtime_schedule_dis_text());
		$("#" + weekScheduleApi.obj_id + "").append($offtime_schedule_text_bg);
		$("#" + weekScheduleApi.obj_id + "").append(weekScheduleApi.get_action_btn());

		Object.keys(weekScheduleApi.data).forEach(function(key) {
			var weekday = key;
			$("#" + weekScheduleApi.obj_id + "").find("#" + weekday).find(".offtime_bg").remove();
			var weekday_obj = weekScheduleApi.data[weekday];
			$.each(weekday_obj, function( index, value ) {
				var offtime_obj = value
				weekScheduleApi.set_offtime(offtime_obj);
			});
		});

		var $popup_edit_weekSchedule = $("<div>");
		$popup_edit_weekSchedule.addClass("popup_edit_weekSchedule");
		$popup_edit_weekSchedule.attr({"id": "popup_edit_weekSchedule", "onselectstart":false});
		$("#" + weekScheduleApi.obj_id + "").append($popup_edit_weekSchedule);
		$("#" + weekScheduleApi.obj_id + "").prepend($("<div>").addClass("current_time_text"));

		var get_current_time_text = function(){
			JS_timeObj.setTime(systime_millsec);
			systime_millsec += 1000;
			JS_timeObj2 = JS_timeObj.toString();
			JS_timeObj2 = JS_timeObj2.substring(0,3) + ", " +
			JS_timeObj2.substring(4,10) + "  " +
			checkTime(JS_timeObj.getHours()) + ":" +
			checkTime(JS_timeObj.getMinutes()) + ":" +
			checkTime(JS_timeObj.getSeconds()) + "  " +
			JS_timeObj.getFullYear();
			return JS_timeObj2;
		}
		$(".current_time_text").html(get_current_time_text());
		clearInterval(weekScheduleApi.current_time_interval);
		weekScheduleApi.current_time_interval = setInterval(function(){
			$(".current_time_text").html(get_current_time_text());
		},1000);
	},
	"set_offtime" : function(_parmObj){
		var $all_weekday_bg = $("#" + weekScheduleApi.obj_id + "").find(".overview_a_week_bg .weekday_time_bg");
		if($all_weekday_bg.length > 0)
			$all_weekday_bg.eq(_parmObj.weekday).append(weekScheduleApi.overview_offtime_component(_parmObj, $all_weekday_bg.width()));
	},
	"overview_a_week_component" : function(_controlMode) {
		var $overview_a_week_bg = $("<div>");
		$overview_a_week_bg.addClass("overview_a_week_bg");
		$.each(weekScheduleApi.weekday_mapping, function( index, value ) {
			var weekday_obj = value;
			var weekday_text = weekday_obj.text.substr(0,3).toUpperCase();
			var $weekday_bg = $("<div>");
			$weekday_bg.appendTo($overview_a_week_bg);
			$weekday_bg.addClass("weekday_bg");
			var $weekday_title = $("<div>");
			$weekday_title.appendTo($weekday_bg);
			$weekday_title.addClass("weekday_title");
			$weekday_title.html(weekday_text);
			var $weekday_time_bg = $("<div>");
			$weekday_time_bg.appendTo($weekday_bg);
			$weekday_time_bg.addClass("weekday_time_bg");
			$weekday_time_bg.attr("id", "w_" + weekday_obj.bitwise + "");
			if(_controlMode == undefined || _controlMode != "view"){
				$weekday_time_bg.unbind("click");
				$weekday_time_bg.click(function(e){
					e = e || event;
					e.stopPropagation();
					var weekday = $(this).attr("id").replace("w_", "");
					$("#popup_edit_weekSchedule").empty();
					if(weekScheduleApi.data[$(this).attr("id")].length == 0){
						var offtime_obj = new weekScheduleApi.offtime_attr();
						offtime_obj.weekday = parseInt(weekday);
						var click_hour = Math.floor(((e.offsetX/$(this).width())*(60*24))/60);
						var start_hour = 0;
						var end_hour = 24;
						if(click_hour == 24)//click 24:00 border
							click_hour = 20;
						if(click_hour != 0){
							start_hour = (Math.floor(click_hour/weekScheduleApi.demarcation_hour))*weekScheduleApi.demarcation_hour;
							end_hour = start_hour + weekScheduleApi.demarcation_hour;
						}
						else{//click demarcation border, when click_hour = 0
							start_hour = 0;
							end_hour = 24;
						}
						offtime_obj.start_hour = start_hour;
						offtime_obj.end_hour = end_hour;
						$("#popup_edit_weekSchedule").append(weekScheduleApi.edit_time_setting_component("new", offtime_obj));
					}
					else
						$("#popup_edit_weekSchedule").append(weekScheduleApi.edit_time_period_list_component(weekday, weekScheduleApi.data[$(this).attr("id")]));

					$("#popup_edit_weekSchedule").fadeIn();
					adjust_panel_block_top("popup_edit_weekSchedule", 100);
					$("#popup_edit_weekSchedule").unbind("click");
					$("#popup_edit_weekSchedule").click(function(e){
						e = e || event;
						e.stopPropagation();
						weekScheduleApi.reset_custom_select_status();
					});
				});
			}
			else
				$weekday_time_bg.css("cursor", "initial");
		});

		return $overview_a_week_bg;
	},
	"overview_demarcation_line_component" : function(_hour, _contentWidth) {
		var left = Math.round(((_hour*60)/(24*60))*_contentWidth);// (hour*60(minute)/(24*60))*div width
		var $demarcation_line = $("<div>");
		$demarcation_line.addClass("demarcation_line");
		$demarcation_line.css("left", left);
		$demarcation_line.attr("demarcation_hour", _hour);
		return $demarcation_line;
	},
	"overview_time_title_component" : function(_time_array, _contentWidth) {
		var $overview_time_title_bg = $("<div>");
		$overview_time_title_bg.addClass("overview_time_title_bg");

		var $space_bg = $("<div>");
		$space_bg.appendTo($overview_time_title_bg);
		$space_bg.addClass("space_bg");

		var $time_title_bg = $("<div>");
		$time_title_bg.appendTo($overview_time_title_bg);
		$time_title_bg.addClass("time_title_bg");

		var $first_time_text = $("<span>");
		$first_time_text.appendTo($time_title_bg);
		$first_time_text.html("00:00");
		$first_time_text.addClass("first_time_text");

		var $last_time_text = $("<span>");
		$last_time_text.appendTo($time_title_bg);
		$last_time_text.html("24:00");
		$last_time_text.addClass("last_time_text");

		$.each(_time_array, function( index, value ) {
			var left = Math.round((((value*60)/(24*60))*_contentWidth) - 17);
			var $demarcation_time = $("<span>");
			$demarcation_time.appendTo($time_title_bg);
			$demarcation_time.css("left", left);
			var display_time = weekScheduleApi.add_left_pad(value, 2) + ":00";
			$demarcation_time.html(display_time);
		});

		return $overview_time_title_bg;
	},
	"overview_offtime_component" : function(_parmObj, _contentWidth){
		if(_parmObj.start_hour === "" || _parmObj.start_hour === undefined || _parmObj.end_hour === "" || _parmObj.end_hour === undefined || 
			_parmObj.start_min === "" || _parmObj.start_min === undefined || _parmObj.end_min === "" || _parmObj.end_min === undefined || 
			_parmObj.start_hour >_parmObj.end_hour)
			return;
		var start_time = _parmObj.start_hour*60 + _parmObj.start_min;
		var end_time = _parmObj.end_hour*60 + _parmObj.end_min;
		var start = Math.round(((start_time)/(24*60))*_contentWidth);
		var end = Math.round(((end_time)/(24*60))*_contentWidth);
		var width = end - start;
		if(width == 0 || width == 1)//for time period is too short
			width = 2;
		var $offtime_bg = $("<div>");
		$offtime_bg.addClass("offtime_bg");
		if(_parmObj.enable == "0")
			$offtime_bg.addClass("dis");
		$offtime_bg.css({"left":start, "width":width});
		return $offtime_bg;
	},
	"edit_time_setting_component" : function(_editMode, _parmObj, _sche_type){
		var $content_bg = $("<div>");

		var $edit_offtime_title = $("<div>");
		$edit_offtime_title.appendTo($content_bg);
		$edit_offtime_title.addClass("edit_offtime_title");
		$edit_offtime_title.html("<#weekSche_Offtime_Sche#>");

		var $close_btn = $("<div>");
		$close_btn.appendTo($content_bg);
		$close_btn.addClass("close_btn");
		$close_btn.unbind("click");
		$close_btn.click(function(e){
			e = e || event;
			e.stopPropagation();
			$(this).parents(".popup_edit_weekSchedule").fadeOut();
		});

		$content_bg.append("<br>");

		if(_sche_type != "PC"){
			var $selected_weekday_title = $("<div>");
			$selected_weekday_title.appendTo($content_bg);
			$selected_weekday_title.addClass("selected_weekday_title");
			$selected_weekday_title.html(weekScheduleApi.weekday_mapping[_parmObj.weekday].text);
		}

		var $edit_hour_minute_bg = $("<div>");
		$edit_hour_minute_bg.appendTo($content_bg);
		$edit_hour_minute_bg.addClass("edit_hour_minute_bg");

		var get_hour_minute_component = function(_type){
			var $edit_time_setting_bg = $("<div>");
			$edit_time_setting_bg.addClass("edit_time_setting_bg");

			var $time_setting_title = $("<div>");
			$time_setting_title.appendTo($edit_time_setting_bg);
			$time_setting_title.addClass("time_setting_title");
			if(_type == "start")
				$time_setting_title.html("<span><#weekSche_Start_Time#></span>");
			else
				$time_setting_title.html("<span><#weekSche_End_Time#></span>");

			var get_time_setting_component = function(_type, _unit){
				var time_unit_parm = {
					"text":{"hour":"<#Hour#>", "minute":"<#Minute#>"},
					"interval":{"hour":1, "minute":15},
					"end_time":{"hour":24, "minute":59},
				}
				var $content_bg = $("<div>");
			
				var $time_setting_unit_title = $("<div>");
				$time_setting_unit_title.appendTo($content_bg);
				$time_setting_unit_title.addClass("time_setting_unit_title");
				$time_setting_unit_title.html(time_unit_parm["text"][_unit]);

				var $custom_select_input_bg = $("<div>");
				$content_bg.append($custom_select_input_bg);
				$time_setting_unit_title.appendTo($content_bg);
				$custom_select_input_bg.addClass("custom_select_input_bg");

				var $input_obj = $("<input/>");
				$input_obj.appendTo($custom_select_input_bg);
				$input_obj.addClass("custom_input " + _type + " " + _unit + "");
				var value_idx = _type + "_" + ((_unit == "minute") ? "min" : _unit);
				var default_value = _parmObj[value_idx];
				var input_obj_value = weekScheduleApi.add_left_pad(default_value, 2);
				$input_obj.val(input_obj_value);
				$input_obj.attr({"type" : "text", "maxlength" : 2, "autocomplete" : "off", "autocorrect" : "off", "autocapitalize" : "off"});
				$input_obj.keypress(function(){
					return validator.isNumber(this,event);
				});
				$input_obj.blur(function(){
					var value = $.trim($(this).val());
					$(this).val(value);
					var show_valid_hint = function(_hint, _$this){
						_$this.parents(".edit_hour_minute_bg").find(".custom_input").attr("disabled", true);
						_$this.parents(".edit_hour_minute_bg").find(".custom_select").removeClass("deactivate activate").addClass("deactivate");
						_$this.attr("disabled", false);
						_$this.siblings(".custom_select").toggleClass("deactivate activate");
						_$this.siblings(".valid_hint").html(_hint).show();
						_$this.siblings(".custom_select").addClass("focus");
					};
					if(value.length == 0){
						show_valid_hint("<#JS_fieldblank#>", $(this));
						return false;
					}
					if(isNaN(value) || (_unit == "hour" && (parseInt(value) > 24)) || (_unit == "minute" && (parseInt(value) > 59))  ){
						show_valid_hint("<#FirewallConfig_URLActiveTime_itemhint2#>", $(this));
						return false;
					}

					$(this).parents(".edit_hour_minute_bg").find(".custom_input").attr("disabled", false);
					$(this).parents(".edit_hour_minute_bg").find(".custom_select").removeClass("deactivate activate focus").addClass("activate");
					$(this).siblings(".valid_hint").hide().empty();

					$(this).val(weekScheduleApi.add_left_pad($(this).val(), 2));
				});

				var $custom_select = $("<div>");
				$custom_select.appendTo($custom_select_input_bg);
				$custom_select.addClass("custom_select activate");
				$custom_select.unbind("click");
				$custom_select.click(function(e){
					e = e || event;
					e.stopPropagation();
					if($custom_select.hasClass("activate")){
						if($(this).siblings().hasClass("custom_select_hide")){//click first time or not click again
							$("#popup_edit_weekSchedule").find(".custom_select_list").each(function(index, item) {
								if(!($(this).hasClass("custom_select_hide")))
									$(this).toggleClass("custom_select_hide");
							});
							$("#popup_edit_weekSchedule").find(".custom_select").each(function(index, item) {
								if($(this).hasClass("arrow_active"))
									$(this).toggleClass("arrow_active");
							});
							$(this).siblings(".custom_select_hide").toggleClass("custom_select_hide");
						}
						else//click again
							$(this).siblings(".custom_select_list").toggleClass("custom_select_hide");

						$(this).toggleClass("arrow_active");
					}
				});

				var $custom_select_list = $("<div>");
				$custom_select_list.appendTo($custom_select_input_bg);
				$custom_select_list.addClass("custom_select_list custom_select_hide");

				var end_time = time_unit_parm["end_time"][_unit];
				var interval =  time_unit_parm["interval"][_unit];
				for(var idx = 0; idx <= end_time; idx += interval) {
					var optionText = weekScheduleApi.add_left_pad(idx, 2);
					var optionValue = idx;
					var $custom_select_option = $("<div>");
					$custom_select_option.appendTo($custom_select_list);
					$custom_select_option.addClass("custom_select_option");
					$custom_select_option.attr("value", optionValue);
					$custom_select_option.html(optionText);
					$custom_select_option.unbind("click");
					$custom_select_option.click(function(e){
						var select_value = $(this).attr("value");
						var input_obj_value = weekScheduleApi.add_left_pad(select_value, 2);
						$input_obj.val(input_obj_value);
						$(this).parent().toggleClass("custom_select_hide");
						$(this).parent().siblings(".custom_select").toggleClass("arrow_active");
						if($(this).parent().siblings(".custom_select").hasClass("focus")){
							$(this).parent().siblings(".custom_select").toggleClass("focus");
							$(this).parent().siblings(".valid_hint").hide().empty();

							$(this).parents(".edit_hour_minute_bg").find(".custom_input").attr("disabled", false);
							$(this).parents(".edit_hour_minute_bg").find(".custom_select").removeClass("deactivate activate focus").addClass("activate");
						}
					});
				}

				var valid_hint = $("<span>");
				valid_hint.appendTo($custom_select_input_bg);
				valid_hint.addClass("valid_hint");

				return  $content_bg;
			}

			$edit_time_setting_bg.append(get_time_setting_component(_type, "hour"));
			$edit_time_setting_bg.append(get_time_setting_component(_type, "minute"));

			return $edit_time_setting_bg;
		};
		
		$edit_hour_minute_bg.append(get_hour_minute_component("start"));
		$edit_hour_minute_bg.append(get_hour_minute_component("end"));

		var $all_day_bg = $("<div>");
		$all_day_bg.appendTo($edit_hour_minute_bg);
		$all_day_bg.addClass("all_day_bg");

		var $all_day_checkbox = $("<div>");
		$all_day_checkbox.appendTo($all_day_bg);
		$all_day_checkbox.addClass("all_day_checkbox");
		$all_day_checkbox.unbind("click");
		$all_day_checkbox.click(function(e){
			e = e || event;
			e.stopPropagation();
			weekScheduleApi.reset_custom_select_status();
			$(this).toggleClass("clicked");
			if($(this).hasClass("clicked")){
				$(this).parents(".edit_hour_minute_bg").find(".custom_input.start.hour").val(weekScheduleApi.add_left_pad(0, 2));
				$(this).parents(".edit_hour_minute_bg").find(".custom_input.start.minute").val(weekScheduleApi.add_left_pad(0, 2));
				$(this).parents(".edit_hour_minute_bg").find(".custom_input.end.hour").val(weekScheduleApi.add_left_pad(24, 2));
				$(this).parents(".edit_hour_minute_bg").find(".custom_input.end.minute").val(weekScheduleApi.add_left_pad(00, 2));
				$(this).parents(".edit_hour_minute_bg").find(".custom_input").attr("disabled", true);
				$(this).parents(".edit_hour_minute_bg").find(".custom_select").removeClass("deactivate activate focus").addClass("deactivate");
				$(this).parents(".edit_hour_minute_bg").find(".valid_hint").hide().empty();
			}
			else{
				$(this).parents(".edit_hour_minute_bg").find(".custom_input").attr("disabled", false);
				$(this).parents(".edit_hour_minute_bg").find(".custom_select").removeClass("deactivate activate").addClass("activate");
			}
		});

		var $all_day_text = $("<div>");
		$all_day_text.appendTo($all_day_bg);
		$all_day_text.addClass("all_day_text");
		$all_day_text.html("<#weekSche_All_Day#>");

		$edit_hour_minute_bg.append($("<div>").addClass("horizontal_line"));

		var $routine_schedule_title = $("<div>");
		$routine_schedule_title.appendTo($edit_hour_minute_bg);
		$routine_schedule_title.html("<#weekSche_Schedule#>");
		if(_editMode == "edit" && _sche_type != "PC")
			$routine_schedule_title.hide();

		var $routine_schedule_bg = $("<div>");
		$routine_schedule_bg.appendTo($edit_hour_minute_bg);
		$routine_schedule_bg.addClass("routine_schedule_bg");
		$.each(weekScheduleApi.weekday_mapping, function( index, value ) {
			var weekday_obj = value;
			var abbreviation_weekday = weekday_obj.text.substr(0,3).toUpperCase();
			var $edit_schedule_week = $("<div>");
			$edit_schedule_week.appendTo($routine_schedule_bg);
			 $edit_schedule_week.attr("weekday", index);
			$edit_schedule_week.html(abbreviation_weekday);
			if(_sche_type != "PC"){
				if(index == _parmObj.weekday){
					$edit_schedule_week.addClass("clicked");
					$edit_schedule_week.addClass("current_weekday");
					$edit_schedule_week.css("cursor", "initial");
				}
			}
			else{
				var weekNum = parseInt(_parmObj.weekday);
				if(weekNum >> weekday_obj.bitwise & 1)
					$edit_schedule_week.addClass("clicked");
			}

			if(index != _parmObj.weekday || _sche_type == "PC"){
				$edit_schedule_week.unbind("click");
				$edit_schedule_week.click(function(e){
					e = e || event;
					e.stopPropagation();
					weekScheduleApi.reset_custom_select_status();
					$(this).toggleClass("clicked");
				});
			}
		});

		if(_editMode == "edit" && _sche_type != "PC")
			$routine_schedule_bg.hide().children("div").unbind("click");

		$edit_done_btn = $("<div>");
		$edit_hour_minute_bg.append($edit_done_btn);
		$edit_done_btn.addClass("button_gen edit_done_btn");
		$edit_done_btn.html("<#CTL_finish#>");
		$edit_done_btn.unbind("click");
		$edit_done_btn.click(function(e){
			e = e || event;
			e.stopPropagation();
			var deactivate_len = $(this).parents(".edit_hour_minute_bg").find(".custom_select.deactivate").length;
			var legal_flag = (deactivate_len == 4 || deactivate_len == 0) ? true : false;//4 is mean have click all day, 0 means format not error.
			var start_hour = parseInt($(this).parents(".edit_hour_minute_bg").find(".custom_input.start.hour").val());
			var start_min = parseInt($(this).parents(".edit_hour_minute_bg").find(".custom_input.start.minute").val());
			var end_hour = parseInt($(this).parents(".edit_hour_minute_bg").find(".custom_input.end.hour").val());
			var end_min = parseInt($(this).parents(".edit_hour_minute_bg").find(".custom_input.end.minute").val());
			if(isNaN(start_hour) || isNaN(start_min) || isNaN(end_hour) || isNaN(end_min))
				legal_flag = false;

			if(_sche_type != "PC"){
				var start_num = start_hour*60 + start_min;
				var end_num = end_hour*60 + end_min;
				if(start_num >= end_num)
					legal_flag = false;
				if(end_num > 1440)//24:00
					legal_flag = false;
			}

			if(legal_flag){
				var click_weekday_list = $(this).parents(".edit_hour_minute_bg").find(".routine_schedule_bg > div.clicked");
				if(_editMode == "new"){
					if(_sche_type != "PC"){
						var current_data_num = 0;
						Object.keys(weekScheduleApi.data).forEach(function(key) {
							var weekday = key;
							var weekday_obj = weekScheduleApi.data[weekday];
							current_data_num += weekday_obj.length;
						});
						if(current_data_num + click_weekday_list.length > weekScheduleApi.data_max){
							popupHint.init();
							var hint = "<#weekSche_MAX_Num#>".replace("#MAXNUM", weekScheduleApi.data_max);
							hint += "<br>";
							hint += "<#weekSche_MAX_Del_Hint#>";
							popupHint.set_text(hint);
							popupHint.set_btn_ok();
							popupHint.hide_btn("cancel");
							return;
						}
					}
				}
				if(_sche_type != "PC"){
					click_weekday_list.each(function(i, obj) {
						var weekday = $(obj).attr("weekday");
						var offtime_obj = new weekScheduleApi.offtime_attr();
						offtime_obj.enable = 1;
						offtime_obj.weekday = parseInt(weekday);
						offtime_obj.start_hour = start_hour;
						offtime_obj.start_min = start_min;
						offtime_obj.end_hour = end_hour;
						offtime_obj.end_min = end_min;

						if(_editMode == "edit")
							weekScheduleApi.data["w_" + weekday][_parmObj.idx] = JSON.parse(JSON.stringify(offtime_obj));
						else
							weekScheduleApi.data["w_" + weekday].push(JSON.parse(JSON.stringify(offtime_obj)));

						$("#" + weekScheduleApi.obj_id + "").find("#w_" + weekday).find(".offtime_bg").remove();
						var weekday_obj = weekScheduleApi.data["w_" + weekday];
						$.each(weekday_obj, function( index, value ) {
							var offtime_obj = value
							weekScheduleApi.set_offtime(offtime_obj);
						});
						$(this).parents(".popup_edit_weekSchedule").fadeOut();
					});
				}
				else{
					var total_weekday = 0;
					click_weekday_list.each(function(i, obj) {
						total_weekday += Math.pow(2, parseInt($(obj).attr("weekday")));
					});
					if(total_weekday == 0){
						popupHint.init();
						var hint = "Week field is blank.";
						popupHint.set_text(hint);
						popupHint.set_btn_ok();
						popupHint.hide_btn("cancel");
						return;
					}
					var ori_enable = _parmObj.enable;
					var ori_id = _parmObj.id;
					var ori_weekday = _parmObj.weekday;
					var PC_offtime_attr = new weekScheduleApi.PC_offtime_attr();
					PC_offtime_attr.weekday = total_weekday;
					PC_offtime_attr.start_hour = start_hour;
					PC_offtime_attr.start_min = start_min;
					PC_offtime_attr.end_hour = end_hour;
					PC_offtime_attr.end_min = end_min;
					if(_editMode == "edit"){
						PC_offtime_attr.enable = ori_enable;
						PC_offtime_attr.id = ori_id;
					}
					else{
						PC_offtime_attr.enable = 1;
						var id = weekScheduleApi.PC_current_id.toString() + weekScheduleApi.add_left_pad(((parseInt(total_weekday)).toString(16).toUpperCase()), 2) +
						weekScheduleApi.add_left_pad(start_hour, 2) + weekScheduleApi.add_left_pad(start_min, 2) +
						weekScheduleApi.add_left_pad(end_hour, 2) + weekScheduleApi.add_left_pad(end_min, 2);
						PC_offtime_attr.id = id;
						weekScheduleApi.PC_current_id++;
					}
					if(_editMode == "edit"){
						switch(ori_weekday){
							case 127 ://Daily
								weekScheduleApi.PC_remove_offtime(weekScheduleApi.PC_data["Daily"], "id", ori_id);
								break;
							case 62 ://Weekdays
								weekScheduleApi.PC_remove_offtime(weekScheduleApi.PC_data["Weekdays"], "id", ori_id);
								break;
							case 65 ://Weekend
								weekScheduleApi.PC_remove_offtime(weekScheduleApi.PC_data["Weekend"], "id", ori_id);
								break;
							default ://Other
								weekScheduleApi.PC_remove_offtime(weekScheduleApi.PC_data["Other"], "id", ori_id);
								break;
						}
					}

					switch(total_weekday){
						case 127 ://Daily
							if(_editMode == "edit")
							PC_offtime_attr.title = "Daily";
							weekScheduleApi.PC_data["Daily"].push(JSON.parse(JSON.stringify(PC_offtime_attr)));
							break;
						case 62 ://Weekdays
							PC_offtime_attr.title = "Weekdays";
							weekScheduleApi.PC_data["Weekdays"].push(JSON.parse(JSON.stringify(PC_offtime_attr)));
							break;
						case 65 ://Weekend
							PC_offtime_attr.title = "Weekend";
							weekScheduleApi.PC_data["Weekend"].push(JSON.parse(JSON.stringify(PC_offtime_attr)));
							break;
						default ://Other
							PC_offtime_attr.title = weekScheduleApi.PC_get_week_title(total_weekday);
							weekScheduleApi.PC_data["Other"].push(JSON.parse(JSON.stringify(PC_offtime_attr)));
							break;
					}
					$("#" + weekScheduleApi.obj_id + " .offtime_schedule_content_bg").empty();
					$("#" + weekScheduleApi.obj_id + " .offtime_schedule_content_bg").append(weekScheduleApi.PC_get_grid_period_component());
					$(this).parents(".popup_edit_weekSchedule").fadeOut();
				}
			}
			else{
				popupHint.init();
				popupHint.set_text("<#weekSche_format_incorrect#> <#weekSche_check_and_retype#>");
				popupHint.set_btn_ok();
				popupHint.hide_btn("cancel");
			}
		});

		return $content_bg;
	},
	"edit_time_period_list_component" : function(_weekday, _offtime_list){
		var $content_bg = $("<div>");

		var $edit_offtime_title = $("<div>");
		$edit_offtime_title.appendTo($content_bg);
		$edit_offtime_title.addClass("edit_offtime_title");
		$edit_offtime_title.html("<#weekSche_Offtime_Sche#>");

		var $close_btn = $("<div>");
		$close_btn.appendTo($content_bg);
		$close_btn.addClass("close_btn");
		$close_btn.unbind("click");
		$close_btn.click(function(e){
			e = e || event;
			e.stopPropagation();
			$(this).parents(".popup_edit_weekSchedule").fadeOut();
		});

		$content_bg.append("<br>");

		var $selected_weekday_title = $("<div>");
		$selected_weekday_title.appendTo($content_bg);
		$selected_weekday_title.addClass("selected_weekday_title");
		$selected_weekday_title.html(weekScheduleApi.weekday_mapping[_weekday].text);

		var $period_list_bg = $("<div>");
		$period_list_bg.appendTo($content_bg);
		$period_list_bg.addClass("period_list_bg");

		$.each(_offtime_list, function( index, value ) {
			var offtime_obj = value;
			
			var $each_period_bg = $("<div>");
			$each_period_bg.appendTo($period_list_bg);
			$each_period_bg.addClass("each_period_bg");
			$each_period_bg.hover(
				function(){$(this).children(".period_trash_icon").show();},
				function(){$(this).children(".period_trash_icon").hide();}
			);
			$each_period_bg.unbind("click");
			$each_period_bg.click(function(e){
				e = e || event;
				e.stopPropagation();
				$("#popup_edit_weekSchedule").empty();
				offtime_obj.idx = $(this).index();
				$("#popup_edit_weekSchedule").append(weekScheduleApi.edit_time_setting_component("edit", offtime_obj));
			});

			var $period_text = $("<div>");
			$period_text.appendTo($each_period_bg);
			$period_text.addClass("period_text");
			var display_period_text = "";
			display_period_text += weekScheduleApi.add_left_pad(offtime_obj.start_hour, 2);
			display_period_text += ":";
			display_period_text += weekScheduleApi.add_left_pad(offtime_obj.start_min, 2);
			display_period_text += " - ";
			display_period_text += weekScheduleApi.add_left_pad(offtime_obj.end_hour, 2);
			display_period_text += ":";
			display_period_text += weekScheduleApi.add_left_pad(offtime_obj.end_min, 2);
			$period_text.html(display_period_text);
			
			var $period_switch = $("<div>");
			$period_switch.appendTo($each_period_bg);
			$period_switch.addClass("period_switch");
			if(offtime_obj.enable)
				$period_switch.addClass("on");
			else
				$period_switch.addClass("off");
			$period_switch.unbind("click");
			$period_switch.click(function(e){
				e = e || event;
				e.stopPropagation();
				$(this).toggleClass("off  on");
			
				if($(this).hasClass("period_switch on"))
					offtime_obj.enable = 1;
				else
					offtime_obj.enable = 0;

				$("#" + weekScheduleApi.obj_id + "").find("#w_" + _weekday).find(".offtime_bg").remove();
				var weekday_obj = weekScheduleApi.data["w_" + _weekday];
				$.each(weekday_obj, function( index, value ) {
					var offtime_obj = value
					weekScheduleApi.set_offtime(offtime_obj);
				});
			});

			var $period_trash_icon = $("<div>");
			$period_trash_icon.appendTo($each_period_bg);
			$period_trash_icon.addClass("period_trash_icon");
			$period_trash_icon.unbind("click");
			$period_trash_icon.click(function(e){
				e = e || event;
				e.stopPropagation();
				var delete_idx = $(this).parents(".each_period_bg").index();
				$(this).parents(".each_period_bg").remove();
				weekScheduleApi.data["w_" + _weekday].splice(delete_idx, 1);
				$("#" + weekScheduleApi.obj_id + "").find("#w_" + _weekday).find(".offtime_bg").remove();
				var weekday_obj = weekScheduleApi.data["w_" + _weekday];
				$.each(weekday_obj, function( index, value ) {
					var offtime_obj = value
					weekScheduleApi.set_offtime(offtime_obj);
				});
				if(weekScheduleApi.data["w_" + _weekday].length == 0){
					$("#popup_edit_weekSchedule").empty();
					$("#popup_edit_weekSchedule").hide();
					//$("#popup_edit_weekSchedule").append(weekScheduleApi.edit_time_setting_component(_weekday));
				}
			});
			$period_trash_icon.hide();
		});

		var $add_new_offtime = $("<div>");
		$add_new_offtime.appendTo($content_bg);
		$add_new_offtime.addClass("add_new_offtime");
		$("<span>")
			.appendTo($add_new_offtime)
			.html("<#weekSche_Add_New#>")
			.appendTo($add_new_offtime)
			.unbind("click")
			.click(function(e){
				e = e || event;
				e.stopPropagation();
				$("#popup_edit_weekSchedule").empty();
				var offtime_obj = new weekScheduleApi.offtime_attr();
				offtime_obj.weekday = parseInt(_weekday);
				$("#popup_edit_weekSchedule").append(weekScheduleApi.edit_time_setting_component("new", offtime_obj));
			});

		return $content_bg;
	},
	"reset_custom_select_status" : function(){
		$("#popup_edit_weekSchedule").find(".custom_select_list").each(function(index, item) {
			if(!($(this).hasClass("custom_select_hide")))
				$(this).toggleClass("custom_select_hide");
		});
		$("#popup_edit_weekSchedule").find(".custom_select").each(function(index, item) {
			if($(this).hasClass("arrow_active"))
				$(this).toggleClass("arrow_active");
		});
	},
	"add_left_pad" : function(_num, _len){
		if(_len == undefined) _len = 2;
		return ("0".repeat(_len) + _num).slice(-_len); 
	},
	"get_offtime_schedule_text" : function(){
		return $("<div>").addClass("offtime_schedule_text").html("<#weekSche_Offtime_Sche#>");
	},
	"get_offtime_schedule_dis_text" : function(){
		return $("<div>").addClass("offtime_schedule_text dis").html("<#btn_Disabled#> <#weekSche_Offtime_Sche#>");
	},
	"get_action_btn" : function(){
		var $action_btn_bg = $("<div>");
		$action_btn_bg.addClass("action_btn_bg");

		var $clear_all_btn = $("<div>");
		$clear_all_btn.appendTo($action_btn_bg);
		$clear_all_btn.addClass("button_gen left_btn");
		$clear_all_btn.html("<#weekSche_Clear_All#>");
		$clear_all_btn.unbind("click");
		$clear_all_btn.click(function(e){
			e = e || event;
			e.stopPropagation();
			$("#popup_edit_weekSchedule").hide();
			popupHint.init();
			popupHint.set_text("<#weekSche_Del_All#>");
			popupHint.set_btn_ok(function(){
				weekScheduleApi.init_data("");
				$("#" + weekScheduleApi.obj_id + "").find(".offtime_bg").remove();
			});
			popupHint.set_btn_cancel();
		});

		var $cancel_btn = $("<div>");
		$cancel_btn.appendTo($action_btn_bg);
		$cancel_btn.addClass("button_gen right_btn");
		$cancel_btn.html("<#CTL_Cancel#>");
		$cancel_btn.unbind("click");
		$cancel_btn.click(function(e){
			e = e || event;
			e.stopPropagation();
			if(typeof weekScheduleApi.callback_btn_cancel == "function")
				weekScheduleApi.callback_btn_cancel();
		});

		var $apply_btn = $("<div>");
		$apply_btn.appendTo($action_btn_bg);
		$apply_btn.addClass("button_gen right_btn");
		$apply_btn.html("<#CTL_apply#>");
		$apply_btn.unbind("click");
		$apply_btn.click(function(e){
			e = e || event;
			e.stopPropagation();
			if(typeof weekScheduleApi.callback_btn_apply == "function")
				weekScheduleApi.callback_btn_apply();
		});

		return $action_btn_bg;
	},
	"PC_data" : [],
	"PC_current_id" : 0,
	"PC_other_client_rule_num" : 0,
	"PC_offtime_attr" : function(){
		this.id = "";
		this.enable = 1;
		this.weekday = 0;
		this.start_hour = 0;
		this.start_min = 0;
		this.end_hour = 24;
		this.end_min = 0;
		this.title = "";
	},
	"PC_init_data" : function(_dataString){
		weekScheduleApi.PC_data = [];
		weekScheduleApi.PC_data["Daily"] = [];
		weekScheduleApi.PC_data["Weekdays"] = [];
		weekScheduleApi.PC_data["Weekend"] = [];
		weekScheduleApi.PC_data["Other"] = [];
		var id = 100;
		if(_dataString != ""){
			var splitData = _dataString.split("<");
			$.each(splitData, function( index, value ) {
				if(value.length == 12){
					var enable = parseInt(value.substr(1,1));
					var weekday =  parseInt(value.substr(2,2), 16);
					var start_hour = parseInt(value.substr(4,2));
					var start_min = parseInt(value.substr(6,2));
					var end_hour = parseInt(value.substr(8,2));
					var end_min = parseInt(value.substr(10,2));
					var PC_offtime_attr = new weekScheduleApi.PC_offtime_attr();
					PC_offtime_attr.id = id.toString() + value.substr(2,10);//100 + index + weekday + start_time + end_time
					id++;
					PC_offtime_attr.enable = enable;
					PC_offtime_attr.weekday = weekday;
					PC_offtime_attr.start_hour = start_hour;
					PC_offtime_attr.start_min = start_min;
					PC_offtime_attr.end_hour = end_hour;
					PC_offtime_attr.end_min = end_min;
					switch(weekday){
						case 127 ://Daily
							PC_offtime_attr.title = "Daily";
							weekScheduleApi.PC_data["Daily"].push(JSON.parse(JSON.stringify(PC_offtime_attr)));
							break;
						case 62 ://Weekdays
							PC_offtime_attr.title = "Weekdays";
							weekScheduleApi.PC_data["Weekdays"].push(JSON.parse(JSON.stringify(PC_offtime_attr)));
							break;
						case 65 ://Weekend
							PC_offtime_attr.title = "Weekend";
							weekScheduleApi.PC_data["Weekend"].push(JSON.parse(JSON.stringify(PC_offtime_attr)));
							break;
						default ://Other
							PC_offtime_attr.title = weekScheduleApi.PC_get_week_title(weekday);
							weekScheduleApi.PC_data["Other"].push(JSON.parse(JSON.stringify(PC_offtime_attr)));
							break;
					}
				}
			});
		}
		weekScheduleApi.PC_current_id = id;
	},
	"PC_init_layout" : function(_objID){
		weekScheduleApi.obj_id = _objID;
		$("#" + weekScheduleApi.obj_id + "").empty();
		$("#" + weekScheduleApi.obj_id + "").addClass("PC_SCHED");
		$("#" + weekScheduleApi.obj_id + "").append(weekScheduleApi.PC_get_level_demarcation_line());
		$("#" + weekScheduleApi.obj_id + "").append(weekScheduleApi.PC_get_header_bg());

		var offtime_count = 0;
		Object.keys(weekScheduleApi.PC_data).forEach(function(key) {
			offtime_count += weekScheduleApi.PC_data[key].length;
		});
		$("#" + weekScheduleApi.obj_id + "").append($("<div>").addClass("offtime_schedule_content_bg gridview_mode"));
		$("#" + weekScheduleApi.obj_id + "").append($("<div>").addClass("offtime_overview_content_bg overview_mode"));
		if(offtime_count == 0)
			$("#" + weekScheduleApi.obj_id + " .offtime_schedule_content_bg").append(weekScheduleApi.PC_get_no_data_component());
		else
			$("#" + weekScheduleApi.obj_id + " .offtime_schedule_content_bg").append(weekScheduleApi.PC_get_grid_period_component());

		$("#" + weekScheduleApi.obj_id + "").append(weekScheduleApi.PC_get_level_demarcation_line("gridview_mode"));
		$("#" + weekScheduleApi.obj_id + "").append(weekScheduleApi.PC_get_action_btn());

		var $popup_edit_weekSchedule = $("<div>");
		$popup_edit_weekSchedule.addClass("popup_edit_weekSchedule");
		$popup_edit_weekSchedule.attr({"id": "popup_edit_weekSchedule", "onselectstart":false});
		$("#" + weekScheduleApi.obj_id + "").append($popup_edit_weekSchedule);
		$("#" + weekScheduleApi.obj_id + "").find(".overview_mode").hide();
	},
	"PC_get_level_demarcation_line" : function(_class){
		var $demarcation_line = $("<div>");
		$demarcation_line.addClass("level_demarcation_line");
		if(_class)
			$demarcation_line.addClass(_class);
		return $demarcation_line;
	},
	"PC_get_header_bg" : function(){
		var $header_bg = $("<div>").addClass("offtime_schedule_header");
		var $title = $("<div>").addClass("title").html("<#weekSche_Offtime_Sche#>");
		$title.appendTo($header_bg);
		var $add_icon = $("<div>").addClass("btn_icon_bg add_icon gridview_mode");
		$add_icon.appendTo($header_bg);
		$add_icon.unbind("click");
		$add_icon.click(function(e){
			e = e || event;
			e.stopPropagation();
			var current_data_num = 0;
			Object.keys(weekScheduleApi.PC_data).forEach(function(key) {
				current_data_num += weekScheduleApi.PC_data[key].length;
			});
			var total_client_rule_num = weekScheduleApi.PC_other_client_rule_num + current_data_num;
			if(total_client_rule_num + 1 > weekScheduleApi.data_max){
				popupHint.init();
				var hint = "<#weekSche_MAX_Num#>".replace("#MAXNUM", weekScheduleApi.data_max);
				hint += "<br>";
				hint += "<#weekSche_MAX_Del_Hint#>";
				popupHint.set_text(hint);
				popupHint.set_btn_ok();
				popupHint.hide_btn("cancel");
				return;
			}

			$("#popup_edit_weekSchedule").empty();
			var abbreviation_weekday = JS_timeObj.toString().substring(0,3).toUpperCase();
			var specific_weekday = weekScheduleApi.weekday_mapping.filter(function(item, index, _array){
				return (item.text.substr(0,3).toUpperCase() == abbreviation_weekday);
			})[0];
			var PC_offtime_attr = new weekScheduleApi.PC_offtime_attr();
			var weekday = Math.pow(2, parseInt(specific_weekday.bitwise));
			var start_hour = 8;
			var start_min = 0;
			var end_hour = 12;
			var end_min = 0;
			PC_offtime_attr.weekday = weekday;
			PC_offtime_attr.start_hour = start_hour;
			PC_offtime_attr.start_min = start_min;
			PC_offtime_attr.end_hour = end_hour;
			PC_offtime_attr.end_min = end_min;
			$("#popup_edit_weekSchedule").append(weekScheduleApi.edit_time_setting_component("new", PC_offtime_attr, "PC"));
			$("#popup_edit_weekSchedule").fadeIn();
			adjust_panel_block_top("popup_edit_weekSchedule", 100);
			$("#popup_edit_weekSchedule").unbind("click");
			$("#popup_edit_weekSchedule").click(function(e){
				e = e || event;
				e.stopPropagation();
				weekScheduleApi.reset_custom_select_status();
			});
		});

		var $overview_icon = $("<div>").addClass("btn_icon_bg overview_icon gridview_mode");
		$overview_icon.appendTo($header_bg);
		$overview_icon.unbind("click");
		$overview_icon.click(function(e){
			e = e || event;
			e.stopPropagation();
			$("#popup_edit_weekSchedule").hide();
			$("#" + weekScheduleApi.obj_id + "").find(".gridview_mode").hide();
			$("#" + weekScheduleApi.obj_id + "").find(".overview_mode").show();

			var overview_data = [];
			$.each(weekScheduleApi.weekday_mapping, function( index, value ) {
				var weekday_obj = value;
				overview_data["w_" + weekday_obj.bitwise] = [];//initial data obj
			});
			Object.keys(weekScheduleApi.PC_data).forEach(function(key) {
				$.each(weekScheduleApi.PC_data[key], function( index, value ) {
					var PC_offtime_obj = value;
					var weekNum = PC_offtime_obj.weekday;
					var offtime_obj = new weekScheduleApi.offtime_attr();
					var offtime_cross_days_obj = "";
					offtime_obj.enable = PC_offtime_obj.enable;
					offtime_obj.start_hour = PC_offtime_obj.start_hour
					offtime_obj.start_min = PC_offtime_obj.start_min;
					offtime_obj.end_hour = PC_offtime_obj.end_hour;
					offtime_obj.end_min = PC_offtime_obj.end_min;
					var start_num = offtime_obj.start_hour*60 + offtime_obj.start_min;
					var end_num = offtime_obj.end_hour*60 + offtime_obj.end_min;
					if(start_num >= end_num){
						offtime_cross_days_obj = JSON.parse(JSON.stringify(offtime_obj));
						offtime_cross_days_obj.start_hour = 0;
						offtime_cross_days_obj.start_min = 0;
						offtime_cross_days_obj.end_hour = offtime_obj.end_hour;
						offtime_cross_days_obj.end_min = offtime_obj.end_min;
						offtime_obj.end_hour = 24;
						offtime_obj.end_min = 0;
					}
					$.each(weekScheduleApi.weekday_mapping, function( index, value ) {
						var weekday_obj = value;
						if(weekNum >> weekday_obj.bitwise & 1){
							offtime_obj.weekday = parseInt(weekday_obj.bitwise);
							overview_data["w_" + offtime_obj.weekday + ""].push(JSON.parse(JSON.stringify(offtime_obj)));
							if(typeof offtime_cross_days_obj == "object"){
								var cross_days_weekday = ( ((offtime_obj.weekday + 1) > 6) ? 0 : (offtime_obj.weekday + 1));
								offtime_cross_days_obj.weekday = cross_days_weekday;
								overview_data["w_" + cross_days_weekday + ""].push(JSON.parse(JSON.stringify(offtime_cross_days_obj)));
							}
						}
					});
				});
			});

			$(".offtime_overview_content_bg").empty();
			$(".offtime_overview_content_bg").append(weekScheduleApi.overview_a_week_component("view"));
			var abbreviation_weekday = JS_timeObj.toString().substring(0,3).toUpperCase();
			var specific_weekday = weekScheduleApi.weekday_mapping.filter(function(item, index, _array){
				return (item.text.substr(0,3).toUpperCase() == abbreviation_weekday);
			})[0];
			var current_weekday_index = specific_weekday.bitwise;
			$(".offtime_overview_content_bg").find(".weekday_title").eq(current_weekday_index).addClass("current_weekday");

			var demarcation_hour_array = [];
			for(var i = weekScheduleApi.demarcation_hour; i < 24; i += weekScheduleApi.demarcation_hour)
				demarcation_hour_array.push(i);
			var $all_weekday_bg = $(".offtime_overview_content_bg").find(".overview_a_week_bg .weekday_time_bg");
			if($all_weekday_bg.length > 0){
				$.each(demarcation_hour_array, function( index, value ) {
					$all_weekday_bg.append(weekScheduleApi.overview_demarcation_line_component(value, $all_weekday_bg.width()));//demarcation_line
				});
				$(".offtime_overview_content_bg").prepend(weekScheduleApi.overview_time_title_component(demarcation_hour_array, $all_weekday_bg.width()));//time title text
			}
			var $offtime_schedule_text_bg = $("<div>").addClass("offtime_schedule_text_bg");
			$offtime_schedule_text_bg.append(weekScheduleApi.get_offtime_schedule_text()).append(weekScheduleApi.get_offtime_schedule_dis_text());
			$(".offtime_overview_content_bg").append($offtime_schedule_text_bg);

			Object.keys(overview_data).forEach(function(key) {
				var weekday = key;
				$(".offtime_overview_content_bg").find("#" + weekday).find(".offtime_bg").remove();
				var weekday_obj = overview_data[weekday];
				$.each(weekday_obj, function( index, value ) {
					var offtime_obj = value
					weekScheduleApi.set_offtime(offtime_obj);
				});
			});
		});

		var $gridview_icon = $("<div>").addClass("btn_icon_bg gridview_icon overview_mode");
		$gridview_icon.appendTo($header_bg);
		$gridview_icon.unbind("click");
		$gridview_icon.click(function(e){
			e = e || event;
			e.stopPropagation();
			$("#" + weekScheduleApi.obj_id + "").find(".gridview_mode").show();
			$("#" + weekScheduleApi.obj_id + "").find(".overview_mode").hide();
		});

		return $header_bg;
	},
	"PC_get_action_btn" : function(){
		var $action_btn_bg = $("<div>");
		$action_btn_bg.addClass("action_btn_bg gridview_mode");

		var $cancel_btn = $("<div>");
		$cancel_btn.appendTo($action_btn_bg);
		$cancel_btn.addClass("button_gen");
		$cancel_btn.html("<#CTL_Cancel#>");
		$cancel_btn.unbind("click");
		$cancel_btn.click(function(e){
			e = e || event;
			e.stopPropagation();
			if(typeof weekScheduleApi.callback_btn_cancel == "function")
				weekScheduleApi.callback_btn_cancel();
		});

		var $apply_btn = $("<div>");
		$apply_btn.appendTo($action_btn_bg);
		$apply_btn.addClass("button_gen");
		$apply_btn.html("<#CTL_apply#>");
		$apply_btn.unbind("click");
		$apply_btn.click(function(e){
			e = e || event;
			e.stopPropagation();
			if(typeof weekScheduleApi.callback_btn_apply == "function")
				weekScheduleApi.callback_btn_apply();
		});

		return $action_btn_bg;
	},
	"PC_get_no_data_component" : function(){
		var $no_data_bg = $("<div>").addClass("no_data_bg ");
		$no_data_bg.append($("<div>").addClass("no_data_icon"));
		$no_data_bg.append($("<div>").addClass("no_data_text").html("<#IPConnection_VSList_Norule#>"));
		return $no_data_bg;
	},
	"PC_get_week_title" : function(_weekNum){
		var weekNum = parseInt(_weekNum);
		var weekTitle = "";
		$.each(weekScheduleApi.weekday_mapping, function( index, value ) {
			var weekday_obj = value;
			if(weekNum >> weekday_obj.bitwise & 1){
				if(weekTitle != "")
					weekTitle += " / "
				weekTitle += weekday_obj.text.substr(0,3).toUpperCase();
			}
		});
		return weekTitle
	},
	"PC_get_grid_period_component" : function(){
		weekScheduleApi.PC_sort_data();
		var $grid_period_component = $("<div>");
		Object.keys(weekScheduleApi.PC_data).forEach(function(key) {
			var week_group = key;
			var $grid_period_group_bg = $("<div>");
			$grid_period_group_bg.appendTo($grid_period_component);
			$grid_period_group_bg.attr("id", "grid_period_" + week_group + "");
			$.each(weekScheduleApi.PC_data[week_group], function( index, value ) {
				var PC_offtime_obj = value;
				var $grid_period_bg = $("<div>").appendTo($grid_period_group_bg);
				$grid_period_bg.attr({"id":PC_offtime_obj.id, "week_group":week_group});
				$grid_period_bg.addClass("grid_period_bg");

				var $grid_period_title = $("<div>");
				$grid_period_title.appendTo($grid_period_bg);
				$grid_period_title.addClass("grid_period_title");
				var display_title = "";
				if(week_group == "Other")
					display_title = value.title
						.replace("MON", "<#date_Mon_itemdesc#>").replace("TUE", "<#date_Tue_itemdesc#>").replace("WED", "<#date_Wed_itemdesc#>")
						.replace("THU", "<#date_Thu_itemdesc#>").replace("FRI", "<#date_Fri_itemdesc#>").replace("SAT", "<#date_Sat_itemdesc#>")
						.replace("SUN", "<#date_Sun_itemdesc#>");
				else
					display_title = week_group.replace("Daily", "<#weekSche_Everyday#>").replace("Weekdays", "<#weekSche_Weekdays#>").replace("Weekend", "<#weekSche_Weekend#>");

				$grid_period_title.html(display_title);

				var $grid_period_content = $("<div>");
				$grid_period_content.appendTo($grid_period_bg);
				$grid_period_content.addClass("grid_period_content");

				if(PC_offtime_obj.enable)
					$grid_period_content.addClass("on");
				else
					$grid_period_content.addClass("off");

				$grid_period_content.hover(
					function(){$(this).children(".period_trash_icon").show();},
					function(){$(this).children(".period_trash_icon").hide();}
				);
				$grid_period_content.unbind("click");
				$grid_period_content.click(function(e){
					e = e || event;
					e.stopPropagation();
					$("#popup_edit_weekSchedule").empty();
					$("#popup_edit_weekSchedule").append(weekScheduleApi.edit_time_setting_component("edit", PC_offtime_obj, "PC"));
					$("#popup_edit_weekSchedule").fadeIn();
					adjust_panel_block_top("popup_edit_weekSchedule", 100);
					$("#popup_edit_weekSchedule").unbind("click");
					$("#popup_edit_weekSchedule").click(function(e){
						e = e || event;
						e.stopPropagation();
						weekScheduleApi.reset_custom_select_status();
					});
				});
				var $period_text_bg = $("<div>");
				$period_text_bg.appendTo($grid_period_content);
				$period_text_bg.addClass("period_text_bg");
				var $start_time = $("<div>");
				$start_time.addClass("start_time");
				$start_time.attr("title", "<#weekSche_Start_Time#>");
				$start_time.appendTo($period_text_bg);
				$start_time.html(weekScheduleApi.add_left_pad(PC_offtime_obj.start_hour, 2) + ":" + weekScheduleApi.add_left_pad(PC_offtime_obj.start_min, 2));

				var $dash = $("<div>");
				$dash.addClass("dash");
				$dash.appendTo($period_text_bg);

				var $end_time = $("<div>");
				$end_time.addClass("end_time");
				$end_time.attr("title", "<#weekSche_End_Time#>");
				$end_time.appendTo($period_text_bg);
				$end_time.html(weekScheduleApi.add_left_pad(PC_offtime_obj.end_hour, 2) + ":" + weekScheduleApi.add_left_pad(PC_offtime_obj.end_min, 2));

				var $period_switch = $("<div>");
				$period_switch.appendTo($grid_period_content);
				$period_switch.addClass("period_switch");
				if(PC_offtime_obj.enable)
					$period_switch.addClass("on");
				else
					$period_switch.addClass("off");
				$period_switch.unbind("click");
				$period_switch.click(function(e){
					e = e || event;
					e.stopPropagation();
					$(this).toggleClass("off  on");
					$grid_period_content.toggleClass("off  on");
					if($(this).hasClass("period_switch on"))
						PC_offtime_obj.enable = 1;
					else
						PC_offtime_obj.enable = 0;
				});

				var $period_trash_icon = $("<div>");
				$period_trash_icon.appendTo($grid_period_content);
				$period_trash_icon.addClass("period_trash_icon");
				$period_trash_icon.unbind("click");
				$period_trash_icon.click(function(e){
					e = e || event;
					e.stopPropagation();
					var delete_id = $(this).parents(".grid_period_bg").attr("id");
					var delete_week_group = $(this).parents(".grid_period_bg").attr("week_group");
					$(this).parents(".grid_period_bg").remove();
					weekScheduleApi.PC_remove_offtime(weekScheduleApi.PC_data[delete_week_group], "id", delete_id);
					if(week_group != "Other")
						$grid_period_group_bg.children(".grid_period_bg").children(".grid_period_title").hide().eq(0).show();
					else{
						$grid_period_group_bg.children(".grid_period_bg").children(".grid_period_title").show();
						var pre_title = "";
						$.each($grid_period_group_bg.children(".grid_period_bg").children(".grid_period_title"), function( index, element ) {
							var current_title = $(element).html();
							if(pre_title == current_title)
								$(element).hide();
							pre_title = current_title;
						});
					}
					if(weekScheduleApi.PC_data[delete_week_group].length == 0 && delete_week_group != "Other")
						$("#grid_period_" + delete_week_group + "").hide();

					var offtime_count = 0;
					Object.keys(weekScheduleApi.PC_data).forEach(function(key) {
						offtime_count += weekScheduleApi.PC_data[key].length;
					});
					if(offtime_count == 0){
						$("#" + weekScheduleApi.obj_id + " .offtime_schedule_content_bg").empty();
						$("#" + weekScheduleApi.obj_id + " .offtime_schedule_content_bg").append(weekScheduleApi.PC_get_no_data_component());
					}
				});
				$period_trash_icon.hide();
			});
			if(week_group != "Other")
				$grid_period_group_bg.children(".grid_period_bg").children(".grid_period_title").hide().eq(0).show();
			else{
				var pre_title = "";
				$.each($grid_period_group_bg.children(".grid_period_bg").children(".grid_period_title"), function( index, element ) {
					var current_title = $(element).html();
					if(pre_title == current_title)
						$(element).hide();
					pre_title = current_title;
				});
			}
		});
		return $grid_period_component;
	},
	"PC_remove_offtime" : function(array, property, value){
		$.each(array, function( index, element ) {
			if(element[property] == value){
				array.splice(index, 1);
				return false;
			}
		});
	},
	"PC_transform_offtime_json_to_string" : function(){
		var result = "";
		Object.keys(weekScheduleApi.PC_data).forEach(function(key) {
			var week_group = key;
			$.each(weekScheduleApi.PC_data[week_group], function( index, value ) {
				var PC_offtime_obj = value;
				var enable = PC_offtime_obj.enable;
				var weekday = weekScheduleApi.add_left_pad(((parseInt(PC_offtime_obj.weekday)).toString(16).toUpperCase()), 2);
				var start_hour = weekScheduleApi.add_left_pad(PC_offtime_obj.start_hour, 2);
				var start_min = weekScheduleApi.add_left_pad(PC_offtime_obj.start_min, 2);
				var end_hour = weekScheduleApi.add_left_pad(PC_offtime_obj.end_hour, 2);
				var end_min = weekScheduleApi.add_left_pad(PC_offtime_obj.end_min, 2);
				if(result != "")
					result += "<";
				result += "W";
				result += enable;
				result += weekday;
				result += start_hour;
				result += start_min;
				result += end_hour;
				result += end_min;
			});
		});
		return result;
	},
	"PC_sort_data" : function(){
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
		if(weekScheduleApi.PC_data["Daily"].length > 0)
			weekScheduleApi.PC_data["Daily"].sort(sortWeekdayAndTime);
		if(weekScheduleApi.PC_data["Weekdays"].length > 0)
			weekScheduleApi.PC_data["Weekdays"].sort(sortWeekdayAndTime);
		if(weekScheduleApi.PC_data["Weekend"].length > 0)
			weekScheduleApi.PC_data["Weekend"].sort(sortWeekdayAndTime);
		if(weekScheduleApi.PC_data["Other"].length > 0)
			weekScheduleApi.PC_data["Other"].sort(sortWeekdayAndTime);
	}
}
var popupHint = {
	"init" : function(){
		if($("#" + weekScheduleApi.obj_id + "").next(".popup_hint_component").length < 1){
			var $popupHint = $("<div>");
			$("#" + weekScheduleApi.obj_id + "").after($popupHint);
			$popupHint.addClass("popup_hint_component popup_hint_bg");
			$popupHint.attr({"id":"popup_hint_bg", "onselectstart":"false"});

			$("<div>").addClass("hint_text").appendTo($popupHint);

			var $action_bg = $("<div>").addClass("action_bg").appendTo($popupHint);
			var $action_cancel = $("<input/>").attr({"id":"action_cancel", "type":"button", "value":"<#CTL_Cancel#>"}).addClass("button_gen");
			var $action_ok = $("<input/>").attr({"id":"action_ok", "type":"button", "value":"<#CTL_ok#>"}).addClass("button_gen");
			$action_bg.append($action_cancel).append($action_ok);

			$("<div>").addClass("popup_hint_component mask_bg").appendTo($("body"));
		}
		$(".popup_hint_component").fadeIn();
		adjust_panel_block_top("popup_hint_bg", 100);
	},
	"set_text" : function(_text){
		$(".popup_hint_component .hint_text").html(_text);
	},
	set_btn_ok : function(_fun){
		$(".popup_hint_component .action_bg #action_ok").show();
		$(".popup_hint_component .action_bg #action_ok").unbind("click");
		$(".popup_hint_component .action_bg #action_ok").click(function(e){
			e = e || event;
			e.stopPropagation();
			if(_fun)
				_fun();
			$(".popup_hint_component").fadeOut();
		});
		
	},
	set_btn_cancel : function(_fun){
		$(".popup_hint_component .action_bg #action_cancel").show();
		$(".popup_hint_component .action_bg #action_cancel").unbind("click");
		$(".popup_hint_component .action_bg #action_cancel").click(function(e){
			e = e || event;
			e.stopPropagation();
			if(_fun)
				_fun();
			$(".popup_hint_component").fadeOut();
		});	
	},
	"hide_btn" : function(_type){
		$(".popup_hint_component .action_bg #action_" + _type + "").hide();
	}
}
