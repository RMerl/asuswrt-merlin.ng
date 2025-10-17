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

function LoadingTime(seconds, flag){
	showtext(document.getElementById("proceeding_main_txt"), "<#Main_alert_proceeding_desc1#>...");
	document.getElementById("Loading").style.visibility = "visible";
	
	y = y+progress;
	if(typeof(seconds) == "number" && seconds >= 0){
		if(seconds != 0){
			/* trigger IE8 counter */
			if(navigator.appName.indexOf("Microsoft") >= 0){
				var childsel=document.createElement("span");
				document.body.appendChild(childsel);
   			document.body.removeChild(document.body.lastChild);
			}

			showtext(document.getElementById("proceeding_main_txt"), "<#Main_alert_proceeding_desc4#>");
			showtext(document.getElementById("proceeding_txt"), '<span style="color:#FFFFCC;">' + Math.round(y)+"% </span><#Main_alert_proceeding_desc1#>");
			--seconds;
			setTimeout("LoadingTime("+seconds+", '"+flag+"');", 1000);
		}
		else{
			showtext(document.getElementById("proceeding_main_txt"), translate("<#Main_alert_proceeding_desc3#>"));
			showtext(document.getElementById("proceeding_txt"), "");
			y = 0;
			
			if(flag != "waiting")
				setTimeout("hideLoading();",1000);			
		}
	}
}

function LoadingProgress(seconds){
	document.getElementById("LoadingBar").style.visibility = "visible";
	
	y = y + progress;
	if(typeof(seconds) == "number" && seconds >= 0){
		if(seconds != 0){
			document.getElementById("proceeding_img").style.width = Math.round(y) + "%";
			document.getElementById("proceeding_img_text").innerHTML = Math.round(y) + "%";
	
			if(document.getElementById("loading_block1")){
				document.getElementById("proceeding_img_text").style.width = document.getElementById("loading_block1").clientWidth;
				document.getElementById("proceeding_img_text").style.marginLeft = "175px";
			}	
			--seconds;
			setTimeout("LoadingProgress("+seconds+");", 1000);
		}
		else{
			document.getElementById("proceeding_img_text").innerHTML = "<#Main_alert_proceeding_desc3#>";
			y = 0;
			if(location.pathname.indexOf("Advanced_MobileBroadband_Content") > 0){
				setTimeout("hideLoadingBar();",1000);
				htmlbodyforIE = document.getElementsByTagName("html");
				htmlbodyforIE[0].style.overflow = "";
			}
			else if(location.pathname.indexOf("QIS_wizard.htm") < 0 && location.pathname.indexOf("Advanced_FirmwareUpgrade_Content") < 0 && location.pathname.indexOf("Advanced_SettingBackup_Content") < 0){
				setTimeout("hideLoadingBar();",1000);
				location.href = "/";
			}
		}
	}
}

class AsusLoaderComponent {

