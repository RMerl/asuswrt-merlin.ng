const ASUS_PINCODE = {

    PincodeDivStyle: `<style>
        .popup_bg {
            position: fixed;
            top: 0;
            right: 0;
            bottom: 0;
            left: 0;
            z-index: 2000;
            backdrop-filter: blur(2px);
            -webkit-backdrop-filter: blur(2px);
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
    
        .modal-header {
            display: -webkit-box;
            display: -ms-flexbox;
            display: flex;
            -webkit-box-align: start;
            -ms-flex-align: start;
            align-items: flex-start;
            -webkit-box-pack: justify;
            -ms-flex-pack: justify;
            justify-content: space-between;
            padding: 1rem;
            border-top-left-radius: 0.3rem;
            border-top-right-radius: 0.3rem;
        }
    
        .modal-header .close {
            padding: 1rem;
            margin: -1rem -1rem -1rem auto;
        }
    
        .modal-title {
            color: #006ce1;
            font-weight: bold;
            font-size: 2em;
            margin: 0;
        }
    
        .modal-body {
            position: relative;
            -webkit-box-flex: 1;
            -ms-flex: 1 1 auto;
            flex: 1 1 auto;
            padding: 1rem;
        }
    
        .modal-footer {
            display: -webkit-box;
            display: -ms-flexbox;
            display: flex;
            -webkit-box-align: center;
            -ms-flex-align: center;
            align-items: center;
            justify-content: space-evenly;
            padding: 1rem;
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
    
        .close {
            float: right;
            font-size: 1.5rem;
            font-weight: 700;
            line-height: 1;
            color: #000;
            text-shadow: 0 1px 0 #fff;
            opacity: .5;
        }
    
        .close:not(:disabled):not(.disabled) {
            cursor: pointer;
        }
    
        button.close {
            padding: 0;
            background-color: transparent;
            border: 0;
            -webkit-appearance: none;
        }
    
        .btn {
            display: inline-block;
            font-weight: 400;
            text-align: center;
            white-space: nowrap;
            vertical-align: middle;
            -webkit-user-select: none;
            -moz-user-select: none;
            -ms-user-select: none;
            user-select: none;
            border: 1px solid transparent;
            font-size: 1rem;
            line-height: 1.5;
            transition: color .15s ease-in-out, background-color .15s ease-in-out, border-color .15s ease-in-out, box-shadow .15s ease-in-out;
            padding: 0.5rem 1rem;
            border-radius: 0.3rem;
        }
    
        .btn:hover {
            cursor: pointer;
        }
    
        .btn.disabled, .btn:disabled,
        .pincode-btn.disabled, .pincode-btn:disabled {
            opacity: .65;
            cursor: not-allowed;
        }
    
        .btn-block {
            display: block;
            width: 100%;
        }
        
        .btn-circle {
            border-radius: 50%;
        }
    
        .btn-primary {
            color: #fff;
            background-color: #007bff;
            border-color: #007bff;
        }
    
        .btn-primary:hover {
            color: #fff;
            background-color: #0069d9;
            border-color: #0062cc;
        }
    
        .btn-primary.disabled, .btn-primary:disabled {
            color: #fff;
            background-color: #007bff;
            border-color: #007bff;
        }
        
        .btn-outline-primary {
            color: #007bff;
            background-color: transparent;
            background-image: none;
            border-color: #007bff;
        }
        
        .btn-outline-primary:hover {
            box-shadow: 0 0 0 4px #61ADFF40;
        }
    
        .btn-secondary {
            color: #fff;
            background-color: #6c757d;
            border-color: #6c757d;
        }
    
        .btn-secondary:hover {
            color: #fff;
            background-color: #5a6268;
            border-color: #545b62;
        }
    
        .btn-secondary.disabled, .btn-secondary:disabled {
            color: #fff;
            background-color: #6c757d;
            border-color: #6c757d;
        }
    
        .scroll-info {
            font-weight: bold;
            color: #FFC107;
            background: #fcf8ec;
            padding: 5px 10px;
            border-radius: 5px;
            margin: 1em;
            text-align: center;
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
        
        .pincode_show_time_selection {
            color: #000000;
        }
    
        .pincode_show_time_selection .time-text {
            font-weight: bold;
        }
    
        .pincode_show_time_selection .unit {
            font-size: 0.8em;
        }
    
        .pincode_show_time_selection input[type="radio"] {
            display: none;
        }
    
        .pincode_show_time_selection input[type="radio"] + label {
            width: 50px;
            height: 50px;
            display: flex;
            align-items: center;
            justify-content: center;
            flex-direction: column;
            border-radius: 50%;
            background-color: #ffffff;
            border: 1px solid #d2d2d2;
            line-height: normal;
            cursor: pointer;
        }
    
        .pincode_show_time_selection input[type="radio"]:checked + label {
            background-color: #007bff;
            color: #FFFFFF;
        }
    
        .pincode {
            font-size: 68px;
            font-family: fantasy;
            line-height: initial;
            border: solid 1px #989898;
            padding: 0 20px;
        }
        
        .pincode.disabled {
            color: #c0c0c0;
        }
    
        .pincode-time {
            font-size: 38px;
            font-weight: bold;
            color: #0093FF;
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
    
        .gg-link {
            box-sizing: border-box;
            position: relative;
            display: block;
            transform: rotate(-45deg) scale(1);
            width: 8px;
            height: 2px;
            background: currentColor;
            border-radius: 4px
        }
    
        .gg-link::after,
        .gg-link::before {
            content: "";
            display: block;
            box-sizing: border-box;
            position: absolute;
            border-radius: 3px;
            width: 8px;
            height: 10px;
            border: 2px solid;
            top: -4px
        }
    
        .gg-link::before {
            border-right: 0;
            border-top-left-radius: 40px;
            border-bottom-left-radius: 40px;
            left: -6px
        }
    
        .gg-link::after {
            border-left: 0;
            border-top-right-radius: 40px;
            border-bottom-right-radius: 40px;
            right: -6px
        }
    
        .gg-mail,
        .gg-mail::after {
            display: block;
            box-sizing: border-box;
            height: 14px;
            border: 2px solid
        }
    
        .gg-mail {
            overflow: hidden;
            transform: scale(1);
            position: relative;
            width: 18px;
            border-radius: 2px
        }
    
        .gg-mail::after {
            content: "";
            position: absolute;
            border-radius: 3px;
            width: 14px;
            transform: rotate(-45deg);
            bottom: 3px;
            left: 0
        }
    
        .gg-info {
            box-sizing: border-box;
            position: relative;
            display: block;
            transform: scale(1);
            width: 20px;
            height: 20px;
            border: 2px solid;
            border-radius: 40px
        }
    
        .gg-info::after,
        .gg-info::before {
            content: "";
            display: block;
            box-sizing: border-box;
            position: absolute;
            border-radius: 3px;
            width: 2px;
            background: currentColor;
            left: 7px
        }
    
        .gg-info::after {
            bottom: 2px;
            height: 8px
        }
    
        .gg-info::before {
            height: 2px;
            top: 2px
        }
        
        .gg-chevron-down {
            box-sizing: border-box;
            position: relative;
            display: block;
            transform: scale(1);
            width: 22px;
            height: 22px;
            border: 2px solid transparent;
            border-radius: 100px
        }
        
        .gg-chevron-down::after {
            content: "";
            display: block;
            box-sizing: border-box;
            position: absolute;
            width: 10px;
            height: 10px;
            border-bottom: 2px solid;
            border-right: 2px solid;
            transform: rotate(45deg);
            left: 4px;
            top: 2px
        } 
        
        .icon-check {
            --check-svg: url("data:image/svg+xml,%3Csvg width='24' height='24' viewBox='0 0 24 24' fill='none' xmlns='http://www.w3.org/2000/svg'%3E%3Cpath d='M2.75 12c0-5.11 4.14-9.25 9.25-9.25s9.25 4.14 9.25 9.25-4.14 9.25-9.25 9.25S2.75 17.11 2.75 12zM10.83 15l-3.18-3.18M17.19 8.64L10.83 15' stroke='%23000' stroke-width='1.2' stroke-linecap='round' stroke-linejoin='round'/%3E%3C/svg%3E");
            width: 48px;
            height: 48px;
            mask-image: var(--check-svg);
            mask-repeat: no-repeat;
            mask-position: center;
            mask-size: contain;
            -webkit-mask-image: var(--check-svg);
            -webkit-mask-repeat: no-repeat;
            -webkit-mask-position: center;
            -webkit-mask-size: contain;
            background-color: #007bff;
        }
                    
        .pincode-btn {
            display: flex;
            flex-direction: column;
            align-items: center;
            cursor: pointer;
        }
    
        .pincode-btn > div:first-child {
            width: 50px;
            height: 50px;
            border-radius: 50%;
            display: flex;
            justify-content: center;
            align-items: center;
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
        
        .page-title {
            font-size: 1.2em;
            font-weight: bold;
            color: #000;
            margin: 10px 0;
            display: flex;
            align-items: center;
            gap: 5px;
        }
        
        .page-content {
            text-align: left;
            display: flex;
            flex-direction: column;
            gap: 10px;
        }
        
        .page-desc {
            text-align: left;   
        }
        
        .site-manager-btn {
            width: 100px;
            margin: 20px 0;
        }
        
        .site-manager-bg{
            width: 450px;
        }
        
        .RT .page-title,
        .ROG .page-title,
        .TUF .page-title{
            color: #FFF;
        }
        
        .RT .btn-outline-primary {
            color: #FFF;
            background-color: transparent;
            background-image: none;
            border-color: #FFF;
        }
        
        .RT .btn-outline-primary:hover {
            box-shadow: 0 0 0 4px #61ADFF40;
        }
        
      </style>`,

    ModalTheme: {
        ROG: `<style>
            .modal-content {
                background-color: #000000e6;
                border: 1px solid rgb(161, 10, 16);
            }
            .modal-title {
                color: #cf0a2c;
            }
            .btn-primary {
                background-color: #91071f;
                border: 0;
            }
            .btn-primary.disabled, .btn-primary:disabled {
                color: #fff;
                background-color: #91071f;
            }
            .btn-primary:hover {
                background-color: #cf0a2c;
            }
        </style>`,

        RT: `<style>
            .modal-content {
                background-color: #2B373B;
            }
            .modal-title {
                color: #FFFFFF;
            }
        </style>`,

        TUF: `<style>
            .modal-content {
                background-color: #000000e6;
                border: 1px solid #92650F;
            }
            .modal-title {
                color: #ffa523;
            }
            .btn-primary {
                background-color: #ffa523;
                border: 0;
            }
            .btn-primary.disabled, .btn-primary:disabled {
                color: #fff;
                background-color: #ffa523;
            }
            .btn-primary:hover {
                background-color: #D0982C;
            }
        </style>`
    },

    getTheme: () => {
        let ui_support = httpApi.hookGet("get_ui_support");

        function isSupport(_ptn) {
            return ui_support[_ptn] ? true : false;
        }

        let theme = 'RT';
        if (isSupport("rog")) {
            return "ROG";
        } else if (isSupport("tuf")) {
            return "TUF";
        } else if (isSupport("UI4")) {
            return "";
        } else {
            return theme;
        }
    }
}

