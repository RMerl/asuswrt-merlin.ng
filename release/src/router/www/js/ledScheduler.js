const ExternalDeps = {
	schedule: null,
	scheduleHandleData: null,
	getComponentSwitch: null,
	getComponentCustomizeConfirm: null,
	getComponentCustomizeAlert: null
};

class AuraScheduler {
	constructor(params) {
		this.params = params || {};
		this.theme = this.params?.theme || 'RT';
		this.hide_btn_back_container = this.params?.hide_btn_back_container || false;
		this.schedMode = this.params?.schedMode || 'aura';

		const htmlElement = document.documentElement;
		this.asuswrtColor = htmlElement.getAttribute('data-asuswrt-color') || '';
		this.asuswrtTheme = htmlElement.getAttribute('data-asuswrt-theme') || '';
		this.asuswrtStyle = htmlElement.getAttribute('data-asuswrt-style') || '';

		this.element = document.createElement('div');
		this.element.id = "led-scheduler";
		this.element.setAttribute('data-asuswrt-color', this.asuswrtColor);
		this.element.setAttribute('data-asuswrt-theme', this.asuswrtTheme);
		this.element.setAttribute('data-asuswrt-style', this.asuswrtStyle);

		const shadowRoot = this.element.attachShadow({mode: 'open'});
		this.shadowRoot = shadowRoot;

		this._initThemeObserver();

		const template = document.createElement('template');
		template.innerHTML = `
			<style>
				.profile_setting_item.schedule_ui {
					height: auto !important;
				}
				.profile_setting_item.schedule_ui > .schedule_ui {
					margin-left: 2%;
					margin-top: 12px;
					margin-bottom: 12px;
					flex: 0 0 96%;
				}
				.btn_back_container.title {
					font-size: 12px;
					font-weight: 600;
					text-transform: uppercase;
					letter-spacing: 0.2px;
					height: 30px;
					cursor: pointer;
					display: flex;
					color: #FFFFFF;
					align-items: center;
					margin: 6px;
				}
				.btn_back_container .icon_arrow_left {
					margin-right: 12px;
				}
				.profile_setting_item,
				.profile_setting_item.nowrap.switch_item .input_container {
					min-width: 300px;
				}
				.popup_container {
					width: initial !important;
				}
				.profile_setting_item > .title, .profile_setting_two_item .title {
					font-size: 14px !important;
				}
				.popup_container.popup_customize_alert {
					position: fixed;
					left: 50%;
					right: initial !important;
					top: 40% !important;
					transform: translate(-50%, -50%);
					z-index: 9999 !important;
					width: 90% !important;
				}
				.container {
					width: 100% !important;
					margin: 1% 0 !important;
				}
				.icon_text_container {
					display: flex;
					align-items: center;
					gap: 5px;
				}
				.icon_text_container a {
					margin-left: 5px;
				}
				.icon-circle-mask {
					display: inline-block;
					border-radius: 50%;
					background: #43525D;
					height: 30px;
					width: 30px;
					display: flex;
					justify-content: center;
					align-items: center;
				}
				.icon-comments {
					-webkit-mask: url(/images/New_ui/icon_comments.svg) no-repeat;
					mask: url(/images/New_ui/icon_comments.svg) no-repeat;
					height: 14px;
					width: 14px;
					display: inline-block;
					background-color: #FFFFFF;
				}
				.icon-delete-circle {
					width: 18px;
					height: 18px;
					background-color: #F44336;
					border-radius: 50%;
					position: relative;
					display: inline-block;
					margin-top: 3px;
					vertical-align: middle;
				}
				.icon-delete-circle::before,
				.icon-delete-circle::after {
					content: '';
					position: absolute;
					width: 9px;
					height: 2px;
					background-color: white;
					top: 50%;
					left: 50%;
					border-radius: 1px;
				}
				.icon-delete-circle::before {
					transform: translate(-50%, -50%) rotate(45deg);
				}
				.icon-delete-circle::after {
					transform: translate(-50%, -50%) rotate(-45deg);
				}
				.icon-check-circle {
					width: 18px;
					height: 18px;
					background-color: #4CAF50;
					border-radius: 50%;
					position: relative;
					display: inline-block;
					margin-top: 3px;
					vertical-align: middle;
				}
				.icon-check-circle::before {
					content: '';
					position: absolute;
					width: 4px;
					height: 9px;
					border: 2px solid white;
					border-top: none;
					border-left: none;
					top: 3px;
					left: 6px;
					transform: rotate(45deg);
				}
			</style>
			<div id='popupMsgCntr'>
				<div class="popup_container popup_customize_alert"></div>
				<div class="hidden_mask popup_customize_alert"></div>
			</div>
			<div id='ledScheduleCntr' class='container'></div>
		`;
		shadowRoot.appendChild(template.content.cloneNode(true));

		const scripts = ['/RWD_UI/rwd_component.js', '/js/weekSchedule/schedule_ui.js'];
		let styles = ['/RWD_UI/rwd_component.css?v=<% nvram_char_to_ascii("", "extendno"); %>', '/js/weekSchedule/schedule_ui.css'];
		if (this.theme && this.theme !== 'RT') {
			styles.push(`/RWD_UI/rwd_component_${this.theme}.css`);
			styles.push(`/js/weekSchedule/schedule_ui_${this.theme}.css`);
		}
		if (isSupport("UI4")) {
			styles.push(`/RWD_UI/rwd_component_v4.css`);
			styles.push(`/js/weekSchedule/schedule_ui_v4.css`);
		}
		this._loadResources(scripts, styles, () => {
			ExternalDeps.schedule = (typeof top !== 'undefined' && top.schedule) ? top.schedule : schedule;
			ExternalDeps.scheduleHandleData = (typeof top !== 'undefined' && top.schedule_handle_data) ? top.schedule_handle_data : schedule_handle_data;
			ExternalDeps.getComponentSwitch = (typeof top !== 'undefined' && top.Get_Component_Switch) ? top.Get_Component_Switch : Get_Component_Switch;
			ExternalDeps.getComponentCustomizeConfirm = (typeof top !== 'undefined' && top.Get_Component_Customize_Confirm) ? top.Get_Component_Customize_Confirm : Get_Component_Customize_Confirm;
			ExternalDeps.getComponentCustomizeAlert = (typeof top !== 'undefined' && top.Get_Component_Customize_Alert) ? top.Get_Component_Customize_Alert : Get_Component_Customize_Alert;
			this._Get_Component_LedScheduler();
			this._handleExpandBtn();
		});
	}

	render() {
		return this.element;
	}

	getHelpIcon() {
		const show_customize_alert = this._show_customize_alert.bind(this);
		const $aura_help_icon = document.createElement("div");
		$aura_help_icon.className = "help_icon";
		$aura_help_icon.tabIndex = 0;
		$aura_help_icon.setAttribute("role", "button");
		$aura_help_icon.setAttribute("aria-label", "<#HOWTOSETUP#>");
		$aura_help_icon.addEventListener("click", (e) => {
			e.stopPropagation();
			let text = `
				<div class='title'>FAQ</div>
				<div>
					<#Schedule_Help_Hint#> <#Schedule_FAQ#>
				</div>
				<br />
				<div class="icon_text_container">
					<div class="icon-circle-mask"><i class="icon-comments"></i></div>
					<a target="_blank" href="https://www.asus.com/support/" tabindex="0" role="button"
						aria-label="<#HOWTOSETUP#>"
						onkeydown="if(event.key==='Enter'||event.key===' '){this.click();}">
						<#HOWTOSETUP#>
					</a>
				</div>
			`;/* untranslated */
			show_customize_alert(text);
		});
		return $aura_help_icon;
	}

