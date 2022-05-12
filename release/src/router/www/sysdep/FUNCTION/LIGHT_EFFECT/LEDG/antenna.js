var antenna_item_post = {
	"LED_Set" : {"R" : 0, "G" : 255, "B" : 0},
	"Mode" : 1
}
var antenna_animate = {
	"interval" : false,
	"breathing" : function(_obj, _rgbArr) {
		var breathing_color = function(_obj, _rgbArr) {
			_obj.animate({
				backgroundColor: "rgba("+ _rgbArr[0] + ", " + _rgbArr[1] + ", " + _rgbArr[2] + ", 0.2)"
			}, 1000*3);

			_obj.animate({
				backgroundColor: "rgba("+ _rgbArr[0] + ", " + _rgbArr[1] + ", " + _rgbArr[2] + ", 1)"
			}, 1000*3);

		};
		breathing_color(_obj, _rgbArr);
		antenna_animate.interval = setInterval(function(){
			breathing_color(_obj, _rgbArr);
		}, 1000*6);
	},
	"clear" : function(_obj) {
		clearInterval(antenna_animate.interval);
		_obj.stop(true,true);
	}
}
function init_antenna_led(_ledg_scheme){
	var antled_info = httpApi.nvramGet(["antled_scheme", "antled_scheme_old"], true);
	var antled_scheme = antled_info.antled_scheme;
	var antled_scheme_old = antled_info.antled_scheme_old;

	antenna_item_post.Mode = antled_scheme;
	if(antled_scheme == "0" && antled_scheme_old != "")
		antenna_item_post.Mode = antled_scheme_old;

	$("#antenna_setting .setting_scheme .scheme_bg[mode=" + antenna_item_post.Mode + "]").addClass("active");
	if(_ledg_scheme == "0")
		antled_scheme = "0";

	if(antled_scheme == "0") {
		antenna_animate.clear($("#antenna_bg"));
		$("#antenna_bg").removeClass().addClass("light_effect_bg").css("background", "initial");
	}
	else {
		antenna_set_scheme_mode();
	}

	$("#antenna_setting .setting_scheme .scheme_bg").unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		var id = e.currentTarget.id;
		if(id == "")
			return;
		antenna_animate.clear($("#antenna_bg"));
		$("#antenna_bg").removeClass().addClass("light_effect_bg");
		antenna_item_post.Mode = e.currentTarget.getAttribute("mode");
		$("#antenna_setting .setting_scheme .scheme_bg").removeClass("active");
		$("#antenna_setting .setting_scheme #" + id + "").addClass("active");
		antenna_set_scheme_mode();
		var postData = {"antled_scheme": antenna_item_post.Mode};
		httpApi.set_antled(postData);
	});
}
function antenna_set_scheme_mode(){
	var mode = antenna_item_post.Mode;
	var rgbArr = [antenna_item_post["LED_Set"]["R"], antenna_item_post["LED_Set"]["G"], antenna_item_post["LED_Set"]["B"]];
	switch(mode) {
		case "1" :
			$("#antenna_bg").css("background", "rgb("+ rgbArr[0] + ", " + rgbArr[1] + ", " + rgbArr[2] + ")");
			break;
		case "2" :
			$("#antenna_bg").css("background", "rgb("+ rgbArr[0] + ", " + rgbArr[1] + ", " + rgbArr[2] + ")");
			antenna_animate.breathing($("#antenna_bg"), rgbArr);
			break;
	}
}
function antenna_switch_enable(switch_status){
	antenna_animate.clear($("#antenna_bg"));
	if(switch_status){
		antenna_item_post.Mode = $("#antenna_setting .scheme_bg.active").attr("Mode");
		antenna_set_scheme_mode();
	}
	else{
		antenna_item_post.Mode = "0";
		$("#antenna_bg").removeClass().addClass("light_effect_bg").css("background", "initial");
	}
	var postData = {"antled_scheme": antenna_item_post.Mode};
	httpApi.set_antled(postData);
}