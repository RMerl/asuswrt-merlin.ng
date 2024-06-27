ASUS_POLICY = {
    Dict: {
        ScrollDown: `<#ASUS_POLICY_Scroll_Down#>`,
        AgeCheck: `<#ASUS_POLICY_Age_Check#>`,
        AgeConfirm: `<#ASUS_eula_age_confirm#>`,
        Agree: `<#CTL_Agree#>`,
        Disagree: `<#CTL_Disagree#>`,
        Notice: `<#Notice#>`,
        Update_Notice: `<#ASUS_POLICY_Update_Notice#>`,
        Notice1: `<#ASUS_POLICY_Notice_1#>`,
        Notice2: `<#ASUS_POLICY_Notice_2#>`,
        ReadAgain: `<#ASUS_POLICY_Read_Again#>`,
        KnowRisk: `<#ASUS_POLICY_Know_Risk#>`,
        QisDesc: `<#ASUS_POLICY_QIS_Desc#>`,
        Ok: `<#CTL_ok#>`,
        SelectLanguage: `<#selected_language#>`,
        ModalName: `<#Web_Title2#>`,
    }, Content: {
        PP: {
            Title: `<#ASUS_PP_Title#>`,
            Desc: `<#ASUS_PP_Desc#>`,
            HTML: `<style>
                    .policy_title{
                        font-weight: bold;
                        text-decoration: underline;
                        margin-bottom: 1em;
                    }
                    .policy_desc{
                        font-weight: normal;
                        margin-bottom: 1em;
                    }
                    .policy_end{
                        border: 1px solid #ddd;
                        padding: 5px;
                        margin: 1em;
                    }
                    .policy_law {
                        font-weight: normal;
                    }
                    .policy_law ol {
                        padding-left: 1em;
                    }
                    .policy_law ol ol{
                        padding-left: 2em;
                    }
                    .policy_law li{
                        margin-top: 1em;
                        margin-bottom: 1em;
                    }
                    .policy_law li li::marker{
                        content: '(' counter(list-item) ') ';
                    }
                </style>
                <div>
                    <div class="policy_title"><#ASUS_PP_Title#></u></b></div>
                    <div class="policy_desc"><#ASUS_PP_Desc#></div>
                    <div class="policy_law">
                        <ol>
                            <li><#ASUS_PP_Law_1#>
                                <ol>
                                    <li><#ASUS_PP_Law_1_1#></li>
                                    <li><#ASUS_PP_Law_1_2#><#ASUS_PP_Law_1_3#></li>
                                </ol>
                            </li>
                            <li><#ASUS_PP_Law_2#>
                                <ol>
                                    <li><#ASUS_PP_Law_2_1#></li>
                                    <li><#ASUS_PP_Law_2_2#></li>
                                    <li><#ASUS_PP_Law_2_3#></li>
                                </ol>
                            </li>
                            <li><#ASUS_PP_Law_3#>
                                <ol>
                                    <li><#ASUS_PP_Law_3_1#><#ASUS_PP_Law_3_2#></li>
                                    <li><#ASUS_PP_Law_3_3#></li>
                                </ol>
                            </li>
                            <li><#ASUS_PP_Law_4#></li>
                        </ol>
                        <div class="policy_end"><#ASUS_PP_End#></div>
                    </div>
                </div>`,
        },
        EULA: {
            Title: `<#ASUS_EULA_Title#>`,
            Desc: `<#ASUS_EULA_Desc#>`,
            HTML: `<style>
                    .policy_title{
                        font-weight: bold;
                        text-decoration: underline;
                        margin-bottom: 1em;
                    }
                    .policy_desc{
                        font-weight: normal;
                        margin-bottom: 1em;
                    }
                    .policy_law ol {
                        margin: 0 0.5em 0 0.5em;
                    }
                    .policy_law ol {
                        padding-left: 1em;
                    }
                    .policy_law ol ol{
                        padding-left: 2em;
                    }
                    .policy_law li{
                        margin-top: 1em;
                        margin-bottom: 1em;
                    }
                    .policy_law li li::marker{
                        content: counters(list-item,".",list-item)" ";
                    }
                    .policy_law > ol > li:first-child{
                        font-weight: bold;
                    }
                    .policy_law > ol > li > div:first-child{
                        font-weight: normal;
                    }
                    .policy_law > ol > li li{
                        font-weight: normal;
                    }
                    .font-weight-bold {
                        font-weight: bold;
                    }
                </style>
                <div>
                    <div class="policy_title"><#ASUS_EULA_Title_B#></u></b></div>
                    <div class="policy_desc"><#ASUS_EULA_Desc#></div>
                    <div class="policy_law">
                        <ol>
                            <li><#ASUS_EULA_Law_1#>
                                <ol>
                                    <li><b><#ASUS_EULA_Law_1_1#></b>
                                        <ol>
                                            <li><#ASUS_EULA_Law_1_1_1#></li>
                                            <li><#ASUS_EULA_Law_1_1_2#></li>
                                        </ol>
                                    </li>
                                    <li><b><#ASUS_EULA_Law_1_2#></b>
                                        <ol>
                                            <li><#ASUS_EULA_Law_1_2_1#></li>
                                            <li><#ASUS_EULA_Law_1_2_2#></li>
                                        </ol>
                                    </li>
                                    <li><b><#ASUS_EULA_Law_1_3#></b>
                                        <ol>
                                            <li><#ASUS_EULA_Law_1_3_1#></li>
                                            <li><#ASUS_EULA_Law_1_3_2#></li>
                                            <li><#ASUS_EULA_Law_1_3_3#></li>
                                        </ol>
                                    </li>
                                    <li><b><#ASUS_EULA_Law_1_4#></b>
                                        <div>
                                            <#ASUS_EULA_Law_1_4_Desc#>                                            
                                        </div>
                                    </li>
                                </ol>
                            </li>
                            <li><#ASUS_EULA_Law_2#>
                                <ol>
                                    <li><b><#ASUS_EULA_Law_2_1#></b>
                                        <ol>
                                            <li><#ASUS_EULA_Law_2_1_1#></li>
                                            <li><#ASUS_EULA_Law_2_1_2#></li>
                                        </ol>
                                    </li>
                                    <li><b><#ASUS_EULA_Law_2_2#></b>
                                        <ol>
                                            <li><#ASUS_EULA_Law_2_2_1#></li>
                                            <li><#ASUS_EULA_Law_2_2_2#></li>
                                        </ol>
                                    </li>
                                </ol>
                            </li>
                            <li><#ASUS_EULA_Law_3#>
                                <div><#ASUS_EULA_Law_3_Desc#></div>
                            </li>
                            <li><#ASUS_EULA_Law_4#>
                                <div><#ASUS_EULA_Law_4_Desc#></div>
                            </li>
                            <li><#ASUS_EULA_Law_5#>
                                <div><#ASUS_EULA_Law_5_Desc#></div>
                            </li>
                            <li><#ASUS_EULA_Law_6#>
                                <ol>
                                    <li><#ASUS_EULA_Law_6_1#></li>
                                    <li><#ASUS_EULA_Law_6_2#></li>
                                </ol>
                            </li>
                            <li><#ASUS_EULA_Law_7#>
                                <div><#ASUS_EULA_Law_7_Desc#></div>
                            </li>
                            <li><#ASUS_EULA_Law_8#>
                                <ol>
                                    <li><b><#ASUS_EULA_Law_8_1#></b>
                                        <ol>
                                            <li><#ASUS_EULA_Law_8_1_1#></li>
                                            <li><#ASUS_EULA_Law_8_1_2#></li>
                                        </ol>
                                    </li>
                                    <li><b><#ASUS_EULA_Law_8_2#></b>
                                        <div><#ASUS_EULA_Law_8_2_Desc#></div>
                                    </li>
                                    <li><#ASUS_EULA_Law_8_3#></li>
                                </ol>
                            </li>
                            <li><#ASUS_EULA_Law_9#>
                                <div><#ASUS_EULA_Law_9_Desc#></div>
                            </li>
                            <li><#ASUS_EULA_Law_10#>
                                <div><#ASUS_EULA_Law_10_Desc#></div>
                            </li>
                            <li><#ASUS_EULA_Law_11#>
                                <div><#ASUS_EULA_Law_11_Desc#></div>
                            </li>
                            <li><#ASUS_EULA_Law_12#>
                                <div><#ASUS_EULA_Law_12_Desc#></div>
                            </li>
                        </ol>
                    </div>
                </div>`,
        },
        TM: {
            Title: `<#lyra_TrendMicro_agreement#>`,
            HTML: `<div class="d-flex flex-column gap-1">
                        <div><#TM_eula_desc1#></div>
                        <div><#TM_eula_desc2#></div>
                        <div><#TM_privacy_policy#></div>
                        <div><#TM_data_collection#></div>
                        <div><#TM_eula_desc3#></div>
                    </div>`
        },
        THIRDPARTY_PP: {
            Title: `<#Notice#>`,
            HTML: `<div class="d-flex flex-column gap-1">
                        <div class="thirdparty-pp-desc1"><#Thirdparty_PP_Desc1#></div>
                        <div class="thirdparty-pp-desc2"><#Thirdparty_PP_Desc2#></div>
                        <div class="thirdparty-pp-desc3"><#Thirdparty_PP_Desc3#></div>
                    </div>`,
            Feature: {
                'Speedtest': {
                    url_text: `Ookla PRIVACY POLICY`,
                    url: `https://www.speedtest.net/about/privacy`,
                    company: `Ookla`
                },
                'WTFast': {
                    url_text: `AAA Internet Publishing Inc. PRIVACY POLICY`,
                    url: `https://www.wtfast.com/en/privacy-policy/`,
                    company: `AAA Internet Publishing Inc.`
                },
                '网易UU加速器': {
                    url_text: `网易游戏隐私政策`,
                    url: `https://unisdk.update.netease.com/html/latest_v90.html`,
                    company: `网易游戏隐私政策`
                },
                'Surfshark': {
                    url_text: `Surfshark B.V. PRIVACY POLICY`,
                    url: `https://surfshark.com/privacy`,
                    company: `Surfshark B.V`
                },
                'Alexa': {
                    url_text: `Alexa Terms of Use`,
                    url: `https://us.amazon.com/gp/help/customer/display.html?ref_=hp_left_v4_sib&nodeId=G201809740`,
                    company: `Amazon.com Services LLC`
                },
                'IFTTT': {
                    url_text: `IFTTT INC. PRIVACY POLICY`,
                    url: `https://ifttt.com/terms`,
                    company: `IFTTT INC.`
                },
                'Google Assistant': {
                    url_text: `Google LLC PRIVACY POLICY`,
                    url: `https://policies.google.com/privacy`,
                    company: `GOOGLE LLC`
                },
                'FileFlex': {
                    url_text: `Qnext PRIVACY POLICY`,
                    url: `https://fileflex.com/privacy-policy/`,
                    company: `Qnext through FileFlex`
                }
            }
        }
    },

    PolicyModalStyle: `<style>
        .popup_bg {
            position: fixed;
            top: 0;
            right: 0;
            bottom: 0;
            left: 0;
            z-index: 2000;
            backdrop-filter: blur(2px);
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
        .modal-title {
            color: #006ce1;
            font-weight: bold;
            font-size: 2em;
            margin-bottom: 0;
            line-height: 1.5;
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
        .policy-scroll-div {
            margin: 5px;
            overflow-y: auto;
            padding: 15px 12px 0 12px;
            border: 1px solid #ddd;
            border-radius: 5px;
            height: 30em;
            color: #000000;
        }
        
        .policy-scroll-div::-webkit-scrollbar {
            width: 10px;
        }
        
        .policy-scroll-div::-webkit-scrollbar-thumb {
            background-color: #7775;
            border-radius: 20px;
        }
        
        .policy-scroll-div::-webkit-scrollbar-thumb:hover {
            background-color: #7777;
        }
        
        .policy-scroll-div::-webkit-scrollbar-track:hover {
            background-color: #5555;
        }
        
        .policy-scroll-div a{
            color: #000000;
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
        .btn.disabled, .btn:disabled {
            opacity: .65;
            cursor: not-allowed;
        }
        .btn-block {
            display: block;
            width: 100%;
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
            color: #ff5722;
            padding: 5px 10px;
            border-radius: 5px;
            margin: 1em;
            text-align: center;
        }
        .d-flex {
            display: flex !important;
        }
        .flex-column {
            -webkit-box-orient: vertical !important;
            -webkit-box-direction: normal !important;
            -ms-flex-direction: column !important;
            flex-direction: column !important;
        }
        .gap-1 {
            gap: 1em;
        }
        .notice_title {
            display: flex !important;
            justify-content: center !important;
            font-size: 1.5em;
            color: #0A5EDB;
            font-weight: bold;
        }
        
        .notice_content {
            color: #000000;
        }
    
        .gg-spinner {
            transform: scale(1)
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
            border-top-color: currentColor
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
        
        .age-label {
            color: #000000;            
        }
        
        .speedtest_logo {
            height: 40px;
            background: url(../images/speedtest/speedtest.svg) no-repeat center;
        }
    
        @media (min-width: 576px) {
            .modal-dialog {
                max-width: 500px;
                margin: 5rem auto;
            }
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
            .policy-scroll-div,
            .policy-scroll-div a{
                color: #FFFFFF;
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
            .notice_title {
                color: #cf0a2c;
            }
            .notice_content {
                color: #FFFFFF;
            }
            .age-label {
                color: #FFFFFF;            
            }
        </style>`,

        RT: `<style>
            .modal-content {
                background-color: #2B373B;
            }
            .modal-title {
                color: #FFFFFF;
            }
            .policy-scroll-div,
            .policy-scroll-div a{
                color: #FFFFFF;
            }
            .notice_title {
                color: #FFFFFF;
            }
            .notice_content {
                color: #FFFFFF;
            }
            .age-label {
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
            .policy-scroll-div,
            .policy-scroll-div a{
                color: #FFFFFF;
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
            .notice_title {
                color: #ffa523;
            }
            .notice_content {
                color: #FFFFFF;
            }
            .age-label {
                color: #FFFFFF;            
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
        } else if (isSupport("BUSINESS")) {
            return "";
        } else {
            return theme;
        }
    }
}

function areScriptsLoaded(scriptUrls) {
    const scripts = document.getElementsByTagName('script');
    for (let i = 0; i < scriptUrls.length; i++) {
        const scriptUrl = scriptUrls[i];
        let isLoaded = false;
        for (let j = 0; j < scripts.length; j++) {
            if (scripts[j].src.indexOf(scriptUrl) > 0) {
                isLoaded = true;
                break;
            }
        }
        if (!isLoaded) {
            return false;
        }
    }
    return true;
}

const dependencies = [
    'httpApi.js',
];

if (!areScriptsLoaded(dependencies)) {
    console.log('Dependencies did not load');
}

policy_status = {
    "Policy_lang": "EN",
    "EULA": "0",
    "EULA_time": "",
    "PP": "0",
    "PP_time": "",
    "TM": "0",
    "TM_time": "",
};

async function PolicyStatus() {
    await httpApi.newEula.get()
        .then(data => {
            policy_status.EULA = data.ASUS_NEW_EULA;
            policy_status.EULA_time = data.ASUS_NEW_EULA_time;
        });

    await httpApi.privateEula.get()
        .then(data => {
            policy_status.PP = data.ASUS_PP;
            policy_status.PP_time = data.ASUS_PP_time;
        })

    const TM_EULA = await httpApi.nvramGet(['TM_EULA', 'TM_EULA_time'], true);
    policy_status.TM = TM_EULA.TM_EULA;
    policy_status.TM_time = TM_EULA.TM_EULA_time;

    policy_status.Policy_lang = await httpApi.nvramGet(['preferred_lang'], true).preferred_lang;
}

PolicyStatus();


class PolicyScrollDiv {

    constructor(props) {
        const {policy, theme = '', scrollCallBack} = props;
        const div = document.createElement('div');
        const shadowRoot = div.attachShadow({mode: 'open'});
        const template = document.createElement('template');
        template.innerHTML = `
      <style>
        .policy-scroll-div {
            background-color: #fff;
            margin: 5px;
            overflow-y: auto;
            padding: 15px 12px 0 12px;
            border: 1px solid #ddd;
            border-radius: 5px;
            height: 50vh;
            color: #333;
        }
        .policy-scroll-div a {
            color: #333;
        }
        .policy-scroll-div::-webkit-scrollbar {
            width: 10px;
            height: 10px;
        }
        
        .policy-scroll-div::-webkit-scrollbar-thumb {
            background-color: #7775;
            border-radius: 20px;
        }
        
        .policy-scroll-div::-webkit-scrollbar-thumb:hover {
            background-color: #7777;
        }
        
        .policy-scroll-div::-webkit-scrollbar-track:hover {
            background-color: #5555;
        }
        
        .policy-scroll-div.ROG,
        .policy-scroll-div.TUF{
            border-radius: 0;
        }
        
        .policy-scroll-div.RT,
        .policy-scroll-div.ROG,
        .policy-scroll-div.TUF{
            border: 1px solid #4D4D4D;
            background-color: #181818;
            color: #FFFFFF;
        }
        
        .policy-scroll-div.RT a,
        .policy-scroll-div.ROG a,
        .policy-scroll-div.TUF a{
            color: #FFFFFF;
        }
      </style>
      <div class="policy-scroll-div ${theme}"></div>
    `;

        shadowRoot.appendChild(template.content.cloneNode(true));
        shadowRoot.querySelector('.policy-scroll-div').addEventListener('click', this.handleClick.bind(this));
        shadowRoot.querySelector('.policy-scroll-div').addEventListener('scroll', scrollCallBack.bind(this));
        if (policy) {
            shadowRoot.querySelector('.policy-scroll-div').innerHTML = ASUS_POLICY.Content[policy].HTML;
        }

        this.element = div;
    }

    render() {
        return this.element
    }

    reset() {
        this.element.shadowRoot.querySelector('.policy-scroll-div').scrollTop = 0;
    }

    handleClick() {
        const scrollDiv = this.element.shadowRoot.querySelector('.policy-scroll-div');
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

class PolicyPopupBg extends HTMLElement {
    constructor() {
        super();
        this.attachShadow({mode: 'open'});

        const template = document.createElement('template');
        template.innerHTML = `
      <style>
        .policy_popup_pg {
            position: absolute;
            top: 0;
            left: 0;
            height: 100vh;
            width: 100vw;
            z-index: 9;
            backdrop-filter: blur(2px);
        }
      </style>
      <div class="policy_popup_pg"><slot></slot></div>
    `;

        this.shadowRoot.appendChild(template.content.cloneNode(true));
        this.div = this.shadowRoot.querySelector('div');
    }

    connectedCallback() {


    }
}

class PolicyModal extends HTMLElement {
    constructor() {
        super();
        this.attachShadow({mode: 'open'});

        const template = document.createElement('template');
        template.innerHTML = `
          ${ASUS_POLICY.PolicyModalStyle}
          <div class="popup_bg">
            <div class="modal">
                <div class="modal-dialog">
                    <div class="modal-content">
                        <div class="modal-header">
                            <div class="modal-title"></div>
                        </div>
                        <div class="modal-body">
                            <div class="policy-scroll-div"></div>
                        </div>
                        <div class="modal-footer">
                        </div>
                    </div>
                </div>
            </div>
          </div>
        `;

        const theme = ASUS_POLICY.getTheme();
        if (theme !== "") {
            template.innerHTML += ASUS_POLICY.ModalTheme[theme];
        }

        this.shadowRoot.appendChild(template.content.cloneNode(true));
        this.div = this.shadowRoot.querySelector('div');
        this.div.querySelector('div.policy-scroll-div').addEventListener('click', this.handleScrollDivClick.bind(this.div.querySelector('div.policy-scroll-div')));
        this.div.querySelector('div.policy-scroll-div').addEventListener('scroll', this.handleScrollCheck.bind(this));

        this.ageCheckbox = document.createElement('input');
        this.ageCheckbox.type = 'checkbox';
        this.ageCheckbox.id = 'ASUS_EULA_enable';

        this.ageLabel = document.createElement('label');
        this.ageLabel.classList.add('age-label')
        this.ageLabel.setAttribute('for', 'ASUS_EULA_enable');
        this.ageLabel.textContent = ASUS_POLICY.Dict.AgeCheck;

        this.ageCheckDiv = document.createElement('div');
        this.ageCheckDiv.style.width = '100%';
        this.ageCheckDiv.appendChild(this.ageCheckbox);
        this.ageCheckDiv.appendChild(this.ageLabel);

        this.loadingImg = document.createElement('i');
        this.loadingImg.className = 'gg-spinner';

        this.closeBtn = document.createElement('button');
        this.closeBtn.type = 'button';
        this.closeBtn.className = 'btn btn-primary btn-block close';
        this.closeBtn.innerHTML = ASUS_POLICY.Dict.Ok;
        this.closeBtn.addEventListener('click', this.handleClickClose.bind(this));

        this.agreeBtn = document.createElement('button');
        this.agreeBtn.type = 'button';
        this.agreeBtn.className = 'btn btn-primary agree disabled';
        this.agreeBtn.innerHTML = ASUS_POLICY.Dict.Agree;
        this.agreeBtn.addEventListener('click', this.handleClickAgree.bind(this));


        this.disagreeBtn = document.createElement('button');
        this.disagreeBtn.type = 'button';
        this.disagreeBtn.className = 'btn btn-secondary disagree disabled';
        this.disagreeBtn.innerHTML = ASUS_POLICY.Dict.Disagree;
        this.disagreeBtn.addEventListener('click', this.handleClickDisagree.bind(this));

    }

    agreeCallback = () => {
    };
    disagreeCallback = () => {
    };

    setAgreeCallback(callback) {
        if (typeof callback === "function") {
            this.agreeCallback = callback;
        }
    }

    setDisagreeCallback(callback) {
        if (typeof callback === "function") {
            this.disagreeCallback = callback;
        }
    }

    readAgainCallback = () => {
    };
    knowRiskCallback = () => {
    };

    setReadAgainCallback(callback) {
        if (typeof callback === "function") {
            this.readAgainCallback = callback;
        }
    }

    setKnowRiskCallback(callback) {
        if (typeof callback === "function") {
            this.knowRiskCallback = callback;
        }
    }

    policy_type = "";
    submit_reload = "0";

    connectedCallback() {
        const {policy, submit_reload, party} = this.attributes;

        if (policy) {
            this.policy_type = policy.value;

            this.div.querySelector('div.modal-title').innerHTML = ASUS_POLICY.Content[policy.value].Title;
            this.div.querySelector('div.policy-scroll-div').innerHTML = ASUS_POLICY.Content[policy.value].HTML;


            switch (policy.value) {
                case "EULA":
                    if (policy_status.EULA === "0") {
                        const scrollInfoDiv = document.createElement('div');
                        scrollInfoDiv.className = "scroll-info";
                        scrollInfoDiv.innerHTML = ASUS_POLICY.Dict.ScrollDown;

                        this.div.querySelector('div.modal-body').appendChild(scrollInfoDiv);
                        this.div.querySelector('div.modal-body').appendChild(this.ageCheckDiv);
                        this.div.querySelector('div.modal-footer').appendChild(this.agreeBtn);
                    } else {
                        this.div.querySelector('div.modal-footer').appendChild(this.closeBtn);
                    }
                    break;
                case "PP":
                    if (policy_status.PP < "2" || policy_status.PP_time === "") {
                        const scrollInfoDiv = document.createElement('div');
                        scrollInfoDiv.className = "scroll-info";
                        scrollInfoDiv.innerHTML = ASUS_POLICY.Dict.ScrollDown;

                        this.div.querySelector('div.modal-body').appendChild(scrollInfoDiv);
                        this.div.querySelector('div.modal-footer').appendChild(this.disagreeBtn);
                        this.div.querySelector('div.modal-footer').appendChild(this.agreeBtn);
                    } else {
                        this.div.querySelector('div.modal-footer').appendChild(this.closeBtn);
                    }
                    break;
                case "TM":
                    this.div.querySelector('div.policy-scroll-div').querySelector('#tm_eula_url').setAttribute("href", "https://nw-dlcdnet.asus.com/trend/tm_privacy");
                    this.div.querySelector('div.policy-scroll-div').querySelector('#tm_disclosure_url').setAttribute("href", "https://nw-dlcdnet.asus.com/trend/tm_pdcd");
                    this.div.querySelector('div.policy-scroll-div').querySelector('#eula_url').setAttribute("href", `https://nw-dlcdnet.asus.com/support/forward.html?model=&type=TMeula&lang=${policy_status.Policy_lang}&kw=&num=`);
                    if (policy_status.TM === "0" || policy_status.TM_time === "") {
                        this.agreeBtn.classList.remove("disabled");
                        this.disagreeBtn.classList.remove("disabled");
                        this.div.querySelector('div.modal-footer').appendChild(this.disagreeBtn);
                        this.div.querySelector('div.modal-footer').appendChild(this.agreeBtn);
                    } else {
                        this.div.querySelector('div.modal-footer').appendChild(this.closeBtn);
                    }
                    break;
                case "THIRDPARTY_PP":
                    this.div.querySelector('div.policy-scroll-div .thirdparty-pp-desc1').innerHTML = this.div.querySelector('div.policy-scroll-div .thirdparty-pp-desc1').innerHTML.replace("%1$@", party.value).replace("[aa]%2$@[/aa]", `<a target="_blank" href="${ASUS_POLICY.Content[policy.value].Feature[party.value].url}" style="text-decoration: underline;cursor: pointer;">${ASUS_POLICY.Content[policy.value].Feature[party.value].url_text}</a>`);
                    this.div.querySelector('div.policy-scroll-div .thirdparty-pp-desc2').innerHTML = this.div.querySelector('div.policy-scroll-div .thirdparty-pp-desc2').innerHTML.replaceAll("%1$@", `${ASUS_POLICY.Content[policy.value].Feature[party.value].company}`).replaceAll("%2$@", party.value);
                    this.div.querySelector('div.policy-scroll-div .thirdparty-pp-desc3').innerHTML = this.div.querySelector('div.policy-scroll-div .thirdparty-pp-desc3').innerHTML.replaceAll("%1$@", `${ASUS_POLICY.Content[policy.value].Feature[party.value].company}`).replaceAll("%2$@", party.value);
                    this.div.querySelector('div.modal-footer').appendChild(this.closeBtn);
                    if (party.value == 'Speedtest') {
                        const speedtestDesc = document.createElement('div');
                        speedtestDesc.innerHTML = `<#InternetSpeed_desc#>`;
                        const referenceElement = this.div.querySelector('div.policy-scroll-div .thirdparty-pp-desc1');
                        referenceElement.parentNode.insertBefore(speedtestDesc, referenceElement);
                        const speedtestLogo = document.createElement('div');
                        speedtestLogo.innerHTML = `<div class="speedtest_logo"></div>`;
                        this.div.querySelector('div.policy-scroll-div').appendChild(speedtestLogo);
                    }
                    break;
            }
        }
        if (submit_reload) {
            this.submit_reload = submit_reload.value;
        }
    }

    enableBtn = function (btn, boolean) {
        if (btn !== null) {
            if (boolean) {
                btn.classList.remove("disabled");
            } else {
                btn.classList.add("disabled");
            }
        }
    }

    handleScrollDivClick() {
        const scrollDiv = this;
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

    handleScrollCheck() {
        const scrollDiv = this.div.querySelector('div.policy-scroll-div');
        const scrollTop = scrollDiv.scrollTop;
        const offsetHeight = scrollDiv.offsetHeight;
        const scrollHeight = scrollDiv.scrollHeight;
        if ((scrollTop + offsetHeight) >= scrollHeight) {
            this.enableBtn(this.agreeBtn, true);
            this.enableBtn(this.disagreeBtn, true);
        }
    }

    handleClickClose() {
        top.document.body.style.removeProperty('overflow');
        this.closest('#policy_popup_modal').remove();
    }

    handleClickAgree(e) {
        if (!e.target.classList.contains("disabled")) {
            switch (this.policy_type) {
                case "EULA":
                    let yearChecked = this.ageCheckbox.checked;
                    if (!yearChecked) {
                        alert(ASUS_POLICY.Dict.AgeConfirm);
                        this.ageLabel.style.color = top.businessWrapper ? "red" : "#ff5722";
                        return false;
                    } else {
                        this.agreeCallback();
                        this.agreeBtn.innerHTML = '';
                        this.agreeBtn.appendChild(this.loadingImg);
                        httpApi.newEula.set(1, () => {
                            top.document.body.style.removeProperty('overflow');
                            this.remove();
                            if (this.submit_reload == 1) {
                                top.window.location.reload();
                            }
                        })
                    }
                    break
                case "PP":
                    this.agreeCallback();
                    this.agreeBtn.innerHTML = '';
                    this.agreeBtn.appendChild(this.loadingImg);
                    httpApi.privateEula.set("1", () => {
                        top.document.body.style.removeProperty('overflow');
                        this.remove();
                        if (this.submit_reload == 1) {
                            top.window.location.reload();
                        }
                    })
                    break
                case "TM":
                    this.agreeCallback();
                    this.agreeBtn.innerHTML = '';
                    this.agreeBtn.appendChild(this.loadingImg);
                    httpApi.enableEula('tm', "1", () => {
                        top.document.body.style.removeProperty('overflow');
                        this.remove();
                        if (this.submit_reload == 1) {
                            top.window.location.reload();
                        }
                    })
            }
        }
    }

    handleClickDisagree(e) {
        if (!e.target.classList.contains("disabled")) {
            switch (this.policy_type) {
                case "PP":
                    const parentNode = this.parentNode;
                    parentNode.removeChild(this);
                    this.disagreeCallback();
                    const policyModal = new PolicyWithdrawModalComponent({
                        policy: "PP",
                        submit_reload: this.submit_reload,
                        agreeCallback: this.agreeCallback,
                        disagreeCallback: this.disagreeCallback,
                        readAgainCallback: this.readAgainCallback,
                        knowRiskCallback: this.knowRiskCallback
                    });
                    parentNode.appendChild(policyModal.render());
                    break
                case "TM":
                    this.disagreeCallback();
                    httpApi.enableEula("tm", "0", () => {
                        top.document.body.style.removeProperty('overflow');
                        this.remove();
                        if (this.submit_reload == 1) {
                            top.window.location.reload();
                        }
                    })
                    break
            }
        }
    }
}

