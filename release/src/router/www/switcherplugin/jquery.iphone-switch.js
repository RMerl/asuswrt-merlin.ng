jQuery.fn.iphoneSwitch = function (start_state, switched_on_callback, switched_off_callback, options) {
    var state = start_state == '1' ? start_state : '0';

    class ToggleButton {
        constructor(active = false, withText = false, options = {}) {
            this.active = active;
            this.withText = withText;
            this.options = {
                label: options.label || 'Toggle switch',
                ariaDescribedBy: options.ariaDescribedBy || null,
                ...options
            };

            this.element = document.createElement('div');
            const shadowRoot = this.element.attachShadow({mode: 'open'});
            this.shadowRoot = shadowRoot;

            const template = document.createElement('template');
            const toggleId = `toggle-${Math.random().toString(36).substr(2, 9)}`;

            template.innerHTML = `
            <style>
                .toggle-button {
                    width: 2.5rem;
                    height: 1rem;
                    display: flex;
                    padding: 4px;
                    border-radius: var(--radius-lg);
                    align-items: center;
                    justify-content: space-between;
                    border: 1.5px solid var(--switch-btn-off-normal-ellipse-stroke-color);
                    background-color: var(--switch-bg-color);
                    cursor: pointer;
                    transition: all 0.6s;
                    position: relative;
                    overflow: hidden;
                }
        
                .toggle-button.with-text::before {
                    font-family: "Roboto";
                    content: "OFF";
                    font-size: 12px;
                    font-weight: bold;
                    color: var(--switch-btn-off-normal-ellipse-stroke-color);
                    transition: all 0.6s;
                    position: absolute;
                    right: 4px;
                    top: 50%;
                    transform: translateY(-50%);
                }
        
                .toggle-button.with-text::after {
                    font-family: "Roboto";
                    content: "ON";
                    font-size: 12px;
                    font-weight: bold;
                    color: var(--switch-btn-on-normal-text-color);
                    transition: all 0.6s;
                    position: absolute;
                    left: 4px;
                    opacity: 0;
                    top: 50%;
                    transform: translateY(-50%);
                }
        
                .toggle-button.with-text.active::before {
                    opacity: 0;
                }
        
                .toggle-button.with-text.active::after {
                    opacity: 1;
                }
        
                .toggle-button.active {
                    background-color: var(--switch-btn-on-normal-fill-color);
                    border: 1.5px solid var(--switch-btn-on-normal-ellipse-stroke-color);
                }
        
                .toggle-button .toggle-button-handle {
                    width: 16px;
                    height: 16px;
                    border-radius: var(--radius-lg);
                    background-color: var(--switch-btn-off-normal-ellipse-stroke-color);
                    transition: all 0.6s;
                    position: absolute;
                    left: 4px;
                }
        
                .toggle-button.active .toggle-button-handle {
                    background-color: var(--switch-btn-on-normal-ellipse-fill-color);
                    transform: translateX(20px);
                }
        
                .toggle-button.with-text.active .toggle-button-handle {
                    background-color: var(--switch-btn-on-normal-ellipse-fill-color);
                    transform: translateX(26px);
                }
            </style>
            <div 
                id="${toggleId}"
                class="toggle-button ${withText ? 'with-text' : ''} ${active ? 'active' : ''}"
                role="switch"
                tabindex="0"
                aria-checked="${active}"
                aria-label="${this.options.label}"
                ${this.options.ariaDescribedBy ? `aria-describedby="${this.options.ariaDescribedBy}"` : ''}
                >
                <div class="toggle-button-handle" aria-hidden="true"></div>
            </div>
        `;
            shadowRoot.appendChild(template.content.cloneNode(true));

            this.toggleElement = shadowRoot.querySelector('.toggle-button');

            // 點擊事件
            this.toggleElement.addEventListener('click', () => {
                this.toggle();
            });

            // 鍵盤支援
            this.toggleElement.addEventListener('keydown', (e) => {
                if (e.key === 'Enter' || e.key === ' ') {
                    e.preventDefault();
                    this.toggle();
                }
            });
        }

        render() {
            return this.element;
        }

        toggle() {
            this.active = !this.active;
            this.toggleElement.classList.toggle('active');
            this.toggleElement.setAttribute('aria-checked', this.active.toString());

            // 觸發變更事件
            if (this.onChangeCallback) {
                this.onChangeCallback(this.active);
            }

            // 觸發自定義事件
            this.element.dispatchEvent(new CustomEvent('change', {
                detail: {active: this.active, value: this.active}
            }));
        }

        setOnChange(callback) {
            this.onChangeCallback = callback;
        }

        getValue() {
            return this.active;
        }

        enable() {
            this.active = true;
            this.toggleElement.classList.add('active');
            this.toggleElement.setAttribute('aria-checked', 'true');
        }

        disable() {
            this.active = false;
            this.toggleElement.classList.remove('active');
            this.toggleElement.setAttribute('aria-checked', 'false');
        }

        setDisabled(disabled) {
            if (disabled) {
                this.toggleElement.setAttribute('tabindex', '-1');
                this.toggleElement.setAttribute('aria-disabled', 'true');
                this.toggleElement.style.opacity = '0.5';
                this.toggleElement.style.cursor = 'not-allowed';
            } else {
                this.toggleElement.setAttribute('tabindex', '0');
                this.toggleElement.removeAttribute('aria-disabled');
                this.toggleElement.style.opacity = '';
                this.toggleElement.style.cursor = '';
            }
        }

        setLabel(label) {
            this.toggleElement.setAttribute('aria-label', label);
        }
    }

    if (parent.webWrapper) {
        // Use ToggleButton component from component.module.js
        return this.each(function () {
            const toggleButton = new ToggleButton(state == '1', true, {
                label: this.getAttribute('aria-label') || 'Toggle switch'
            });

            // Set up change callback
            toggleButton.setOnChange((isActive) => {
                const newState = isActive ? '1' : '0';
                if (isActive) {
                    switched_on_callback();
                } else {
                    switched_off_callback();
                }
            });

            // Replace the current element with the ToggleButton
            jQuery(this).html('');
            jQuery(this).append(toggleButton.render());
        });
    } else {

        // define default settings
        var settings = {
            mouse_over: 'pointer',
            mouse_out: 'default',
            switch_container_path: '/switcherplugin/iphone_switch_container_on.png',
            switch_path: '/switcherplugin/iphone_switch.png',
            switch_height: 32,
            switch_width: 74,
            borderSize: "7"
        };

        if (options) {
            jQuery.extend(settings, options);
        }

        // create the switch
        return this.each(function () {

            var container;
            var image;

            // make the container
            container = '<div class="iphone_switch_container" style="height:' + settings.switch_height + 'px; width:' + settings.switch_width + 'px; position: relative; overflow: hidden">';

            // make the switch image based on starting state
            image = '<img id="iphone_switch" class="iphone_switch" src="' + settings.switch_container_path + '" style="border-radius:' + settings.borderSize + 'px;height:' + settings.switch_height + 'px; width:' + settings.switch_width + 'px; background-image:url(' + settings.switch_path + '); background-repeat:no-repeat; background-position:' + (state == '1' ? 0 : -37) + 'px" /></div>';

            // insert into placeholder
            jQuery(this).html(container + image);

            jQuery(this).mouseover(function () {
                jQuery(this).css("cursor", settings.mouse_over);
            });

            jQuery(this).mouseout(function () {
                jQuery(this).css("background", settings.mouse_out);
            });

            // click handling
            jQuery(this).unbind("click"); // initial click event
            jQuery(this).click(function () {
                if ((this.id == "stream_ad_enable" || this.id == "pop_ad_enable" || //AiProtection_AdBlock
                    this.id == "radio_protection_enable" || this.id == "radio_mals_enable" || this.id == "radio_vp_enable" || this.id == "radio_cc_enable" || //AiProtection_HomeProtection
                    this.id == "radio_web_restrict_enable" ||  //AiProtection_AppProtector, AiProtection_WebProtector
                    this.id == "traffic_analysis_enable" ||  //TrafficAnalyzer_Statistic
                    this.id == "apps_analysis_enable" ||  //AdaptiveQoS_Bandwidth_Monitor
                    this.id == "bwdpi_wh_enable" ||  //AdaptiveQoS_WebHistory
                    this.id == "radio_clouddisk_enable" ||   //Cloud_main
                    this.id == "radio_wps_enable" ||  //Advanced_WWPS_Content
                    this.id == "nm_radio_dualwan_enable" ||  //Internet
                    this.id == "simdetect_switch" ||  //Advanced_MobileBroadband_Content
                    this.id == "dns_switch" ||   //Advanced_IPTV_Content
                    this.id == "radio_anonymous_enable" ||	//Advanced_AiDisk_ftp
                    this.id == "radio_fbwifi_enable" ||   //Guest_network_fbwifi
                    this.id == "vlan_enable" ||  //Advanced_TagBasedVLAN_Content
                    this.id == "ad_radio_dualwan_enable" ||
                    this.id == "tencent_qmacc_enable" ||
                    this.id == "radio_service_enable" ||
                    this.id == "radio_IG_enable") && typeof (curState) != "undefined") {
                    state = curState;
                } else if (this.id.length > 18 && this.id.substr(0, 18) == "wtfast_rule_enable") {
                    var index = (this.id).substr(18);
                    var index_int = parseInt(index);
                    state = rule_enable_array[index_int];
                } else if (this.id.substr(0, 16) == "vlan_rule_enable") {
                    var index = (this.id).substr(16);
                }

                if ((this.id == "wandhcp_switch") && typeof (curWandhcpState))
                    state = curWandhcpState;

                if (state == '1') {
                    jQuery(this).find('.iphone_switch').animate({backgroundPosition: -37}, "slow", function () {
                        jQuery(this).attr('src', settings.switch_container_path);
                        if (typeof (index))
                            switched_off_callback(index);
                        else
                            switched_off_callback();
                    });
                    state = '0';
                } else {
                    jQuery(this).find('.iphone_switch').animate({backgroundPosition: 0}, "slow", function () {
                        jQuery(this).find('.iphone_switch').attr('src', settings.switch_container_path);
                        if (typeof (index))
                            switched_on_callback(index);
                        else
                            switched_on_callback();
                    });
                    state = '1';
                }
            });

        });
    }

};
