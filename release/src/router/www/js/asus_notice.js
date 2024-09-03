class ScrollDiv {

    constructor(props) {
        const {content, theme = '', scrollCallBack} = props;
        const div = document.createElement('div');
        const shadowRoot = div.attachShadow({mode: 'open'});
        const template = document.createElement('template');
        template.innerHTML = `
          <style>
            .scroll-div {
                background-color: #fff;
                overflow-y: auto;
                padding: 15px 12px 0 12px;
                border: 1px solid #ddd;
                border-radius: 5px;
                height: 50vh;
                color: #333;
                font-size: 1.3em;
            }
            .scroll-div a {
                color: #333;
            }
            
            .scroll-div ul{
                padding-left: 20px;
                margin: 0;
                line-height: 1.3em;
            }
            
            .scroll-div ul li{
                margin-bottom: 1.2em;
            }
            
            .scroll-div::-webkit-scrollbar {
                width: 10px;
                height: 10px;
            }
            
            .scroll-div::-webkit-scrollbar-thumb {
                background-color: #7775;
                border-radius: 20px;
            }
            
            .scroll-div::-webkit-scrollbar-thumb:hover {
                background-color: #7777;
            }
            
            .scroll-div::-webkit-scrollbar-track:hover {
                background-color: #5555;
            }
            
            .scroll-div.ROG,
            .scroll-div.TUF{
                border-radius: 0;
            }
            
            .scroll-div.RT{
                border: 1px solid #4D4D4D;
                background-color: #303D43;
                color: #FFFFFF;
            }
            
            .scroll-div.ROG,
            .scroll-div.TUF{
                border: 1px solid #4D4D4D;
                background-color: #181818;
                color: #FFFFFF;
            }
            
            .scroll-div.RT a,
            .scroll-div.ROG a,
            .scroll-div.TUF a{
                color: #FFFFFF;
            }
          </style>
          <div class="scroll-div ${theme}"></div>
        `;

        shadowRoot.appendChild(template.content.cloneNode(true));
        shadowRoot.querySelector('.scroll-div').addEventListener('click', this.handleClick.bind(this));
        shadowRoot.querySelector('.scroll-div').addEventListener('scroll', scrollCallBack.bind(this));
        shadowRoot.querySelector('.scroll-div').innerHTML = content;

        this.element = div;
    }

    render() {
        return this.element
    }

    reset() {
        this.element.shadowRoot.querySelector('.scroll-div').scrollTop = 0;
    }

    handleClick() {
        const scrollDiv = this.element.shadowRoot.querySelector('.scroll-div');
        let currentScrollTop = scrollDiv.scrollTop;
        const targetScrollTop = currentScrollTop + scrollDiv.offsetHeight;
        const animationDuration = 1000;
        const frameDuration = 15;
        const scrollDistancePerFrame = (targetScrollTop - currentScrollTop) / (animationDuration / frameDuration);

        function animateScroll() {
            currentScrollTop += scrollDistancePerFrame;
            scrollDiv.scrollTop = currentScrollTop;

            if (currentScrollTop < targetScrollTop) {
                requestAnimationFrame(animateScroll);
            }
        }

        animateScroll();
    }
}