class DisagreeNoticeComponent {
    constructor(props) {
        this.props = props;
        this.element = document.createElement('div');
        this.element.innerHTML = `
            ${ASUS_POLICY.PolicyModalStyle}
            <div class="d-flex flex-column gap-1">
                <div class="notice_title">${ASUS_POLICY.Dict.Notice}</div>
                <div class="d-flex flex-column gap-1" style="margin: 0 10px;">
                    <div class="notice_content">${ASUS_POLICY.Dict.Notice1}</div>
                    <div style="color: #ff5722">${ASUS_POLICY.Dict.Notice2}</div>
                    <div style="color: #ff5722">
                        <ul>
                            <li>Account Binding</li>
                            <li>Config transfer</li>
                            <li>DDNS</li>
                            <li>Alexa</li>
                            <li>IFTTT</li>
                            <li>Google Assistant</li>
                            <li>Remote Connection</li>
                            <li>Notification</li>
                        </ul>
                    </div>
                </div>
            </div>`;

        const theme = ASUS_POLICY.getTheme();
        if (theme !== "") {
            this.element.innerHTML += ASUS_POLICY.ModalTheme[theme];
        }
    }

    render() {
        return this.element;
    }
}

class PolicyWithdrawModal extends HTMLElement {
    constructor() {
        super();
        this.attachShadow({mode: 'open'});

        const template = document.createElement('template');
        template.innerHTML = `
          ${ASUS_POLICY.PolicyModalStyle}
          <div class="popup_bg">
            <div class="modal">
                <div class="modal-dialog">
                    <div class="modal-content">
                        <div class="modal-body">
                        </div>
                        <div class="modal-footer">
                            <button type="button" class="btn btn-secondary know-risk">${ASUS_POLICY.Dict.KnowRisk}</button>
                            <button type="button" class="btn btn-primary read-again">${ASUS_POLICY.Dict.ReadAgain}</button>
                        </div>
                    </div>
                </div>
            </div>
          </div>
        `;

        const theme = ASUS_POLICY.getTheme();
        if (theme !== "") {
            template.innerHTML += ASUS_POLICY.ModalTheme[theme];
        }

        this.loadingImg = document.createElement('i');
        this.loadingImg.className = 'gg-spinner';

        this.shadowRoot.appendChild(template.content.cloneNode(true));
        this.div = this.shadowRoot.querySelector('div');
        this.div.querySelector('button.know-risk').addEventListener('click', this.handleClickKnowRisk.bind(this));
        this.div.querySelector('button.read-again').addEventListener('click', this.handleClickReadAgain.bind(this));
    }