	_Get_Component_LedScheduler() {
		const theme = this.theme;
		const getValue = this._getValue.bind(this);
		const findLedScheConflicts = this._findLedScheConflicts.bind(this);
		const parseTimeString = this._parseTimeString.bind(this);
		const show_customize_confirm = this._show_customize_confirm.bind(this);
		const show_customize_alert = this._show_customize_alert.bind(this);
		const updateScheduleUI = this._updateScheduleUI.bind(this);
		const ledScheduleCntr = this.shadowRoot.querySelector('#ledScheduleCntr');
		const popupMsgCntr = this.shadowRoot.querySelector('#popupMsgCntr');
		const supportAuraNightSched = (() => {
			const ledg_night_mode = httpApi.nvramGet(["ledg_night_mode"])["ledg_night_mode"];
			const nv_support = (ledg_night_mode != "" && ledg_night_mode != undefined);
			const model_support = isSupport("ledg_night_mode") != 0;
			return nv_support && model_support && !!isSupport("AURA_NIGHT_SCHED");
		})();
		const MaxRule_AURA_SCHED = isSupport('MaxRule_AURA_SCHED') || 5;
		const nvramArr = supportAuraNightSched
			? ['aura_timesched', 'aura_sched', 'aura_night_timesched', 'aura_night_sched']
			: ['aura_timesched', 'aura_sched'];
		const auraConfig = httpApi.nvramCharToAscii(nvramArr, true);
		this.aura_timesched = decodeURIComponent(auraConfig['aura_timesched']) || '0';
		this.aura_sched = decodeURIComponent(auraConfig['aura_sched']) || '';
		if (supportAuraNightSched) {
			this.aura_night_timesched = decodeURIComponent(auraConfig['aura_night_timesched']) || '0';
			this.aura_night_sched = decodeURIComponent(auraConfig['aura_night_sched']) || '';
		}

		let $container = $("<div>").addClass("profile_setting").appendTo($(ledScheduleCntr));
		if (!this.hide_btn_back_container) {
			const $title = $("<div>").addClass("title btn_back_container").appendTo($container)
				.unbind("click").click(function(e){
					e = e || event;
					e.stopPropagation();
					switch_mode("panel");
				});
			$("<div>").addClass("icon_arrow_left").appendTo($title);
			$("<div>").html(`<#btn_Back#>`).appendTo($title);
		}

		const auraSchedule = new ExternalDeps.schedule({
			data_max: MaxRule_AURA_SCHED,
			data:ExternalDeps.scheduleHandleData.string_to_json_array(this.aura_sched),
			hideTitle: true,
			alternate_days: true,
			btn_save_callback: saveAuraConfig,
			icon_switch_callback: saveAuraConfig,
			icon_trash_callback: saveAuraConfig,
			show_customize_confirm: this._show_customize_confirm.bind(this),
			show_customize_alert: this._show_customize_alert.bind(this)
		});
		let auraNightSchedule = null;
		if (supportAuraNightSched) {
			auraNightSchedule = new ExternalDeps.schedule({
				data_max: MaxRule_AURA_SCHED,
				data:ExternalDeps.scheduleHandleData.string_to_json_array(this.aura_night_sched),
				hideTitle: true,
				alternate_days: true,
				btn_save_callback: saveAuraNightConfig,
				icon_switch_callback: saveAuraNightConfig,
				icon_trash_callback: saveAuraNightConfig,
				show_customize_confirm: this._show_customize_confirm.bind(this),
				show_customize_alert: this._show_customize_alert.bind(this)
			});
		}

		$("<div>")
			.html(`<#SDN_Sync_To_All_Node#>. <#Schedule_Unsupported_Nodes#>`)
			.addClass("item_hint").appendTo($container);/* untranslated */

		const auraScheParm = {"title":`<#Schedule_Aura_Offline#>`, "type":"switch", "id":"auraSche", "set_value":this.aura_timesched === '1' ? "on" : "off"};
		ExternalDeps.getComponentSwitch(auraScheParm).appendTo($container).find("#" + auraScheParm.id + "").click(function(e){
			e = e || event;
			e.stopPropagation();
			const $controlContainer = $(this).closest(".profile_setting").find("#container_aura_schedule");
			const switchStatus = $(this).hasClass("on") ? '1': '0';
			if (switchStatus === '1') {
				$controlContainer.empty().append(auraSchedule.Get_UI()).show();
			}
			else {
				$controlContainer.hide();
			}
			httpApi.nvramSet({"action_mode": "apply", "rc_service": "no_service", "aura_timesched": switchStatus}, ()=>{
				httpApi.nvramCharToAscii(["aura_timesched"], true);
				this.aura_timesched = switchStatus;
			});
		});
		$("<div>").addClass("profile_setting_item schedule_ui")
			.attr({"id":"container_aura_schedule"})
			.append(auraSchedule.Get_UI()).hide().appendTo($container);
		if (this.aura_timesched === '1') {
			$container.find("#container_aura_schedule").show();
		}

		if (supportAuraNightSched) {
			const auraNightScheParm = {"title":`<#BoostKey_Aura_NightMode#> <#weekSche_Schedule#>`, "type":"switch", "id":"auraNight_sche", "set_value": this.aura_night_timesched === '1' ? "on" : "off"};
			ExternalDeps.getComponentSwitch(auraNightScheParm).appendTo($container).find("#" + auraNightScheParm.id + "").click(function(e){
				e = e || event;
				e.stopPropagation();
				const $controlContainer = $(this).closest(".profile_setting").find("#container_auraNight_schedule");
				const switchStatus = $(this).hasClass("on") ? '1': '0';
				if (switchStatus === '1') {
					$controlContainer.empty().append(auraNightSchedule.Get_UI()).show();
				}
				else {
					$controlContainer.hide();
				}
				httpApi.nvramSet({"action_mode": "apply", "rc_service": "no_service", "aura_night_timesched": switchStatus}, ()=>{
					httpApi.nvramCharToAscii(["aura_night_timesched"], true);
					this.aura_night_timesched = switchStatus;
				});
			});
			$("<div>").addClass("profile_setting_item schedule_ui")
				.attr({"id":"container_auraNight_schedule"})
				.append(auraNightSchedule.Get_UI()).hide().appendTo($container);
			if (this.aura_night_timesched === '1') {
				$container.find("#container_auraNight_schedule").show();
			}
		}

		if (this.schedMode === 'aura' && !this.aura_sched) {
			const $auraSche = $(this.shadowRoot.querySelector('#auraSche'));
			if ($auraSche.length) {
				if (this.aura_timesched === '0') {
					$auraSche.trigger('click');
				}
				const $addNew = $container.find("#container_aura_schedule").find('.add_new_component');
				if ($addNew.length) {
					$addNew.trigger('click');
				}
			}
		}
		if (this.schedMode === 'auraNight' && !this.aura_night_sched) {
			const $auraNight_sche = $(this.shadowRoot.querySelector('#auraNight_sche'));
			if ($auraNight_sche.length) {
				if (this.aura_night_timesched === '0') {
					$auraNight_sche.trigger('click');
				}
				const $addNew = $container.find("#container_auraNight_schedule").find('.add_new_component');
				if ($addNew.length) {
					$addNew.trigger('click');
				}
			}
		}

		function saveAuraConfig(parm) {
			const saveConfig = (schedConfig) => {
				const nvramSetObj = {...{"action_mode": "apply", "rc_service": "no_service"}, ...schedConfig};
				httpApi.nvramSet(nvramSetObj, () => {
					httpApi.nvramCharToAscii(["aura_sched"], true);
					if ('aura_sched' in schedConfig) {
						this.aura_sched = schedConfig.aura_sched;
					}
					if ('aura_night_sched' in schedConfig) {
						this.aura_night_sched = schedConfig.aura_night_sched;
					}
				});
			};
			const oriSchedule = parm.oriSchedule;
			const currentSchedule = parm.currentSchedule;
			let auraNightDataArr = [];
			if (supportAuraNightSched) {
				const auraNightData = getValue(auraNightSchedule) || '';
				auraNightDataArr = auraNightData ? auraNightData.split('<') : [];
			}
			let ledDataArr = [];
			if (isSupport('LED_SCHED')) {
				const ledData = decodeURIComponent(httpApi.nvramCharToAscii(["led_sched"], true)["led_sched"]);
				ledDataArr = ledData ? ledData.split('<') : [];
			}
			const auraData = ExternalDeps.scheduleHandleData.json_array_to_string(auraSchedule.Get_Value());
			const conflictsArr = findLedScheConflicts(currentSchedule, auraNightDataArr);
			const conflictsLedArr = findLedScheConflicts(currentSchedule, ledDataArr);
			let auraNightDataArrClone;
			let ledDataArrClone;
			if (conflictsArr.length > 0 || conflictsLedArr.length > 0) {
				let hint = `<#Schedule_Conflicts#> <#Schedule_System_Adjust#>:<br>`;
				if (conflictsArr.length > 0) {
					hint += `
						<div style="display:flex; gap:6px;">
							<div class="icon-delete-circle" aria-hidden="true"></div>
							<div>
								<#Schedule_Cancel_LED_Night#> : `;
								auraNightDataArrClone = $.extend(true, [], auraNightDataArr);
								conflictsArr.forEach(conflictTime => {
									auraNightDataArrClone = auraNightDataArrClone.map(timeString => {
										if (timeString === conflictTime) {
											return timeString.slice(0, 1) + '0' + timeString.slice(2);
										}
										return timeString;
									});
									hint += `<br>${parseTimeString(conflictTime)}`;
								});
					hint += `
							</div>
						</div>`;
				}
				if (conflictsLedArr.length > 0) {
					if (conflictsArr.length > 0) hint += `<div style="height: 6px;"></div>`;
					hint += `
						<div style="display:flex; gap:6px;">
							<div class="icon-delete-circle" aria-hidden="true"></div>
							<div>
								<#Schedule_Cancel_LED#> : `;
								ledDataArrClone = $.extend(true, [], ledDataArr);
								conflictsLedArr.forEach(conflictTime => {
									ledDataArrClone = ledDataArrClone.map(timeString => {
										if (timeString === conflictTime) {
											return timeString.slice(0, 1) + '0' + timeString.slice(2);
										}
										return timeString;
									});
									hint += `<br>${parseTimeString(conflictTime)}`;
								});
					hint += `
							</div>
						</div>`;
				}

				const hintColor = theme === 'WHITE' ? "#006CE1;" : "#FC0;";
				hint += `<div style="height: 12px;"></div>
					<div style="display:flex; gap:6px;">
						<div class="icon-check-circle" aria-hidden="true"></div>
						<div>
							<span style='color:${hintColor}'><#Schedule_Set_AURA#> : <br>${parseTimeString(currentSchedule)}</span>
						</div>
					</div>`;
				show_customize_confirm(hint);
				const $confirm_obj = $(popupMsgCntr).find(".popup_container.popup_customize_alert");
				const okButton = $confirm_obj.find("[data-btn=ok]")[0];
				if (okButton) {
					okButton.addEventListener('click', function(e) {
						let schedConfig = {
							"aura_sched": auraData
						};
						if (auraNightDataArrClone) {
							const auraNightNewData = auraNightDataArrClone.join('<');
							updateScheduleUI({
								type: "auraNight",
								timeString: auraNightNewData,
								scheduleInstance: auraNightSchedule
							});
							if (supportAuraNightSched) {
								schedConfig["aura_night_sched"] = auraNightNewData;
							}
						}
						if (ledDataArrClone) {
							const ledNewData = ledDataArrClone.join('<');
							schedConfig["led_sched"] = ledNewData;
						}
						saveConfig(schedConfig);
					});
				}
				const cancelButton = $confirm_obj.find("[data-btn=cancel]")[0];
				if (cancelButton) {
					cancelButton.addEventListener('click', function(e) {
						updateScheduleUI({
							type: "aura",
							timeString: oriSchedule,
							scheduleInstance: auraSchedule
						});
					});
				}
			}
			else {
				let schedConfig = {
					"aura_sched": auraData
				};
				saveConfig(schedConfig);
			}
		}
		function saveAuraNightConfig(parm) {
			const saveConfig = (schedConfig) => {
				const nvramSetObj = {...{"action_mode": "apply", "rc_service": "no_service"}, ...schedConfig};
				httpApi.nvramSet(nvramSetObj, () => {
					httpApi.nvramCharToAscii(["aura_sched"], true);
					if ('aura_sched' in schedConfig) {
						this.aura_sched = schedConfig.aura_sched;
					}
					if ('aura_night_sched' in schedConfig) {
						this.aura_night_sched = schedConfig.aura_night_sched;
					}
				});
			};
			const oriSchedule = parm.oriSchedule;
			const currentSchedule = parm.currentSchedule;
			const auraData = getValue(auraSchedule) || '';
			const auraDataArr = auraData.split('<') || [];
			let ledDataArr = [];
			if (isSupport('LED_SCHED')) {
				const ledData = decodeURIComponent(httpApi.nvramCharToAscii(["led_sched"], true)["led_sched"]);
				ledDataArr = ledData ? ledData.split('<') : [];
			}
			const auraNightData = ExternalDeps.scheduleHandleData.json_array_to_string(auraNightSchedule.Get_Value());
			const conflictsArr = findLedScheConflicts(currentSchedule, auraDataArr);
			const conflictsLedArr = findLedScheConflicts(currentSchedule, ledDataArr);
			let auraDataArrClone;
			let ledDataArrClone;
			if (conflictsArr.length > 0 || conflictsLedArr.length > 0) {
				let hint = `<#Schedule_Conflicts#> <#Schedule_System_Adjust#>:<br>`;
				if (conflictsArr.length > 0) {
					hint += `
						<div style="display:flex; gap:6px;">
							<div class="icon-delete-circle" aria-hidden="true"></div>
							<div>
								<#Schedule_Cancel_AURA#> : `;
								auraDataArrClone = $.extend(true, [], auraDataArr);
								conflictsArr.forEach(conflictTime => {
									auraDataArrClone = auraDataArrClone.map(timeString => {
										if (timeString === conflictTime) {
											return timeString.slice(0, 1) + '0' + timeString.slice(2);
										}
										return timeString;
									});
									hint += `<br>${parseTimeString(conflictTime)}`;
								});
					hint += `
							</div>
						</div>`;
				}
				if (conflictsLedArr.length > 0) {
					if (conflictsArr.length > 0) hint += `<div style="height: 6px;"></div>`;
					hint += `
						<div style="display:flex; gap:6px;">
							<div class="icon-delete-circle" aria-hidden="true"></div>
							<div>
								<#Schedule_Cancel_LED#> : `;
								ledDataArrClone = $.extend(true, [], ledDataArr);
								conflictsLedArr.forEach(conflictTime => {
									ledDataArrClone = ledDataArrClone.map(timeString => {
										if (timeString === conflictTime) {
											return timeString.slice(0, 1) + '0' + timeString.slice(2);
										}
										return timeString;
									});
									hint += `<br>${parseTimeString(conflictTime)}`;
								});
					hint += `
							</div>
						</div>`;
				}

				const hintColor = isSupport("UI4") ? "#006CE1;" : theme === 'WHITE' ? "#006CE1;" : "#FC0;";
				hint += `<div style="height: 12px;"></div>
					<div style="display:flex; gap:6px;">
						<div class="icon-check-circle" aria-hidden="true"></div>
						<div>
							<span style='color:${hintColor}'><#Schedule_Set_Night#> : <br>${parseTimeString(currentSchedule)}</span>
						</div>
					</div>`;
				show_customize_confirm(hint);
				const $confirm_obj = $(popupMsgCntr).find(".popup_container.popup_customize_alert");
				const okButton = $confirm_obj.find("[data-btn=ok]")[0];
				if (okButton) {
					okButton.addEventListener('click', function(e) {
						let schedConfig = {
							"aura_night_sched": auraNightData
						};
						if (auraDataArrClone) {
							const auraNewData = auraDataArrClone.join('<');
							updateScheduleUI({
								type: "aura",
								timeString: auraNewData,
								scheduleInstance: auraSchedule
							});
							schedConfig["aura_sched"] = auraNewData;
						}
						if (ledDataArrClone) {
							const ledNewData = ledDataArrClone.join('<');
							schedConfig["led_sched"] = ledNewData;
						}
						saveConfig(schedConfig);
					});
				}
				const cancelButton = $confirm_obj.find("[data-btn=cancel]")[0];
				if (cancelButton) {
					cancelButton.addEventListener('click', function(e) {
						updateScheduleUI({
							type: "auraNight",
							timeString: oriSchedule,
							scheduleInstance: auraNightSchedule
						});
					});
				}
			}
			else {
				let schedConfig = {
					"aura_night_sched": auraNightData
				};
				saveConfig(schedConfig);
			}
		}
	}

