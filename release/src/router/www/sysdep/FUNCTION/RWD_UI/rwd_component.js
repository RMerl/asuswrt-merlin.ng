if ( !Element.prototype.scrollIntoViewIfNeeded ) {
	Element.prototype.scrollIntoViewIfNeeded = function ( centerIfNeeded = true ) {
		const el = this;
		new IntersectionObserver( function( [entry] ) {
			const ratio = entry.intersectionRatio;
			if (ratio < 1) {
				let place = ratio <= 0 && centerIfNeeded ? 'center' : 'nearest';
				el.scrollIntoView( {
					block: place,
					inline: place,
				} );
			}
			this.disconnect();
		} ).observe(this);
	};
}
var ip_RegExp = {
	"IPv4" : "^([01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])\\.([01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])\\.([01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])\\.([01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])$",
	"IPv4_CIDR" : "^([01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])\\.([01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])\\.([01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])\\.([01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])(\/([0-9]|[1-2][0-9]|3[0-2]))$",
	"IPv6" : "^((([0-9A-Fa-f]{1,4}:){7}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){6}:[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){5}:([0-9A-Fa-f]{1,4}:)?[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){4}:([0-9A-Fa-f]{1,4}:){0,2}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){3}:([0-9A-Fa-f]{1,4}:){0,3}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){2}:([0-9A-Fa-f]{1,4}:){0,4}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){6}((\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b)\\.){3}(\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b))|(([0-9A-Fa-f]{1,4}:){0,5}:((\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b)\\.){3}(\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b))|(::([0-9A-Fa-f]{1,4}:){0,5}((\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b)\\.){3}(\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b))|([0-9A-Fa-f]{1,4}::([0-9A-Fa-f]{1,4}:){0,5}[0-9A-Fa-f]{1,4})|(::([0-9A-Fa-f]{1,4}:){0,6}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){1,7}:))$",
	"IPv6_CIDR" : "^((([0-9A-Fa-f]{1,4}:){7}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){6}:[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){5}:([0-9A-Fa-f]{1,4}:)?[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){4}:([0-9A-Fa-f]{1,4}:){0,2}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){3}:([0-9A-Fa-f]{1,4}:){0,3}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){2}:([0-9A-Fa-f]{1,4}:){0,4}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){6}((\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b)\\.){3}(\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b))|(([0-9A-Fa-f]{1,4}:){0,5}:((\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b)\\.){3}(\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b))|(::([0-9A-Fa-f]{1,4}:){0,5}((\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b)\\.){3}(\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b))|([0-9A-Fa-f]{1,4}::([0-9A-Fa-f]{1,4}:){0,5}[0-9A-Fa-f]{1,4})|(::([0-9A-Fa-f]{1,4}:){0,6}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){1,7}:))(\/([0-9]|[1-9][0-9]|1[01][0-9]|12[0-8]))$"
};
jQuery.fn.show_validate_hint = function(hintStr){
	$(this).closest(".profile_setting").find(".validate_hint").remove();

	$("<div>")
		.html(hintStr)
		.addClass("validate_hint")
		.insertAfter($(this).closest(".profile_setting_item, .profile_setting_two_item"));

		resize_iframe_height();
}
function Get_Component_Customize_Alert(_text){
	var $popup_content_container = $("<div>").addClass("popup_content_container");

	var $customize_alert = $("<div>").addClass("customize_alert");
	$customize_alert.appendTo($popup_content_container);
	var $desc = $("<div>").addClass("desc").html(_text);
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
function Get_Component_Customize_Confirm(_text){
	let $popup_content_container = $("<div>").addClass("popup_content_container");

	let $customize_confirm = $("<div>").addClass("customize_alert confirm").appendTo($popup_content_container);
	let $desc = $("<div>").addClass("desc").html(_text).appendTo($customize_confirm);
	let $action_btn_container = $("<div>").addClass("action_btn_container").appendTo($customize_confirm);
	$("<div>").attr({"data-btn":"ok"}).addClass("ok btn").html("<#CTL_ok#>")
		.unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			close_popup_customize_alert();
		}).appendTo($action_btn_container);

	$("<div>").attr({"data-btn":"cancel"}).addClass("cancel btn").html("<#CTL_Cancel#>")
		.unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			close_popup_customize_alert();
		}).appendTo($action_btn_container);

	return $popup_content_container;
}
function Get_Component_Del_Profile(){
	var $popup_content_container = $("<div>").addClass("popup_content_container");

	var $del_profile = $("<div>").addClass("del_profile");
	$del_profile.appendTo($popup_content_container);
	var $title = $("<div>").addClass("title").html("Delete Profile");/* untranslated */
	$title.appendTo($del_profile);
	var $desc = $("<div>").addClass("desc").html("<#VPN_Fusion_Delete_Alert#>");
	$desc.appendTo($del_profile);
	var $action_btn_container = $("<div>").addClass("action_btn_container");
	$action_btn_container.appendTo($del_profile);
	var $del = $("<div>").attr({"data-btn":"del"}).addClass("del btn").html("<#CTL_del#>");
	$del.appendTo($action_btn_container);
	var $cancel = $("<div>").addClass("cancel btn").html("<#CTL_Cancel#>");
	$cancel.appendTo($action_btn_container);
	$cancel.unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		close_popup_container($popup_content_container);
	});

	return $popup_content_container;
}
function Get_Component_Popup_Profile_Title(_text){
	var $popup_title_container = $("<div>").addClass("popup_title_container");
	$("<div>").addClass("title").attr({"data-container":"profile_title"}).html(htmlEnDeCode.htmlEncode(_text)).appendTo($popup_title_container);
	var $close_btn = $("<div>").attr({"id":"title_close_btn"}).addClass("close_btn");
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
	$("<div>").addClass("title").attr({"data-container":"profile_title"}).html(htmlEnDeCode.htmlEncode(_text)).appendTo($title_container);
	var $del_btn = $("<div>").attr({"id":"title_del_btn"}).addClass("del_btn");
	$del_btn.appendTo($title_container);
	return $title_container;
}
function Get_Component_Pure_Text(_parm){
	let $container = $("<div>").addClass("profile_setting_item textView_item");
	if(_parm.container_id != undefined)
		$container.attr("id", _parm.container_id);
	let pure_text = "";
	if(_parm.text != undefined)
		pure_text = _parm.text;

	if(_parm.openHint != undefined){
		let hint_array = _parm.openHint.split("_");
		$("<a>").addClass("hintstyle").attr({"href":"javascript:void(0);"}).html(htmlEnDeCode.htmlEncode(_parm.title)).unbind("click").click(function(){
			openHint(hint_array[0], hint_array[1], "rwd_vpns");
		}).appendTo($("<div>").addClass("title").appendTo($container));
	}
	else
		$("<div>").addClass("title").html(htmlEnDeCode.htmlEncode(_parm.title)).appendTo($container);

	let $input_container = $("<div>").addClass("input_container").appendTo($container);
	$("<div>")
		.addClass("textViewOnly")
		.attr({"id":_parm.id})
		.html(htmlEnDeCode.htmlEncode(pure_text))
		.appendTo($input_container);

	return $container;
}
function Get_Component_Pure_Text_And_Btn(_parm){
	var $container = $("<div>").addClass("profile_setting_two_item nowrap");
	if(_parm.container_id != undefined)
		$container.attr("id", _parm.container_id);

	var $cntr_1 = $("<div>").appendTo($container);

	if(_parm.openHint != undefined){
		var hint_array = _parm.openHint.split("_");
		$("<a>").addClass("hintstyle").attr({"href":"javascript:void(0);"}).html(htmlEnDeCode.htmlEncode(_parm.title)).unbind("click").click(function(){
			openHint(hint_array[0], hint_array[1], "rwd_vpns");
		}).appendTo($("<div>").addClass("title").appendTo($cntr_1));
	}
	else
		$("<div>").addClass("title").html(htmlEnDeCode.htmlEncode(_parm.title)).appendTo($cntr_1);

	var $input_container = $("<div>").addClass("input_container").appendTo($cntr_1);
	$("<div>").addClass("textViewOnly").attr({"id":_parm.id}).html(htmlEnDeCode.htmlEncode(_parm.text)).appendTo($input_container);

	var $cntr_2 = $("<div>").appendTo($container);
	var btn_text = "";
	if(_parm.btn_text != undefined)
		 btn_text = _parm.btn_text;
	var $btn_cntr = $("<div>").addClass("profile_btn_container").appendTo($cntr_2);
	$("<div>").html(htmlEnDeCode.htmlEncode(btn_text)).appendTo($btn_cntr);
	$("<div>").addClass("icon_arrow_right").appendTo($btn_cntr);

	return $container;
}
function Get_Component_Btn(_parm){
	var $container = $("<div>").addClass("profile_setting_item nowrap");

	var btn_text = "";
	if(_parm.text != undefined)
		 btn_text = _parm.text;
	var $btn_cntr = $("<div>").addClass("profile_btn_container").appendTo($container);
	if(_parm.id != undefined)
		$btn_cntr.attr("id", _parm.id);
	$("<div>").html(htmlEnDeCode.htmlEncode(btn_text)).appendTo($btn_cntr);
	$("<div>").addClass("icon_arrow_right").appendTo($btn_cntr);

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
	if(_parm.type == "password"){
		$("<div>").addClass("icon_lock").appendTo($input_container);
	}
	var $input = $("<input/>")
					.addClass("textInput")
					.attr({"id":_parm.id, "type":_parm.type, "maxlength":maxlength, "autocomplete":"off","autocorrect":"off","autocapitalize":"off","spellcheck":"false"})
					.val(set_value)
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
		$input.addClass("padding_lock")
		$("<div>").addClass("icon_eye close").attr({"for": _parm.id}).unbind("click").click(function(){
			var targetObj = $(this);
			$(this).toggleClass("close open");
			$("#"+$(this).attr("for")).prop("type", (function(){return targetObj.hasClass("icon_eye close") ? "password" : "text";}()));
		}).appendTo($input_container);
	}

	if(_parm.placeholder)
		$input.attr("placeholder", htmlEnDeCode.htmlEncode(_parm.placeholder));

	return $container;
}
function Get_Component_Input_And_Btn(_parm){
	var maxlength = 64;
	if(_parm.maxlength != undefined)
		maxlength = _parm.maxlength;
	var set_value = "";
	if(_parm.set_value != undefined)
		set_value = _parm.set_value;

	var $container = $("<div>").addClass("profile_setting_two_item nowrap input_btn");
	if(_parm.container_id != undefined)
		$container.attr("id", _parm.container_id);

	var $cntr_1 = $("<div>").addClass("cntr_1").appendTo($container);

	if(_parm.openHint != undefined){
		var hint_array = _parm.openHint.split("_");
		$("<a>").addClass("hintstyle").attr({"href":"javascript:void(0);"}).html(htmlEnDeCode.htmlEncode(_parm.title)).unbind("click").click(function(){
			openHint(hint_array[0], hint_array[1], "rwd_vpns");
		}).appendTo($("<div>").addClass("title").appendTo($cntr_1));
	}
	else
		$("<div>").addClass("title").html(htmlEnDeCode.htmlEncode(_parm.title)).appendTo($cntr_1);

	var $input_container = $("<div>").addClass("input_container").appendTo($cntr_1);
	if(_parm.type == "password"){
		$("<div>").addClass("icon_lock").appendTo($input_container);
	}
	var $input = $("<input/>")
					.addClass("textInput")
					.attr({"id":_parm.id, "type":_parm.type, "maxlength":maxlength, "autocomplete":"off","autocorrect":"off","autocapitalize":"off","spellcheck":"false"})
					.val(set_value)
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
		$input.addClass("padding_lock")
		$("<div>").addClass("icon_eye close").attr({"for": _parm.id}).unbind("click").click(function(){
			var targetObj = $(this);
			$(this).toggleClass("close open");
			$("#"+$(this).attr("for")).prop("type", (function(){return targetObj.hasClass("icon_eye close") ? "password" : "text";}()));
		}).appendTo($input_container);
	}

	if(_parm.placeholder)
		$input.attr("placeholder", htmlEnDeCode.htmlEncode(_parm.placeholder));

	var $cntr_2 = $("<div>").addClass("cntr_2").appendTo($container);
	var btn_text = "";
	if(_parm.btn_text != undefined)
		 btn_text = _parm.btn_text;
	var $btn_cntr = $("<div>").addClass("profile_btn_container").appendTo($cntr_2);
	$("<div>").html(htmlEnDeCode.htmlEncode(btn_text)).appendTo($btn_cntr);
	$("<div>").addClass("icon_arrow_right").appendTo($btn_cntr);

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

	var set_value = "";
	if(_parm.set_value != undefined)
		set_value = _parm.set_value;

	var $input_container = $("<div>").addClass("input_container").appendTo($container);
	var $input = $("<textarea/>")
					.addClass("textareaInput")
					.attr({"id":_parm.id, "rows":rows, "cols":cols, "maxlength":maxlength})
					.val(set_value)
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
		if(_parm.options.length == 0)
			return;
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

	var selected_text = "<#Setting_factorydefault_value#>";
	if(specific_option != undefined)
		selected_text = specific_option.text;
	var $selected_text = $("<div>").attr("id", _parm.id).addClass("sel_text").appendTo($custom_select_container).html(htmlEnDeCode.htmlEncode(selected_text));
	// space
	$("<div>").css("height", "3px").appendTo($custom_select_container);
	// select options
	var $select_options_container = $("<div>").attr("id", "select_" + _parm.id).addClass("select_options_container container_hide").appendTo($custom_select_container);
	_parm.options.forEach(function(item){
		var $option = $("<div>").attr("value", item.value).html(htmlEnDeCode.htmlEncode(item.text))
				.unbind("click").click(function(e){
					e = e || event;
					e.stopPropagation();
					if($(this).attr("data-disabled") == "true")
						return false;
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
function Get_Component_Custom_Select_And_Btn(_parm){
	var $container = $("<div>").addClass("profile_setting_two_item nowrap input_btn");
	if(_parm.container_id != undefined)
		$container.attr("id", _parm.container_id);

	var $cntr_1 = $("<div>").addClass("cntr_1").appendTo($container);

	// title
	if(_parm.openHint != undefined){
		var hint_array = _parm.openHint.split("_");
		$("<a>").addClass("hintstyle").attr({"href":"javascript:void(0);"}).html(htmlEnDeCode.htmlEncode(_parm.title)).unbind("click").click(function(){
			openHint(hint_array[0], hint_array[1], "rwd_vpns");
		}).appendTo($("<div>").addClass("title").appendTo($cntr_1));
	}
	else
		$("<div>").addClass("title").html(htmlEnDeCode.htmlEncode(_parm.title)).appendTo($cntr_1);

	// input field
	var $input_container = $("<div>").addClass("input_container").appendTo($cntr_1);
	var $custom_select_container = $("<div>").addClass("custom_select_container").appendTo($input_container);
	$custom_select_container.unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		if(_parm.options.length == 0)
			return;
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

	var selected_text = "<#Setting_factorydefault_value#>";
	if(specific_option != undefined)
		selected_text = specific_option.text;
	var $selected_text = $("<div>").attr("id", _parm.id).addClass("sel_text").appendTo($custom_select_container).html(htmlEnDeCode.htmlEncode(selected_text));
	// space
	$("<div>").css("height", "3px").appendTo($custom_select_container);
	// select options
	var $select_options_container = $("<div>").attr("id", "select_" + _parm.id).addClass("select_options_container container_hide").appendTo($custom_select_container);
	_parm.options.forEach(function(item){
		var $option = $("<div>").attr("value", item.value).html(htmlEnDeCode.htmlEncode(item.text))
				.unbind("click").click(function(e){
					e = e || event;
					e.stopPropagation();
					if($(this).attr("data-disabled") == "true")
						return false;
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

	var $cntr_2 = $("<div>").addClass("cntr_2").appendTo($container);
	var btn_text = "";
	if(_parm.btn_text != undefined)
		 btn_text = _parm.btn_text;
	var $btn_cntr = $("<div>").addClass("profile_btn_container").appendTo($cntr_2);
	$("<div>").html(htmlEnDeCode.htmlEncode(btn_text)).appendTo($btn_cntr);
	$("<div>").addClass("icon_arrow_right").appendTo($btn_cntr);

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
	var $switch_icon = $("<div>").attr("id", _parm.id).addClass("icon_switch on").unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		if($(this).attr("temp_disable") == "disabled")
			return;
		$(this).toggleClass("off on");
	}).appendTo($input_container);

	if(_parm.set_value != undefined && (_parm.set_value == "on" || _parm.set_value == "off"))
		$switch_icon.removeClass("off on").addClass(_parm.set_value);

	return $container;
}
function Get_Component_Switch_Text(_parm){
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

	var $input_container = $("<div>").addClass("input_container").appendTo($container);

	var $switch_text_container = $("<div>").addClass("switch_text_container").appendTo($input_container);
	_parm.options.forEach(function(item){
		$("<div>").addClass("switch_text_item").html(htmlEnDeCode.htmlEncode(item.text)).attr({"data-option-id":item.option_id})
			.unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				$(this).closest(".switch_text_container").find(".switch_text_item").removeClass("selected");
				$(this).addClass("selected");
			}).appendTo($switch_text_container);
	});
	$switch_text_container.children().first().addClass("selected");

	return $container;
}
function Get_Component_Category_Slide_Title(_parm) {
	var $container = $("<div>").addClass("category_slide_title");
	if(_parm.id != undefined){
		$container.attr("id", _parm.id);
	}
	var $title_cntr = $("<div>").addClass("title_cntr").appendTo($container);
	$title_cntr.append($("<span>").addClass("title").html(htmlEnDeCode.htmlEncode(_parm.title)));
	$title_cntr.unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		$parent_cntr = $(this).parent(".category_slide_title");
		$parent_cntr.toggleClass("expand");
		if(_parm.slide_target != undefined){
			if($parent_cntr.hasClass("expand"))
				$(this).closest(".profile_setting").find("[data-slide_target='" + _parm.slide_target + "']").slideDown();
			else
				$(this).closest(".profile_setting").find("[data-slide_target='" + _parm.slide_target + "']").slideUp();
		}
	});
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
function Get_Component_Error_Hint(_parm){
	var error_text = "";
	if(_parm.text != undefined)
		error_text = _parm.text;
	var $container = $("<div>").addClass("error_hint_container").hide();
	$container.unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		return;
	});
	var $text_cntr =  $("<div>").addClass("text_cntr").appendTo($container);
	$("<div>").append($("<div>").addClass("icon_error_outline")).appendTo($text_cntr);
	$("<div>").html(htmlEnDeCode.htmlEncode(error_text)).appendTo($text_cntr);
	var $action_cntr =  $("<div>").addClass("action_cntr").appendTo($container);
	var $ok = $("<div>").addClass("ok btn").html("<#CTL_ok#>");
	$ok.appendTo($action_cntr);
	$ok.unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		$(this).closest(".error_hint_container").hide();
	});
	return $container
}
function set_value_Custom_Select(_obj, _id, _value){
	var $items = $(_obj).find("#" + _id + ", #select_" + _id + "");
	var $text = $items.eq(0);
	var $select = $items.eq(1);
	var selected_text = $select.children().removeClass("selected").filter("[value='" + _value + "']").addClass("selected").html();
	if(selected_text == undefined){//error handle, select first option.
		selected_text = $select.children().removeClass("selected").filter(":first").addClass("selected").html();
	}
	$text.html(htmlEnDeCode.htmlEncode(selected_text));
}
function resize_iframe_height(_preheight){
	if($(parent.document).find(".rwd_iframe").length == "1"){
		var menu_height = $(parent.document).find("#mainMenu").height();
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
		$(parent.document).find(".rwd_iframe").css("height", (Math.max(menu_height, container_height, pop_height) + margin_bottom));
	}
}
function showLoading_RWD(seconds, flag){
	$("#Loading").css({"width":"", "height":""});
	progress = 100/seconds;
	y = 0;
	LoadingTime(seconds, flag);
}
function show_customize_alert(_text){
	$(".popup_customize_alert").css("display", "flex");
	$(".container, .popup_container.popup_element, .popup_container.popup_element_second").addClass("blur_effect");
	$(".popup_container .popup_customize_alert").empty();
	$(".popup_container.popup_customize_alert").append(Get_Component_Customize_Alert(_text));
	adjust_popup_container_top($(".popup_container.popup_customize_alert"), 100);

}
function show_customize_confirm(_text){
	$(".popup_customize_alert").css("display", "flex");
	$(".container, .popup_container.popup_element, .popup_container.popup_element_second").addClass("blur_effect");
	$(".popup_container .popup_customize_alert").empty();
	$(".popup_container.popup_customize_alert").append(Get_Component_Customize_Confirm(_text));
	adjust_popup_container_top($(".popup_container.popup_customize_alert"), 100);
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
	$(".container, .qis_container").removeClass("blur_effect");
	$(".popup_container.popup_element").removeClass().addClass("popup_container popup_element").empty();
	resize_iframe_height();
}
function close_popup_second(){
	$(".popup_element_second").hide().empty();
	$(".popup_container.popup_element_second").removeClass().addClass("popup_container popup_element_second");
	if($(".popup_container.popup_element").css("display") == "none")
		$(".container, .qis_container, .popup_container.popup_element").removeClass("blur_effect");
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