	constructor(props) {
        this.id = props.id;
		const type = (typeof props.type=='undefined') ? 'spinner' : props.type;
        let seconds = (typeof props.seconds == 'undefined') ? 0 : props.seconds;
        this.seconds = seconds;
		this.settingSecond = seconds;

        const div = document.createElement('div');
        div.id = this.id;
        const shadowRoot = div.attachShadow({mode: 'open'});
        const template = document.createElement('template');
		const style = `<style>
			.popup_bg {
				position: fixed;
				top: 0;
				right: 0;
				bottom: 0;
				left: 0;
				z-index: 2000;
				backdrop-filter: blur(3px);
				-webkit-backdrop-filter: blur(3px);
				background-color: rgba(255, 255, 255, 0.6);
			}
		
			.modal {
				position: fixed;
				top: 0;
				left: 0;
				z-index: 1060;
				display: block;
				width: 100%;
				height: 100%;
				overflow-x: hidden;
				overflow-y: auto;
				outline: 0;
			}
		
			.modal-dialog {
				position: relative;
				width: auto;
				margin: 0.5rem;
				pointer-events: none;
			}
		
			.modal-content {
				position: relative;
				display: -webkit-box;
				display: -ms-flexbox;
				display: flex;
				-webkit-box-orient: vertical;
				-webkit-box-direction: normal;
				-ms-flex-direction: column;
				flex-direction: column;
				width: 100%;
				pointer-events: auto;
				background-color: #fff;
				background-clip: padding-box;
				border: 1px solid rgba(0, 0, 0, .2);
				border-radius: 0.3rem;
				outline: 0;
				padding: 5px;
			}
		
			.mt-0 {
				margin-top: 0 !important;
			}
			
			.mt-1 {
				margin-top: .25rem!important;
			}
			
			.mt-2 {
				margin-top: .5rem!important;
			}
		
			.d-flex {
				display: flex;
			}
		
			.flex-column {
				-webkit-box-orient: vertical !important;
				-webkit-box-direction: normal !important;
				-ms-flex-direction: column !important;
				flex-direction: column !important;
			}
		
			.flex-row {
				-webkit-box-orient: horizontal !important;
				-webkit-box-direction: normal !important;
				-ms-flex-direction: row !important;
				flex-direction: row !important;
			}
		
			.gap-1 {
				gap: 1em;
			}
		
			.gap-2 {
				gap: 2em;
			}
		
			.gap-3 {
				gap: 3em;
			}
		
			.align-items-center {
				-webkit-box-align: center !important;
				-ms-flex-align: center !important;
				align-items: center !important;
			}
			
			.align-items-start {
				-webkit-box-align: start !important;
				-ms-flex-align: start !important;
				align-items: flex-start !important;
			}
		
			.justify-content-center {
				-webkit-box-pack: center !important;
				-ms-flex-pack: center !important;
				justify-content: center !important;
			}
			
			.justify-content-start {
				-webkit-box-pack: start !important;
				-ms-flex-pack: start !important;
				justify-content: flex-start !important;
			}
		
			.text-left {
				text-align: left !important;
			}
		
			.notice_title {
				display: flex !important;
				justify-content: center !important;
				font-size: 1.5em;
				color: #0A5EDB;
			}
				
			.text-white {
				color: #fff !important;
			}
		
			.gg-spinner {
				transform: scale(3)
			}
		
			.gg-spinner,
			.gg-spinner::after,
			.gg-spinner::before {
				box-sizing: border-box;
				position: relative;
				display: block;
				width: 20px;
				height: 20px
			}
		
			.gg-spinner::after,
			.gg-spinner::before {
				content: "";
				position: absolute;
				border-radius: 100px
			}
		
			.gg-spinner::before {
				animation: spinner 1s cubic-bezier(.6, 0, .4, 1) infinite;
				border: 3px solid transparent;
				border-top-color: #007bff
			}
		
			.gg-spinner::after {
				border: 3px solid;
				opacity: .2
			}
		
			@keyframes spinner {
				0% {
					transform: rotate(0deg)
				}
				to {
					transform: rotate(359deg)
				}
			}
			
			small {
				margin: 5px;
				font-size: 0.5em;
			}
			
			.text-primary {
				fill: #007bff;
			}
		
			.text-primary:hover {
				fill: #0069d9;
			}
		
			@media (min-width: 576px) {
				.modal-dialog {
					max-width: 500px;
					margin: 5rem auto;
				}
			}  
      </style>`;

		if(type == 'spinner') {
			template.innerHTML = `
            ${style}
            <div id="loader" class="popup_bg d-flex flex-column align-items-center justify-content-center gap-2">
                <i class="gg-spinner"></i>
                <div id="proceeding_txt"><#Main_alert_proceeding_desc1#>...</div>        
            </div>
        `;
		}else{
			template.innerHTML = `
			${style}
			<div id="loader" class="popup_bg d-flex flex-column align-items-center justify-content-center gap-2">
				<div id="loading_block1" class="modal-dialog modal-content d-flex flex-column align-items-center justify-content-center gap-2">
					<div id="proceeding_main_txt" class="notice_title">Please wait...</div>
					<div id="proceeding_txt" class="text-white"></div>
					<div id="proceeding_img" class="progress progress-striped active">
						<div id="proceeding_img_text" class="progress-bar progress-bar-info" style="width: 0%"></div>
					</div>
				</div>
			</div>`;
		}
        shadowRoot.appendChild(template.content.cloneNode(true));
		this.shadowRoot = shadowRoot;
        this.element = div;
    }