const pincode_status = {
    "pincode": '',
    "duration": '',
    "end_time": '',
    "oauth_auth_status": ''
};

pincode_status.pincode = httpApi.nvramGet(['newsite_provisioning_pincode'], true).newsite_provisioning_pincode;
pincode_status.duration = httpApi.nvramGet(['newsite_provisioning_duration'], true).newsite_provisioning_duration;
pincode_status.end_time = httpApi.nvramGet(['newsite_provisioning_timestamp'], true).newsite_provisioning_timestamp;
pincode_status.oauth_auth_status = httpApi.nvramGet(['oauth_auth_status'], true).oauth_auth_status;

class PincodeTimeSelector extends HTMLElement {
    constructor() {
        super();
        const template = document.createElement('template');
        template.innerHTML = `
            <div class="d-flex gap-2 flex-row">
                <div class="pincode_show_time_selection">
                    <input type="radio" id="pincode-time-5" name="pincode-time" value="5">
                    <label for="pincode-time-5">
                        <div class="time-text">5</div>
                        <div class="unit">min</div>
                    </label>
                </div>
                <div class="pincode_show_time_selection">
                    <input type="radio" id="pincode-time-30" name="pincode-time" value="30">
                    <label for="pincode-time-30">
                        <div class="time-text">30</div>
                        <div class="unit">min</div>
                    </label>
                </div>
                <div class="pincode_show_time_selection">
                    <input type="radio" id="pincode-time-60" name="pincode-time" value="60">
                    <label for="pincode-time-60">
                        <div class="time-text">1</div>
                        <div class="unit">hr</div>
                    </label>
                </div>
                <div class="pincode_show_time_selection">
                    <input type="radio" id="pincode-time-360" name="pincode-time" value="360">
                    <label for="pincode-time-360">
                        <div class="time-text">6</div>
                        <div class="unit">hr</div>
                    </label>
                </div>
                <div class="pincode_show_time_selection">
                    <input type="radio" id="pincode-time-720" name="pincode-time" value="720">
                    <label for="pincode-time-720">
                        <div class="time-text">12</div>
                        <div class="unit">hr</div>
                    </label>
                </div>
                <div class="pincode_show_time_selection">
                    <input type="radio" id="pincode-time-1440" name="pincode-time" value="1440">
                    <label for="pincode-time-1440">
                        <div class="time-text">24</div>
                        <div class="unit">hr</div>
                    </label>
                </div>
            </div>
        `;
        this.appendChild(template.content.cloneNode(true));
        this.div = this.querySelector('div');
        const radioElements = this.div.querySelectorAll('input[type="radio"]');
        radioElements.forEach((radio) => {
            radio.addEventListener('change', this.handleChangeTime.bind(this));
        });
    }

