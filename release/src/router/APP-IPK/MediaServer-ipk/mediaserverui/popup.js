// JavaScript Document
var winH,winW;
		
function winW_H(){
	if(parseInt(navigator.appVersion) > 3){
		winW = document.documentElement.scrollWidth;
		if(document.documentElement.clientHeight > document.documentElement.scrollHeight)
			winH = document.documentElement.clientHeight;
		else
			winH = document.documentElement.scrollHeight;
	}
} 

function getWH_pop(){
	var winWidth;
	var winHeight;
	winWidth = document.documentElement.scrollWidth;
	if(document.documentElement.clientHeight > document.documentElement.scrollHeight)
	winHeight = document.documentElement.clientHeight;
	else
	winHeight = document.documentElement.scrollHeight;
	$("Loading").style.width = winWidth+"px";
	$("Loading").style.height = winHeight+"px";
	
	}

function LoadingTime(seconds, flag){
	showtext($("proceeding_main_txt"), multiLanguage_array[multi_INT][35]+"...");
	setInterval('getWH_pop();',1000);
	$("Loading").style.visibility = "visible";
	
	y = y+progress;
	if(typeof(seconds) == "number" && seconds >= 0){
		if(seconds != 0){
			showtext($("proceeding_main_txt"), multiLanguage_array[multi_INT][36]);
			showtext($("proceeding_txt"), Math.round(y)+"% "+multiLanguage_array[multi_INT][35]);
			--seconds;
			setTimeout("LoadingTime("+seconds+", '"+flag+"');", 1000);
		}
		else{
			showtext($("proceeding_main_txt"), translate(multiLanguage_array[multi_INT][37]));
			showtext($("proceeding_txt"), "");
			y = 0;
			
			if(flag != "waiting")
				setTimeout("hideLoading();",1000);			
		}
	}
}
function LoadingProgress(seconds){
	$("LoadingBar").style.visibility = "visible";
	
	y = y + progress;
	if(typeof(seconds) == "number" && seconds >= 0){
		if(seconds != 0){
			$("proceeding_img").style.width = Math.round(y) + "%";
			$("proceeding_img_text").innerHTML = Math.round(y) + "%";
			--seconds;
			setTimeout("LoadingProgress("+seconds+");", 1000);
		}
		else{
			$("proceeding_img_text").innerHTML = "Complete!";
			y = 0;
			setTimeout("hideLoadingBar();",1000);
			location.href = "index.asp";
		}
	}
}

function showLoading(seconds, flag){
	disableCheckChangedStatus();
	
	if(location.pathname.indexOf("QIS_wizard.htm") < 0)
		hideHint();
	clearHintTimeout();
	
	htmlbodyforIE = document.getElementsByTagName("html");  //this both for IE&FF, use "html" but not "body" because <!DOCTYPE html PUBLIC.......>
	//htmlbodyforIE[0].style.overflow = "hidden";	  //hidden the Y-scrollbar for preventing from user scroll it.
	
	winW_H();
	var blockmarginTop;
	var sheight = document.documentElement.scrollHeight;
	var cheight = document.documentElement.clientHeight

	//alert("document.documentElement.scrollTop: "+document.documentElement.scrollTop + "\ndocument.documentElement.scrollHeight: "+ document.documentElement.scrollHeight + "\ndocument.documentElement.clientHeight: "+ document.documentElement.clientHeight);
	//blockmarginTop = (navigator.userAgent.indexOf("Safari")>=0)?document.documentElement.scrollHeight - document.documentElement.clientHeight+200:document.documentElement.scrollTop+200;
	blockmarginTop = (navigator.userAgent.indexOf("Safari")>=0)?(sheight-cheight<=0)?200:sheight-cheight+200:document.documentElement.scrollTop+200;
	
	//Lock modified it for Safari4 display issue.
	$("loadingBlock").style.marginTop = 200+"px";
	$("Loading").style.width = winW+"px";
	$("Loading").style.height = winH+"px";
	
	loadingSeconds = seconds;
	progress = 100/loadingSeconds;
	y = 0;
	
	LoadingTime(seconds, flag);
}

function dr_advise(){
	disableCheckChangedStatus();
	
	clearHintTimeout();
	
	htmlbodyforIE = document.getElementsByTagName("html");  //this both for IE&FF, use "html" but not "body" because <!DOCTYPE html PUBLIC.......>
	//htmlbodyforIE[0].style.overflow = "hidden";	  //hidden the Y-scrollbar for preventing from user scroll it.
	
	winW_H();
	var blockmarginTop;
	blockmarginTop = document.documentElement.scrollTop + 200;	
	$("dr_sweet_advise").style.marginTop = blockmarginTop+"px"
	$("hiddenMask").style.width = winW+"px";
	$("hiddenMask").style.height = winH+"px";	
	$("hiddenMask").style.visibility = "visible";
}

function showLoadingBar(seconds){
	disableCheckChangedStatus();
	
	if(location.pathname.indexOf("QIS_wizard.htm") < 0)
		hideHint();
	clearHintTimeout();
	
	htmlbodyforIE = document.getElementsByTagName("html");  //this both for IE&FF, use "html" but not "body" because <!DOCTYPE html PUBLIC.......>
	//htmlbodyforIE[0].style.overflow = "hidden";	  //hidden the Y-scrollbar for preventing from user scroll it.
	
	winW_H();
	var blockmarginTop;
	blockmarginTop = document.documentElement.scrollTop + 200;
	$("loadingBarBlock").style.marginTop = blockmarginTop+"px";
	$("LoadingBar").style.width = winW+"px";
	$("LoadingBar").style.height = winH+"px";
	
	loadingSeconds = seconds;
	progress = 100/loadingSeconds;
	y = 0;
	LoadingProgress(seconds);
}

function hideLoadingBar(){
	enableCheckChangedStatus();
	$("LoadingBar").style.visibility = "hidden";
}

function hideLoading(flag){
	if(flag != "noDrSurf")
		enableCheckChangedStatus();
	
	$("Loading").style.visibility = "hidden";
}             

function simpleSSID(obj){
	var SSID = document.loginform.wl_ssid.value;
	
	if(SSID.length < 16)
		showtext(obj, SSID);
	else{
		obj.title = SSID;
		showtext(obj, SSID.substring(0, 16)+"...");
	}
}