	isShow() {
		return top.document.querySelector(`#${this.id}`) != null;
	}

	setSeconds(seconds) {
		this.seconds = seconds;
		this.settingSecond = seconds;
	}

	render() {
        return this.element;
    }

	show() {
		if (top.document.querySelector(`#${this.id}`) == null) {
			top.document.body.appendChild(this.element);
		}
		if (typeof (this.seconds) == "number" && this.seconds >= 0) {
			const proceeding_txt = this.shadowRoot.getElementById('proceeding_txt');
			if (this.seconds != 0) {
				const loop = setInterval(() => {
					this.seconds--;
					const leftSeconds = Math.round((this.settingSecond - this.seconds) / this.settingSecond * 100);
					proceeding_txt.textContent = `<#Main_alert_proceeding_desc4#> ${leftSeconds}% <#Main_alert_proceeding_desc1#>`;
					if (this.seconds == 0) {
						setTimeout(() => {
							proceeding_txt.textContent = `<#Main_alert_proceeding_desc3#>`;
							setTimeout(() => {
								this.hide();
							}, 1000);
						}, 1000);
						clearInterval(loop);
					}
				}, 1000);
			}
		}
	}

    hide() {
		top.document.body.style.removeProperty('overflow');
		this.setSeconds(0);
		const proceeding_txt = this.shadowRoot.getElementById('proceeding_txt');
		proceeding_txt.textContent = `<#Main_alert_proceeding_desc1#>...`;

		this.element.remove();
		const loader = top.document.querySelector(`#${this.id}`);
		if(loader){
			loader.remove();
		}
		top.loader = null;
    }
}

let businessLoader = null;
if (isSupport("UI4") && businessLoader == null) {
	businessLoader = new AsusLoaderComponent({
		id: 'loader',
		type: 'spinner',
		seconds: null
	});
	top.loader = businessLoader;
}

function showLoading(seconds, flag) {
	if (isSupport("UI4") && businessLoader) {
		businessLoader.setSeconds(seconds);
		businessLoader.show();
	} else {
		if (window.scrollTo) {
			window.scrollTo({
				top: 0,
				left: 0,
				behavior: 'smooth'
			});
		}

		if (parent.window.scrollTo) {
			parent.window.scrollTo({
				top: 0,
				left: 0,
				behavior: 'smooth'
			});
		}

		disableCheckChangedStatus();

		htmlbodyforIE = document.getElementsByTagName("html");  //this both for IE&FF, use "html" but not "body" because <!DOCTYPE html PUBLIC.......>
		htmlbodyforIE[0].style.overflow = "hidden";	  //hidden the Y-scrollbar for preventing from user scroll it.

		winW_H();

		var blockmarginTop;
		var blockmarginLeft;
		if (window.innerWidth)
			winWidth = window.innerWidth;
		else if ((document.body) && (document.body.clientWidth))
			winWidth = document.body.clientWidth;

		if (window.innerHeight)
			winHeight = window.innerHeight;
		else if ((document.body) && (document.body.clientHeight))
			winHeight = document.body.clientHeight;

		if (document.documentElement && document.documentElement.clientHeight && document.documentElement.clientWidth) {
			winHeight = document.documentElement.clientHeight;
			winWidth = document.documentElement.clientWidth;
		}

		if (winWidth > 1050) {

			winPadding = (winWidth - 1050) / 2;
			winWidth = 1105;
			blockmarginLeft = (winWidth * 0.35) + winPadding;
		} else if (winWidth <= 1050) {
			blockmarginLeft = (winWidth) * 0.35 + document.body.scrollLeft;

		}

		if (winHeight > 660)
			winHeight = 660;

		blockmarginTop = winHeight * 0.3

		document.getElementById("loadingBlock").style.marginTop = blockmarginTop + "px";
		if (re_mode == "1") {
			document.getElementById("loadingBlock").style.left = "50%";
			document.getElementById("loadingBlock").style.marginLeft = "-200px";
		} else
			document.getElementById("loadingBlock").style.marginLeft = blockmarginLeft + "px";

		document.getElementById("Loading").style.width = winW + "px";
		document.getElementById("Loading").style.height = winH + "px";

		loadingSeconds = seconds;
		progress = 100 / loadingSeconds;
		y = 0;

		LoadingTime(seconds, flag);
	}
}