    selectTime = 300
    changeTimeCallback = () => {
    }

    setChangeTimeCallback(callback) {
        if (typeof callback === "function") {
            this.changeTimeCallback = callback;
        }
    }

    handleChangeTime = (e) => {
        this.selectTime = e.target.value * 60;
        this.changeTimeCallback();
    }
}

class PincodeDiv extends HTMLElement {
    constructor() {
        super();
        this.attachShadow({mode: 'open'});

        const pincodeRegisterDiv = document.createElement('div');
        pincodeRegisterDiv.id = "pincode_register";
        pincodeRegisterDiv.classList.add('d-flex', 'gap-1', 'flex-column');
        pincodeRegisterDiv.innerHTML = `
            <div class="text-left"><#Pincode_Choose_Time#></div>
            <div id="pincode_time_selector"></div>
            <div class="text-left">
                <button id="pincode-gen-btn" class="btn btn-primary disabled"><#Pincode_Generate#></button>
            </div>
        `

        const Pincode_Sharing_Desc = `<#Pincode_Sharing_Desc#>`.replaceAll(`%@`,`ASUS Site Manager`);
        const Pincode_Expire = `<#Pincode_Expire#>`

        const pincodeViewerDiv = document.createElement('div');
        pincodeViewerDiv.id = "pincode_viewer";
        pincodeViewerDiv.classList.add('d-flex', 'gap-1', 'flex-column', 'text-left');
        pincodeViewerDiv.innerHTML = `
            <div class="d-flex flex-row align-items-center gap-1">
                <div id="pincode" class="pincode"></div>
                <div class="d-flex gap-1 flex-row">
                    <div class="pincode-btn copy-pincode"><div class="btn-primary"><i class="gg-link text-white"></i></div>Copy</div>
                    <div class="pincode-btn email-pincode"><div class="btn-primary"><i class="gg-mail text-white"></i></div><#AiProtection_WebProtector_EMail#></div>
                </div>
            </div>
            <div class="text-left">${Pincode_Sharing_Desc} ${Pincode_Expire}</div>
            <div class="d-flex flex-row align-items-center gap-1">
                <div id="pincode_time" class="pincode-time"></div>
            </div>
            <div class="text-left">
                <button type="button" class="btn btn-primary" id="stopPincodeShare"><#Pincode_Stop_Sharing#></button>
                <button type="button" class="btn btn-primary" id="regeneratePincode" style="display: none;"><#Pincode_Regenerate#></button>
            </div>
        `

        const Pincode_Already_Managed = `<#Pincode_Already_Managed#>`.replaceAll(`%@`,`ASUS Site Manager`);
        const pincodeBindedDiv = document.createElement('div');
        pincodeBindedDiv.id = "pincode_binded";
        pincodeBindedDiv.classList.add('d-flex', 'gap-1', 'flex-column', 'text-left', 'mt-2');
        pincodeBindedDiv.innerHTML = `
            <div class="d-flex flex-row align-items-center gap-1">
                <div class="icon-check"></div>
                <div style="font-size:18px;">${Pincode_Already_Managed}</div>
            </div>
        `

        const template = document.createElement('template');
        template.innerHTML = `
            ${ASUS_PINCODE.PincodeDivStyle}
            <div id="pincode_content" class="d-flex gap-1 flex-column"></div>
        `;

        this.shadowRoot.appendChild(template.content.cloneNode(true));
        this.div = this.shadowRoot.querySelector('div');

        this.pincodeRegister = pincodeRegisterDiv;
        this.pincodeViewer = pincodeViewerDiv;
        this.pincodeBinded = pincodeBindedDiv;
        this.loader = new PincodeLoaderComponent();

        this.pincode_time_selector = new PincodeTimeSelector();
        this.pincodeRegister.querySelector('#pincode_time_selector').appendChild(this.pincode_time_selector);
        this.pincode_time_selector.setChangeTimeCallback(() => {
            this.pincodeRegister.querySelector('#pincode-gen-btn').classList.remove("disabled");
        })

        this.pincodeRegister.querySelector('#pincode-gen-btn').addEventListener('click', this.handlePincodeGen.bind(this));
        this.pincodeViewer.querySelector('#stopPincodeShare').addEventListener('click', this.handleShowPincodeDeleteConfirm.bind(this));
        this.pincodeViewer.querySelector('#regeneratePincode').addEventListener('click', this.initPincodeRegister.bind(this));

        if (pincode_status.oauth_auth_status == 1) {
            this.loader.show();
            this.initPincodeViewer();
        } else {
            if (pincode_status.oauth_auth_status == 2) {
                this.initPincodeBinded();
            } else if (pincode_status.end_time > Math.floor(Date.now() / 1000)) {
                this.initPincodeViewer();
            } else {
                this.initPincodeRegister();
            }
        }
    }


