var asusGameProfile = []

setTimeout(function(){
    $.ajax({
        url: "https://nw-dlcdnet.asus.com/plugin/js/opennat_pf.json",
        dataType: 'jsonp',
        timeout: 2000,
    })    
}, 1500)

function updatePfList(pfList){
    asusGameProfile = pfList;
    
    $("#new_profile_name")
        .attr({"placeholder": "Search for games"})
        .keyup(function(){
            var searchResult = [];
            $(".listContainer").remove()

            if(this.value.length < 3) return false;

            for(var i=0; i<asusGameProfile.length; i++){
                if(asusGameProfile[i].title.toUpperCase().indexOf(this.value.toUpperCase()) != -1) searchResult.push(i) 
            }
        
            if(searchResult.length == 0) return false;
        
            var listContainer = $("<div>")
                .addClass("listContainer")
                .css({
                    "margin-left": "186px",
                    "margin-top": "310px",
                    "position": "absolute",
                    "z-index": "9999",
                    "max-width": "450px",
                    "min-width": "250px",
                    "height": "250px",
                    "padding": "10px",
                    "background-color": "#70000E",
                    "color": "#FFF",
                    "overflow-y": "scroll"
                })
                
            for(var j=0; j<searchResult.length; j++){
                $("<div>")
                    .hover(function(){
                        $(this)
                            .css({"background": "#C20A21", "cursor": "pointer"})
                            .addClass("selectSearchItem")
                    }, function(){
                        $(this)
                            .css({"background": "#70000E"})
                            .removeClass("selectSearchItem")
                    })
                    .css({
                        "height": "20px",
                        "line-height": "20px"
                    })
                    .html(asusGameProfile[searchResult[j]].title)
                    .attr({"id": "searchResult_" + searchResult[j]})
                    .click(function(e){
                        var resultIdx = e.target.id.replace("searchResult_", "");
                        asusGameProfile[resultIdx].id = e.target.id;
                        gameProfile.profile.push(asusGameProfile[resultIdx]);
                        $('.game-selected').attr({"id": e.target.id})
        
                        quickAddRule(e.target.id);
                        $(".listContainer").remove();
                    })
                    .appendTo(listContainer)
            }
        
            $(this).after(listContainer)
        })
        .after(
            $("<img>")
                .attr({"src": "/images/button-close.gif"})
                .click(function(){
                    $(".game-p-default")
                        .parent()
                        .attr({"id": "default_game"})
                        .click()

                    $("#new_profile_name").val("");
                    $(".listContainer").remove();
                })
        )
        .blur(function(e){
            if($(".selectSearchItem").length == 0) $(".listContainer").remove()
        })
}