    submit_reload = '';

    agreeCallback = () => {
    };
    disagreeCallback = () => {
    };

    setAgreeCallback(callback) {
        if (typeof callback === "function") {
            this.agreeCallback = callback;
        }
    }

    setDisagreeCallback(callback) {
        if (typeof callback === "function") {
            this.disagreeCallback = callback;
        }
    }

    readAgainCallback = () => {
    };
    knowRiskCallback = () => {
    };

    setReadAgainCallback(callback) {
        if (typeof callback === "function") {
            this.readAgainCallback = callback;
        }
    }

    setKnowRiskCallback(callback) {
        if (typeof callback === "function") {
            this.knowRiskCallback = callback;
        }
    }

    connectedCallback() {
        const {submit_reload = '0'} = this.attributes;
        const disagreeNoticeComponent = new DisagreeNoticeComponent();
        this.div.querySelector('div.modal-body').appendChild(disagreeNoticeComponent.render());

        if (submit_reload) {
            this.submit_reload = submit_reload.value;
        }
    }

    handleClickKnowRisk(e) {
        e.target.innerHTML = '';
        e.target.appendChild(this.loadingImg);
        this.knowRiskCallback();
        httpApi.privateEula.set("0", () => {
            top.document.body.style.removeProperty('overflow')
            this.remove();
            if (this.submit_reload == 1) {
                top.window.location.reload();
            }
        })
    }