    timerInterval = null
    timerBindedInterval = null

    refreshPincode = () => {
        pincode_status.pincode = httpApi.nvramGet(['newsite_provisioning_pincode'], true).newsite_provisioning_pincode;
        pincode_status.duration = httpApi.nvramGet(['newsite_provisioning_duration'], true).newsite_provisioning_duration;
        pincode_status.end_time = httpApi.nvramGet(['newsite_provisioning_timestamp'], true).newsite_provisioning_timestamp;
    }

    initPincodeRegister = () => {
        this.loader.hide();
        this.shadowRoot.querySelector('#pincode_content').append(this.pincodeRegister);
        this.pincodeViewer.remove();
        this.pincodeBinded.remove();
        clearInterval(this.timerInterval);
        this.pincodeRegister.querySelector('#pincode-gen-btn').classList.add('disabled');
        const radios = this.pincodeRegister.querySelectorAll('input[type="radio"]');
        radios.forEach((radio) => {
            radio.checked = radio.defaultChecked;
        });
    }

    initPincodeViewer = () => {
        this.loader.hide();
        this.shadowRoot.querySelector('#pincode_content').append(this.pincodeViewer);
        this.pincodeRegister.remove();
        this.pincodeBinded.remove();
        this.refreshPincode();
        clearInterval(this.timerInterval);
        this.pincodeViewer.querySelector('.copy-pincode').addEventListener('click', this.handleCopyPincode.bind(this));
        this.pincodeViewer.querySelector('.email-pincode').addEventListener('click', this.handleEmailPincode.bind(this));
        this.pincodeViewer.querySelector('#pincode').innerText = `${pincode_status.pincode.slice(0, 3)} ${pincode_status.pincode.slice(3)}`
        this.timerInterval = setInterval(this.updateCountdown.bind(this), 1000);
        this.updateCountdown.bind(this);
    }

