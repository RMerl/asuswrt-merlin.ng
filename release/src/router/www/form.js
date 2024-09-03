// Form validation and smooth UI

function combineIP(obj){ //combine all IP info before validation and submit
	
	var IP_List = document.getElementById(obj+"_div").childNodes;
	var IP_str = "";
	
	for(var i=0; i < IP_List.length; i++){	
		//alert("\'"+ IP_List[i].nodeValue+ "\'");
		if(IP_List[i].type == "text"){
			if(IP_List[i].value != ""){
				IP_List[i].value = parseInt(IP_List[i].value,10);
			}
			
			IP_str += IP_List[i].value;
		}
		else if(IP_List[i].nodeValue.indexOf(".") != -1){
			IP_str += ".";                                  
		}
	}
	if(IP_str != "..."){
		document.getElementById(obj).value = IP_str;
	}else{
		document.getElementById(obj).value = "";
	}
}

function IPinputCtrl(obj, t){
	var IP_List = document.getElementById(obj.name+"_div").childNodes;
	
	document.getElementById(obj.name+"_div").style.background = (t==0)?"#999999":"#FFFFFF";
	for(var i=0; i < IP_List.length; i++){
		if(IP_List[i].type == "text"){
			IP_List[i].disabled = (t==0)?true:false;
			IP_List[i].style.color = (t==0)?"#FFFFFF":"#000000";
			IP_List[i].style.background = (t==0)?"#999999":"#FFFFFF";
		}
	}
}

function cal_panel_block(obj, multiple) {
	var isMobile = function() {
		var tmo_support = ('<% nvram_get("rc_support"); %>'.search("tmo") == -1) ? false : true;
		if(!tmo_support)
			return false;
		
		if(	navigator.userAgent.match(/iPhone/i)	|| 
			navigator.userAgent.match(/iPod/i)		||
			navigator.userAgent.match(/iPad/i)		||
			(navigator.userAgent.match(/Android/i) && (navigator.userAgent.match(/Mobile/i) || navigator.userAgent.match(/Tablet/i))) ||
			(navigator.userAgent.match(/Opera/i) && (navigator.userAgent.match(/Mobi/i) || navigator.userAgent.match(/Mini/i))) ||	// Opera mobile or Opera Mini
			navigator.userAgent.match(/IEMobile/i)	||	// IE Mobile
			navigator.userAgent.match(/BlackBerry/i)	//BlackBerry
		 ) {
			return true;
		}
		else {
			return false;
		}
	};
	var blockmarginLeft;
	var winWidth = 0;
	if (window.innerWidth) {
		winWidth = window.innerWidth;
	}
	else if ((document.body) && (document.body.clientWidth)) {
		winWidth = document.body.clientWidth;
	} else {
		winWidth = 1105;
	}

	if (document.documentElement  && document.documentElement.clientHeight && document.documentElement.clientWidth) {
		winWidth = document.documentElement.clientWidth;
	}

	if(winWidth > 1050) {
		winPadding = (winWidth - 1050) / 2;
		winWidth = 1105;
		blockmarginLeft = (winWidth * multiple) + winPadding;
	}
	else if(winWidth <= 1050) {
		if(isMobile()) {
			if(document.body.scrollLeft < 50) {
				blockmarginLeft= (winWidth) * multiple + document.body.scrollLeft;
			}
			else if(document.body.scrollLeft >320) {
				blockmarginLeft = 320;
			}
			else {
				blockmarginLeft = document.body.scrollLeft;
			}	
		}
		else {
			blockmarginLeft = (winWidth) * multiple + document.body.scrollLeft;	
		}
	}

	if(re_mode == "1"){
		document.getElementById(obj).style.left = "50%";
		document.getElementById(obj).style.marginLeft = "-250px";
	}
	else
		document.getElementById(obj).style.marginLeft = blockmarginLeft + "px";
}

function adjust_TM_eula_height(_objID) {
	var scrollTop = window.pageYOffset || document.documentElement.scrollTop || document.body.scrollTop || 0;
	document.getElementById(_objID).style.top = (scrollTop + 10) + "px";
	var visiable_height = document.documentElement.clientHeight;
	var tm_eula_container_height = parseInt(document.getElementById(_objID).offsetHeight);
	var tm_eula_visiable_height = visiable_height - tm_eula_container_height;
	if(tm_eula_visiable_height < 0) {
		var tm_eula_content_height = parseInt(document.getElementById("tm_eula_content").style.height);
		document.getElementById("tm_eula_content").style.height = (tm_eula_content_height - Math.abs(tm_eula_visiable_height) - 20) + "px"; //content height - overflow height - margin top and margin bottom
	}
}
function adjust_panel_block_top(_objID, _offsetHeight) {
	var scrollTop = window.pageYOffset || document.documentElement.scrollTop || document.body.scrollTop || 0;
	document.getElementById(_objID).style.top = (scrollTop + _offsetHeight) + "px";
}

