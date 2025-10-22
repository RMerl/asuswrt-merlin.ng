import { StyleManager } from './component-styles.module.js';

export class AsuswrtPopupPanel {
    constructor() {
        this.element = document.createElement('div');
        const shadowRoot = this.element.attachShadow({mode: 'open'});
        this.shadowRoot = shadowRoot;
        
        const template = document.createElement('template');
        const styles = StyleManager.combineStyles('base', 'popup');
        
        const modalId = `modal-${Math.random().toString(36).substr(2, 9)}`;
        const titleId = `${modalId}-title`;
        const bodyId = `${modalId}-body`;
        
        template.innerHTML = `
            <style>${styles}</style>
            <div class="popup_bg" aria-hidden="false">
                <div 
                    class="modal" 
                    role="dialog" 
                    aria-modal="true"
                    aria-labelledby="${titleId}"
                    aria-describedby="${bodyId}"
                    tabindex="-1">
                    <div class="modal-dialog">
                        <div class="modal-content">
                            <div class="modal-header">
                                <div id="${titleId}" class="modal-title" role="heading" aria-level="2">Popup Title</div>
                                <button 
                                    class="modal-close" 
                                    type="button"
                                    aria-label="Close dialog"
                                    tabindex="0">
                                    <i class="icon-close" aria-hidden="true"></i>
                                </button>
                            </div>
                            <div id="${bodyId}" class="modal-body p-0" role="document"></div>
                            <div class="modal-footer"></div>
                        </div>
                    </div>
                </div>
            </div>
        `;
        shadowRoot.appendChild(template.content.cloneNode(true));

        const closeButton = shadowRoot.querySelector('.modal-close');
        const modal = shadowRoot.querySelector('.modal');
        
        // 點擊關閉按鈕
        closeButton.addEventListener('click', () => {
            this.close();
        });
        
        // 鍵盤支援 - ESC 鍵關閉
        modal.addEventListener('keydown', (e) => {
            if (e.key === 'Escape') {
                this.close();
            }
        });
        
        // 鍵盤支援 - Tab 鍵循環焦點
        this.setupFocusTrap(modal);
        
        // 點擊背景關閉（可選）
        shadowRoot.querySelector('.popup_bg').addEventListener('click', (e) => {
            if (e.target === e.currentTarget) {
                this.close();
            }
        });
        
        // 鍵盤支援 - Enter 和 Space 鍵在關閉按鈕上
        closeButton.addEventListener('keydown', (e) => {
            if (e.key === 'Enter' || e.key === ' ') {
                e.preventDefault();
                this.close();
            }
        });
        
        // 當彈窗顯示時，聚焦到模態框
        setTimeout(() => {
            modal.focus();
        }, 100);
    }
    
    setupFocusTrap(modal) {
        const focusableElements = modal.querySelectorAll(
            'button, [href], input, select, textarea, [tabindex]:not([tabindex="-1"])'
        );
        const firstElement = focusableElements[0];
        const lastElement = focusableElements[focusableElements.length - 1];

        modal.addEventListener('keydown', (e) => {
            if (e.key === 'Tab') {
                if (e.shiftKey) {
                    if (document.activeElement === firstElement) {
                        lastElement.focus();
                        e.preventDefault();
                    }
                } else {
                    if (document.activeElement === lastElement) {
                        firstElement.focus();
                        e.preventDefault();
                    }
                }
            }
        });
    }

    render() {
        return this.element;
    }

    close() {
        this.element.remove();
    }
}

