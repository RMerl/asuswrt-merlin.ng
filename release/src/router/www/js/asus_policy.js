const ASUS_POLICY = {
    Dict: {
        ScrollDown: `<#ASUS_POLICY_Scroll_Down#>`,
        AgeCheck: `<#ASUS_POLICY_Age_Check#>`,
        AgeConfirm: `<#ASUS_eula_age_confirm#>`,
        Agree: `<#CTL_Agree#>`,
        Disagree: `<#CTL_Decline#>`,
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
            HTMLv2: `<style>
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
                </style>
                <div>
                    <div class="policy_title"><#ASUS_PP_Title#></u></b></div>
                    <div class="policy_desc"><#ASUS_PP_Desc_v2#></div>
                    <div class="policy_desc">${`<#ASUS_PP_Desc_2#>`.replace(/\[aa\](.*?)\[\/aa\]/g, `<span class="link" onclick="showPersonalData(2)" style="text-decoration: underline;cursor: pointer;">$1</span>`).replaceAll("%1$@", `<#menu5_6#>`).replaceAll("%2$@", `<#menu_privacy#>`)}</div>
                    <div class="policy_desc"><#ASUS_PP_Desc_3#></div>
                    <div class="policy_desc"><#ASUS_PP_Desc_4#></div>
                    <div class="policy_desc"><#ASUS_PP_Desc_5#></div>
                    <div class="policy_show"></div>
                </div>`,
            HTMLv4: `<style>
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
                </style>
                <div>
                    <div class="policy_title"><#ASUS_PP_Title#></u></b></div>
                    <div class="policy_desc"><#ASUS_PP_Desc_v4#></div>
                    <div class="policy_desc">${`<#ASUS_PP_Desc_2#>`.replace(/\[aa\](.*?)\[\/aa\]/g, `<span class="link" onclick="showPersonalData(4)" style="text-decoration: underline;cursor: pointer;">$1</span>`).replaceAll("%1$@", `<#menu5_6#>`).replaceAll("%2$@", `<#menu_privacy#>`)}</div>
                    <div class="policy_desc"><#ASUS_PP_Desc_3#></div>
                    <div class="policy_desc"><#ASUS_PP_Desc_4#></div>
                    <div class="policy_desc"><#ASUS_PP_Desc_5#></div>
                    <div class="policy_show"></div>
                </div>`,
            PersonalData: `<#ASUS_Personal#>`,
            PersonalDataHTMLv2: `
                <style>
                    .policy_law {
                        font-weight: normal;
                    }
                    .policy_law > ol:first-of-type {
                        margin-top: 0;
                    }
                    .policy_law > ol:first-of-type > li:first-of-type {
                        margin-top: 0;
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
                    .mt-1 {
                        margin-top: 1em;
                    }
                    .mb-1 {
                        margin-bottom: 1em;
                    }
                </style>
                <div class="policy_law">
                    <ol>
                        <li>${`<#ASUS_PP_Law_1#>`.replaceAll("[bu]", "<b><u>").replaceAll("[/bu]", "</b></u>").replaceAll("%@", `<#CTL_Agree#>`)}
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
                        <li>${`<#ASUS_PP_Law_3#>`.replaceAll("%@", `<#CTL_Decline#>`)}
                            <ol>
                                <li><#ASUS_PP_Law_3_1#><#ASUS_PP_Law_3_2#></li>
                                <li><#ASUS_PP_Law_3_3#></li>
                            </ol>
                        </li>
                        <li><#ASUS_PP_Law_4#></li>
                    </ol>
                    <div class="policy_end"><#ASUS_PP_End#></div>
                </div>
                <div class="policy_show"></div>
            `,
            PersonalDataHTMLv4: `
                <style>
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
                    .mt-1 {
                        margin-top: 1em;
                    }
                    .mb-1 {
                        margin-bottom: 1em;
                    }
                </style>
                <div class="policy_law">
                    <div><#ASUS_Personal_Desc#></div>
                    <ol>
                        <li>
                            <div class="mb-1"><#ASUS_Personal_Law_1_1#></div>
                            <div class="mb-1"><#ASUS_Personal_Law_1_2#></div>
                            <div>${`<#ASUS_Personal_Law_1_3#>`.replace('[https://www.asus.com/support/faq/1053743/]', `[<a target="_blank" href="https://www.asus.com/support/faq/1053743/">https://www.asus.com/support/faq/1053743/</a>]`)}</div>
                        </li>
                        <li><#ASUS_Personal_Law_2#>
                            <ol>
                                <li><#ASUS_Personal_Law_2_1_1#><#ASUS_Personal_Law_2_1_2#></li>
                                <li><#ASUS_Personal_Law_2_2#></li>
                            </ol>
                        </li>
                        <li><#ASUS_Personal_Law_3#></li>
                        <li><#ASUS_Personal_Law_4#></li>
                        <li><#ASUS_Personal_Law_5#></li>
                    </ol>
                </div>
                <div class="policy_show"></div>
            `
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
                </div>
                <div class="policy_show"></div>
            `,
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
                'NordVPN': {
                    url_text: `Nordvpn S.A. PRIVACY POLICY`,
                    url: `https://my.nordaccount.com/zh-tw/legal/privacy-policy/nordvpn/`,
                    company: `Nordvpn S.A.`
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
                },
                'CyberGhost': {
                    url_text: `CyberGhost PRIVACY POLICY`,
                    url: `https://www.cyberghostvpn.com/en_US/privacypolicy`,
                    company: `CyberGhost S.R.L.`
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
            padding: 12px;
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
        
        .policy-scroll-div a, .policy-scroll-div .link {
            color: #006ce1;
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
        
        @media (min-width: 992px) {
            .modal-lg, .modal-xl {
                max-width: 800px;
            }
        }
        
        @media (min-width: 1200px) {
            .modal-xl {
                max-width: 1020px;
            }
        }
        
        .popup_bg.RT .modal-content {
            background-color: #2B373B;
        }
        .popup_bg.RT .modal-title {
            color: #FFFFFF;
        }
        .popup_bg.RT .policy-scroll-div {
            color: #FFFFFF;
        }
        .popup_bg.RT .policy-scroll-div a{
            color: #0A5EDB;
        }
        .popup_bg.RT .notice_title {
            color: #FFFFFF;
        }
        .popup_bg.RT .notice_content {
            color: #FFFFFF;
        }
        .popup_bg.RT .age-label {
            color: #FFFFFF;            
        }
        
        .popup_bg.ROG .modal-content {
            background-color: #000000e6;
            border: 1px solid rgb(161, 10, 16);
        }
        .popup_bg.ROG .modal-title {
            color: #cf0a2c;
        }
        .popup_bg.ROG .policy-scroll-div {
            color: #FFFFFF;
        }
        .popup_bg.ROG .policy-scroll-div a, .popup_bg.ROG .policy-scroll-div .link{
            color: #cf0a2c;
        }
        .popup_bg.ROG .btn-primary {
            background-color: #91071f;
            border: 0;
        }
        .popup_bg.ROG .btn-primary.disabled, 
        .popup_bg.ROG .btn-primary:disabled {
            color: #fff;
            background-color: #91071f;
        }
        .popup_bg.ROG .btn-primary:hover {
            background-color: #cf0a2c;
        }
        .popup_bg.ROG .notice_title {
            color: #cf0a2c;
        }
        .popup_bg.ROG .notice_content {
            color: #FFFFFF;
        }
        .popup_bg.ROG .age-label {
            color: #FFFFFF;            
        }
        
        .popup_bg.TUF .modal-content {
            background-color: #000000e6;
            border: 1px solid #92650F;
        }
        .popup_bg.TUF .modal-title {
            color: #ffa523;
        }
        .popup_bg.TUF .policy-scroll-div {
            color: #FFFFFF;
        }
        .popup_bg.TUF .policy-scroll-div a, .popup_bg.TUF .policy-scroll-div .link{
            color: #ffa523;
        }
        .popup_bg.TUF .btn-primary {
            background-color: #ffa523;
            border: 0;
        }
        .popup_bg.TUF .btn-primary.disabled, .btn-primary:disabled {
            color: #fff;
            background-color: #ffa523;
        }
        .popup_bg.TUF .btn-primary:hover {
            background-color: #D0982C;
        }
        .popup_bg.TUF .notice_title {
            color: #ffa523;
        }
        .popup_bg.TUF .notice_content {
            color: #FFFFFF;
        }
        .popup_bg.TUF .age-label {
            color: #FFFFFF;            
        }
        .popup_bg.TS .modal-content {
            background-color: #000000e6;
            border: 1px solid #92650F;
        }
        .popup_bg.TS .modal-title {
            color: #ffa523;
        }
        .popup_bg.TS .policy-scroll-div,
        .popup_bg.TS .policy-scroll-div a{
            color: #FFFFFF;
        }
        .popup_bg.TS .btn-primary {
            background-color: #ffa523;
            border: 0;
        }
        .popup_bg.TS .btn-primary.disabled, .btn-primary:disabled {
            color: #fff;
            background-color: #ffa523;
        }
        .popup_bg.TS .btn-primary:hover {
            background-color: #D0982C;
        }
        .popup_bg.TS .notice_title {
            color: #ffa523;
        }
        .popup_bg.TS .notice_content {
            color: #FFFFFF;
        }
        .popup_bg.TS .age-label {
            color: #FFFFFF;
        }
      </style>`,
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

const dependencies = ['httpApi.js',];

if (!areScriptsLoaded(dependencies)) {
    console.log('Dependencies did not load');
}

let policy_status = {
    "Policy_lang": "EN", "EULA": "0", "EULA_time": "", "PP": "0", "PP_time": "", "TM": "0", "TM_time": "",
};

async function PolicyStatus() {
    let policy_status = {};
    await httpApi.privateEula.get()
        .then(data => {
            policy_status.PP = data.ASUS_PP;
            policy_status.PP_time = data.ASUS_PP_time;
        })

    const asus_pp_support = await httpApi.hookGet("get_ui_support").asus_pp;

    const nvram_data = await httpApi.nvramGet(["ASUS_NEW_EULA", "ASUS_NEW_EULA_time", 'TM_EULA', 'TM_EULA_time', 'preferred_lang', 'ASUS_PP_AutoWebUpgradeDisable'], true);
    policy_status.EULA = nvram_data.ASUS_NEW_EULA;
    policy_status.EULA_time = nvram_data.ASUS_NEW_EULA_time;
    policy_status.TM = nvram_data.TM_EULA;
    policy_status.TM_time = nvram_data.TM_EULA_time;
    policy_status.Policy_lang = nvram_data.preferred_lang;
    policy_status.ASUS_PP_AutoWebUpgradeDisable = nvram_data.ASUS_PP_AutoWebUpgradeDisable;
    policy_status.ASUS_PP_support = asus_pp_support;
    return policy_status;
}

PolicyStatus().then(data => {
    policy_status = data;
});

class PolicyScrollDiv {

    constructor(props) {
        const {policy, theme = '', scrollCallBack, policyStatus = policy_status} = props;
        if (policyStatus.PP == 0) {
            this.ppVersion = policyStatus.ASUS_PP_support;
        } else {
            this.ppVersion = policyStatus.PP;
        }
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

        .policy-scroll-div.TS {
            border: 2px solid #2ED9C3;
            border-radius: 8px;
            background-color: #181818;
            color: #FFFFFF;
        }

        .policy-scroll-div.RT a,
        .policy-scroll-div.ROG a,
        .policy-scroll-div.TUF a,
        .policy-scroll-div.TS a{
            color: #FFFFFF;
        }
      </style>
      <div class="policy-scroll-div ${theme}"></div>
    `;

        shadowRoot.appendChild(template.content.cloneNode(true));
        shadowRoot.querySelector('.policy-scroll-div').addEventListener('scroll', scrollCallBack.bind(this));
        if (policy) {
            if (policy == "PP") {
                shadowRoot.querySelector('.policy-scroll-div').innerHTML = ASUS_POLICY.Content[policy][`HTMLv${this.ppVersion}`].replace(/\[aa\](.*?)\[\/aa\]/g, `<a target="_blank" href="https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Policy&lang=` + policyStatus.Policy_lang + `&kw=&num=" style="text-decoration: underline;cursor: pointer;">$1</a>`)
            } else {
                shadowRoot.querySelector('.policy-scroll-div').innerHTML = ASUS_POLICY.Content[policy].HTML.replace(/\[aa\](.*?)\[\/aa\]/g, `<a target="_blank" href="https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Policy&lang=` + policyStatus.Policy_lang + `&kw=&num=" style="text-decoration: underline;cursor: pointer;">$1</a>`)
            }
        }

        const observer = new IntersectionObserver((entries) => {
            entries.forEach(entry => {
                if (entry.isIntersecting) {
                    scrollCallBack({target: shadowRoot.querySelector('.policy-scroll-div')});
                }
            });
        }, {threshold: 0.1});

        const targetElement = shadowRoot.querySelector('.policy-scroll-div .policy_show')
        observer.observe(targetElement);

        window.addEventListener('resize', () => {
            scrollCallBack({target: shadowRoot.querySelector('.policy-scroll-div')});
        });

        this.element = div;
    }

    render() {
        return this.element
    }

    reset() {
        this.element.shadowRoot.querySelector('.policy-scroll-div').scrollTop = 0;
    }
}

class PolicyModalComponent {
    constructor(props) {
        const {
            id = 'policy_popup_modal',
            policy,
            party,
            theme = this.getTheme(),
            securityUpdate = false,
            websUpdate = false,
            policyStatus = policy_status,
            singPPVersion = 0,
            agreeCallback = function () {
            },
            disagreeCallback = function () {
            },
            readAgainCallback = function () {
            },
            knowRiskCallback = function () {
            },
        } = props;
        this.id = id;
        this.policy = policy;
        this.singPPVersion = singPPVersion;
        if (policyStatus.PP == 0) {
            this.ppVersion = policyStatus.ASUS_PP_support;
        } else {
            this.ppVersion = policyStatus.PP;
        }
        if (singPPVersion > 0 && policyStatus.PP == 2) {
            this.ppVersion = singPPVersion;
        }
        this.securityUpdate = securityUpdate;
        this.websUpdate = websUpdate;
        this.policyStatus = policyStatus;
        httpApi.log('policy_status', JSON.stringify(policyStatus));

        this.agreeCallback = agreeCallback;
        this.disagreeCallback = disagreeCallback;
        this.readAgainCallback = readAgainCallback;
        this.knowRiskCallback = knowRiskCallback;

        const div = document.createElement('div');
        div.id = id;
        const shadowRoot = div.attachShadow({mode: 'open'});
        const template = document.createElement('template');
        template.innerHTML = `
          ${ASUS_POLICY.PolicyModalStyle}
          <div class="popup_bg ${theme}">
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

        shadowRoot.appendChild(template.content.cloneNode(true));
        this.element = div;

        if (this.element.shadowRoot.querySelector('div.policy-scroll-div')) {
            this.element.shadowRoot.querySelector('div.policy-scroll-div').addEventListener('scroll', this.handleScrollCheck.bind(this));
        }

        const ageCheckbox = document.createElement('input');
        ageCheckbox.type = 'checkbox';
        ageCheckbox.id = 'ASUS_EULA_enable';
        this.ageCheckbox = ageCheckbox;

        const ageLabel = document.createElement('label');
        ageLabel.classList.add('age-label')
        ageLabel.setAttribute('for', 'ASUS_EULA_enable');
        ageLabel.textContent = ASUS_POLICY.Dict.AgeCheck;
        this.ageLabel = ageLabel;

        const ageCheckDiv = document.createElement('div');
        ageCheckDiv.style.width = '100%';
        ageCheckDiv.appendChild(ageCheckbox);
        ageCheckDiv.appendChild(ageLabel);
        this.ageCheckDiv = ageCheckDiv;

        const loadingImg = document.createElement('i');
        loadingImg.className = 'gg-spinner';
        this.loadingImg = loadingImg;

        const closeBtn = document.createElement('button');
        closeBtn.type = 'button';
        closeBtn.className = 'btn btn-primary btn-block close';
        closeBtn.innerHTML = ASUS_POLICY.Dict.Ok;
        closeBtn.addEventListener('click', this.handleClickClose.bind(this));
        this.closeBtn = closeBtn;

        const agreeBtn = document.createElement('button');
        agreeBtn.type = 'button';
        agreeBtn.className = (policy === 'EULA') ? 'btn btn-primary agree disabled' : 'btn btn-primary disagree';
        agreeBtn.innerHTML = ASUS_POLICY.Dict.Agree;
        agreeBtn.addEventListener('click', this.handleClickAgree.bind(this));
        this.agreeBtn = agreeBtn;


        const disagreeBtn = document.createElement('button');
        disagreeBtn.type = 'button';
        disagreeBtn.className = (policy === 'EULA') ? 'btn btn-secondary disagree disabled' : 'btn btn-primary disagree';
        disagreeBtn.innerHTML = ASUS_POLICY.Dict.Disagree;
        disagreeBtn.addEventListener('click', this.handleClickDisagree.bind(this));
        this.disagreeBtn = disagreeBtn;

        if (policy) {
            this.element.shadowRoot.querySelector('div.modal-title').innerHTML = ASUS_POLICY.Content[policy].Title;
            if (policy == "PP") {
                this.element.shadowRoot.querySelector('div.policy-scroll-div').innerHTML = ASUS_POLICY.Content[policy][`HTMLv${this.ppVersion}`].replace(/\[aa\](.*?)\[\/aa\]/g, `<a target="_blank" href="https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Policy&lang=` + this.policyStatus.Policy_lang + `&kw=&num=" style="text-decoration: underline;cursor: pointer;">$1</a>`);
            } else {
                this.element.shadowRoot.querySelector('div.policy-scroll-div').innerHTML = ASUS_POLICY.Content[policy].HTML.replace(/\[aa\](.*?)\[\/aa\]/g, `<a target="_blank" href="https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Policy&lang=` + this.policyStatus.Policy_lang + `&kw=&num=" style="text-decoration: underline;cursor: pointer;">$1</a>`);
            }

            switch (policy) {
                case "EULA":
                    if (this.policyStatus.EULA === "0") {
                        const scrollInfoDiv = document.createElement('div');
                        scrollInfoDiv.className = "scroll-info";
                        scrollInfoDiv.innerHTML = ASUS_POLICY.Dict.ScrollDown;
                        this.element.shadowRoot.querySelector('div.modal-body').appendChild(scrollInfoDiv);
                        this.element.shadowRoot.querySelector('div.modal-body').appendChild(ageCheckDiv);
                        this.element.shadowRoot.querySelector('div.modal-footer').appendChild(agreeBtn);
                    } else {
                        this.element.shadowRoot.querySelector('div.modal-footer').appendChild(closeBtn);
                    }
                    break;
                case "PP":
                    if (this.policyStatus.PP < "2" || this.policyStatus.PP_time === "" || this.ppVersion > this.policyStatus.PP) {
                        this.element.shadowRoot.querySelector('div.modal-footer').appendChild(disagreeBtn);
                        this.element.shadowRoot.querySelector('div.modal-footer').appendChild(agreeBtn);
                    } else {
                        this.element.shadowRoot.querySelector('div.modal-footer').appendChild(closeBtn);
                    }
                    break;
                case "TM":
                    this.element.shadowRoot.querySelector('div.policy-scroll-div').querySelector('#tm_eula_url').setAttribute("href", "https://nw-dlcdnet.asus.com/trend/tm_privacy");
                    this.element.shadowRoot.querySelector('div.policy-scroll-div').querySelector('#tm_disclosure_url').setAttribute("href", "https://nw-dlcdnet.asus.com/trend/tm_pdcd");
                    this.element.shadowRoot.querySelector('div.policy-scroll-div').querySelector('#eula_url').setAttribute("href", `https://nw-dlcdnet.asus.com/support/forward.html?model=&type=TMeula&lang=${this.policyStatus.Policy_lang}&kw=&num=`);
                    if (this.policyStatus.TM === "0" || this.policyStatus.TM_time === "") {
                        agreeBtn.classList.remove("disabled");
                        disagreeBtn.classList.remove("disabled");
                        this.element.shadowRoot.querySelector('div.modal-footer').appendChild(disagreeBtn);
                        this.element.shadowRoot.querySelector('div.modal-footer').appendChild(agreeBtn);
                    } else {
                        this.element.shadowRoot.querySelector('div.modal-footer').appendChild(this.closeBtn);
                    }
                    break;
                case "THIRDPARTY_PP":
                    this.element.shadowRoot.querySelector('div.policy-scroll-div .thirdparty-pp-desc1').innerHTML = this.element.shadowRoot.querySelector('div.policy-scroll-div .thirdparty-pp-desc1').innerHTML.replace("%1$@", party).replace("[aa]%2$@[/aa]", `<a target="_blank" href="${ASUS_POLICY.Content[policy].Feature[party].url}" style="text-decoration: underline;cursor: pointer;">${ASUS_POLICY.Content[policy].Feature[party].url_text}</a>`);
                    this.element.shadowRoot.querySelector('div.policy-scroll-div .thirdparty-pp-desc2').innerHTML = this.element.shadowRoot.querySelector('div.policy-scroll-div .thirdparty-pp-desc2').innerHTML.replaceAll("%1$@", `${ASUS_POLICY.Content[policy].Feature[party].company}`).replaceAll("%2$@", party);
                    this.element.shadowRoot.querySelector('div.policy-scroll-div .thirdparty-pp-desc3').innerHTML = this.element.shadowRoot.querySelector('div.policy-scroll-div .thirdparty-pp-desc3').innerHTML.replaceAll("%1$@", `${ASUS_POLICY.Content[policy].Feature[party].company}`).replaceAll("%2$@", party);
                    this.element.shadowRoot.querySelector('div.modal-footer').appendChild(closeBtn);
                    if (party == 'Speedtest') {
                        const speedtestDesc = document.createElement('div');
                        speedtestDesc.innerHTML = `<#InternetSpeed_desc#>`;
                        const referenceElement = this.element.shadowRoot.querySelector('div.policy-scroll-div .thirdparty-pp-desc1');
                        referenceElement.parentNode.insertBefore(speedtestDesc, referenceElement);
                        const speedtestLogo = document.createElement('div');
                        speedtestLogo.innerHTML = `<div class="speedtest_logo"></div>`;
                        this.element.shadowRoot.querySelector('div.policy-scroll-div').appendChild(speedtestLogo);
                    }
                    break;
            }
        }

    }

    policy = "";
    ageCheckbox = null;
    ageLabel = null;
    ageCheckDiv = null;
    loadingImg = null;
    closeBtn = null;
    agreeBtn = null;
    disagreeBtn = null;

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
        } else if (isSupport("UI4")) {
            return "";
        } else {
            return theme;
        }
    }

    checkPolicy = () => {
        const status = PolicyStatus()
            .then(data => {
                httpApi.log('policy_status', JSON.stringify(data));
                if (data.EULA == "0") {
                    const policyModal = new PolicyUpdateModalComponent({
                        securityUpdate: true, websUpdate: true, policyStatus: data
                    });
                    policyModal.show();
                } else if (data.EULA == 1 && ((data.PP == 1 && data.PP_time != "") || (data.PP == 0 && data.PP_time == ""))) {
                    const policyModal = new PolicyModalComponent({
                        policy: 'PP',
                        securityUpdate: true,
                        websUpdate: true,
                        policyStatus: data,
                    });
                    policyModal.show();
                }
            });
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

    enableBtn = function (btn, boolean) {
        if (btn !== null) {
            if (boolean) {
                btn.classList.remove("disabled");
            } else {
                btn.classList.add("disabled");
            }
        }
    }

    handleScrollCheck() {
        const scrollDiv = this.element.shadowRoot.querySelector('div.policy-scroll-div');
        const scrollTop = scrollDiv.scrollTop;
        const offsetHeight = scrollDiv.offsetHeight;
        const scrollHeight = scrollDiv.scrollHeight;
        if ((scrollTop + offsetHeight) >= scrollHeight) {
            this.enableBtn(this.agreeBtn, true);
            this.enableBtn(this.disagreeBtn, true);
        }
    }

    handleClickClose() {
        this.hide();
    }

    handleClickAgree(e) {
        if (!e.target.classList.contains("disabled")) {
            switch (this.policy) {
                case "EULA":
                    let yearChecked = this.ageCheckbox.checked;
                    if (!yearChecked) {
                        alert(ASUS_POLICY.Dict.AgeConfirm);
                        this.ageLabel.style.color = top.businessWrapper ? "red" : "#ff5722";
                        return false;
                    } else {
                        this.agreeBtn.classList.add("disabled");
                        this.agreeBtn.innerHTML = '';
                        this.agreeBtn.appendChild(this.loadingImg);
                        httpApi.newEula.set(1, async () => {
                            await this.hide();
                            await this.checkPolicy();
                        })
                    }
                    break
                case "PP":
                    this.agreeBtn.classList.add("disabled");
                    this.disagreeBtn.classList.add("disabled");
                    this.agreeBtn.innerHTML = '';
                    this.agreeBtn.appendChild(this.loadingImg);
                    setTimeout(() => {
                        httpApi.privateEula.set("1", async () => {
                            if (this.securityUpdate) {
                                await httpApi.securityUpdate.set(1);
                            }
                            if (this.websUpdate) {
                                if (this.policyStatus.ASUS_PP_AutoWebUpgradeDisable !== "1") {
                                    await httpApi.nvramSet({
                                        "webs_update_enable": 1, "action_mode": "apply", "rc_service": "saveNvram"
                                    }, () => {
                                    }, false);
                                }
                            }
                            await this.agreeCallback();
                            await this.hide();
                            await this.checkPolicy();
                        })
                    }, 1000);
                    break
                case "TM":
                    this.agreeCallback();
                    this.agreeBtn.innerHTML = '';
                    this.agreeBtn.appendChild(this.loadingImg);
                    httpApi.enableEula('tm', "1", () => {
                        top.document.body.style.removeProperty('overflow');
                        this.hide();
                    })
            }
        }
    }

    handleClickDisagree(e) {
        if (!e.target.classList.contains("disabled")) {
            switch (this.policy) {
                case "PP":
                    this.hide();
                    const policyModal = new PolicyWithdrawModalComponent({
                        policy: "PP",
                        policyStatus: this.policyStatus,
                        securityUpdate: this.securityUpdate,
                        websUpdate: this.websUpdate,
                        singPPVersion: this.singPPVersion,
                        agreeCallback: this.agreeCallback,
                        disagreeCallback: this.disagreeCallback,
                        readAgainCallback: this.readAgainCallback,
                        knowRiskCallback: this.knowRiskCallback
                    });
                    policyModal.show();
                    break
                case "TM":
                    this.disagreeCallback();
                    httpApi.enableEula("tm", "0", () => {
                        top.document.body.style.removeProperty('overflow');
                        this.hide();
                    })
                    break
            }
        }
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

class PersonalDataModalComponent {
    constructor(props) {
        const {
            id = 'personal_popup_modal',
            policyStatus = policy_status,
            theme = this.getTheme(),
        } = props;
        this.id = id;
        if (policyStatus.PP == 0) {
            this.ppVersion = policyStatus.ASUS_PP_support;
        } else {
            this.ppVersion = policyStatus.PP;
        }
        const div = document.createElement('div');
        div.id = id;
        const shadowRoot = div.attachShadow({mode: 'open'});
        const template = document.createElement('template');
        template.innerHTML = `
          ${ASUS_POLICY.PolicyModalStyle}
          <div class="popup_bg ${theme}">
            <div class="modal">
                <div class="modal-dialog modal-xl">
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

        shadowRoot.appendChild(template.content.cloneNode(true));
        this.element = div;

        const closeBtn = document.createElement('button');
        closeBtn.type = 'button';
        closeBtn.className = 'btn btn-primary btn-block close';
        closeBtn.innerHTML = 'Understood';
        closeBtn.addEventListener('click', this.handleClickClose.bind(this));
        this.closeBtn = closeBtn;

        this.element.shadowRoot.querySelector('div.modal-title').innerHTML = ASUS_POLICY.Content.PP.PersonalData;
        this.element.shadowRoot.querySelector('div.policy-scroll-div').innerHTML = ASUS_POLICY.Content.PP[`PersonalDataHTMLv${this.ppVersion}`].replace(/\[aa\](.*?)\[\/aa\]/g, `<a target="_blank" href="https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Policy&lang=` + policyStatus.Policy_lang + `&kw=&num=" style="text-decoration: underline;cursor: pointer;">$1</a>`);

        const scrollInfoDiv = document.createElement('div');
        scrollInfoDiv.className = "scroll-info";
        scrollInfoDiv.innerHTML = ASUS_POLICY.Dict.ScrollDown;
        this.element.shadowRoot.querySelector('div.modal-footer').appendChild(closeBtn);


    }

    policy = "";
    closeBtn = null;

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
        } else if (isSupport("UI4")) {
            return "";
        } else {
            return theme;
        }
    }

    handleClickClose() {
        this.hide();
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

class DisagreeNoticeComponent {
    constructor(props) {
        const {ppVersion = 2} = props;
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
                            ${(ppVersion == 4) ?
            `<li>AFC</li>
                            <li>ASUS Expert Site Manager</li>` : ``}
                        </ul>
                    </div>
                </div>
            </div>`;
    }

    render() {
        return this.element;
    }
}

class PolicyWithdrawModalComponent extends PolicyModalComponent {
    constructor(props) {
        super(props);
        const {
            id = 'policy_popup_modal',
            theme = this.getTheme(),
            securityUpdate = false,
            websUpdate = false,
            policyStatus = policy_status,
            singPPVersion = 0,
            agreeCallback = function () {
            },
            disagreeCallback = function () {
            },
            readAgainCallback = function () {
            },
            knowRiskCallback = function () {
            },
        } = props;
        this.id = id;
        this.singPPVersion = singPPVersion;
        if (policyStatus.PP == 0) {
            this.ppVersion = policyStatus.ASUS_PP_support;
        } else {
            this.ppVersion = policyStatus.PP;
        }
        if (singPPVersion > 0 && policyStatus.PP == 2) {
            this.ppVersion = singPPVersion;
        }
        this.securityUpdate = securityUpdate;
        this.websUpdate = websUpdate;
        this.policyStatus = policyStatus;
        httpApi.log('policy_status', JSON.stringify(policyStatus));

        this.agreeCallback = agreeCallback;
        this.disagreeCallback = disagreeCallback;
        this.readAgainCallback = readAgainCallback;
        this.knowRiskCallback = knowRiskCallback;

        const div = document.createElement('div');
        div.id = id;
        const shadowRoot = div.attachShadow({mode: 'open'});
        const template = document.createElement('template');
        template.innerHTML = `
          ${ASUS_POLICY.PolicyModalStyle}
          <div class="popup_bg ${theme}">
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

        this.loadingImg = document.createElement('i');
        this.loadingImg.className = 'gg-spinner';

        shadowRoot.appendChild(template.content.cloneNode(true));

        this.element = div;

        const disagreeNoticeComponent = new DisagreeNoticeComponent({ppVersion: this.ppVersion});
        this.element.shadowRoot.querySelector('div.modal-body').appendChild(disagreeNoticeComponent.render());
        this.element.shadowRoot.querySelector('button.know-risk').addEventListener('click', this.handleClickKnowRisk.bind(this));
        this.element.shadowRoot.querySelector('button.read-again').addEventListener('click', this.handleClickReadAgain.bind(this));
    }


    handleClickKnowRisk(e) {
        e.target.innerHTML = '';
        e.target.appendChild(this.loadingImg);
        httpApi.privateEula.set("0", async () => {
            await this.knowRiskCallback();
            await this.hide();
            await this.checkPolicy();
        })
    }

    handleClickReadAgain() {
        this.hide();
        this.readAgainCallback();
        if (window.location.pathname.toUpperCase().search("QIS_") < 0) {
            const policyModal = new PolicyModalComponent({
                policy: "PP",
                policyStatus: this.policyStatus,
                securityUpdate: this.securityUpdate,
                websUpdate: this.websUpdate,
                singPPVersion: this.singPPVersion,
                agreeCallback: this.agreeCallback,
                disagreeCallback: this.disagreeCallback,
                readAgainCallback: this.readAgainCallback,
                knowRiskCallback: this.knowRiskCallback
            });
            policyModal.show();
        }
    }
}

class PolicyUpdateModalComponent extends PolicyModalComponent {
    constructor(props) {
        super(props);
        const {
            id = 'policy_popup_modal', theme = this.getTheme(), securityUpdate = true, websUpdate = true,
            policyStatus = policy_status,
        } = props;
        this.id = id;
        this.securityUpdate = securityUpdate;
        this.websUpdate = websUpdate;
        this.policyStatus = policyStatus;
        httpApi.log('policy_status', JSON.stringify(policyStatus));

        const div = document.createElement('div');
        div.id = id;
        const shadowRoot = div.attachShadow({mode: 'open'});
        const template = document.createElement('template');
        template.innerHTML = `
          ${ASUS_POLICY.PolicyModalStyle}
          <div class="popup_bg ${theme}">
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

        shadowRoot.appendChild(template.content.cloneNode(true));
        shadowRoot.querySelector('button.ok').addEventListener('click', this.handleClickReadEula.bind(this));

        this.element = div;
    }

    handleClickReadEula() {
        this.hide();
        const policyModal = new PolicyModalComponent({
            id: 'policy_popup_modal',
            policy: "EULA",
            securityUpdate: this.securityUpdate,
            websUpdate: this.websUpdate,
            policyStatus: this.policyStatus
        });
        policyModal.show();
    }
}

class ThirdPartyPolicyModalComponent extends PolicyModalComponent {
    constructor(props) {
        super(props);
        this.props = props;
    }
}

class QisPolicyPageComponent {
    constructor(props) {
        const {
            policy, theme = this.getTheme(), policyStatus = policy_status
        } = props;
        this.policyStatus = policyStatus;
        if (policyStatus.PP == 0) {
            this.ppVersion = policyStatus.ASUS_PP_support;
        } else {
            this.ppVersion = policyStatus.PP;
        }
        httpApi.log('policy_status', JSON.stringify(policyStatus));
        const div = document.createElement('div');
        div.style.height = '100%';
        const shadowRoot = div.attachShadow({mode: 'open'});
        const template = document.createElement('template');
        let title = `${ASUS_POLICY.Content[policy].Title}`
        if (policy === "EULA") {
            if (this.policyStatus.Policy_lang === "CN") {
                title = title.replace(/(.{4})/g, '<div>$1</div>');
            } else if (this.policyStatus.Policy_lang === "TW") {
                title = title.replace(/(.{5})/g, '<div>$1</div>');
            }
        } else if (policy === "PP") {
            if (this.policyStatus.Policy_lang === "CN") {
                title = title.replaceAll(' ', '').replace(/(.{4})/g, '<div>$1</div>');
            } else if (this.policyStatus.Policy_lang === "TW") {
                title = title.replace(/(.{4})/g, '<div>$1</div>');
            }
        }
        template.innerHTML = `
            <style>
                :host {
                    --rt-primary: #006CE1;
                    --business-primary: #006CE1;
                    --rog-primary: #FF1929;
                    --tuf-primary: #FFAA32;
                    --ts-primary: #2ED9C3;
                    
                    --business-notice: #B42D18;
                    --rt-notice: #E75B4B;
                    --rog-notice: #00D5FF;
                    --tuf-notice: #00D5FF;
                    --ts-notice: #2ED9C3;
                }
                .bg {
                    background-color: #F5F5F5;
                    width: 100%;
                    height: -webkit-fill-available;
                    background-size: cover;
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
                    display: flex;
                    flex-direction: row;
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
                        width: 95%;
                    }
                    
                    .page-title{
                        font-size: 3em;
                        display: flex;
                        flex-direction: row;
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
                        width: 90%;
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
                        display: flex;
                        flex-direction: column;
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

                .bg.TS{
                    background-color: transparent;
                }
                .bg.TS .header{
                    border: none;
                    background-color: transparent;
                    height: 110px;
                }
                .bg.TS .header .header-name{
                    display: none;
                }
                .bg.TS .icon-logo{
                    --logo-svg: unset;
                    background:url('../images/New_ui/logo_TX.png') no-repeat left bottom;
                    height: 96px;
                    width: 450px;
                    min-width: 450px;
                    margin-left: 230px;
                }
                .bg.TS .page-title{
                    color: var(--ts-primary);
                }
                .bg.TS .page-desc{
                    color: #FFF;
                }
                .bg.TS .scroll-info{
                    color: var(--ts-notice);
                }
                .bg.TS .btn{
                    color: #FFF;
                    border-radius: 8px;
                    border: 2px solid #2ED9C3;
                }
                .bg.TS .btn.disabled{
                    background-color: #222222;
                    color: #999999;
                }
                .bg.TS .btn.disabled::after{
                    --borderColor: #2ED9C3;
                    border-radius: 8px;
                }
                .bg.TS .btn.agree{
                    background-color: #141618;
                    color: #FFF;
                }
                .bg.TS .btn.agree.disabled{
                    background-color: #141618;
                    color: #FFF;
                    border: 2px solid #125B51;
                }
                .bg.TS .btn.disagree{
                    background-color: #222222;
                }
                .bg.TS .btn.disabled:hover{
                    box-shadow: none;
                }
                .bg.TS .checkbox-wrapper-40 {
                    --borderColor: var(--ts-primary);
                    color: #FFF;
                }
                .bg.TS .toolbar-btn{
                    color: #DCDCDC;
                }
                .bg.TS .toolbar-btn .lang-icon{
                    background-color: #DCDCDC;
                }
                .bg.TS .dropdown-menu{
                    background-color: #0A0A0A;
                    border: 1px solid #4D4D4D;
                    border-radius: 0;
                }
                .bg.TS .dropdown-menu > ul > li{
                    color: #B3B3B3;
                }
                .bg.TS .dropdown-menu > ul > li:hover{
                    background-color: #262626;
                    border-radius: 0;rgb(255, 87, 34)
                    color: var(--tuf-primary);
                }
                @media screen and (min-width: 576px){
                    .bg.TS .btn:hover{
                        box-shadow: 0 0 8px 0 #2ED9C3;
                    }
                    .bg.TS .btn::after{
                        content: "";
                        height: 100%;
                        min-width: 200px;
                        max-width: 300px;
                        width: 25%;
                        position: absolute;
                        z-index: 3;
                        border-radius: 8px;
                        --borderColor: #2ED9C3;
                        background: linear-gradient(to left, var(--borderColor), var(--borderColor)) left top no-repeat, linear-gradient(to bottom, var(--borderColor), var(--borderColor)) left top no-repeat, linear-gradient(to left, var(--borderColor), var(--borderColor)) right top no-repeat, linear-gradient(to bottom, var(--borderColor), var(--borderColor)) right top no-repeat, linear-gradient(to left, var(--borderColor), var(--borderColor)) left bottom no-repeat, linear-gradient(to bottom, var(--borderColor), var(--borderColor)) left bottom no-repeat, linear-gradient(to left, var(--borderColor), var(--borderColor)) right bottom no-repeat, linear-gradient(to left, var(--borderColor), var(--borderColor)) right bottom no-repeat;
                        background-size: 1px 4px, 4px 1px, 1px 4px, 4px 1px;
                    }
                }

            </style>
            <div class="bg ${theme}">
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
                        <div class="page-title">${title}</div>
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
                    readAgainCallback: clickPopupReadAgainBtn,
                    knowRiskCallback: clickPopupKnowRiskBtn,
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
                    httpApi.privateEula.set("1", function () {
                        httpApi.securityUpdate.set(1);
                        if (policyStatus.ASUS_PP_AutoWebUpgradeDisable !== "1") {
                            httpApi.nvramSet({
                                "webs_update_enable": 1,
                                "action_mode": "apply",
                                "rc_service": "saveNvram",
                                "skip_modify_flag": "1"
                            }, () => {
                            }, false);
                        }
                        window.goTo.Welcome();
                    })
                } else if (policy === "EULA") {
                    const yearChecked = shadowRoot.querySelector("#ASUS_EULA_enable").checked;

                    if (!yearChecked) {
                        alert(`${ASUS_POLICY.Dict.AgeConfirm}`);
                        shadowRoot.querySelector(".checkbox-wrapper-40").style.color = top.businessWrapper ? "red" : "#ff5722";
                        return false;
                    }
                    applyBtn.innerHTML = Get_Component_btnLoading();
                    httpApi.newEula.set("1", function () {
                        goTo.PP();
                    })
                }
            }
        }

        const clickPopupReadAgainBtn = () => {
            // enableBtn(abortBtn, false);
            // enableBtn(applyBtn, false);
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
            goTo.Welcome();
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
            abortBtn.className = "btn disagree";
            abortBtn.textContent = ASUS_POLICY.Dict.Disagree;
            shadowRoot.querySelector('.page-footer').appendChild(abortBtn);
            abortBtn.addEventListener("click", handleClickAbortBtn);

        }

        applyBtn = document.createElement('div');
        applyBtn.className = (policy === "EULA") ? "btn agree disabled" : "btn disagree";
        applyBtn.textContent = ASUS_POLICY.Dict.Agree;
        shadowRoot.querySelector('.page-footer').appendChild(applyBtn);
        applyBtn.addEventListener('click', handleClickApplyBtn);

        const scrollDiv = new PolicyScrollDiv({
            policy: policy, theme: theme, scrollCallBack: checkScrollHeight
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
        } else {
            this.element.shadowRoot.querySelector('.scroll-info').remove();
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

        const dropdownMenu = shadowRoot.querySelector(".dropdown-menu");

        if (dropdownMenu) {
            dropdownMenu.addEventListener('click', (e) => {
                const item = e.target.closest("li[data-lang]");
                if (item) {
                    const lang = item.getAttribute("data-lang");
                    changeLanguage(lang);
                }
            });
        }

        const entries = Object.entries(system_language.supportList);
        const halfLength = Math.ceil(entries.length / 2);
        const firstUl = shadowRoot.querySelector(".dropdown-menu ul:first-child");
        const lastUl = shadowRoot.querySelector(".dropdown-menu ul:last-child");

        if (firstUl && lastUl) {
            const fragmentFirstUl = document.createDocumentFragment();
            const fragmentLastUl = document.createDocumentFragment();

            for (let i = 0; i < entries.length; i++) {
                const [key, value] = entries[i];
                const li = document.createElement("li");
                li.setAttribute("data-lang", key);
                li.textContent = value;
                if (i < halfLength) {
                    fragmentFirstUl.appendChild(li);
                } else {
                    fragmentLastUl.appendChild(li);
                }
            }
            firstUl.appendChild(fragmentFirstUl);
            lastUl.appendChild(fragmentLastUl);
        }

        function changeLanguage(lang) {
            httpApi.nvramSet({
                action_mode: "apply",
                rc_service: "email_info",
                flag: "set_language",
                preferred_lang: lang,
                "skip_modfiy_flag": "1"
            }, function () {
                setTimeout(function () {
                    location.reload();
                }, 10);
            });
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
        } else if (isSupport("UI4")) {
            return "";
        } else if (isSupport("TS_UI")) {
            return "TS";
        } else {
            return theme;
        }
    }

    render() {
        return this.element;
    }
}

function showPersonalData() {
    const PersonalDataModal = new PersonalDataModalComponent({});
    PersonalDataModal.show();
}


if (typeof module !== 'undefined') {
    module.exports = {
        PolicyModalComponent, PolicyWithdrawModalComponent, ThirdPartyPolicyModalComponent
    };
}