	_loadResources(scriptSrcs, styleSrcs, callback) {
		let loadedScripts = 0;
		let loadedStyles = 0;
		const totalResources = scriptSrcs.length + styleSrcs.length;
		const onResourceLoaded = () => {
			if (++loadedScripts + loadedStyles === totalResources) {
				callback();
			}
		};
		scriptSrcs.forEach(src => {
			const script = document.createElement('script');
			script.src = src;
			script.onload = onResourceLoaded;
			script.onerror = onResourceLoaded;
			this.shadowRoot.appendChild(script);
		});
		styleSrcs.forEach(src => {
			const link = document.createElement('link');
			link.rel = 'stylesheet';
			link.href = src;
			link.onload = onResourceLoaded;
			link.onerror = onResourceLoaded;
			this.shadowRoot.appendChild(link);
		});
	}

	_parseTimeString(timeString) {
		if (typeof timeString !== 'string' || timeString.length !== 12) {
			return '';
		}
		const startTime = timeString.slice(4, 8);
		const endTime = timeString.slice(8, 12);
		const weekday = timeString.slice(2, 4);
		const formattedStartTime = formatTime(startTime);
		const formattedEndTime = formatTime(endTime);
		const days = getWeekdays(weekday);
		return `${days.join(' / ')} ${formattedStartTime} - ${formattedEndTime}`;

		function formatTime(time) {
			const hours = parseInt(time.slice(0, 2), 10);
			const minutes = time.slice(2, 4);
			return `${hours}:${minutes}`;
		}
		function getWeekdays(weekday) {
			const weekdayMapping = [
				{"bitwise":0, "text":`<#date_Sun_itemdesc#>`},
				{"bitwise":1, "text":`<#date_Mon_itemdesc#>`},
				{"bitwise":2, "text":`<#date_Tue_itemdesc#>`},
				{"bitwise":3, "text":`<#date_Wed_itemdesc#>`},
				{"bitwise":4, "text":`<#date_Thu_itemdesc#>`},
				{"bitwise":5, "text":`<#date_Fri_itemdesc#>`},
				{"bitwise":6, "text":`<#date_Sat_itemdesc#>`}
			];
			const bitwise = parseInt(weekday, 16)
			const binaryMask = bitwise.toString(2).padStart(7, '0');
			const weekdays = [];

			if (bitwise === 127) weekdays.push(`<#weekSche_Everyday#>`);
			else if (bitwise === 62) weekdays.push(`<#weekSche_Weekdays#>`);
			else if (bitwise === 65) weekdays.push(`<#weekSche_Weekend#>`);
			else {
				for (let i = binaryMask.length - 1; i >= 0; i--) {
					const bit = binaryMask[i];
					const weekdayIndex = binaryMask.length - 1 - i;
					if (bit === '1') {
						weekdays.push(weekdayMapping[weekdayIndex].text);
					}
				}
			}
			return weekdays;
		}
	}