class NoticePopupModalComponent {
    constructor(props) {
        const {
            id, title, body = "", theme = this.getTheme(),
            showCloseBtn = true,
            applyBtn = {
                show: true,
                text: "OK",
                clickCallBack: function () {
                }
            },
            readCheck = {
                show: true,
            }
        } = props;

        this.id = id;

        const div = document.createElement('div');
        div.id = id;
        const shadowRoot = div.attachShadow({mode: 'open'});
        const template = document.createElement('template');
        template.innerHTML = `
            <style>
                :host {
                    --rt-primary: #006CE1;
                    --business-primary: #006CE1;
                    --rog-primary: #FF1929;
                    --tuf-primary: #FFAA32;                    
                    
                    --business-notice: #B42D18;
                    --rt-notice: #E75B4B;
                    --rog-notice: #00D5FF;
                    --tuf-notice: #00D5FF;
                }
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
                    color: var(--business-primary);
                    font-weight: bold;
                    font-size: 2em;
                    margin: 0;
                }
            
                .modal-body {
                    position: relative;
                    -webkit-box-flex: 1;
                    -ms-flex: 1 1 auto;
                    flex: 1 1 auto;
                    padding: 0 1rem;
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
            
                .justify-content-center {
                    -webkit-box-pack: center !important;
                    -ms-flex-pack: center !important;
                    justify-content: center !important;
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
                
                .w-100 {
                    width: 100%;
                }
            
                @media (min-width: 576px) {
                    .modal-dialog {
                        max-width: 500px;
                        margin: 5rem auto;
                    }
                }
                
                .popup_bg.ROG .modal-content {
                    background-color: #000000e6;
                    border: 1px solid rgb(161, 10, 16);
                    border-radius: 0;
                }
                .popup_bg.ROG .modal-title {
                    color: #cf0a2c;
                }
                .popup_bg.ROG .btn{
                    color: #FFF;
                    border-radius: 0;
                    border: 1px solid #871019;
                }
                .popup_bg.ROG .btn.disabled{
                    background-color: #222222;
                    color: #999999;
                }
                .popup_bg.ROG .btn.btn-primary{
                    background-color: #BA1622;
                }
                .popup_bg.ROG .btn:hover{
                    box-shadow: 0 0 8px 0 #BA1622;
                }
                
                .popup_bg.RT .modal-content {
                    background-color: #2B373B;
                }
                .popup_bg.RT .modal-title {
                    color: #FFFFFF;
                }
                .popup_bg.RT .btn{
                    border: 1px solid #248DFF;
                    color: #FFF;
                }
                .popup_bg.RT .btn.disabled{
                    background-color: #000;
                    border-color: #B3B3B3;
                    color: #999999;
                }
                .popup_bg.RT .btn.btn-primary{
                    background-color: #248DFF;
                    border: 1px solid #248DFF;
                }
                .popup_bg.RT .btn:hover{
                    box-shadow: 0 0 0 4px #61ADFF33;
                }    
                
                .popup_bg.TUF .modal-content {
                    background-color: #000000e6;
                    border: 1px solid #92650F;
                    border-radius: 0;
                }
                .popup_bg.TUF .modal-title {
                    color: #ffa523;
                }
                .popup_bg.TUF .btn{
                    color: #FFF;
                    border-radius: 0;
                    border: 1px solid #F48116;
                }
                .popup_bg.TUF .btn.btn-primary{
                    background-color: #FF9E1B;
                    color: #000;
                }
                .popup_bg.TUF .btn.disabled{
                    background-color: #222222;
                    color: #999999;
                }
                .popup_bg.TUF .btn.disabled::after{
                    --borderColor: #FF9E1B;
                }
                .popup_bg.TUF .btn:hover{
                    box-shadow: 0 0 8px 0 #F48116;
                }
                
                .checkbox-wrapper-40 {
                    --borderColor: var(--business-primary);
                    --borderWidth: .125em;
                    color: #000;
                }
                
                .checkbox-wrapper-40 label {
                    display: block;
                    max-width: 100%;
                    margin: 0 auto;
                }
                
                .checkbox-wrapper-40 input[type=checkbox] {
                    -webkit-appearance: none;
                    appearance: none;
                    vertical-align: middle;
                    background: #fff;
                    font-size: 1.5em;
                    border-radius: 0.125em;
                    display: inline-block;
                    border: var(--borderWidth) solid var(--borderColor);
                    width: 1em;
                    height: 1em;
                    position: relative;
                }
                .checkbox-wrapper-40 input[type=checkbox]:before,
                .checkbox-wrapper-40 input[type=checkbox]:after {
                    content: "";
                    position: absolute;
                    background: var(--borderColor);
                    width: calc(var(--borderWidth) * 3);
                    height: var(--borderWidth);
                    top: 50%;
                    left: 10%;
                    transform-origin: left center;
                }
                .checkbox-wrapper-40 input[type=checkbox]:before {
                    transform: rotate(45deg) translate(calc(var(--borderWidth) / -2), calc(var(--borderWidth) / -2)) scaleX(0);
                    transition: transform 200ms ease-in 200ms;
                }
                .checkbox-wrapper-40 input[type=checkbox]:after {
                    width: calc(var(--borderWidth) * 5);
                    transform: rotate(-45deg) translateY(calc(var(--borderWidth) * 2)) scaleX(0);
                    transform-origin: left center;
                    transition: transform 200ms ease-in;
                }
                .checkbox-wrapper-40 input[type=checkbox]:checked:before {
                    transform: rotate(45deg) translate(calc(var(--borderWidth) / -2), calc(var(--borderWidth) / -2)) scaleX(1);
                    transition: transform 200ms ease-in;
                }
                .checkbox-wrapper-40 input[type=checkbox]:checked:after {
                    width: calc(var(--borderWidth) * 5);
                    transform: rotate(-45deg) translateY(calc(var(--borderWidth) * 2)) scaleX(1);
                    transition: transform 200ms ease-out 200ms;
                }
                .popup_bg.RT .checkbox-wrapper-40 {
                    --borderColor: var(--rt-primary);
                    color: #FFF;
                }
                .popup_bg.ROG .checkbox-wrapper-40 {
                    --borderColor: var(--rog-primary);
                    color: #FFF;
                }
                .popup_bg.TUF .checkbox-wrapper-40 {
                    --borderColor: var(--tuf-primary);
                    color: #FFF;
                }
                
              </style>
            <div class="popup_bg ${theme}">
                <div class="modal">
                    <div class="modal-dialog">
                        <div class="modal-content">
                            <div class="modal-header">
                                <h5 class="modal-title">${title}</h5>
                                ${(showCloseBtn) ? `<button type="button" class="close"><span>&times;</span></button>` : ``}
                            </div>
                            <div class="modal-body">
                                ${body}
                            </div>
                            <div class="modal-footer">
                                <div class="d-flex flex-column gap-1 w-100">
                                    ${(readCheck.show) ? `<div class="checkbox-wrapper-40 m-2 w-100"><label><input id="readCheckbox" type="checkbox"/><span class="checkbox">I have read it, do not show it again</span></label></div>` : ``}
                                    ${(applyBtn?.show) ? `<button type="button" class="btn btn-primary" data-dismiss="modal">${applyBtn.text}</button>` : ``}
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        `;

        this.template = typeof props.template !== 'undefined' ? props.template : template;

        shadowRoot.appendChild(this.template.content.cloneNode(true));

        this.element = div;

        if (this.element.shadowRoot.querySelector('button.close')) {
            this.element.shadowRoot.querySelector('button.close').addEventListener('click', this.hide.bind(this));
        }

        if (applyBtn?.show) {
            this.element.shadowRoot.querySelector('.btn-primary').addEventListener('click', applyBtn.clickCallBack);
        }
    }