export class AsuswrtButton {
    constructor(props) {
        const {
            text = '',
            className = '',
            animation = '', //slide
            onclick = () => {},
            ariaLabel = null,
            ariaDescribedBy = null,
            disabled = false,
            type = 'button' // button, submit, reset
        } = props;
        
        this.element = document.createElement('div');
        const shadowRoot = this.element.attachShadow({mode: 'open'});
        this.shadowRoot = shadowRoot;
        this.disabled = disabled;
        this.onclick = onclick;
        
        const template = document.createElement('template');
        const styles = StyleManager.combineStyles('base', 'button');
        const buttonId = `button-${Math.random().toString(36).substr(2, 9)}`;

        const ariaAttributes = [
            ariaLabel ? `aria-label="${ariaLabel}"` : '',
            ariaDescribedBy ? `aria-describedby="${ariaDescribedBy}"` : '',
            disabled ? 'aria-disabled="true"' : ''
        ].filter(attr => attr).join(' ');

        if (animation === 'slide') {
            template.innerHTML = `
                <style>${styles}</style>
                <button 
                    id="${buttonId}"
                    class="btn ${className}"
                    type="${type}"
                    ${ariaAttributes}
                    ${disabled ? 'disabled' : ''}
                    tabindex="${disabled ? '-1' : '0'}"
                    role="button">
                    <div class="btn-text">
                        <span>${text}</span>
                    </div>
                    <div class="hover-color" aria-hidden="true"></div>
                </button>
            `;
        } else {
            template.innerHTML = `
                <style>${styles}</style>
                <button 
                    id="${buttonId}"
                    class="btn ${className}"
                    type="${type}"
                    ${ariaAttributes}
                    ${disabled ? 'disabled' : ''}
                    tabindex="${disabled ? '-1' : '0'}"
                    role="button">
                    <div class="btn-text">
                        <span>${text}</span>
                    </div>
                </button>
            `;
        }
        shadowRoot.appendChild(template.content.cloneNode(true));

        this.buttonElement = shadowRoot.querySelector('.btn');
        
        // 點擊事件
        this.buttonElement.addEventListener('click', (e) => {
            if (!this.disabled) {
                onclick(e);
            }
        });
        
        // 鍵盤支援
        this.buttonElement.addEventListener('keydown', (e) => {
            if ((e.key === 'Enter' || e.key === ' ') && !this.disabled) {
                e.preventDefault();
                onclick(e);
            }
        });
        
        return this.element;
    }
    
    setDisabled(disabled) {
        this.disabled = disabled;
        if (disabled) {
            this.buttonElement.setAttribute('disabled', '');
            this.buttonElement.setAttribute('aria-disabled', 'true');
            this.buttonElement.setAttribute('tabindex', '-1');
        } else {
            this.buttonElement.removeAttribute('disabled');
            this.buttonElement.removeAttribute('aria-disabled');
            this.buttonElement.setAttribute('tabindex', '0');
        }
    }
    
    setText(text) {
        const textSpan = this.buttonElement.querySelector('.btn-text span');
        if (textSpan) {
            textSpan.textContent = text;
        }
    }
    
    setAriaLabel(label) {
        this.buttonElement.setAttribute('aria-label', label);
    }
    
    focus() {
        this.buttonElement.focus();
    }

    // render() {
    //     return this.element;
    // }
}

export class ToggleButton {
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
        const styles = StyleManager.combineStyles('base', 'toggle');
        const toggleId = `toggle-${Math.random().toString(36).substr(2, 9)}`;
        