    initPincodeBinded = () => {
        this.loader.hide();
        this.pincodeViewer.remove();
        this.pincodeRegister.remove();
        this.shadowRoot.querySelector('#pincode_content').append(this.pincodeBinded);
        clearInterval(this.timerInterval);
        this.timerBindedInterval = setInterval(() => {
            const oauth_auth_status = httpApi.nvramGet(['oauth_auth_status'], true).oauth_auth_status;
            if (oauth_auth_status == 0) {
                this.initPincodeRegister();
                clearInterval(this.timerBindedInterval);
            }
        }, 3000)
    }

    agree = () => {

        this.loader.show();

        const urlencoded = new URLSearchParams();
        urlencoded.append("duration", this.pincode_time_selector.selectTime);
        const requestOptions = {
            method: 'POST',
            body: urlencoded,
        };
        fetch("get_provision_pincode.cgi?force=1", requestOptions)
            .then(response => response.json())
            .then(result => {
                if (result.pincode !== "") {
                    this.initPincodeViewer()
                } else {
                    throw new Error(`<#Pincode_Share_Error#>`);
                }
            })
            .catch(error => {
                console.log('error', error)
                alert(`<#Pincode_Share_Error#>`);
                this.loader.hide();
            });
    }

    handleShowPincodeDeleteConfirm = () => {
        const PincodeStopSharingConfirm = `<#Pincode_Stop_Sharing_Confirm#>`.replaceAll(`%@`,`ASUS Site Manager`);
        const pop = new PincodeDeleteConfirmComponent(
            {
                id: 'pincode_delete_confirm',
                title: '',
                body: `
                    <div class="d-flex flex-column gap-2 text-left">
                        <div class="d-flex flex-column">
                            <div>${PincodeStopSharingConfirm}</div>
                        </div>
                        <div class="d-flex flex-row align-items-center justify-content-center gap-3">
                            <div class="btn btn-primary cancel"><#CTL_Cancel#></div>
                            <div class="btn btn-primary ok"><#CTL_ok#></div>
                        </div>
                    </div>
                `,
                deletePincodeCallback: this.initPincodeRegister.bind(this)
            }
        );
        pop.show();
    }