    handleClickReadAgain() {
        const parentNode = this.parentNode;
        this.remove();
        this.readAgainCallback();
        if (window.location.pathname.toUpperCase().search("QIS_") < 0) {
            const policyModal = new PolicyModalComponent({
                policy: "PP",
                submit_reload: this.submit_reload,
                agreeCallback: this.agreeCallback,
                disagreeCallback: this.disagreeCallback,
                readAgainCallback: this.readAgainCallback,
                knowRiskCallback: this.knowRiskCallback
            });
            parentNode.appendChild(policyModal.render());
        }
    }
}

class PolicyUpdateModal extends HTMLElement {
    constructor() {
        super();
        this.attachShadow({mode: 'open'});

        const template = document.createElement('template');
        template.innerHTML = `
          ${ASUS_POLICY.PolicyModalStyle}
          <div class="popup_bg">
            <div class="modal">
                <div class="modal-dialog">
                    <div class="modal-content">
                        <div class="modal-body">
                            <div class="d-flex flex-column gap-1">
                                <div class="notice_title">${ASUS_POLICY.Dict.Notice}</div>
                                <div class="notice_content">${ASUS_POLICY.Dict.Update_Notice}</div>
                            </div>
                        </div>
                        <div class="modal-footer">
                            <button type="button" class="btn btn-primary ok">${ASUS_POLICY.Dict.Ok}</button>
                        </div>
                    </div>
                </div>
            </div>
          </div>
        `;
        const theme = ASUS_POLICY.getTheme();
        if (theme !== "") {
            template.innerHTML += ASUS_POLICY.ModalTheme[theme];
        }

        this.shadowRoot.appendChild(template.content.cloneNode(true));
        this.div = this.shadowRoot.querySelector('div');
        this.div.querySelector('button.ok').addEventListener('click', this.handleClickReadEula.bind(this));
    }

    connectedCallback() {

    }

