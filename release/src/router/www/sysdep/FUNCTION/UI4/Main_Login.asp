<!DOCTYPE html>
<html data-asuswrt-theme="rog">
<head>
    <meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
    <meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
    <meta HTTP-EQUIV="Expires" CONTENT="-1">
    <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
    <link rel="icon" href="images/favicon.png">
    <link type="text/css" rel="stylesheet" href="css/color-table.css">
    <script type="text/javascript" src="/js/jquery.js"></script>
    <script type="text/javascript" src="/js/https_redirect/https_redirect.js"></script>
    <script type="text/javascript" src="/js/qrcode/jquery.qrcode.min.js"></script>
    <title>ASUS Login</title>
    <style>
        /* Reset and Base Styles */
        html, body {
            height: 100%;
            margin: 0;
        }

        .background-container {
            background: url(images/New_ui/login_ui4_bg.png) no-repeat center;
            inset: 0;
            z-index: -1;
            overflow: hidden;
            pointer-events: none;
            position: absolute;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -59%) scale(1);
            width: 200%;
            height: 200%;
        }


        canvas {
            display: block;
            width: 100%;
            height: 100%;
        }

        html::-webkit-scrollbar {
            display: block;
            width: 4px;
            height: 4px;
            padding: 2px;
        }

        html::-webkit-scrollbar-thumb {
            background-color: #248DFF !important;
            border-radius: 50px;
        }

        html::-webkit-scrollbar-track {
            background-color: #CCC !important;
        }

        body {
            font-family: var(--global-font-style);
            display: flex;
            justify-content: center;
            align-items: center;
            overflow: hidden;
            background-color: #f5f5f5;
        }

        .header_login {
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: space-around;
        }

        .login-filed {
            width: inherit;
            display: flex;
            flex-direction: column;
            align-items: center;
            margin-top: 16px;
        }

        .login-content {
            width: 80%;
            display: flex;
            flex-direction: column;
            align-items: center;
            margin-top: 16px;
            gap: 16px
        }

        @media (max-width: 768px) {
            .login-content {
                width: 100%;
            }
        }

        .title-name {
            font-size: 28pt;
            color: #181818;
            font-weight: 300;
            display: flex;
            flex-direction: column;
            align-items: flex-start;
            justify-content: space-between;
        }

        .prod_madelName {
            font-size: 14pt;
            color: #181818;
            /* margin-left:78px;
            margin-top: 10px; */
        }

        .login-img {
            width: 160px;
            height: 100px;
            mask-image: url(data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0NSIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDQ1IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJHcm91cF8yIj48cGF0aCBpZD0iVmVjdG9yX19fX18wXzBfV0ZUR01PVUxVSSIgZmlsbD0iI0NDMDAwRSIgZD0iTTE0LjcyODQgMTkuMDEzMkMxNC43Mjg0IDE5LjAxMzIgMTUuNTk1OSAxOS41OTA3IDE2LjM0OTcgMTkuOTIwMUMyMi4wNzY0IDIyLjQzMjEgMzAuNDQ5NSAyMy45NjI5IDMyLjA3NzggMjMuNDM2NEMzNi40ODQ3IDIyLjAwNzYgNDEuNDE1OCAxMi42NjI2IDQyLjk2NzUgOC4zNTMwOEM0Mi45Njc1IDguMzUzMDggMzguMzM3OSAxMC4xOTcgMzMuNjQxMSAxMi4zOTgyQzI5LjcxNjYgMTQuMjM3NSAyNy4zMzQ1IDE1LjQxODEgMjYuMjE0MiAxNS45ODRDMjYuMDkxMyAxNi4wMzI3IDI2LjAxNzEgMTYuMDYyOSAyNS45ODQ2IDE2LjA3NjhMMjYuMTg0MSAxNi4wMDAzQzI1LjY3ODQgMTYuMjU1NCAyNS40Mzk1IDE2LjM4MDYgMjUuNDM5NSAxNi4zODA2TDM4LjgyNSAxMi4zMjRMMzYuMzY2NCAxMy4wODAxQzM2LjM2NjQgMTMuMDgwMSAzMy40MzQ2IDE5Ljk2ODggMjkuNjIxNSAyMC43MzY1QzI1LjgwNiAyMS41MDQzIDE5LjAzNTYgMTguODIzIDE5LjAyNjMgMTguODIwN0MxOS41ODUzIDE4LjM5MTYgMjYuNjM2NCAxMy4wOTg2IDQzLjMzNjMgNi4yODQxNEM0My4zNTcyIDYuMjc3MTggNDMuMzgwNCA2LjI3MDIzIDQzLjQwMzUgNi4yNjMyN0M0My45NTMzIDYuMDM4MjggNDQuNzA0NyA0LjczOTQgNDQuNzI3OSA0LjAwMTgyQzQ0LjcyNzkgNC4wMDE4MiAzOC44MjczIDQuNTM3NjEgMzMuNjA2MyA3LjIxNDI0QzI2LjU4MyAxMC41NjgxIDE0LjcyODQgMTkuMDEzMiAxNC43Mjg0IDE5LjAxMzJaTTMxLjE1IDIxLjU1M0MzMS4wMzg2IDIxLjU5NDcgMzAuOTI3MyAyMS42MzE4IDMwLjgxNiAyMS42NjJDMzAuOTI3MyAyMS42MzE4IDMxLjAzODYgMjEuNTk0NyAzMS4xNSAyMS41NTNaTTQwLjExOTIgNy4zMDkzM0M0MS4xNDY3IDYuOTQwNTQgNDIuMTY5NiA2LjYxMzUgNDMuMTkwMiA2LjMyNTg5QzQyLjE2OTYgNi42MjA0NiA0MS4xNDQ0IDYuOTQ3NSA0MC4xMTkyIDcuMzA5MzNaIi8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8xX1VLVFJLQk1PSk8iIGZpbGw9IiNDQzAwMEUiIGQ9Ik0yNS45ODI2IDE2LjA3NDRMMjUuOTcwMiAxNi4wNzkxQzI1Ljk2ODYgMTYuMDc5MSAyNS45NzQ4IDE2LjA3NzUgMjUuOTgyNiAxNi4wNzQ0WiIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMl9JQk9JS0xXRlpCIiBmaWxsPSIjQ0MwMDBFIiBkPSJNMCAxMC41ODlDMCAxMC41ODkgMC4wMDQ2Mzg4NyAxMC41OTgzIDAuMDA5Mjc3NzQgMTAuNjEyMkMwLjAwNjk1ODMgMTAuNjA5OSAwIDEwLjYwMjkgMCAxMC42MDI5QzAgMTAuNjAyOSAwLjAwNjk1ODMgMTAuNjEyMiAwLjAxMTU5NzIgMTAuNjE5MUMwLjEzOTE2NiAxMC45NjAxIDEuMzI2NzIgMTQuMDg2NyAyLjU0NDQyIDE1LjY1MjNDMy43Mzg5MyAxNy4xOTI0IDguNTYzMzUgMTguMzkzOSA5LjI0Mjk1IDE4LjU1NjNDOS4yOTM5OCAxOC41Nzk1IDkuMzI2NDUgMTguNTkxIDkuMzI2NDUgMTguNTkxQzkuMzE0ODUgMTguNTg0MSA5LjMwMzI1IDE4LjU3NDggOS4yOTE2NiAxOC41Njc5QzkuMzEyNTMgMTguNTcyNSA5LjMyNjQ1IDE4LjU3NDggOS4zMjY0NSAxOC41NzQ4QzYuMjYwMTUgMTYuNDI5MyAwIDEwLjU4OSAwIDEwLjU4OVoiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzNfT0RCSkNTSkRZUiIgZmlsbD0iI0NDMDAwRSIgZD0iTTEzLjMwMTkgMTkuNDg4N0MxMi44NjU4IDE4LjY1NiAxMy40MDYyIDE3LjU1NDMgMTYuOTY4OSAxNC45MTcxQzIwLjE4ODMgMTIuNTMyNyAzMS43ODMxIDIuNzg2NDQgNDIuODc0NiAwLjE2Nzc5M0M0Mi44NzQ2IDAuMTY3NzkzIDM3LjA1OTggLTAuNzgwODU3IDI4Ljc0OTMgMS44MzMxNUMyNS43ODUxIDIuNzYzMjQgMjEuNDI0NSA2LjkxMDM5IDEzLjQ0MzMgMTQuNzI2OUMxMi4zNTc4IDE1LjMzMjMgOC40NjEyIDEzLjA5NCA2LjIyMjk0IDEyLjAxNzhDNi4yMjI5NCAxMi4wMTc4IDkuOTQ1NjMgMTcuOTA0NSAxMS4yNDY4IDE5LjY2NzNDMTMuMjY5NCAyMi40MTEyIDE2LjkwNjMgMjMuOTk3NyAxNi45MDYzIDIzLjk5NzdDMTYuOTA2MyAyMy45OTc3IDE2Ljg5NDcgMjMuOTg2MSAxNi44NzM4IDIzLjk2NTJDMTYuNTcyMyAyMy42NDA1IDE0LjA4ODEgMjAuOTg0NyAxMy4zMDE5IDE5LjQ4ODdaIi8+PC9nPjwvZz48L3N2Zz4=);
            /* background-image: url('images/New_ui/icon_titleName.png'); */
            mask-repeat: no-repeat;
            background-color: #dc2626;
            background-image: linear-gradient(to right, #4a4dff 0%, #dc2626, #ffa7a7, #dc2626, #4a4dff 100%);
            background-size: 400%;
            mask-size: 100%;
            animation: gradientMove 5s linear infinite;
        }

        @keyframes gradientMove {
            0% {
                background-position: 100% 0%;
            }
            100% {
                background-position: -100% 0%;
            }
        }

        .p1 {
            font-size: 12pt;
            color: #181818;
            width: 480px;
            line-height: 32pt;
            background-color: rgba(255, 255, 255, 0.4);
            /* font-weight: bolder; */
        }

        .submit-button {
            background: var(--white-alpha-60);
            color: var(--primary-60);
            padding: 12px 0px;
            font-size: 16px;
            cursor: pointer;
            width: 100%;
            display: flex;
            align-items: center;
            justify-content: center;
            position: relative;
            z-index: 1;
        }

        .submit-button.disabled {
            filter: grayscale(100%) opacity(0.5);
            pointer-events: none;
        }

        .form_input {
            background-color: rgba(255, 255, 255, 0.1);
            /* background-color:#576D73\9; */
            border-radius: 0px;
            padding: 12px 16px;
            width: 100%;
            border: none;
            border-bottom: 1px solid #ccc;
            /* height:32px; */
            color: #262626;
            font-size: 14pt;
            box-sizing: border-box;
        }

        input.form_input:focus {
            outline: none;
            box-shadow: 0px -2px 0px 0px var(--textbox-focus-stroke-color) inset;
            background-color: rgba(255, 255, 255, 0.4);
        }

        .logout_field {
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
        }

        .nologin {
            background-color: rgba(255, 255, 255, 0.4);
            padding: 16px;
            line-height: 36px;
            border-radius: 0px;
            width: 100%;
            border: 1px solid #ccc;
            color: #262626;
            color: #FFF \9; /* IE6 IE7 IE8 */
            font-size: 14pt;
            margin-top: 90px;
            margin-bottom: 90px;
        }

        .div_tr {
            width: 100%;
            display: flex;
            flex-direction: column;
        }

        .div-td {
            display: table-cell;
        }

        .title-gap {
            display: flex;
            justify-content: center;
            width: 100%;
        }

        .img-gap {
            padding-right: 16px;
            /* vertical-align:middle; */
        }

        .password_gap {
            /* margin:30px 0px 0px 78px; */
        }

        .error-hint {
            color: #EF4444;
            margin: 16px 0;
            font-size: 18px;
        }

        .error-hint1 {
            font-size: 24px;
            line-height: 32px;
            width: 580px;
        }

        .main_field_gap {
            display: flex;
            flex-direction: row;
            align-items: center;
            justify-content: center;
            gap: 16px
        }

        form.with-qrcode {
            width: 80%;
        }

        .warming_desc {
            margin: 0;
            font-size: 14px;
            color: #EF4444;
            line-height: 20px;
            width: 520px;
        }

        .captcha_field {
            display: flex;
            align-items: center;
            gap: 8px;
        }

        #captcha_img_div {
            width: 160px;
            height: 60px;
            border-radius: 4px;
            background-color: #FFF;
            display: flex;
            align-items: center;
            justify-content: center;
        }

        #captcha_pic {
            width: 90%;
            height: 90%;
        }

        #reCaptcha {
            width: 30px;
            height: 30px;
            background-image: url("data:image/svg+xml,%3Csvg%20xmlns%3D%22http%3A%2F%2Fwww.w3.org%2F2000%2Fsvg%22%20width%3D%2224%22%20height%3D%2224%22%20fill%3D%22none%22%20viewBox%3D%220%200%2024%2024%22%3E%3Cg%20id%3D%22Group%22%3E%3Cg%20id%3D%22Action%20%2F%20reflash%22%3E%3Cg%20id%3D%22Layer%202%22%3E%3Cpath%20id%3D%22Vector_____0_4_VMEPPJRXUQ%22%20stroke%3D%22black%22%20stroke-linecap%3D%22round%22%20stroke-linejoin%3D%22round%22%20stroke-width%3D%221.5%22%20d%3D%22M18.0003%209.97337C17.6362%208.2303%2016.6541%206.95472%2015.2172%205.81407C13.7803%204.67342%2011.9751%204.03328%2010.1014%204.00001C7.90879%204.04315%205.82395%204.91007%204.30516%206.4102C2.78637%207.91033%201.95793%209.92092%202.00192%2012C1.95792%2014.0791%202.78637%2016.0897%204.30516%2017.5898C5.82395%2019.09%207.90879%2019.9569%2010.1014%2020C11.9766%2019.9687%2013.784%2019.3294%2015.2228%2018.1886C16.6616%2017.0478%2017.1651%2016.2857%2017.7369%2015.1429%22%2F%3E%3Cpath%20id%3D%22Vector_____0_7_HMQCGOEGFO%22%20fill%3D%22black%22%20d%3D%22M14.6955%209.37402C14.3567%209.07129%2014.5009%208.50052%2014.9478%208.42753C15.4453%208.3328%2016.4676%208.01256%2016.9506%207.95396C17.2245%207.89571%2017.4624%207.82309%2017.6571%207.64943C18.068%207.35263%2019.0038%206.83033%2019.3787%206.51915C19.7391%206.2441%2020.287%206.54648%2020.2365%206.9871L19.4809%2011.5605C19.4665%2012.0155%2018.9114%2012.2548%2018.5726%2011.9521L14.6955%209.37402Z%22%2F%3E%3C%2Fg%3E%3C%2Fg%3E%3C%2Fg%3E%3C%2Fsvg%3E");
            background-repeat: no-repeat;
            background-position: center;
        }

        .div_app_link {
            display: flex;
            position: absolute;
            top: 10px;
            right: 15px;
            flex-direction: row;
            align-items: center;
            gap: 10px;
        }

        .div_app_link div {
            border: 1px solid #626262;
            border-radius: 50%;
            padding: 6px;
            height: 13px;
            width: 13px;
            display: flex;
            align-items: center;
            justify-content: center;
        }

        .div_app_link svg {
            font-size: 20px;
            fill: #626262;
        }

        .div_app_link a {
            font-size: 8px;
            color: #626262;
            text-decoration: underline;
        }

        .div_qr {
            display: flex;
            flex-direction: column;
            align-items: center;
            width: 100%;
        }

        .qr_content {
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
        }

        .qr_filed {
            display: flex;
            align-items: center;
            gap: 10px;
            flex-direction: column;
        }

        .qr_timer_div {
            display: flex;
            flex-direction: column;
            align-items: center;
            gap: 10px;
        }

        .qr_title {
            font-size: 16pt;
            color: #006ce1;
            line-height: 48pt;
        }

        .qr_text {
            display: flex;
            gap: 5px;
        }

        .vs {
            display: flex;
            flex-direction: column;
            align-items: center;
            height: 70vh;
            gap: 5px;
        }

        .vs_line {
            border-left: 3px solid #62626222;
            height: 100%;
        }

        .qrcodeLoginSupport {
            display: none;
        }

        .gap-1 {
            gap: .25rem !important;
        }

        .d-flex {
            display: flex;
        }

        .mb-2 {
            margin-bottom: 0.85rem;
        }

        .me-2 {
            margin-right: 0.85rem;
        }

        .flex-row {
            flex-direction: row;
        }

        .flex-column {
            flex-direction: column;
        }

        .dropdown-content {
            display: none;
            position: absolute;
            right: 15px;
            top: 35px;
            overflow-y: auto;
            padding: 16px;
            margin-top: 10px;
            border-radius: 8px;
            background: #F5F5F5;
            border: 1px solid #dcdcdc;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
            animation-name: fadeInDown;
            -webkit-animation-name: fadeInDown;
            animation-duration: 0.35s;
            animation-fill-mode: both;
            -webkit-animation-duration: 0.35s;
            -webkit-animation-fill-mode: both;
        }

        .dropdown-content.show {
            display: block;
            z-index: 10;
            animation-name: fadeInUp;
            -webkit-animation-name: fadeInUp;
            animation-duration: 0.4s;
            animation-fill-mode: both;
            -webkit-animation-duration: 0.4s;
            -webkit-animation-fill-mode: both;
        }

        .option {
            display: flex;
            width: 100%;
            justify-content: center;
            align-items: center;
            flex-direction: column;
            background-color: #fff;
            border: 1px solid #dcdcdc;
            border-radius: 8px;
            padding: 12px;
        }

        #login_qr {
            background-repeat: no-repeat;
            background-size: cover;
            height: 150px;
            width: 150px;
            margin-bottom: 0.5rem;
            background-position: center;
        }

        .qr_toggle {
            width: 48px;
            height: 48px;
            display: inline-block;
            --svg: url("data:image/svg+xml,%3Csvg width='30' height='30' viewBox='0 0 24 24' fill='none' xmlns='http://www.w3.org/2000/svg'%3E%3Cpath d='M20 18v2h-2m2-6h-1l-2 2m-1 2h-2v2M4 4h6v6H4V4zm10 0h6v6h-6V4zM4 14h6v6H4v-6zm10 0v1h1v-1h-1zM17 7h.001M7 7h.001M7 17h.001' stroke='%23000' stroke-linecap='round' stroke-linejoin='round' stroke-width='2'/%3E%3C/svg%3E");
            mask-image: var(--svg);
            background-color: #62626222;
            background-repeat: no-repeat;
            background-size: contain;
            background-position: center;
            -webkit-mask-image: var(--svg);
            -webkit-mask-repeat: no-repeat;
            -webkit-mask-position: center;
            -webkit-mask-size: contain;
            cursor: pointer;
        }

        .icon-refresh {
            width: 16px;
            height: 16px;
            display: inline-block;
            --svg: url("data:image/svg+xml,%3Csvg width='800' height='800' viewBox='0 0 24 24' fill='none' xmlns='http://www.w3.org/2000/svg'%3E%3Cpath d='M3 3v5m0 0h5M3 8l3-2.708A9 9 0 1 1 12 21a9.003 9.003 0 0 1-8.777-7' stroke='%23000' stroke-width='2' stroke-linecap='round' stroke-linejoin='round'/%3E%3C/svg%3E");
            mask-image: var(--svg);
            background-color: #000;
            background-repeat: no-repeat;
            background-size: contain;
            background-position: center;
            -webkit-mask-image: var(--svg);
            -webkit-mask-repeat: no-repeat;
            -webkit-mask-position: center;
            -webkit-mask-size: contain;
            cursor: pointer;
        }

        .android-qr-code, .ios-qr-code {
            background-repeat: no-repeat;
            background-size: cover;
            height: 115px;
            width: 115px;
            margin-bottom: 0.5rem;
            background-position: center;
        }

        .android-qr-code > canvas,
        .ios-qr-code > canvas {
            width: 100%;
            height: 100%;
        }

        .logo-google-play, .logo-apple-store {
            background-repeat: no-repeat;
            height: 35px;
            display: block;
            background-position-x: center;
            width: 100%;
            background-size: cover;
        }

        .logo-google-play {
            background-image: url("data:image/svg+xml,%3Csvg%20xmlns%3D%22http%3A%2F%2Fwww.w3.org%2F2000%2Fsvg%22%20width%3D%22108%22%20height%3D%2232%22%20fill%3D%22none%22%20viewBox%3D%220%200%20108%2032%22%3E%3Cg%20id%3D%22Group%22%3E%3Cg%20id%3D%22logo-google-play%22%3E%3Cg%20id%3D%22XMLID_37_%22%3E%3Cpath%20id%3D%22XMLID_65______0_1_GLCEXLXQKC%22%20fill%3D%22black%22%20d%3D%22M103.989%2031.9972H4C1.80086%2031.9972%200%2030.1964%200%2027.9972V4.00001C0%201.80086%201.80086%200%204%200H103.989C106.188%200%20107.989%201.80086%20107.989%204.00001V27.9972C107.989%2030.1992%20106.188%2031.9972%20103.989%2031.9972Z%22%2F%3E%3C%2Fg%3E%3Cg%20id%3D%22XMLID_92_%22%3E%3Cg%20id%3D%22XMLID_93_%22%3E%3Cpath%20id%3D%22XMLID_94______0_2_UGPICUWHUY%22%20fill%3D%22%23A6A6A6%22%20d%3D%22M103.989%200.642985C105.841%200.642985%20107.349%202.15081%20107.349%204.00288V28.0001C107.349%2029.8521%20105.841%2031.36%20103.989%2031.36H4C2.14794%2031.36%200.640115%2029.8521%200.640115%2028.0001V4.00288C0.640115%202.15081%202.14794%200.642985%204%200.642985H103.989ZM103.989%200.00287043H4C1.80086%200.00287043%200%201.80373%200%204.00288V28.0001C0%2030.1992%201.80086%2032.0001%204%2032.0001H103.989C106.188%2032.0001%20107.989%2030.1992%20107.989%2028.0001V4.00288C107.989%201.80088%20106.188%200.00287043%20103.989%200.00287043Z%22%2F%3E%3C%2Fg%3E%3C%2Fg%3E%3Cg%20id%3D%22XMLID_76_%22%3E%3Cpath%20id%3D%22XMLID_90______0_3_OGUPCYJBLJ%22%20fill%3D%22white%22%20stroke%3D%22white%22%20stroke-miterlimit%3D%2210%22%20stroke-width%3D%220.2%22%20d%3D%22M37.9315%208.19648C37.9315%208.86789%2037.7324%209.39989%2037.3341%209.79818C36.8817%2010.2704%2036.2957%2010.5094%2035.5702%2010.5094C34.8761%2010.5094%2034.2871%2010.2704%2033.8035%209.78965C33.3199%209.30885%2033.0752%208.71426%2033.0752%208.00302C33.0752%207.29178%2033.317%206.69719%2033.8035%206.21639C34.2871%205.73559%2034.8761%205.49662%2035.5702%205.49662C35.9145%205.49662%2036.2416%205.56489%2036.5546%205.6986C36.8675%205.83232%2037.1179%206.01155%2037.3056%206.23346L36.8846%206.65451C36.566%206.27613%2036.1278%206.08552%2035.5702%206.08552C35.0638%206.08552%2034.6285%206.26191%2034.2587%206.61753C33.8889%206.97315%2033.7068%207.43403%2033.7068%208.00302C33.7068%208.56917%2033.8917%209.03289%2034.2587%209.38851C34.6285%209.74413%2035.0638%209.92052%2035.5702%209.92052C36.1051%209.92052%2036.5517%209.74129%2036.9102%209.38567C37.1435%209.15238%2037.2772%208.82806%2037.3113%208.41269H35.5674V7.83517H37.8917C37.9202%207.96035%2037.9315%208.07984%2037.9315%208.19648Z%22%2F%3E%3Cpath%20id%3D%22XMLID_88______0_4_LHDTXLWYIL%22%20fill%3D%22white%22%20stroke%3D%22white%22%20stroke-miterlimit%3D%2210%22%20stroke-width%3D%220.2%22%20d%3D%22M41.6186%206.19072H39.4337V7.71277H41.4052V8.29029H39.4337V9.81234H41.6186V10.4013H38.8163V5.60181H41.6186V6.19072Z%22%2F%3E%3Cpath%20id%3D%22XMLID_86______0_5_FGBDEIRRBU%22%20fill%3D%22white%22%20stroke%3D%22white%22%20stroke-miterlimit%3D%2210%22%20stroke-width%3D%220.2%22%20d%3D%22M44.219%2010.4013H43.6016V6.19072H42.2616V5.60181H45.5589V6.19072H44.219V10.4013Z%22%2F%3E%3Cpath%20id%3D%22XMLID_84______0_6_NECLKKZULN%22%20fill%3D%22white%22%20stroke%3D%22white%22%20stroke-miterlimit%3D%2210%22%20stroke-width%3D%220.2%22%20d%3D%22M47.9459%2010.4013V5.60181H48.5632V10.4013H47.9459Z%22%2F%3E%3Cpath%20id%3D%22XMLID_82______0_7_ZXFWUPEHGT%22%20fill%3D%22white%22%20stroke%3D%22white%22%20stroke-miterlimit%3D%2210%22%20stroke-width%3D%220.2%22%20d%3D%22M51.2974%2010.4013H50.6801V6.19072H49.3401V5.60181H52.6374V6.19072H51.2974V10.4013Z%22%2F%3E%3Cpath%20id%3D%22XMLID_79______0_8_RIMYHTSJKI%22%20fill%3D%22white%22%20stroke%3D%22white%22%20stroke-miterlimit%3D%2210%22%20stroke-width%3D%220.2%22%20d%3D%22M58.882%209.78115C58.4097%2010.2676%2057.8236%2010.5095%2057.1209%2010.5095C56.4182%2010.5095%2055.8322%2010.2676%2055.3627%209.78115C54.8905%209.29751%2054.6572%208.70291%2054.6572%208.00305C54.6572%207.30319%2054.8933%206.7086%2055.3627%206.22496C55.835%205.73847%2056.4211%205.49665%2057.1209%205.49665C57.8179%205.49665%2058.404%205.74131%2058.8791%206.2278C59.3514%206.71429%2059.5904%207.30604%2059.5904%208.00305C59.5875%208.70291%2059.3514%209.29466%2058.882%209.78115ZM55.8179%209.38001C56.1736%209.73847%2056.6088%209.92055%2057.1209%209.92055C57.6359%209.92055%2058.0711%209.74132%2058.4239%209.38001C58.7795%209.02155%2058.9588%208.56066%2058.9588%208.00305C58.9588%207.44544%2058.7795%206.98456%2058.4239%206.6261C58.0683%206.26763%2057.633%206.08555%2057.1209%206.08555C56.606%206.08555%2056.1736%206.26479%2055.8179%206.6261C55.4623%206.98456%2055.2859%207.44544%2055.2859%208.00305C55.2859%208.56066%2055.4623%209.0187%2055.8179%209.38001Z%22%2F%3E%3Cpath%20id%3D%22XMLID_77______0_9_MZQSSJXHXU%22%20fill%3D%22white%22%20stroke%3D%22white%22%20stroke-miterlimit%3D%2210%22%20stroke-width%3D%220.2%22%20d%3D%22M60.4523%2010.4013V5.60181H61.2034L63.5363%209.33439H63.5619L63.5363%208.40978V5.60181H64.1536V10.4013H63.5107L61.0697%206.48659H61.0441L61.0697%207.4112V10.4013H60.4523Z%22%2F%3E%3C%2Fg%3E%3Cpath%20id%3D%22XMLID_52______0_10_ALETBLWHHC%22%20fill%3D%22white%22%20d%3D%22M54.5038%2017.3997C52.6233%2017.3997%2051.0899%2018.8307%2051.0899%2020.8023C51.0899%2022.7624%2052.6233%2024.2048%2054.5038%2024.2048C56.3843%2024.2048%2057.9178%2022.7624%2057.9178%2020.8023C57.9178%2018.8307%2056.3843%2017.3997%2054.5038%2017.3997ZM54.5038%2022.8649C53.4739%2022.8649%2052.5835%2022.0142%2052.5835%2020.8023C52.5835%2019.5789%2053.4739%2018.7397%2054.5038%2018.7397C55.5337%2018.7397%2056.4242%2019.5761%2056.4242%2020.8023C56.4242%2022.0142%2055.5337%2022.8649%2054.5038%2022.8649ZM47.0529%2017.3997C45.1724%2017.3997%2043.6389%2018.8307%2043.6389%2020.8023C43.6389%2022.7624%2045.1724%2024.2048%2047.0529%2024.2048C48.9334%2024.2048%2050.4668%2022.7624%2050.4668%2020.8023C50.4697%2018.8307%2048.9362%2017.3997%2047.0529%2017.3997ZM47.0529%2022.8649C46.023%2022.8649%2045.1325%2022.0142%2045.1325%2020.8023C45.1325%2019.5789%2046.023%2018.7397%2047.0529%2018.7397C48.0828%2018.7397%2048.9732%2019.5761%2048.9732%2020.8023C48.9732%2022.0142%2048.0856%2022.8649%2047.0529%2022.8649ZM38.1908%2018.4438V19.8862H41.6446C41.5422%2020.697%2041.2719%2021.2916%2040.8594%2021.7041C40.3559%2022.2077%2039.5706%2022.7596%2038.1908%2022.7596C36.0657%2022.7596%2034.4014%2021.0469%2034.4014%2018.9189C34.4014%2016.7909%2036.0628%2015.0782%2038.1908%2015.0782C39.3374%2015.0782%2040.1766%2015.5306%2040.794%2016.1081L41.8125%2015.0896C40.9504%2014.2646%2039.8011%2013.633%2038.1908%2013.633C35.2776%2013.633%2032.831%2016.0028%2032.831%2018.9161C32.831%2021.8293%2035.2805%2024.1991%2038.1908%2024.1991C39.7641%2024.1991%2040.9476%2023.6842%2041.8751%2022.7169C42.8281%2021.7639%2043.124%2020.4239%2043.124%2019.34C43.124%2019.0043%2043.0984%2018.697%2043.0472%2018.4381H38.1908V18.4438ZM74.4327%2019.5647C74.1482%2018.8051%2073.2862%2017.3997%2071.5195%2017.3997C69.767%2017.3997%2068.3104%2018.7795%2068.3104%2020.8023C68.3104%2022.7084%2069.7528%2024.2048%2071.6873%2024.2048C73.2464%2024.2048%2074.1482%2023.2518%2074.5238%2022.697L73.363%2021.9232C72.9761%2022.4893%2072.4469%2022.8649%2071.6873%2022.8649C70.9277%2022.8649%2070.3844%2022.5178%2070.0373%2021.835L74.5863%2019.9545L74.4327%2019.5647ZM69.7954%2020.6998C69.7556%2019.3855%2070.8139%2018.7141%2071.5735%2018.7141C72.1653%2018.7141%2072.6688%2019.0099%2072.8367%2019.4367L69.7954%2020.6998ZM66.097%2024H67.5906V14H66.097V24ZM63.6475%2018.1622H63.5963C63.2606%2017.7639%2062.6176%2017.4025%2061.804%2017.4025C60.1027%2017.4025%2058.5436%2018.8961%2058.5436%2020.8165C58.5436%2022.7226%2060.1027%2024.2048%2061.804%2024.2048C62.6148%2024.2048%2063.2606%2023.8435%2063.5963%2023.431H63.6475V23.9203C63.6475%2025.2233%2062.9505%2025.9175%2061.8296%2025.9175C60.9135%2025.9175%2060.3473%2025.2603%2060.1169%2024.7055L58.8139%2025.2461C59.1866%2026.1479%2060.1795%2027.2575%2061.8296%2027.2575C63.5821%2027.2575%2065.0643%2026.2276%2065.0643%2023.7127V17.6045H63.6475V18.1622ZM61.9348%2022.8649C60.905%2022.8649%2060.0401%2022.0028%2060.0401%2020.8165C60.0401%2019.6188%2060.905%2018.7425%2061.9348%2018.7425C62.9533%2018.7425%2063.7528%2019.6188%2063.7528%2020.8165C63.7499%2022.0028%2062.9505%2022.8649%2061.9348%2022.8649ZM81.437%2014H77.8609V24H79.3545V20.2105H81.4398C83.0956%2020.2105%2084.7201%2019.0128%2084.7201%2017.1038C84.7201%2015.1949%2083.0899%2014%2081.437%2014ZM81.474%2018.8193H79.3516V15.3912H81.474C82.5892%2015.3912%2083.2236%2016.3158%2083.2236%2017.1038C83.2236%2017.8805%2082.5892%2018.8193%2081.474%2018.8193ZM90.6973%2017.3826C89.6162%2017.3826%2088.4982%2017.8577%2088.0344%2018.9132L89.3602%2019.4651C89.6418%2018.9132%2090.171%2018.7311%2090.7229%2018.7311C91.4939%2018.7311%2092.2791%2019.1949%2092.2933%2020.0171V20.1195C92.0231%2019.9659%2091.4455%2019.7326%2090.7371%2019.7326C89.309%2019.7326%2087.8552%2020.5178%2087.8552%2021.9829C87.8552%2023.3201%2089.0245%2024.1821%2090.3389%2024.1821C91.3431%2024.1821%2091.895%2023.7326%2092.2421%2023.2034H92.2933V23.9744H93.7329V20.1394C93.7357%2018.3727%2092.41%2017.3826%2090.6973%2017.3826ZM90.5181%2022.862C90.0288%2022.862%2089.346%2022.6174%2089.346%2022.0142C89.346%2021.2432%2090.1938%2020.9474%2090.9278%2020.9474C91.5849%2020.9474%2091.8922%2021.0896%2092.2905%2021.2831C92.1795%2022.2077%2091.3801%2022.862%2090.5181%2022.862ZM98.9847%2017.6017L97.2749%2021.9374H97.2236L95.4484%2017.6017H93.841L96.5039%2023.6614L94.9847%2027.0327H96.5409L100.646%2017.6017H98.9847ZM85.5394%2024H87.033V14H85.5394V24Z%22%2F%3E%3Cg%20id%3D%22XMLID_39_%22%3E%3Cpath%20id%3D%22XMLID_8______0_11_FEFIOEVMPN%22%20fill%3D%22url(%23paint0_linear_1463_161490)%22%20d%3D%22M8.34693%206.03142C8.11364%206.27893%207.97709%206.66015%207.97709%207.15517V24.8451C7.97709%2025.3401%208.11364%2025.7213%208.34693%2025.9689L8.40667%2026.0258L18.3156%2016.1168V16.0001V15.8835L8.40667%205.97451L8.34693%206.03142Z%22%2F%3E%3Cpath%20id%3D%22XMLID_7______0_12_XDHQTZCJCT%22%20fill%3D%22url(%23paint1_linear_1463_161490)%22%20d%3D%22M21.6215%2019.4223L18.3185%2016.1164V15.9998V15.8832L21.6215%2012.5802L21.6954%2012.6228L25.6101%2014.8476C26.7281%2015.482%2026.7281%2016.5233%2025.6101%2017.1577L21.6954%2019.3825L21.6215%2019.4223Z%22%2F%3E%3Cpath%20id%3D%22XMLID_6______0_13_QHXUSQJOJM%22%20fill%3D%22url(%23paint2_linear_1463_161490)%22%20d%3D%22M21.6954%2019.3797L18.3184%2015.9999L8.34972%2025.9686C8.71956%2026.3584%209.32554%2026.4067%2010.0112%2026.017L21.6954%2019.3797Z%22%2F%3E%3Cpath%20id%3D%22XMLID_5______0_14_WBUCYPMWFJ%22%20fill%3D%22url(%23paint3_linear_1463_161490)%22%20d%3D%22M21.6954%2012.6232L10.0112%205.98308C9.32554%205.59333%208.71672%205.64169%208.34972%206.03145L18.3184%2016.0002L21.6954%2012.6232Z%22%2F%3E%3Cg%20id%3D%22XMLID_42_%22%3E%3Cpath%20id%3D%22XMLID_4______0_15_JOMLEQFORV%22%20fill%3D%22black%22%20d%3D%22M21.6213%2019.3059L10.0111%2025.9034C9.36242%2026.2732%208.78205%2026.2476%208.40936%2025.9119L8.34962%2025.9716L8.40936%2026.0285C8.78205%2026.3643%209.36242%2026.3899%2010.0111%2026.02L21.6953%2019.3799L21.6213%2019.3059Z%22%20opacity%3D%220.2%22%2F%3E%3Cpath%20id%3D%22XMLID_3______0_16_GXJFQFZFOT%22%20fill%3D%22black%22%20d%3D%22M8.34683%2025.8522C8.11354%2025.6047%207.97698%2025.2234%207.97698%2024.7284V24.8451C7.97698%2025.3401%208.11354%2025.7213%208.34683%2025.9688L8.40657%2025.9091L8.34683%2025.8522Z%22%20opacity%3D%220.12%22%2F%3E%3C%2Fg%3E%3Cpath%20id%3D%22XMLID_2______0_17_QCFXGUADMP%22%20fill%3D%22black%22%20d%3D%22M25.6102%2017.0383L21.6216%2019.3057L21.6955%2019.3797L25.6102%2017.1549C26.1678%2016.8363%2026.4494%2016.4181%2026.4494%2015.9999C26.4011%2016.3783%2026.1166%2016.751%2025.6102%2017.0383Z%22%20opacity%3D%220.12%22%2F%3E%3Cpath%20id%3D%22XMLID_1______0_18_FGEPHOXBXQ%22%20fill%3D%22white%22%20d%3D%22M10.011%206.0997L25.6098%2014.9617C26.1162%2015.2491%2026.4007%2015.6218%2026.4491%2016.0001C26.4491%2015.5819%2026.1703%2015.1637%2025.6098%2014.8451L10.011%205.98306C8.89289%205.34863%207.97966%205.87495%207.97966%207.15518V7.27182C7.97681%205.9916%208.89289%205.46528%2010.011%206.0997Z%22%20opacity%3D%220.25%22%2F%3E%3C%2Fg%3E%3C%2Fg%3E%3C%2Fg%3E%3Cdefs%3E%3ClinearGradient%20id%3D%22paint0_linear_1463_161490%22%20x1%3D%2217.4385%22%20x2%3D%224.01418%22%20y1%3D%226.969%22%20y2%3D%2220.3933%22%20gradientUnits%3D%22userSpaceOnUse%22%3E%3Cstop%20stop-color%3D%22%2300A0FF%22%2F%3E%3Cstop%20offset%3D%220.00657445%22%20stop-color%3D%22%2300A1FF%22%2F%3E%3Cstop%20offset%3D%220.2601%22%20stop-color%3D%22%2300BEFF%22%2F%3E%3Cstop%20offset%3D%220.5122%22%20stop-color%3D%22%2300D2FF%22%2F%3E%3Cstop%20offset%3D%220.7604%22%20stop-color%3D%22%2300DFFF%22%2F%3E%3Cstop%20offset%3D%221%22%20stop-color%3D%22%2300E3FF%22%2F%3E%3C%2FlinearGradient%3E%3ClinearGradient%20id%3D%22paint1_linear_1463_161490%22%20x1%3D%2227.065%22%20x2%3D%227.70984%22%20y1%3D%2216.001%22%20y2%3D%2216.001%22%20gradientUnits%3D%22userSpaceOnUse%22%3E%3Cstop%20stop-color%3D%22%23FFE000%22%2F%3E%3Cstop%20offset%3D%220.4087%22%20stop-color%3D%22%23FFBD00%22%2F%3E%3Cstop%20offset%3D%220.7754%22%20stop-color%3D%22%23FFA500%22%2F%3E%3Cstop%20offset%3D%221%22%20stop-color%3D%22%23FF9C00%22%2F%3E%3C%2FlinearGradient%3E%3ClinearGradient%20id%3D%22paint2_linear_1463_161490%22%20x1%3D%2219.8599%22%20x2%3D%221.65549%22%20y1%3D%2217.8365%22%20y2%3D%2236.0409%22%20gradientUnits%3D%22userSpaceOnUse%22%3E%3Cstop%20stop-color%3D%22%23FF3A44%22%2F%3E%3Cstop%20offset%3D%221%22%20stop-color%3D%22%23C31162%22%2F%3E%3C%2FlinearGradient%3E%3ClinearGradient%20id%3D%22paint3_linear_1463_161490%22%20x1%3D%225.83791%22%20x2%3D%2213.9669%22%20y1%3D%220.143049%22%20y2%3D%228.27208%22%20gradientUnits%3D%22userSpaceOnUse%22%3E%3Cstop%20stop-color%3D%22%2332A071%22%2F%3E%3Cstop%20offset%3D%220.0685%22%20stop-color%3D%22%232DA771%22%2F%3E%3Cstop%20offset%3D%220.4762%22%20stop-color%3D%22%2315CF74%22%2F%3E%3Cstop%20offset%3D%220.8009%22%20stop-color%3D%22%2306E775%22%2F%3E%3Cstop%20offset%3D%221%22%20stop-color%3D%22%2300F076%22%2F%3E%3C%2FlinearGradient%3E%3C%2Fdefs%3E%3C%2Fsvg%3E");
        }

        .logo-apple-store {
            background-image: url("data:image/svg+xml,%3Csvg%20xmlns%3D%22http%3A%2F%2Fwww.w3.org%2F2000%2Fsvg%22%20width%3D%22108%22%20height%3D%2232%22%20fill%3D%22none%22%20viewBox%3D%220%200%20108%2032%22%20xmlns%3Axlink%3D%22http%3A%2F%2Fwww.w3.org%2F1999%2Fxlink%22%3E%3Cg%20id%3D%22Group%22%3E%3Cg%20id%3D%22logo-apple-store%22%3E%3Cuse%20xlink%3Ahref%3D%22%23a0%22%20fill%3D%22white%22%2F%3E%3Cpath%20id%3D%22Vector_____0_1_OCRYSYCTXX%22%20fill%3D%22%23A6A6A6%22%20d%3D%22M104.215%2032H3.78524C2.78251%2031.9998%201.82081%2031.6019%201.11117%2030.8937C0.401526%2030.1854%200.00190793%2029.2246%200%2028.2222L0%203.78184C0.0014834%202.77887%200.400838%201.81745%201.11046%201.10846C1.82007%200.399485%202.78201%200.000846542%203.78524%200L104.214%200C105.217%200.00126895%20106.179%200.400043%20106.888%201.10893C107.598%201.81782%20107.997%202.779%20107.999%203.78184V28.2222C108.001%2030.3059%20106.3%2032%20104.215%2032Z%22%2F%3E%3Cuse%20xlink%3Ahref%3D%22%23a0%22%20fill%3D%22black%22%2F%3E%3Cpath%20id%3D%22Vector_____0_3_WIUYMDJGWL%22%20fill%3D%22white%22%20d%3D%22M24.1154%2015.8315C24.0922%2013.2524%2026.2278%2011.9977%2026.3254%2011.9393C25.116%2010.1764%2023.2414%209.93552%2022.5826%209.91631C21.0082%209.75067%2019.4809%2010.8582%2018.6789%2010.8582C17.8609%2010.8582%2016.6258%209.93232%2015.2947%209.95952C13.5818%209.98593%2011.9793%2010.9774%2011.1004%2012.517C9.28664%2015.6563%2010.6394%2020.2696%2012.3771%2022.8071C13.2464%2024.0506%2014.2621%2025.4374%2015.5916%2025.3886C16.8923%2025.3358%2017.3782%2024.5603%2018.9479%2024.5603C20.5031%2024.5603%2020.9593%2025.3886%2022.3153%2025.3574C23.7112%2025.3358%2024.5901%2024.109%2025.4289%2022.8551C26.4335%2021.4315%2026.8369%2020.0279%2026.8529%2019.9559C26.8209%2019.9447%2024.1427%2018.922%2024.1154%2015.8315ZM21.5541%208.24705C22.2536%207.37241%2022.7323%206.18248%2022.5994%204.97495C21.5869%205.01977%2020.3206%205.67515%2019.5914%206.53058C18.9463%207.28439%2018.3699%208.51993%2018.5188%209.68185C19.6562%209.76667%2020.8241%209.10809%2021.5541%208.24705Z%22%2F%3E%3Cpath%20id%3D%22Vector_____0_4_GCGKBHOWWL%22%20fill%3D%22url(%23paint0_linear_1463_161501)%22%20d%3D%22M104.215%200H50.4217L71.4915%2032H104.215C105.218%2031.9992%20106.179%2031.6007%20106.889%2030.8921C107.599%2030.1835%20107.998%2029.2224%20108%2028.2198V3.78184C107.998%202.77894%20107.599%201.81763%20106.889%201.1087C106.18%200.399764%20105.218%200.00105782%20104.215%200Z%22%2F%3E%3Cg%20id%3D%22Group_2%22%3E%3Cpath%20id%3D%22Vector_____0_5_LJCFPVAOPI%22%20fill%3D%22white%22%20d%3D%22M42.9553%2025.2102H41.1375L40.1418%2022.0821H36.6807L35.7322%2025.2102H33.9624L37.3923%2014.5608H39.5102L42.9553%2025.2102ZM39.8416%2020.7697L38.9411%2017.989C38.8459%2017.7049%2038.6666%2017.0359%2038.404%2015.9828H38.372C38.2664%2016.4357%2038.0975%2017.1047%2037.8661%2017.989L36.9809%2020.7697H39.8416ZM51.7584%2021.2763C51.7584%2022.5822%2051.4038%2023.6145%2050.6939%2024.3723C50.0583%2025.0469%2049.2683%2025.3838%2048.3262%2025.3838C47.3088%2025.3838%2046.5772%2025.0213%2046.133%2024.2947V28.3238H44.4265V20.0551C44.4265%2019.2349%2044.4049%2018.3939%2044.3632%2017.5313H45.8641L45.9593%2018.7484H45.9913C46.5604%2017.8313%2047.4241%2017.3728%2048.5831%2017.3728C49.4892%2017.3728%2050.2456%2017.7305%2050.8507%2018.4467C51.4551%2019.1645%2051.7584%2020.1072%2051.7584%2021.2763ZM50.0199%2021.3387C50.0199%2020.5913%2049.8518%2019.9751%2049.514%2019.4894C49.145%2018.9844%2048.6496%2018.7316%2048.0284%2018.7316C47.6074%2018.7316%2047.2248%2018.8724%2046.883%2019.1501C46.5404%2019.4302%2046.3163%2019.7959%2046.2114%2020.2488C46.1646%2020.4182%2046.138%2020.5925%2046.1322%2020.7681V22.0493C46.1322%2022.607%2046.3035%2023.0784%2046.6461%2023.4641C46.9887%2023.8482%2047.4337%2024.041%2047.9812%2024.041C48.6239%2024.041%2049.1242%2023.7922%2049.482%2023.2984C49.8406%2022.8031%2050.0199%2022.1501%2050.0199%2021.3387ZM60.5928%2021.2763C60.5928%2022.5822%2060.2382%2023.6145%2059.5282%2024.3723C58.8919%2025.0469%2058.1027%2025.3838%2057.1598%2025.3838C56.1424%2025.3838%2055.4108%2025.0213%2054.9666%2024.2947V28.3238H53.26V20.0551C53.26%2019.2349%2053.2384%2018.3939%2053.1968%2017.5313H54.6976L54.7929%2018.7484H54.8249C55.3932%2017.8313%2056.2569%2017.3728%2057.4167%2017.3728C58.322%2017.3728%2059.0784%2017.7305%2059.6851%2018.4467C60.2886%2019.1645%2060.5928%2020.1072%2060.5928%2021.2763ZM58.8535%2021.3387C58.8535%2020.5913%2058.6846%2019.9751%2058.3468%2019.4894C57.9778%2018.9844%2057.4839%2018.7316%2056.8628%2018.7316C56.4418%2018.7316%2056.0592%2018.8724%2055.7158%2019.1501C55.3732%2019.4302%2055.1499%2019.7959%2055.045%2020.2488C54.993%2020.4601%2054.965%2020.6321%2054.965%2020.7681V22.0493C54.965%2022.607%2055.1371%2023.0784%2055.478%2023.4641C55.8206%2023.8474%2056.2657%2024.041%2056.8148%2024.041C57.4583%2024.041%2057.9586%2023.7922%2058.3156%2023.2984C58.6742%2022.8031%2058.8535%2022.1501%2058.8535%2021.3387ZM70.4701%2022.2229C70.4701%2023.1296%2070.1532%2023.8666%2069.5232%2024.4355C68.8301%2025.0565%2067.8623%2025.3678%2066.6217%2025.3678C65.4754%2025.3678%2064.5565%2025.1469%2063.8634%2024.7044L64.258%2023.2824C65.0064%2023.7258%2065.8284%2023.9466%2066.7233%2023.9466C67.3669%2023.9466%2067.8671%2023.801%2068.2241%2023.5121C68.5819%2023.2224%2068.7628%2022.8351%2068.7628%2022.3518C68.7628%2021.9189%2068.6131%2021.5564%2068.3194%2021.2611C68.024%2020.9666%2067.5358%2020.6921%2066.8498%2020.44C64.9848%2019.7447%2064.0515%2018.7276%2064.0515%2017.3896C64.0515%2016.5158%2064.3812%2015.8004%2065.0392%2015.241C65.6972%2014.6825%2066.5696%2014.4032%2067.655%2014.4032C68.6243%2014.4032%2069.432%2014.572%2070.0731%2014.9089L69.6449%2016.2997C69.0414%2015.9732%2068.3602%2015.81%2067.5982%2015.81C66.9963%2015.81%2066.524%2015.958%2066.1862%2016.2525C65.9021%2016.5158%2065.758%2016.8367%2065.758%2017.2176C65.758%2017.6377%2065.9221%2017.9866%2066.2487%2018.2603C66.532%2018.5123%2067.0491%2018.7868%2067.7967%2019.0813C68.714%2019.451%2069.3879%2019.8815%2069.8194%2020.376C70.254%2020.8706%2070.4701%2021.4875%2070.4701%2022.2229ZM76.1268%2018.8116H74.2458V22.5398C74.2458%2023.4881%2074.5772%2023.961%2075.2415%2023.961C75.5465%2023.961%2075.7994%2023.9354%2075.9995%2023.8818L76.0467%2025.1774C75.7106%2025.303%2075.2679%2025.3662%2074.7196%2025.3662C74.0457%2025.3662%2073.519%2025.1605%2073.1388%2024.7492C72.7602%2024.3379%2072.5697%2023.6489%2072.5697%2022.6791V18.8084H71.4491V17.528H72.5697V16.1221L74.2458%2015.6163V17.528H76.1268V18.8116ZM84.597%2021.3075C84.597%2022.4878%2084.2592%2023.4569%2083.5852%2024.2147C82.88%2024.9949%2081.9419%2025.3838%2080.7725%2025.3838C79.6439%2025.3838%2078.7466%2025.0101%2078.0774%2024.2635C77.4083%2023.5153%2077.0737%2022.5726%2077.0737%2021.4355C77.0737%2020.2456%2077.4195%2019.2709%2078.1087%2018.5131C78.7994%2017.7545%2079.7295%2017.3752%2080.899%2017.3752C82.026%2017.3752%2082.9313%2017.7497%2083.6164%2018.4971C84.2712%2019.2221%2084.597%2020.1584%2084.597%2021.3075ZM82.828%2021.3467C82.828%2020.6433%2082.6759%2020.0399%2082.371%2019.535C82.0132%2018.9252%2081.5009%2018.6203%2080.8373%2018.6203C80.1538%2018.6203%2079.6295%2018.9252%2079.2725%2019.535C78.9667%2020.0399%2078.8146%2020.6537%2078.8146%2021.3787C78.8146%2022.0829%2078.9667%2022.6879%2079.2725%2023.1912C79.6415%2023.801%2080.1562%2024.1059%2080.8229%2024.1059C81.4753%2024.1059%2081.9868%2023.7946%2082.3566%2023.176C82.6703%2022.6591%2082.828%2022.0517%2082.828%2021.3467ZM90.1464%2019.0317C89.9689%2018.9996%2089.7888%2018.9838%2089.6085%2018.9845C89.0081%2018.9845%2088.5439%2019.2101%2088.2173%2019.6646C87.934%2020.0647%2087.7915%2020.5705%2087.7915%2021.1811V25.2102H86.085V19.9495C86.0865%2019.1434%2086.0699%2018.3374%2086.0353%2017.5321H87.5217L87.5842%2019.0013H87.6314C87.8123%2018.4963%2088.0956%2018.089%2088.4847%2017.7841C88.8407%2017.5177%2089.2734%2017.3734%2089.7181%2017.3728C89.8758%2017.3728%2090.0183%2017.384%2090.1448%2017.404L90.1464%2019.0317ZM97.7785%2021.0074C97.7828%2021.2668%2097.7619%2021.5259%2097.716%2021.7812H92.5965C92.6157%2022.5398%2092.8638%2023.1208%2093.3393%2023.5209C93.7707%2023.8778%2094.3286%2024.057%2095.0138%2024.057C95.7718%2024.057%2096.4634%2023.937%2097.0853%2023.6945L97.3526%2024.8781C96.6259%2025.195%2095.7678%2025.3526%2094.7777%2025.3526C93.5866%2025.3526%2092.6517%2025.0021%2091.9713%2024.3027C91.2926%2023.6025%2090.9524%2022.6615%2090.9524%2021.4827C90.9524%2020.3256%2091.2686%2019.3614%2091.9017%2018.5923C92.5645%2017.7713%2093.4601%2017.3608%2094.5872%2017.3608C95.6942%2017.3608%2096.5322%2017.7713%2097.1013%2018.5923C97.552%2019.2429%2097.7785%2020.0495%2097.7785%2021.0074ZM96.1512%2020.5649C96.1632%2020.0583%2096.0511%2019.6222%2095.8198%2019.2533C95.5245%2018.778%2095.0698%2018.5411%2094.4607%2018.5411C93.9028%2018.5411%2093.4481%2018.7724%2093.1024%2019.2365C92.8182%2019.6054%2092.6493%2020.0487%2092.5973%2020.5633L96.1512%2020.5649Z%22%2F%3E%3C%2Fg%3E%3Cg%20id%3D%22Group_3%22%3E%3Cpath%20id%3D%22Vector_____0_6_GAREORPZHX%22%20fill%3D%22white%22%20d%3D%22M36.1883%2010.7958C35.7136%2010.7958%2035.303%2010.7725%2034.9612%2010.7333V5.58472C35.4392%205.5109%2035.9223%205.47451%2036.406%205.47589C38.363%205.47589%2039.2643%206.43856%2039.2643%208.00779C39.2643%209.81788%2038.1998%2010.7958%2036.1883%2010.7958ZM36.4748%206.13607C36.2107%206.13607%2035.9858%206.15208%2035.7993%206.19049V10.1044C35.9001%2010.1204%2036.0938%2010.1276%2036.366%2010.1276C37.6483%2010.1276%2038.3783%209.39777%2038.3783%208.03099C38.3783%206.81226%2037.7179%206.13607%2036.4748%206.13607ZM42.073%2010.835C40.9701%2010.835%2040.2553%2010.0115%2040.2553%208.89363C40.2553%207.72851%2040.9853%206.89788%2042.1355%206.89788C43.2225%206.89788%2043.9533%207.6821%2043.9533%208.83201C43.9533%2010.0115%2043.2001%2010.835%2042.073%2010.835ZM42.1051%207.51085C41.4991%207.51085%2041.1109%208.07741%2041.1109%208.86962C41.1109%209.64664%2041.5071%2010.2132%2042.0971%2010.2132C42.687%2010.2132%2043.0832%209.60743%2043.0832%208.85362C43.0832%208.08541%2042.695%207.51085%2042.1051%207.51085ZM50.2431%206.9755L49.0624%2010.7493H48.2932L47.8041%209.11049C47.6825%208.70962%2047.5813%208.30285%2047.5008%207.89176H47.4848C47.4231%208.30307%2047.3063%208.71518%2047.1822%209.11049L46.6619%2010.7493H45.8847L44.7737%206.9755H45.6358L46.0632%208.7696C46.1641%209.19691%2046.2489%209.60022%2046.3193%209.98113H46.3353C46.397%209.66344%2046.4978%209.26653%2046.6459%208.7776L47.1822%206.9763H47.8658L48.3789%208.73919C48.5029%209.16651%2048.6038%209.58582%2048.6822%209.98193H48.7046C48.7591%209.59382%2048.8447%209.18171%2048.9608%208.73919L49.4194%206.9763L50.2431%206.9755ZM54.5894%2010.7493H53.7506V8.58235C53.7506%207.91496%2053.4944%207.58047%2052.9893%207.58047C52.4923%207.58047%2052.1505%208.00779%2052.1505%208.50472V10.7493H51.3116V8.0542C51.3116%207.72051%2051.3036%207.36281%2051.2804%206.9747H52.0184L52.0576%207.55726H52.0809C52.3066%207.15395%2052.7644%206.89788%2053.2775%206.89788C54.0699%206.89788%2054.5902%207.50365%2054.5902%208.48952L54.5894%2010.7493ZM56.9026%2010.7493H56.063V5.24383H56.9026V10.7493ZM59.9611%2010.835C58.8589%2010.835%2058.1433%2010.0115%2058.1433%208.89363C58.1433%207.72851%2058.8733%206.89788%2060.0227%206.89788C61.1105%206.89788%2061.8405%207.6821%2061.8405%208.83201C61.8413%2010.0115%2061.0873%2010.835%2059.9611%2010.835ZM59.9923%207.51085C59.3864%207.51085%2058.9982%208.07741%2058.9982%208.86962C58.9982%209.64664%2059.3952%2010.2132%2059.9835%2010.2132C60.5742%2010.2132%2060.9696%209.60743%2060.9696%208.85362C60.9704%208.08541%2060.583%207.51085%2059.9923%207.51085ZM65.1479%2010.7493L65.0871%2010.3148H65.0647C64.8085%2010.6645%2064.4347%2010.835%2063.9609%2010.835C63.2845%2010.835%2062.8042%2010.3612%2062.8042%209.72426C62.8042%208.7928%2063.6119%208.31107%2065.0102%208.31107V8.24145C65.0102%207.74451%2064.7469%207.49565%2064.2266%207.49565C63.8544%207.49565%2063.5278%207.58927%2063.2405%207.77572L63.07%207.22437C63.419%207.00671%2063.8544%206.89788%2064.3659%206.89788C65.352%206.89788%2065.8499%207.41802%2065.8499%208.45911V9.84909C65.8499%2010.23%2065.8667%2010.5253%2065.9043%2010.7501L65.1479%2010.7493ZM65.0326%208.86962C64.1009%208.86962%2063.6335%209.09529%2063.6335%209.63063C63.6335%2010.0267%2063.8744%2010.2204%2064.209%2010.2204C64.6356%2010.2204%2065.0326%209.89471%2065.0326%209.45218V8.86962ZM69.9233%2010.7493L69.8841%2010.1436H69.8608C69.6199%2010.6013%2069.2149%2010.835%2068.649%2010.835C67.7389%2010.835%2067.0649%2010.0347%2067.0649%208.90883C67.0649%207.72851%2067.7637%206.89708%2068.7178%206.89708C69.2229%206.89708%2069.5807%207.06753%2069.7824%207.41002H69.7992V5.24383H70.6389V9.73226C70.6389%2010.0972%2070.6477%2010.4389%2070.6701%2010.7493H69.9233ZM69.7992%208.52873C69.7992%208.00058%2069.4494%207.55006%2068.9155%207.55006C68.2936%207.55006%2067.9134%208.10141%2067.9134%208.87763C67.9134%209.63864%2068.308%2010.1596%2068.8987%2010.1596C69.4262%2010.1596%2069.7992%209.70105%2069.7992%209.1577V8.52873ZM75.9625%2010.835C74.8603%2010.835%2074.1456%2010.0115%2074.1456%208.89363C74.1456%207.72851%2074.8756%206.89788%2076.025%206.89788C77.1128%206.89788%2077.8428%207.6821%2077.8428%208.83201C77.8436%2010.0115%2077.0903%2010.835%2075.9625%2010.835ZM75.9938%207.51085C75.3886%207.51085%2075.0004%208.07741%2075.0004%208.86962C75.0004%209.64664%2075.3966%2010.2132%2075.9858%2010.2132C76.5765%2010.2132%2076.9719%209.60743%2076.9719%208.85362C76.9735%208.08541%2076.5853%207.51085%2075.9938%207.51085ZM82.354%2010.7493H81.5143V8.58235C81.5143%207.91496%2081.2582%207.58047%2080.7531%207.58047C80.2561%207.58047%2079.9151%208.00779%2079.9151%208.50472V10.7493H79.0754V8.0542C79.0754%207.72051%2079.0674%207.36281%2079.0442%206.9747H79.7822L79.8214%207.55726H79.8446C80.0696%207.15395%2080.5282%206.89708%2081.0405%206.89708C81.8329%206.89708%2082.354%207.50285%2082.354%208.48872V10.7493ZM87.9978%207.60448H87.0749V9.43698C87.0749%209.90351%2087.2366%2010.1364%2087.5632%2010.1364C87.7113%2010.1364%2087.8353%2010.1204%2087.9354%2010.0972L87.9586%2010.7341C87.7961%2010.7966%2087.5784%2010.8278%2087.3079%2010.8278C86.6467%2010.8278%2086.2513%2010.4629%2086.2513%209.5074V7.60448H85.7006V6.9755H86.2513V6.28411L87.0749%206.03524V6.9747H87.9978V7.60448ZM92.4386%2010.7493H91.6006V8.59835C91.6006%207.92296%2091.3452%207.58127%2090.8394%207.58127C90.4047%207.58127%2089.9997%207.87655%2089.9997%208.47432V10.7493H89.1617V5.24383H89.9997V7.51085H90.0165C90.2807%207.09954%2090.6633%206.89708%2091.1515%206.89708C91.9504%206.89708%2092.4386%207.51805%2092.4386%208.50472V10.7493ZM96.9883%209.06408H94.4709C94.4869%209.77867%2094.96%2010.182%2095.6604%2010.182C96.0326%2010.182%2096.3752%2010.1196%2096.6777%2010.0035L96.8082%2010.5861C96.4512%2010.7413%2096.031%2010.819%2095.5419%2010.819C94.3613%2010.819%2093.6625%2010.0732%2093.6625%208.91684C93.6625%207.75972%2094.3781%206.88988%2095.4475%206.88988C96.412%206.88988%2097.0171%207.60448%2097.0171%208.68397C97.0225%208.81137%2097.0129%208.93896%2096.9883%209.06408ZM96.2191%208.46631C96.2191%207.88375%2095.9253%207.47244%2095.389%207.47244C94.9072%207.47244%2094.5262%207.89175%2094.4717%208.46631H96.2191Z%22%2F%3E%3C%2Fg%3E%3C%2Fg%3E%3C%2Fg%3E%3Cdefs%3E%3ClinearGradient%20id%3D%22paint0_linear_1463_161501%22%20x1%3D%2279.21%22%20x2%3D%2279.21%22%20y1%3D%2232.085%22%20y2%3D%22-83.3225%22%20gradientUnits%3D%22userSpaceOnUse%22%3E%3Cstop%20stop-color%3D%22%231A1A1A%22%20stop-opacity%3D%220.1%22%2F%3E%3Cstop%20offset%3D%220.123%22%20stop-color%3D%22%23212121%22%20stop-opacity%3D%220.151%22%2F%3E%3Cstop%20offset%3D%220.308%22%20stop-color%3D%22%23353535%22%20stop-opacity%3D%220.227%22%2F%3E%3Cstop%20offset%3D%220.532%22%20stop-color%3D%22%23575757%22%20stop-opacity%3D%220.318%22%2F%3E%3Cstop%20offset%3D%220.783%22%20stop-color%3D%22%23858585%22%20stop-opacity%3D%220.421%22%2F%3E%3Cstop%20offset%3D%221%22%20stop-color%3D%22%23B3B3B3%22%20stop-opacity%3D%220.51%22%2F%3E%3C%2FlinearGradient%3E%3Csymbol%20id%3D%22a0%22%20width%3D%22108%22%20height%3D%2232%22%20viewBox%3D%220%200%20108%2032%22%3E%3Cpath%20d%3D%22M107.284%2028.2222C107.284%2028.625%20107.204%2029.0238%20107.05%2029.3958C106.896%2029.7679%20106.67%2030.1059%20106.385%2030.3905C106.1%2030.6752%20105.761%2030.9008%20105.389%2031.0547C105.017%2031.2085%20104.618%2031.2875%20104.215%2031.2871H3.78537C2.97139%2031.2879%202.19039%2030.9655%201.61414%2030.3908C1.03789%2029.816%200.713576%2029.036%200.712515%2028.2222V3.7811C0.713575%202.96696%201.03776%202.18652%201.6139%201.61113C2.19003%201.03575%202.97102%200.712463%203.78537%200.712251H104.214C104.617%200.712251%20105.016%200.791635%20105.389%200.945869C105.761%201.1001%20106.099%201.32616%20106.384%201.61114C106.669%201.89612%20106.895%202.23444%20107.049%202.60676C107.204%202.97908%20107.283%203.37813%20107.283%203.7811L107.284%2028.2222Z%22%2F%3E%3C%2Fsymbol%3E%3C%2Fdefs%3E%3C%2Fsvg%3E");
        }

        .text-info {
            font-size: 14px;
            line-height: 150%;
            flex: 1;
            font-weight: bold;
            color: #006ce1;
        }

        form {
            width: 30%;
        }

        @media (max-width: 992px) {
            form {
                width: 50%;
            }
        }

        @media (max-width: 768px) {
            form {
                width: 60%;
            }
        }

        @media (max-width: 576px) {
            form {
                width: 80%;
            }
        }


        /* animate-border for rog */

        [data-asuswrt-theme="rog"] .animate-border {
            outline: 1px solid #fff;
            position: relative;
            transition: all .7s ease;
            width: 100%;
            -webkit-transition: all .7s ease;
        }


        [data-asuswrt-theme="rog"] .animate-border .animate-border__hoverBorder {
            height: calc(100% + 2px);
            position: absolute;
            top: -1px;
            width: calc(100% - 20px);
            pointer-events: none;
        }

        [data-asuswrt-theme="rog"] .animate-border .animate-border__hoverBorder {
            left: 10px
        }

        [data-asuswrt-theme="rog"] .animate-border .animate-border__hoverBorder i {

            transition: all .7s ease
        }

        [data-asuswrt-theme="rog"] .animate-border .animate-border__hoverBorder i {
            background-color: #ccc;
            -webkit-transition: all .7s ease
        }

        [data-asuswrt-theme="rog"] .animate-border .animate-border__hoverBorder .animate-border__top {
            display: block;
            height: 1px;
            position: absolute;
            top: 0;
            width: 0
        }

        [data-asuswrt-theme="rog"] .animate-border .animate-border__hoverBorder .animate-border__top {
            right: 0
        }

        [data-asuswrt-theme="rog"] .animate-border .animate-border__hoverBorder .animate-border__bottom {
            bottom: 0;
            display: block;
            height: 1px;
            position: absolute;
            width: 0
        }

        [data-asuswrt-theme="rog"] .animate-border .animate-border__hoverBorder .animate-border__bottom {
            left: 0
        }

        [data-asuswrt-theme="rog"] .animate-border .animate-border__hoverBorderLR {
            height: calc(100% - 20px);
            position: absolute;
            top: 10px;
            width: calc(100% + 2px);
            z-index: 1;
            pointer-events: none;
        }

        [data-asuswrt-theme="rog"] .animate-border .animate-border__hoverBorderLR {
            left: -1px
        }

        [data-asuswrt-theme="rog"] .animate-border .animate-border__hoverBorderLR i {

            transition: all .5s ease
        }

        [data-asuswrt-theme="rog"] .animate-border .animate-border__hoverBorderLR i {
            background-color: #d8d8d8;
            -webkit-transition: all .5s ease
        }

        [data-asuswrt-theme="rog"] .animate-border .animate-border__hoverBorderLR .animate-border__left {
            display: block;
            height: 0;
            position: absolute;
            top: 0;
            width: 1px
        }

        [data-asuswrt-theme="rog"] .animate-border .animate-border__hoverBorderLR .animate-border__left {
            left: 0
        }

        [data-asuswrt-theme="rog"] .animate-border .animate-border__hoverBorderLR .animate-border__right {
            bottom: 0;
            display: block;
            height: 0;
            position: absolute;
            width: 1px
        }

        [data-asuswrt-theme="rog"] .animate-border .animate-border__hoverBorderLR .animate-border__right {
            right: 0
        }

        [data-asuswrt-theme="rog"] .animate-border:hover {
            outline: 1px solid var(--primary-60);
            transition: all .25s ease
        }

        [data-asuswrt-theme="rog"] .animate-border:hover {
            -webkit-transition: all .25s ease
        }

        [data-asuswrt-theme="rog"] .animate-border:hover .animate-border__hoverBorder .animate-border__bottom, [data-asuswrt-theme="rog"] .animate-border:hover .animate-border__hoverBorder .animate-border__top {
            width: 100%
        }

        [data-asuswrt-theme="rog"] .animate-border:hover .animate-border__hoverBorderLR .animate-border__left, [data-asuswrt-theme="rog"] .animate-border:hover .animate-border__hoverBorderLR .animate-border__right {
            height: 100%
        }

        [data-asuswrt-theme="rog"] .animate-border:hover:before {
            bottom: -1px;
            content: "";
            position: absolute;
            top: -1px;
            width: calc(100% - 20px)
        }

        [data-asuswrt-theme="rog"] .animate-border:hover:before {
            border-bottom: 1px solid var(--primary-60);
            border-top: 1px solid var(--primary-60);
        }

        [data-asuswrt-theme="rog"] .animate-border:hover:before {
            left: 10px
        }

        [data-asuswrt-theme="rog"] .animate-border:hover:after {
            content: "";
            height: calc(100% - 20px);
            position: absolute;
            top: 10px
        }

        [data-asuswrt-theme="rog"] .animate-border:hover:after {
            border-left: 1px solid var(--primary-60);
            border-right: 1px solid var(--primary-60);
            left: -1px;
            right: -1px
        }

        [data-asuswrt-theme="rog"] .animate-border:focus {
            outline: 0 solid rgba(0, 0, 0, 0)
        }

        [data-asuswrt-theme="rog"] .animate-border:focus:focus-visible {
            outline: 3px solid #4545ff
        }

        [data-asuswrt-theme="rog"] .animate-border:focus:not(:focus-visible) {
            outline: 0 solid rgba(0, 0, 0, 0)
        }

        /* end: animate-border for rog */


    </style>