    handlePincodeGen = (e) => {
        if (!e.target.classList.contains("disabled")) {
            const policyStatus = PolicyStatus()
			.then(data => {
                if (data.PP == 0 || data.PP_time == '') {
                    const policyModal = new PolicyModalComponent({
                        policy: "PP",
                        policyStatus: data,
                        agreeCallback: () => {
                            top.document.querySelector('#policy_popup_modal').remove();
                            PolicyStatus();
                            this.agree();
                        },
                        disagreeCallback: () => {
                            alert(`<#ASUS_POLICY_Function_Confirm#>`)
                        },
                        knowRiskCallback: () => {
                            top.document.querySelector('#policy_popup_modal').remove();
                        }
                    });
                    policyModal.show();
                } else {
                    this.agree()
                }
            });
        }
    }

    updateCountdown = () => {
        const oauth_auth_status = httpApi.nvramGet(['oauth_auth_status'], true).oauth_auth_status;

        if (oauth_auth_status == 1) {
            this.loader.show();
            this.pincodeViewer.querySelector('#pincode_time').innerHTML = `00<small>h</small>:00<small>m</small>:00<small>s</small>`;
            this.pincodeViewer.querySelector('#pincode').classList.add('disabled');
            this.pincodeViewer.querySelector('#stopPincodeShare').classList.add('disabled');
            this.pincodeViewer.querySelectorAll('.pincode-btn').forEach(elem => {
                elem.classList.add('disabled')
            });
        } else {
            this.pincodeViewer.querySelector('#pincode').classList.remove('disabled');
            this.pincodeViewer.querySelector('#stopPincodeShare').classList.remove('disabled');
            this.pincodeViewer.querySelector('#stopPincodeShare').style.display = 'block';
            this.pincodeViewer.querySelector('#regeneratePincode').style.display = 'none';
            this.pincodeViewer.querySelectorAll('.pincode-btn').forEach(elem => {
                elem.classList.remove('disabled')
            });

            const currentUnixTime = Math.floor(Date.now() / 1000);
            const remainingTime = pincode_status.end_time - currentUnixTime;
            const hours = Math.floor(remainingTime / 3600);
            const minutes = Math.floor((remainingTime % 3600) / 60);
            const seconds = remainingTime % 60;
            this.pincodeViewer.querySelector('#pincode_time').innerHTML = `${hours.toString().padStart(2, '0')}<small>h</small>:${minutes.toString().padStart(2, '0')}<small>m</small>:${seconds.toString().padStart(2, '0')}<small>s</small>`;
            if (remainingTime <= 0) {
                this.pincodeViewer.querySelector('#pincode_time').innerHTML = `00<small>h</small>:00<small>m</small>:00<small>s</small>`;
                this.pincodeViewer.querySelector('#pincode').classList.add('disabled');
                this.pincodeViewer.querySelector('#stopPincodeShare').style.display = 'none';
                this.pincodeViewer.querySelector('#regeneratePincode').style.display = 'block';
                this.pincodeViewer.querySelectorAll('.pincode-btn').forEach(elem => {
                    elem.classList.add('disabled')
                });
                clearInterval(this.timerInterval);
            }
        }

        if (oauth_auth_status == 2) {
            this.initPincodeBinded();
        }

    }

    handleCopyPincode = (e) => {
        if (!e.target.classList.contains("disabled") && !e.target.parentElement.classList.contains("disabled") && !e.target.parentElement.parentElement.classList.contains("disabled")) {
            if (window.isSecureContext && navigator.clipboard) {
                navigator.clipboard.writeText(text);
            } else {
                const textArea = document.createElement("textarea");
                textArea.value = pincode_status.pincode;
                document.body.appendChild(textArea);
                textArea.select();
                try {
                    document.execCommand('copy')
                } catch (err) {
                    console.error('Unable to copy to clipboard', err)
                }
                document.body.removeChild(textArea)
            }
        }
    }

