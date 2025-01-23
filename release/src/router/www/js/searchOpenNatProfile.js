var asusGameProfile = []

fetch("https://nw-dlcdnet.asus.com/plugin/js/opennat_pf.json")
	.then(response => response.text())
	.then(data => {
		const jsonData = JSON.parse(data.replace(/^updatePfList\((.*)\)$/g, '$1'));
		updatePfList(jsonData);
	})

function updatePfList(pfList){
	asusGameProfile = pfList;
	const style_mapping = {
		"RT":{"bg":"#576D73", "hover":"#3366FF", "text":"#FFF"},
		"ROG":{"bg":"rgb(62,3,13)", "hover":"#3366FF", "text":"#FFF"},
		"TUF":{"bg":"rgb(55,55,55)", "hover":"#3366FF", "text":"#FFF"},
		"WHITE":{"bg":"#f5f5f5", "hover":"#ffffff", "text":"#248dff"},
	};
	const getTheme = ()=>{
		let ui_support = httpApi.hookGet("get_ui_support");

		function isSupport(_ptn) {
			return ui_support[_ptn] ? true : false;
		}
		let theme = 'RT';
		if (isSupport("rog")) {
			return "ROG";
		} else if (isSupport("tuf")) {
			return "TUF";
		} else if (isSupport("BUSINESS")) {
			return "WHITE";
		} else {
			return theme;
		}
	};
	const specific_style = style_mapping[getTheme()] || getTheme("RT");

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
					"background-color": specific_style.bg,
					"color": specific_style.text,
					"overflow-y": "scroll"
				})

				for(var j=0; j<searchResult.length; j++){
					$("<div>")
						.hover(function(){
							$(this)
								.css({"background": specific_style.hover, "cursor": "pointer"})
								.addClass("selectSearchItem")
						}, function(){
							$(this)
								.css({"background": specific_style.bg})
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