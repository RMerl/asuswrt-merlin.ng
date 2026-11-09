function confirm_asus(content){
	/* default config:
		content = {
			title: "Logout Confirm",
			ribbon: "beta css1",
			ribbon_wrapper: "beta css2 ",
			contentA: "#JS_logout#",
			contentB: "#sub_title#",
			contentC: "#note to user before click button#"
			left_button: "Cancel",
			left_button_callback: function(){},
			left_button_args: {},
			right_button: "Yes",
			right_button_callback: function(){},
			right_button_args: {},
			note: "",
			iframe: "",
			margin: "",
			note_display_flag: 0:stable only, 1:beta only
		}
	*/
	var code = "";
	var release_note_code = "";
	code += '<div class="confirm_block"';
	code += 'style="position:absolute;margin:'+content.margin+';"';
	code += '>';
	code += '<div class="confirm">';
	code += '<div class="confirm_title">' + content.title + '</div>';
	if( content.ribbon != undefined )
		code += '<div class="'+content.ribbon_wrapper+'"><div class="'+content.ribbon+'">BETA</div></div>';
	code += '<div class="confirm_contentA">' + content.contentA +'</div>';
	
	if(content.iframe.search("get_release_note") >= 0){
		var webs_state_info_version = webs_state_info.slice(5).splice(3,0,".");
		var webs_state_info_version_beta = webs_state_info_beta.slice(5).splice(3,0,".");
		if(content.note_display_flag==0){
			release_note_code += '<div style="display:table;width:93%" class="confirm_contentB">';
			release_note_code += '<iframe id="status_iframe" src="'+ content.iframe +'" width="100%" marginwidth="0" marginheight="0" scrolling="no" align="center"></iframe>';
			release_note_code += '</div>';
		}
		else if(content.note_display_flag==1){
			release_note_code += '<div style="display:table;width:93%" class="confirm_contentB">';
			release_note_code += '<iframe id="status_iframe" src="'+ content.iframe +'" width="100%" marginwidth="0" marginheight="0" scrolling="no" align="center"></iframe>';
			release_note_code += '</div>';
		}
		code += release_note_code;
		code += '<div class="confirm_contentC">' + content.contentC +'</div>';
	}
	if(content.iframe.search("AFC") >= 0){
		code += '<div class="confirm_contentD">' + content.contentD +'</div>';
	}
	
	code += '<div style="display:flex;width:100%;margin-top:10px;justify-content:center;align-items:center;" class="business_confirm_container">';
	if(content.left_button && content.left_button != "Hidden"){
		if(rog_support){
			code += '<div id="confirm_button_gen_long_left" class="confirm_button_gen_long_left_rog">' + content.left_button + '</div>';	//confirm_button confirm_button_left
		}
		else if(tuf_support){
			code += '<div id="confirm_button_gen_long_left" class="confirm_button_gen_long_left_tuf">' + content.left_button + '</div>';	//confirm_button confirm_button_left	
		}
		else{
			code += '<div id="confirm_button_gen_long_left" class="confirm_button_gen_long_left">' + content.left_button + '</div>';	//confirm_button confirm_button_left
		}
	}
	if(content.right_button){
		if(rog_support){
			code += '<div id="confirm_button_gen_long_right" class="confirm_button_gen_long_right_rog">' + content.right_button + '</div>';	//confirm_button confirm_button_right
		}
		else if(tuf_support){
			code += '<div id="confirm_button_gen_long_right" class="confirm_button_gen_long_right_tuf">' + content.right_button + '</div>';	//confirm_button confirm_button_right
		}
		else{
			code += '<div id="confirm_button_gen_long_right" class="confirm_button_gen_long_right">' + content.right_button + '</div>';	//confirm_button confirm_button_right
		}
	}
	code += '</div>';		
	code += '</div></div>';
	
	$('body').append(code);
	$('#confirm_button_gen_long_left').bind("click", function(){return content.left_button_callback.call(content.left_button_args)});
	$('#confirm_button_gen_long_right').bind("click", function(){return content.right_button_callback.call(content.right_button_args)});
	try { fix_confirm_block_in_shell(); } catch(e) {}
}