	_findLedScheConflicts(currentSchedule, compareSchedule) {
		if (typeof currentSchedule !== 'string' || !Array.isArray(compareSchedule)) {
			return [];
		}
		const type1 = currentSchedule[0];
		const enable1 = parseInt(currentSchedule[1], 10);
		const weekday1 = parseInt(currentSchedule.slice(2, 4), 16);
		const startTime1 = parseTime(currentSchedule.slice(4, 8));
		const endTime1 = parseTime(currentSchedule.slice(8, 12));
		if (enable1 === 0) {
			return [];
		}
		const currentRanges = expandScheduleToDayRanges(weekday1, startTime1, endTime1);
		const conflicts = [];
		for (const schedule of compareSchedule) {
			if (typeof schedule !== 'string' || schedule.length < 12) {
				continue;
			}
			const type2 = schedule[0];
			const enable2 = parseInt(schedule[1], 10);
			const weekday2 = parseInt(schedule.slice(2, 4), 16);
			const startTime2 = parseTime(schedule.slice(4, 8));
			const endTime2 = parseTime(schedule.slice(8, 12));
			if (type1 !== type2 || enable2 === 0) {
				continue;
			}
			const compareRanges = expandScheduleToDayRanges(weekday2, startTime2, endTime2);
			for (const currentRange of currentRanges) {
				for (const compareRange of compareRanges) {
					if (
						currentRange.day === compareRange.day &&
						currentRange.start < compareRange.end &&
						compareRange.start < currentRange.end
					) {
						conflicts.push(schedule);
						break;
					}
				}
				if (conflicts.includes(schedule)) break;
			}

		}
		return conflicts;

		function expandScheduleToDayRanges(weekdayMask, start, end) {
			const ranges = [];
			for (let i = 0; i < 7; i++) {
				if ((weekdayMask & (1 << i)) === 0) continue;
				if (start < end) {
					ranges.push({ day: i, start, end });
				} else {
					ranges.push({ day: i, start, end: 1440 });//1440 = 24H*60M
					ranges.push({ day: (i + 1) % 7, start: 0, end });
				}
			}
			return ranges;
		}
		function parseTime(time) {
			const hours = parseInt(time.slice(0, 2), 10);
			const minutes = parseInt(time.slice(2, 4), 10);
			return hours * 60 + minutes;
		}
	}

	_getValue(scheduleObj) {
		if (!scheduleObj) {
			return '';
		}
		return ExternalDeps.scheduleHandleData.json_array_to_string(scheduleObj.Get_Value());
	}

	_updateScheduleUI(parm = {}) {
		const {
			timeString = '',
			type = '',
			scheduleInstance = ''
		} = parm;
		if (!type || !scheduleInstance) return;

		const $cntrSchedule = $(this.shadowRoot.querySelector(`#container_${type}_schedule`));
		scheduleInstance.Update_Data(timeString);
		$cntrSchedule.empty().append(scheduleInstance.Get_UI());
	}

	_show_customize_confirm(text, callback) {
		const $popupMsgCntr = $(this.shadowRoot.querySelector('#popupMsgCntr'));
		$popupMsgCntr.find(".container, .popup_container.popup_element, .popup_container.popup_element_second").addClass("blur_effect");
		const $popupCustomizeAlertContainer = $popupMsgCntr.find(".popup_container.popup_customize_alert").css("display", "flex").empty();
		const $hiddenMaskPopupCustomizeAlert = $popupMsgCntr.find(".hidden_mask.popup_customize_alert").css("display", "flex").empty();
		$popupCustomizeAlertContainer.append(ExternalDeps.getComponentCustomizeConfirm(text, this));
		if (typeof callback === "function") callback($popupMsgCntr);
	}

	_show_customize_alert(text, callback) {
		const $popupMsgCntr = $(this.shadowRoot.querySelector('#popupMsgCntr'));
		$popupMsgCntr.find(".container, .popup_container.popup_element, .popup_container.popup_element_second").addClass("blur_effect");
		const $popupCustomizeAlertContainer = $popupMsgCntr.find(".popup_container.popup_customize_alert").css("display", "flex").empty();
		const $hiddenMaskPopupCustomizeAlert = $popupMsgCntr.find(".hidden_mask.popup_customize_alert").css("display", "flex").empty();
		$popupCustomizeAlertContainer.append(ExternalDeps.getComponentCustomizeAlert(text, this));
		if (typeof callback === "function") callback($popupMsgCntr);
	}

	_close_popup_customize_alert() {
		const $popupMsgCntr = $(this.shadowRoot.querySelector('#popupMsgCntr'));
		$popupMsgCntr.find(".popup_customize_alert").hide().empty();
		const $popupElements = $popupMsgCntr.find(".popup_container.popup_element, .popup_container.popup_element_second");
		const isNoneDisplay = $popupElements.filter(":visible").length === 0;
		if (isNoneDisplay) {
			$popupMsgCntr.find(".container, .popup_container.popup_element, .popup_container.popup_element_second").removeClass("blur_effect");
		}
		if ($popupMsgCntr.find(".popup_container.popup_element_second:visible").length === 0 || $popupMsgCntr.find(".popup_container.popup_element_second").children().length === 0) {
			$popupMsgCntr.find(".popup_container.popup_element_second").removeClass("blur_effect");
		}
		if ($popupMsgCntr.find(".popup_container.popup_element:visible").length === 0) {
			$popupMsgCntr.find(".popup_container.popup_element").removeClass("blur_effect");
		}
	}

	_handleExpandBtn() {
		const $ledScheduleCntr = $(this.shadowRoot.querySelector('#ledScheduleCntr'));
		const $allExpandBtn = $ledScheduleCntr.find('[data-component="iconExpandCollapse"]');
		$allExpandBtn.each(function() {
			const $header = $(this).closest('.schedule_header_container');
			if ($header.is(':visible')) {
				$(this).removeClass('expand');
				$(this).closest('.schedule_ui').find('[view_mode="list"]').hide();
			}
		});
		const $targetExpand = $ledScheduleCntr.find(`#container_${this.schedMode}_schedule [data-component="iconExpandCollapse"]`);
		if ($targetExpand.length) {
			$targetExpand.addClass('expand')
				.closest('.schedule_ui').find('[view_mode="list"]').show();
		}
	}

	_initThemeObserver() {
		const htmlElement = document.documentElement;
		const observer = new MutationObserver((mutations) => {
			mutations.forEach((mutation) => {
				if (mutation.type === 'attributes' &&
					(mutation.attributeName === 'data-asuswrt-color' ||
					 mutation.attributeName === 'data-asuswrt-theme' ||
					 mutation.attributeName === 'data-asuswrt-style')) {
					this._updateThemeAttributes();
				}
			});
		});

		observer.observe(htmlElement, {
			attributes: true,
			attributeFilter: ['data-asuswrt-color', 'data-asuswrt-theme', 'data-asuswrt-style']
		});

		this.themeObserver = observer;
	}