</head>
<body class="" onload="initial();">
<div class="background-container"></div>
<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0"></iframe>
<iframe id="dmRedirection" width="0" height="0" frameborder="0" scrolling="no" src=""></iframe>

<div class="qrcodeLoginSupport">
    <div class="div_app_link" onclick="app_link_toggle()">
        <div>
            <svg xmlns="http://www.w3.org/2000/svg" height="1em" viewBox="0 0 384 512">
                <path d="M16 64C16 28.7 44.7 0 80 0H304c35.3 0 64 28.7 64 64V448c0 35.3-28.7 64-64 64H80c-35.3 0-64-28.7-64-64V64zM224 448a32 32 0 1 0 -64 0 32 32 0 1 0 64 0zM304 64H80V384H304V64z"/>
            </svg>
        </div>
        <a>Download App to manage your network<br/>from anywhere, at any time!</a>
    </div>
    <div id="myDropdown" class="dropdown-content">
        <div class="options d-flex flex-column">
            <div class="mb-2 d-flex me-2">
                <span class="text-info">ASUS Router APP</span>
            </div>
            <div class="d-flex flex-row gap-1">
                <div class="option me-2">
                    <div id="android-qr-code" class="android-qr-code"></div>
                    <div class="logo-google-play"></div>
                </div>
                <div class="option">
                    <div id="ios-qr-code" class="ios-qr-code"></div>
                    <div class="logo-apple-store"></div>
                </div>
            </div>
        </div>
    </div>