        template.innerHTML = `
            <style>${styles}</style>
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
            detail: { active: this.active, value: this.active }
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

export class AsuswrtWidgetLoading {
    constructor(props) {
        const div = document.createElement('div');
        div.style.width = '100%';
        div.style.height = 'calc(100% - 56px)';
        div.style.position = 'absolute';
        
        const shadowRoot = div.attachShadow({mode: 'open'});
        const template = document.createElement('template');
        const styles = StyleManager.combineStyles('base', 'loading');
        
        template.innerHTML = `
            <style>${styles}</style>
            <div class="blur"></div>
            <div class="asuswrt-widget-loader">
                <object data="/images/ic-asus-stroke.svg" type="image/svg+xml" style="width: 64px; height: 64px;"></object>
            </div>
        `;
        shadowRoot.appendChild(template.content.cloneNode(true));
        this.shadowRoot = shadowRoot;
        this.element = div;
    }

    render() {
        return this.element;
    }

    hide() {
        this.element.remove();
    }
}

export class ClientSelector {
    constructor(options = {}) {
        this.options = {
            placeholder: `<#SearchDevice_By_Name_Mac_Ip#>`,
            data: [],
            width: '100%',
            maxHeight: '300px',
            showSelectedValue: true,
            className: '',
            onSelect: null,
            searchFields: ['name', 'mac', 'ip'], // 可搜尋的欄位
            ...options
        };

        this.isOpen = false;
        this.selectedIndex = -1;
        this.filteredItems = [];
        this.selectedValue = null;

        this.createElements();
        this.bindEvents();
    }

    createElements() {
        // 建立 Shadow DOM
        this.container = document.createElement('div');
        const shadowRoot = this.container.attachShadow({mode: 'open'});
        this.shadowRoot = shadowRoot;
        
        // 注入樣式
        const styles = StyleManager.combineStyles('base', 'clientSelector');
        const template = document.createElement('template');
        
        const selectorId = `client-selector-${Math.random().toString(36).substr(2, 9)}`;
        const inputId = `${selectorId}-input`;
        const listboxId = `${selectorId}-listbox`;
        const selectedId = `${selectorId}-selected`;
        
        template.innerHTML = `
            <style>${styles}</style>
            <div class="client-selector ${this.options.className}" id="${selectorId}">
                <div class="cs-dropdown-container">
                    <input 
                        id="${inputId}"
                        type="text" 
                        class="cs-dropdown-input" 
                        placeholder="${this.options.placeholder}" 
                        readonly
                        role="combobox"
                        aria-expanded="false"
                        aria-haspopup="listbox"
                        aria-controls="${listboxId}"
                        aria-autocomplete="list"
                        aria-label="Device selector"
                        tabindex="0">
                    <div class="cs-dropdown-arrow" aria-hidden="true"></div>
                    <div 
                        id="${listboxId}"
                        class="cs-dropdown-list" 
                        role="listbox"
                        aria-label="Available devices"
                        tabindex="-1"></div>
                </div>
                ${this.options.showSelectedValue ? `
                    <div class="cs-selected-value" style="display: none;" id="${selectedId}">
                        <div class="cs-selected-info">
                            <strong><#Selected_Device#>:</strong>
                            <button class="cs-close-button" type="button" aria-label="Clear device selection" tabindex="0">×</button>
                            <div class="cs-device-info">
                                <div class="cs-device-name"></div>
                                <div class="cs-device-details">
                                    <span class="cs-device-mac"></span>
                                    <span class="cs-device-ip"></span>
                                </div>
                            </div>
                        </div>
                    </div>
                ` : ''}
            </div>
        `;
        
        shadowRoot.appendChild(template.content.cloneNode(true));
        
        // 取得 Shadow DOM 內的元素引用
        this.selectorContainer = shadowRoot.querySelector('.client-selector');
        this.dropdownContainer = shadowRoot.querySelector('.cs-dropdown-container');
        this.input = shadowRoot.querySelector('.cs-dropdown-input');
        this.arrow = shadowRoot.querySelector('.cs-dropdown-arrow');
        this.list = shadowRoot.querySelector('.cs-dropdown-list');
        this.selectedDisplay = shadowRoot.querySelector('.cs-selected-value');
        
        // 創建選項
        this.createItems();
    }

    createItems() {
        this.items = [];
        this.options.data.forEach((item, index) => {
            const element = document.createElement('div');
            element.className = 'cs-dropdown-item';
            const deviceName = (item.nickName !== '') ? item.nickName : item.name;
            const statusText = item.isOnline ? 'online' : 'offline';
            const ipText = (item.ip !== '') ? item.ip : 'N/A';
            
            element.innerHTML = `
                <div class="cs-device-item">
                    <div class="cs-device-name">
                        <div class="client-status" aria-label="Device is ${statusText}">
                            ${(item.isOnline) ? `<i class='connect-status online' aria-hidden="true"></i>` : `<i class='connect-status offline' aria-hidden="true"></i>`}
                        </div>
                        ${deviceName}
                    </div>
                    <div class="cs-device-details">
                        <span class="cs-device-mac">MAC: ${item.mac}</span>
                        <span class="cs-device-ip">IP: ${ipText}</span>
                    </div>
                </div>
            `;
            
            // 設置無障礙屬性
            element.setAttribute('role', 'option');
            element.setAttribute('aria-label', `${deviceName}, MAC: ${item.mac}, IP: ${ipText}, Status: ${statusText}`);
            element.setAttribute('aria-selected', 'false');
            element.setAttribute('tabindex', '-1');
            
            element.dataset.value = item.mac;
            element.dataset.name = deviceName;
            element.dataset.mac = item.mac;
            element.dataset.ip = item.ip;
            element.dataset.index = index;
            this.list.appendChild(element);
            this.items.push(element);
        });
        this.filteredItems = [...this.items];
    }

    // 移除 injectStyles 方法，改用 Shadow DOM 內建樣式

    bindEvents() {
        // 點擊輸入框
        this.input.addEventListener('click', (e) => {
            e.stopPropagation();
            this.toggle();
        });

        // 輸入搜尋
        this.input.addEventListener('input', () => {
            this.filterItems();
        });

        // 鍵盤導航
        this.input.addEventListener('keydown', (e) => {
            this.handleKeyDown(e);
        });

        // 點擊選項
        this.items.forEach((item) => {
            item.addEventListener('click', () => {
                this.selectItem(item);
            });

            // 滑鼠懸停效果
            item.addEventListener('mouseenter', () => {
                this.clearSelection();
                item.classList.add('selected');
            });
        });

        // 點擊關閉按鈕
        if (this.selectedDisplay) {
            const closeButton = this.selectedDisplay.querySelector('.cs-close-button');
            if (closeButton) {
                closeButton.addEventListener('click', (e) => {
                    e.preventDefault();
                    e.stopPropagation();
                    this.clearSelectedValue();
                });
            }
        }

        // 點擊外部關閉 (需要考慮 Shadow DOM)
        document.addEventListener('click', (e) => {
            if (!this.container.contains(e.target) && !this.shadowRoot.contains(e.target)) {
                this.close();
            }
        });
    }

    rebindEvents() {
        // 點擊選項
        this.items.forEach((item) => {
            item.addEventListener('click', () => {
                this.selectItem(item);
            });

            // 滑鼠懸停效果
            item.addEventListener('mouseenter', () => {
                this.clearSelection();
                item.classList.add('selected');
            });
        });
    }

    toggle() {
        if (this.isOpen) {
            this.close();
        } else {
            this.open();
        }
    }

    open() {
        this.isOpen = true;
        this.input.removeAttribute('readonly');
        this.input.setAttribute('aria-expanded', 'true');
        this.input.focus();
        this.list.classList.add('open');
        this.arrow.classList.add('open');
        this.input.style.borderRadius = 'var(--global-radius, 8px) var(--global-radius, 8px) 0 0';
        
        // 通知螢幕閱讀器
        this.announceToScreenReader(`Device list opened. ${this.filteredItems.length} options available.`);
    }

    close() {
        this.isOpen = false;
        this.input.setAttribute('readonly', 'true');
        this.input.setAttribute('aria-expanded', 'false');
        this.list.classList.remove('open');
        this.arrow.classList.remove('open');
        this.input.style.borderRadius = 'var(--global-radius, 8px)';
        this.selectedIndex = -1;
        this.clearSelection();
        
        // 通知螢幕閱讀器
        this.announceToScreenReader('Device list closed.');
    }
    
    announceToScreenReader(message) {
        // 創建臨時的 live region 來通知螢幕閱讀器
        const announcement = document.createElement('div');
        announcement.setAttribute('aria-live', 'polite');
        announcement.setAttribute('aria-atomic', 'true');
        announcement.style.position = 'absolute';
        announcement.style.left = '-10000px';
        announcement.style.width = '1px';
        announcement.style.height = '1px';
        announcement.style.overflow = 'hidden';
        
        this.shadowRoot.appendChild(announcement);
        announcement.textContent = message;
        
        // 移除臨時元素
        setTimeout(() => {
            if (announcement.parentNode) {
                announcement.parentNode.removeChild(announcement);
            }
        }, 1000);
    }

    filterItems() {
        const searchTerm = this.input.value.toLowerCase();
        let hasVisibleItems = false;

        this.filteredItems = [];

        this.items.forEach((item) => {
            const name = item.dataset.name.toLowerCase();
            const mac = item.dataset.mac.toLowerCase();
            const ip = item.dataset.ip.toLowerCase();

            // 檢查是否在任何搜尋欄位中找到匹配
            const isVisible = this.options.searchFields.some(field => {
                switch (field) {
                    case 'name':
                        return name.includes(searchTerm);
                    case 'mac':
                        return mac.includes(searchTerm);
                    case 'ip':
                        return ip.includes(searchTerm);
                    default:
                        return false;
                }
            });

            if (isVisible) {
                item.classList.remove('hidden');
                this.filteredItems.push(item);
                hasVisibleItems = true;
            } else {
                item.classList.add('hidden');
            }
        });

        this.toggleNoResults(!hasVisibleItems);

        if (!this.isOpen && searchTerm) {
            this.open();
        }
    }

    toggleNoResults(show) {
        let noResultsElement = this.list.querySelector('.cs-no-results');

        if (show && !noResultsElement) {
            noResultsElement = document.createElement('div');
            noResultsElement.className = 'cs-no-results';
            noResultsElement.textContent = 'No matching devices found.';
            this.list.appendChild(noResultsElement);
        } else if (!show && noResultsElement) {
            noResultsElement.remove();
        }
    }

    handleKeyDown(e) {
        if (!this.isOpen) {
            if (e.key === 'Enter' || e.key === 'ArrowDown' || e.key === 'ArrowUp') {
                e.preventDefault();
                this.open();
            }
            return;
        }

        switch (e.key) {
            case 'ArrowDown':
                e.preventDefault();
                this.navigateDown();
                break;
            case 'ArrowUp':
                e.preventDefault();
                this.navigateUp();
                break;
            case 'Enter':
                e.preventDefault();
                this.selectCurrentItem();
                break;
            case 'Escape':
                e.preventDefault();
                this.close();
                break;
        }
    }

    navigateDown() {
        this.clearSelection();
        this.selectedIndex = Math.min(this.selectedIndex + 1, this.filteredItems.length - 1);
        this.highlightItem();
    }

    navigateUp() {
        this.clearSelection();
        this.selectedIndex = Math.max(this.selectedIndex - 1, 0);
        this.highlightItem();
    }

    highlightItem() {
        if (this.selectedIndex >= 0 && this.selectedIndex < this.filteredItems.length) {
            this.filteredItems[this.selectedIndex].classList.add('selected');
            this.scrollToItem(this.filteredItems[this.selectedIndex]);
        }
    }

    scrollToItem(item) {
        const listRect = this.list.getBoundingClientRect();
        const itemRect = item.getBoundingClientRect();

        if (itemRect.bottom > listRect.bottom) {
            this.list.scrollTop += itemRect.bottom - listRect.bottom;
        } else if (itemRect.top < listRect.top) {
            this.list.scrollTop -= listRect.top - itemRect.top;
        }
    }

    selectCurrentItem() {
        if (this.selectedIndex >= 0 && this.selectedIndex < this.filteredItems.length) {
            this.selectItem(this.filteredItems[this.selectedIndex]);
        }
    }

    selectItem(item) {
        console.log(item)
        const value = item.dataset.value;
        const name = item.dataset.name;
        const mac = item.dataset.mac;
        const ip = item.dataset.ip;
        const index = parseInt(item.dataset.index);

        this.selectedValue = {value, name, mac, ip, index};
        this.input.value = name;
        this.input.setAttribute('aria-describedby', `Selected device: ${name}`);

        // 更新選項的 aria-selected 狀態
        this.items.forEach(opt => opt.setAttribute('aria-selected', 'false'));
        item.setAttribute('aria-selected', 'true');

        if (this.options.showSelectedValue && this.selectedDisplay) {
            const nameEl = this.selectedDisplay.querySelector('.cs-device-name');
            const macEl = this.selectedDisplay.querySelector('.cs-device-mac');
            const ipEl = this.selectedDisplay.querySelector('.cs-device-ip');

            nameEl.textContent = name;
            macEl.textContent = `MAC: ${mac}`;
            ipEl.textContent = ip ? `IP: ${ip}` : 'IP: N/A';

            this.selectedDisplay.style.display = 'block';
            this.selectedDisplay.setAttribute('aria-live', 'polite');
        }

        this.close();
        
        // 通知螢幕閱讀器選擇結果
        this.announceToScreenReader(`Selected device: ${name}`);

        // 執行回調函數
        if (typeof this.options.onSelect === 'function') {
            this.options.onSelect(this.selectedValue);
        }

        // 觸發自定義事件
        this.container.dispatchEvent(new CustomEvent('clientSelect', {
            detail: this.selectedValue
        }));
    }

    clearSelection() {
        this.items.forEach(item => item.classList.remove('selected'));
    }

    clearSelectedValue() {
        this.selectedValue = null;
        this.input.value = '';
        if (this.selectedDisplay) {
            this.selectedDisplay.style.display = 'none';
        }

        // 觸發清除事件
        this.container.dispatchEvent(new CustomEvent('clientClear', {
            detail: null
        }));

        // 如果有 onClear 回調函數，執行它
        if (typeof this.options.onClear === 'function') {
            this.options.onClear();
        }
    }

    // 公開方法
    render(targetElement) {
        if (typeof targetElement === 'string') {
            targetElement = document.querySelector(targetElement);
        }

        if (!targetElement) {
            console.error('ClientSelector: 目標元素不存在');
            return this;
        }

        targetElement.appendChild(this.container);
        return this;
    }

    getInputValue() {
        return this.input.value;
    }

    getValue() {
        return this.selectedValue;
    }

    getMac() {
        return this.selectedValue ? this.selectedValue.mac : null;
    }

    setValue(value) {
        const item = this.items.find(item => item.dataset.value === value);
        if (item) {
            this.selectItem(item);
        }
        return this;
    }

    setData(data) {
        this.options.data = data;
        this.list.innerHTML = '';
        this.createItems();
        this.rebindEvents();
        return this;
    }

    reset() {
        this.selectedValue = null;
        this.input.value = '';
        if (this.selectedDisplay) {
            this.selectedDisplay.style.display = 'none';
        }
        this.close();
        return this;
    }

    destroy() {
        this.container.remove();
        return this;
    }

    on(event, callback) {
        this.container.addEventListener(event, callback);
        return this;
    }
}

