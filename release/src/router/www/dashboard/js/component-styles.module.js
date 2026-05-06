// 樣式管理器 - 將 CSS 內嵌到 JavaScript 中
export const ComponentStyles = {
    // 共用變數和基礎樣式
    base: `
        @import "/css/bootstrap.min.css";
        @import "/css/bootstrap-customize.css";
        @import "/css/svg-icon.css";
        @import "/css/color-table.css";
        @import "/css/wrt-ui-alias-tokens.css";
    `,

    // 彈窗相關樣式
    popup: `
        .popup_bg {
            position: fixed;
            top: 0;
            right: 0;
            bottom: 0;
            left: 0;
            z-index: 2000;
            background: var(--mask-popup-bg-color);
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
            display: flex;
            flex-direction: column;
            width: 100%;
            pointer-events: auto;
            background-color: var(--popup-bg-color);
            background-clip: padding-box;
            border-radius: var(--radius-sm);
            border: 1px solid var(--popup-stroke-color);
            gap: var(--wrt-gutter-md);
            box-shadow: var(--shadow-elevation40);
        }

        .modal-header {
            display: flex;
            align-items: flex-start;
            justify-content: space-between;
            border-top-left-radius: 0.3rem;
            border-top-right-radius: 0.3rem;
            padding: var(--wrt-gutter-lg) var(--wrt-gutter-md) 0;
            border-bottom: none;
        }

        .modal-title {
            color: var(--text-primary);
            font-weight: bold;
            font-size: var(--font-size-body-01);
            margin-bottom: 0;
            line-height: 1.5;
        }

        .modal-close {
            cursor: pointer;
            width: 16px;
            height: 16px;
            background: none;
            border: none;
            padding: 0;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        
        .modal-close:hover {
            opacity: 0.7;
        }
        
        .modal-close:focus {
            outline: 2px solid var(--icon-default);
            outline-offset: 2px;
        }

        .modal-body {
            position: relative;
            flex: 1 1 auto;
        }

        .modal-footer {
            display: flex;
            align-items: center;
            justify-content: space-between;
            padding: 0 16px 10px;
            border-top: none;
        }

        @media (min-width: 576px) {
            .modal-dialog {
                max-width: 420px;
                margin: 5rem auto;
            }
        }

        /* 關閉圖示 */
        i.icon-close {
            display: block;
            height: 100%;
            width: 100%;
            background-color: var(--icon-default);
            mask-image: var(--ic-close-stroke);
            -webkit-mask-image: var(--ic-close-stroke);
            mask-repeat: no-repeat;
            mask-position: center;
            mask-size: contain;
            -webkit-mask-repeat: no-repeat;
            -webkit-mask-position: center;
            -webkit-mask-size: contain;
        }
    `,

    // 按鈕相關樣式
    button: `
        .btn {
            border-radius: var(--global-radius);
            cursor: pointer;
            text-align: center;
            -webkit-transition: all .2s ease-in-out;
            transition: all .2s ease-in-out;
            min-width: 90px;
            height: 32px;
            display: inline-block;
            font-size: var(--font-size-body-01);
            overflow: hidden;
            position: relative;
            color: var(--btn-text-color);
        }

        .btn.btn-primary {
            background-color: var(--wrt-btn-primary-bg);
            color: var(--wrt-btn-primary-color);
        }

        .btn.btn-secondary {
            background-color: transparent;
            color: var(--wrt-btn-secondary-color);
            border: 1px solid var(--wrt-btn-secondary-border);
        }

        .btn-primary:hover {
            background-color: var(--wrt-btn-primary-hover-bg) !important;
        }

        .btn-text {
            box-sizing: border-box;
            height: 100%;
            line-height: 18px;
            position: static;
            text-transform: uppercase;
        }

        .btn .btn-text span {
            align-items: center;
            display: flex;
            height: 100%;
            justify-content: center;
            line-height: 1.15;
            overflow: hidden;
            position: relative;
            word-break: break-word;
            z-index: 2;
        }

        .hover-color {
            position: absolute;
            width: 0;
            z-index: 1;
            transition: all .3s ease;
            -webkit-transition: all .3s ease;
            transform-origin: bottom left;
            bottom: 0;
            height: 44px;
            background-color: var(--wrt-btn-primary-hover-bg);
        }

        .btn:hover .hover-color {
            width: 135%;
        }
    `,

    // 切換開關樣式
    toggle: `
        .toggle-button {
            width: 46px;
            height: 24px;
            display: flex;
            padding: 4px;
            border-radius: var(--radius-lg);
            align-items: center;
            justify-content: space-between;
            border: 1.5px solid var(--wrt-switch-track-border-off);
            cursor: pointer;
            transition: all 0.6s;
            position: relative;
            overflow: hidden;
        }

        .toggle-button.with-text {
            width: 52px;
        }

        .toggle-button.with-text::before {
            font-family: "Roboto";
            content: "OFF";
            font-size: var(--font-size-body-01);
            font-weight: bold;
            color: var(--wrt-switch-track-border-off);
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
            color: var(--wrt-switch-label-color-on);
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
            background-color: var(--wrt-switch-track-bg-on);
            border: 1.5px solid var(--wrt-switch-track-border-on);
        }

        .toggle-button .toggle-button-handle {
            width: 16px;
            height: 16px;
            border-radius: var(--radius-lg);
            background-color: var(--wrt-switch-track-border-off);
            transition: all 0.6s;
            position: absolute;
            left: 4px;
        }

        .toggle-button.active .toggle-button-handle {
            background-color: var(--wrt-switch-thumb-bg-on);
            transform: translateX(20px);
        }

        .toggle-button.with-text.active .toggle-button-handle {
            background-color: var(--wrt-switch-thumb-bg-on);
            transform: translateX(26px);
        }
    `,

    // 載入動畫樣式
    loading: `
        .blur {
            position: absolute;
            top: 0;
            right: 0;
            bottom: 0;
            left: 0;
            z-index: 1900;
            backdrop-filter: blur(3px);
            -webkit-backdrop-filter: blur(3px);
        }

        .asuswrt-widget-loader {
            display: flex;
            align-items: center;
            justify-content: center;
            flex-direction: column;
            height: 100%;
            z-index: 2000;
        }

        .asuswrt-widget-loader > object {
            height: 64px;
            width: 64px;
            z-index: 2000;
        }

        @keyframes spinner {
            0% {
                transform: rotate(0deg)
            }
            to {
                transform: rotate(359deg)
            }
        }
    `,

    // 密碼輸入框樣式
    passwordInput: `
        .asuswrt-password-input {
            position: relative;
        }

        .asuswrt-password-input .password-input-container {
            position: relative;
            display: inline-block;
            width: 100%;
        }

        .asuswrt-password-input .form-control {
            width: 100%;
            padding: 0.375rem 0.75rem;
            border: 1px solid var(--border-default);
            border-radius: var(--global-radius, 0.375rem);
            font-size: 1rem;
            background-color: var(--wrt-input-bg);
            background-clip: padding-box;
            transition: border-color 0.15s ease-in-out, box-shadow 0.15s ease-in-out;
        }

        /* 當有切換按鈕時，調整右側 padding */
        .asuswrt-password-input.has-toggle .form-control {
            padding-right: 40px !important;
        }

        /* 當沒有切換按鈕時，使用預設 padding */
        .asuswrt-password-input:not(.has-toggle) .form-control {
            padding-right: 0.75rem !important;
        }

        .asuswrt-password-input .form-control:focus {
            outline: 0;
        }

        .asuswrt-password-input .password-toggle {
            position: absolute;
            right: 10px;
            top: 50%;
            transform: translateY(-50%);
            background: none;
            border: none;
            cursor: pointer;
            padding: 0;
            width: 24px;
            height: 24px;
            display: flex;
            align-items: center;
            justify-content: center;
            z-index: 2;
        }

        .asuswrt-password-input .password-toggle:hover {
            opacity: 0.7;
        }

        i.icon.icon-eye {
            display: block;
            height: 20px;
            width: 20px;
            background-color: var(--icon-default);
            mask-image: var(--ic-eye-open);
            -webkit-mask-image: var(--ic-eye-open);
            mask-repeat: no-repeat;
            mask-position: center;
            mask-size: contain;
            -webkit-mask-repeat: no-repeat;
            -webkit-mask-position: center;
            -webkit-mask-size: contain;
        }
        
        i.icon.icon-eye-slash {
            display: block;
            height: 20px;
            width: 20px;
            background-color: var(--icon-default);
            mask-image: var(--ic-eye-close);
            -webkit-mask-image: var(--ic-eye-close);
            mask-repeat: no-repeat;
            mask-position: center;
            mask-size: contain;
            -webkit-mask-repeat: no-repeat;
            -webkit-mask-position: center;
            -webkit-mask-size: contain;
        }
    
        .asuswrt-password-input .strength-meter {
            height: 8px;
            background-color: var(--wrt-input-bg);
            border-radius: var(--global-radius);
            overflow: hidden;
            margin-top: 8px;
        }

        .asuswrt-password-input .strength-fill {
            height: 100%;
            transition: width 0.3s ease, background-color 0.3s ease;
            border-radius: var(--global-radius);
        }

        .asuswrt-password-input .strength-text {
            font-size: 0.875rem;
            margin-top: 4px;
            color: var(--wrt-input-text-color);
        }

        .asuswrt-password-input .bg-danger { background-color: #dc3545 !important; }
        .asuswrt-password-input .bg-warning { background-color: #ffc107 !important; }
        .asuswrt-password-input .bg-info { background-color: #0dcaf0 !important; }
        .asuswrt-password-input .bg-primary { background-color: #0d6efd !important; }
        .asuswrt-password-input .bg-success { background-color: #198754 !important; }

        .asuswrt-password-input .form-control.is-valid {
            border-color: #198754;
            background-image: none !important;
            padding-right: 40px !important;
        }

        .asuswrt-password-input .form-control.is-invalid {
            border-color: #dc3545;
            background-image: none !important;
            padding-right: 40px !important;
        }

        /* 移除 Bootstrap 預設的驗證圖示 */
        .asuswrt-password-input .form-control:valid,
        .asuswrt-password-input .form-control.is-valid,
        .asuswrt-password-input .form-control:invalid,
        .asuswrt-password-input .form-control.is-invalid {
            background-image: none !important;
            background-repeat: no-repeat !important;
            background-position: right calc(0.375em + 0.1875rem) center !important;
            background-size: calc(0.75em + 0.375rem) calc(0.75em + 0.375rem) !important;
        }

        .asuswrt-password-input .valid-feedback {
            display: block;
            width: 100%;
            margin-top: 0.25rem;
            font-size: 0.875em;
            color: #198754;
        }

        .asuswrt-password-input .invalid-feedback {
            display: block;
            width: 100%;
            margin-top: 0.25rem;
            font-size: 0.875em;
            color: #dc3545;
        }
    `,

    // 客戶端選擇器樣式
    clientSelector: `
        .client-selector {
            position: relative;
            width: 100%;
        }
        
        .cs-dropdown-container {
            position: relative;
            width: 100%;
        }
        
        .cs-dropdown-input {
            height: 38px;
            width: 100%;
            padding: 12px 40px 12px 16px;
            font-size: 14px;
            border: 1px solid var(--border-default);
            border-radius: var(--global-radius, 8px);
            outline: none;
            transition: all 0.3s ease;
            background: var(--wrt-input-bg);
            cursor: pointer;
            box-sizing: border-box;
            color: var(--wrt-input-placeholder);
        }
        
        .cs-dropdown-input:focus {
            border-color: var(--wrt-input-focus-border);
            background: var(--wrt-input-focus-bg);
            box-shadow: 0 0 0 3px var(--wrt-input-focus-ring);
        }
        
        .cs-dropdown-arrow {
            position: absolute;
            right: 16px;
            top: 50%;
            transform: translateY(-50%);
            width: 0;
            height: 0;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 5px solid var(--icon-default);
            transition: transform 0.3s ease;
            pointer-events: none;
        }
        
        .cs-dropdown-arrow.open {
            transform: translateY(-50%) rotate(180deg);
            border-top: 5px solid var(--icon-primary);
        }
        
        .cs-dropdown-list {
            position: absolute;
            top: 100%;
            left: 0;
            right: 0;
            background: var(--wrt-input-bg);
            border: 1px solid var(--border-default);
            border-top: none;
            border-radius: 0 0 var(--global-radius) var(--global-radius);
            border-radius: var(--global-radius);
            max-height: 300px;
            overflow-y: auto;
            z-index: 1000;
            opacity: 0;
            visibility: hidden;
            transform: translateY(-10px);
            transition: all 0.3s ease;
            box-shadow: var(--shadow-elevation20);
            box-sizing: border-box;
        }
        
        .cs-dropdown-list.open {
            opacity: 1;
            visibility: visible;
            transform: translateY(0);
        }
        
        .cs-dropdown-item {
            padding: 12px 16px;
            cursor: pointer;
            transition: background-color 0.2s ease;
            border-bottom: 1px solid var(--border-default);
        }
        
        .cs-dropdown-item:last-child {
            border-bottom: none;
        }
        
        .cs-dropdown-item:hover {
            background-color: var(--wrt-list-group-hover-bg);
        }
        
        .cs-dropdown-item.selected {
            background-color: var(--wrt-list-group-active-bg);
            color: var(--wrt-list-group-active-color);
        }
        
        .cs-dropdown-item.hidden {
            display: none;
        }
        
        .cs-device-item {
            display: flex;
            flex-direction: column;
            gap: 4px;
        }
        
        .cs-device-name {
            display: flex;
            gap: 5px;
            align-items: center;
            font-weight: 600;
            font-size: 14px;
            color: var(--text-primary);
        }
        
        .cs-dropdown-item.selected .cs-device-name {
            color: var(--wrt-list-group-active-color);
        }
        
        .cs-device-details {
            display: flex;
            gap: 6px;
            font-size: 12px;
            color: var(--text-secondary);
            flex-wrap: wrap;
        }
        
        .cs-dropdown-item.selected .cs-device-details {
            color: var(--wrt-list-group-active-color);
        }
        
        .cs-device-mac,
        .cs-device-ip {
            background: rgba(var(--surface-invert-rgb),0.1);
            padding: 2px 6px;
            border-radius: 3px;
            font-size: 11px;
        }

        .cs-address-disabled {
            opacity: 0.5;
            cursor: not-allowed !important;
            pointer-events: none;
        }

        .cs-dropdown-item.selected .cs-device-mac,
        .cs-dropdown-item.selected .cs-device-ip {
            background: rgba(var(--surface-invert-rgb),0.2);
        }
        
        .cs-no-results {
            padding: 16px;
            text-align: center;
            color: var(--text-secondary);
            font-style: italic;
            font-size: 14px;
        }
        
        .cs-selected-value {
            margin-top: 12px;
            padding: 8px 16px;
            background: var(--surface-card);
            border-radius: var(--global-radius, 8px);
            border-left: 4px solid var(--brand-primary);
            box-shadow: 0 0 0 1px var(--border-default);
        }
        
        .cs-selected-info {
            position: relative;
        }
        
        .cs-selected-info strong {
            color: var(--brand-primary);
            font-size: 14px;
            display: block;
            margin-bottom: 8px;
            text-align: left;
        }
        
        .cs-close-button {
            position: absolute;
            top: 0;
            right: 0;
            background: none;
            border: none;
            font-size: 18px;
            font-weight: bold;
            color: var(--icon-default);
            cursor: pointer;
            padding: 0;
            width: 20px;
            height: 20px;
            display: flex;
            align-items: center;
            justify-content: center;
            border-radius: 50%;
            transition: all 0.2s ease;
        }
        
        .cs-close-button:hover {
            background-color: var(--surface-hover);
            color: var(--icon-active);
        }
        
        .cs-close-button:active {
            transform: scale(0.95);
        }
        
        .cs-selected-value .cs-device-info {
            display: flex;
            flex-direction: column;
            gap: 6px;
        }
        
        .cs-selected-value .cs-device-name {
            font-weight: 600;
            font-size: 16px;
            color: var(--text-primary);
        }
        
        .cs-selected-value .cs-device-details {
            display: flex;
            gap: 6px;
            flex-wrap: wrap;
        }
        
        .cs-selected-value .cs-device-mac,
        .cs-selected-value .cs-device-ip {
            background: var(--surface-hover);
            padding: 4px 8px;
            border-radius: 4px;
            font-size: 12px;
            color: var(--text-secondary);
        }
        
        .cs-dropdown-list::-webkit-scrollbar {
            width: var(--wrt-scrollbar-width);
        }
        
        .cs-dropdown-list::-webkit-scrollbar-track {
            background: var(--wrt-scrollbar-track-hover-color);
            border-radius: var(--wrt-scrollbar-radius);
        }
        
        .cs-dropdown-list::-webkit-scrollbar-thumb {
            background: var(--wrt-scrollbar-thumb-color);
            border-radius: var(--wrt-scrollbar-radius);
        }
        
        .cs-dropdown-list::-webkit-scrollbar-thumb:hover {
            background: var(--wrt-scrollbar-thumb-hover-color);
        }
        
        i.connect-status {
            width: 8px;
            height: 8px;
            display: block;
            border-radius: 100px;
        }
        
        i.connect-status.online {
            background-color: var(--hint-on-status-fill);
        }
        
        i.connect-status.offline {
            background-color: var(--hint-off-status-fill);
        }
        
        @media (max-width: 480px) {
            .cs-device-details {
                flex-direction: column;
                gap: 4px !important;
            }
            
            .cs-selected-value .cs-device-details {
                flex-direction: column;
                gap: 8px !important;
            }
        }
    `
};

// 樣式組合工具
export class StyleManager {
    static combineStyles(...styleKeys) {
        return styleKeys
            .map(key => ComponentStyles[key] || '')
            .filter(style => style.trim())
            .join('\n\n');
    }

    static createStyleSheet(styles) {
        return `<style>${styles}</style>`;
    }

    static injectStyles(shadowRoot, ...styleKeys) {
        const combinedStyles = this.combineStyles(...styleKeys);
        const styleElement = document.createElement('style');
        styleElement.textContent = combinedStyles;
        shadowRoot.appendChild(styleElement);
    }
}