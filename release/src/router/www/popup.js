// JavaScript Document
var winH, winW;

function winW_H() {
    if (parseInt(navigator.appVersion) > 3) {
        winW = document.documentElement.scrollWidth;
        if (document.documentElement.clientHeight > document.documentElement.scrollHeight)
            winH = document.documentElement.clientHeight;
        else
            winH = document.documentElement.scrollHeight;
    }
}

function LoadingTime(seconds, flag) {
    showtext(document.getElementById("proceeding_main_txt"), "<#Main_alert_proceeding_desc1#>...");
    document.getElementById("Loading").style.visibility = "visible";

    y = y + progress;
    if (typeof (seconds) == "number" && seconds >= 0) {
        if (seconds != 0) {
            /* trigger IE8 counter */
            if (navigator.appName.indexOf("Microsoft") >= 0) {
                var childsel = document.createElement("span");
                document.body.appendChild(childsel);
                document.body.removeChild(document.body.lastChild);
            }

            showtext(document.getElementById("proceeding_main_txt"), "<#Main_alert_proceeding_desc4#>");
            showtext(document.getElementById("proceeding_txt"), '<span style="color:#FFFFCC;">' + Math.round(y) + "% </span><#Main_alert_proceeding_desc1#>");
            --seconds;
            setTimeout("LoadingTime(" + seconds + ", '" + flag + "');", 1000);
        } else {
            showtext(document.getElementById("proceeding_main_txt"), translate("<#Main_alert_proceeding_desc3#>"));
            showtext(document.getElementById("proceeding_txt"), "");
            y = 0;

            if (flag != "waiting")
                setTimeout("hideLoading();", 1000);
        }
    }
}

function LoadingProgress(seconds) {
    document.getElementById("LoadingBar").style.visibility = "visible";

    y = y + progress;
    if (typeof (seconds) == "number" && seconds >= 0) {
        if (seconds != 0) {
            document.getElementById("proceeding_img").style.width = Math.round(y) + "%";
            document.getElementById("proceeding_img_text").innerHTML = Math.round(y) + "%";

            if (document.getElementById("loading_block1")) {
                document.getElementById("proceeding_img_text").style.width = document.getElementById("loading_block1").clientWidth;
                document.getElementById("proceeding_img_text").style.marginLeft = "175px";
            }
            --seconds;
            setTimeout("LoadingProgress(" + seconds + ");", 1000);
        } else {
            document.getElementById("proceeding_img_text").innerHTML = "<#Main_alert_proceeding_desc3#>";
            y = 0;
            if (location.pathname.indexOf("Advanced_MobileBroadband_Content") > 0) {
                setTimeout("hideLoadingBar();", 1000);
                htmlbodyforIE = document.getElementsByTagName("html");
                htmlbodyforIE[0].style.overflow = "";
            } else if (location.pathname.indexOf("QIS_wizard.htm") < 0 && location.pathname.indexOf("Advanced_FirmwareUpgrade_Content") < 0 && location.pathname.indexOf("Advanced_SettingBackup_Content") < 0) {
                setTimeout("hideLoadingBar();", 1000);
                location.href = "/";
            }
        }
    }
}

class AsusLoaderComponent {