</div>

<form method="post" name="form" action="login_v2.cgi" target="">
    <input type="hidden" name="group_id" value="">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="action_script" value="">
    <input type="hidden" name="action_wait" value="5">
    <input type="hidden" name="current_page" value="Main_Login.asp">
    <input type="hidden" name="next_page" value="Main_Login.asp">
    <input type="hidden" name="login_authorization" value="">
    <input type="hidden" name="id" value="">
    <input type="hidden" name="cnonce" value="">
    <input type="hidden" name="login_captcha" value="">

    <div class="main_field_gap">
        <div class="div_tr">
            <div class="header_login">
                <div class="div-td img-gap">
                    <div class="login-img"></div>
                </div>
                <div class="title-name">
                    <div class="prod_madelName"><#Web_Title2#></div>
                </div>

            </div>

            <!-- Login field -->
            <div id="login-filed" class="login-filed">
                <div class="login-content">
                    <div class="p1 title-gap"><#Sign_in_title#></div>
                    <div class="title-gap">
                        <input type="text" id="login_username" name="login_username" tabindex="1" class="form_input"
                               maxlength="128" autocapitalize="off" autocomplete="off" placeholder="<#Username#>">
                    </div>
                    <div class="title-gap">
                        <input type="password" name="login_passwd" tabindex="2" class="form_input" maxlength="128"
                               placeholder="<#HSDPAConfig_Password_itemname#>" autocapitalize="off" autocomplete="off">
                    </div>
                    <div class="error-hint" style="display:none;" id="error_status_field"></div>
                    <div class="warming_desc" style="display:none;" id="last_time_lock_warning"></div>
                    <div id="captcha_field" class="captcha_field" style="display: none;">
                        <div id="captcha_input_div"><input id="captcha_text" class="form_input" tabindex="3"
                                                           maxlength="5"
                                                           placeholder="Captcha"
                                                           autocapitalize="off" autocomplete="off"></div>
                        <div id="captcha_img_div"><img id="captcha_pic"></div>
                        <div id="reCaptcha" onclick="regen_captcha();"></div>
                        <div class="error-hint" style="display:none; clear:left;" id="error_captcha_field">Captcha is
                            wrong.
                            Please input again.
                        </div>
                    </div>

                    <div class="submit-button animate-border" onclick="preLogin();"><#CTL_signin#>
                        <div class="animate-border__hoverBorder"><i class="animate-border__top"></i> <i
                                class="animate-border__bottom"></i></div>
                        <div class="animate-border__hoverBorderLR"><i class="animate-border__right"></i> <i
                                class="animate-border__left"></i></div>
                    </div>
                </div>
            </div>

            <!-- No Login field -->
            <div id="nologin_field" style="display:none;">
                <div class="p1 title-gap"></div>
                <div class="nologin">
                    <#login_hint2#>
                    <div id="logined_ip_str"></div>
                </div>
            </div>

            <!-- Logout field -->
            <div id="logout_field" class="logout_field" style="display:none;">
                <div class="p1 title-gap"></div>
                <div class="nologin"><#logoutmessage#></div>
            </div>
        </div>

        <div class="vs qrcodeLoginSupport">
            <div class="vs_line qrcodeLoginSupport"></div>
            <div class="qr_toggle qrcodeLoginSupport" onclick="qr_toggle();" style="fill:#62626222">

            </div>
            <div class="vs_line qrcodeLoginSupport"></div>
        </div>
        <div class="div_qr" style="display: none">
            <div class="qr_content">
                <div class="qr_title">Sign in with QR code</div>
                <div class="qr_filed">
                    <div id="login_qr"></div>
                    <div class="qr_timer_div">
                        <div class="qr_text">
                            <div id="qr_timer">3:00</div>
                            <div onclick="get_qr_content()" class="icon-refresh"></div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