    handleEmailPincode = (e) => {
        if (!e.target.classList.contains("disabled") && !e.target.parentElement.classList.contains("disabled") && !e.target.parentElement.parentElement.classList.contains("disabled")) {
            const utcTimestamp = pincode_status.end_time * 1000;
            const date = new Date(utcTimestamp);
            const year = date.getFullYear();
            const month = (date.getMonth() + 1).toString().padStart(2, '0');
            const day = date.getDate().toString().padStart(2, '0');
            const hours = date.getHours().toString().padStart(2, '0');
            const minutes = date.getMinutes().toString().padStart(2, '0');
            const timezone = `(UTC/GMT${date.getTimezoneOffset() > 0 ? `-` : `+`}${date.getTimezoneOffset() / -60})`
            const formattedTime = `${hours}:${minutes}`;
            const formattedDate = `${year}-${month}-${day} ${formattedTime} ${timezone}`;
            const Pincode_Email_Title = `<#Pincode_Email_Title#>`;
            const Pincode_Email_Desc_1 = `<#Pincode_Email_Desc_1#>`;
            const Pincode_Email_Desc_2 = `<#Pincode_Email_Desc_2#>`
                .replaceAll('%1$@', `ASUS Site Manager`)
                .replaceAll('%2$@', `${formattedDate}`);
            const Pincode_Email_Footer = `<#footer_copyright_desc#>`
            window.location = `mailto:?subject=${Pincode_Email_Title}&body=${Pincode_Email_Desc_1} %0D%0A${pincode_status.pincode} ${Pincode_Email_Desc_2}%0D%0A%0D%0A${Pincode_Email_Footer}`
        }
    }
}

class PincodeTitleDiv {

    constructor() {
        const Pincode_Desc = `<#Pincode_Desc#>`.replaceAll('%@', `ASUS Site Manager`);
        const div = document.createElement('div');
        const shadowRoot = div.attachShadow({mode: 'open'});
        const template = document.createElement('template');
        template.innerHTML = `
            ${ASUS_PINCODE.PincodeDivStyle}
            <div class="d-flex align-items-center gap-1">${Pincode_Desc}</div>
        `;

        shadowRoot.appendChild(template.content.cloneNode(true));

        this.element = div;
    }

    render() {
        return this.element;
    }
}

customElements.define('pincode-div', PincodeDiv);
customElements.define('pincode-timeselector', PincodeTimeSelector);

class PincodeComponent {
    constructor(props) {
        this.props = props;
        this.element = document.createElement('pincode-div');
    }

    render() {
        return this.element;
    }
}

class PopupModalComponent {
    constructor(props) {
        this.id = props.id;
        this.title = props.title;
        this.body = props.body;
        const div = document.createElement('div');
        div.id = this.id;
        const shadowRoot = div.attachShadow({mode: 'open'});
        const template = document.createElement('template');
        template.innerHTML = `
            ${ASUS_PINCODE.PincodeDivStyle}
            <div class="popup_bg">
                <div class="modal">
                    <div class="modal-dialog">
                        <div class="modal-content">
                            <div class="modal-header">
                                <h5 class="modal-title">${this.title}</h5>
                                <button type="button" class="close">
                                    <span>&times;</span>
                                </button>
                            </div>
                            <div class="modal-body">
                                ${this.body}
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        `;

        this.template = typeof props.template !== 'undefined' ? props.template : template;

        shadowRoot.appendChild(this.template.content.cloneNode(true));

        this.element = div;
        top.document.body.style.overflow = 'hidden';

        if (this.element.shadowRoot.querySelector('button.close')) {
            this.element.shadowRoot.querySelector('button.close').addEventListener('click', this.hide.bind(this));
        }
    }

    render() {
        return this.element;
    }

    show() {
        if (top.document.querySelector(`#${this.id}`) == null) {
            top.document.body.appendChild(this.element);
        }
    }

    hide() {
        top.document.body.style.removeProperty('overflow');
        this.element.remove();
    }
}

class PincodeDeleteConfirmComponent extends PopupModalComponent {
    constructor(props) {
        super(props);
        this.element.shadowRoot.querySelector('.btn.cancel').addEventListener('click', this.hide.bind(this));
        this.element.shadowRoot.querySelector('.btn.ok').addEventListener('click', this.handlePincodeDel.bind(this));
        this.loader = new PincodeLoaderComponent();
        this.deletePincodeCallback = props.deletePincodeCallback;
    }

    handlePincodeDel = () => {
        this.hide();
        this.loader.show();
        fetch("del_provision_pincode.cgi")
            .then(response => response.json())
            .then(result => {
                if (result.statusCode == 200) {
                    this.loader.hide();
                    this.deletePincodeCallback();
                }
            })
            .catch(error => console.log('error', error));
    }
}

class PincodeLoaderComponent extends PopupModalComponent {
    constructor() {
        const template = document.createElement('template');
        template.innerHTML = `
            ${ASUS_PINCODE.PincodeDivStyle}
            <div id="pincode_loader" class="popup_bg d-flex flex-column align-items-center justify-content-center gap-2">
                <i class="gg-spinner"></i>
                <div><#QIS_autoMAC_desc2#></div>
            </div>
        `;
        super({id: 'popup_loader', title: '', body: '', template: template});
    }
}

