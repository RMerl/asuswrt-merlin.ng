jQuery.fn.show_validate_hint = function(hintStr){
	$(this).closest(".profile_setting").find(".validate_hint").remove();

	$("<div>")
		.html(hintStr)
		.addClass("validate_hint")
		.insertAfter($(this).closest(".profile_setting_item"));

		resize_iframe_height();
}
jQuery.fn.show_item_hint = function(hintStr){
	$(this).closest(".profile_setting").find(".validate_hint").remove();

	$("<div>")
		.html(hintStr)
		.addClass("item_hint")
		.insertAfter($(this).closest(".profile_setting_item"));
}
var ip_RegExp = {
	"IPv4" : "^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$",
	"IPv4_CIDR" : "^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])(\/([0-9]|[1-2][0-9]|3[0-2]))$",
	"IPv6" : "^((([0-9A-Fa-f]{1,4}:){7}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){6}:[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){5}:([0-9A-Fa-f]{1,4}:)?[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){4}:([0-9A-Fa-f]{1,4}:){0,2}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){3}:([0-9A-Fa-f]{1,4}:){0,3}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){2}:([0-9A-Fa-f]{1,4}:){0,4}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){6}((\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b)\\.){3}(\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b))|(([0-9A-Fa-f]{1,4}:){0,5}:((\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b)\\.){3}(\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b))|(::([0-9A-Fa-f]{1,4}:){0,5}((\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b)\\.){3}(\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b))|([0-9A-Fa-f]{1,4}::([0-9A-Fa-f]{1,4}:){0,5}[0-9A-Fa-f]{1,4})|(::([0-9A-Fa-f]{1,4}:){0,6}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){1,7}:))$",
	"IPv6_CIDR" : "^((([0-9A-Fa-f]{1,4}:){7}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){6}:[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){5}:([0-9A-Fa-f]{1,4}:)?[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){4}:([0-9A-Fa-f]{1,4}:){0,2}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){3}:([0-9A-Fa-f]{1,4}:){0,3}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){2}:([0-9A-Fa-f]{1,4}:){0,4}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){6}((\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b)\\.){3}(\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b))|(([0-9A-Fa-f]{1,4}:){0,5}:((\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b)\\.){3}(\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b))|(::([0-9A-Fa-f]{1,4}:){0,5}((\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b)\\.){3}(\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b))|([0-9A-Fa-f]{1,4}::([0-9A-Fa-f]{1,4}:){0,5}[0-9A-Fa-f]{1,4})|(::([0-9A-Fa-f]{1,4}:){0,6}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){1,7}:))(\/([0-9]|[1-9][0-9]|1[01][0-9]|12[0-8]))$"
};
function Get_Component_Customize_Alert(_text){
	var $popup_content_container = $("<div>").addClass("popup_content_container");

	var $customize_alert = $("<div>").addClass("customize_alert");
	$customize_alert.appendTo($popup_content_container);
	var $desc = $("<div>").addClass("desc").html(htmlEnDeCode.htmlEncode(_text));
	$desc.appendTo($customize_alert);
	var $action_btn_container = $("<div>").addClass("action_btn_container");
	$action_btn_container.appendTo($customize_alert);
	var $ok = $("<div>").addClass("ok btn").html("<#CTL_ok#>");
	$ok.appendTo($action_btn_container);
	$ok.unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		close_popup_customize_alert();
	});

	return $popup_content_container;
}
function Get_Component_Popup_Profile_Title(_text){
	var $popup_title_container = $("<div>").addClass("popup_title_container");
	$("<div>").addClass("title").html(htmlEnDeCode.htmlEncode(_text)).appendTo($popup_title_container);
	var $close_btn = $("<div>").addClass("vpn_icon_all_collect close_btn");
	$close_btn.appendTo($popup_title_container);
	$close_btn.unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		close_popup();
	});
	return $popup_title_container;
}
function Get_Component_Profile_Title(_text){
	var $title_container = $("<div>").addClass("profile_setting_title");
	$("<div>").addClass("title").html(htmlEnDeCode.htmlEncode(_text)).appendTo($title_container);;
	return $title_container;
}
function Get_Component_Pure_Text(_parm){
	var $container = $("<div>").addClass("profile_setting_item textView_item");
	if(_parm.container_id != undefined)
		$container.attr("id", _parm.container_id);

	if(_parm.openHint != undefined){
		var hint_array = _parm.openHint.split("_");
		$("<a>").addClass("hintstyle").attr({"href":"javascript:void(0);"}).html(htmlEnDeCode.htmlEncode(_parm.title)).unbind("click").click(function(){
			openHint(hint_array[0], hint_array[1], "rwd_vpns");
		}).appendTo($("<div>").addClass("title").appendTo($container));
	}
	else
		$("<div>").addClass("title").html(htmlEnDeCode.htmlEncode(_parm.title)).appendTo($container);

	var $input_container = $("<div>").addClass("input_container").appendTo($container);
	var $input = $("<div>")
					.addClass("textView")
					.attr({"id":_parm.id})
					.html(htmlEnDeCode.htmlEncode(_parm.text))
					.appendTo($input_container);

	return $container;
}
function Get_Component_Input(_parm){
	var maxlength = 64;
	if(_parm.maxlength != undefined)
		maxlength = _parm.maxlength;

	var $container = $("<div>").addClass("profile_setting_item");
	if(_parm.container_id != undefined)
		$container.attr("id", _parm.container_id);

	if(_parm.openHint != undefined){
		var hint_array = _parm.openHint.split("_");
		$("<a>").addClass("hintstyle").attr({"href":"javascript:void(0);"}).html(htmlEnDeCode.htmlEncode(_parm.title)).unbind("click").click(function(){
			openHint(hint_array[0], hint_array[1], "rwd_vpns");
		}).appendTo($("<div>").addClass("title").appendTo($container));
	}
	else
		$("<div>").addClass("title").html(htmlEnDeCode.htmlEncode(_parm.title)).appendTo($container);

	var set_value = "";
	if(_parm.set_value != undefined)
		set_value = _parm.set_value;

	var $input_container = $("<div>").addClass("input_container").appendTo($container);
	var $input = $("<input/>")
					.addClass("textInput")
					.attr({"id":_parm.id, "type":_parm.type, "maxlength":maxlength, "autocomplete":"off","autocorrect":"off","autocapitalize":"off","spellcheck":"false"})
					.val(htmlEnDeCode.htmlEncode(set_value))
					.unbind("blur").blur(function(e){
						e = e || event;
						e.stopPropagation();
					})
					.on('click', function () {
						var target = this;
						setTimeout(function(){
							target.scrollIntoViewIfNeeded();//for mobile view
						},400);
					})
					.appendTo($input_container);

	if(_parm.need_check)
		$input.attr("need_check", true);

	if(_parm.type == "password"){
		$("<div>").addClass("vpn_icon_all_collect icon_eye close").attr({"for": _parm.id}).unbind("click").click(function(){
			var targetObj = $(this);
			$(this).toggleClass("close open");
			$("#"+$(this).attr("for")).prop("type", (function(){return targetObj.hasClass("icon_eye close") ? "password" : "text";}()));
		}).appendTo($input_container);
	}

	if(_parm.placeholder)
		$input.attr("placeholder", htmlEnDeCode.htmlEncode(_parm.placeholder));

	return $container;
}
function Get_Component_Two_Input(_parm){
	var $container = $("<div>").addClass("profile_setting_item two_input");

	if(_parm.container_id != undefined)
		$container.attr("id", _parm.container_id);

	if(_parm.openHint != undefined){
		var hint_array = _parm.openHint.split("_");
		$("<a>").addClass("hintstyle").attr({"href":"javascript:void(0);"}).html(htmlEnDeCode.htmlEncode(_parm.title)).unbind("click").click(function(){
			openHint(hint_array[0], hint_array[1], "rwd_vpns");
		}).appendTo($("<div>").addClass("title").appendTo($container));
	}
	else
		$("<div>").addClass("title").html(htmlEnDeCode.htmlEncode(_parm.title)).appendTo($container);

	var $input_container = $("<div>").addClass("input_container").appendTo($container);
	var $input_1 = $("<input/>")
						.addClass("textInput")
						.attr({"id":_parm.id_1, "type":_parm.type_1, "maxlength":_parm.maxlength_1, "autocomplete":"off","autocorrect":"off","autocapitalize":"off","spellcheck":"false"})
						.val(htmlEnDeCode.htmlEncode(""))
						.unbind("blur").blur(function(e){
							e = e || event;
							e.stopPropagation();
						}).on('click', function () {
							var target = this;
							setTimeout(function(){
								target.scrollIntoViewIfNeeded();
							},400);
						})
						.appendTo($input_container);
	if(_parm.need_check_1)
		$input_1.attr("need_check", true);

	var $input_2 = $("<input/>")
						.addClass("textInput")
						.attr({"id":_parm.id_2, "type":_parm.type_2, "maxlength":_parm.maxlength_2, "autocomplete":"off","autocorrect":"off","autocapitalize":"off","spellcheck":"false"})
						.val(htmlEnDeCode.htmlEncode(""))
						.unbind("blur").blur(function(e){
							e = e || event;
							e.stopPropagation();
						})
						.on('click', function () {
							var target = this;
							setTimeout(function(){
								target.scrollIntoViewIfNeeded();
							},400);
						})
						.appendTo($input_container);

	if(_parm.need_check_2)
		$input_2.attr("need_check", true);

	return $container;
}
function Get_Component_Textarea(_parm){
	var rows = 8;
	if(_parm.rows != undefined)
		rows = _parm.rows;
	var cols = 55;
	if(_parm.cols != undefined)
		cols = _parm.cols;
	var maxlength = 15000;
	if(_parm.maxlength != undefined)
		maxlength = _parm.maxlength;

	var $container = $("<div>").addClass("profile_setting_item textarea");
	if(_parm.container_id != undefined)
		$container.attr("id", _parm.container_id);

	$("<div>").addClass("title").html(htmlEnDeCode.htmlEncode(_parm.title)).appendTo($container);

	var $input_container = $("<div>").addClass("input_container").appendTo($container);
	var $input = $("<textarea/>")
					.addClass("textareaInput")
					.attr({"id":_parm.id, "rows":rows, "cols":cols, "maxlength":maxlength})
					.val(htmlEnDeCode.htmlEncode(""))
					.appendTo($input_container);

	if(_parm.need_check)
		$input.attr("need_check", true);

	return $container;
}
function Get_Component_Custom_Select(_parm){
	var $container = $("<div>").addClass("profile_setting_item");
	if(_parm.container_id != undefined)
		$container.attr("id", _parm.container_id);

	// title
	if(_parm.openHint != undefined){
		var hint_array = _parm.openHint.split("_");
		$("<a>").addClass("hintstyle").attr({"href":"javascript:void(0);"}).html(htmlEnDeCode.htmlEncode(_parm.title)).unbind("click").click(function(){
			openHint(hint_array[0], hint_array[1], "rwd_vpns");
		}).appendTo($("<div>").addClass("title").appendTo($container));
	}
	else
		$("<div>").addClass("title").html(htmlEnDeCode.htmlEncode(_parm.title)).appendTo($container);

	// input field
	var $input_container = $("<div>").addClass("input_container").appendTo($container);
	var $custom_select_container = $("<div>").addClass("custom_select_container").appendTo($input_container);
	$custom_select_container.unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		if($(this).attr("temp_disable") == "disabled")
			return;
		$(this).toggleClass("arrow_up");
		$(this).children(".select_options_container").toggleClass("container_hide");
	});
	$custom_select_container.unbind("mouseleave").mouseleave(function(e){
		e = e || event;
		e.stopPropagation();
		if($(this).children(".select_options_container").css("display") == "block"){
			$(this).closest(".custom_select_container").toggleClass("arrow_up");
			$(this).children(".select_options_container").toggleClass("container_hide");
		}
	});
	// selected text
	var specific_option = [];
	if(_parm.set_value == undefined || _parm.set_value == ""){
		specific_option = _parm.options[0];
	}
	else{
		specific_option = _parm.options.filter(function(item, index, array){
			return (item.value == _parm.set_value);
		})[0];
	}
	var $selected_text = $("<div>").attr("id", _parm.id).appendTo($custom_select_container).html(htmlEnDeCode.htmlEncode(specific_option.text));
	// space
	$("<div>").css("height", "3px").appendTo($custom_select_container);
	// select options
	var $select_options_container = $("<div>").attr("id", "select_" + _parm.id).addClass("select_options_container container_hide").appendTo($custom_select_container);
	_parm.options.forEach(function(item){
		var $option = $("<div>").attr("value", item.value).html(htmlEnDeCode.htmlEncode(item.text))
				.unbind("click").click(function(e){
					e = e || event;
					e.stopPropagation();
					var option_value = $(this).attr("value");
					$(this).closest(".select_options_container").toggleClass("container_hide");

					var specific_option = _parm.options.filter(function(item, index, array){
						return (item.value == option_value);
					})[0];

					if(specific_option != undefined){
						$(this).closest(".select_options_container").children().removeClass("selected");
						$(this).closest(".select_options_container").children("[value='" + specific_option.value + "']").addClass("selected");
						$(this).closest(".custom_select_container").children("#" + _parm.id + "").html(htmlEnDeCode.htmlEncode(specific_option.text));
						$(this).closest(".custom_select_container").toggleClass("arrow_up");
					}
				})
				.appendTo($select_options_container);

		if(item.value == specific_option.value)
			$option.addClass("selected");
	});

	return $container;
}
function Get_Component_Switch(_parm){
	var $container = $("<div>").addClass("profile_setting_item nowrap switch_item");
	if(_parm.container_id != undefined)
		$container.attr("id", _parm.container_id);

	if(_parm.openHint != undefined){
		var hint_array = _parm.openHint.split("_");
		$("<a>").addClass("hintstyle").attr({"href":"javascript:void(0);"}).html(htmlEnDeCode.htmlEncode(_parm.title)).unbind("click").click(function(){
			openHint(hint_array[0], hint_array[1], "rwd_vpns");
		}).appendTo($("<div>").addClass("title").appendTo($container));
	}
	else
		$("<div>").addClass("title").html(htmlEnDeCode.htmlEncode(_parm.title)).appendTo($container);

	var $input_container = $("<div>").addClass("input_container").appendTo($container);
	var $switch_icon = $("<div>").attr("id", _parm.id).addClass("switch on").unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		if($(this).attr("temp_disable") == "disabled")
			return;
		$(this).toggleClass("off on");
	}).appendTo($input_container);

	return $container;
}
function Get_Component_Switch_Two_Btn(_parm){
	var $container = $("<div>").addClass("profile_setting_item two_btn_switch");

	var $btn_options_container = $("<div>").addClass("btn_options_container").appendTo($container);
	_parm.options.forEach(function(item, index){
		var $btn_container = $("<div>").addClass("btn_container").attr({"value":item.value}).appendTo($btn_options_container);
		if(index == 0)
			$btn_container.addClass("selected");
		$("<div>").addClass("btn_text").html(htmlEnDeCode.htmlEncode(item.text)).appendTo($btn_container);
	});

	return $container;
}
function Get_Component_Checkbox(_parm){
	var $container = $("<div>").addClass("profile_setting_item checkbox_item");
	if(_parm.container_id != undefined)
		$container.attr("id", _parm.container_id);

	$container.append($("<div>").addClass("title").html(htmlEnDeCode.htmlEncode(_parm.title)));

	var $input_container = $("<div>").addClass("input_container").attr("id", _parm.id).appendTo($container);
	_parm.options.forEach(function(item){
		var $cb_option_container = $("<div>").addClass("cb_option_container").appendTo($input_container).attr("cb_group", _parm.id);
		var $cb_container = $("<div>").addClass("cb_container").appendTo($cb_option_container);
		var $checkbox_icon = $("<div>").addClass("checkbox").appendTo($cb_container);
		$checkbox_icon.attr({"value":item.value});
		$("<div>").html(htmlEnDeCode.htmlEncode(item.text)).appendTo($cb_option_container);
	});

	return $container;
}
function Get_Component_Radio(_parm){
	var display_type = "horizontal";
	if(_parm.display_type != undefined)
		display_type = _parm.display_type;

	var $container = $("<div>").addClass("profile_setting_item radio_item " + display_type + "");
	if(_parm.container_id != undefined)
		$container.attr("id", _parm.container_id);

	if(_parm.openHint != undefined){
		var hint_array = _parm.openHint.split("_");
		$("<a>").addClass("hintstyle").attr({"href":"javascript:void(0);"}).html(htmlEnDeCode.htmlEncode(_parm.title)).unbind("click").click(function(){
			openHint(hint_array[0], hint_array[1], "rwd_vpns");
		}).appendTo($("<div>").addClass("title").appendTo($container));
	}
	else
		$("<div>").addClass("title").html(htmlEnDeCode.htmlEncode(_parm.title)).appendTo($container);

	var $input_container = $("<div>").addClass("input_container").attr("id", _parm.id).appendTo($container);
	var $radio_options_container = $("<div>").addClass("radio_options_container " + display_type + "").appendTo($input_container);
	_parm.options.forEach(function(item, index){
		var $radio_container = $("<div>").addClass("radio_container").attr({"value":item.value}).appendTo($radio_options_container)
			.unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				$(this).closest(".radio_options_container").find(".radio_container").removeClass("selected");
				$(this).addClass("selected");
			});
		if(index == 0)
			$radio_container.addClass("selected");

		$("<div>").addClass("radio_icon").appendTo($radio_container);
		$("<div>").html(htmlEnDeCode.htmlEncode(item.text)).appendTo($radio_container);
	});

	return $container;
}
function Get_Component_Help(_parm){
	var $container = $("<div>").addClass("profile_setting_item nowrap help_item");
	$container.append($("<div>").addClass("title").html(htmlEnDeCode.htmlEncode(_parm.title)));

	var $input_container = $("<div>").addClass("input_container").appendTo($container);
	$("<div>").addClass("vpn_icon_all_collect vpnc_help_icon").appendTo($input_container);

	return $container;
}
function Get_Component_Add_Client_Title(){
	var $container = $("<div>").addClass("profile_setting_item nowrap acc_pwd");
	var $title = $("<div>").addClass("title").appendTo($container);

	$("<div>").addClass("vpn_icon_all_collect key_icon").appendTo($title);
	$("<div>").attr({"id":"vpns_clientlist_title"}).appendTo($title);

	var $input_container = $("<div>").addClass("input_container").appendTo($container);
	var $add_acc_bg = $("<div>").addClass("add_acc_bg").appendTo($input_container);
	$("<div>").attr({"id":"vpns_clientlist_num"}).addClass("acc_num").appendTo($add_acc_bg);
	$("<div>").attr({"id":"vpns_clientlist_add"}).addClass("vpn_icon_all_collect acc_pwd_add_icon").appendTo($add_acc_bg);
	return $container;
}
function Get_Component_Client_No_Item(){
	return $("<div>").addClass("client_container no_data").html("<#IPConnection_VSList_Norule#>");
}
function Get_Component_Button(_parm){
	var $container = $("<div>").addClass("profile_setting_item nowrap btn_item");
	if(_parm.openHint != undefined){
		var hint_array = _parm.openHint.split("_");
		$("<a>").addClass("hintstyle").attr({"href":"javascript:void(0);"}).html(htmlEnDeCode.htmlEncode(_parm.title)).unbind("click").click(function(){
			openHint(hint_array[0], hint_array[1], "rwd_vpns");
		}).appendTo($("<div>").addClass("title").appendTo($container));
	}
	else
		$("<div>").addClass("title").html(htmlEnDeCode.htmlEncode(_parm.title)).appendTo($container);

	var $input_container = $("<div>").addClass("input_container").appendTo($container);
	var $btn_container = $("<div>").addClass("btn_container").attr({"id":_parm.id}).unbind("click").appendTo($input_container);
	$("<div>").addClass("text import_file").html(htmlEnDeCode.htmlEncode(_parm.btn_text)).appendTo($btn_container);

	return $container;
}
function Get_Component_Slide_Title(_parm) {
	var $container = $("<div>").addClass("profile_title_item arrow_icon");

	if(_parm.id != undefined){
		$container.attr("id", _parm.id);
	}
	if(_parm.default_icon != undefined){
		$container.addClass(_parm.default_icon);
	}
	$container.append($("<span>").addClass("title").html(htmlEnDeCode.htmlEncode(_parm.title)));

	$container.unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		$(this).toggleClass("arrow_up");
		if(_parm.slide_target != undefined){
			if($(this).hasClass("arrow_up"))
				$(this).closest(".profile_setting").find("[data-slide_target='" + _parm.slide_target + "']").slideDown();
			else
				$(this).closest(".profile_setting").find("[data-slide_target='" + _parm.slide_target + "']").slideUp();
		}
	});

	return $container;
}
function set_value_Custom_Select(_obj, _id, _value){
	var $items = $(_obj).find("#" + _id + ", #select_" + _id + "");
	var $text = $items.eq(0);
	var $select = $items.eq(1);
	var selected_text = $select.children().removeClass("selected").filter("[value='" + _value + "']").addClass("selected").html();
	$text.html(htmlEnDeCode.htmlEncode(selected_text));
}
function resize_iframe_height(_preheight){
	if(window.parent.$("#vpns_iframe").length == "1"){
		var menu_height = window.parent.$("#mainMenu").height();
		var container_height = $(".container").height();

		if(_preheight != undefined && (typeof _preheight == "number"))
			container_height += _preheight;

		var pop_height = 0;
		if($(".popup_container").css("display") == "flex")
			pop_height = $(".popup_container").height() + $(".popup_container").offset().top;
		$(".popup_container").each(function(index){
			if($(this).css("display") == "flex"){
				pop_height = Math.max(pop_height, ($(this).height() + $(this).offset().top));
			}
		});

		var margin_bottom = 30;
		window.parent.$("#vpns_iframe").css("height", (Math.max(menu_height, container_height, pop_height) + margin_bottom));
	}
}
function showLoading(seconds, flag){
	$("#Loading").css({"width":"", "height":""});
	progress = 100/seconds;
	y = 0;
	LoadingTime(seconds, flag);
}
function show_customize_alert(_text){
	$(".popup_customize_alert").css("display", "flex");
	$(".container, .popup_container.popup_element, .popup_container.popup_element_second").addClass("blur_effect");
	$(".popup_container .popup_customize_alert").empty();
	if(_text != ""){
		$(".popup_container.popup_customize_alert").append(Get_Component_Customize_Alert(_text));
		adjust_popup_container_top($(".popup_container.popup_customize_alert"), 100);
	}
}
function close_popup_container(_obj){
	if(_obj == "all"){
		close_popup();
		close_popup_second();
	}
	else{
		var $popup_container = $(_obj).closest(".popup_container");
		if($popup_container.hasClass("popup_element")){
			close_popup();
		}
		if($popup_container.hasClass("popup_element_second")){
			close_popup_second();
		}
	}
}
function close_popup(){
	$(".popup_element").hide();
	$(".container").removeClass("blur_effect");
	$(".popup_container.popup_element").empty();
	resize_iframe_height();
}
function close_popup_second(){
	$(".popup_element_second").hide().empty();
	$(".popup_container.popup_element_second").removeClass().addClass("popup_container popup_element_second");
	if($(".popup_container.popup_element").css("display") == "none")
		$(".container, .popup_container.popup_element").removeClass("blur_effect");
	else
		$(".popup_container.popup_element").removeClass("blur_effect");
	resize_iframe_height();
}
function close_popup_customize_alert(){
	$(".popup_customize_alert").hide();
	$(".popup_customize_alert").empty();
	if($(".popup_container.popup_element").css("display") == "none" && $(".popup_container.popup_element_second").css("display") == "none")
		$(".container, .popup_container.popup_element, .popup_container.popup_element_second").removeClass("blur_effect");
	if($(".popup_container.popup_element_second").css("display") != "none" || $(".popup_container.popup_element_second").children().length == 0)
		$(".popup_container.popup_element_second").removeClass("blur_effect");
	if($(".popup_container.popup_element").css("display") != "none")
		$(".popup_container.popup_element").removeClass("blur_effect");
	resize_iframe_height();
}
function adjust_popup_container_top(_obj, _offsetHeight){
	$(_obj).css({top: ""});
	var scrollTop = window.pageYOffset || document.documentElement.scrollTop || document.body.scrollTop || 0;
	var parent_scrollTop = parent.window.pageYOffset || parent.document.documentElement.scrollTop || parent.document.body.scrollTop || 0;
	if(scrollTop == 0 && parent_scrollTop != 0)
		parent_scrollTop = parent_scrollTop - 200;
	var final_scrollTop = Math.max(scrollTop, parent_scrollTop);
	if(final_scrollTop != 0){
		$(_obj).css({top: (final_scrollTop + _offsetHeight)});
	}
}