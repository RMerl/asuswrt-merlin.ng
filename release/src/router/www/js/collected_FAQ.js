var popup_hint_api = {
  "get_component_title": function(){
    return $("<div>").addClass("hint_title");
  },
  "get_component_text": function(){
    return $("<div>").addClass("hint_text");
  },
  "get_component_action": function(){
    var $action_bg = $("<div>").addClass("action_bg");
    var $action_canacl = $("<input/>").attr({"id":"action_canacl", "type":"button", "value":"<#CTL_Cancel#>"}).addClass("button_gen");
    var $action_ok = $("<input/>").attr({"id":"action_ok", "type":"button", "value":"<#CTL_ok#>"}).addClass("button_gen");
    $action_bg.append($action_canacl).append($action_ok);
    return $action_bg;
  },
  "get_component_loading": function(){
    var loadContainer = $("<div>").addClass("cssload-bell");
    for(var i=0; i<5; i++){
      $("<div>")
        .addClass("cssload-circle")
        .append($("<div>").addClass("cssload-inner"))
        .appendTo(loadContainer);
    }
    return $("<div>").addClass("cssload-bellContainer").append(loadContainer);
  },
  "get_component_mask": function(){
    return $("<div>").addClass("mask_bg");
  }
};

function popout_faq(_text, _text2){

  var text_tmp = (_text)? _text : `<#HOWTOSETUP#>`;
  var text_tmp2 = (_text2)? _text2 : "";
  $(".faq_info_icon").unbind("click");
  $(".faq_info_icon").click(function(e){
    // Viz tmp closeAllSelect();
    e = e || event;
    e.stopPropagation();
    var $faq_info_bg = $("<div>");
    $faq_info_bg.addClass("faq_info_bg");
      
    var $faq = $("<div>");
    $faq.addClass("faq_bg");
    var faq_text = "<div class='setup_info'>FAQ</div><br>";
    faq_text += "<a id='link_info_faq' href='' target='_blank' class='faq_link'>";
    faq_text += text_tmp;
    faq_text += "</a>";

    if(text_tmp2){
      faq_text += "<br>";
      faq_text += "<a id='link_info_faq2' href='' target='_blank' class='faq_link'>";
      faq_text += text_tmp2;
      faq_text += "</a>";      
    }

    $faq.html(faq_text);
    $faq_info_bg.append($faq);
    $("#faq_hint_msg").empty().append(popup_hint_api.get_component_title()).append(popup_hint_api.get_component_text()).append(popup_hint_api.get_component_action());
    $("#faq_hint_msg .hint_title").html(`<#NewFeatureAbout#>`);
    $("#faq_hint_msg .hint_text").html($faq_info_bg).promise().done(function(){
      var href_index = get_faq_index(FAQ_List, current_page, 1);
      $("#faq_hint_msg .hint_text #link_info_faq").attr("href", faq_href+href_index);

      if(text_tmp2){
        var href_index2 = get_faq_index(FAQ_List, current_page, 2);
         $("#faq_hint_msg .hint_text #link_info_faq2").attr("href", faq_href+href_index2);
      }

    });
    $("#faq_hint_msg #action_canacl").hide();
    $("#faq_hint_msg #action_ok").show().unbind("click").click(function(e){
      e = e || event;
      e.stopPropagation();
      $("#faq_hint_msg").hide();
    });
    $("#faq_hint_msg").show();
    adjust_panel_block_top("faq_hint_msg", 100);
  });

}