function showLoadingBar(seconds){
	if(window.scrollTo)
		window.scrollTo(0,0);

	disableCheckChangedStatus();
	
	htmlbodyforIE = document.getElementsByTagName("html");  //this both for IE&FF, use "html" but not "body" because <!DOCTYPE html PUBLIC.......>
	htmlbodyforIE[0].style.overflow = "hidden";	  //hidden the Y-scrollbar for preventing from user scroll it.
	
	winW_H();

	var blockmarginTop;
	var blockmarginLeft;
	if (window.innerWidth)
		winWidth = window.innerWidth;
	else if ((document.body) && (document.body.clientWidth))
		winWidth = document.body.clientWidth;
	
	if (window.innerHeight)
		winHeight = window.innerHeight;
	else if ((document.body) && (document.body.clientHeight))
		winHeight = document.body.clientHeight;

	if (document.documentElement  && document.documentElement.clientHeight && document.documentElement.clientWidth){
		winHeight = document.documentElement.clientHeight;
		winWidth = document.documentElement.clientWidth;
	}

	if(winWidth >1050){

		winPadding = (winWidth-1050)/2;	
		winWidth = 1105;
		blockmarginLeft= (winWidth*0.3)+winPadding;
	}
	else if(winWidth <=1050){
		blockmarginLeft= (winWidth)*0.3+document.body.scrollLeft;	

	}
	
	if(winHeight >660)
		winHeight = 660;
	
	blockmarginTop= winHeight*0.3			

	document.getElementById("loadingBarBlock").style.marginTop = blockmarginTop+"px";
	// marked by Jerry 2012.11.14 using CSS to decide the margin
	document.getElementById("loadingBarBlock").style.marginLeft = blockmarginLeft+"px";


	/*blockmarginTop = document.documentElement.scrollTop + 200;
	document.getElementById("loadingBarBlock").style.marginTop = blockmarginTop+"px";*/

	document.getElementById("LoadingBar").style.width = winW+"px";
	document.getElementById("LoadingBar").style.height = winH+"px";
	
	loadingSeconds = seconds;
	progress = 100/loadingSeconds;
	y = 0;
	LoadingProgress(seconds);
}

function hideLoadingBar(){
	document.getElementById("LoadingBar").style.visibility = "hidden";
}

function hideLoading(flag){
	if (isSupport("UI4") && businessLoader) {
		businessLoader.hide();
	}
	else {
		document.getElementById("Loading").style.visibility = "hidden";
		htmlbodyforIE = document.getElementsByTagName("html");  //this both for IE&FF, use "html" but not "body" because <!DOCTYPE html PUBLIC.......>
		htmlbodyforIE[0].style.overflow = "";	  //hidden the Y-scrollbar for preventing from user scroll it.
		document.getElementById("Loading").style.width = "initial";
		document.getElementById("Loading").style.height = "initial";
	}
}

function dr_advise(){
	disableCheckChangedStatus();
	
	htmlbodyforIE = document.getElementsByTagName("html");  //this both for IE&FF, use "html" but not "body" because <!DOCTYPE html PUBLIC.......>
	htmlbodyforIE[0].style.overflow = "hidden";	  //hidden the Y-scrollbar for preventing from user scroll it.
	
	winW_H();
	var blockmarginTop;
	blockmarginTop = document.documentElement.scrollTop + 200;	
	document.getElementById("dr_sweet_advise").style.marginTop = blockmarginTop+"px"
	document.getElementById("hiddenMask").style.width = winW+"px";
	document.getElementById("hiddenMask").style.height = winH+"px";	
	document.getElementById("hiddenMask").style.visibility = "visible";
}

function cancel_dr_advise(){	
	parent.document.getElementById("hiddenMask").style.visibility = "hidden";
	htmlbodyforIE = parent.document.getElementsByTagName("html");  //this both for IE&FF, use "html" but not "body" because <!DOCTYPE html PUBLIC.......>
	htmlbodyforIE[0].style.overflow = "scroll";	  //hidden the Y-scrollbar for preventing from user scroll it.	
	window.scrollTo(0, 0);	//x-axis , y-axis	
}