var FAQ_List = "";
function updatFAQListOnline(){

  $.getJSON("/ajax/collected_FAQ.json", function(local_data){
    FAQ_List = Object.keys(local_data).map(function(e){
        return local_data[e];
    });

    $.getJSON("https://nw-dlcdnet.asus.com/plugin/js/collected_FAQ.json",
      function(cloud_data){
        if(JSON.stringify(local_data) != JSON.stringify(cloud_data)){
          if(Object.keys(cloud_data).length > 0){
            FAQ_List = Object.keys(cloud_data).map(function(e){
              return cloud_data[e];
            });
          }
        }
      }
    );

  });
}

function get_faq_index(FAQ_List, _cur_page, _index){
    var index_tmp=0;
    if(FAQ_List == ""){updatFAQListOnline();}
    for(var x=0;x<FAQ_List.length;x++){
       if(FAQ_List[x].Page == _cur_page){
         switch (_index){
             case 1:
                 index_tmp = FAQ_List[x].FAQ_Link;
                 break;
             case 2:
                 index_tmp = FAQ_List[x].FAQ_Link2;
                 break;
             case 3:
                 index_tmp = FAQ_List[x].FAQ_Link3;
                 break;
             default:
                 index_tmp = FAQ_List[x].FAQ_Link;
                 break;
         }
         return index_tmp;
       }
    }
}


function show_feature_desc(_text1, _text2, _text3){
  var text_tmp1 = (_text1)? _text1 : "";
  var text_tmp2 = (_text2)? _text2 : "";
  var text_tmp3 = (_text3)? _text3 : "";
  $(".container").addClass("blur_effect");
  if($(".popup_container.popup_element").css("display") == "flex"){
    $(".popup_container.popup_element").addClass("blur_effect");
  }
  $(".popup_element_second").css("display", "flex");
  if(current_page.indexOf("Advanced_System_Content") >= 0){
    if(top.webWrapper)
      $(".popup_element_second").css("margin-top","700px");
    else
      $(".popup_element_second").css("margin-top","490px");
  }
  $(".popup_container.popup_element_second").empty();
  $(".popup_container.popup_element_second").append(Get_Component_Feature_Desc(_text1, _text2, _text3));

    function close_popup(){
      $(".popup_element_second").hide();
      $(".container, .qis_container").removeClass("blur_effect");
      $(".popup_container.popup_element").removeClass().addClass("popup_container popup_element").empty();
    }


  function Get_Component_Feature_Desc(_text1, _text2, _text3){
    if(_text1){
      var href_index = get_faq_index(FAQ_List, current_page, 1);
    }
    if(_text2){
      var href_index2 = get_faq_index(FAQ_List, current_page, 2);
    }
    if(_text3){
      var href_index3 = get_faq_index(FAQ_List, current_page, 3);
    }

    var faq_text = "";
    if(_text1){
      faq_text += '<div class="text-list"><div class="icon-circle-mask"><i class="icon-comments"></i></div><a id="link_info_faq" target="_blank" href="https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang='+ui_lang;
      faq_text += '&kw=&num='+href_index+'">';
      faq_text += _text1;
      faq_text += '</a></div>';
    }
    if(_text2){
      faq_text += '<div class="text-list"><div class="icon-circle-mask"><i class="icon-comments"></i></div><a id="link_info_faq" target="_blank" href="https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang='+ui_lang;
      faq_text += '&kw=&num='+href_index2+'">';
      faq_text += _text2;
      faq_text += "</a></div>";
    }
    if(_text3){
      faq_text += '<div class="text-list"><div class="icon-circle-mask"><i class="icon-comments"></i></div><a id="link_info_faq" target="_blank" href="https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang='+ui_lang;
      faq_text += '&kw=&num='+href_index3+'">';
      faq_text += _text3;
      faq_text += "</a></div>";
    }

    var $container = $("<div>");
    var $popup_title_container = $("<div>").addClass("popup_title_container");
    $popup_title_container.appendTo($container);
    $("<div>").addClass("title").html("<#NewFeatureAbout#>").appendTo($popup_title_container);
    var $close_btn = $("<div>").addClass("close_btn").html('&times;');
    $close_btn.appendTo($popup_title_container);
    $close_btn.unbind("click").click(function(e){
      e = e || event;
      e.stopPropagation();
            close_popup();
    });

    var $popup_content_container = $("<div>").addClass("popup_content_container");
    if(isSupport("rog")){
    	$popup_content_container.addClass("rog");
    }
    else if(isSupport("tuf")){
    	$popup_content_container.addClass("tuf");
    }
    $popup_content_container.appendTo($container);

    var $feature_desc_cntr = $("<div>").addClass("feature_desc_container").appendTo($popup_content_container);

    var $content_title = $("<div>").addClass("title").html("FAQ");
    if(isSupport("rog")){
    	$content_title.addClass("rog");
    }
    else if(isSupport("tuf")){
    	$content_title.addClass("tuf");
    }
    $content_title.appendTo($feature_desc_cntr);

    $("<div>").addClass("desc").html(faq_text).appendTo($feature_desc_cntr);

    return $container;
  }
}