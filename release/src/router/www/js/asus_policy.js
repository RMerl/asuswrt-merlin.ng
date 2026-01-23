const ASUS_POLICY = {
    Dict: {
        ScrollDown: `<#ASUS_POLICY_Scroll_Down#>`,
        AgeCheck: `<#ASUS_POLICY_Age_Check#>`,
        AgeConfirm: `<#ASUS_eula_age_confirm#>`,
        I_Understand: `<#ASUS_EULA_I_Understand#>`,
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
            Versions: [2, 4, 5],
            Title: `<#ASUS_PP_Title#>`,
            Title_v5: `<#ASUS_Notice#>`,
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
                    <div class="policy_title" role="heading" aria-level="1"><#ASUS_PP_Title#></u></b></div>
                    <div class="policy_desc"><#ASUS_PP_Desc_v2#></div>
                    <div class="policy_desc">${`<#ASUS_PP_Desc_2#>`.replace(/\[aa\](.*?)\[\/aa\]/g, `<span class="link" onclick="showPersonalData(2)" style="text-decoration: underline;cursor: pointer;">$1</span>`).replaceAll("%1$@", `<#menu5_6#>`).replaceAll("%2$@", `<#menu_privacy#>`)}</div>
                    <div class="policy_desc"><#ASUS_PP_Desc_3#></div>
                    <div class="policy_desc">${`<#ASUS_PP_Desc_4#>`.replaceAll("%@", `<#CTL_Decline#>`)}</div>
                    <div class="policy_desc">${`<#ASUS_PP_Desc_5#>`.replace(/\[aa\](.*?)\[\/aa\]/g, `<a target="_blank" class="url-policy" href="https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Policy&lang=EN&kw=&num=" style="text-decoration: underline;cursor: pointer;">$1</a>`)}</div>
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
                    <div class="policy_title" role="heading" aria-level="1"><#ASUS_PP_Title#></u></b></div>
                    <div class="policy_desc"><#ASUS_PP_Desc_v4#></div>
                    <div class="policy_desc">${`<#ASUS_PP_Desc_2#>`.replace(/\[aa\](.*?)\[\/aa\]/g, `<span class="link" onclick="showPersonalData(4)" style="text-decoration: underline;cursor: pointer;">$1</span>`).replaceAll("%1$@", `<#menu5_6#>`).replaceAll("%2$@", `<#menu_privacy#>`)}</div>
                    <div class="policy_desc"><#ASUS_PP_Desc_3#></div>
                    <div class="policy_desc"><#ASUS_PP_Desc_4#></div>
                    <div class="policy_desc">${`<#ASUS_PP_Desc_5#>`.replace(/\[aa\](.*?)\[\/aa\]/g, `<a target="_blank" class="url-policy" href="https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Policy&lang=EN&kw=&num=" style="text-decoration: underline;cursor: pointer;">$1</a>`)}</div>
                    <div class="policy_desc"><#ASUS_PP_Desc_6#></div>
                    <div class="policy_show"></div>
                </div>`,
            HTMLv5: `<style>
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
                    <div class="policy_title" role="heading" aria-level="1"><#ASUS_PP_Title_v5#></u></b></div>
                    <div class="policy_desc"><#ASUS_PP_Desc_v4#></div>
                    <div class="policy_desc">
                        ${`<#ASUS_PP_Desc_2#>`.replace(/\[aa\](.*?)\[\/aa\]/g, `<span class="link" onclick="showPersonalData(4)" style="text-decoration: underline;cursor: pointer;">$1</span>`).replaceAll("%1$@", `<#menu5_6#>`).replaceAll("%2$@", `<#menu_privacy#>`)} 
                        <#ASUS_PP_Desc_3#>
                        ${
                `<#ASUS_PP_Desc_4_v5#>`
                    .replaceAll('<', '&lt;')
                    .replaceAll('>', '&gt;')
                    .replace("%@", `<#CTL_Decline#>`)
                    .replace(/\[aa\](.*?)\[\/aa\]/g, `<span class="link" onclick="showPersonalData(4)" style="text-decoration: underline;cursor: pointer;">$1</span>`)
            }
                    </div>
                    <div class="policy_desc"><#ASUS_PP_Desc_EU_1#>: ${`<#ASUS_PP_Desc_EU_2#>`.replace(/\[aa\](.*?)\[\/aa\]/g, `<span class="link" onclick="showPersonalData(4)" style="text-decoration: underline;cursor: pointer;">$1</span>`)}</div>
                    <div class="policy_desc">${`<#ASUS_PP_Desc_5_v5#>`.replace(/\[aa\](.*?)\[\/aa\]/g, `<a target="_blank" class="url-policy" href="https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Policy&lang=EN&kw=&num=" style="text-decoration: underline;cursor: pointer;"><#ASUS_PP_Desc_EU_3#></a>`)}</div>
                    <div class="policy_desc"><#ASUS_PP_Desc_6#></div>
                    <div class="policy_show"></div>
                </div>`,
            PersonalData: `<#ASUS_Notice#>`,
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
                        <li>${`<#ASUS_Personal_Law_5#>`.replace(/\[aa\](.*?)\[\/aa\]/g, `<a target="_blank" class="url-policy" href="https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Policy&lang=EN&kw=&num=" style="text-decoration: underline;cursor: pointer;">$1</a>`)}</li>                    </ol>
                </div>
                <div class="policy_show"></div>
            `,
            PersonalDataHTMLv5: `
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
                    <div class="mb-1"><b><#ASUS_Personal_Title#></b></div>
                    <div class="mb-1"><b><#ASUS_Personal_Part_1#></b></div>
                    <div class="mb-1"><#ASUS_Personal_Desc#></div>
                    <ol>
                        <li>
                            <div class="mb-1"><#ASUS_Personal_Law_1_1#></div>
                            <div class="mb-1"><#ASUS_Personal_Law_1_2#></div>
                            <div>${`<#ASUS_Personal_Law_1_3_v5#>`.replace('[%@]', `[<a target="_blank" href="https://www.asus.com/support/faq/1053743/">https://www.asus.com/support/faq/1053743/</a>]`)}</div>
                        </li>
                        <li>${`<#ASUS_Personal_Law_2_v5#>`.replaceAll('<', '&lt;')
                .replaceAll('>', '&gt;')
                .replace("%@", `<#CTL_Decline#>`)}
                            <ol>
                                <li><#ASUS_Personal_Law_2_1_1#><#ASUS_Personal_Law_2_1_2#></li>
                                <li><#ASUS_Personal_Law_2_2#></li>
                            </ol>
                        </li>
                        <li><#ASUS_Personal_Law_3#></li>
                        <li><#ASUS_Personal_Law_4#></li>
                        <li>${`<#ASUS_Personal_Law_5#>`.replace(/\[aa\](.*?)\[\/aa\]/g, `<a target="_blank" class="url-policy" href="https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Policy&lang=EN&kw=&num=" style="text-decoration: underline;cursor: pointer;"><#ASUS_PP_Desc_EU_3#></a>`)}</li>
                    </ol>
                    <div class="mb-1"><b><#ASUS_Personal_Part_2#></b></div>
                    <div class="mb-1"><#ASUS_Personal_EU_Desc#></div>
                    <ol>
                        <li><#ASUS_Personal_EU_Law_1#>
                            <ol>
                                <li><#ASUS_Personal_EU_Law_1_1#></li>
                                <li><#ASUS_Personal_EU_Law_1_2#></li>
                                <li>${`<#ASUS_Personal_EU_Law_1_3#>`.replace('%@', '10 MB')}</li>
                            </ol>                            
                        </li>
                        <li><#ASUS_Personal_EU_Law_2#></li>
                        <li><#ASUS_Personal_EU_Law_3#>
                            <ol>
                                <li><#ASUS_Personal_EU_Law_3_1#></li>
                                <li><#ASUS_Personal_EU_Law_3_2#></li>
                            </ol>
                        </li>
                        <li><#ASUS_Personal_EU_Law_4#>
                            <ol>
                                <li>${`<#ASUS_Personal_EU_Law_4_1#>`.replace(/\[aa\](.*?)\[\/aa\]/g, `<a class="url-personal-asus-privacy" target="_blank" href="https://nw-dlcdnet.asus.com/support/forward.html?model=&type=AsusPrivacy&lang=EN&kw=&num=">$1</a>`)}</li>
                            </ol>
                        </li>
                    </ol>
                </div>
                <div class="policy_show"></div>
            `,
            FeedbackNoticeHTML: `
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
                    <div class="mb-1"><b>${`<#ASUS_Feedback_Law_1#>`.replace(/\[aa\](.*?)\[\/aa\]/g, `<a class="url-policy" target="_blank" href="https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Policy&lang=EN&kw=&num=">$1</a>`)}</b></div>
                    <div class="mb-1"><b><#ASUS_PP_Desc_EU_1#>: <#ASUS_Feedback_Law_2#></b></div>
                    <ol>
                        <li><#ASUS_Feedback_Law_2_1#>
                            <ol>
                                <li><#ASUS_Feedback_Law_2_1_1#></li>
                                <li><#ASUS_Feedback_Law_2_1_2#></li>
                                <li>${`<#ASUS_Feedback_Law_2_1_3#>`.replace('[%@]', '10 MB')}</li>
                            </ol>
                        </li>
                        <li><#ASUS_Feedback_Law_2_2#></li>
                        <li><#ASUS_Feedback_Law_2_3#>
                            <ol>
                                <li><#ASUS_Feedback_Law_2_3_1#></li>
                                <li><#ASUS_Feedback_Law_2_3_2#></li>
                            </ol>
                        </li>
                        <li>${`<#ASUS_Feedback_Law_2_4#>`.replace(/\[aa\](.*?)\[\/aa\]/g, `<a class="url-personal-asus-privacy" target="_blank" href="https://nw-dlcdnet.asus.com/support/forward.html?model=&type=AsusPrivacy&lang=EN&kw=&num=">$1</a>`)}</li>
                    </ol>
                </div>
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
                    <div class="policy_title" role="heading" aria-level="1"><#ASUS_EULA_Title_B#></u></b></div>
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
                        <div>${`<#TM_eula_new_desc1#>`.replace('%@', `<#AiProtection_two-way_IPS#>`)}</div>
						<div>${`<#TM_eula_new_desc2#>`.replace('%@', `<#AiProtection_two-way_IPS#>`)}</div>
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
        },
        AIBOARD_EULA: {
            Title: `<#ASUS_Aiboard_EULA_Title#>`,
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
                    .policy_law > ol > li{
                        font-weight: bold;
                    }
                    .policy_law > ol > li > div:first-child{
                        font-weight: normal;
                    }
                    .policy_law > ol > li:not(:first-child) li{
                        font-weight: bold;
                    }
                    .policy_law > ol > li:not(:first-child) li div{
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
                    <div class="policy_desc"><#ASUS_Aiboard_EULA_Desc#></div>
                    <div class="policy_law">
                        <ol>
                            <li><#ASUS_Aiboard_EULA_Law_1#>
                                <ol>
                                    <li><#ASUS_Aiboard_EULA_Law_1_1#></li>
                                    <li><#ASUS_Aiboard_EULA_Law_1_2#></li>
                                    <li><#ASUS_Aiboard_EULA_Law_1_3#></li>
                                </ol>
                            </li>
                            <li><#ASUS_Aiboard_EULA_Law_2#>
                                <ol>
                                    <li><#ASUS_Aiboard_EULA_Law_2_1#>
                                        <div><#ASUS_Aiboard_EULA_Law_2_1_Desc#></div>
                                    </li>
                                    <li><#ASUS_Aiboard_EULA_Law_2_2#>
                                        <div><#ASUS_Aiboard_EULA_Law_2_2_Desc#></div>
                                    </li>
                                    <li><#ASUS_Aiboard_EULA_Law_2_3#>
                                        <div><#ASUS_Aiboard_EULA_Law_2_3_Desc#></div>
                                    </li>
                                    <li><#ASUS_Aiboard_EULA_Law_2_4#>
                                        <div><#ASUS_Aiboard_EULA_Law_2_4_Desc#></div>
                                    </li>
                                    <li><#ASUS_Aiboard_EULA_Law_2_5#>
                                        <div><#ASUS_Aiboard_EULA_Law_2_5_Desc#></div>
                                    </li>
                                    <li><#ASUS_Aiboard_EULA_Law_2_6#>
                                        <div><#ASUS_Aiboard_EULA_Law_2_6_Desc#></div>
                                    </li>
                                    <li><#ASUS_Aiboard_EULA_Law_2_7#>
                                        <div><#ASUS_Aiboard_EULA_Law_2_7_Desc#></div>
                                    </li>
                                    <li><#ASUS_Aiboard_EULA_Law_2_8#>
                                        <div><#ASUS_Aiboard_EULA_Law_2_8_Desc#></div>
                                    </li>
                                    <li><#ASUS_Aiboard_EULA_Law_2_9#>
                                        <div><#ASUS_Aiboard_EULA_Law_2_9_Desc#></div>
                                    </li>
                                </ol>
                            </li>
                        </ol>
                    </div>
                </div>
            `,
        }
    },

    PolicyModalStyle: `<style>
        ${!(isSupport("UI4") || isSupport("business")) ? `@import "/css/color-table.css";` : ``}
        .popup_bg {
            position: fixed;
            top: 0;
            right: 0;
            bottom: 0;
            left: 0;
            z-index: 2000;
            backdrop-filter: blur(2px);
            -webkit-backdrop-filter: blur(2px);
            color: var(--body-text-color);
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
            background-color: var(--popup-bg-color);
            background-clip: padding-box;
            border: 1px solid var(--policy-border-color);
            border-radius: var(--global-radius);
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
            color: var(--primary-60);
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
            background-color: transparent;
            transition: color .15s ease-in-out, background-color .15s ease-in-out, border-color .15s ease-in-out, box-shadow .15s ease-in-out;
            padding: 0.5rem 1rem;
            border-radius: var(--global-radius);
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
            color: var(--primary-btn-text-color);
            background-color: var(--primary-btn-normal-fill);
            border-color: var(--primary-btn-normal-fill);
            border-radius: var(--radius-sm);
            border-image: var(--primary-btn-normal-stroke);
        }
        .btn-primary:hover {
            background-color: var(--primary-btn-hover-fill);
            border-color: var(--primary-btn-normal-stroke);
        }
        .btn-primary.disabled, .btn-primary:disabled {
            color: var(--primary-btn-disable-text-color);
            background-color: var(--primary-btn-disable-fill);
            border-color: var(--primary-btn-disable-fill);
        }
        .btn-secondary {
            color: var(--secondary-btn-text-color);
            background-color: var(--secondary-btn-normal-fill);
            border-color: var(--secondary-btn-normal-stroke);
        }
        .btn-secondary:hover {
            background-color: var(--secondary-btn-hover-fill);
            color: var(--secondary-btn-hover-text-color);
        }
        .btn-secondary.disabled, .btn-secondary:disabled {
            color: var(--secondary-btn-disable-text-color);
            background-color: var(--secondary-btn-disable-stroke);
            border-color: var(--secondary-btn-disable-stroke);
        }
        .btn-outline-secondary {
            color: #6c757d;
            border-color: #6c757d;
        }
        .btn-outline-secondary:hover {
            color: #fff;
            background-color: #6c757d;
            border-color: #6c757d;
        }
        
        [role~="icon"] {
            -webkit-mask-repeat: no-repeat;
            mask-repeat: no-repeat;
            -webkit-mask-size: 100%;
            mask-size: 100%;
            transition: transform 0.35s ease;
            background-color: rgb(var(--color-icon-default));
        }
        
        .skip-btn {
            display: flex;
            align-items: center;    
            justify-content: center;
            border: none;
            background-color: transparent;
            cursor: pointer;
        }
        
        .skip-btn .icon-close {
            width: 24px;
            height: 24px;
            mask-image: url("data:image/svg+xml;utf8,%3Csvg%20width%3D%2224%22%20height%3D%2224%22%20viewBox%3D%220%200%2024%2024%22%20fill%3D%22none%22%20xmlns%3D%22http%3A%2F%2Fwww.w3.org%2F2000%2Fsvg%22%3E%0A%3Cpath%20d%3D%22M18%205.91003L6%2018.09%22%20stroke%3D%22black%22%20stroke-width%3D%221.2%22%20stroke-linecap%3D%22round%22%20stroke-linejoin%3D%22round%22%2F%3E%0A%3Cpath%20d%3D%22M6%205.91003L18%2018.09%22%20stroke%3D%22black%22%20stroke-width%3D%221.2%22%20stroke-linecap%3D%22round%22%20stroke-linejoin%3D%22round%22%2F%3E%0A%3C%2Fsvg%3E%0A");
        }

        .skip-btn:hover .icon-close {
            background-color: rgb(var(--color-icon-hover));
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
        .align-items-center {
            -webkit-box-align: center !important;
            -ms-flex-align: center !important;
            align-items: center !important;
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
            color: var(--policy-notice-title-color);
            font-weight: bold;
        }
        
        .notice_content {
            color: var(--body-text-color);
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

// Default policy status
let policy_status = {
    isFetchDone: false,
    Policy_lang: "EN",
    EULA: '',
    EULA_read: 1,
    EULA_allow_skip: 0,
    EULA_force_sign: 1,
    PP: '',
    PP_read: 1,
    PP_force_sign: 1,
    TM: "", TM_time: "",
    AIBOARD_EULA: '',
    AIBOARD_EULA_read: 1,
};

async function PolicyStatus() {
    let policy_status = {};

    try {
        const data = await httpApi.privateEula.get();
        policy_status.EULA = data.EULA;
        policy_status.EULA_read = data.EULA_read;
        policy_status.EULA_allow_skip = data.EULA_allow_skip;
        policy_status.EULA_force_sign = data.EULA_force_sign;
        policy_status.PP = data.PP;
        policy_status.PP_time = data.PP_time;
        policy_status.PP_read = data.PP_read;
        policy_status.PP_force_sign = data.PP_force_sign;
        policy_status.AIBOARD_EULA = data.AIBOARD_EULA;
        policy_status.AIBOARD_EULA_read = data.AIBOARD_EULA_read;
        policy_status.AIBOARD_EULA_allow_skip = data.AIBOARD_EULA_allow_skip;
        policy_status.AIBOARD_EULA_force_sign = data.AIBOARD_EULA_force_sign;
    } catch (e) {
        policy_status.isFetchDone = false;
        return policy_status;
    }

    const ui_support = await httpApi.hookGet("get_ui_support");
    const asus_pp_support = ui_support.asus_pp;

    const nvram_data = await httpApi.nvramGet([
        'TM_EULA', 'TM_EULA_time', 'preferred_lang', 'ASUS_PP_AutoWebUpgradeDisable'
    ], true);

    policy_status.TM = nvram_data.TM_EULA;
    policy_status.TM_time = nvram_data.TM_EULA_time;
    policy_status.Policy_lang = nvram_data.preferred_lang;
    policy_status.ASUS_PP_AutoWebUpgradeDisable = nvram_data.ASUS_PP_AutoWebUpgradeDisable;
    policy_status.ASUS_PP_support = asus_pp_support;
    policy_status.isFetchDone = true;

    return policy_status;
}

PolicyStatus().then(data => {
    policy_status = data;
});

class AccessibleLanguageDropdown {
    constructor(selectLangBtn, dropdownMenu, lang, httpApi) {
        this.lang = lang;
        this.httpApi = httpApi;

        // 狀態變數
        this.focusedIndex = -1;
        this.isDropdownOpen = false;
        this.typeaheadString = '';
        this.lastTypeahead = 0;

        // DOM 元素
        this.selectLang = selectLangBtn;
        this.dropdownMenu = dropdownMenu;

        this.init();
    }

    init() {
        if (!this.selectLang || !this.dropdownMenu) {
            console.error('Required DOM elements not found');
            return;
        }

        // const theme = this.getTheme();
        // if (theme !== "") {
        //     this.selectLang.parentNode.classList.add(theme);
        // }

        this.injectStyles();
        this.setupEventListeners();
        this.populateLanguageOptions();
    }

    injectStyles() {
        const styleId = 'accessible-language-dropdown-styles';

        // 檢查是否已經注入過樣式
        if (document.getElementById(styleId)) {
            return;
        }

        const style = document.createElement('style');
        style.id = styleId;
        style.textContent = `
        
            .d-flex {
                display: flex !important;
            }
            
            .align-items-center {
				-webkit-box-align: center !important;
				-ms-flex-align: center !important;
				align-items: center !important;
			}
			
			.gap-1 {
                gap: .25rem !important;
            }
			
			.line-height-unset {
			    line-height: unset !important;
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

            .dropdown-menu li[role="option"]{
                padding: 6px;
                cursor: pointer;
                font-size: 14px;
                color: #262626;
                text-align: inherit;
                text-decoration: none;
                white-space: nowrap;
                transition: background-color 0.2s ease;
                border: none;
                outline: none;
                border-radius: 4px;
            }

            .dropdown-menu li[role="option"][aria-selected="true"],
            [data-asuswrt-color="light"] .dropdown-menu li[role="option"][aria-selected="true"] {
                background-color: #c9c9c9;
            }

            .dropdown-menu li[role="option"]:hover,
            .dropdown-menu li[role="option"]:focus,
            [data-asuswrt-color="light"] .dropdown-menu li[role="option"]:hover,
            [data-asuswrt-color="light"] .dropdown-menu li[role="option"]:focus{
                background-color: #DCDCDC;
                border-radius: 4px;
                color: #181818;
            }

            .dropdown-menu li[role="option"]:focus {
                outline: 2px solid #007cba;
                outline-offset: -2px;
            }
            
            @media screen and (min-width: 576px){
                .dropdown-menu {
                    width: inherit;
                    left: unset;
                }
            }
            
            .qis-lang-btn {
                color: #FFF !important;
                border: 1px solid #666;
                border-radius: 5px;
                padding: 8px;
                background: #353535;
                cursor: pointer;
            }
            
            .qis-lang-dropdown-menu {
                flex-direction: column;
                height: inherit;
                max-height: 80vh;
                overflow-y: scroll;
            }
            
            .qis-lang-dropdown-menu::-webkit-scrollbar {
                width: 10px;
                height: 10px;
                background-color: #F5F5F5;
                border-radius: 0 8px 8px 0;
            }
            
            .qis-lang-dropdown-menu::-webkit-scrollbar-thumb {
                background-color: #7775;
                border-radius: 20px;
            }
            
            .qis-lang-dropdown-menu::-webkit-scrollbar-thumb:hover {
                background-color: #7777;
            }
            
            .qis-lang-dropdown-menu::-webkit-scrollbar-track:hover {
                background-color: #5555;
                border-radius: 8px;
            }
            
            .lang-icon {
                display: inline-block;
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
            
            
            
        `;

        this.selectLang.parentNode.appendChild(style);
    }

    getTheme() {
        let ui_support = httpApi.hookGet("get_ui_support");

        function isSupport(_ptn) {
            return ui_support[_ptn] ? true : false;
        }

        let theme = 'rt';
        if (isSupport("UI4")) {
            theme = 'asus'
        }
        if (isSupport("rog")) {
            return "rog";
        } else if (isSupport("tuf")) {
            return "tuf";
        } else if (isSupport("business")) {
            return "business";
        } else if (isSupport("TS_UI")) {
            return "ts";
        } else {
            return theme;
        }
    }

    setupEventListeners() {
        // 按鈕事件
        this.selectLang.addEventListener("click", (e) => this.handleButtonClick(e));
        this.selectLang.addEventListener("keydown", (e) => this.handleButtonKeydown(e));

        // 下拉選單事件
        this.dropdownMenu.addEventListener('keydown', (e) => this.handleDropdownKeydown(e));
        this.dropdownMenu.addEventListener('click', (e) => this.handleDropdownClick(e));
        this.dropdownMenu.addEventListener('mouseenter', (e) => this.handleMouseEnter(e));

        // 點擊外部關閉
        document.addEventListener('click', (e) => this.handleOutsideClick(e));
    }

    populateLanguageOptions() {
        const system_language = (() => {
            let list = this.httpApi.hookGet("language_support_list");
            return {supportList: {...list}};
        })();

        const entries = Object.entries(system_language.supportList);
        const halfLength = Math.ceil(entries.length / 2);
        const firstUl = this.dropdownMenu.querySelector("ul:first-child");
        const lastUl = this.dropdownMenu.querySelector("ul:last-child");

        if (firstUl && lastUl) {
            const fragmentFirstUl = document.createDocumentFragment();
            const fragmentLastUl = document.createDocumentFragment();

            for (let i = 0; i < entries.length; i++) {
                const [key, value] = entries[i];
                const li = document.createElement("li");
                li.setAttribute("role", "option");
                li.setAttribute("tabindex", "-1");
                li.setAttribute("data-lang", key);
                li.setAttribute("aria-setsize", "-1");
                li.setAttribute("aria-posinset", "-1");

                if (key === this.lang) {
                    li.setAttribute("aria-selected", "true");
                } else {
                    li.setAttribute("aria-selected", "false");
                }

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
    }

    clearFocusedOption() {
        this.dropdownMenu.querySelectorAll('li').forEach(option => {
            if (!option.getAttribute('aria-selected') || option.getAttribute('aria-selected') === 'false') {
                option.style.backgroundColor = '';
            }
            option.setAttribute('tabindex', '-1');
        });
    }

    setFocusedOption(index) {
        this.clearFocusedOption();

        const options = this.dropdownMenu.querySelectorAll('li');
        if (index >= 0 && index < options.length) {
            this.focusedIndex = index;
            const option = options[index];
            option.setAttribute('tabindex', '0');
            option.focus();

            // 確保選項在視窗內可見
            option.scrollIntoView({
                block: 'nearest',
                behavior: 'smooth'
            });
        }
    }

    openDropdown() {
        this.dropdownMenu.classList.add("show");
        this.selectLang.setAttribute("aria-expanded", "true");
        this.isDropdownOpen = true;

        // 設定初始焦點到已選擇的項目或第一個項目
        const selectedOption = this.dropdownMenu.querySelector('li[aria-selected="true"]');
        if (selectedOption) {
            const selectedIndex = Array.from(this.dropdownMenu.querySelectorAll('li')).indexOf(selectedOption);
            this.setFocusedOption(selectedIndex);
        } else {
            this.setFocusedOption(0);
        }

        // 將焦點移到下拉選單容器
        this.dropdownMenu.focus();
    }

    closeDropdown(returnFocus = true) {
        this.dropdownMenu.classList.remove("show");
        this.selectLang.setAttribute("aria-expanded", "false");
        this.isDropdownOpen = false;
        this.focusedIndex = -1;
        this.clearFocusedOption();

        // 只在需要時返回焦點到按鈕
        if (returnFocus) {
            this.selectLang.focus();
        }
    }

    moveFocus(direction) {
        const options = this.dropdownMenu.querySelectorAll('li');
        const newIndex = this.focusedIndex + direction;

        if (newIndex < 0) {
            this.setFocusedOption(options.length - 1);
        } else if (newIndex >= options.length) {
            this.setFocusedOption(0);
        } else {
            this.setFocusedOption(newIndex);
        }
    }

    selectCurrentOption() {
        const options = this.dropdownMenu.querySelectorAll('li');
        if (this.focusedIndex >= 0 && this.focusedIndex < options.length) {
            const option = options[this.focusedIndex];
            const lang = option.getAttribute("data-lang");
            if (lang) {
                this.changeLanguage(lang);
                this.closeDropdown(false);
            }
        }
    }

    handleButtonClick(e) {
        e.preventDefault();
        e.stopPropagation();

        if (this.isDropdownOpen) {
            this.closeDropdown();
        } else {
            this.openDropdown();
        }
    }

    handleButtonKeydown(e) {
        switch (e.key) {
            case 'Enter':
            case ' ':
            case 'ArrowDown':
            case 'ArrowUp':
                e.preventDefault();
                if (!this.isDropdownOpen) {
                    this.openDropdown();
                }
                break;
            case 'Escape':
                if (this.isDropdownOpen) {
                    this.closeDropdown();
                }
                break;
        }
    }

    handleDropdownKeydown(e) {
        if (!this.isDropdownOpen) return;

        switch (e.key) {
            case 'ArrowDown':
                e.preventDefault();
                this.moveFocus(1);
                break;
            case 'ArrowUp':
                e.preventDefault();
                this.moveFocus(-1);
                break;
            case 'Home':
                e.preventDefault();
                this.setFocusedOption(0);
                break;
            case 'End':
                e.preventDefault();
                const options = this.dropdownMenu.querySelectorAll('li');
                this.setFocusedOption(options.length - 1);
                break;
            case 'Enter':
            case ' ':
                e.preventDefault();
                this.selectCurrentOption();
                break;
            case 'Escape':
                e.preventDefault();
                this.closeDropdown();
                break;
            case 'Tab':
                // 允許Tab鍵在選項間移動
                if (e.shiftKey) {
                    e.preventDefault();
                    this.moveFocus(-1);
                } else {
                    e.preventDefault();
                    this.moveFocus(1);
                }
                break;
            default:
                // 字母數字鍵快速跳轉
                this.handleTypeahead(e.key);
                break;
        }
    }

    handleDropdownClick(e) {
        const item = e.target.closest("li[data-lang]");
        if (item) {
            const lang = item.getAttribute("data-lang");
            this.changeLanguage(lang);
            this.closeDropdown();
        }
    }

    handleMouseEnter(e) {
        const item = e.target.closest('li[data-lang]');
        if (item) {
            const options = this.dropdownMenu.querySelectorAll('li');
            const index = Array.from(options).indexOf(item);
            this.setFocusedOption(index);
        }
    }

    handleOutsideClick(e) {
        if (!this.selectLang.contains(e.target) && !this.dropdownMenu.contains(e.target)) {
            this.closeDropdown(false);
        }
    }

    handleTypeahead(key) {
        const char = key.toLowerCase();
        const currentTime = Date.now();

        // 如果距離上次輸入超過1秒，重置搜尋字串
        if (currentTime - this.lastTypeahead > 1000) {
            this.typeaheadString = '';
        }

        this.typeaheadString += char;
        this.lastTypeahead = currentTime;

        // 尋找匹配的選項
        const options = this.dropdownMenu.querySelectorAll('li');
        for (let i = 0; i < options.length; i++) {
            const optionText = options[i].textContent.toLowerCase();
            if (optionText.startsWith(this.typeaheadString)) {
                this.setFocusedOption(i);
                break;
            }
        }
    }

    changeLanguage(lang) {
        this.httpApi.nvramSet({
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

    // 公開方法：手動開啟下拉選單
    open() {
        if (!this.isDropdownOpen) {
            this.openDropdown();
        }
    }

    // 公開方法：手動關閉下拉選單
    close() {
        if (this.isDropdownOpen) {
            this.closeDropdown(false);
        }
    }

    // 公開方法：獲取當前選擇的語言
    getCurrentLanguage() {
        const selectedOption = this.dropdownMenu.querySelector('li[aria-selected="true"]');
        return selectedOption ? selectedOption.getAttribute('data-lang') : null;
    }

    // 公開方法：設定選擇的語言（不觸發變更）
    setCurrentLanguage(lang) {
        const options = this.dropdownMenu.querySelectorAll('li');
        options.forEach(option => {
            if (option.getAttribute('data-lang') === lang) {
                option.setAttribute('aria-selected', 'true');
            } else {
                option.setAttribute('aria-selected', 'false');
            }
        });
    }

    // 公開方法：銷毀實例
    destroy() {
        document.removeEventListener('click', this.handleOutsideClick);
        // 移除其他事件監聽器會在元素被移除時自動清理
    }
}

class PolicyScrollDiv {

    constructor(props) {
        const {policy, theme = '', scrollCallBack, policyStatus = policy_status} = props;
        if (policyStatus.PP === 0 || policyStatus.PP === "" || (policyStatus.PP_read === 0 && policyStatus.PP > 0)) {
            this.ppVersion = policyStatus.ASUS_PP_support;
        } else {
            this.ppVersion = policyStatus.PP;
        }
        const supportPPVersion = ASUS_POLICY.Content.PP.Versions.includes(this.ppVersion) ? this.ppVersion : Math.max(...ASUS_POLICY.Content.PP.Versions);

        const div = document.createElement('div');
        const shadowRoot = div.attachShadow({mode: 'open'});
        const template = document.createElement('template');
        template.innerHTML = `
      <style>
        .policy-scroll-div {
            background-color: var(--popup-bg-color);
            margin: 5px;
            overflow-y: auto;
            padding: 15px 12px 0 12px;
            border: 1px solid #ddd;
            border-radius: 5px;
            height: 50vh;
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
        
      </style>
      <div class="policy-scroll-div ${theme}"></div>
    `;

        shadowRoot.appendChild(template.content.cloneNode(true));
        shadowRoot.querySelector('.policy-scroll-div').addEventListener('scroll', scrollCallBack.bind(this));
        if (policy) {
            if (policy == "PP") {
                shadowRoot.querySelector('.policy-scroll-div').innerHTML = ASUS_POLICY.Content[policy][`HTMLv${supportPPVersion}`]
                shadowRoot.querySelector('.policy-scroll-div').querySelector('.url-policy')?.setAttribute('href', `https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Policy&lang=` + policyStatus.Policy_lang + `&kw=&num=`);
            } else {
                shadowRoot.querySelector('.policy-scroll-div').innerHTML = ASUS_POLICY.Content[policy].HTML;
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
            signPPVersion = 0,
            modalSize = '',
            agreeBtnText = '',
            disagreeBtnText = '',
            closeBtnText = '',
            showDisagreeBtn = true,
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

        const hasAgreedBefore = policyStatus.PP >= 1;
        const shouldForcePopup = policyStatus.PP_read === 0 && (policyStatus.PP === "" || (policyStatus.PP >= 0 && policyStatus.PP_force_sign === 1));
        if (shouldForcePopup) {
            this.ppVersion = policyStatus.ASUS_PP_support;
        } else {
            if(hasAgreedBefore){
                this.ppVersion = policyStatus.PP;
            }else{
                this.ppVersion = policyStatus.ASUS_PP_support;
            }
        }
        if (signPPVersion > 0) {
            this.ppVersion = signPPVersion;
        }
        const supportPPVersion = ASUS_POLICY.Content.PP.Versions.includes(this.ppVersion) ? this.ppVersion : Math.max(...ASUS_POLICY.Content.PP.Versions);
        policyStatus.signPPVersion = signPPVersion;
        policyStatus.supportPPVersion = supportPPVersion;
        policyStatus.PPVersions = ASUS_POLICY.Content.PP.Versions;

        this.hasAgreedBefore = hasAgreedBefore;
        this.shouldForcePopup = shouldForcePopup;

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

        let color = "dark";
        if (isSupport("UI4") || theme == "business") {
            color = "light";
        }

        template.innerHTML = `
          ${ASUS_POLICY.PolicyModalStyle}
          <div class="popup_bg" data-asuswrt-color="${color}" data-asuswrt-theme="${theme}">
            <div class="modal" tabindex="0">
                <div class="modal-dialog ${modalSize}">
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

        // 設置 focus trap
        this.setupFocusTrap();

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
        closeBtn.innerHTML = closeBtnText || ASUS_POLICY.Dict.Ok;
        closeBtn.addEventListener('click', this.handleClickClose.bind(this));
        this.closeBtn = closeBtn;

        const agreeBtn = document.createElement('button');
        agreeBtn.type = 'button';
        agreeBtn.className = (policy === 'EULA') ? 'btn btn-primary agree disabled' : 'btn btn-primary disagree';
        agreeBtn.innerHTML = agreeBtnText || ASUS_POLICY.Dict.Agree;
        agreeBtn.addEventListener('click', this.handleClickAgree.bind(this));
        this.agreeBtn = agreeBtn;

        const disagreeBtn = document.createElement('button');
        disagreeBtn.type = 'button';
        disagreeBtn.className = (policy === 'EULA') ? 'btn btn-outline-secondary disagree disabled' : 'btn btn-primary disagree';
        disagreeBtn.innerHTML = disagreeBtnText || ASUS_POLICY.Dict.Disagree;
        disagreeBtn.addEventListener('click', this.handleClickDisagree.bind(this));
        this.disagreeBtn = disagreeBtn;

        const skipBtn = document.createElement('button');
        skipBtn.type = 'button';
        skipBtn.className = 'skip-btn';
        const skipIcon = document.createElement('i');
        skipIcon.setAttribute('role', 'icon');
        skipIcon.className = 'icon-close';
        skipBtn.appendChild(skipIcon);
        skipBtn.addEventListener('click', this.handleClickSkip.bind(this));
        this.skipBtn = skipBtn;

        if (policy) {
            if (policy == "PP") {
                this.element.shadowRoot.querySelector('div.modal-title').innerHTML = ASUS_POLICY.Content.PP.Title_v5;
                this.element.shadowRoot.querySelector('div.policy-scroll-div').innerHTML = ASUS_POLICY.Content[policy][`HTMLv${supportPPVersion}`];
                this.element.shadowRoot.querySelector('div.policy-scroll-div').querySelector('.url-policy')?.setAttribute('href', `https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Policy&lang=` + this.policyStatus.Policy_lang + `&kw=&num=`);
            } else {
                this.element.shadowRoot.querySelector('div.modal-title').innerHTML = ASUS_POLICY.Content[policy].Title;
                this.element.shadowRoot.querySelector('div.policy-scroll-div').innerHTML = ASUS_POLICY.Content[policy].HTML;
            }

            switch (policy) {
                case "EULA":
                    if (this.policyStatus.EULA < 1) {
                        const scrollInfoDiv = document.createElement('div');
                        scrollInfoDiv.className = "scroll-info";
                        scrollInfoDiv.innerHTML = ASUS_POLICY.Dict.ScrollDown;
                        this.element.shadowRoot.querySelector('div.modal-body').appendChild(scrollInfoDiv);
                        if (this.policyStatus.EULA_allow_skip === 1) {
                            this.element.shadowRoot.querySelector('div.modal-header').appendChild(skipBtn);
                        }else{
                            this.element.shadowRoot.querySelector('div.modal-body').appendChild(ageCheckDiv);
                        }
                        agreeBtn.innerHTML = ASUS_POLICY.Dict.I_Understand;
                        this.element.shadowRoot.querySelector('div.modal-footer').appendChild(agreeBtn);
                    } else {
                        this.element.shadowRoot.querySelector('div.modal-footer').appendChild(closeBtn);
                    }
                    break;
                case "PP":
                    if (this.shouldForcePopup) {
                        if (this.policyStatus.PP === "" && this.policyStatus.EULA_allow_skip === 1) {
                            this.element.shadowRoot.querySelector('div.modal-body').appendChild(ageCheckDiv);
                        }
                        this.element.shadowRoot.querySelector('div.modal-footer').appendChild(disagreeBtn);
                        this.element.shadowRoot.querySelector('div.modal-footer').appendChild(agreeBtn);
                    } else {
                        if (this.hasAgreedBefore) {
                            this.element.shadowRoot.querySelector('div.modal-footer').appendChild(closeBtn);
                        } else {
                            if (this.policyStatus.PP === "" && this.policyStatus.EULA_allow_skip === 1) {
                                this.element.shadowRoot.querySelector('div.modal-body').appendChild(ageCheckDiv);
                            }
                            this.element.shadowRoot.querySelector('div.modal-footer').appendChild(disagreeBtn);
                            this.element.shadowRoot.querySelector('div.modal-footer').appendChild(agreeBtn);
                        }
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
                case "AIBOARD_EULA":
                    if (this.policyStatus.AIBOARD_EULA < 1) {
                        if (showDisagreeBtn) {
                            disagreeBtn.classList.remove("disabled");
                            this.element.shadowRoot.querySelector('div.modal-footer').appendChild(disagreeBtn);
                        }
                        agreeBtn.classList.remove("disabled");
                        this.element.shadowRoot.querySelector('div.modal-footer').appendChild(agreeBtn);
                    } else {
                        this.element.shadowRoot.querySelector('div.modal-footer').appendChild(closeBtn);
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

    // 新增：設置 focus trap
    setupFocusTrap() {
        // 添加 keydown 事件監聽器來處理 tab trap
        this.element.addEventListener('keydown', this.handleKeyDown.bind(this));
    }

    // 新增：focus trap 相關方法
    getFocusableElements() {
        const shadowRoot = this.element.shadowRoot;
        const focusableSelector = 'button:not([disabled]), input:not([disabled]), a[href], [tabindex]:not([tabindex="-1"])';
        return Array.from(shadowRoot.querySelectorAll(focusableSelector));
    }

    handleKeyDown(event) {
        if (event.key === 'Tab') {
            const focusableElements = this.getFocusableElements();
            if (focusableElements.length === 0) return;

            const firstElement = focusableElements[0];
            const lastElement = focusableElements[focusableElements.length - 1];

            if (event.shiftKey) {
                // Shift + Tab (向後)
                if (this.element.shadowRoot.activeElement === firstElement || this.element.shadowRoot.activeElement === null) {
                    event.preventDefault();
                    lastElement.focus();
                }
            } else {
                // Tab (向前)
                if (this.element.shadowRoot.activeElement === lastElement) {
                    event.preventDefault();
                    firstElement.focus();
                }
            }
        }
    }

    focusFirstElement() {
        const focusableElements = this.getFocusableElements();
        if (focusableElements.length > 0) {
            focusableElements[0].focus();
        }
    }

    getTheme() {
        let ui_support = httpApi.hookGet("get_ui_support");

        function isSupport(_ptn) {
            return ui_support[_ptn] ? true : false;
        }

        let theme = 'rt';
        if (isSupport("UI4")) {
            theme = 'asus'
        }
        if (isSupport("rog")) {
            return "rog";
        } else if (isSupport("tuf")) {
            return "tuf";
        } else if (isSupport("business")) {
            return "business";
        } else if (isSupport("TS_UI")) {
            return "ts";
        } else {
            return theme;
        }
    }

    checkPolicy = (Read = '') => {
        PolicyStatus()
            .then(data => {
                switch (Read){
                    case 'EULA':
                        data.EULA_read = 1;
                        break;
                    case 'PP':
                        data.PP_read = 1;
                        break;
                    case 'AIBOARD_EULA':
                        data.AIBOARD_EULA_read = 1;
                        break;
                    default:
                        break;
                }
                httpApi.log('policy_status', JSON.stringify(data));
                if (data.EULA_read === 0) {
                    const policyModal = new PolicyUpdateModalComponent({
                        securityUpdate: true, websUpdate: true, policyStatus: data
                    });
                    policyModal.show();
                } else if (data.EULA_read === 1 && data.PP_read === 0) {
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
                    if (this.policyStatus.EULA_allow_skip === 0) {
                        let yearChecked = this.ageCheckbox.checked;
                        if (!yearChecked) {
                            alert(ASUS_POLICY.Dict.AgeConfirm);
                            this.ageLabel.style.color = top.businessWrapper ? "red" : "#ff5722";
                            return false;
                        }
                    }

                    this.agreeBtn.classList.add("disabled");
                    this.agreeBtn.innerHTML = '';
                    this.agreeBtn.appendChild(this.loadingImg);
                    httpApi.newEula.set(1, async () => {
                        await this.hide();
                        await this.checkPolicy(this.policy);
                    })

                    break
                case "PP":
                    if (this.policyStatus.PP_read === 0 && this.policyStatus.EULA_allow_skip === 1) {
                        let yearChecked = this.ageCheckbox.checked;
                        if (!yearChecked) {
                            alert(ASUS_POLICY.Dict.AgeConfirm);
                            this.ageLabel.style.color = top.businessWrapper ? "red" : "#ff5722";
                            return false;
                        }
                    }

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
                            await this.checkPolicy(this.policy);
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
                    break
                case "AIBOARD_EULA":
                    this.agreeBtn.classList.add("disabled");
                    this.disagreeBtn.classList.add("disabled");
                    this.agreeBtn.innerHTML = '';
                    this.agreeBtn.appendChild(this.loadingImg);
                    setTimeout(() => {
                        httpApi.AIBOARD_EULA.set("1", async () => {
                            await this.agreeCallback();
                            await this.hide();
                            await this.checkPolicy(this.policy);
                        })
                    }, 1000);
                    break
            }
        }
    }

    handleClickDisagree(e) {
        if (!e.target.classList.contains("disabled")) {
            switch (this.policy) {
                case "PP":
                    if (this.policyStatus.PP_read === 0 && this.policyStatus.EULA_allow_skip === 1) {
                        let yearChecked = this.ageCheckbox.checked;
                        if (!yearChecked) {
                            alert(ASUS_POLICY.Dict.AgeConfirm);
                            this.ageLabel.style.color = top.businessWrapper ? "red" : "#ff5722";
                            return false;
                        }
                    }

                    this.hide();
                    const policyModal = new PolicyWithdrawModalComponent({
                        policy: "PP",
                        policyStatus: this.policyStatus,
                        securityUpdate: this.securityUpdate,
                        websUpdate: this.websUpdate,
                        signPPVersion: this.signPPVersion,
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
                case "AIBOARD_EULA":
                    this.disagreeCallback();
                    this.hide();
            }
        }
    }

    handleClickSkip(e) {
        httpApi.newEula.set(0, async () => {
            await this.hide();
            await this.checkPolicy(this.policy);
        })

    }

    render() {
        setTimeout(() => {
            this.focusFirstElement();
        }, 100);
        return this.element;
    }

    show() {
        top.document.body.style.overflow = 'hidden';
        if (top.document.querySelector(`#${this.id}`) == null) {
            top.document.body.appendChild(this.element);
        }

        setTimeout(() => {
            this.focusFirstElement();
        }, 100);
    }

    hide() {
        top.document.body.style.removeProperty('overflow');
        this.element.remove();
    }
}

// PolicyWithdrawModalComponent 類別
class PolicyWithdrawModalComponent extends PolicyModalComponent {
    constructor(props) {
        super(props);
        const {
            id = 'policy_popup_modal',
            theme = this.getTheme(),
            securityUpdate = false,
            websUpdate = false,
            policyStatus = policy_status,
            signPPVersion = 0,
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
        const hasAgreedBefore = policyStatus.PP >= 1;
        const shouldForcePopup = policyStatus.PP_read === 0 && (policyStatus.PP === "" || (policyStatus.PP >= 0 && policyStatus.PP_force_sign === 1));
        if (shouldForcePopup) {
            this.ppVersion = policyStatus.ASUS_PP_support;
        } else {
            if(hasAgreedBefore){
                this.ppVersion = policyStatus.PP;
            }else{
                this.ppVersion = policyStatus.ASUS_PP_support;
            }
        }
        if (signPPVersion > 0) {
            this.ppVersion = signPPVersion;
        }
        const supportPPVersion = ASUS_POLICY.Content.PP.Versions.includes(this.ppVersion) ? this.ppVersion : Math.max(...ASUS_POLICY.Content.PP.Versions);
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

        let color = "dark";
        if (isSupport("UI4") || theme == "business") {
            color = "light";
        }

        const filename = window.location.pathname.split('/').pop();
        const thisPage = filename==="QIS_wizard.htm" ? 'qis' : '';

        template.innerHTML = `
          ${ASUS_POLICY.PolicyModalStyle}
          <div class="popup_bg" data-asuswrt-color="${color}" data-asuswrt-theme="${theme}" data-asuswrt-page="${thisPage}">
            <div class="modal" tabindex="0">
                <div class="modal-dialog">
                    <div class="modal-content">
                        <div class="modal-body">
                        </div>
                        <div class="modal-footer">
                            <button type="button" class="btn btn-outline-secondary know-risk">${ASUS_POLICY.Dict.KnowRisk}</button>
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

        const disagreeNoticeComponent = new DisagreeNoticeComponent({ppVersion: supportPPVersion});
        this.element.shadowRoot.querySelector('div.modal-body').appendChild(disagreeNoticeComponent.render());
        this.element.shadowRoot.querySelector('button.know-risk').addEventListener('click', this.handleClickKnowRisk.bind(this));
        this.element.shadowRoot.querySelector('button.read-again').addEventListener('click', this.handleClickReadAgain.bind(this));

        // 重新設置 focus trap，因為我們創建了新的元素
        this.setupFocusTrap();

        // 設置無障礙屬性以便屏幕閱讀器朗讀內容
        this.setupAccessibility();
    }

    handleClickKnowRisk(e) {
        e.target.innerHTML = '';
        e.target.appendChild(this.loadingImg);
        httpApi.privateEula.set("0", async () => {
            await this.knowRiskCallback();
            await this.hide();
            await this.checkPolicy(this.policy);
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
                signPPVersion: this.signPPVersion,
                agreeCallback: this.agreeCallback,
                disagreeCallback: this.disagreeCallback,
                readAgainCallback: this.readAgainCallback,
                knowRiskCallback: this.knowRiskCallback
            });
            policyModal.show();
        }
    }

    setupAccessibility() {
        const modalContent = this.element.shadowRoot.querySelector('.modal-content');
        const modalBody = this.element.shadowRoot.querySelector('.modal-body');

        // 設置 ARIA 屬性
        modalContent.setAttribute('role', 'dialog');
        modalContent.setAttribute('aria-modal', 'true');
        modalContent.setAttribute('aria-labelledby', 'withdraw-modal-title');
        modalContent.setAttribute('aria-describedby', 'withdraw-modal-content');

        // 創建隱藏的標題元素供屏幕閱讀器使用
        const hiddenTitle = document.createElement('div');
        hiddenTitle.id = 'withdraw-modal-title';
        hiddenTitle.textContent = ASUS_POLICY.Dict.PolicyWithdrawTitle || 'Withdraw Policy Notice';
        hiddenTitle.style.cssText = 'position: absolute; left: -10000px; width: 1px; height: 1px; overflow: hidden;';
        modalContent.appendChild(hiddenTitle);

        // 為內容區域設置 ID
        modalBody.id = 'withdraw-modal-content';

        // 設置 live region 來通知屏幕閱讀器內容變化
        const liveRegion = document.createElement('div');
        liveRegion.setAttribute('aria-live', 'polite');
        liveRegion.setAttribute('aria-atomic', 'true');
        liveRegion.style.cssText = 'position: absolute; left: -10000px; width: 1px; height: 1px; overflow: hidden;';
        modalContent.appendChild(liveRegion);
        this.liveRegion = liveRegion;
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

        const hasAgreedBefore = policyStatus.PP >= 1;
        const shouldForcePopup = policyStatus.PP_read === 0 && (policyStatus.PP === "" || (policyStatus.PP >= 0 && policyStatus.PP_force_sign === 1));
        if (shouldForcePopup) {
            this.ppVersion = policyStatus.ASUS_PP_support;
        } else {
            if(hasAgreedBefore){
                this.ppVersion = policyStatus.PP;
            }else{
                this.ppVersion = policyStatus.ASUS_PP_support;
            }
        }
        const supportPPVersion = ASUS_POLICY.Content.PP.Versions.includes(this.ppVersion) ? this.ppVersion : Math.max(...ASUS_POLICY.Content.PP.Versions);

        const div = document.createElement('div');
        div.id = id;
        const shadowRoot = div.attachShadow({mode: 'open'});
        const template = document.createElement('template');
        let color = "dark";
        if (isSupport("UI4") || theme == "business") {
            color = "light";
        }

        const filename = window.location.pathname.split('/').pop();
        const thisPage = filename==="QIS_wizard.htm" ? 'qis' : '';

        template.innerHTML = `
          ${ASUS_POLICY.PolicyModalStyle}
          <div class="popup_bg" data-asuswrt-color="${color}" data-asuswrt-theme="${theme}" data-asuswrt-page="${thisPage}">
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
        closeBtn.innerHTML = ASUS_POLICY.Dict.I_Understand;
        closeBtn.addEventListener('click', this.handleClickClose.bind(this));
        this.closeBtn = closeBtn;

        this.element.shadowRoot.querySelector('div.modal-title').innerHTML = ASUS_POLICY.Content.PP.PersonalData;
        this.element.shadowRoot.querySelector('div.policy-scroll-div').innerHTML = ASUS_POLICY.Content.PP[`PersonalDataHTMLv${supportPPVersion}`];
        this.element.shadowRoot.querySelector('div.policy-scroll-div').querySelector('.url-policy')?.setAttribute('href', `https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Policy&lang=${policyStatus.Policy_lang}&kw=&num=`);
        this.element.shadowRoot.querySelector('div.policy-scroll-div').querySelector('.url-personal-asus-privacy')?.setAttribute('href', `https://nw-dlcdnet.asus.com/support/forward.html?model=&type=AsusPrivacy&lang=${policyStatus.Policy_lang}&kw=&num=`);

        this.element.shadowRoot.querySelector('div.modal-footer').appendChild(closeBtn);


    }

    policy = "";
    closeBtn = null;

    getTheme() {
        let ui_support = httpApi.hookGet("get_ui_support");

        function isSupport(_ptn) {
            return ui_support[_ptn] ? true : false;
        }

        let theme = 'rt';
        if (isSupport("UI4")) {
            theme = 'asus'
        }
        if (isSupport("rog")) {
            return "rog";
        } else if (isSupport("tuf")) {
            return "tuf";
        } else if (isSupport("business")) {
            return "business";
        } else if (isSupport("TS_UI")) {
            return "ts";
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
                <div class="notice_title" role="heading" aria-level="1">${ASUS_POLICY.Dict.Notice}</div>
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
                            ${(ppVersion >= 4) ?
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
        let color = "dark";
        if (isSupport("UI4") || theme == "business") {
            color = "light";
        }
        template.innerHTML = `
          ${ASUS_POLICY.PolicyModalStyle}
          <div class="popup_bg" data-asuswrt-color="${color}" data-asuswrt-theme="${theme}">
            <div class="modal" tabindex="0">
                <div class="modal-dialog" tabindex="0">
                    <div class="modal-content">
                        <div class="modal-body">
                            <div class="d-flex flex-column gap-1">
                                <div class="notice_title" role="heading" aria-level="1">${ASUS_POLICY.Dict.Notice}</div>
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
            policy, theme = this.getTheme(), policyStatus = policy_status,
        } = props;
        this.policyStatus = policyStatus;
        const hasAgreedBefore = policyStatus.PP >= 1;
        const shouldForcePopup = policyStatus.PP_read === 0 && (policyStatus.PP === "" || (policyStatus.PP >= 0 && policyStatus.PP_force_sign === 1));
        if (shouldForcePopup) {
            this.ppVersion = policyStatus.ASUS_PP_support;
        } else {
            if(hasAgreedBefore){
                this.ppVersion = policyStatus.PP;
            }else{
                this.ppVersion = policyStatus.ASUS_PP_support;
            }
        }
        const supportPPVersion = ASUS_POLICY.Content.PP.Versions.includes(this.ppVersion) ? this.ppVersion : Math.max(...ASUS_POLICY.Content.PP.Versions);
        httpApi.log('policy_status', JSON.stringify(policyStatus));
        const div = document.createElement('div');
        div.style.height = '100%';
        const shadowRoot = div.attachShadow({mode: 'open'});
        const template = document.createElement('template');
        let title = `${ASUS_POLICY.Content[policy].Title}`
        let title_orig = title;
        if (policy === "PP") {
            if (supportPPVersion >= 5) {
                title = `${ASUS_POLICY.Content.PP.Title_v5}`
            } else {
                title = `${ASUS_POLICY.Content.PP.Title}`
            }
        }

        if (policy === "EULA") {
            if (this.policyStatus.Policy_lang === "CN") {
                title = title.replace(/(.{4})/g, '<div>$1</div>');
            } else if (this.policyStatus.Policy_lang === "TW") {
                title = title.replace(/(.{5})/g, '<div>$1</div>');
            }
        } else if (policy === "PP" && supportPPVersion < 5) {
            if (this.policyStatus.Policy_lang === "CN") {
                title = title.replaceAll(' ', '').replace(/(.{4})/g, '<div>$1</div>');
            } else if (this.policyStatus.Policy_lang === "TW") {
                title = title.replace(/(.{4})/g, '<div>$1</div>');
            }
        }
        template.innerHTML = `
            <style>                
                :host {
                    --ts-primary: #2ED9C3;
                    --business-notice: #B42D18;
                    --rt-notice: #E75B4B;
                    --rog-notice: #00D5FF;
                    --tuf-notice: #00D5FF;
                    --ts-notice: #2ED9C3;
                }       
                .d-flex {
                    display: flex !important;
                }
                .align-items-center {
                    -webkit-box-align: center !important;
                    -ms-flex-align: center !important;
                    align-items: center !important;
                }
                .gap-1 {
                    gap: .25rem !important;
                }
                .bg {
                    background-color: var(--qis-bg-color);
                    color: var(--body-text-color);
                    width: 100%;
                    height: -webkit-fill-available;
                    background-size: cover;
                }
                .header {
                    height: 60px;
                    background-color: var(--primary-60);
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
                    font-family: var(--model-name-font-style);
                    display: block;
                    font-weight: normal;
                    font-size: 1.5em;
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
                    color: var(--primary-60);
                    word-break: break-word;
                    display: flex;
                    flex-direction: row;
                }
                
                .page-content{
                    margin-bottom: 80px;
                }
                
                .page-desc{
                    font-size: 1em;
                    font-weight: bold;
                    margin: 16px 0;
                }
                .scroll-info {
                    font-weight: bold;
                    color: #ff5722;
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
    
                .dropdown-menu li[role="option"]{
                    padding: 6px;
                    cursor: pointer;
                    font-size: 14px;
                    color: #262626;
                    text-align: inherit;
                    text-decoration: none;
                    white-space: nowrap;
                    transition: background-color 0.2s ease;
                    border: none;
                    outline: none;
                    border-radius: 4px;
                }
    
                .dropdown-menu li[role="option"][aria-selected="true"]{
                    background-color: var(--primary-40);
                    color: var(--white-alpha-100);
                }
    
                .dropdown-menu li[role="option"]:hover,
                .dropdown-menu li[role="option"]:focus{
                    background-color: #DCDCDC;
                    border-radius: 4px;
                    color: #181818;
                }
    
                .dropdown-menu li[role="option"]:focus {
                    outline: 2px solid var(--primary-40);
                    outline-offset: -2px;
                }
                
                @media screen and (min-width: 576px){
                    .dropdown-menu {
                        width: inherit;
                        left: unset;
                    }
                }
                
                .lang-icon {
                    display: inline-block;
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
                    background: var(--primary-60);
                }
                
                .btn.disagree {
                    border: 1px solid var(--primary-60);
                    color: var(--primary-60);
                    background-color: transparent;
                }
                
                .btn.agree {
                    border: 1px solid var(--primary-60);
                }

                .btn.disabled {
                    box-shadow: none;
                    background-color: transparent;
                    border: 1px solid #999999;
                    cursor: not-allowed;
                    color: #818181;
                }
                
                .btn.disabled.agree {
                    border: 1px solid var(--primary-btn-disable-fill);
                    color: var(--primary-btn-disable-fill);
                }
                
                .btn.disabled.disagree {
                    background-color: #FFF;
                }
                
                .btn.abort{
                    background: #FFFFFF;
                    color: #007AFF;
                }
                
                [role~="icon"] {
                    -webkit-mask-repeat: no-repeat;
                    mask-repeat: no-repeat;
                    -webkit-mask-size: 100%;
                    mask-size: 100%;
                    transition: transform 0.35s ease;
                    background-color: rgb(var(--color-icon-default));
                }
                
                .skip-btn {
                    display: flex;
                    align-items: center;    
                    justify-content: center;
                    border: none;
                    background-color: transparent;
                    cursor: pointer;
                    position: absolute;
                    right: 10px;
                    top:70px;
                }
                
                .skip-btn .icon-close {
                    width: 24px;
                    height: 24px;
                    mask-image: url("data:image/svg+xml;utf8,%3Csvg%20width%3D%2224%22%20height%3D%2224%22%20viewBox%3D%220%200%2024%2024%22%20fill%3D%22none%22%20xmlns%3D%22http%3A%2F%2Fwww.w3.org%2F2000%2Fsvg%22%3E%0A%3Cpath%20d%3D%22M18%205.91003L6%2018.09%22%20stroke%3D%22black%22%20stroke-width%3D%221.2%22%20stroke-linecap%3D%22round%22%20stroke-linejoin%3D%22round%22%2F%3E%0A%3Cpath%20d%3D%22M6%205.91003L18%2018.09%22%20stroke%3D%22black%22%20stroke-width%3D%221.2%22%20stroke-linecap%3D%22round%22%20stroke-linejoin%3D%22round%22%2F%3E%0A%3C%2Fsvg%3E%0A");
                }
        
                .skip-btn:hover .icon-close {
                    background-color: rgb(var(--color-icon-hover));
                }
                
                .policy-age {
                    margin: 1em 0;
                    font-size: 1.5em;
                    font-weight: bold;
                }
                
                .checkbox-wrapper-40 {
                    --borderColor: var(--primary-60);
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
                        border-radius: var(--global-radius);
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
                
                ${theme === "business" ? `
                .bg .header{
                    background-color: var(--primary-60);
                }
                .bg .icon-logo{
                    background-image: url("data:image/svg+xml;utf8,%3Csvg%20width%3D%22199%22%20height%3D%2248%22%20viewBox%3D%220%200%20199%2048%22%20fill%3D%22none%22%20xmlns%3D%22http%3A%2F%2Fwww.w3.org%2F2000%2Fsvg%22%3E%0A%3Cpath%20d%3D%22M33.096%2017H21.14C18.368%2017%2017.136%2018.736%2016.856%2020.388V17H10.22C9.29608%2017%208.84808%2017.28%208.40008%2017.924L6.58008%2020.584H33.096V17Z%22%20fill%3D%22%232D2926%22%2F%3E%0A%3Cpath%20d%3D%22M0%2031.0002H4.76L10.724%2021.3682L6.468%2020.9482L0%2031.0002Z%22%20fill%3D%22%232D2926%22%2F%3E%0A%3Cpath%20d%3D%22M37.7169%2017H33.7969V20.584H37.7169V17Z%22%20fill%3D%22%232D2926%22%2F%3E%0A%3Cpath%20d%3D%22M62.6084%2022.6001L50.3444%2021.8721C50.3444%2023.7481%2051.5764%2025.0081%2053.6764%2025.1761L62.4124%2025.8481C63.0844%2025.9041%2063.5044%2026.0721%2063.5044%2026.6321C63.5044%2027.1641%2063.0284%2027.3601%2062.0484%2027.3601H50.1484V31.0001H62.3004C66.1364%2031.0001%2067.8164%2029.6561%2067.8164%2026.6041C67.8164%2023.8601%2066.3044%2022.8241%2062.6084%2022.6001Z%22%20fill%3D%22%232D2926%22%2F%3E%0A%3Cpath%20d%3D%22M50.1749%2021.8716L46.3109%2021.6476V25.7636C46.3109%2027.0516%2045.6669%2027.4436%2043.4269%2027.4436H40.3469C38.4989%2027.4436%2037.7149%2026.9116%2037.7149%2025.7636V21.2556L33.7949%2020.9756V26.0996H33.7109C33.4869%2024.6156%2032.9269%2022.8796%2029.2029%2022.6276L16.9389%2021.8156C16.9389%2023.6916%2018.2829%2024.8116%2020.3829%2025.0356L28.9229%2025.9036C29.5949%2025.9596%2030.0989%2026.1556%2030.0989%2026.7156C30.0989%2027.3316%2029.5949%2027.3876%2028.8109%2027.3876H16.7989V21.7876L12.8789%2021.5356V30.9996H28.8389C32.3949%2030.9996%2033.5429%2029.1516%2033.7109%2027.6116H33.7949C34.2429%2030.0476%2036.3709%2030.9996%2039.9829%2030.9996H43.9589C48.1869%2030.9996%2050.1749%2029.7116%2050.1749%2026.6876V21.8716Z%22%20fill%3D%22%232D2926%22%2F%3E%0A%3Cpath%20d%3D%22M67.5365%2020.584V17H54.3765C51.6325%2017%2050.4005%2018.708%2050.1765%2020.332C50.1765%2020.36%2050.1765%2020.36%2050.1765%2020.388V17H46.3125V20.584H50.1765H67.5365Z%22%20fill%3D%22%232D2926%22%2F%3E%0A%3Cpath%20d%3D%22M77.1884%2016.9989L75.8164%2017.0015L75.8427%2031.0015L77.2147%2030.9989L77.1884%2016.9989Z%22%20fill%3D%22%232D2926%22%2F%3E%0A%3Cpath%20d%3D%22M85.2427%2017.4055C86.0547%2017.2375%2087.6787%2017.1255%2089.2187%2017.1255C91.0947%2017.1255%2092.2427%2017.2935%2093.2227%2017.8815C94.1747%2018.3855%2094.8468%2019.3095%2094.8468%2020.5415C94.8468%2021.7455%2094.1468%2022.8935%2092.6068%2023.4535V23.4815C94.1468%2023.9015%2095.2948%2025.0775%2095.2948%2026.8415C95.2948%2028.0735%2094.7348%2029.0255%2093.8948%2029.7255C92.9148%2030.5095%2091.2628%2030.9575%2088.5468%2030.9575C87.0348%2030.9575%2085.9148%2030.8455%2085.2148%2030.7615V17.4055H85.2427ZM88.2948%2022.5855H89.3027C90.9267%2022.5855%2091.7948%2021.9135%2091.7948%2020.9055C91.7948%2019.8695%2091.0107%2019.3375%2089.6107%2019.3375C88.9387%2019.3375%2088.5748%2019.3655%2088.2948%2019.4215V22.5855ZM88.2948%2028.6335C88.6028%2028.6615%2088.9667%2028.6615%2089.4707%2028.6615C90.8707%2028.6615%2092.1027%2028.1295%2092.1027%2026.7015C92.1027%2025.3295%2090.8707%2024.7975%2089.3307%2024.7975H88.2948V28.6335Z%22%20fill%3D%22%232D2926%22%2F%3E%0A%3Cpath%20d%3D%22M100.111%2017.2104V25.0224C100.111%2027.3744%20101.007%2028.5504%20102.575%2028.5504C104.199%2028.5504%20105.067%2027.4304%20105.067%2025.0224V17.2104H108.119V24.8264C108.119%2029.0264%20105.991%2031.0144%20102.463%2031.0144C99.0473%2031.0144%2097.0312%2029.1104%2097.0312%2024.7704V17.2104H100.111Z%22%20fill%3D%22%232D2926%22%2F%3E%0A%3Cpath%20d%3D%22M110.415%2027.6258C111.255%2028.0458%20112.515%2028.4658%20113.831%2028.4658C115.231%2028.4658%20115.987%2027.8778%20115.987%2026.9818C115.987%2026.1418%20115.343%2025.6378%20113.719%2025.0778C111.451%2024.2938%20109.995%2023.0338%20109.995%2021.0738C109.995%2018.7498%20111.927%2016.9858%20115.147%2016.9858C116.687%2016.9858%20117.807%2017.3218%20118.619%2017.6578L117.947%2020.1498C117.415%2019.8978%20116.435%2019.5058%20115.091%2019.5058C113.747%2019.5058%20113.103%2020.1218%20113.103%2020.8218C113.103%2021.6898%20113.859%2022.0818%20115.623%2022.7258C118.031%2023.6218%20119.151%2024.8538%20119.151%2026.7858C119.151%2029.0538%20117.387%2030.9858%20113.663%2030.9858C112.123%2030.9858%20110.583%2030.5938%20109.799%2030.1458L110.415%2027.6258Z%22%20fill%3D%22%232D2926%22%2F%3E%0A%3Cpath%20d%3D%22M124.021%2017.2104V30.7905H120.941V17.2104H124.021Z%22%20fill%3D%22%232D2926%22%2F%3E%0A%3Cpath%20d%3D%22M126.486%2030.7904V17.2104H130.07L132.898%2022.1944C133.71%2023.6224%20134.522%2025.3304%20135.11%2026.8424H135.166C134.97%2025.0504%20134.914%2023.2024%20134.914%2021.1864V17.2104H137.742V30.7904H134.522L131.61%2025.5544C130.798%2024.0984%20129.93%2022.3624%20129.258%2020.7664L129.202%2020.7944C129.286%2022.5864%20129.314%2024.4904%20129.314%2026.7304V30.8184H126.486V30.7904Z%22%20fill%3D%22%232D2926%22%2F%3E%0A%3Cpath%20d%3D%22M148.3%2025.0505H143.288V28.2705H148.86V30.7905H140.18V17.2104H148.58V19.7305H143.288V22.5585H148.3V25.0505Z%22%20fill%3D%22%232D2926%22%2F%3E%0A%3Cpath%20d%3D%22M150.792%2027.6258C151.632%2028.0458%20152.892%2028.4658%20154.208%2028.4658C155.608%2028.4658%20156.364%2027.8778%20156.364%2026.9818C156.364%2026.1418%20155.72%2025.6378%20154.096%2025.0778C151.828%2024.2938%20150.372%2023.0338%20150.372%2021.0738C150.372%2018.7498%20152.304%2016.9858%20155.524%2016.9858C157.064%2016.9858%20158.184%2017.3218%20158.996%2017.6578L158.324%2020.1498C157.792%2019.8978%20156.812%2019.5058%20155.468%2019.5058C154.124%2019.5058%20153.48%2020.1218%20153.48%2020.8218C153.48%2021.6898%20154.236%2022.0818%20156%2022.7258C158.408%2023.6218%20159.528%2024.8538%20159.528%2026.7858C159.528%2029.0538%20157.764%2030.9858%20154.04%2030.9858C152.5%2030.9858%20150.96%2030.5938%20150.176%2030.1458L150.792%2027.6258Z%22%20fill%3D%22%232D2926%22%2F%3E%0A%3Cpath%20d%3D%22M161.124%2027.6258C161.964%2028.0458%20163.224%2028.4658%20164.54%2028.4658C165.94%2028.4658%20166.696%2027.8778%20166.696%2026.9818C166.696%2026.1418%20166.052%2025.6378%20164.428%2025.0778C162.16%2024.2938%20160.704%2023.0338%20160.704%2021.0738C160.704%2018.7498%20162.636%2016.9858%20165.856%2016.9858C167.396%2016.9858%20168.516%2017.3218%20169.328%2017.6578L168.656%2020.1498C168.124%2019.8978%20167.144%2019.5058%20165.8%2019.5058C164.456%2019.5058%20163.812%2020.1218%20163.812%2020.8218C163.812%2021.6898%20164.568%2022.0818%20166.332%2022.7258C168.74%2023.6218%20169.86%2024.8538%20169.86%2026.7858C169.86%2029.0538%20168.096%2030.9858%20164.372%2030.9858C162.832%2030.9858%20161.292%2030.5938%20160.508%2030.1458L161.124%2027.6258Z%22%20fill%3D%22%232D2926%22%2F%3E%0A%3Cpath%20d%3D%22M190.123%2038H183.991H177.859L186.679%2024L177.859%2010H183.991H190.123L198.943%2024L190.123%2038Z%22%20fill%3D%22%231984D6%22%2F%3E%0A%3Cpath%20d%3D%22M186.68%2024.0002H198.944L192.812%2014.2842L186.68%2024.0002Z%22%20fill%3D%22%231672BD%22%2F%3E%0A%3Cpath%20d%3D%22M192.812%2033.716L198.944%2024H186.68L192.812%2033.716Z%22%20fill%3D%22%2312568C%22%2F%3E%0A%3C%2Fsvg%3E%0A");
                    background-repeat: no-repeat;
                    background-color: transparent;
                    mask-image: none;
                    -webkit-mask-image: none;
                    height: 48px;
                    width: 200px;
                    min-width: 200px;
                }
                ` : ``}
                
                ${theme === "rog" ? `
                .bg .header{
                    border: none;
                    background-color: transparent;
                    border-image: linear-gradient(to right, #0A5EDB, #DB0A2C) 30;
                    border-bottom-width: 2px;
                    border-bottom-style: solid;
                }
                .bg .icon-logo{
                    background-color: #EA0029;
                    --logo-svg: url("data:image/svg+xml,%3Csvg width='45' height='24' viewBox='0 0 45 24' fill='none' xmlns='http://www.w3.org/2000/svg'%3E%3Cpath d='M14.729 19.014s.867.578 1.62.907c5.727 2.512 14.1 4.043 15.729 3.517 4.407-1.43 9.338-10.774 10.89-15.084 0 0-4.63 1.844-9.327 4.045-3.924 1.84-6.306 3.02-7.427 3.586l-.23.093.2-.077c-.505.255-.744.38-.744.38l13.385-4.056-2.459.756s-2.931 6.889-6.744 7.657c-3.816.767-10.586-1.914-10.596-1.916.56-.43 7.61-5.722 24.31-12.537.021-.007.044-.014.068-.02.55-.226 1.3-1.524 1.324-2.262 0 0-5.9.536-11.122 3.212-7.023 3.354-18.878 11.8-18.878 11.8zm16.421 2.54zM40.12 7.31a46.095 46.095 0 0 1 3.07-.983c-1.02.295-2.045.622-3.07.983zM25.982 16.075l-.012.005.012-.005zM0 10.59l.01.023-.01-.01.012.017c.127.341 1.315 3.468 2.532 5.033 1.195 1.54 6.02 2.742 6.699 2.904.051.023.083.035.083.035l-.034-.023.034.007C6.26 16.43 0 10.59 0 10.59z' fill='%23EA0029'/%3E%3Cpath d='M13.302 19.49c-.436-.833.104-1.935 3.667-4.572 3.22-2.384 14.814-12.13 25.906-14.75 0 0-5.815-.948-14.125 1.666-2.965.93-7.325 5.077-15.306 12.894-1.086.605-4.983-1.633-7.22-2.71 0 0 3.722 5.887 5.023 7.65 2.023 2.744 5.66 4.33 5.66 4.33s-.012-.01-.033-.032c-.301-.325-2.786-2.98-3.572-4.476z' fill='%23EA0029'/%3E%3C/svg%3E");
                    height: 30px;
                    width: 40px;
                    min-width: 30px;
                }
                ` : ``}
                
                ${theme === "asus" || theme === "rt" ? `
                .bg .header{
                    background-color: var(--primary-60);
                }
                .bg .icon-logo{
                    background-color: #FFF;
                    --logo-svg: url("data:image/svg+xml,%3Csvg width='103' height='20' viewBox='0 0 103 20' fill='none' xmlns='http://www.w3.org/2000/svg'%3E%3Cpath fill-rule='evenodd' clip-rule='evenodd' d='M71.292 20h19.235c4.825-.275 5.507-4.803 5.507-4.803.17-1.191.03-2.16.03-2.16-.139-1.072-1.68-4.292-4.97-4.604-1.95-.18-19.488-1.205-19.488-1.205.519 1.785 1.217 2.325 1.744 2.819 1.213 1.156 3.032 1.433 3.032 1.433L89.96 12.6c.423.023 1.266.066 1.266 1.078 0 .306-.157.622-.352.795 0 0-.463.252-.894.252H71.305L71.292 20zm-53.089-.045h25.141c4.38-.869 4.837-4.803 4.837-4.803.207-1.168.085-2.114.085-2.114-.131-.788-1.645-4.268-4.931-4.57-1.96-.182-19.4-1.552-19.4-1.552.337 1.73 1.123 2.603 1.646 3.101 1.21 1.151 3.136 1.477 3.136 1.477.466.043 13.442 1.107 13.442 1.107.414.022 1.197.125 1.188 1.14 0 .122-.114 1.016-1.11 1.016H23.43V6.872l-5.227-.378v13.46zm53.05-12.74l-5.28-.313v6.128s-.016 1.768-1.74 1.768h-9.076s-1.537-.127-1.537-1.75V6.071l-5.333-.384v9.4c.863 4.509 4.975 4.855 4.975 4.855s.42.028.488.034h12.257s5.246-.418 5.246-5.4V7.213zm-5.37-1.908h5.37V.025h-5.37v5.282zm-17.648 0h5.375V.025h-5.376v5.282zm47.947.005V0H76.945c-2.437.152-3.513 1.305-4.316 2.138-.856.882-1.337 2.742-1.337 2.742v.427l24.89.005zm-48.886-.005V0h-17.98C26.871.152 25.802 1.305 25 2.138c-.854.882-1.34 2.742-1.34 2.742V.035h-9.545c-.72 0-1.407.302-1.925 1.011-.54.703-2.725 4.261-2.725 4.261h37.831zM9.251 5.63L0 19.952h6.323l8.892-13.693-5.964-.63zM99.362 2.53v-.834h.761c.11 0 .2.018.265.05.108.063.177.181.177.354 0 .166-.045.278-.133.343a.583.583 0 0 1-.343.087h-.727zm-.314 1.326h.314V2.81h.714c.123 0 .212.009.279.041.104.061.16.163.165.326l.029.41.002.19a.28.28 0 0 1 .031.079h.398v-.058c-.052-.018-.075-.068-.104-.147-.017-.044-.017-.113-.019-.206l-.014-.327a.583.583 0 0 0-.071-.308.522.522 0 0 0-.221-.155.705.705 0 0 0 .254-.224.714.714 0 0 0 .086-.356c0-.279-.108-.473-.328-.579a1.051 1.051 0 0 0-.44-.08h-1.075v2.44zM97.636 2.53c0 1.24 1.011 2.25 2.255 2.25.6 0 1.155-.233 1.585-.657a2.23 2.23 0 0 0 .656-1.593A2.24 2.24 0 0 0 99.892.28a2.256 2.256 0 0 0-2.256 2.251zm.175 0A2.08 2.08 0 0 1 99.89.458c.55 0 1.065.22 1.456.606.39.4.6.918.6 1.466 0 .55-.21 1.077-.6 1.46a2.037 2.037 0 0 1-1.456.613c-1.148 0-2.08-.93-2.08-2.073z' fill='%23fff'/%3E%3C/svg%3E");
                    height: 20px;
                    width: 120px;
                    min-width: 120px;
                }
                ` : ``}
                
                ${theme === "tuf" ? `
                .bg .header{
                    border: none;
                    border-bottom: 2px solid var(--primary-60);
                    background-color: transparent;
                }
                .bg .icon-logo{
                    background-color: #8B8A8B;
                    --logo-svg: url("data:image/svg+xml,%3Csvg width='42' height='34' viewBox='0 0 42 34' fill='none' xmlns='http://www.w3.org/2000/svg'%3E%3Cpath d='M19.716 11.647L10.937 6.57v22.107l8.78 5.056V11.647zM8.822 5.365L0 .266v16.967l8.822 5.077V5.365zM41.74 7.819V.266l-8.823 5.099v7.552l8.822-5.098zM41.74 9.68l-8.823 5.098v7.531l8.822-5.077V9.68zM22.023 11.648v22.086l8.758-5.056V6.592l-8.758 5.056z' fill='%238B8A8B'/%3E%3C/svg%3E");
                    height: 30px;
                    width: 40px;
                    min-width: 30px;
                }
                ` : ``}
                
                .bg .btn.disabled:hover{
                    box-shadow: none;
                }
                .bg .checkbox-wrapper-40 {
                    --borderColor: var(--primary-60);
                }
                .bg .toolbar-btn{
                    color: var(--body-text-color);
                }
                .bg .toolbar-btn .lang-icon{
                    background-color: var(--body-text-color);
                }
                
                @media screen and (min-width: 576px){
                    .bg .btn:hover{
                        box-shadow: var(--primary-btn-hover-boxshadow);
                    }
                    .bg .btn::after{
                        content: "";
                        height: 100%;
                        min-width: 200px;
                        max-width: 300px;
                        width: 25%;
                        position: absolute;
                        z-index: 3;
                        --borderColor: #FF3535;
                        ${theme !== "asus" && theme !== "rt" ? `
                        background: linear-gradient(to left, var(--borderColor), var(--borderColor)) left top no-repeat, linear-gradient(to bottom, var(--borderColor), var(--borderColor)) left top no-repeat, linear-gradient(to left, var(--borderColor), var(--borderColor)) right top no-repeat, linear-gradient(to bottom, var(--borderColor), var(--borderColor)) right top no-repeat, linear-gradient(to left, var(--borderColor), var(--borderColor)) left bottom no-repeat, linear-gradient(to bottom, var(--borderColor), var(--borderColor)) left bottom no-repeat, linear-gradient(to left, var(--borderColor), var(--borderColor)) right bottom no-repeat, linear-gradient(to left, var(--borderColor), var(--borderColor)) right bottom no-repeat;
                        background-size: 1px 4px, 4px 1px, 1px 4px, 4px 1px;
                        `:``}
                    }           
                    
                    .btn.disabled.agree::after {
                        --borderColor: var(--primary-btn-disable-fill);
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
                    color: var(--primary-60);
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
                 .sr-only {
                    position: absolute;
                    width: 1px;
                    height: 1px;
                    padding: 0;
                    margin: -1px;
                    overflow: hidden;
                    clip: rect(0, 0, 0, 0);
                    white-space: nowrap;
                    border: 0;
                }
            </style>
            <div class="bg ${theme}">
                <div class="header">
                    <div class="header-title"><div class="icon-logo"></div><div class="header-name">${ASUS_POLICY.Dict.ModalName}</div></div>
                    <div class="header-dropdown">
                        <button id="selectLang" class="toolbar-btn" aria-expanded="false" aria-haspopup="listbox" aria-labelledby="lang-label" aria-controls="language-listbox">
                            <span id="lang-label" class="sr-only">Select Language</span>
                            <div class="d-flex align-items-center gap-1">
                                <i class="lang-icon"></i>
                                <span class="line-height-unset">${ASUS_POLICY.Dict.SelectLanguage}</span>
                            </div>
                        </button>
                        <div class="dropdown-menu" tabindex="-1" role="listbox" aria-labelledby="lang-label">
                            <ul role="group"></ul>
                            <ul role="group"></ul>
                        </div>
                    </div>
                </div>
                <div class="container">
                    <div id="skip-btn-div"></div>
                    <div class="page">
                        <div class="page-title" role="heading" aria-level="1" aria-label="${title_orig}">${title}</div>
                        <div>
                            <div class="page-content">
                                <div class="page-desc">${ASUS_POLICY.Dict.QisDesc}</div>
                                <div class="policy-content"></div>
                                ${policy == 'EULA' ? `<div class="scroll-info">${ASUS_POLICY.Dict.ScrollDown}</div>` : ``}
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
            if (policyStatus.EULA_allow_skip == 1) {
                const yearChecked = shadowRoot.querySelector("#ASUS_EULA_enable").checked;

                if (!yearChecked) {
                    alert(`${ASUS_POLICY.Dict.AgeConfirm}`);
                    shadowRoot.querySelector(".checkbox-wrapper-40").style.color = top.businessWrapper ? "red" : "#ff5722";
                    return false;
                }
            }

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

                    if (policyStatus.EULA_allow_skip == 1) {
                        const yearChecked = shadowRoot.querySelector("#ASUS_EULA_enable").checked;

                        if (!yearChecked) {
                            alert(`${ASUS_POLICY.Dict.AgeConfirm}`);
                            shadowRoot.querySelector(".checkbox-wrapper-40").style.color = top.businessWrapper ? "red" : "#ff5722";
                            return false;
                        }
                    }

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

                    if (policyStatus.EULA_allow_skip == 0) {
                        const yearChecked = shadowRoot.querySelector("#ASUS_EULA_enable").checked;

                        if (!yearChecked) {
                            alert(`${ASUS_POLICY.Dict.AgeConfirm}`);
                            shadowRoot.querySelector(".checkbox-wrapper-40").style.color = top.businessWrapper ? "red" : "#ff5722";
                            return false;
                        }
                    }
                    applyBtn.innerHTML = Get_Component_btnLoading();
                    httpApi.newEula.set("1", function () {
                        goTo.PP();
                    })
                }
            }
        }

        const handleClickSkipBtn = (e) => {
            if (policy === "EULA") {
                httpApi.newEula.set("0", function () {
                    goTo.PP();
                })
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


        if (policy === "EULA" && policyStatus.EULA_allow_skip === 1) {
            let skipBtn = document.createElement('button');
            skipBtn.type = "button";
            skipBtn.className = "skip-btn";
            skipBtn.setAttribute("aria-label", ASUS_POLICY.Dict.Skip);
            skipBtn.innerHTML = `<i role="icon" class="icon-close"></i>`;
            shadowRoot.querySelector('#skip-btn-div').appendChild(skipBtn);
            skipBtn.addEventListener('click', handleClickSkipBtn);
        }


        if (policy !== "EULA") {
            abortBtn = document.createElement('div');
            abortBtn.className = "btn disagree";
            abortBtn.textContent = ASUS_POLICY.Dict.Disagree;
            shadowRoot.querySelector('.page-footer').appendChild(abortBtn);
            abortBtn.addEventListener("click", handleClickAbortBtn);
        }

        applyBtn = document.createElement('div');
        applyBtn.className = (policy === "EULA") ? "btn agree disabled" : "btn disagree";
        if(policy === "EULA"){
            applyBtn.textContent = ASUS_POLICY.Dict.I_Understand;
        }else {
            applyBtn.textContent = ASUS_POLICY.Dict.Agree;
        }
        shadowRoot.querySelector('.page-footer').appendChild(applyBtn);
        applyBtn.addEventListener('click', handleClickApplyBtn);

        const scrollDiv = new PolicyScrollDiv({
            policy: policy, theme: theme, scrollCallBack: checkScrollHeight
        });
        this.element.shadowRoot.querySelector('.policy-content').appendChild(scrollDiv.render());

        if ((policy === "EULA" && policyStatus.EULA_allow_skip === 0) || (policy === "PP" && policyStatus.EULA_allow_skip === 1)) {
            const ageCheckboxDiv = document.createElement('div');
            ageCheckboxDiv.className = "checkbox-wrapper-40";
            ageCheckboxDiv.innerHTML = `
                <label>
                    <input id="ASUS_EULA_enable" type="checkbox"/>
                    <span class="checkbox">${ASUS_POLICY.Dict.AgeCheck}</span>
                </label>`;
            this.element.shadowRoot.querySelector('.policy-age').appendChild(ageCheckboxDiv);
        }

        const languageDropdown = new AccessibleLanguageDropdown(
            this.element.shadowRoot.querySelector("#selectLang"),
            this.element.shadowRoot.querySelector(".dropdown-menu"),
            this.policyStatus.Policy_lang, httpApi);
    }

    getTheme() {
        let ui_support = httpApi.hookGet("get_ui_support");

        function isSupport(_ptn) {
            return ui_support[_ptn] ? true : false;
        }

        let theme = 'rt';
        if (isSupport("UI4")) {
            theme = 'asus'
        }
        if (isSupport("rog")) {
            return "rog";
        } else if (isSupport("tuf")) {
            return "tuf";
        } else if (isSupport("business")) {
            return "business";
        } else if (isSupport("TS_UI")) {
            return "ts";
        } else {
            return theme;
        }
    }

    render() {
        return this.element;
    }
}

class FeedbackNoticeModalComponent {
    constructor(props) {
        const {
            id = 'feedback_notice_popup_modal',
            policyStatus = policy_status,
            theme = this.getTheme(),
        } = props;
        this.id = id;
        const div = document.createElement('div');
        div.id = id;
        const shadowRoot = div.attachShadow({mode: 'open'});
        const template = document.createElement('template');
        let color = "dark";
        if (isSupport("UI4") || theme == "business") {
            color = "light";
        }
        template.innerHTML = `
          ${ASUS_POLICY.PolicyModalStyle}
          <div class="popup_bg" data-asuswrt-color="${color}" data-asuswrt-theme="${theme}">
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
        closeBtn.innerHTML = ASUS_POLICY.Dict.I_Understand;
        closeBtn.addEventListener('click', this.handleClickClose.bind(this));
        this.closeBtn = closeBtn;

        this.element.shadowRoot.querySelector('div.modal-title').innerHTML = ASUS_POLICY.Content.PP.PersonalData;
        this.element.shadowRoot.querySelector('div.policy-scroll-div').innerHTML = ASUS_POLICY.Content.PP.FeedbackNoticeHTML;
        this.element.shadowRoot.querySelector('div.policy-scroll-div').querySelector('.url-policy')?.setAttribute('href', `https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Policy&lang=${policyStatus.Policy_lang}&kw=&num=`);
        this.element.shadowRoot.querySelector('div.policy-scroll-div').querySelector('.url-personal-asus-privacy')?.setAttribute('href', `https://nw-dlcdnet.asus.com/support/forward.html?model=&type=AsusPrivacy&lang=${policyStatus.Policy_lang}&kw=&num=`);

        this.element.shadowRoot.querySelector('div.modal-footer').appendChild(closeBtn);


    }

    policy = "";
    closeBtn = null;

    getTheme() {
        let ui_support = httpApi.hookGet("get_ui_support");

        function isSupport(_ptn) {
            return ui_support[_ptn] ? true : false;
        }

        let theme = 'rt';
        if (isSupport("UI4")) {
            theme = 'asus'
        }
        if (isSupport("rog")) {
            return "rog";
        } else if (isSupport("tuf")) {
            return "tuf";
        } else if (isSupport("business")) {
            return "business";
        } else if (isSupport("TS_UI")) {
            return "ts";
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

function showPersonalData() {
    const PersonalDataModal = new PersonalDataModalComponent({});
    PersonalDataModal.show();
}

function showFeedbackNotice() {
    const FeedbackNoticeModal = new FeedbackNoticeModalComponent({});
    FeedbackNoticeModal.show();
}


if (typeof module !== 'undefined') {
    module.exports = {
        PolicyModalComponent, PolicyWithdrawModalComponent, ThirdPartyPolicyModalComponent
    };
}