	_updateThemeAttributes() {
		const htmlElement = document.documentElement;
		this.asuswrtColor = htmlElement.getAttribute('data-asuswrt-color') || '';
		this.asuswrtTheme = htmlElement.getAttribute('data-asuswrt-theme') || '';
		this.asuswrtStyle = htmlElement.getAttribute('data-asuswrt-style') || '';

		this.element.setAttribute('data-asuswrt-color', this.asuswrtColor);
		this.element.setAttribute('data-asuswrt-theme', this.asuswrtTheme);
		this.element.setAttribute('data-asuswrt-style', this.asuswrtStyle);
	}
}

class LedScheduler extends AuraScheduler {
	constructor(params) {
		super(params);
		this.schedMode = this.params?.schedMode || 'led';
		this.supportLedNightMode = this.params?.supportLedNightMode || false;
	}
	_Get_Component_LedScheduler() {
		const theme = this.theme;
		const getValue = this._getValue.bind(this);
		const findLedScheConflicts = this._findLedScheConflicts.bind(this);
		const parseTimeString = this._parseTimeString.bind(this);
		const show_customize_confirm = this._show_customize_confirm.bind(this);
		const show_customize_alert = this._show_customize_alert.bind(this);
		const updateScheduleUI = this._updateScheduleUI.bind(this);
		const ledScheduleCntr = this.shadowRoot.querySelector('#ledScheduleCntr');
		const popupMsgCntr = this.shadowRoot.querySelector('#popupMsgCntr');
		const supportLedNightSched = (() => {
			return this.supportLedNightMode && !!isSupport("LED_NIGHT_SCHED");
		})();
		const MaxRule_LED_SCHED = isSupport('MaxRule_LED_SCHED') || 5;
		const nvramArr = supportLedNightSched
			? ['led_timesched', 'led_sched', 'led_night_timesched', 'led_night_sched']
			: ['led_timesched', 'led_sched'];
		const ledConfig = httpApi.nvramCharToAscii(nvramArr, true);
		this.led_timesched = decodeURIComponent(ledConfig['led_timesched']) || '0';
		this.led_sched = decodeURIComponent(ledConfig['led_sched']) || '';
		if (supportLedNightSched) {
			this.led_night_timesched = decodeURIComponent(ledConfig['led_night_timesched']) || '0';
			this.led_night_sched = decodeURIComponent(ledConfig['led_night_sched']) || '';
		}

		let $container = $("<div>").addClass("profile_setting").appendTo($(ledScheduleCntr));

		const ledSchedule = new ExternalDeps.schedule({
			data_max: MaxRule_LED_SCHED,
			data:ExternalDeps.scheduleHandleData.string_to_json_array(this.led_sched),
			hideTitle: true,
			alternate_days: true,
			btn_save_callback: saveLedConfig,
			icon_switch_callback: saveLedConfig,
			icon_trash_callback: saveLedConfig,
			show_customize_confirm: this._show_customize_confirm.bind(this),
			show_customize_alert: this._show_customize_alert.bind(this)
		});
		let ledNightSchedule = null;
		if (supportLedNightSched) {
			ledNightSchedule = new ExternalDeps.schedule({
				data_max: MaxRule_LED_SCHED,
				data:ExternalDeps.scheduleHandleData.string_to_json_array(this.led_night_sched),
				hideTitle: true,
				alternate_days: true,
				btn_save_callback: saveLedNightConfig,
				icon_switch_callback: saveLedNightConfig,
				icon_trash_callback: saveLedNightConfig,
				show_customize_confirm: this._show_customize_confirm.bind(this),
				show_customize_alert: this._show_customize_alert.bind(this)
			});
		}

		$("<div>")
			.html(`<#SDN_Sync_To_All_Node#>. <#Schedule_Unsupported_Nodes#>`)
			.addClass("item_hint").appendTo($container);/* untranslated */

		const ledScheParm = {"title":`<#Schedule_LED_Offline#>`, "type":"switch", "id":"ledSche", "set_value":this.led_timesched === '1' ? "on" : "off"};
		ExternalDeps.getComponentSwitch(ledScheParm).appendTo($container).find("#" + ledScheParm.id + "").click(function(e){
			e = e || event;
			e.stopPropagation();
			const $controlContainer = $(this).closest(".profile_setting").find("#container_led_schedule");
			const switchStatus = $(this).hasClass("on") ? '1': '0';
			if (switchStatus === '1') {
				$controlContainer.empty().append(ledSchedule.Get_UI()).show();
			}
			else {
				$controlContainer.hide();
			}
			httpApi.nvramSet({"action_mode": "apply", "rc_service": "no_service", "led_timesched": switchStatus}, ()=>{
				httpApi.nvramCharToAscii(["led_timesched"], true);
				this.led_timesched = switchStatus;
			});
		});
		$("<div>").addClass("profile_setting_item schedule_ui")
			.attr({"id":"container_led_schedule"})
			.append(ledSchedule.Get_UI()).hide().appendTo($container);
		if (this.led_timesched === '1') {
			$container.find("#container_led_schedule").show();
		}

		if (supportLedNightSched) {
			const ledNightScheParm = {"title":`<#BoostKey_Aura_NightMode#> <#weekSche_Schedule#>`, "type":"switch", "id":"ledNight_sche", "set_value": this.led_night_timesched === '1' ? "on" : "off"};
			ExternalDeps.getComponentSwitch(ledNightScheParm).appendTo($container).find("#" + ledNightScheParm.id + "").click(function(e){
				e = e || event;
				e.stopPropagation();
				const $controlContainer = $(this).closest(".profile_setting").find("#container_ledNight_schedule");
				const switchStatus = $(this).hasClass("on") ? '1': '0';
				if (switchStatus === '1') {
					$controlContainer.empty().append(ledNightSchedule.Get_UI()).show();
				}
				else {
					$controlContainer.hide();
				}
				httpApi.nvramSet({"action_mode": "apply", "rc_service": "no_service", "led_night_timesched": switchStatus}, ()=>{
					httpApi.nvramCharToAscii(["led_night_timesched"], true);
					this.led_night_timesched = switchStatus;
				});
			});
			$("<div>").addClass("profile_setting_item schedule_ui")
				.attr({"id":"container_ledNight_schedule"})
				.append(ledNightSchedule.Get_UI()).hide().appendTo($container);
			if (this.led_night_timesched === '1') {
				$container.find("#container_ledNight_schedule").show();
			}
		}

		if (this.schedMode === 'led' && !this.led_sched) {
			const $ledSche = $(this.shadowRoot.querySelector('#ledSche'));
			if ($ledSche.length) {
				if (this.led_timesched === '0') {
					$ledSche.trigger('click');
				}
				const $addNew = $container.find("#container_led_schedule").find('.add_new_component');
				if ($addNew.length) {
					$addNew.trigger('click');
				}
			}
		}
		if (this.schedMode === 'ledNight' && !this.led_night_sched) {
			const $ledNight_sche = $(this.shadowRoot.querySelector('#ledNight_sche'));
			if ($ledNight_sche.length) {
				if (this.led_night_timesched === '0') {
					$ledNight_sche.trigger('click');
				}
				const $addNew = $container.find("#container_ledNight_schedule").find('.add_new_component');
				if ($addNew.length) {
					$addNew.trigger('click');
				}
			}
		}

		function saveLedConfig(parm) {
			const saveConfig = (schedConfig) => {
				const nvramSetObj = {...{"action_mode": "apply", "rc_service": "no_service"}, ...schedConfig};
				httpApi.nvramSet(nvramSetObj, () => {
					httpApi.nvramCharToAscii(["led_sched"], true);
					if ('led_sched' in schedConfig) {
						this.led_sched = schedConfig.led_sched;
					}
					if ('led_night_sched' in schedConfig) {
						this.led_night_sched = schedConfig.led_night_sched;
					}
				});
			};
			const oriSchedule = parm.oriSchedule;
			const currentSchedule = parm.currentSchedule;
			let ledNightDataArr = [];
			if (supportLedNightSched) {
				const ledNightData = getValue(ledNightSchedule) || '';
				ledNightDataArr = ledNightData ? ledNightData.split('<') : [];
			}
			let auraDataArr = [];
			if (isSupport('ledg') && isSupport('AURA_SCHED')) {
				const auraData = decodeURIComponent(httpApi.nvramCharToAscii(["aura_sched"], true)["aura_sched"]);
				auraDataArr = auraData ? auraData.split('<') : [];
			}
			let auraNightDataArr = [];
			if (isSupport('ledg') && isSupport('AURA_NIGHT_SCHED')) {
				const auraNightData = decodeURIComponent(httpApi.nvramCharToAscii(["aura_night_sched"], true)["aura_night_sched"]);
				auraNightDataArr = auraNightData ? auraNightData.split('<') : [];
			}
			const ledData = ExternalDeps.scheduleHandleData.json_array_to_string(ledSchedule.Get_Value());
			const conflictsArr = findLedScheConflicts(currentSchedule, ledNightDataArr);
			const conflictsAuraArr = findLedScheConflicts(currentSchedule, auraDataArr);
			const conflictsAuraNightArr = findLedScheConflicts(currentSchedule, auraNightDataArr);
			let ledNightDataArrClone;
			let auraDataArrClone;
			let auraNightDataArrClone;
			if (conflictsArr.length > 0 || conflictsAuraArr.length > 0 || conflictsAuraNightArr.length > 0) {
				let hint = `<#Schedule_Conflicts#> <#Schedule_System_Adjust#>:<br>`;
				if (conflictsArr.length > 0) {
					hint += `
						<div style="display:flex; gap:6px;">
							<div class="icon-delete-circle" aria-hidden="true"></div>
							<div>
								<#Schedule_Cancel_LED_Night#> : `;
								ledNightDataArrClone = $.extend(true, [], ledNightDataArr);
								conflictsArr.forEach(conflictTime => {
									ledNightDataArrClone = ledNightDataArrClone.map(timeString => {
										if (timeString === conflictTime) {
											return timeString.slice(0, 1) + '0' + timeString.slice(2);
										}
										return timeString;
									});
									hint += `<br>${parseTimeString(conflictTime)}`;
								});
					hint += `
							</div>
						</div>`;
				}
				if (conflictsAuraArr.length > 0) {
					if (conflictsArr.length > 0) hint += `<div style="height: 6px;"></div>`;
					hint += `
						<div style="display:flex; gap:6px;">
							<div class="icon-delete-circle" aria-hidden="true"></div>
							<div>
								<#Schedule_Cancel_AURA#> : `;
								auraDataArrClone = $.extend(true, [], auraDataArr);
								conflictsAuraArr.forEach(conflictTime => {
									auraDataArrClone = auraDataArrClone.map(timeString => {
										if (timeString === conflictTime) {
											return timeString.slice(0, 1) + '0' + timeString.slice(2);
										}
										return timeString;
									});
									hint += `<br>${parseTimeString(conflictTime)}`;
								});
					hint += `
							</div>
						</div>`;
				}
				if (conflictsAuraNightArr.length > 0) {
					if (conflictsArr.length > 0 || conflictsAuraArr.length > 0) hint += `<div style="height: 6px;"></div>`;
					hint += `
						<div style="display:flex; gap:6px;">
							<div class="icon-delete-circle" aria-hidden="true"></div>
							<div>
								<#Schedule_Cancel_AURA_Night#> : `;
								auraNightDataArrClone = $.extend(true, [], auraNightDataArr);
								conflictsAuraNightArr.forEach(conflictTime => {
									auraNightDataArrClone = auraNightDataArrClone.map(timeString => {
										if (timeString === conflictTime) {
											return timeString.slice(0, 1) + '0' + timeString.slice(2);
										}
										return timeString;
									});
									hint += `<br>${parseTimeString(conflictTime)}`;
								});
					hint += `
							</div>
						</div>`;
				}

				const hintColor = isSupport("UI4") ? "#006CE1;" : theme === 'WHITE' ? "#006CE1;" : "#FC0;";
				hint += `<div style="height: 12px;"></div>
					<div style="display:flex; gap:6px;">
						<div class="icon-check-circle" aria-hidden="true"></div>
						<div>
							<span style='color:${hintColor}'><#Schedule_Set_LED#> : <br>${parseTimeString(currentSchedule)}</span>
						</div>
					</div>`;
				show_customize_confirm(hint);
				const $confirm_obj = $(popupMsgCntr).find(".popup_container.popup_customize_alert");
				const okButton = $confirm_obj.find("[data-btn=ok]")[0];
				if (okButton) {
					okButton.addEventListener('click', function(e) {
						let schedConfig = {
							"led_sched": ledData
						};
						if (ledNightDataArrClone) {
							const ledNightNewData = ledNightDataArrClone.join('<');
							updateScheduleUI({
								type: "ledNight",
								timeString: ledNightNewData,
								scheduleInstance: ledNightSchedule
							});
							if (supportLedNightSched) {
								schedConfig["led_night_sched"] = ledNightNewData;
							}
						}
						if (auraDataArrClone) {
							const auraNewData = auraDataArrClone.join('<');
							schedConfig["aura_sched"] = auraNewData;
						}
						if (auraNightDataArrClone) {
							const auraNightNewData = auraNightDataArrClone.join('<');
							schedConfig["aura_night_sched"] = auraNightNewData;
						}
						saveConfig(schedConfig);
					});
				}
				const cancelButton = $confirm_obj.find("[data-btn=cancel]")[0];
				if (cancelButton) {
					cancelButton.addEventListener('click', function(e) {
						updateScheduleUI({
							type: "led",
							timeString: oriSchedule,
							scheduleInstance: ledSchedule
						});
					});
				}
			}
			else {
				let schedConfig = {
					"led_sched": ledData
				};
				saveConfig(schedConfig);
			}
		}
		function saveLedNightConfig(parm) {
			const saveConfig = (schedConfig) => {
				const nvramSetObj = {...{"action_mode": "apply", "rc_service": "no_service"}, ...schedConfig};
				httpApi.nvramSet(nvramSetObj, () => {
					httpApi.nvramCharToAscii(["led_sched"], true);
					if ('led_sched' in schedConfig) {
						this.led_sched = schedConfig.led_sched;
					}
					if ('led_night_sched' in schedConfig) {
						this.led_night_sched = schedConfig.led_night_sched;
					}
				});
			};
			const oriSchedule = parm.oriSchedule;
			const currentSchedule = parm.currentSchedule;
			const ledData = getValue(ledSchedule) || '';
			const ledDataArr = ledData.split('<') || [];
			const ledNightData = ExternalDeps.scheduleHandleData.json_array_to_string(ledNightSchedule.Get_Value());
			const conflictsArr = findLedScheConflicts(currentSchedule, ledDataArr);
			if (conflictsArr.length > 0) {
				let hint = `<#Schedule_Conflicts#> <#Schedule_System_Adjust#>:<br>`;
				hint += `
					<div style="display:flex; gap:6px;">
						<div class="icon-delete-circle" aria-hidden="true"></div>
						<div>
							<#Schedule_Cancel_LED#> :`;
							let ledDataArrClone = $.extend(true, [], ledDataArr);
							conflictsArr.forEach(conflictTime => {
								ledDataArrClone = ledDataArrClone.map(timeString => {
									if (timeString === conflictTime) {
										return timeString.slice(0, 1) + '0' + timeString.slice(2);
									}
									return timeString;
								});
								hint += `<br>${parseTimeString(conflictTime)}`;
							});
				hint += `
						</div>
					</div>`;
				const hintColor = theme === 'WHITE' ? "#006CE1;" : "#FC0;";
				hint += `<div style="height: 12px;"></div>
					<div style="display:flex; gap:6px;">
						<div class="icon-check-circle" aria-hidden="true"></div>
						<div>
							<span style='color:${hintColor}'><#Schedule_Set_Night#> : <br>${parseTimeString(currentSchedule)}</span>
						</div>
					</div>`;
				show_customize_confirm(hint);
				const $confirm_obj = $(popupMsgCntr).find(".popup_container.popup_customize_alert");
				const okButton = $confirm_obj.find("[data-btn=ok]")[0];
				if (okButton) {
					okButton.addEventListener('click', function(e) {
						const ledNewData = ledDataArrClone.join('<');
						updateScheduleUI({
							type: "led",
							timeString: ledNewData,
							scheduleInstance: ledSchedule
						});
						let schedConfig = {
							"led_sched": ledNewData,
							"led_night_sched": ledNightData
						};
						schedConfig["led_night_sched"] = ledNightData;
						saveConfig(schedConfig);
					});
				}
				const cancelButton = $confirm_obj.find("[data-btn=cancel]")[0];
				if (cancelButton) {
					cancelButton.addEventListener('click', function(e) {
						updateScheduleUI({
							type: "ledNight",
							timeString: oriSchedule,
							scheduleInstance: ledNightSchedule
						});
					});
				}
			}
			else {
				let schedConfig = {
					"led_night_sched": ledNightData
				};
				saveConfig(schedConfig);
			}
		}
	}
}