export class PasswordInput {
    constructor(container, options = {}) {
        if (typeof container === 'string') {
            this.container = document.getElementById(container);
        } else {
            this.container = container;
        }
        
        // 建立 Shadow DOM
        const shadowRoot = this.container.attachShadow({mode: 'open'});
        this.shadowRoot = shadowRoot;
        
        this.options = {
            placeholder: 'Please enter your password',
            showToggle: true,
            showStrengthMeter: true,
            minLength: 8,
            size: '', // 可選: 'form-control-sm' 或 'form-control-lg'
            ...options
        };

        this.password = '';
        this.isVisible = false;
        this.callbacks = [];

        this.init();
    }

    init() {
        this.createHTML();
        this.bindEvents();
    }

    createHTML() {
        const sizeClass = this.options.size ? ` ${this.options.size}` : '';
        const styles = StyleManager.combineStyles('base', 'passwordInput');
        
        const template = document.createElement('template');
        const inputId = `password-input-${Math.random().toString(36).substr(2, 9)}`;
        const strengthMeterId = `strength-meter-${Math.random().toString(36).substr(2, 9)}`;
        const strengthTextId = `strength-text-${Math.random().toString(36).substr(2, 9)}`;
        const toggleId = `toggle-${Math.random().toString(36).substr(2, 9)}`;
        
        template.innerHTML = `
            <style>${styles}</style>
            <div class="asuswrt-password-input" role="group" aria-labelledby="${inputId}-label">
                <div class="password-input-container">
                    <input 
                        id="${inputId}"
                        type="password" 
                        class="form-control${sizeClass}" 
                        placeholder="${this.options.placeholder}"
                        autocomplete="new-password"
                        aria-describedby="${this.options.showStrengthMeter ? `${strengthTextId} passwordHelp` : 'passwordHelp'}"
                        aria-required="true"
                        aria-invalid="false"
                        role="textbox"
                        aria-label="Password input field"
                    >
                    ${this.options.showToggle ? `
                        <button 
                            id="${toggleId}"
                            class="password-toggle" 
                            type="button" 
                            aria-label="Toggle password visibility"
                            aria-controls="${inputId}"
                            aria-pressed="false"
                            tabindex="0">
                            <i class="icon icon-eye" aria-hidden="true"></i>
                        </button>
                    ` : ''}
                </div>
                
                ${this.options.showStrengthMeter ? `
                    <div 
                        id="${strengthMeterId}" 
                        class="strength-meter" 
                        role="progressbar" 
                        aria-valuenow="0" 
                        aria-valuemin="0" 
                        aria-valuemax="100" 
                        aria-label="Password strength indicator">
                        <div class="strength-fill bg-danger" style="width: 0%"></div>
                    </div>
                    <div 
                        id="${strengthTextId}" 
                        class="strength-text" 
                        aria-live="polite" 
                        aria-atomic="true"
                        role="status"></div>
                ` : ''}
            </div>
        `;
        
        this.shadowRoot.appendChild(template.content.cloneNode(true));

        // 獲取 Shadow DOM 內的元素引用
        this.passwordInputDiv = this.shadowRoot.querySelector('.asuswrt-password-input');
        this.input = this.shadowRoot.querySelector('.form-control');
        this.toggleBtn = this.shadowRoot.querySelector('.password-toggle');
        this.toggleIcon = this.shadowRoot.querySelector('.password-toggle i');
        this.strengthFill = this.shadowRoot.querySelector('.strength-fill');
        this.strengthText = this.shadowRoot.querySelector('.strength-text');

        // 根據是否有切換按鈕來添加 CSS 類
        if (this.options.showToggle) {
            this.passwordInputDiv.classList.add('has-toggle');
        }
    }