    handleClickReadEula() {
        const parentNode = this.parentNode;
        this.remove();
        const policyModal = document.createElement('policy-modal');
        policyModal.setAttribute('policy', 'EULA');
        policyModal.setAttribute('submit_reload', '1');
        policyModal.style.display = 'block';
        parentNode.appendChild(policyModal);
    }
}

customElements.define('policy-popup-bg', PolicyPopupBg);
customElements.define('policy-modal', PolicyModal);
customElements.define('policy-withdraw-modal', PolicyWithdrawModal);
customElements.define('policy-update-modal', PolicyUpdateModal);

class PolicyModalComponent {
    constructor(props) {
        this.props = props;

        const policy_modal = document.createElement('policy-modal');
        policy_modal.setAttribute('policy', this.props.policy);
        policy_modal.setAttribute('submit_reload', this.props.submit_reload);
        this.element = policy_modal;

        this.element.setAgreeCallback(this.props.agreeCallback);
        this.element.setDisagreeCallback(this.props.disagreeCallback);
        this.element.setReadAgainCallback(this.props.readAgainCallback);
        this.element.setKnowRiskCallback(this.props.knowRiskCallback);
        top.document.body.style.overflow = 'hidden';
    }

    render() {
        return this.element;
    }

    show() {
        if (top.document.getElementById('policy_popup_modal') == null) {
            const policy_popup_modal = document.createElement('div');
            policy_popup_modal.id = 'policy_popup_modal';
            top.document.body.appendChild(policy_popup_modal);
            policy_popup_modal.appendChild(this.element);
        }
    }
}

class PolicyWithdrawModalComponent {
    constructor(props) {
        this.props = props;

        const policy_modal = document.createElement('policy-withdraw-modal');
        policy_modal.setAttribute('policy', this.props.policy);
        policy_modal.setAttribute('submit_reload', this.props.submit_reload);
        this.element = policy_modal;

        this.element.setAgreeCallback(this.props.agreeCallback);
        this.element.setDisagreeCallback(this.props.disagreeCallback);
        this.element.setReadAgainCallback(this.props.readAgainCallback);
        this.element.setKnowRiskCallback(this.props.knowRiskCallback);
        top.document.body.style.overflow = 'hidden';
    }

    render() {
        return this.element;
    }

    show() {
        if (top.document.getElementById('policy_popup_modal') == null) {
            const policy_popup_modal = document.createElement('div');
            policy_popup_modal.id = 'policy_popup_modal';
            top.document.body.appendChild(policy_popup_modal);
            policy_popup_modal.appendChild(this.element);
        }
    }
}

class PolicyUpdateModalComponent {
    constructor(props) {
        this.props = props;
        this.element = document.createElement('policy-update-modal');
        top.document.body.style.overflow = 'hidden';
    }

    render() {
        return this.element;
    }

    show() {
        if (top.document.getElementById('policy_popup_modal') == null) {
            const policy_popup_modal = document.createElement('div');
            policy_popup_modal.id = 'policy_popup_modal';
            top.document.body.appendChild(policy_popup_modal);
            policy_popup_modal.appendChild(this.element);
        }
    }
}

class ThirdPartyPolicyModalComponent {
    constructor(props) {
        this.props = props;

        const policy_modal = document.createElement('policy-modal');
        policy_modal.setAttribute('policy', this.props.policy);
        policy_modal.setAttribute('party', this.props.party);
        this.element = policy_modal;

        this.element.setAgreeCallback(this.props.agreeCallback);
        this.element.setDisagreeCallback(this.props.disagreeCallback);
        this.element.setReadAgainCallback(this.props.readAgainCallback);
        this.element.setKnowRiskCallback(this.props.knowRiskCallback);
        top.document.body.style.overflow = 'hidden';
    }

    render() {
        return this.element;
    }

    show() {
        if (top.document.getElementById('policy_popup_modal') == null) {
            const policy_popup_modal = document.createElement('div');
            policy_popup_modal.id = 'policy_popup_modal';
            top.document.body.appendChild(policy_popup_modal);
            policy_popup_modal.appendChild(this.element);
        }
    }
}