const ASUS_LEDSCHEDUER = {
	popupDivStyle: `<style>
		:root {
			--color-primary: #006CE1;
			--primary-blue-5000: #248DFF;
			--tuf-color-primary: #FFAA32;
			--rog-color-primary: #FF3535;
		}

		.popup_bg {
			font-family: Roboto-Regular, "Microsoft JhengHei";
			position: fixed;
			top: 0;
			right: 0;
			bottom: 0;
			left: 0;
			z-index: 2000;
			background: rgba(7, 7, 7, 0.54);
			backdrop-filter: blur(6px);
			-webkit-backdrop-filter: blur(6px);
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

		.modal.show .modal-dialog {
			transform: none;
		}
		.modal.fade .modal-dialog {
			transition: transform .3s ease-out;
			transform: translate(0,-50px);
		}

		.modal .close_btn {
			--close-btn-svg:url("data:image/svg+xml,%3Csvg width='28' height='28' viewBox='0 0 28 28' fill='none' xmlns='http://www.w3.org/2000/svg'%3E%3Cpath fill-rule='evenodd' clip-rule='evenodd' d='M14 2.333A11.656 11.656 0 0 0 2.333 14 11.656 11.656 0 0 0 14 25.667 11.656 11.656 0 0 0 25.667 14 11.656 11.656 0 0 0 14 2.333zm5.833 15.855l-1.645 1.645L14 15.645l-4.188 4.188-1.645-1.645L12.355 14 8.167 9.812l1.645-1.645L14 12.355l4.188-4.188 1.645 1.645L15.645 14l4.188 4.188z' fill='%23818181'/%3E%3C/svg%3E");
			width: 28px;
			height: 28px;
			mask-image: var(--close-btn-svg);
			mask-repeat: no-repeat;
			mask-position: center;
			mask-size: contain;
			-webkit-mask-image: var(--close-btn-svg);
			-webkit-mask-repeat: no-repeat;
			-webkit-mask-position: center;
			-webkit-mask-size: contain;
			background-color: var(--icon-color);
			cursor: pointer;
		}

		.modal-dialog.is_Web_iframe {
			position: relative;
			width: 60vw;
			min-width: 400px;
			max-width: 800px;
			margin: 4rem auto;
			pointer-events: none;
		}
		.modal-dialog {
			position: relative;
			width: 90%;
			min-width: 400px;
			max-width: 800px;
			margin: 0px auto;
			pointer-events: none;
		}
		.modal-dialog.isMobile {
			width: 100%;
			min-width: 310px;
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
			background-color: transparent;
			border: transparent;
			border-radius: 0.3rem;
			outline: 0;
			padding: 5px;
			margin-bottom: 100px;
		}
		.isMobile .modal-content {
			padding: 0px;
		}

		.modal-header {
			display: -webkit-box;
			display: -ms-flexbox;
			display: flex;
			-webkit-box-align: start;
			-ms-flex-align: start;
			align-items: center;
			-webkit-box-pack: justify;
			-ms-flex-pack: justify;
			justify-content: space-between;
			padding: 1rem;
			border: transparent;
			border-top-left-radius: 0.3rem;
			border-top-right-radius: 0.3rem;
		}

		.modal-header .close {
			padding: 1rem;
			margin: -1rem -1rem -1rem auto;
		}

		.modal-title {
			color: #000000;
			font-weight: bold;
			font-size: 16px;
			margin: 0;
			display: flex;
			align-items: center;
		}

		.modal-body {
			color: #000000;
			background: #FFFFFF;
			box-shadow: 0px 2px 2px 0px rgba(0, 0, 0, 0.04), 0px 1px 5px 0px rgba(0, 0, 0, 0.08), 0px 3px 1px 0px rgba(0, 0, 0, 0.06);
			border-radius: 10px;
			position: relative;
			-webkit-box-flex: 1;
			-ms-flex: 1 1 auto;
			flex: 1 1 auto;
		}
		:root, :host([data-asuswrt-color="light"]) {
			--text-default: #181818
		}
		:host([data-asuswrt-color="light"]) .popup_bg {
			background: color-mix(in srgb, var(--body-bg-color) 60%, transparent);
		}
		:host([data-asuswrt-color="light"]) .modal-title {
			color: var(--page-title-color) !important;
		}
		:host([data-asuswrt-color="light"]) .modal-body {
			background: rgba(255, 255, 255);
		}
		:host([data-asuswrt-color="light"]) .modal .close_btn {
			mask-image: var(--ic-close-circle-fill);
			background: var(--icon-normal-color) !important;
		}
		:host([data-asuswrt-color="light"]) .help_icon {
			background: var(--icon-normal-color) !important;
		}
		#aura_help_icon_cntr {
			margin-left: 10px;
		}
		.help_icon{
			background-image: none;
			--svg: url("data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMjQiIGhlaWdodD0iMjQiIHZpZXdCb3g9IjAgMCAyNCAyNCIgZmlsbD0ibm9uZSIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj4KPHBhdGggZmlsbC1ydWxlPSJldmVub2RkIiBjbGlwLXJ1bGU9ImV2ZW5vZGQiIGQ9Ik0xMi4yNSAyMS41QzcuMTQxMzcgMjEuNSAzIDE3LjM1ODYgMyAxMi4yNUMzIDcuMTQxMzcgNy4xNDEzNyAzIDEyLjI1IDNDMTcuMzU4NiAzIDIxLjUgNy4xNDEzNyAyMS41IDEyLjI1QzIxLjUgMTcuMzU4NiAxNy4zNTg2IDIxLjUgMTIuMjUgMjEuNVoiIHN0cm9rZT0iI0I5QzVENSIgc3Ryb2tlLXdpZHRoPSIxLjIiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIvPgo8bGluZSB4MT0iMTIuMzUiIHkxPSIxNi44NSIgeDI9IjEyLjM1IiB5Mj0iMTYuNjUiIHN0cm9rZT0iI0I5QzVENSIgc3Ryb2tlLXdpZHRoPSIxLjIiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIvPgo8cGF0aCBkPSJNOS4xNSAxMC4xMzI0QzkuMTUgMTAuNDYzNyA5LjQxODYzIDEwLjczMjQgOS43NSAxMC43MzI0QzEwLjA4MTQgMTAuNzMyNCAxMC4zNSAxMC40NjM3IDEwLjM1IDEwLjEzMjRIOS4xNVpNMTMuMjA4NCAxMi4wNzUzTDEyLjg2NzIgMTEuNTgxN0wxMi44NjcyIDExLjU4MTdMMTMuMjA4NCAxMi4wNzUzWk0xMS42NSAxNC4yNUMxMS42NSAxNC41ODE0IDExLjkxODYgMTQuODUgMTIuMjUgMTQuODVDMTIuNTgxNCAxNC44NSAxMi44NSAxNC41ODE0IDEyLjg1IDE0LjI1SDExLjY1Wk0xMC4zNSAxMC4xMzI0QzEwLjM1IDkuMjE2MiAxMC42MDczIDguNjYzNzcgMTAuOTI2NSA4LjM0MTEyQzExLjI0NzYgOC4wMTY1NiAxMS43MDUgNy44NSAxMi4yNSA3Ljg1VjYuNjVDMTEuNDYxNyA2LjY1IDEwLjY2OSA2Ljg5NTIgMTAuMDczNSA3LjQ5NzEyQzkuNDc2MDYgOC4xMDA5MyA5LjE1IDguOTg5NjggOS4xNSAxMC4xMzI0SDEwLjM1Wk0xMi4yNSA3Ljg1QzEyLjc5NzQgNy44NSAxMy4yNzA1IDguMDE4OCAxMy41OTkgOC4zMTM5OEMxMy45MTc3IDguNjAwMjcgMTQuMTUgOS4wNDkwNCAxNC4xNSA5LjcyMDU5SDE1LjM1QzE1LjM1IDguNzQ1MDggMTQuOTk5IDcuOTU4NTYgMTQuNDAxIDcuNDIxMzFDMTMuODEyOSA2Ljg5Mjk2IDEzLjAzNiA2LjY1IDEyLjI1IDYuNjVWNy44NVpNMTQuMTUgOS43MjA1OUMxNC4xNSAxMC4zMzc0IDEzLjY2NzcgMTEuMDI4NCAxMi44NjcyIDExLjU4MTdMMTMuNTQ5NiAxMi41Njg4QzE0LjQ2OSAxMS45MzMzIDE1LjM1IDEwLjkyODIgMTUuMzUgOS43MjA1OUgxNC4xNVpNMTIuODUgMTQuMjVWMTMuNjg5SDExLjY1VjE0LjI1SDEyLjg1Wk0xMi44NjcyIDExLjU4MTdDMTIuMjE5MyAxMi4wMjk2IDExLjY1IDEyLjc2OTUgMTEuNjUgMTMuNjg5SDEyLjg1QzEyLjg1IDEzLjI5MDggMTMuMTA0OCAxMi44NzYyIDEzLjU0OTYgMTIuNTY4OEwxMi44NjcyIDExLjU4MTdaIiBmaWxsPSIjQjlDNUQ1Ii8+Cjwvc3ZnPgo=");
			-webkit-mask: var(--svg);
			mask: var(--svg);
			-webkit-mask-repeat: no-repeat;
			-webkit-mask-position: center;
			-webkit-mask-size: contain;
			display: block;
			background-color: var(--icon-color);
			width: 24px;
			height: 24px;
			margin-right: 2%;
			cursor: pointer;
			padding: initial;
		}
		</style>`,

	ModalTheme: {
		RT: `<style>
			.modal-title, .modal-body {
				color: #FFFFFF;
			}
			.modal-body {
				color: #FFFFFF;
				background: #444F53;
			}
			:root,
			:host{
				--icon-color: #B9C5D4;
			}
		</style>`,

		ROG: `<style>
			.popup_bg {
				background: rgba(0, 0, 0, 0.5);
				backdrop-filter: blur(4px);
				-webkit-backdrop-filter: blur(6px);
			}
			.modal-body {
				background-color: #1C1C1E;
				border: 0;
				border-radius: 0;
			}
			.modal-title {
				color: #FFF;
			}
			:root,
			:host{
				--icon-color: #B9C5D4;
			}
		</style>`,

		TUF: `<style>
			.popup_bg {
				background: rgba(0, 0, 0, 0.5);
				backdrop-filter: blur(4px);
				-webkit-backdrop-filter: blur(6px);
			}
			.modal-body {
				background-color: #1C1C1E;
				border: 0;
				border-radius: 0;
			}
			.modal-title {
				color: #FFF;
			}
			:root,
			:host{
				--icon-color: #B9C5D4;
			}
		</style>`,

		WHITE: `<style>
			:root,
			:host{
				--icon-color: #4D4D4D;
			}
			.popup_bg {
				background: rgba(255, 255, 255);
			}
		</style>`
	},
}