class SiteManagerComponent {
    constructor(props) {
        this.id = props.id;
        this.title = props.title;
        const div = document.createElement('div');
        div.id = this.id;
        const shadowRoot = div.attachShadow({mode: 'open'});
        const template = document.createElement('template');
        template.innerHTML = `
            ${ASUS_PINCODE.PincodeDivStyle}
            <div class="d-flex flex-column align-items-start ${ASUS_PINCODE.getTheme()}">
                <div class="page-title">${this.title} <a href="#showPincodeHelp" id="showPincodeHelp" class="text-primary"><svg xmlns="http://www.w3.org/2000/svg" height="1em" viewBox="0 0 512 512"><path d="M256 8C119.043 8 8 119.083 8 256c0 136.997 111.043 248 248 248s248-111.003 248-248C504 119.083 392.957 8 256 8zm0 448c-110.532 0-200-89.431-200-200 0-110.495 89.472-200 200-200 110.491 0 200 89.471 200 200 0 110.53-89.431 200-200 200zm107.244-255.2c0 67.052-72.421 68.084-72.421 92.863V300c0 6.627-5.373 12-12 12h-45.647c-6.627 0-12-5.373-12-12v-8.659c0-35.745 27.1-50.034 47.579-61.516 17.561-9.845 28.324-16.541 28.324-29.579 0-17.246-21.999-28.693-39.784-28.693-23.189 0-33.894 10.977-48.942 29.969-4.057 5.12-11.46 6.071-16.666 2.124l-27.824-21.098c-5.107-3.872-6.251-11.066-2.644-16.363C184.846 131.491 214.94 112 261.794 112c49.071 0 101.45 38.304 101.45 88.8zM298 368c0 23.159-18.841 42-42 42s-42-18.841-42-42 18.841-42 42-42 42 18.841 42 42z"/></svg></a></div>
                <div class="page-content">
                    <div class="page-desc"><#Pincode_Site_Manager_Desc1#></div>
                    <div><#Pincode_Site_Manager_Desc2#></div>
                    <div>
                        <a href="https://nw-dlcdnet.asus.com/support/forward.html?model=&type=siteManager&lang=&kw=&num=" target="_blank"><div class="btn btn-outline-primary site-manager-btn">Go</div></a>
                    </div>
                    <div>
                        <img class="site-manager-bg" src="/images/New_ui/site_manager.svg" />
                    </div>
                </div>
            </div>
        `;
        this.template = typeof props.template !== 'undefined' ? props.template : template;
        shadowRoot.appendChild(this.template.content.cloneNode(true));
        shadowRoot.querySelector("#showPincodeHelp").addEventListener('click', this.showPincodeHelp.bind(this));
        this.element = div;
    }

    render() {
        return this.element;
    }

    showPincodeHelp = () => {
        const Pincode_FAQ_1 = `<#Pincode_FAQ_1#>`
            .replaceAll('%@', `ASUS Site Manager`);
        const Pincode_FAQ_1_1 = `<#Pincode_FAQ_1_1#>`
            .replaceAll('%@', `ASUS Site Manager`);
        const Pincode_FAQ_2 = `<#Pincode_FAQ_2#>`
            .replaceAll('%@', `ASUS Site Manager`);
        const Pincode_FAQ_2_1 = `<#Pincode_FAQ_2_1#>`
            .replaceAll('%@', `ASUS Site Manager`);
        const Pincode_FAQ_2_2 = `<#Pincode_FAQ_2_2#>`
            .replaceAll('[aa]', `<a href="https://www.asus.com/support/faq/1053449">`)
            .replaceAll('[/aa]', `</a>`);
        const pincodeHelpModal = new PopupModalComponent({
            id: 'pincode_help_modal',
            title: `<#NewFeatureAbout#>`,
            body: `
                <div class="d-flex flex-column gap-2 text-left">
                    <div class="d-flex flex-column">
                        <div style="font-weight: bold">${Pincode_FAQ_1}</div>
                        <div style="font-size: 0.8em; color: #919191">${Pincode_FAQ_1_1}</div>
                    </div>
                    <div class="d-flex flex-column">
                        <div style="font-weight: bold">${Pincode_FAQ_2}</div>
                        <div style="font-size: 0.8em; color: #919191">${Pincode_FAQ_2_1} ${Pincode_FAQ_2_2}</div>
                    </div>
                </div>
            `
        });
        pincodeHelpModal.show();
    }
}