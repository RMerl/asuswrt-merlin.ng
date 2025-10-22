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

		this.liveupdateDone = false;
		this.liveupdateFailed = false;
		
		this.rescueStart = false;
		this.rescueFwDownload = false;
		this.rescueFwInstall = false;
		this.rescueAPPInstall = false;
		this.rescueDone = false;
		this.rescueFailed = false;

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

      const detailStyle = `<style>

      	#loader_title {
      		font-size:18px;
      	}

		#reset-progress-module { font-family: "Segoe UI", sans-serif; padding: 20px; background: #fff; border: 1px solid #ccc; border-radius: 8px; max-width: 600px; margin: auto; }
		      .stage-bar { width: 60%; display: flex; justify-content: space-between; margin-bottom: 10px; }
		      .stage-bar span { position: relative; flex: 1; text-align: center; font-size: 18px; }
		      .stage-bar span::before {
		        content: "◎";
		        position: absolute;
		        top: -20px;
		        left: 50%;
		        transform: translateX(-50%);
		        font-size: 14px;
		      }
			  .stage-bar span.active::before {
        		content: "◉";
        		color: rgb(0, 123, 255);
      		  }
      		  .stage-bar span.active {
        		color: rgb(0, 123, 255);
      		  }

		      .progress-info { margin: 15px 0; font-size: 16px; }
		      .progress-bar {
		        width: 50%;
		        background: #ddd;
		        border-radius: 5px;
		        overflow: hidden;
		        height: 20px;
		      }
		      .progress-fill {
		        height: 100%;
		        background: rgb(0, 123, 255);
		        width: 0%;
		        transition: width 0.5s;
		      }
		      .checkpoints { margin-top: 20px; padding-left: 20px; }
		      .checkpoints li { margin: 5px 0; font-size: 14px; }
		      .checkpoints li.done::before { content: "✓ "; color: green; }
		      .checkpoints li.current::before { content: "► "; color: orange; }
		      .notice { margin-top: 20px; color: red; font-size: 18px; font-weight: bold; }


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
		}
		else if(type == 'detail'){
        	template.innerHTML = `
            ${detailStyle}
            <div id="loader" class="popup_bg d-flex flex-column align-items-center justify-content-center gap-2">

				<h3 id="loader_title">AI Board Live Update Progress</h3>
			    <div class="stage-bar">
			      <span id="step_start">Downloading</span>
			      <span id="step_docker">Installing</span>
			      <span id="step_done">Done & Reboot AI Board</span>
			    </div>

			    <div class="progress-info" id="proceeding_step">Current step：-</div>
			    <div class="progress-bar">
			      <div class="progress-fill" id="progressFill"></div>
			    </div>
			    <div class="progress-info" id="percentInfo">0%</div>

			    <ul class="checkpoints" id="checkpointList"></ul>
			    <div class="notice" id="notice">Note: The system is working normally. Please do not interrupt the operation.</div>


		    </div>
        `;
		}
		else if(type == 'rescueDetail'){
        	template.innerHTML = `
            ${detailStyle}
            <div id="loader" class="popup_bg d-flex flex-column align-items-center justify-content-center gap-2">

				<h3 id="loader_title">AI Board Rescue Progress</h3>
			    <div class="stage-bar">
			      <span id="step_start">Prepare for rescue</span>
			      <span id="step_upload_fw">Uplaod Firmware</span>
			      <span id="step_install_fw">Install Firmware</span>
			      <span id="step_docker">Install APP</span>
			      <span id="step_done">Done & Reboot AI Board</span>
			    </div>

			    <div class="progress-info" id="proceeding_step">Current step：-</div>
			    <div class="progress-notice" id="proceeding_notice" style="display:none;">This process may take several minutes.</div>
			    <div class="progress-bar">
			      <div class="progress-fill" id="progressFill"></div>
			    </div>
			    <div class="progress-info" id="percentInfo">0%</div>

			    <ul class="checkpoints" id="checkpointList"></ul>
			    <div class="notice" id="notice">Note: The system is working normally. Please do not interrupt the operation.</div>


		    </div>
        `;
		}
		else{
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

    show_progress(flag) {
		if (top.document.querySelector(`#${this.id}`) == null) {
			top.document.body.appendChild(this.element);
		}
		if(flag=="rescue"){
			
			const interval = setInterval(() => {
				if (this.rescueFailed) {
					this.aiboard_query();
					setTimeout(() => { this.hide_detail(); }, 5000);
					setTimeout(() => { top.location.reload(); }, 5000);
			        clearInterval(interval);
			    }
			    if (this.rescueDone) {
			    	setTimeout(() => { this.aiboard_query(); }, 25000);
			        setTimeout(() => { this.hide_detail(); }, 30000);
			        setTimeout(() => { top.location.reload(); }, 30000);
			        clearInterval(interval);
			    }
			    this.update_rescue_status();
			}, 1000);

		}
		if(flag=="liveupdate"){
		
			const interval = setInterval(() => {
				if (this.liveupdateFailed) {
					this.aiboard_query();
					setTimeout(() => { this.hide_detail(); }, 5000);
					setTimeout(() => { top.location.reload(); }, 5000);
			        clearInterval(interval);
			    }
			    if (this.liveupdateDone) {
			    	setTimeout(() => { this.aiboard_query(); }, 25000);
			        setTimeout(() => { this.hide_detail(); }, 30000);
			        setTimeout(() => { top.location.reload(); }, 30000);
			        clearInterval(interval);
			    }
			    this.update_liveupdate_status();
			}, 1000);

		}
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
		if (top?.document?.body) {
			top.document.body.style.removeProperty('overflow');
		}
		this.setSeconds(0);
		const proceeding_txt = this.shadowRoot.getElementById('proceeding_txt');
		proceeding_txt.textContent = `<#Main_alert_proceeding_desc1#>...`;

		this.element.remove();
		const loader = top?.document?.querySelector(`#${this.id}`);
		if(loader){
			loader.remove();
		}
		if (top) {
			top.loader = null;
		}
	}

	hide_detail() {
		if (top?.document?.body) {
			top.document.body.style.removeProperty('overflow');
		}
		this.setSeconds(0);
		
		this.element.remove();
		const loader = top?.document?.querySelector(`#${this.id}`);
		if(loader){
			loader.remove();
		}
		if (top) {
			top.loader = null;
		}
	}

    aiboard_query(){
	    let initPost = {};
	    initPost.action_mode = "apply";
	    initPost.rc_service = "start_ai_request query";
		httpApi.nvramSet(initPost);
	}

    update_liveupdate_status() {
    	const self = this;
    	$.getJSON("/get_aisom_upgrade_info.cgi", function(data) {
            console.log(data);
              const percent = data.ai_prog_current_percent;
              const stepText = `${data.ai_prog_current_image} (${data.ai_prog_current_step}/${data.ai_prog_total_steps})`;

              const proceeding_step = self.shadowRoot.getElementById('proceeding_step');
              const proceeding_notice = self.shadowRoot.getElementById('proceeding_notice');
              const progressFill = self.shadowRoot.getElementById('progressFill');
              const percentInfo = self.shadowRoot.getElementById('percentInfo');
              const notice_div = self.shadowRoot.getElementById('notice');
              progressFill.style.width = percent + '%';
              percentInfo.textContent = percent + '%'


              // Stage progress
              const stages = self.shadowRoot.querySelectorAll('.stage-bar span');
              // remove whole span active class

              stages.forEach(span => span.classList.remove('active'));
              // To add active class while matched status
              if (data.ai_prog_status === "START" || 
              		data.ai_prog_status === "DOWNLOAD_FW" || data.ai_prog_status === "DOWNLOAD_FW_RSA" || data.ai_prog_status === "VERIFY_FW") 
              {
              	proceeding_step.textContent = `Current step：` + stepText;
				stages[0].classList.add('active');
				self.shadowRoot.getElementById('step_start').classList.add('active');
			  } 
			  else if (data.ai_prog_status.includes("DOWNLOAD") && data.ai_prog_current_image === "aisom.swu") {
			  	proceeding_step.textContent = `Current step：` + stepText;
			  	stages[0].classList.add('active');
			  	self.shadowRoot.getElementById('step_start').classList.add('active');
			  }
			  else if (data.ai_prog_status.includes("PROGRESS")) {
			  	proceeding_step.textContent = `Current step：` + stepText;
			  	stages[0].classList.add('active');
			  	self.shadowRoot.getElementById('step_start').classList.add('active');
				stages[1].classList.add('active');
				self.shadowRoot.getElementById('step_docker').classList.add('active');
			  }
			  else if(data.ai_prog_status.includes("failed")) {
			  	proceeding_step.textContent = `Live update failed`;
				stages[2].classList.add('active');
				progressFill.style.visibility = "hidden";
              	percentInfo.style.visibility = "hidden";
				notice_div.style.visibility = "hidden";

				self.liveupdateFailed = true;
			  }
			  else if(data.ai_prog_status.includes("DONE")) {
			  	proceeding_step.textContent = `Live update completed.`;
			  	stages[0].classList.add('active');
			  	self.shadowRoot.getElementById('step_start').classList.add('active');
				stages[1].classList.add('active');
				self.shadowRoot.getElementById('step_docker').classList.add('active');
				stages[2].classList.add('active');
				self.shadowRoot.getElementById('step_done').classList.add('active');
				progressFill.style.visibility = "hidden";
              	percentInfo.style.visibility = "hidden";
				notice_div.style.visibility = "hidden";

				self.liveupdateDone = true;
			  }


              // 檢查點顯示
              const checkpoints = [
                "準備Live Update",
                "安裝aisom.swu",
                "安裝preboot.subimg.gz",
                "安裝tzk.subimg.gz",
                "安裝bl.subimg.gz",
                "安裝boot.subimg.gz",
                "安裝rootfs.subimg.gz",
                "安裝fastlogo.subimg.gz",
                "安裝rescue.subimg.gz",
                "安裝home.tar",
                "安裝postinstall.sh"
              ];
              /*
              const currentStep = data.ai_prog_current_step;
              $('#checkpointList').empty();
              checkpoints.forEach((item, index) => {
                const li = $('<li></li>').text(item);
                if (index + 1 < currentStep) {
                  li.addClass('done');
                } else if (index + 1 === currentStep) {
                  li.addClass('current');
                }
                $('#checkpointList').append(li);
              });
              */
		});
    }

    update_rescue_status() {
    	const self = this;
    	$.getJSON("/get_aisom_upgrade_info.cgi", function(data) {
            console.log(data);
              const percent = data.ai_prog_current_percent;
              const stepText = `${data.ai_prog_current_image} (${data.ai_prog_current_step}/${data.ai_prog_total_steps})`;

              const proceeding_step = self.shadowRoot.getElementById('proceeding_step');
              const progressFill = self.shadowRoot.getElementById('progressFill');
              const percentInfo = self.shadowRoot.getElementById('percentInfo');
              const notice_div = self.shadowRoot.getElementById('notice');
              progressFill.style.width = percent + '%';
              percentInfo.textContent = percent + '%'


              // Stage progress
              
              const stages = self.shadowRoot.querySelectorAll('.stage-bar span');
              // remove whole span active class
              stages.forEach(span => span.classList.remove('active'));
              // To add active class while matched status
              if (data.ai_prog_status === "START") {

              	self.rescueStart = true;
              	proceeding_step.textContent = `Current step： Enter Rescue mode`;
              	//proceeding_notice.style.display = "";
			  } 
			  else if (data.ai_prog_status.includes("DOWNLOAD") && data.ai_prog_current_image.includes("aisom_reset")) {

			  	self.rescueStart = true;
				self.rescueFwDownload = true;
			  	proceeding_step.textContent = `Current step：` + stepText;
			  	//proceeding_notice.style.display = "none";
			  }
			  else if (data.ai_prog_status.includes("PROGRESS") && (data.ai_prog_current_image.includes("subimg.gz") || data.ai_prog_current_image.includes("install")) ) {

			  	self.rescueStart = true;
				self.rescueFwDownload = true;
				self.rescueFwInstall = true;
			  	proceeding_step.textContent = `Current step：` + stepText;
			  	//proceeding_notice.style.display = "none";
			  }
			  else if ( data.ai_prog_status === "DONE" ||
			  	(data.ai_prog_status.includes("DOWNLOAD") && data.ai_prog_current_image.includes("docker_images")) ||
			  	data.ai_prog_status.includes("SUCCESS") ||
			  	data.ai_prog_status.includes("LOAD")
			  	) {

			  	self.rescueStart = true;
				self.rescueFwDownload = true;
				self.rescueFwInstall = true;
				self.rescueAPPInstall = true;
			  	proceeding_step.textContent = `Current step：` + stepText;
			  	//proceeding_notice.style.display = "none";
			  }
			  else if(data.ai_prog_status.includes("failed")) {
			  	proceeding_step.textContent = `AI Board rescue failed`;
			  	//proceeding_notice.style.display = "none";
			  	progressFill.style.visibility = "hidden";
              	percentInfo.style.visibility = "hidden";
				notice_div.style.visibility = "hidden";

				self.rescueFailed = true;
			  }
			  else if(data.ai_prog_status.includes("DOCKER_DONE")) {
			  	proceeding_step.textContent = `AI Board rescue completed.`;
			  	//proceeding_notice.style.display = "none";
			  	progressFill.style.visibility = "hidden";
              	percentInfo.style.visibility = "hidden";
				notice_div.style.visibility = "hidden";

				self.rescueStart = true;
				self.rescueFwDownload = true;
				self.rescueFwInstall = true;
				self.rescueAPPInstall = true;
				self.rescueDone = true;
			  }

			  if (self.rescueStart) {
				stages[0].classList.add('active');
				self.shadowRoot.getElementById('step_start').classList.add('active');
			  }
			  if (self.rescueFwDownload) {
				stages[1].classList.add('active');
				self.shadowRoot.getElementById('step_upload_fw').classList.add('active');
			  }
			  if (self.rescueFwInstall) {
				stages[2].classList.add('active');
				self.shadowRoot.getElementById('step_install_fw').classList.add('active');
			  }
			  if (self.rescueAPPInstall) {
				stages[3].classList.add('active');
				self.shadowRoot.getElementById('step_docker').classList.add('active');
			  }
			  if (self.rescueDone) {
				stages[4].classList.add('active');
				self.shadowRoot.getElementById('step_done').classList.add('active');
			  }


              // 檢查點顯示
              const checkpoints = [
                "進入Rescue模式",
                "Image下載完成",
                "安裝preboot.subimg.gz",
                "安裝key.subimg.gz",
                "安裝tzk.subimg.gz",
                "安裝bl.subimg.gz",
                "安裝kernel.subimg.gz",
                "安裝rootfs.subimg.gz",
                "安裝docker.subimg.gz"
              ];

              /*
              const currentStep = p.current_step;
              $('#checkpointList').empty();
              checkpoints.forEach((item, index) => {
                const li = $('<li></li>').text(item);
                if (index + 1 < currentStep) {
                  li.addClass('done');
                } else if (index + 1 === currentStep) {
                  li.addClass('current');
                }
                $('#checkpointList').append(li);
              });

              */
		});
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

	if(top.webWrapper){
		document.getElementById("loadingBarBlock").style.marginTop = blockmarginTop+"px";
		document.getElementById("LoadingBar").style.width = "100%";
	}
	else{
		document.getElementById("loadingBarBlock").style.marginTop = blockmarginTop+"px";
		document.getElementById("loadingBarBlock").style.marginLeft = blockmarginLeft+"px";
		document.getElementById("LoadingBar").style.width = winW+"px";
		document.getElementById("LoadingBar").style.height = winH+"px";	
	}
	
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

	document.getElementById("dr_sweet_advise").style.position = "fixed";
	document.getElementById("dr_sweet_advise").style.top = "25%";
	document.getElementById("dr_sweet_advise").style.left = "15%";
	document.getElementById("dr_sweet_advise").style.transform = "translate(-50%, -50%)";
	document.getElementById("dr_sweet_advise").style.zIndex = "9999";
	document.getElementById("hiddenMask").style.position = "fixed";
	document.getElementById("hiddenMask").style.top = "0";
	document.getElementById("hiddenMask").style.left = "0";
	document.getElementById("hiddenMask").style.width = "100%";
	document.getElementById("hiddenMask").style.height = "100vh";
	document.getElementById("hiddenMask").style.display = "flex";
	document.getElementById("hiddenMask").style.justifyContent = "center";
	document.getElementById("hiddenMask").style.alignItems = "center";
	document.getElementById("hiddenMask").style.zIndex = "9998";
	document.getElementById("hiddenMask").style.visibility = "visible";
}

function cancel_dr_advise(){	
	parent.document.getElementById("hiddenMask").style.visibility = "hidden";
	htmlbodyforIE = parent.document.getElementsByTagName("html");  //this both for IE&FF, use "html" but not "body" because <!DOCTYPE html PUBLIC.......>
	htmlbodyforIE[0].style.overflow = "scroll";	  //hidden the Y-scrollbar for preventing from user scroll it.	
	window.scrollTo(0, 0);	//x-axis , y-axis	
}