class popupLedScheduler extends HTMLElement {
	constructor(params) {
		super();
		this.params = params || {};
		const theme = this.params?.theme || 'RT';
		const popupTitle = params?.popupTitle || `<weekSche_Schedule>`;
		const ledClassType = this.params?.ledClassType || '';
		const isMobile =  this.params?.isMobile || false;

		const htmlElement = document.documentElement;
		this.asuswrtColor = htmlElement.getAttribute('data-asuswrt-color') || '';
		this.asuswrtTheme = htmlElement.getAttribute('data-asuswrt-theme') || '';
		this.asuswrtStyle = htmlElement.getAttribute('data-asuswrt-style') || '';

		this.setAttribute('data-asuswrt-color', this.asuswrtColor);
		this.setAttribute('data-asuswrt-theme', this.asuswrtTheme);
		this.setAttribute('data-asuswrt-style', this.asuswrtStyle);

		this.attachShadow({mode: 'open'});
		let modalTheme = '';
		modalTheme = ASUS_LEDSCHEDUER.ModalTheme[theme.toUpperCase()];
		const template = document.createElement('template');
		template.innerHTML = `
			${ASUS_LEDSCHEDUER.popupDivStyle}
			<div class="popup_bg">
				<div class="modal">
					<div class="modal-dialog ${isMobile ? 'isMobile' : ''} modal-xl">
						<div class="modal-content">
							<div class="modal-header">
								<div class="modal-title">
								${popupTitle}
								<span id="aura_help_icon_cntr"></span>
								</div>
								<div class="close_btn" data-bs-dismiss="modal" aria-label="Close"></div>
							</div>
							<div class="modal-body">
								<div id='popupLedScheduleCntr' class='container'></div>	
							</div>
						</div>
					</div>
				</div>
			</div>
			 ${modalTheme}
		`;
		this.shadowRoot.appendChild(template.content.cloneNode(true));
		this.shadowRoot.querySelector('.close_btn').addEventListener('click', this.handleModalClose.bind(this));
		let ledScheduler = null;
		if (ledClassType === 'aura') {
			ledScheduler = new AuraScheduler({...this.params, parentShadowRoot: this.shadowRoot});
		}
		else if (ledClassType === 'led') {
			ledScheduler = new LedScheduler(this.params);
		}
		const popupLedScheduleCntr = this.shadowRoot.querySelector('#popupLedScheduleCntr');
		if (ledScheduler) {
			$(popupLedScheduleCntr).empty().off().append(ledScheduler.render());
		} else {
			$(popupLedScheduleCntr).empty().off();
		}
		this.shadowRoot.querySelector("#aura_help_icon_cntr").appendChild(ledScheduler.getHelpIcon());
	}