// When a legacy page is embedded in the settings.html shell it lives in a
// FULL-HEIGHT iframe (the shell scrolls, not the iframe) and the shell injects
// business-white.css. That combination breaks .confirm_block three ways:
//   1. its position:absolute + the caller's scrollTop-based margin anchor it near
//      the top of the tall document instead of the visible area (position:fixed
//      can't help — the iframe viewport IS the whole document);
//   2. business-white forces .business_confirm_container{margin-left:25% !important}
//      which shoves the centred buttons off the right edge;
//   3. business-white hardcodes the dialog to white bg / black text (!important,
//      not gated by colour scheme), so dark theme shows a white box with black text.
// Fix all three here — only inside the shell iframe, so standalone / the main app
// keep their original behaviour.
function fix_confirm_block_in_shell(){
	if(window.frameElement == null) return;               // not embedded → leave as-is
	var block = document.querySelector('.confirm_block');
	if(!block) return;

	// Backdrop behind the dialog; auto-removed whenever the dialog goes away.
	if(!document.getElementById('confirm_shell_backdrop')){
		var bd = document.createElement('div');
		bd.id = 'confirm_shell_backdrop';
		bd.style.cssText = 'position:fixed;inset:0;z-index:9999;' +
			'background:var(--wrt-modal-backdrop-bg, rgba(0,0,0,0.5));';
		document.body.appendChild(bd);
		var mo = new MutationObserver(function(){
			if(!document.querySelector('.confirm_block')){ remove_confirm_shell_backdrop(); mo.disconnect(); }
		});
		mo.observe(document.body, { childList: true });
	}

	// Centre in the region of the iframe that is actually on-screen.
	var center;
	try{
		var feRect = window.frameElement.getBoundingClientRect();  // iframe box in shell viewport
		var parentH = window.parent.innerHeight;
		var visTop = Math.max(0, -feRect.top);
		var visBottom = Math.min(window.frameElement.clientHeight, parentH - feRect.top);
		center = (visTop + visBottom) / 2;
	}catch(e){
		center = (document.documentElement.scrollTop || 0) + window.innerHeight / 2;
	}
	block.style.position = 'absolute';
	block.style.top = Math.round(center) + 'px';
	block.style.left = '50%';
	block.style.right = 'auto';
	block.style.width = '90%';
	block.style.maxWidth = '600px';
	block.style.margin = '0';
	block.style.transform = 'translate(-50%, -50%)';
	block.style.zIndex = '10000';

	// Undo business-white's 25% left margin so the buttons stay inside the box.
	var btnRow = block.querySelector('.business_confirm_container');
	if(btnRow){
		btnRow.style.setProperty('margin-left', '0', 'important');
		btnRow.style.setProperty('margin-right', '0', 'important');
		btnRow.style.setProperty('flex-wrap', 'wrap', 'important');
	}

	// Re-apply theme tokens for colour (inline !important beats the stylesheet),
	// so both light and dark render correctly.
	var confirmEl = block.querySelector('.confirm');
	if(confirmEl){
		confirmEl.style.setProperty('background-color', 'var(--surface-card, #FFFFFF)', 'important');
		confirmEl.style.setProperty('color', 'var(--text-primary, #000000)', 'important');
		confirmEl.style.setProperty('border-color', 'var(--wrt-card-border, var(--border-subtle, #CCCCCC))', 'important');
	}
	['.confirm_title', '.confirm_contentA', '.confirm_contentB', '.confirm_contentD'].forEach(function(sel){
		var el = block.querySelector(sel);
		if(el) el.style.setProperty('color', 'var(--text-primary, #000000)', 'important');
	});
}

function remove_confirm_shell_backdrop(){
	var bd = document.getElementById('confirm_shell_backdrop');
	if(bd) bd.remove();
}

function confirm_cancel(){
	$(".confirm_block").remove();
	remove_confirm_shell_backdrop();
}