    getTheme() {
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
            return "";
        } else {
            return theme;
        }
    }

    getFwString = () => {
        const fwInfo = httpApi.nvramGet(['firmver', 'buildno', 'extendno'], false)
        return `${fwInfo.firmver}.${fwInfo.buildno}_${fwInfo.extendno}`.split("-g")[0];
    }

    isRead() {
        const isNoticeRead = localStorage.getItem(`isNoticeRead_${this.getFwString()}`);
        return isNoticeRead === 'true';
    }

    setRead() {
        localStorage.setItem(`isNoticeRead_${this.getFwString()}`, "true");
    }

    getReadCheck() {
        return this.element.shadowRoot.querySelector('#readCheckbox').checked;
    }

    render() {
        return this.element;
    }

    show() {
        top.document.body.style.overflow = 'hidden';
        if (top.document.querySelector(`#${this.id}`) == null) {
            top.document.body.appendChild(this.element);
        }
    }

    hide() {
        top.document.body.style.removeProperty('overflow');
        this.element.remove();
    }
}

class PopupModalWithScrollComponent extends NoticePopupModalComponent {
    constructor(props) {
        super(props);
        const {content, theme = super.getTheme()} = props;
        const scrollCallBack = this.scrollCallBack.bind(this);
        const scrollDiv = new ScrollDiv({content, theme, scrollCallBack});
        this.element.shadowRoot.querySelector('.modal-body').appendChild(scrollDiv.render());
    }

    scrollCallBack() {
        const scrollDiv = this.element.shadowRoot.querySelector('.scroll-div');
        const scrollInfo = this.element.shadowRoot.querySelector('.scroll-info');
        if (scrollDiv.scrollTop + scrollDiv.offsetHeight >= scrollDiv.scrollHeight) {
            scrollInfo.style.display = 'none';
        } else {
            scrollInfo.style.display = 'block';
        }
    }
}