	handleModalClose = () => {
		top.document.getElementById('ledscheduler_mode').remove();
		top.document.body.style.removeProperty('overflow');
		if (typeof top.dispatchEvent === "function") {
			top.dispatchEvent(new CustomEvent('ledSchedulerClosed', {
				bubbles: true,
				composed: true
			}));
		}
	}

	show = () => {
		if (top.document.getElementById('ledscheduler_mode') == null) {
			top.document.body.style.overflow = 'hidden';
			const modal = document.createElement('div');
			modal.id = 'ledscheduler_mode';
			top.document.body.appendChild(modal);
			modal.appendChild(this);
		}
	}
}

if (!customElements.get('ledscheduler-mode')) {
	customElements.define('ledscheduler-mode', popupLedScheduler);
}

function getLedgScheduleHint(options = {}) {
	const { type = "aura", text = `<#BoostKey_Aura_RGB#>`, switchStatus = "1" } = options;
	const inEffect = httpApi.nvramGet(["aura_off_sched_in_effect", "aura_night_sched_in_effect",
		"led_off_sched_in_effect", "led_night_sched_in_effect"], true);
	if (
		inEffect.aura_off_sched_in_effect === "1" ||
		inEffect.aura_night_sched_in_effect === "1" ||
		inEffect.led_off_sched_in_effect === "1" ||
		inEffect.led_night_sched_in_effect === "1"
	) {
		if (inEffect.aura_night_sched_in_effect === "1" && type === "aura_night" && switchStatus === "1") {
			return { show: false, hint: "" };
		}
		if (inEffect.aura_night_sched_in_effect !== "1" && type === "aura_night" && switchStatus === "0") {
			return { show: false, hint: "" };
		}
		if (inEffect.led_night_sched_in_effect === "1" && type === "led_night" && switchStatus === "1") {
			return { show: false, hint: "" };
		}
		if (inEffect.led_night_sched_in_effect !== "1" && type === "led_night" && switchStatus === "0") {
			return { show: false, hint: "" };
		}
		const next_sched = httpApi.hookGet("get_led_aura_next_sched", true) || {};
		if (!next_sched || typeof next_sched !== 'object' || Object.keys(next_sched).length === 0) {
			return { show: false, hint: "" };
		}

		const sched_type_map = {
			"led": `<#Schedule_LED_Offline#>`,
			"led_night": `<#AiMesh_System_LED_NM#>`,
			"aura": `<#Schedule_Aura_Offline#>`,
			"aura_night": `<#Schedule_Aura_Night#>`
		};
		const wday_map = {
			"0": `<#date_Sun_itemdesc#>`,
			"1": `<#date_Mon_itemdesc#>`,
			"2": `<#date_Tue_itemdesc#>`,
			"3": `<#date_Wed_itemdesc#>`,
			"4": `<#date_Thu_itemdesc#>`,
			"5": `<#date_Fri_itemdesc#>`,
			"6": `<#date_Sat_itemdesc#>`
		};
		const nextSched = `${sched_type_map[next_sched.sched_type]}, ${wday_map[next_sched.next_wday]}, ${next_sched.next_hour}:${next_sched.next_minute}`;
		let hint = switchStatus === "1" ? `<#weekSche_next_sche#>` : `%1$@ is temporarily off. The schedule will resume in the next cycle (%2$@).`;
		hint = hint.replace(/%1\$@/g, text).replace(/%2\$@/g, nextSched);
		return { show: true, hint: hint };
	}
	return { show: false, hint: "" };
}
