var pppIspList = [];

window.updateIspList = function(ispProfile){
    var ispProfileArray = ispProfile.ispList.split(",");
    var arrayMerge = pppIspList.concat(ispProfileArray.filter(
        function(item){return (pppIspList.indexOf(item) < 0)}
    ))

    pppIspList = arrayMerge;

    $('input[name="wan_pppoe_username"]')
        .unbind("keyup")
        .blur(function(){if($(".selectSearchItem").length == 0) $(".listContainer").remove()})
        .keyup(function(e){
            $(".listContainer").remove()

            if(this.value.indexOf("@") != -1){
                var ispName = this.value.split("@")[1];	
                if(ispName.length < 2) return false;

                var searchResult = [];	
                searchResult = pppIspList.filter(function(item){
                    return (item.indexOf(ispName) != -1)
                })

                if(searchResult == 0) return false;

                var listContainer = $("<div>")
                    .addClass("listContainer")
                    .css({
                        "margin-left": "2px",
                        "position": "absolute",
                        "z-index": "9999",
                        "width":  $('input[name="wan_pppoe_username"]').width() + "px",
                        "padding": "10px",
                        "background-color": "#303030",
                        "color": "#FFF",
                        "overflow-y": "auto",
                        "border": "1px solid #DBDBDB",
                        "max-height": "490px"
                    })

                for(var j=0; j<searchResult.length; j++){
                    $("<div>")
                        .hover(function(){
                            $(this)
                                .css({"background": "#007AFF", "cursor": "pointer"})
                                .addClass("selectSearchItem")
                        }, function(){
                            $(this)
                                .css({"background": "#303030"})
                                .removeClass("selectSearchItem")
                        })
                        .css({
                            "font-family": "Lucida Console"
                        })
                        .html(searchResult[j])
                        .attr({"id": searchResult[j].replace(".", "_")})
                        .click(function(e){
                            var ispName = e.target.id.replace("_", ".");
                            var curValue = $('input[name="wan_pppoe_username"]').val()
                            $('input[name="wan_pppoe_username"]').val(curValue.split("@")[0] + "@" + ispName)
                            $(".listContainer").remove();
                        })
                        .appendTo(listContainer)
                }
            
                $(this).after(listContainer)
            }
        });
}

$.getJSON("ajax/pppIspList_V2.json").always(function(local_data, status){
    if(status == "success"){
        updateIspList(local_data);
    }
    $.getJSON("https://nw-dlcdnet.asus.com/plugin/js/pppIspList_V2.json", function(cloud_data){
        updateIspList(cloud_data);
    });
});