const showAsusNotice = () => {

    const bands_bit_mapping = [
        {bit: 1, band: "2G", text: "2.4 GHz"},
        {bit: 2, band: "5G", text: "5 GHz"},
        {bit: 4, band: "5G1", text: "5 GHz-1"},
        {bit: 8, band: "5G2", text: "5 GHz-2"},
        {bit: 16, band: "6G", text: "6 GHz"},
        {bit: 32, band: "6G1", text: "6 GHz-1"},
        {bit: 64, band: "6G2", text: "6 GHz-2"},
    ];

    const checkNvramList = [];
    const noticeContents = [];
    const apgInfo = [];
    const macInfo = [];

    const apg_arr = [];
    const apm_arr = [];
    const sdn_rl_arr = decodeURIComponent(httpApi.nvramCharToAscii(["sdn_rl"]).sdn_rl).split("<");
    sdn_rl_arr.forEach(item => {
        if(item){
            const sdn_item = item.split(">");
            const sdn_type = sdn_item[1];
            const ap_idx = sdn_item[5];
            const ap_prefix = (sdn_type == "MAINFH" || sdn_type == "MAINBH") ? "apm" : "apg";
            if(sdn_type != "DEFAULT" && sdn_type != "MAINBH"){
                if(ap_prefix == "apm"){
                    apm_arr.push(ap_idx);
                }
                else{
                    apg_arr.push(ap_idx);
                }
            }
        }
    });

    for (const i of apg_arr) {
        checkNvramList.push(`apg${i}_disabled`);
    }

    for (let i = 0; i < 4; i++) {
        checkNvramList.push(`wl${i}_macmode`);
    }

    const checkNvramListResult = httpApi.nvramGet(checkNvramList, false);

    for (const key in checkNvramListResult) {
        if (parseInt(checkNvramListResult[key]) === 1) {
            const match = key.match(/apg(\d+)_disabled/);
            if (match) {
                const index = parseInt(match[1]);
                const bandInfo = httpApi.nvramGet([`apg${index}_ssid`, `apg${index}_dut_list`], false);
                const matchBand = bandInfo[`apg${index}_dut_list`].match(/&#62(\d+)&#62/);
                if (matchBand) {
                    const apm_band_bitwise = matchBand[1];
                    const apm_band_info = (() => {
                        let band_info = {"band": [], "band_name": []};
                        bands_bit_mapping.forEach(({bit, band, text}) => {
                            if ((apm_band_bitwise & bit)) {
                                band_info.band.push(text);
                                band_info.band_name.push(band);
                            }
                        });
                        return band_info;
                    })();
                    const bandNameText = apm_band_info.band_name.join(", ");
                    apgInfo.push(`${bandInfo[`apg${index}_ssid`]} (${bandNameText})`);
                }
            }
        }
        const match = key.match(/wl(\d+)_macmode/);
        if (match) {
            const index = parseInt(match[1]);
            if (checkNvramListResult[key] !== "disabled" && checkNvramListResult[key] !== "") {
                macInfo.push({index: index, key: key, value: checkNvramListResult[key]});
            }
        }
    }

    if (apgInfo.length > 0) {
        const noticeTemplate = `Due to system optimization, the %@ network(s) have to be reconfigured. You can create new ones on Network settings.`;
        const noticeText = noticeTemplate.replace("%@", `<b>${apgInfo.join(', ')}</b>`);
        noticeContents.push(noticeText);
    }

    if (macInfo.length > 0) {
        const noticeText = `Due to system optimization, please help to add existing MAC filter again on Network settings.`;
        noticeContents.push(noticeText);
    }

    const noticeModal = new PopupModalWithScrollComponent({
        id: 'popup_notice_loader',
        title: `<#Notice#>`,
        content: `<ul>${noticeContents.map(message => `<li>${message}</li>`).join('')}</ul>`,
        showCloseBtn: false,
        applyBtn: {
            show: true,
            text: `<#CTL_ok#>`,
            clickCallBack: function () {
                noticeModal.hide();
                if (noticeModal.getReadCheck()) {
                    noticeModal.setRead();
                }
                const postData = {};
                for (const macmode of macInfo) {
                    postData[macmode.key] = "disabled";
                    postData[`wl${macmode.index}_maclist_x`] = "";
                }
                postData.action_mode = "apply";
                httpApi.nvramSet(postData);
            }
        },
        // theme: "", // RT, ROG, TUF
    });

    if (noticeContents.length > 0 && !noticeModal.isRead()) {
        noticeModal.show();
    }
}

showAsusNotice();