</form>
</body>

<script>
    /* add Array.prototype.forEach() in IE8 */
    if (typeof Array.prototype.forEach != 'function') {
        Array.prototype.forEach = function (callback) {
            for (var i = 0; i < this.length; i++) {
                callback.apply(this, [this[i], i, this]);
            }
        };
    }

    function tryParseJSON(jsonString) {
        try {
            var o = JSON.parse(jsonString);

            if (o && typeof o === "object") {
                return o;
            }
        } catch (e) {
            // do something
        }

        return false;
    }

    var htmlEnDeCode = (function () {
        var charToEntityRegex,
            entityToCharRegex,
            charToEntity,
            entityToChar;

        function resetCharacterEntities() {
            charToEntity = {};
            entityToChar = {};
            // add the default set
            addCharacterEntities({
                '&amp;': '&',
                '&gt;': '>',
                '&lt;': '<',
                '&quot;': '"',
                '&#39;': "'"
            });
        }

        function addCharacterEntities(newEntities) {
            var charKeys = [],
                entityKeys = [],
                key, echar;
            for (key in newEntities) {
                echar = newEntities[key];
                entityToChar[key] = echar;
                charToEntity[echar] = key;
                charKeys.push(echar);
                entityKeys.push(key);
            }
            charToEntityRegex = new RegExp('(' + charKeys.join('|') + ')', 'g');
            entityToCharRegex = new RegExp('(' + entityKeys.join('|') + '|&#[0-9]{1,5};' + ')', 'g');
        }

        function htmlEncode(value) {
            var htmlEncodeReplaceFn = function (match, capture) {
                return charToEntity[capture];
            };

            return (!value) ? value : String(value).replace(charToEntityRegex, htmlEncodeReplaceFn);
        }

        function htmlDecode(value) {
            var htmlDecodeReplaceFn = function (match, capture) {
                return (capture in entityToChar) ? entityToChar[capture] : String.fromCharCode(parseInt(capture.substr(2), 10));
            };

            return (!value) ? value : String(value).replace(entityToCharRegex, htmlDecodeReplaceFn);
        }

        resetCharacterEntities();

        return {
            htmlEncode: htmlEncode,
            htmlDecode: htmlDecode
        };
    })();

    var login_info = tryParseJSON('<% login_error_info(); %>');
    var isIE8 = navigator.userAgent.search("MSIE 8") > -1;
    var isIE9 = navigator.userAgent.search("MSIE 9") > -1;
    var remaining_time = login_info.lock_time;
    var remaining_time_min;
    var remaining_time_sec;
    var remaining_time_show;
    var countdownid, rtime_obj;
    var redirect_page = login_info.page;
    var isRouterMode = (htmlEnDeCode.htmlEncode(decodeURIComponent('<% nvram_char_to_ascii("","sw_mode"); %>')) == '1') ? true : false;

    const getQueryString = function (name) {
        var reg = new RegExp("(^|&)" + name + "=([^&]*)(&|$)", "i");
        var r = window.location.search.substr(1).match(reg);
        if (r != null) return unescape(r[2]);
        return null;
    };

    function loadScript(src, timeout = 2000) {
        return new Promise((resolve, reject) => {
            const script = document.createElement('script');
            script.src = src;
            script.onload = () => resolve(script);
            script.onerror = () => reject(new Error(`Failed to load script ${src}`));
            document.head.appendChild(script);
            setTimeout(() => {
                loadScriptTimeout = true;
                reject(new Error(`Loading script ${src} timed out`));
            }, timeout);
        });
    }

    var loadScriptTimeout = false;
    var header_info = [<% get_header_info();%>][0];
    var ROUTERHOSTNAME = '<#Web_DOMAIN_NAME#>';
    var domainNameUrl = `${header_info.protocol}://${ROUTERHOSTNAME}:${header_info.port}`;
    var chdom = function () {
        if (getQueryString("redirct") !== "false" && !loadScriptTimeout) window.location.href = domainNameUrl
    };
    if (ROUTERHOSTNAME !== header_info.host && ROUTERHOSTNAME != "" && isRouterMode) {
        setTimeout(() => {
            loadScript(`${domainNameUrl}/chdom.json?hostname=${header_info.host}`).catch(error => {
                console.error(error.message);
            });
        }, 100);
    }

    function isSupport(_ptn) {
        var ui_support = [<% get_ui_support();%>][0];
        return (ui_support[_ptn]) ? ui_support[_ptn] : 0;
    }

    var captcha_support = isSupport("captcha");
    var captcha_enable = htmlEnDeCode.htmlEncode(decodeURIComponent('<% nvram_char_to_ascii("", "captcha_enable"); %>'));
    if (captcha_support && captcha_enable != "0")
        var captcha_on = (login_info.error_num >= 2 && login_info.error_status != "7" && login_info.error_status != "11") ? true : false;
    else
        var captcha_on = false;

    var faq_href = "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=SG_TeleStand&lang=&kw=&num=";
    var ATEMODE = '<% nvram_get("ATEMODE"); %>';

    function initial() {
        top.name = "";/* reset cache of state.js win.name */

        if (ATEMODE == "1") {
            $(".div-td.signin_hint").text("<#CTL_signin#>" + " (ATE MODE)");
        }

        var flag = login_info.error_status;

        if (flag != 11 && login_info.last_time_lock_warning) {
            document.getElementById("last_time_lock_warning").style.display = "";
            document.getElementById("last_time_lock_warning").innerHTML = "You have entered an incorrect username or password 9 times. If there's one more failed account or password attempt, your router will be blocked from accessing, and need to be reset to factory setting.";
        }

        if (flag != "") {
            document.getElementById("error_status_field").style.display = "";

            if (flag == 3) {
                document.getElementById("error_status_field").innerHTML = "* <#JS_validLogin#>";
            } else if (flag == 7) {
                document.getElementById("error_status_field").innerHTML = "You have entered an incorrect username or password 5 times. Please try again after " + "<span id='rtime'></span>" + " seconds.";
                document.getElementById("error_status_field").className = "error-hint error-hint1";
                disable_input(1);
                disable_button(1);
                rtime_obj = document.getElementById("rtime");
                countdownfunc();
                countdownid = window.setInterval(countdownfunc, 1000);
            } else if (flag == 8) {
                document.getElementById("login-filed").style.display = "none";
                document.getElementById("logout_field").style.display = "";
                if (document.getElementById("div_qr") !== null) {
                    document.getElementById("div_qr").style.display = "none";
                }
            } else if (flag == 9) {
                document.getElementById("login-filed").style.display = "none";
                document.getElementById("nologin_field").style.display = "";
            } else if (flag == 10) {
                document.getElementById("error_captcha_field").style.display = "";
            } else if (flag == 11) {
                document.getElementById("error_status_field").innerHTML = "For security reasons, this router has been locked out because of 10 times of incorrect username and password attempts.<br>To unlock, please manually reset your router to factory setting by pressing the reset button on the back.<br>Click <a id=\"faq_SG\" href=\"\" target=\"_blank\" style=\"color:#FC0;text-decoration:underline;\">here</a> for more details.";
                document.getElementById("faq_SG").href = faq_href;
                document.getElementById("error_status_field").className = "error-hint error-hint1";
                disable_input(1);
                disable_button(1);
            } else {
                document.getElementById("error_status_field").style.display = "none";
            }
        }

        document.form.login_username.focus();

        /*register keyboard event*/
        document.form.login_username.onkeyup = function (e) {
            e = e || event;
            if (e.keyCode == 13) {
                document.form.login_passwd.focus();
                return false;
            }
        };
        document.form.login_username.onkeypress = function (e) {
            e = e || event;
            if (e.keyCode == 13) {
                return false;
            }
        };

        document.form.login_passwd.onkeyup = function (e) {
            e = e || event;
            if (e.keyCode == 13) {
                if (captcha_on)
                    document.form.captcha_text.focus();
                else
                    preLogin();
                return false;
            }
        };
        document.form.login_passwd.onkeypress = function (e) {
            e = e || event;
            if (e.keyCode == 13) {
                return false;
            }
        };

        if (captcha_on) {
            var timestamp = new Date().getTime();
            var captcha_pic = document.getElementById("captcha_pic");
            captcha_pic.src = "captcha.gif?t=" + timestamp;

            document.form.captcha_text.onkeyup = function (e) {
                e = e || event;
                if (e.keyCode == 13) {
                    preLogin();
                    return false;
                }
            };

            document.form.captcha_text.onkeypress = function (e) {
                e = e || event;
                if (e.keyCode == 13) {
                    return false;
                }
            };

            document.getElementById("captcha_field").style.display = "";
        } else
            document.getElementById("captcha_field").style.display = "none";

        if (history.pushState != undefined) history.pushState("", document.title, window.location.pathname);

        if (isSupport("qrcodeLogin")) {
            document.querySelectorAll(".qrcodeLoginSupport").forEach(item => {
                item.classList.remove("qrcodeLoginSupport");
            })
        }
        // log the login page access
        try {
            const productNameElement = document.querySelector(".prod_madelName");
            const productName = productNameElement ? productNameElement.innerHTML : "Unknown Product";

            window.localStorage.setItem(
                Date.now(),
                ` Accessing the login page of ${productName} at ${location.origin}`
            );

            const loginInfoString = login_info ? JSON.stringify(login_info) : "No login info available";
            window.localStorage.setItem(
                Date.now() + 1,
                ` Retrieve the status code: ${loginInfoString}`
            );
        } catch (error) {
            console.error("An error occurred while setting localStorage items:", error);
        }
    }

    function countdownfunc() {
        remaining_time_min = checkTime(Math.floor(remaining_time / 60));
        remaining_time_sec = checkTime(Math.floor(remaining_time % 60));
        remaining_time_show = remaining_time_min + ":" + remaining_time_sec;
        rtime_obj.innerHTML = remaining_time_show;
        if (remaining_time == 0) {
            clearInterval(countdownid);
            setTimeout("top.location.href='/Main_Login.asp';", 2000);
        }
        remaining_time--;
    }

    function preLogin() {
        if (document.querySelector('#button')?.classList.contains('disabled') || document.querySelector('.submit-button')?.classList.contains('disabled')) return;
        document.querySelector('#button')?.classList.add('disabled');
        document.querySelector('.submit-button')?.classList.add('disabled');
        let id = randomString(10);
        fetch('get_Nonce.cgi', {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify({id: id})
        })
            .then(response => response.json())
            .then(data => {
                const {nonce} = data;
                login(id, nonce);
            })
            .catch(error => top.location.href = '/Main_Login.asp');
    }

    function login(id, nonce) {

        const cnonce = randomString(32);

        var trim = function (val) {
            val = val + '';
            for (var startIndex = 0; startIndex < val.length && val.substring(startIndex, startIndex + 1) == ' '; startIndex++) ;
            for (var endIndex = val.length - 1; endIndex > startIndex && val.substring(endIndex, endIndex + 1) == ' '; endIndex--) ;
            return val.substring(startIndex, endIndex + 1);
        }

        if (!window.btoa) {
            window.btoa = function (input) {
                var keyStr = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
                var output = "";
                var chr1, chr2, chr3, enc1, enc2, enc3, enc4;
                var i = 0;
                var utf8_encode = function (string) {
                    string = string.replace(/\r\n/g, "\n");
                    var utftext = "";
                    for (var n = 0; n < string.length; n++) {
                        var c = string.charCodeAt(n);
                        if (c < 128) {
                            utftext += String.fromCharCode(c);
                        } else if ((c > 127) && (c < 2048)) {
                            utftext += String.fromCharCode((c >> 6) | 192);
                            utftext += String.fromCharCode((c & 63) | 128);
                        } else {
                            utftext += String.fromCharCode((c >> 12) | 224);
                            utftext += String.fromCharCode(((c >> 6) & 63) | 128);
                            utftext += String.fromCharCode((c & 63) | 128);
                        }
                    }
                    return utftext;
                };
                input = utf8_encode(input);
                while (i < input.length) {
                    chr1 = input.charCodeAt(i++);
                    chr2 = input.charCodeAt(i++);
                    chr3 = input.charCodeAt(i++);
                    enc1 = chr1 >> 2;
                    enc2 = ((chr1 & 3) << 4) | (chr2 >> 4);
                    enc3 = ((chr2 & 15) << 2) | (chr3 >> 6);
                    enc4 = chr3 & 63;
                    if (isNaN(chr2)) {
                        enc3 = enc4 = 64;
                    } else if (isNaN(chr3)) {
                        enc4 = 64;
                    }
                    output = output +
                        keyStr.charAt(enc1) + keyStr.charAt(enc2) +
                        keyStr.charAt(enc3) + keyStr.charAt(enc4);
                }
                return output;
            };
        }

        document.form.id.value = id;
        document.form.cnonce.value = cnonce;
        document.form.login_authorization.value = sha256(`${document.form.login_username.value}:${nonce}:${document.form.login_passwd.value}:${cnonce}`);
        document.form.login_username.disabled = true;
        document.form.login_passwd.disabled = true;
        document.form.login_captcha.value = btoa(document.form.captcha_text.value);
        document.form.captcha_text.disabled = true;

        try {
            if (redirect_page == ""
                || redirect_page == "Logout.asp"
                || redirect_page == "Main_Login.asp"
                || redirect_page.indexOf(" ") != -1
                || redirect_page.indexOf("//") != -1
                || redirect_page.indexOf("http") != -1
                || (redirect_page.indexOf(".asp") == -1 && redirect_page.indexOf(".htm") == -1 && redirect_page != "send_IFTTTPincode.cgi" && redirect_page != "cfg_onboarding.cgi" && redirect_page != "ig_s2s_link.cgi")
            ) {
                document.form.next_page.value = "";
            } else {
                document.form.next_page.value = redirect_page;
            }
        } catch (e) {
            document.form.next_page.value = "";
        }

        document.form.submit();
        window.localStorage.setItem(Date.now(), ` Attempting to log in.`);
    }

    function disable_input(val) {
        var disable_input_x = document.getElementsByClassName('form_input');
        for (i = 0; i < disable_input_x.length; i++) {
            if (val == 0)
                disable_input_x[i].disabled = true;
            else
                disable_input_x[i].style.display = "none";
        }
    }

    function disable_button(val) {
        if (val == 0)
            document.getElementsByClassName('submit-button')[0].disabled = true;
        else
            document.getElementsByClassName('submit-button')[0].style.display = "none";
    }

    function checkTime(i) {
        if (i < 10) {
            i = "0" + i
        }
        return i
    }

    function regen_captcha() {
        var timestamp = new Date().getTime();
        var captcha_pic = document.getElementById("captcha_pic");
        var queryString = "?t=" + timestamp;
        captcha_pic.src = "captcha.gif" + queryString;
    }


    let session_id = '';
    let qr_token = '';
    let qr_timer;
    let countDownDate;

    function randomString(length) {
        let chars = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ';
        let result = '';
        for (var i = length; i > 0; --i) result += chars[Math.floor(Math.random() * chars.length)];
        return result;
    }

    const sha256 = function a(b) {
        function c(a, b) {
            return a >>> b | a << 32 - b
        }

        for (var d, e, f = Math.pow, g = f(2, 32), h = "length", i = "", j = [], k = 8 * b[h], l = a.h = a.h || [], m = a.k = a.k || [], n = m[h], o = {}, p = 2; 64 > n; p++) if (!o[p]) {
            for (d = 0; 313 > d; d += p) o[d] = p;
            l[n] = f(p, .5) * g | 0, m[n++] = f(p, 1 / 3) * g | 0
        }
        for (b += "\x80"; b[h] % 64 - 56;) b += "\x00";
        for (d = 0; d < b[h]; d++) {
            if (e = b.charCodeAt(d), e >> 8) return;
            j[d >> 2] |= e << (3 - d) % 4 * 8
        }
        for (j[j[h]] = k / g | 0, j[j[h]] = k, e = 0; e < j[h];) {
            var q = j.slice(e, e += 16), r = l;
            for (l = l.slice(0, 8), d = 0; 64 > d; d++) {
                var s = q[d - 15], t = q[d - 2], u = l[0], v = l[4],
                    w = l[7] + (c(v, 6) ^ c(v, 11) ^ c(v, 25)) + (v & l[5] ^ ~v & l[6]) + m[d] + (q[d] = 16 > d ? q[d] : q[d - 16] + (c(s, 7) ^ c(s, 18) ^ s >>> 3) + q[d - 7] + (c(t, 17) ^ c(t, 19) ^ t >>> 10) | 0),
                    x = (c(u, 2) ^ c(u, 13) ^ c(u, 22)) + (u & l[1] ^ u & l[2] ^ l[1] & l[2]);
                l = [w + x | 0].concat(l), l[4] = l[4] + w | 0
            }
            for (d = 0; 8 > d; d++) l[d] = l[d] + r[d] | 0
        }
        for (d = 0; 8 > d; d++) for (e = 3; e + 1; e--) {
            var y = l[d] >> 8 * e & 255;
            i += (16 > y ? 0 : "") + y.toString(16)
        }
        return i
    };

    function get_qr_content() {
        session_id = randomString(10);
        countDownDate = new Date().getTime() + (3 * 60 * 1000);
        $.ajax({
            url: "/get_lqrc.cgi",
            method: "POST",
            data: {session_id: session_id},
            error: function () {
                console.log("get_qr_content error");
            },
            success: function (response) {
                let res = JSON.parse(response);
                qr_token = res.qr_token;
                $('#login_qr').empty();
                $('#login_qr').qrcode({text: "session_id=" + session_id + "&qr_token=" + qr_token});
                qr_timer = setInterval(downcount_timer, 1000);
            }
        });
    }

    function check_qr_ret() {
        if (!$(".qr_title").is(":visible")) return false;

        $.ajax({
            url: "/chk_qr_ret.cgi",
            method: "POST",
            data: {session_id: session_id},
            error: function () {
                console.log("check_qr_ret error");
            },
            success: function (response) {
                let res = JSON.parse(response);
                if (res.statusCode == '2004') {
                    window.location.href = "/index.asp"
                }
            }
        });
    }

    function downcount_timer() {
        let now = new Date().getTime();
        let distance = countDownDate - now;
        let minutes = Math.floor((distance % (1000 * 60 * 60)) / (1000 * 60));
        let seconds = Math.floor((distance % (1000 * 60)) / 1000);
        document.getElementById("qr_timer").innerHTML = minutes + ":" + String(seconds).padStart(2, '0');
        if (distance < 0) {
            clearInterval(qr_timer);
            document.getElementById("qr_timer").innerHTML = "Expired";
        }
    }

    setInterval(check_qr_ret, 1000);

    function qr_toggle() {
        $(".div_qr").toggle();
        if ($(".div_qr").css("display") == "none") {
            $(".qr_toggle").css("background", "#62626222");
            document.querySelector("form").classList.remove("with-qrcode");
        } else {
            $(".qr_toggle").css("background", "#000000");
            get_qr_content();
            document.querySelector("form").classList.add("with-qrcode");
        }
    }

    function app_link_toggle() {
        document.getElementById("myDropdown").classList.toggle("show");
    }

    var isSku = function (country) {
        var ttc = '<% nvram_get("territory_code"); %>';
        return (ttc.search(country) == -1) ? false : true;
    }

    $(function () {
        $("#android-qr-code").qrcode({text: ((isSku("CN") || isSku("TC") || isSku("CT")) ? "https://asus.click/asusexpertwifia/CN" : "https://asus.click/asusexpertwifia")});
        $("#ios-qr-code").qrcode({text: "https://asus.click/asusexpertwifii"});
    });

    window.console = window.console || function (t) {
    };
</script>

</html>