class QisPolicyPageComponent {
    constructor(props) {
        const {policy} = props;
        const div = document.createElement('div');
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
                .bg {
                    background-color: #F5F5F5;
                    width: 100%;
                    height: 100%;
                    background-size: cover;
                    min-height: 100vh
                }
                .header {
                    height: 60px;
                    background-color: #006CE1;
                    display: flex;
                    align-items: center;
                    justify-content: space-between;
                }
                .header .header-title{
                    display: flex;
                    align-items: center;
                    height: inherit;
                }
                .header .header-name{
                    display: none;
                }
                .icon-logo {
                    --logo-svg: url('/mobile.customize/logo_asus_business.svg');
                    width: 260px;
                    min-width: 150px;
                    height: 100%;
                    mask-image: var(--logo-svg);
                    mask-repeat: no-repeat;
                    mask-position: center;
                    mask-size: contain;
                    -webkit-mask-image: var(--logo-svg);
                    -webkit-mask-repeat: no-repeat;
                    -webkit-mask-position: center;
                    -webkit-mask-size: contain;
                    background-color: #FFF;
                    margin: 0 10px 0 25px;            
                }
                .container{
                    width: 100%;
                    display: flex;
                    justify-content: center;
                }
                .page {
                    width: 90%;
                    display: flex;
                    justify-content: center;
                    flex-direction: column;
                    margin: 2em 0;
                }
                .page-title{
                    font-size: 2em;
                    text-align: left;
                    font-weight: bold;
                    color: var(--business-primary);
                    word-break: break-word;
                }
                
                .page-content{
                    margin-bottom: 80px;
                }
                
                .page-desc{
                    font-size: 1em;
                    color: #000;
                    font-weight: bold;
                    margin: 16px 0;
                }
                .scroll-info {
                    font-weight: bold;
                    color: var(--business-notice);
                    padding: 5px 10px;
                    border-radius: 5px;
                    margin: 1em;
                    text-align: center;
                }
                
                .page-footer {
                    display: flex;
                    flex-direction: row;
                    justify-content: space-evenly;
                    position: fixed;
                    bottom: 0;
                    left: 0;
                    width: 100%;
                    z-index: 2;
                }

                .btn{
                    text-align: center;
                    font-weight: bold;
                    display: flex;
                    align-items: center;
                    justify-content: center;
                    font-size:1.5em;
                    color: #FFF;
                    cursor:pointer;
                    max-width: 300px;
                    width: 50%;
                    height: 48px;
                    background: #007AFF;
                }
                
                .btn.disagree {
                    border: 1px solid var(--business-primary);
                    color: var(--business-primary);
                    background-color: #FFF;
                }
                
                .btn.agree {
                    border: 1px solid var(--business-primary);
                }

                .btn.disabled {
                    box-shadow: none;
                    background-color: transparent;
                    border: 1px solid #999999;
                    cursor: not-allowed;
                    color: #818181;
                }
                
                .btn.disabled.agree {
                    border: 1px solid #CCE4FF;
                    color: #248DFF;
                    background-color: #CCE4FF;
                }
                
                .btn.disabled.disagree {
                    background-color: #FFF;
                }
                
                .btn.abort{
                    background: #FFFFFF;
                    color: #007AFF;
                }
                
                .policy-age {
                    margin: 1em 0;
                    font-size: 1.5em;
                    font-weight: bold;
                }
                
                .checkbox-wrapper-40 {
                    --borderColor: var(--business-primary);
                    --borderWidth: .125em;
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
                
                @keyframes spinner-two-alt {
                    0% {
                        transform: rotate(0deg)
                    }
                    to {
                        transform: rotate(359deg)
                    }
                }
                
                .gg-spinner-two-alt, .gg-spinner-two-alt::before {
                    box-sizing: border-box;
                    display: block;
                    width: 20px;
                    height: 20px
                }
                
                .gg-spinner-two-alt {
                    transform: scale(1.5);
                    display: inline-block;
                }
                
                .gg-spinner-two-alt::before {
                    content: "";
                    position: absolute;
                    border-radius: 100px;
                    animation: spinner-two-alt 1s cubic-bezier(.6, 0, .4, 1) infinite;
                    border: 3px solid transparent;
                    border-bottom-color: currentColor;
                    border-top-color: currentColor
                }
                
                .toolbar-btn {
                    display: flex;
                    flex-direction: row;
                    align-items: center;
                    gap: 5px;
                    color: #FFF;
                    margin: 10px 25px;
                    cursor: pointer;
                    background: transparent;
                    border: none;
                    min-width: max-content;
                }
                
                .lang-icon {
                    --lang-icon-svg: url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' height='48' width='48'%3E%3Cpath d='M24 45.25q-4.45 0-8.325-1.65-3.875-1.65-6.75-4.55T4.4 32.25q-1.65-3.9-1.65-8.35 0-4.45 1.65-8.325Q6.05 11.7 8.925 8.85 11.8 6 15.675 4.35T24 2.7q4.45 0 8.325 1.65Q36.2 6 39.075 8.85t4.55 6.725Q45.3 19.45 45.3 23.9q0 4.45-1.675 8.35-1.675 3.9-4.55 6.8t-6.75 4.55Q28.45 45.25 24 45.25zm-.05-4.55q1.6-1.75 2.75-4.025 1.15-2.275 1.95-5.475H19.4q.65 3 1.8 5.35 1.15 2.35 2.75 4.15zm-4.1-.6q-1.3-1.8-2.125-4T16.3 31.2H9q1.8 3.5 4.25 5.475 2.45 1.975 6.6 3.425zm8.25-.05q3.5-1.1 6.325-3.375T38.95 31.2h-7.2q-.65 2.65-1.5 4.85-.85 2.2-2.15 4zM8.05 28.35h7.7q-.15-1.35-.175-2.4-.025-1.05-.025-2.05 0-1.25.05-2.175.05-.925.2-2.125H8.05q-.35 1.2-.475 2.1-.125.9-.125 2.2 0 1.25.125 2.25t.475 2.2zm10.8 0H29.2q.15-1.55.175-2.5.025-.95.025-1.95t-.025-1.875q-.025-.875-.175-2.425H18.85q-.2 1.55-.25 2.425-.05.875-.05 1.875t.05 1.95q.05.95.25 2.5zm13.35 0h7.7q.3-1.2.45-2.2.15-1 .15-2.25 0-1.3-.15-2.2-.15-.9-.45-2.1h-7.65q.05 1.8.1 2.65.05.85.05 1.65 0 1.05-.075 2t-.125 2.45zm-.5-11.65h7.25q-1.6-3.35-4.425-5.625Q31.7 8.8 28.05 7.9q1.3 1.8 2.15 3.925.85 2.125 1.5 4.875zm-12.3 0h9.3q-.5-2.55-1.8-5.025Q25.6 9.2 23.95 7.4q-1.45 1.3-2.525 3.45Q20.35 13 19.4 16.7zM9 16.7h7.35q.55-2.6 1.325-4.7.775-2.1 2.125-4.05-3.7.9-6.425 3.1Q10.65 13.25 9 16.7z'/%3E%3C/svg%3E"); 
                    width: 20px;
                    height: 20px;
                    mask-image: var(--lang-icon-svg); 
                    mask-repeat: no-repeat;
                    mask-position: center;
                    mask-size: contain;
                    -webkit-mask-image: var(--lang-icon-svg);
                    -webkit-mask-repeat: no-repeat;
                    -webkit-mask-position: center;
                    -webkit-mask-size: contain;
                    background-color: #FFF;
                }
                
                .dropdown-menu{
                    flex-direction: row;
                    position: absolute;
                    z-index: 1000;
                    display: none;
                    min-width: 10rem;
                    margin: 0;
                    font-size: 1rem;
                    color: #212529;
                    text-align: left;
                    list-style: none;
                    background-color: #F5F5F5;
                    background-clip: padding-box;
                    border: 1px solid rgba(0, 0, 0, .15);
                    border-radius: 8px;
                    right: 10px;
                    box-shadow: 0px 2px 4px 0px #0000001A;
                    gap: 5px;
                    padding: 15px;
                    width: 90%;
                    left: 0;
                }
                
                .dropdown-menu.show {
                    display: flex;
                }
                
                .dropdown-menu ul{
                    list-style-type: none;
                    margin: 0;
                    padding: 0;
                    min-width: 150px;
                    width: 50%;
                }
                
                .dropdown-menu ul > li{
                    padding: 8px;
                    cursor: pointer;
                    font-size: 14px;
                    color: #262626;
                    text-align: inherit;
                    text-decoration: none;
                    white-space: nowrap;
                }
                
                .dropdown-menu ul > li:hover {
                    background-color: #DCDCDC;
                    border-radius: 4px;
                    color: #181818;
                }
                

                @media screen and (min-width: 576px){
                    
                    .page {
                        width: 70%;
                    }
                    
                    .page-title{
                        font-size: 3em;
                    }
                    
                    .page-desc{
                        font-size: 1.8em;
                    }
                    
                    .page-content{
                        margin-bottom: unset;
                    }
                    
                    .page-footer{
                        border: 0;
                        background: unset;
                        position: relative;
                        display: flex;
                        justify-content: space-around;
                        margin: 0;
                        gap: 1em;
                    }
                    
                    .btn{
                        font-weight: bold;
                        align-items: center;
                        justify-content: center;
                        border-radius: 35px;
                        font-size:1.5em;
                        color: #FFF;
                        cursor:pointer;
                        min-width: 200px;
                        max-width: 300px;
                        width: 25%;
                        height: 48px;
                        padding: 4px 0;
                    }
                    
                    .btn:hover{
                        box-shadow: 0 0 0 4px #61ADFF40;
                    }
                    
                    .btn.disabled:hover{
                        box-shadow: none;
                    }
                    
                    .dropdown-menu {
                        width: inherit;
                        left: unset;
                    }
                }
                
                @media screen and (min-width: 768px){
                    .page {
                        width: 80%;
                    }
                }
                
                @media screen and (min-width: 1200px){
                    .page {
                        width: 70%;
                        flex-direction: row;
                        gap: 30px;
                    }
                    .page-title {
                        text-align: right;
                        min-width: 300px;
                    }
                }
                
                .bg.RT {
                    background-color: #000;
                }
                .bg.RT .header{
                    background-color: var(--rt-primary);
                }
                .bg.RT .header .header-name{
                    display: block;
                    color: #FFF;
                }
                .bg.RT .icon-logo{
                    background-color: #FFF;
                    --logo-svg: url("data:image/svg+xml,%3Csvg width='103' height='20' viewBox='0 0 103 20' fill='none' xmlns='http://www.w3.org/2000/svg'%3E%3Cpath fill-rule='evenodd' clip-rule='evenodd' d='M71.292 20h19.235c4.825-.275 5.507-4.803 5.507-4.803.17-1.191.03-2.16.03-2.16-.139-1.072-1.68-4.292-4.97-4.604-1.95-.18-19.488-1.205-19.488-1.205.519 1.785 1.217 2.325 1.744 2.819 1.213 1.156 3.032 1.433 3.032 1.433L89.96 12.6c.423.023 1.266.066 1.266 1.078 0 .306-.157.622-.352.795 0 0-.463.252-.894.252H71.305L71.292 20zm-53.089-.045h25.141c4.38-.869 4.837-4.803 4.837-4.803.207-1.168.085-2.114.085-2.114-.131-.788-1.645-4.268-4.931-4.57-1.96-.182-19.4-1.552-19.4-1.552.337 1.73 1.123 2.603 1.646 3.101 1.21 1.151 3.136 1.477 3.136 1.477.466.043 13.442 1.107 13.442 1.107.414.022 1.197.125 1.188 1.14 0 .122-.114 1.016-1.11 1.016H23.43V6.872l-5.227-.378v13.46zm53.05-12.74l-5.28-.313v6.128s-.016 1.768-1.74 1.768h-9.076s-1.537-.127-1.537-1.75V6.071l-5.333-.384v9.4c.863 4.509 4.975 4.855 4.975 4.855s.42.028.488.034h12.257s5.246-.418 5.246-5.4V7.213zm-5.37-1.908h5.37V.025h-5.37v5.282zm-17.648 0h5.375V.025h-5.376v5.282zm47.947.005V0H76.945c-2.437.152-3.513 1.305-4.316 2.138-.856.882-1.337 2.742-1.337 2.742v.427l24.89.005zm-48.886-.005V0h-17.98C26.871.152 25.802 1.305 25 2.138c-.854.882-1.34 2.742-1.34 2.742V.035h-9.545c-.72 0-1.407.302-1.925 1.011-.54.703-2.725 4.261-2.725 4.261h37.831zM9.251 5.63L0 19.952h6.323l8.892-13.693-5.964-.63zM99.362 2.53v-.834h.761c.11 0 .2.018.265.05.108.063.177.181.177.354 0 .166-.045.278-.133.343a.583.583 0 0 1-.343.087h-.727zm-.314 1.326h.314V2.81h.714c.123 0 .212.009.279.041.104.061.16.163.165.326l.029.41.002.19a.28.28 0 0 1 .031.079h.398v-.058c-.052-.018-.075-.068-.104-.147-.017-.044-.017-.113-.019-.206l-.014-.327a.583.583 0 0 0-.071-.308.522.522 0 0 0-.221-.155.705.705 0 0 0 .254-.224.714.714 0 0 0 .086-.356c0-.279-.108-.473-.328-.579a1.051 1.051 0 0 0-.44-.08h-1.075v2.44zM97.636 2.53c0 1.24 1.011 2.25 2.255 2.25.6 0 1.155-.233 1.585-.657a2.23 2.23 0 0 0 .656-1.593A2.24 2.24 0 0 0 99.892.28a2.256 2.256 0 0 0-2.256 2.251zm.175 0A2.08 2.08 0 0 1 99.89.458c.55 0 1.065.22 1.456.606.39.4.6.918.6 1.466 0 .55-.21 1.077-.6 1.46a2.037 2.037 0 0 1-1.456.613c-1.148 0-2.08-.93-2.08-2.073z' fill='%23fff'/%3E%3C/svg%3E");
                    height: 20px;
                    width: 120px;
                    min-width: 120px;
                }
                .bg.RT .page-title{
                    color: var(--rt-primary);
                }
                .bg.RT .page-desc{
                    color: #FFF;
                }
                .bg.RT .scroll-info{
                    color: var(--rt-notice);
                }
                .bg.RT .btn{
                    border: 1px solid #248DFF;
                    color: #FFF;
                }
                .bg.RT .btn.disabled{
                    background-color: #000;
                    border-color: #B3B3B3;
                    color: #999999;
                }
                .bg.RT .btn.agree{
                    background-color: #248DFF;
                    border: 1px solid #248DFF;
                }
                .bg.RT .btn.disagree{
                    background-color: #000;
                }
                .bg.RT .btn.agree.disabled{
                    background-color: #316DB3;
                    border: 1px solid #316DB3;
                }
                .bg.RT .btn.disabled:hover{
                    box-shadow: none;
                }
                .bg.RT .checkbox-wrapper-40 {
                    --borderColor: var(--rt-primary);
                    color: #FFF;
                }
                
                .bg.RT .dropdown-menu{
                    background-color: #0A0A0A;
                    border: 1px solid #262626;
                }
                
                .bg.RT .dropdown-menu > ul > li{
                    color: #B3B3B3;
                }
                
                .bg.RT .dropdown-menu > ul > li:hover{
                    background-color: #262626;
                    color: #FFFFFF;
                }
                
                @media screen and (min-width: 576px){
                    .bg.RT .btn:hover{
                        box-shadow: 0 0 0 4px #61ADFF33;
                    }      
                }
                
                .bg.ROG {
                    background-color: #000;
                    
                }
                .bg.ROG .header{
                    border: none;
                    background-color: transparent;
                    border-image: linear-gradient(to right, #0A5EDB, #DB0A2C) 30;
                    border-bottom-width: 2px;
                    border-bottom-style: solid;
                    
                }
                .bg.ROG .header .header-name{
                    display: block;
                    color: #F5F5F5;
                    font-family: "rog","TradeGothicBold","Roboto","Segoe UI","Arial","PingFang TC","Microsoft JhengHei","Microsoft YaHei",sans-serif;
                    font-weight: normal;
                    font-size: 1.5em;
                    margin-bottom: 0.5em;
                }
                .bg.ROG .icon-logo{
                    background-color: #EA0029;
                    --logo-svg: url("data:image/svg+xml,%3Csvg width='45' height='24' viewBox='0 0 45 24' fill='none' xmlns='http://www.w3.org/2000/svg'%3E%3Cpath d='M14.729 19.014s.867.578 1.62.907c5.727 2.512 14.1 4.043 15.729 3.517 4.407-1.43 9.338-10.774 10.89-15.084 0 0-4.63 1.844-9.327 4.045-3.924 1.84-6.306 3.02-7.427 3.586l-.23.093.2-.077c-.505.255-.744.38-.744.38l13.385-4.056-2.459.756s-2.931 6.889-6.744 7.657c-3.816.767-10.586-1.914-10.596-1.916.56-.43 7.61-5.722 24.31-12.537.021-.007.044-.014.068-.02.55-.226 1.3-1.524 1.324-2.262 0 0-5.9.536-11.122 3.212-7.023 3.354-18.878 11.8-18.878 11.8zm16.421 2.54zM40.12 7.31a46.095 46.095 0 0 1 3.07-.983c-1.02.295-2.045.622-3.07.983zM25.982 16.075l-.012.005.012-.005zM0 10.59l.01.023-.01-.01.012.017c.127.341 1.315 3.468 2.532 5.033 1.195 1.54 6.02 2.742 6.699 2.904.051.023.083.035.083.035l-.034-.023.034.007C6.26 16.43 0 10.59 0 10.59z' fill='%23EA0029'/%3E%3Cpath d='M13.302 19.49c-.436-.833.104-1.935 3.667-4.572 3.22-2.384 14.814-12.13 25.906-14.75 0 0-5.815-.948-14.125 1.666-2.965.93-7.325 5.077-15.306 12.894-1.086.605-4.983-1.633-7.22-2.71 0 0 3.722 5.887 5.023 7.65 2.023 2.744 5.66 4.33 5.66 4.33s-.012-.01-.033-.032c-.301-.325-2.786-2.98-3.572-4.476z' fill='%23EA0029'/%3E%3C/svg%3E");
                    height: 30px;
                    width: 40px;
                    min-width: 30px;
                }
                .bg.ROG .page-title{
                    color: var(--rog-primary);
                }
                .bg.ROG .page-desc{
                    color: #FFF;
                }
                .bg.ROG .scroll-info{
                    color: var(--rog-notice);
                }
                .bg.ROG .btn{
                    color: #FFF;
                    border-radius: 0;
                    border: 1px solid #871019;
                }
                .bg.ROG .btn.disabled{
                    background-color: #222222;
                    color: #999999;
                }
                .bg.ROG .btn.agree{
                    background-color: #BA1622;
                }
                .bg.ROG .btn.agree.disabled{
                    background-color: #871019;
                }
                .bg.ROG .btn.disagree{
                    background-color: #222222;
                }
                .bg.ROG .btn.disabled:hover{
                    box-shadow: none;
                }
                .bg.ROG .checkbox-wrapper-40 {
                    --borderColor: var(--rog-primary);
                    color: #FFF;
                }
                .bg.ROG .toolbar-btn{
                    color: #DCDCDC;
                }
                .bg.ROG .toolbar-btn .lang-icon{
                    background-color: #DCDCDC;
                }
                .bg.ROG .dropdown-menu{
                    background-color: #0A0A0A;
                    border: 1px solid #181818;
                    border-radius: 0;
                }
                .bg.ROG .dropdown-menu > ul > li{
                    color: #B3B3B3;
                }
                .bg.ROG .dropdown-menu > ul > li:hover{
                    background-color: #262626;
                    border-radius: 0;
                    color: var(--rog-primary);
                }
                @media screen and (min-width: 576px){
                    .bg.ROG .btn:hover{
                        box-shadow: 0 0 8px 0 #BA1622;
                    }
                    .bg.ROG .btn::after{
                        content: "";
                        height: 100%;
                        min-width: 200px;
                        max-width: 300px;
                        width: 25%;
                        position: absolute;
                        z-index: 3;
                        --borderColor: #FF3535;
                        background: linear-gradient(to left, var(--borderColor), var(--borderColor)) left top no-repeat, linear-gradient(to bottom, var(--borderColor), var(--borderColor)) left top no-repeat, linear-gradient(to left, var(--borderColor), var(--borderColor)) right top no-repeat, linear-gradient(to bottom, var(--borderColor), var(--borderColor)) right top no-repeat, linear-gradient(to left, var(--borderColor), var(--borderColor)) left bottom no-repeat, linear-gradient(to bottom, var(--borderColor), var(--borderColor)) left bottom no-repeat, linear-gradient(to left, var(--borderColor), var(--borderColor)) right bottom no-repeat, linear-gradient(to left, var(--borderColor), var(--borderColor)) right bottom no-repeat;
                        background-size: 1px 4px, 4px 1px, 1px 4px, 4px 1px;
                    }           
                }
                
                .bg.TUF{
                    background-color: #000;
                }
                .bg.TUF .header{
                    border: none;
                    border-bottom: 2px solid #FFAA32;
                    background-color: transparent;
                    
                }
                .bg.TUF .header .header-name{
                    display: block;
                    color: #F5F5F5;
                    font-family: "rog","TradeGothicBold","Roboto","Segoe UI","Arial","PingFang TC","Microsoft JhengHei","Microsoft YaHei",sans-serif;
                    font-weight: normal;
                    font-size: 1.5em;
                    margin-bottom: 0.5em;
                }
                .bg.TUF .icon-logo{
                    background-color: #8B8A8B;
                    --logo-svg: url("data:image/svg+xml,%3Csvg width='42' height='34' viewBox='0 0 42 34' fill='none' xmlns='http://www.w3.org/2000/svg'%3E%3Cpath d='M19.716 11.647L10.937 6.57v22.107l8.78 5.056V11.647zM8.822 5.365L0 .266v16.967l8.822 5.077V5.365zM41.74 7.819V.266l-8.823 5.099v7.552l8.822-5.098zM41.74 9.68l-8.823 5.098v7.531l8.822-5.077V9.68zM22.023 11.648v22.086l8.758-5.056V6.592l-8.758 5.056z' fill='%238B8A8B'/%3E%3C/svg%3E");
                    height: 30px;
                    width: 40px;
                    min-width: 30px;
                }
                .bg.TUF .page-title{
                    color: var(--tuf-primary);
                }
                .bg.TUF .page-desc{
                    color: #FFF;
                }
                .bg.TUF .scroll-info{
                    color: var(--tuf-notice);
                }
                .bg.TUF .btn{
                    color: #FFF;
                    border-radius: 0;
                    border: 1px solid #F48116;
                }
                .bg.TUF .btn.disabled{
                    background-color: #222222;
                    color: #999999;
                }
                .bg.TUF .btn.disabled::after{
                    --borderColor: #FF9E1B;
                }
                .bg.TUF .btn.agree{
                    background-color: #FF9E1B;
                    color: #000;
                }
                .bg.TUF .btn.agree.disabled{
                    background-color: #F48116;
                    color: #FFCE85;
                    border: 1px solid #FFAA32;
                }
                .bg.TUF .btn.disagree{
                    background-color: #222222;
                }
                .bg.TUF .btn.disabled:hover{
                    box-shadow: none;
                }  
                .bg.TUF .checkbox-wrapper-40 {
                    --borderColor: var(--tuf-primary);
                    color: #FFF;
                }
                .bg.TUF .toolbar-btn{
                    color: #DCDCDC;
                }
                .bg.TUF .toolbar-btn .lang-icon{
                    background-color: #DCDCDC;
                }
                .bg.TUF .dropdown-menu{
                    background-color: #0A0A0A;
                    border: 1px solid #4D4D4D;
                    border-radius: 0;
                }
                .bg.TUF .dropdown-menu > ul > li{
                    color: #B3B3B3;
                }
                .bg.TUF .dropdown-menu > ul > li:hover{
                    background-color: #262626;
                    border-radius: 0;
                    color: var(--tuf-primary);
                }
                @media screen and (min-width: 576px){
                    .bg.TUF .btn:hover{
                        box-shadow: 0 0 8px 0 #F48116;
                    }
                    .bg.TUF .btn::after{
                        content: "";
                        height: 100%;
                        min-width: 200px;
                        max-width: 300px;
                        width: 25%;
                        position: absolute;
                        z-index: 3;
                        --borderColor: #FFCE85;
                        background: linear-gradient(to left, var(--borderColor), var(--borderColor)) left top no-repeat, linear-gradient(to bottom, var(--borderColor), var(--borderColor)) left top no-repeat, linear-gradient(to left, var(--borderColor), var(--borderColor)) right top no-repeat, linear-gradient(to bottom, var(--borderColor), var(--borderColor)) right top no-repeat, linear-gradient(to left, var(--borderColor), var(--borderColor)) left bottom no-repeat, linear-gradient(to bottom, var(--borderColor), var(--borderColor)) left bottom no-repeat, linear-gradient(to left, var(--borderColor), var(--borderColor)) right bottom no-repeat, linear-gradient(to left, var(--borderColor), var(--borderColor)) right bottom no-repeat;
                        background-size: 1px 4px, 4px 1px, 1px 4px, 4px 1px;
                    }           
                }
                
            </style>
            <div class="bg ${ASUS_POLICY.getTheme()}">
                <div class="header">
                    <div class="header-title"><div class="icon-logo"></div><div class="header-name">${ASUS_POLICY.Dict.ModalName}</div></div>
                    <div class="header-dropdown">
                        <button id="selectLang" class="toolbar-btn"><i class="lang-icon"></i>${ASUS_POLICY.Dict.SelectLanguage}</button>
                        <div class="dropdown-menu">
                            <ul></ul>
                            <ul></ul>
                        </div>
                    </div>
                </div>
                <div class="container">
                    <div class="page">
                        <div class="page-title">${ASUS_POLICY.Content[policy].Title}</div>
                        <div>
                            <div class="page-content">
                                <div class="page-desc">${ASUS_POLICY.Dict.QisDesc}</div>
                                <div class="policy-content"></div>
                                <div class="scroll-info">${ASUS_POLICY.Dict.ScrollDown}</div>
                                <div class="policy-age"></div>
                            </div>
                            <div class="page-footer"></div>
                        </div>
                    </div>
                </div>
            </div>
        `;

        this.template = typeof props.template !== 'undefined' ? props.template : template;
        shadowRoot.appendChild(this.template.content.cloneNode(true));
        this.element = div;

        const handleClickAbortBtn = (e) => {
            if (!e.target.classList.contains("disabled")) {
                const policyModal = new PolicyWithdrawModalComponent({
                    policy: "PP",
                    submit_reload: 1,
                    readAgainCallback: clickPopupReadAgainBtn,
                    knowRiskCallback: clickPopupKnowRiskBtn
                });
                top.document.body.appendChild(policyModal.render());
            }
        }

        const Get_Component_btnLoading = () => {
            return `<div class="gg-spinner-two-alt"></div>`;
        }

        const handleClickApplyBtn = (e) => {
            if (!e.target.classList.contains("disabled")) {
                if (policy === "PP") {
                    applyBtn.innerHTML = Get_Component_btnLoading();
                    setTimeout(function () {
                        httpApi.privateEula.set("1", function () {
                            httpApi.securityUpdate.set(1);
                            httpApi.nvramSet({
                                "webs_update_enable": 1, "action_mode": "apply", "rc_service": "saveNvram"
                            }, () => {
                            }, false);
                            location.reload();
                        })
                    }, 1000);
                } else if (policy === "EULA") {
                    const yearChecked = shadowRoot.querySelector("#ASUS_EULA_enable").checked;

                    if (!yearChecked) {
                        alert(`${ASUS_POLICY.Dict.AgeConfirm}`);
                        shadowRoot.querySelector(".checkbox-wrapper-40").style.color = top.businessWrapper ? "red" : "#ff5722";
                        return false;
                    }
                    applyBtn.innerHTML = Get_Component_btnLoading();
                    setTimeout(function () {
                        httpApi.newEula.set("1", function () {
                            location.reload();
                        })
                    }, 1000);
                }
            }
        }

        const clickPopupReadAgainBtn = () => {
            enableBtn(abortBtn, false);
            enableBtn(applyBtn, false);
            scrollDiv.reset();
            top.document.body.style.removeProperty('overflow');
        }

        const checkScrollHeight = (e) => {
            let target = e.target;
            if ((target.scrollTop + target.offsetHeight) >= target.scrollHeight) {
                enableBtn(abortBtn, true);
                enableBtn(applyBtn, true);
            }
        }

        const clickPopupKnowRiskBtn = () => {
            window.goTo.loadPage("welcome", true);
        }

        const enableBtn = function (btn, boolean) {
            if (btn !== null) {
                if (boolean) {
                    btn.classList.remove("disabled");
                } else {
                    btn.classList.add("disabled");
                }
            }
        }

        let abortBtn = null;
        let applyBtn = null;

        if (policy !== "EULA") {
            abortBtn = document.createElement('div');
            abortBtn.className = "btn disagree disabled";
            abortBtn.textContent = ASUS_POLICY.Dict.Disagree;
            shadowRoot.querySelector('.page-footer').appendChild(abortBtn);
            abortBtn.addEventListener("click", handleClickAbortBtn);

        }

        applyBtn = document.createElement('div');
        applyBtn.className = "btn agree disabled";
        applyBtn.textContent = ASUS_POLICY.Dict.Agree;
        shadowRoot.querySelector('.page-footer').appendChild(applyBtn);
        applyBtn.addEventListener('click', handleClickApplyBtn);

        const scrollDiv = new PolicyScrollDiv({
            policy: policy, theme: ASUS_POLICY.getTheme(), scrollCallBack: checkScrollHeight
        });
        this.element.shadowRoot.querySelector('.policy-content').appendChild(scrollDiv.render());

        if (policy === "EULA") {
            const ageCheckboxDiv = document.createElement('div');
            ageCheckboxDiv.className = "checkbox-wrapper-40";
            ageCheckboxDiv.innerHTML = `
                <label>
                    <input id="ASUS_EULA_enable" type="checkbox"/>
                    <span class="checkbox">${ASUS_POLICY.Dict.AgeCheck}</span>
                </label>`;
            this.element.shadowRoot.querySelector('.policy-age').appendChild(ageCheckboxDiv);
        }

        const selectLang = shadowRoot.querySelector("#selectLang");
        selectLang.addEventListener("click", function () {
            const dropdownMenu = shadowRoot.querySelector(".dropdown-menu");
            if (dropdownMenu.classList.contains("show")) {
                dropdownMenu.classList.remove("show");
            } else {
                dropdownMenu.classList.add("show");
            }
        })

        const system_language = (() => {
            let list = httpApi.hookGet("language_support_list");
            return {supportList: {...list}};
        })();

        const entries = Object.entries(system_language.supportList);
        const halfLength = Math.ceil(entries.length / 2);
        for (let i = 0; i < entries.length; i++) {
            const [key, value] = entries[i];
            const targetUl = i < halfLength ? shadowRoot.querySelector(".dropdown-menu ul:first-child") : shadowRoot.querySelector(".dropdown-menu ul:last-child");
            targetUl.innerHTML += `<li data-lang="${key}">${value}</li>`;
        }

        const selectLangItems = shadowRoot.querySelectorAll(".dropdown-menu li");
        selectLangItems.forEach((item) => {
            item.addEventListener('click', (e) => {
                const lang = e.target.getAttribute("data-lang");
                changeLanguage(lang);
            });
        });

        function changeLanguage(lang) {
            httpApi.nvramSet({
                action_mode: "apply", rc_service: "email_info", flag: "set_language", preferred_lang: lang,
            }, function () {
                setTimeout(function () {
                    location.reload();
                }, 10);
            });
        }

    }

    render() {
        return this.element;
    }
}


if (typeof module !== 'undefined') {
    module.exports = {
        PolicyModalComponent, PolicyWithdrawModalComponent, PolicyUpdateModalComponent, ThirdPartyPolicyModalComponent
    };
}