    bindEvents() {
        // 密碼輸入事件
        this.input.addEventListener('input', (e) => {
            this.password = e.target.value;
            this.updateStrength();
            this.notifyCallbacks();
        });

        // 顯示/隱藏密碼切換
        if (this.toggleBtn) {
            this.toggleBtn.addEventListener('click', () => {
                this.togglePasswordVisibility();
            });
        }
    }

    togglePasswordVisibility() {
        this.isVisible = !this.isVisible;
        this.input.type = this.isVisible ? 'text' : 'password';

        if (this.toggleIcon && this.toggleBtn) {
            this.toggleIcon.className = this.isVisible ? 'icon icon-eye-slash' : 'icon icon-eye';
            this.toggleBtn.setAttribute('aria-pressed', this.isVisible.toString());
            this.toggleBtn.setAttribute('aria-label', this.isVisible ? 'Hide password' : 'Show password');
            
            // 通知螢幕閱讀器狀態變化
            this.input.setAttribute('aria-describedby', 
                this.input.getAttribute('aria-describedby') + (this.isVisible ? ' password-visible' : ''));
        }
    }

    checkPasswordStrength(password) {
        const checks = {
            length: password.length >= this.options.minLength,
            uppercase: /[A-Z]/.test(password),
            lowercase: /[a-z]/.test(password),
            number: /\d/.test(password),
            special: /[!@#$%^&*()_+\-=\[\]{};':"\\|,.<>\/?]/.test(password)
        };

        const score = Object.values(checks).filter(Boolean).length;

        let strength = 'very-weak';
        let strengthText = '';
        let strengthClass = 'bg-danger';
        let strengthWidth = '0%';

        if (score === 0 && password.length === 0) {
            strength = 'none';
            strengthText = '';
            strengthWidth = '0%';
        } else if (score <= 1) {
            strength = 'very-weak';
            strengthText = `<#PASS_score0#>`;
            strengthClass = 'bg-danger';
            strengthWidth = '20%';
        } else if (score === 2) {
            strength = 'weak';
            strengthText = `<#PASS_score1#>`;
            strengthClass = 'bg-warning';
            strengthWidth = '40%';
        } else if (score === 3) {
            strength = 'medium';
            strengthText = `<#PASS_score2#>`;
            strengthClass = 'bg-info';
            strengthWidth = '60%';
        } else if (score === 4) {
            strength = 'strong';
            strengthText = `<#PASS_score3#>`;
            strengthClass = 'bg-primary';
            strengthWidth = '80%';
        } else if (score === 5) {
            strength = 'very-strong';
            strengthText = `<#PASS_score4#>`;
            strengthClass = 'bg-success';
            strengthWidth = '100%';
        }

        return {
            strength,
            strengthText,
            strengthClass,
            strengthWidth,
            score,
            checks
        };
    }

    updateStrength() {
        const result = this.checkPasswordStrength(this.password);

        // 更新強度指示器
        if (this.strengthFill && this.strengthText) {
            // 移除舊的背景類
            this.strengthFill.className = this.strengthFill.className.replace(/bg-\w+/g, '');
            // 添加新的背景類
            this.strengthFill.classList.add('strength-fill', result.strengthClass);
            this.strengthFill.style.width = result.strengthWidth;

            // 更新進度條的 ARIA 屬性
            const progressValue = parseInt(result.strengthWidth);
            const strengthMeter = this.strengthFill.parentElement;
            if (strengthMeter) {
                strengthMeter.setAttribute('aria-valuenow', progressValue.toString());
                strengthMeter.setAttribute('aria-label', `Password strength: ${result.strengthText || 'None'} (${progressValue}%)`);
            }

            // 更新狀態文字，並通知螢幕閱讀器
            const strengthMessage = result.strengthText ? `Password strength: ${result.strengthText}` : '';
            this.strengthText.textContent = strengthMessage;
            
            // 為螢幕閱讀器提供更詳細的描述
            if (result.strengthText) {
                this.strengthText.setAttribute('aria-label', 
                    `Password strength is ${result.strengthText}. Score: ${result.score} out of 5.`);
            }
        }

        // 移除密碼要求檢查功能

        // 更新輸入框樣式和無障礙屬性
        this.input.classList.remove('is-valid', 'is-invalid');
        if (this.password.length > 0) {
            if (this.isValid()) {
                this.input.classList.add('is-valid');
                this.input.setAttribute('aria-invalid', 'false');
            } else {
                this.input.classList.add('is-invalid');
                this.input.setAttribute('aria-invalid', 'true');
            }
        } else {
            this.input.setAttribute('aria-invalid', 'false');
        }

        return result;
    }

    // 註冊密碼變化回調
    onPasswordChange(callback) {
        this.callbacks.push(callback);
    }

    // 通知所有回調函數
    notifyCallbacks() {
        const strength = this.checkPasswordStrength(this.password);
        this.callbacks.forEach(callback => {
            callback(this.password, strength);
        });
    }

    // 獲取當前密碼
    getPassword() {
        return this.password;
    }

    // 設置密碼
    setPassword(password) {
        this.password = password;
        this.input.value = password;
        this.updateStrength();
        this.notifyCallbacks();
    }

    // 清空密碼
    clear() {
        this.setPassword('');
    }

    // 獲取密碼強度
    getStrength() {
        return this.checkPasswordStrength(this.password);
    }

    // 驗證密碼是否符合最低要求
    isValid() {
        const strength = this.checkPasswordStrength(this.password);
        return strength.score >= 3; // 至少中等強度
    }

    // 設置驗證狀態
    setValidationState(isValid, message = '') {
        this.input.classList.remove('is-valid', 'is-invalid');

        // 移除現有的回饋信息
        const existingFeedback = this.shadowRoot.querySelector('.invalid-feedback, .valid-feedback');
        if (existingFeedback) {
            existingFeedback.remove();
        }

        if (isValid !== null) {
            this.input.classList.add(isValid ? 'is-valid' : 'is-invalid');

            if (message) {
                const feedbackDiv = document.createElement('div');
                feedbackDiv.className = isValid ? 'valid-feedback' : 'invalid-feedback';
                feedbackDiv.textContent = message;
                
                // 插入到密碼輸入容器之後
                const passwordContainer = this.shadowRoot.querySelector('.password-input-container');
                passwordContainer.parentNode.insertBefore(feedbackDiv, passwordContainer.nextSibling);
            }
        }
    }

    // 禁用/啟用輸入
    setDisabled(disabled) {
        this.input.disabled = disabled;
        if (this.toggleBtn) {
            this.toggleBtn.disabled = disabled;
        }
    }
}