    constructor(props) {
        this.id = props.id;
        const type = (typeof props.type == 'undefined') ? 'spinner' : props.type;
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
        this.rescueCountdownTotalSeconds = 40 * 60;
        this.rescueCountdownTimestamp = null;
        this.rescueCountdownStored = false;
        this.rescueCountdownCleared = false;

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
				background-color: var(--loader-mask-bg, rgba(255, 255, 255, 0.6));
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
				border-top-color: var(--brand-color, #007bff);
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
		      .progress-countdown {
		        margin: -30px 0px 0px 0px;
		        padding: 2px 14px;
		        border-radius: 8px;
		        /*background: #eef5ff;*/
		        border: 1px solid #b7d4ff;
		        color: #1f3f75;
		        font-size: 16px;
		        font-weight: 600;
		        text-align: center;
		      }
		      .progress-countdown .time {
		        margin: 0 6px;
		        color: #0a5edb;
		        font-size: 64px;
		        font-weight: 700;
		      }
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
		      .notice {
		        margin-top: 0px;
		        padding: 2px 16px;
		        max-width: 80%;
		        border-left: 6px solid #d9534f;
		        border-radius: 8px;
		        background: #fff4dc;
		        box-shadow: 0 4px 12px rgba(0, 0, 0, 0.08);
		        color: #b22222;
		        font-size: 24px;
		        font-weight: 700;
		        line-height: 1.5;
		      }


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

        // Dedicated style for the AI Board rescue progress UI (type 'rescueDetail').
        // Kept separate from detailStyle so the live-update loader is untouched.
        const rescueDetailStyle = `<style>
			.popup_bg {
				position: fixed; inset: 0; z-index: 2000;
				display: flex; align-items: center; justify-content: center;
				padding: 24px;
				backdrop-filter: blur(4px); -webkit-backdrop-filter: blur(4px);
				background-color: var(--wrt-overlay-bg, rgba(8,10,16,0.66));
				font-family: "Segoe UI", system-ui, -apple-system, sans-serif;
			}
			.rescue-card {
				/* AI Board brand blue (matches design); fixed so it stays blue
				   regardless of the active ROG/TUF brand theme. Surfaces and text
				   below still use design tokens so light/dark both look right. */
				--rb-accent: var(--brand-color, #2f6bff);
				--rb-accent-2:  var(--primary-40, #5b8cff);
				--rb-track: rgba(140,160,190,0.22);
				--rb-warn-bg: rgba(240,180,80,0.10);
				--rb-warn-border: rgba(240,180,80,0.34);
				--rb-warn-fg: #e3ad5f;
				box-sizing: border-box;
				width: 100%; max-width: 640px;
				background: var(--surface-card, #121826);
				border: 1px solid var(--border-subtle, rgba(255,255,255,0.08));
				border-radius: var(--wrt-card-radius, 16px);
				box-shadow: 0 24px 60px rgba(0,0,0,0.45);
				padding: 24px 28px 22px;
				color: var(--text-primary, #e6eaf2);
			}
			.rescue-card *, .rescue-card *::before, .rescue-card *::after { box-sizing: border-box; }

			/* header */
			.rescue-header { display:flex; align-items:center; gap:14px; margin-bottom:20px; }
			.rescue-badge {
				flex:0 0 auto; width:44px; height:44px; border-radius:12px;
				display:flex; align-items:center; justify-content:center;
				background: var(--brand-color);
			}
			.rescue-badge svg { width:24px; height:24px; display:block; }
			/* icon-AiBoard mask inlined (shadow DOM can't reach the page's
			   .icon-AiBoard rule); coloured white over the blue badge. */
			.rescue-badge .ai-icon {
				width:24px; height:24px; display:block;
				background-color:#fff;
				-webkit-mask-repeat:no-repeat; mask-repeat:no-repeat;
				-webkit-mask-position:center; mask-position:center;
				-webkit-mask-size:contain; mask-size:contain;
				-webkit-mask-image:url("data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyMCIgaGVpZ2h0PSIyMCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDIwIDIwIj48ZyBpZD0iR3JvdXAiPjxnIGlkPSJWZWN0b3IiPjxwYXRoIGZpbGw9IiMxODE4MTgiIGZpbGwtcnVsZT0iZXZlbm9kZCIgZD0iTTEyLjc3NzggMS4xMTExMUgxMS42NjY3VjBIMTIuNzc3OFYxLjExMTExWk0xNy4yMjIyIDQuNjcyMjJMMTUuMzI3OCAyLjc3Nzc4SDQuNjcyMjJMMi43Nzc3OCA0LjY3MjIyVjE1LjMyMjJMNC42NzIyMiAxNy4yMTY3SDE1LjMyNzhMMTcuMjIyMiAxNS4zMjIyVjQuNjcyMjJaTTQuNDQ0NDQgMi4yMjIyMkgxNS41NTU2TDE3Ljc3NzggNC40NDQ0NFYxNS41NTU2TDE1LjU1NTYgMTcuNzc3OEg0LjQ0NDQ0TDIuMjIyMjIgMTUuNTU1NlY0LjQ0NDQ0TDQuNDQ0NDQgMi4yMjIyMlpNMTMuODg4OSAxLjExMTExSDE2LjQzNjVMMTguODg4OSAzLjU2MzQ1VjYuMTExMTFIMjBWMy4xMDMyMUwxNi44OTY4IDBIMTMuODg4OVYxLjExMTExWk0xMC41NTU2IDEuMTExMTFIOS40NDQ0NFYwSDEwLjU1NTZWMS4xMTExMVpNNy4yMjIyMiAxLjExMTExSDguMzMzMzNWMEg3LjIyMjIyVjEuMTExMTFaTTEyLjc3NzggMjBIMTEuNjY2N1YxOC44ODg5SDEyLjc3NzhWMjBaTTkuNDQ0NDQgMjBIMTAuNTU1NlYxOC44ODg5SDkuNDQ0NDRWMjBaTTguMzMzMzMgMjBINy4yMjIyMlYxOC44ODg5SDguMzMzMzNWMjBaTTE4Ljg4ODkgMTEuNjY2N1YxMi43Nzc4SDIwVjExLjY2NjdIMTguODg4OVpNMTguODg4OSAxMC41NTU2VjkuNDQ0NDRIMjBWMTAuNTU1NkgxOC44ODg5Wk0xOC44ODg5IDcuMjIyMjJWOC4zMzMzM0gyMFY3LjIyMjIySDE4Ljg4ODlaTTAgMTIuNzc3OFYxMS42NjY3SDEuMTExMTFWMTIuNzc3OEgwWk0wIDkuNDQ0NDRWMTAuNTU1NkgxLjExMTExVjkuNDQ0NDRIMFpNMCA4LjMzMzMzVjcuMjIyMjJIMS4xMTExMVY4LjMzMzMzSDBaTTMuMTAzMjEgMEwwIDMuMTAzMjFWNi4xMTExMUgxLjExMTExVjMuNTYzNDVMMy41NjM0NSAxLjExMTExSDYuMTExMTFWMEgzLjEwMzIxWk0xOC44ODg5IDE2LjQzNjVWMTMuODg4OUgyMFYxNi44OTY4TDE2Ljg5NjggMjBIMTMuODg4OVYxOC44ODg5SDE2LjQzNjVMMTguODg4OSAxNi40MzY1Wk0wIDEzLjg4ODlWMTYuODk2OEwzLjEwMzIxIDIwSDYuMTExMTFWMTguODg4OUgzLjU2MzQ1TDEuMTExMTEgMTYuNDM2NVYxMy44ODg5SDBaIiBjbGlwLXJ1bGU9ImV2ZW5vZGQiLz48cGF0aCBmaWxsPSJibGFjayIgZmlsbC1ydWxlPSJldmVub2RkIiBkPSJNOC44NTYzNCA1LjYyODI2TDEyLjM2NTUgMTUuMTg2NkwxMi40MjA4IDE1LjMzNjRIMTAuOTcwMUwxMC45NDQgMTUuMjYxNUwxMC4wMDk4IDEyLjYyOTFINi4zMDFMNS4zNjY3NSAxNS4yNjE1TDUuMzQwNzEgMTUuMzM2NEgzLjg4ODg5TDMuOTQ0MjMgMTUuMTg2Nkw3LjQ1MzM0IDUuNjI4MjZMNy40ODA0NyA1LjU1NTU2SDguODMwM0w4Ljg1NjM0IDUuNjI4MjZaTTYuNzQ0NzkgMTEuMzgwMkg5LjU2NTk3TDguMTU1MzggNy40MDQ1MUw2Ljc0NDc5IDExLjM4MDJaIiBjbGlwLXJ1bGU9ImV2ZW5vZGQiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTE1LjAxODQgMTUuMzM2NEgxMy42MzkzVjUuNTU1NTZIMTUuMDE4NFYxNS4zMzY0WiIvPjwvZz48L2c+PC9zdmc+");
				mask-image:url("data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyMCIgaGVpZ2h0PSIyMCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDIwIDIwIj48ZyBpZD0iR3JvdXAiPjxnIGlkPSJWZWN0b3IiPjxwYXRoIGZpbGw9IiMxODE4MTgiIGZpbGwtcnVsZT0iZXZlbm9kZCIgZD0iTTEyLjc3NzggMS4xMTExMUgxMS42NjY3VjBIMTIuNzc3OFYxLjExMTExWk0xNy4yMjIyIDQuNjcyMjJMMTUuMzI3OCAyLjc3Nzc4SDQuNjcyMjJMMi43Nzc3OCA0LjY3MjIyVjE1LjMyMjJMNC42NzIyMiAxNy4yMTY3SDE1LjMyNzhMMTcuMjIyMiAxNS4zMjIyVjQuNjcyMjJaTTQuNDQ0NDQgMi4yMjIyMkgxNS41NTU2TDE3Ljc3NzggNC40NDQ0NFYxNS41NTU2TDE1LjU1NTYgMTcuNzc3OEg0LjQ0NDQ0TDIuMjIyMjIgMTUuNTU1NlY0LjQ0NDQ0TDQuNDQ0NDQgMi4yMjIyMlpNMTMuODg4OSAxLjExMTExSDE2LjQzNjVMMTguODg4OSAzLjU2MzQ1VjYuMTExMTFIMjBWMy4xMDMyMUwxNi44OTY4IDBIMTMuODg4OVYxLjExMTExWk0xMC41NTU2IDEuMTExMTFIOS40NDQ0NFYwSDEwLjU1NTZWMS4xMTExMVpNNy4yMjIyMiAxLjExMTExSDguMzMzMzNWMEg3LjIyMjIyVjEuMTExMTFaTTEyLjc3NzggMjBIMTEuNjY2N1YxOC44ODg5SDEyLjc3NzhWMjBaTTkuNDQ0NDQgMjBIMTAuNTU1NlYxOC44ODg5SDkuNDQ0NDRWMjBaTTguMzMzMzMgMjBINy4yMjIyMlYxOC44ODg5SDguMzMzMzNWMjBaTTE4Ljg4ODkgMTEuNjY2N1YxMi43Nzc4SDIwVjExLjY2NjdIMTguODg4OVpNMTguODg4OSAxMC41NTU2VjkuNDQ0NDRIMjBWMTAuNTU1NkgxOC44ODg5Wk0xOC44ODg5IDcuMjIyMjJWOC4zMzMzM0gyMFY3LjIyMjIySDE4Ljg4ODlaTTAgMTIuNzc3OFYxMS42NjY3SDEuMTExMTFWMTIuNzc3OEgwWk0wIDkuNDQ0NDRWMTAuNTU1NkgxLjExMTExVjkuNDQ0NDRIMFpNMCA4LjMzMzMzVjcuMjIyMjJIMS4xMTExMVY4LjMzMzMzSDBaTTMuMTAzMjEgMEwwIDMuMTAzMjFWNi4xMTExMUgxLjExMTExVjMuNTYzNDVMMy41NjM0NSAxLjExMTExSDYuMTExMTFWMEgzLjEwMzIxWk0xOC44ODg5IDE2LjQzNjVWMTMuODg4OUgyMFYxNi44OTY4TDE2Ljg5NjggMjBIMTMuODg4OVYxOC44ODg5SDE2LjQzNjVMMTguODg4OSAxNi40MzY1Wk0wIDEzLjg4ODlWMTYuODk2OEwzLjEwMzIxIDIwSDYuMTExMTFWMTguODg4OUgzLjU2MzQ1TDEuMTExMTEgMTYuNDM2NVYxMy44ODg5SDBaIiBjbGlwLXJ1bGU9ImV2ZW5vZGQiLz48cGF0aCBmaWxsPSJibGFjayIgZmlsbC1ydWxlPSJldmVub2RkIiBkPSJNOC44NTYzNCA1LjYyODI2TDEyLjM2NTUgMTUuMTg2NkwxMi40MjA4IDE1LjMzNjRIMTAuOTcwMUwxMC45NDQgMTUuMjYxNUwxMC4wMDk4IDEyLjYyOTFINi4zMDFMNS4zNjY3NSAxNS4yNjE1TDUuMzQwNzEgMTUuMzM2NEgzLjg4ODg5TDMuOTQ0MjMgMTUuMTg2Nkw3LjQ1MzM0IDUuNjI4MjZMNy40ODA0NyA1LjU1NTU2SDguODMwM0w4Ljg1NjM0IDUuNjI4MjZaTTYuNzQ0NzkgMTEuMzgwMkg5LjU2NTk3TDguMTU1MzggNy40MDQ1MUw2Ljc0NDc5IDExLjM4MDJaIiBjbGlwLXJ1bGU9ImV2ZW5vZGQiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTE1LjAxODQgMTUuMzM2NEgxMy42MzkzVjUuNTU1NTZIMTUuMDE4NFYxNS4zMzY0WiIvPjwvZz48L2c+PC9zdmc+");
			}
			.rescue-title { font-size:18px; font-weight:700; line-height:1.2; color: var(--text-primary,#e6eaf2); }
			.rescue-subtitle { margin-top:4px; font-size:12.5px; color: var(--text-secondary,#93a0b5); }

			/* stepper */
			.stage-bar { display:flex; margin: 6px 0 26px; }
			.stage-bar .step {
				position:relative; flex:1; display:flex; flex-direction:column;
				align-items:center; gap:10px; text-align:center; font-style:normal;
			}
			.step-line {
				position:absolute; top:18px; left:50%; width:100%; height:3px;
				background: var(--rb-track); border-radius:2px; z-index:0;
			}
			.step:last-child .step-line { display:none; }
			.step.active:not(.current) .step-line { background: var(--rb-accent); }
			.dot {
				position:relative; z-index:1;
				width:36px; height:36px; border-radius:50%;
				display:flex; align-items:center; justify-content:center;
				background: var(--surface-card, #121826);
				border:2px solid var(--rb-track);
				color: var(--text-secondary, #93a0b5);
				transition: background .25s ease, border-color .25s ease, color .25s ease;
			}
			.dot-num { font-size:14px; font-weight:600; font-style:normal; }
			.dot-check, .dot-spin { display:none; }
			.dot-check::before { content:"\\2713"; font-size:18px; font-weight:700; color:#fff; }
			/* done */
			.step.active:not(.current) .dot { background: var(--rb-accent); border-color: var(--rb-accent); }
			.step.active:not(.current) .dot-num { display:none; }
			.step.active:not(.current) .dot-check { display:block; }
			/* current (in progress) */
			.step.current .dot { border-color: var(--rb-accent); }
			.step.current .dot-num { display:none; }
			.step.current .dot-spin {
				display:block; width:20px; height:20px; border-radius:50%;
				border:2px solid var(--rb-accent); border-top-color: transparent;
				animation: rb-spin .8s linear infinite;
			}
			.step.current .dot::after {
				content:""; position:absolute; inset:-5px; border-radius:50%;
				border:2px solid var(--rb-accent); animation: rb-pulse 1.6s ease-out infinite;
			}
			.step-label { font-size:12px; color: var(--text-secondary,#93a0b5); }
			.step.active .step-label { color: var(--text-primary,#e6eaf2); }
			.step.current .step-label { color: var(--rb-accent); font-weight:600; }
			@keyframes rb-spin { to { transform: rotate(360deg); } }
			@keyframes rb-pulse { 0%{transform:scale(1);opacity:.6;} 100%{transform:scale(1.5);opacity:0;} }

			/* progress */
			.rescue-progress-head { display:flex; align-items:flex-end; justify-content:space-between; margin-bottom:10px; gap:12px; }
			.rescue-step-name { font-size:14px; font-weight:600; color: var(--text-primary,#e6eaf2); }
			.rescue-percent { font-size:30px; font-weight:800; line-height:1; color: var(--rb-accent); }
			.progress-bar {
				position:relative; width:100%; height:10px; border-radius:999px;
				background: var(--rb-track); overflow:hidden;
			}
			.progress-fill {
				position:relative; height:100%; width:0%; border-radius:999px; overflow:hidden;
				background: linear-gradient(90deg, var(--rb-accent), var(--rb-accent-2));
				transition: width .5s ease;
			}
			.progress-fill::after {
				content:""; position:absolute; inset:0;
				background: linear-gradient(100deg, transparent 20%, rgba(255,255,255,0.45) 50%, transparent 80%);
				transform: translateX(-100%); animation: rb-shimmer 1.6s ease-in-out infinite;
			}
			@keyframes rb-shimmer { to { transform: translateX(100%); } }
			.rescue-eta { display:flex; align-items:center; gap:6px; margin-top:10px; font-size:12.5px; color: var(--text-secondary,#93a0b5); }
			.rescue-eta svg { width:14px; height:14px; opacity:.85; flex:0 0 auto; }
			.rescue-eta .time { color: var(--text-primary,#e6eaf2); font-weight:600; }
			.progress-notice { margin-top:8px; font-size:12.5px; color: var(--text-secondary,#93a0b5); }

			/* amber warning banner */
			.notice {
				display:flex; align-items:flex-start; gap:10px;
				margin-top:20px; padding:12px 14px; border-radius:10px;
				background: var(--rb-warn-bg); border:1px solid var(--rb-warn-border);
				color: var(--rb-warn-fg); font-size:12.5px; font-weight:500; line-height:1.55;
			}
			.notice .notice-ico { flex:0 0 auto; display:flex; margin-top:1px; }
			.notice .notice-ico svg { width:18px; height:18px; display:block; }
			.checkpoints { display:none; }
		</style>`;

        if (type == 'spinner') {
            template.innerHTML = `
            ${style}
            <div id="loader" class="popup_bg d-flex flex-column align-items-center justify-content-center gap-2">
                <i class="gg-spinner"></i>
                <div id="proceeding_txt"><#Main_alert_proceeding_desc1#>...</div>        
            </div>
        `;
        } else if (type == 'detail') {
            const _dot = (n) => `<i class="dot"><i class="dot-num">${n}</i><i class="dot-check"></i><i class="dot-spin"></i></i>`;
            template.innerHTML = `
            ${rescueDetailStyle}
            <div id="loader" class="popup_bg rescue-board">
              <div class="rescue-card">

                <div class="rescue-header">
                  <div class="rescue-badge"><i class="ai-icon"></i></div>
                  <div>
                    <div class="rescue-title" id="loader_title"><#Setting_live_ai_title#></div>
                    <div class="rescue-subtitle"><#Setting_prog_ai_serveralTime#></div>
                  </div>
                </div>

                <div class="stage-bar">
                  <span class="step" id="step_start">${_dot(1)}<i class="step-line"></i><i class="step-label"><#fw_downloading#></i></span>
                  <span class="step" id="step_docker">${_dot(2)}<i class="step-line"></i><i class="step-label"><#Setting_live_ai_install#></i></span>
                  <span class="step" id="step_done">${_dot(3)}<i class="step-line"></i><i class="step-label"><#Setting_live_ai_done#></i></span>
                </div>

                <div class="rescue-progress">
                  <div class="rescue-progress-head">
                    <span class="rescue-step-name" id="proceeding_step"><#Setting_prog_ai_currStep#></span>
                    <span class="rescue-percent" id="percentInfo">0%</span>
                  </div>
                  <div class="progress-bar">
                    <div class="progress-fill" id="progressFill"></div>
                  </div>
                </div>

                <ul class="checkpoints" id="checkpointList"></ul>
                <div class="notice" id="notice">
                  <span class="notice-ico"><svg viewBox="0 0 24 24"><path fill="currentColor" d="M12 2 1 21h22L12 2Zm0 4.8L19.5 19h-15L12 6.8ZM11 10v5h2v-5h-2Zm0 6v2h2v-2h-2Z"/></svg></span>
                  <span><#Setting_prog_ai_Note#></span>
                </div>

              </div>
            </div>
        `;
        } else if (type == 'rescueDetail') {
            const _dot = (n) => `<i class="dot"><i class="dot-num">${n}</i><i class="dot-check"></i><i class="dot-spin"></i></i>`;
            template.innerHTML = `
            ${rescueDetailStyle}
            <div id="loader" class="popup_bg rescue-board">
              <div class="rescue-card">

                <div class="rescue-header">
                  <div class="rescue-badge"><i class="ai-icon"></i></div>
                  <div>
                    <div class="rescue-title" id="loader_title"><#Setting_rescue_ai_title#></div>
                  </div>
                </div>

                <div class="stage-bar">
                  <span class="step" id="step_start">${_dot(1)}<i class="step-line"></i><i class="step-label"><#Setting_rescue_ai_prepare#></i></span>
                  <span class="step" id="step_upload_fw">${_dot(2)}<i class="step-line"></i><i class="step-label"><#Setting_rescue_ai_FWUpload#></i></span>
                  <span class="step" id="step_install_fw">${_dot(3)}<i class="step-line"></i><i class="step-label"><#Setting_rescue_ai_FWInstall#></i></span>
                  <span class="step" id="step_docker">${_dot(4)}<i class="step-line"></i><i class="step-label"><#Setting_rescue_ai_appInstall#></i></span>
                  <span class="step" id="step_done">${_dot(5)}<i class="step-line"></i><i class="step-label">Done &amp; Reboot</i></span>
                </div>

                <div class="rescue-progress">
                  <div class="rescue-progress-head">
                    <span class="rescue-step-name" id="proceeding_step"><#Setting_prog_ai_currStep#></span>
                    <span class="rescue-percent" id="percentInfo">0%</span>
                  </div>
                  <div class="progress-bar">
                    <div class="progress-fill" id="progressFill"></div>
                  </div>
                  <div class="progress-notice" id="proceeding_notice" style="display:none;"><#Setting_prog_ai_serveralTime#></div>
                </div>
                <div class="rescue-eta" id="rescue_eta_row" style="display:none;">
                    <svg viewBox="0 0 24 24"><path fill="currentColor" d="M12,20A8,8 0 0,0 20,12A8,8 0 0,0 12,4A8,8 0 0,0 4,12A8,8 0 0,0 12,20M12,2A10,10 0 0,1 22,12A10,10 0 0,1 12,22C6.47,22 2,17.5 2,12A10,10 0 0,1 12,2M12.5,7V12.25L17,14.92L16.25,16.15L11,13V7H12.5Z"/></svg>
                    <span class="rescue-eta-label"><#mssid_time_remaining#></span>
                    <span id="rescue_countdown" class="time"></span>
                  </div>
                <ul class="checkpoints" id="checkpointList"></ul>
                <div class="notice" id="notice">
                  <span class="notice-ico"><svg viewBox="0 0 24 24"><path fill="currentColor" d="M12 2 1 21h22L12 2Zm0 4.8L19.5 19h-15L12 6.8ZM11 10v5h2v-5h-2Zm0 6v2h2v-2h-2Z"/></svg></span>
                  <span><#Setting_prog_ai_Note#></span>
                </div>

              </div>
            </div>
        `;
        } else {
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
        if (flag == "rescue") {
            const interval = setInterval(() => {
                if (this.rescueFailed) {
                    this.aiboard_query();
                    setTimeout(() => {
                        this.hide_detail();
                    }, 5000);
                    setTimeout(() => {
                        top.location.reload();
                    }, 5000);
                    clearInterval(interval);
                }
                if (this.rescueDone) {
                    setTimeout(() => {
                        this.aiboard_query();
                    }, 25000);
                    setTimeout(() => {
                        this.hide_detail();
                    }, 30000);
                    setTimeout(() => {
                        top.location.reload();
                    }, 30000);
                    clearInterval(interval);
                }
                this.update_rescue_status();
            }, 1000);

        }
        if (flag == "liveupdate") {

            const interval = setInterval(() => {
                if (this.liveupdateFailed) {
                    this.aiboard_query();
                    setTimeout(() => {
                        this.hide_detail();
                    }, 5000);
                    setTimeout(() => {
                        top.location.reload();
                    }, 5000);
                    clearInterval(interval);
                }
                if (this.liveupdateDone) {
                    setTimeout(() => {
                        this.aiboard_query();
                    }, 25000);
                    setTimeout(() => {
                        this.hide_detail();
                    }, 30000);
                    setTimeout(() => {
                        top.location.reload();
                    }, 30000);
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
        if (loader) {
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
        if (loader) {
            loader.remove();
        }
        if (top) {
            top.loader = null;
        }
    }

    aiboard_query() {
        let initPost = {};
        initPost.action_mode = "apply";
        initPost.rc_service = "start_ai_request query";
        httpApi.nvramSet(initPost);
    }

    formatRescueCountdown(seconds) {
        const safeSeconds = Math.max(0, seconds);
        const minutes = Math.floor(safeSeconds / 60);
        const remainingSeconds = safeSeconds % 60;
        return `${String(minutes).padStart(2, '0')}:${String(remainingSeconds).padStart(2, '0')}`;
    }

    getRescueCountdownTimestamp() {
        if (Number.isInteger(this.rescueCountdownTimestamp) && this.rescueCountdownTimestamp > 0) {
            return this.rescueCountdownTimestamp;
        }

        const countdownNvram = httpApi.nvramGet(["ai_rescue_countdown_ts"]);
        const countdownTimestamp = parseInt(countdownNvram.ai_rescue_countdown_ts, 10);
        if (!isNaN(countdownTimestamp) && countdownTimestamp > 0) {
            this.rescueCountdownTimestamp = countdownTimestamp;
            this.rescueCountdownStored = true;
        }

        return this.rescueCountdownTimestamp;
    }

    setRescueCountdownTimestamp(timestamp) {
        if (!Number.isInteger(timestamp) || timestamp <= 0) {
            return;
        }
        if (this.rescueCountdownStored && this.rescueCountdownTimestamp === timestamp) {
            return;
        }

        this.rescueCountdownTimestamp = timestamp;
        this.rescueCountdownStored = true;
        this.rescueCountdownCleared = false;

        httpApi.nvramSet({
            action_mode: "apply",
            ai_rescue_countdown_ts: String(timestamp)
        });
    }

    clearRescueCountdownTimestamp() {
        if (this.rescueCountdownCleared) {
            return;
        }

        this.rescueCountdownTimestamp = null;
        this.rescueCountdownStored = false;
        this.rescueCountdownCleared = true;
        this.toggleRescueCountdown(false);

        httpApi.nvramSet({
            action_mode: "apply",
            ai_rescue_countdown_ts: ""
        });
    }

    toggleRescueCountdown(showCountdown) {
        const rescueCountdown = this.shadowRoot.getElementById('rescue_countdown');
        if (!rescueCountdown) {
            return;
        }
        // Toggle the whole ETA row (clock + "剩餘時間" label + time) so the
        // label always sits next to the time, not in the percentage slot.
        const etaRow = this.shadowRoot.getElementById('rescue_eta_row') || rescueCountdown.parentElement;

        if (!showCountdown) {
            if (etaRow) etaRow.style.display = "none";
            rescueCountdown.textContent = "";
            return;
        }

        const countdownTimestamp = this.getRescueCountdownTimestamp();
        if (!countdownTimestamp) {
            if (etaRow) etaRow.style.display = "none";
            rescueCountdown.textContent = "";
            return;
        }

        const elapsedSeconds = Math.max(0, Math.floor(Date.now() / 1000) - countdownTimestamp);
        const remainingSeconds = Math.max(0, this.rescueCountdownTotalSeconds - elapsedSeconds);
        if (etaRow) etaRow.style.display = "";
        rescueCountdown.textContent = this.formatRescueCountdown(remainingSeconds);
    }

    update_liveupdate_status() {
        const self = this;
        $.getJSON("/get_aisom_upgrade_info.cgi", function (data) {
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
            if (typeof data.ai_prog_status !== "undefined") {
                if (data.ai_prog_status === "START" ||
                    data.ai_prog_status === "DOWNLOAD_FW" || data.ai_prog_status === "DOWNLOAD_FW_RSA" || data.ai_prog_status === "VERIFY_FW") {
                    proceeding_step.textContent = `<#Setting_prog_ai_currStep#> ` + stepText;
                    stages[0].classList.add('active');
                    self.shadowRoot.getElementById('step_start').classList.add('active');
                } else if (data.ai_prog_status.includes("DOWNLOAD") && data.ai_prog_current_image === "aisom.swu") {
                    proceeding_step.textContent = `<#Setting_prog_ai_currStep#> ` + stepText;
                    stages[0].classList.add('active');
                    self.shadowRoot.getElementById('step_start').classList.add('active');
                } else if (data.ai_prog_status.includes("PROGRESS")) {
                    proceeding_step.textContent = `<#Setting_prog_ai_currStep#> ` + stepText;
                    stages[0].classList.add('active');
                    self.shadowRoot.getElementById('step_start').classList.add('active');
                    stages[1].classList.add('active');
                    self.shadowRoot.getElementById('step_docker').classList.add('active');
                } else if (data.ai_prog_status.includes("failed")) {
                    proceeding_step.textContent = `Live update failed`;
                    stages[2].classList.add('active');
                    progressFill.style.visibility = "hidden";
                    percentInfo.style.visibility = "hidden";
                    notice_div.style.visibility = "hidden";

                    self.liveupdateFailed = true;
                } else if (data.ai_prog_status.includes("DONE")) {
                    proceeding_step.textContent = `<#Setting_live_ai_complete#>`;
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
            }

            // The highest reached (.active) step is the one in progress
            // (spinner + pulse); earlier reached steps are completed (✓).
            let lastActiveIdx = -1;
            stages.forEach((el, i) => {
                el.classList.remove('current');
                if (el.classList.contains('active')) lastActiveIdx = i;
            });
            if (lastActiveIdx >= 0 && !self.liveupdateFailed) {
                stages[lastActiveIdx].classList.add('current');
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
        $.getJSON("/get_aisom_upgrade_info.cgi", function (data) {
            //console.log(data);
            const percent = data.ai_prog_current_percent;
            const stepText_prepare = `${data.ai_prog_current_image} (${data.ai_prog_current_step}/${data.ai_prog_total_steps})`;
            const stepText = `${data.ai_prog_current_image} (${data.ai_prog_current_step}/${data.ai_prog_total_steps})`;
            const proceeding_step = self.shadowRoot.getElementById('proceeding_step');
            const progressFill = self.shadowRoot.getElementById('progressFill');
            const percentInfo = self.shadowRoot.getElementById('percentInfo');
            const notice_div = self.shadowRoot.getElementById('notice');
            percentInfo.textContent = percent + '%';
            progressFill.style.width = percent + '%';
            let showRescueCountdown = false;


            // Stage progress

            const stages = self.shadowRoot.querySelectorAll('.stage-bar span');
            // remove whole span active class
            stages.forEach(span => span.classList.remove('active'));
            // To add active class while matched status
            if (typeof data.ai_prog_status !== "undefined") {
                if (data.ai_prog_status === "START") {

                    self.rescueStart = true;
                    proceeding_step.textContent = `<#Setting_prog_ai_currStep#> <#Setting_rescue_ai_enter_new#> ` + stepText_prepare;
                    //proceeding_notice.style.display = "";
                } else if (data.ai_prog_status.includes("DOWNLOAD") && data.ai_prog_current_image.includes("aisom_reset")) {

                    if (!self.getRescueCountdownTimestamp()) {
                        self.setRescueCountdownTimestamp(Math.floor(Date.now() / 1000));
                    }
                    self.rescueStart = true;
                    self.rescueFwDownload = true;
                    proceeding_step.textContent = `<#Setting_prog_ai_currStep#> ` + stepText;
                    showRescueCountdown = true;
                    // percent stays in the big slot; remaining time shows in the ETA row
                    //proceeding_notice.style.display = "none";
                } else if (data.ai_prog_status.includes("PROGRESS") && (data.ai_prog_current_image.includes("subimg.gz") || data.ai_prog_current_image.includes("install"))) {

                    self.rescueStart = true;
                    self.rescueFwDownload = true;
                    self.rescueFwInstall = true;
                    proceeding_step.textContent = `<#Setting_prog_ai_currStep#> ` + stepText;
                    showRescueCountdown = true;
                    // percent stays in the big slot; remaining time shows in the ETA row
                    //proceeding_notice.style.display = "none";
                } else if (data.ai_prog_status === "DONE" ||
                    (data.ai_prog_status.includes("DOWNLOAD") && data.ai_prog_current_image.includes("docker_images")) ||
                    data.ai_prog_status.includes("SUCCESS") ||
                    data.ai_prog_status.includes("LOAD")
                ) {

                    self.rescueStart = true;
                    self.rescueFwDownload = true;
                    self.rescueFwInstall = true;
                    self.rescueAPPInstall = true;
                    proceeding_step.textContent = `<#Setting_prog_ai_currStep#> ` + stepText;
                    // percent stays in the big slot; remaining time shows in the ETA row
                    showRescueCountdown = true;
                    //proceeding_notice.style.display = "none";
                } else if (data.ai_prog_status.includes("failed")) {
                    proceeding_step.textContent = `AI Board rescue failed`;
                    //proceeding_notice.style.display = "none";
                    progressFill.style.visibility = "hidden";
                    percentInfo.style.visibility = "hidden";
                    notice_div.style.visibility = "hidden";

                    self.clearRescueCountdownTimestamp();
                    self.rescueFailed = true;
                } else if (data.ai_prog_status.includes("DOCKER_DONE")) {
                    proceeding_step.textContent = `<#Setting_rescue_ai_complete#>`;
                    //proceeding_notice.style.display = "none";
                    progressFill.style.visibility = "hidden";
                    //percentInfo.style.visibility = "hidden";
                    //notice_div.style.visibility = "hidden";

                    self.clearRescueCountdownTimestamp();
                    self.rescueStart = true;
                    self.rescueFwDownload = true;
                    self.rescueFwInstall = true;
                    self.rescueAPPInstall = true;
                    self.rescueDone = true;
                    percentInfo.textContent = '<#usb_initializing#>';
                }
            }

            self.toggleRescueCountdown(showRescueCountdown);

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

            // The highest reached (.active) step is the one in progress
            // (spinner + pulse); the earlier reached steps are completed (✓).
            let lastActiveIdx = -1;
            stages.forEach((el, i) => {
                el.classList.remove('current');
                if (el.classList.contains('active')) lastActiveIdx = i;
            });
            if (lastActiveIdx >= 0 && !self.rescueFailed) {
                stages[lastActiveIdx].classList.add('current');
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

function showLoadingBar(seconds) {
    if (window.scrollTo)
        window.scrollTo(0, 0);

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
        blockmarginLeft = (winWidth * 0.3) + winPadding;
    } else if (winWidth <= 1050) {
        blockmarginLeft = (winWidth) * 0.3 + document.body.scrollLeft;
    }

    if (winHeight > 660)
        winHeight = 660;

    blockmarginTop = winHeight * 0.3

    if (top.webWrapper) {
        document.getElementById("loadingBarBlock").style.marginTop = blockmarginTop + "px";
        document.getElementById("LoadingBar").style.width = "100%";
    } else {
        document.getElementById("loadingBarBlock").style.marginTop = blockmarginTop + "px";
        document.getElementById("loadingBarBlock").style.marginLeft = blockmarginLeft + "px";
        document.getElementById("LoadingBar").style.width = winW + "px";
        document.getElementById("LoadingBar").style.height = winH + "px";
    }

    loadingSeconds = seconds;
    progress = 100 / loadingSeconds;
    y = 0;
    LoadingProgress(seconds);
}

function hideLoadingBar() {
    document.getElementById("LoadingBar").style.visibility = "hidden";
}

function hideLoading(flag) {
    if (isSupport("UI4") && businessLoader) {
        businessLoader.hide();
    } else {
        document.getElementById("Loading").style.visibility = "hidden";
        htmlbodyforIE = document.getElementsByTagName("html");  //this both for IE&FF, use "html" but not "body" because <!DOCTYPE html PUBLIC.......>
        htmlbodyforIE[0].style.overflow = "";	  //hidden the Y-scrollbar for preventing from user scroll it.
        document.getElementById("Loading").style.width = "initial";
        document.getElementById("Loading").style.height = "initial";
    }
}

function dr_advise() {
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

function cancel_dr_advise() {
    parent.document.getElementById("hiddenMask").style.visibility = "hidden";
    htmlbodyforIE = parent.document.getElementsByTagName("html");  //this both for IE&FF, use "html" but not "body" because <!DOCTYPE html PUBLIC.......>
    htmlbodyforIE[0].style.overflow = "scroll";	  //hidden the Y-scrollbar for preventing from user scroll it.
    window.scrollTo(0, 0);	//x-axis , y-axis
}
