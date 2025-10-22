export class WiFiInterferenceChart {
    constructor(mac) {
        this.interferenceEnabled = false;
        this.wifiStatus = [];
        this.chartData = [];
        this.interferenceData = [];
        this.dfsData = [];
        this.sysData = [];
        this.chart = null;
        this.txopChartInstance = null;
        this.mac = mac;

        this.nowTime = new Date();
        this.chartStartTime = new Date(this.nowTime - 1 * 60 * 60 * 1000);
        this.chartEndTime = new Date(this.nowTime - (-3 * 60 * 1000));

        this.channelBarShow = true;
        this.wifiInterShow = true;
        this.nonwifiInterShow = true;
        this.dfsShow = true;

        // Add optimization helpers
        this.domCache = new Map(); // Cache DOM elements
        this.eventHandlers = new Map(); // Track event handlers
        this.isInitialized = false;

        // Auto refresh functionality - load from localStorage
        this.autoRefresh = this.loadAutoRefreshState();
        this.refreshInterval = null;
        this.currentTimeRange = "1H"; // Default time range

        this.element = null;

        this.signalRange = {
            min: {value: 30, width: 30},
            max: {value: 70, width: 70}
        };

        // Initialize band data
        this.initializeBandData();

        // Create DOM element
        this.createElement();

        // Setup event handlers
        this.setupEventHandlers();

        // Cache commonly used DOM elements after creation
        this.cacheDOMElements();
    }

    // Helper method for DOM caching
    getCachedElement(selector) {
        if (!this.element) {
            console.error('Element not initialized yet');
            return null;
        }

        if (!this.domCache.has(selector)) {
            const element = this.element.querySelector(selector);
            this.domCache.set(selector, element);
        }
        return this.domCache.get(selector);
    }

    // Helper method for event handler tracking
    addEventHandler(element, event, handler) {
        const key = `${element.constructor.name}_${event}_${Date.now()}`;
        this.eventHandlers.set(key, {element, event, handler});
        element.addEventListener(event, handler);
        return key;
    }

    // Load AUTO REFRESH state from localStorage
    loadAutoRefreshState() {
        try {
            const saved = localStorage.getItem('wifiInterference_autoRefresh');
            return saved === 'true';
        } catch (error) {
            console.warn('Failed to load auto refresh state:', error);
            return false; // Default to disabled
        }
    }

    // Save AUTO REFRESH state to localStorage
    saveAutoRefreshState(state) {
        try {
            localStorage.setItem('wifiInterference_autoRefresh', state.toString());
        } catch (error) {
            console.warn('Failed to save auto refresh state:', error);
        }
    }

    // Cache commonly used DOM elements
    cacheDOMElements() {
        if (!this.element) return;

        const selectors = [
            '#wifi_inter_chart',
            '#txop_chart',
            '#band_change .segmented_picker',
            '#range_options_btn',
            '#range_options_popup',
            '#popup_close',
            '#interference_toggle',
            '#auto_refresh_toggle',
            '.channel_plan_title',
            '.channel_plan_freq',
            '.timeline'
        ];

        // Optimized caching using for-of loop for better performance
        for (const selector of selectors) {
            this.getCachedElement(selector);
        }
    }

    // Initialize band data
    initializeBandData() {
        const wlBandSeqData = system.wlBandSeq;
        const order = ['2g', '2g1', '5g', '5g1', '5g2', '6g', '6g1', '6g2'];
        const origSupportedBand = order.filter(key => wlBandSeqData.hasOwnProperty(key));
        const supportedBand = [];
        for (const band of origSupportedBand) {
            if (band.startsWith('5g') && origSupportedBand.filter(b => b.startsWith('5g')).length === 1) {
                supportedBand.push({band: '5G', bandName: '5G1', channel: wlBandSeqData[band].channel});
            } else if (band.startsWith('5g') && origSupportedBand.filter(b => b.startsWith('5g')).length === 2) {
                supportedBand.push({
                    band: band.toUpperCase(),
                    bandName: band.toUpperCase(),
                    channel: wlBandSeqData[band].channel
                });
            } else if (band.startsWith('6g') && origSupportedBand.filter(b => b.startsWith('6g')).length === 1) {
                supportedBand.push({band: '6G', bandName: '6G1', channel: wlBandSeqData[band].channel});
            } else if (band.startsWith('6g') && origSupportedBand.filter(b => b.startsWith('6g')).length === 2) {
                supportedBand.push({
                    band: band.toUpperCase(),
                    bandName: band.toUpperCase(),
                    channel: wlBandSeqData[band].channel
                });
            } else if (band.startsWith('2g')) {
                supportedBand.push({band: '2G', bandName: '2G', channel: wlBandSeqData[band].channel});
            } else {
                supportedBand.push({band: band, bandName: band, channel: wlBandSeqData[band].channel});
            }
        }
        this.showBand = supportedBand[0].band.toUpperCase();
        this.controlChannelList = supportedBand[0].channel;
        this.supportedBand = supportedBand;
    }

    // Extract band text helper
    getBandText(band) {
        switch (band) {
            case "2G1":
            case "2G":
                return "2.4 GHz";
            case "5G":
                return "5 GHz";
            case "5G1":
                return "5 GHz-1";
            case "5G2":
                return "5 GHz-2";
            case "6G":
                return "6 GHz";
            case "6G1":
                return "6 GHz-1";
            case "6G2":
                return "6 GHz-2";
            default:
                return "";
        }
    }

    getBandMaxChannel(band) {
        const channelList = this.supportedBand.find(item => item.band.toUpperCase() === band)?.channel || [];
        const numbers = channelList.map(Number);
        return Math.max(...numbers);
    }

    // Extract band frequency text helper (optimized with caching)
    getBandFreqText(band) {
        if (!this.bandFreqCache) {
            this.bandFreqCache = new Map();
        }

        if (this.bandFreqCache.has(band)) {
            return this.bandFreqCache.get(band);
        }
        const channelList = this.supportedBand.find(item => item.band.toUpperCase() === band)?.channel || [];
        const numbers = channelList.map(Number);
        const max = Math.max(...numbers);
        const min = Math.min(...numbers);
        const minFreq = this.getChannelFrequencyRange(band, min, 20).startMHz;
        const maxFreq = this.getChannelFrequencyRange(band, max, 20).endMHz;
        const result = `${minFreq} - ${maxFreq} MHz`;

        this.bandFreqCache.set(band, result);
        return result;
    }

    getSignalWidth(signalRange) {
        const range = Math.abs(signalRange.max.value - signalRange.min.value);
        if (range === 0) return;

        signalRange.min.width = Math.abs(signalRange.min.value);
        signalRange.max.width = Math.abs(signalRange.max.value);

        const signalBar = this.element?.querySelector('.signal-bar');
        if (!signalBar) return;
        this.element.querySelector('.signal-bar').style.background = `linear-gradient(to right, 
                                                      rgba(255, 255, 0, 0) 0%,
                                                      rgba(255, 255, 0, 0) ${signalRange.min.width}%,
                                                      var(--color-chart-wifi-interference-medium-1) ${signalRange.min.width}%,
                                                      var(--color-chart-wifi-interference-medium-5) ${signalRange.max.width}%,
                                                      var(--color-chart-wifi-interference-verybad-1) ${signalRange.max.width}%,
                                                      var(--color-chart-wifi-interference-verybad-5) 100%`;
    }


    // Create DOM element
    createElement() {
        const div = document.createElement('div');
        div.classList.add('wifi-channel-panel');

        div.innerHTML = this.getTemplate();
        this.element = div;

        // Verify critical elements exist
        const chartCanvas = this.element.querySelector('#wifi_inter_chart');
        if (!chartCanvas) {
            console.error('Chart canvas not found in template');
            throw new Error('Chart canvas element missing from template');
        }

        // Populate band options
        const bandPicker = this.element.querySelector("#band_change .segmented_picker");
        if (bandPicker) {
            for (const item of this.supportedBand) {
                const band = item.band.toUpperCase();
                const option = document.createElement('div');
                option.classList.add('segmented_picker_option');
                if (this.showBand === band) {
                    option.classList.add('active');
                }
                option.dataset.value = band;
                option.innerHTML = `<div class="block_filter_name">${this.getBandText(band)}</div>`;
                bandPicker.appendChild(option);
            }
        } else {
            console.error('Band picker element not found');
        }


        this.element.querySelectorAll('.signal-bar-pointer').forEach(pointer => {
            pointer.addEventListener('input', function (e) {
                const targetElement = e.currentTarget;
                const signal = targetElement.dataset.signal;
                if (signal === 'max') {
                    if (targetElement.value < this.signalRange.min.value) {
                        targetElement.value = this.signalRange.min.value;
                    }
                }
                if (signal === 'min') {
                    if (targetElement.value > this.signalRange.max.value) {
                        targetElement.value = this.signalRange.max.value;
                    }
                }
                this.signalRange[signal].value = Number(targetElement.value);
                this.element.querySelector(`.signal-${signal}`).innerText = `${parseInt(this.signalRange[signal].value)} %`;
                this.getSignalWidth(this.signalRange);
                this.updateChartData();

            }.bind(this));
        })
    }

    // Template generation
    getTemplate() {
        return `
            <style>
            /* START: wifi-channel-panel */
            
                .wifi-insight-card-title {
                    min-width: 80px;
                }

                .wifi-channel-panel .segmented_picker_label {
                    font-weight: 400;
                    font-size: 14px;
                    line-height: 16px;
                    display: flex;
                    align-items: center;
                }
                
                .wifi-channel-panel .segmented_picker {
                    display: flex;
                    flex-direction: row;
                    align-items: flex-start;
                    padding: 2px;
                    background-color: var(--switch-menu-bg-color);
                    border-radius: var(--global-radius);
                    width: 100%;
                }
                
                .wifi-channel-panel .segmented_picker_option {
                    font-size: 0.75rem;
                    height: 28px;
                    display: flex;
                    align-items: center;
                    justify-content: center;
                    flex: 1;
                    cursor: pointer;
                    color: var(--switch-menu-option-text-color);
                
                }
                
                .wifi-channel-panel .segmented_picker_option.active {
                    background: var(--switch-menu-option-selected-bg-color);
                    color: var(--switch-menu-option-selected-text-color);
                    border-radius: var(--global-radius);
                    box-shadow: var(--shadow-elevation10);
                }
                
                .wifi-channel-panel .channel_plan {
                    display: flex;
                    flex-direction: row;
                    align-items: center;
                    padding: 8px 16px;
                    gap: 8px;
                }
                
                .wifi-channel-panel .channel_plan .channel_plan_title,
                .wifi-channel-panel .channel_chart_title
                {
                    font-weight: 600;
                    font-size: 16px;
                    line-height: 20px;
                    letter-spacing: -0.08px;
                    vertical-align: middle;
                    text-transform: capitalize;
                }
                
                .wifi-channel-panel .channel_plan .channel_plan_freq {
                    font-weight: 500;
                    font-size: 13px;
                    line-height: 32px;
                    letter-spacing: 0px;
                    color: var(--neutral-gray-7020, rgba(102, 102, 102, 1));
                }
                
                .wifi-channel-panel .timeline {
                    position: relative;
                    overflow-y: auto;
                    min-height: 300px;
                    max-height: 100%;
                    -webkit-overflow-scrolling: touch;
                    padding-left: 20px;
                }
                
                .wifi-channel-panel .timeline .dfs{
                    border: 1px solid var(--neutral-30);
                    background-color: var(--neutral-20);
                    width: fit-content;
                    padding: 5px 10px;
                    border-radius: 8px;
                }
                
                .wifi-channel-panel .timeline .info{
                    border: 1px solid #ffecb5;
                    background-color: #fff3cd;
                    width: fit-content;
                    padding: 5px 10px;
                    border-radius: 8px;
                }
                
                .wifi-channel-panel .entry {
                    position: relative;
                    margin-bottom: 16px;
                    border-radius: 8px;
                    padding: 12px 16px;
                    background: rgba(255, 255, 255, 0.8);
                    border: 1px solid rgba(0, 0, 0, 0.08);
                    box-shadow: 0 1px 3px rgba(0, 0, 0, 0.04);
                    transition: all 0.2s ease;
                }
                
                .wifi-channel-panel .entry.now {
                    background: var(--primary-blue-1040, rgba(236, 245, 255, 1));
                    border: 1px solid var(--primary-blue-5000, rgba(36, 141, 255, 1));
                }
                
                .wifi-channel-panel .entry.now .time {
                    color: var(--body-text-color);
                }
                
                .wifi-channel-panel .entry::before {
                    content: '';
                    position: absolute;
                    left: -16px;
                    top: 50%;
                    transform: translateY(-50%);
                    width: 12px;
                    height: 12px;
                    background-color: #888;
                    border-radius: 50%;
                    border: 3px solid white;
                    box-shadow: 0 0 0 1px rgba(0, 0, 0, 0.1);
                    z-index: 2;
                }
                
                .wifi-channel-panel .entry.now::before {
                    background-color: #00c853; /* 綠色 */
                    box-shadow: 0 0 0 1px rgba(0, 200, 83, 0.3), 0 0 8px rgba(0, 200, 83, 0.4);
                }
                
                .wifi-channel-panel .entry::after {
                    content: "";
                    width: 2px;
                    height: 150%;
                    background: #CCC;
                    position: absolute;
                    top: 50%;
                    left: -11px;
                    z-index: -1;
                }
                
                .wifi-channel-panel .time {
                    display: flex;
                    align-items: center;
                    gap: 5px;
                    font-weight: 400;
                    color: gray;
                }
                
                .wifi-channel-panel .time .now-text {
                    position: absolute;
                    top: 8px;
                    right: 12px;
                    background: #2196f3;
                    color: white;
                    padding: 4px 8px;
                    border-radius: 12px;
                    font-size: 12px;
                    font-weight: 500;
                }
                
                .wifi-channel-panel .channel {
                    margin: 3px 0;
                    font-size: 14px;
                    font-weight: 500;
                }
                
                .wifi-channel-panel .tag {
                    display: inline-block;
                    margin-top: 4px;
                    padding: 2px 6px;
                    background-color: #e0e0e0;
                    border-radius: 4px;
                    font-size: 0.8em;
                }
                
                .wifi-channel-panel i.arrow_icon {
                    display: block;
                    --arrow-icon: url("data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJOYXZpZ2F0aW9uIC8gYXJyb3dfZm9yd2FyZCI+PGcgaWQ9Ikdyb3VwIDM0NzMiPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMF9CT1BCRENETUFFIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNNSAxMkgxOCIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMV9TUE9UT1lLTFJZIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTkgMTEuODU5MkwxMi4xNjg0IDE4LjcxODYiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzJfVEVUSFVQSkhGQyIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTE5IDExLjg1OTRMMTIuMTY4NCA1Ii8+PC9nPjwvZz48L2c+PC9zdmc+");
                    mask-image: var(--arrow-icon);
                    -webkit-mask-image: var(--arrow-icon);
                    width: 24px;
                    height: 24px;
                    background-color: var(--primary-50);
                }
                
                @media (max-width: 992px) {
                    .wifi-channel-panel .timeline {
                        display: inline-flex;
                        border: none;
                        border-left: none;
                        overflow-x: auto;
                        overflow-y: hidden;
                        white-space: nowrap;
                        height: auto;
                        max-height: none;
                        min-height: auto;
                        margin-left: 0;
                        padding-left: 0;
                        width: 100%;
                        -webkit-overflow-scrolling: touch;
                        gap: 12px;
                    }

                    .wifi-channel-panel .entry {
                        width: 180px;
                        min-width: 180px;
                        flex-shrink: 0;
                        margin-right: 0;
                        margin-bottom: 0;
                        border: 1px solid rgba(0, 0, 0, 0.12);
                    }

                    .wifi-channel-panel .entry::before {
                        display: none;
                    }
                    
                    .wifi-channel-panel .entry::after {
                        display: none;
                    }
                }
            
                .wifi-channel-panel .range-options-container {
                    position: relative;
                    display: inline-block;
                }
                .wifi-channel-panel .range-options-btn {
                    display: flex;
                    width: 32px;
                    height: 32px;
                    justify-content: center;
                    align-items: center;
                    background: #f8f9fa;
                    border: 1px solid #dee2e6;
                    border-radius: 50%;
                    cursor: pointer;
                    font-size: 16px;
                    color: #495057;
                    transition: all 0.2s ease;
                }
                .wifi-channel-panel .range-options-btn:hover {
                    background: #e9ecef;
                    border-color: #adb5bd;
                }
                .wifi-channel-panel .more-icon {
                    font-weight: bold;
                    font-size: 18px;
                }
                .wifi-channel-panel .range-options-popup {
                    position: absolute;
                    top: 100%;
                    right: 0;
                    background: white;
                    border: 1px solid #dee2e6;
                    border-radius: 8px;
                    min-width: 300px;
                    box-shadow: 0 4px 12px rgba(0, 0, 0, 0.15);
                    z-index: 1000;
                    margin-top: 4px;
                }
                .wifi-channel-panel .popup-header {
                    display: flex;
                    justify-content: space-between;
                    align-items: center;
                    padding: 12px 16px;
                    border-bottom: 1px solid #dee2e6;
                    font-weight: 600;
                    background: #f8f9fa;
                    border-radius: 8px 8px 0 0;
                }
                .wifi-channel-panel .popup-close {
                    background: none;
                    border: none;
                    font-size: 20px;
                    cursor: pointer;
                    color: #6c757d;
                    padding: 0;
                    width: 24px;
                    height: 24px;
                    display: flex;
                    align-items: center;
                    justify-content: center;
                }
                .wifi-channel-panel .popup-close:hover {
                    color: #495057;
                }
                .wifi-channel-panel .popup-body {
                    padding: 16px;
                }
                .wifi-channel-panel .option-group {
                    margin-bottom: 20px;
                    display: flex;
                    justify-content: space-between;
                    align-items: center;
                }
                .wifi-channel-panel .option-group:last-child {
                    margin-bottom: 0;
                }
                .wifi-channel-panel .option-group label {
                    margin-bottom: 0;
                    font-weight: 500;
                    color: #495057;
                }
                .wifi-channel-panel .option-group i.icon-time {
                    display: block;
                    --svg-icon: url('data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJIdWdlLWljb24vdGltZSBhbmQgZGF0ZS9vdXRsaW5lL3RpbWUtZmFzdCI+PHBhdGggaWQ9IlJlY3RhbmdsZSA0MzdfX19fXzBfMF9aQ1ZTUEJETlJKIiBzdHJva2U9IiMyODMwM0YiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjUiIGQ9Ik0yMiAxMFY2QzIyIDMuNzkwODYgMjAuMjA5MSAyIDE4IDJINkMzLjc5MDg2IDIgMiAzLjc5MDg2IDIgNlYxOEMyIDIwLjIwOTEgMy43OTA4NiAyMiA2IDIySDIyIi8+PHBhdGggaWQ9IlZlY3RvciAxMTcxX19fX18wXzFfQ0JFWE9FU1RDTiIgc3Ryb2tlPSIjMjgzMDNGIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS41IiBkPSJNMTIgNlYxMC41TTEwLjUgMTMuNUw5IDE1Ii8+PHBhdGggaWQ9IkVsbGlwc2UgNDYwX19fX18wXzJfRFdGT0JVV1BIWSIgc3Ryb2tlPSIjMjgzMDNGIiBzdHJva2Utd2lkdGg9IjEuNSIgZD0iTTEzLjUgMTJDMTMuNSAxMi44Mjg0IDEyLjgyODQgMTMuNSAxMiAxMy41QzExLjE3MTYgMTMuNSAxMC41IDEyLjgyODQgMTAuNSAxMkMxMC41IDExLjE3MTYgMTEuMTcxNiAxMC41IDEyIDEwLjVDMTIuODI4NCAxMC41IDEzLjUgMTEuMTcxNiAxMy41IDEyWiIvPjxwYXRoIGlkPSJWZWN0b3IgMTE2MF9fX19fMF8zX1lFR1FQWEdRTFciIHN0cm9rZT0iIzI4MzAzRiIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuNSIgZD0iTTE2IDE4SDIyIi8+PHBhdGggaWQ9IlZlY3RvciAxMTYxX19fX18wXzRfSUFDQ05RSEJRQiIgc3Ryb2tlPSIjMjgzMDNGIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS41IiBkPSJNMTggMTRMMjIgMTQiLz48L2c+PC9nPjwvc3ZnPg==');
                    mask-image: var(--svg-icon);
                    -webkit-mask-image: var(--svg-icon);
                    width: 24px;
                    height: 24px;
                    background-color: var(--body-text-color);
                }
                
                .wifi-channel-panel .interference-status-text {
                    border: 1px solid #28a745;
                    padding: 4px 8px;
                    border-radius: 4px;
                    background: linear-gradient(90deg, #d4edda 0%, #c3e6cb 50%, #d4edda 100%);
                    background-size: 200% 100%;
                    color: #155724;
                    font-size: 12px;
                    font-weight: 500;
                    position: relative;
                    overflow: hidden;
                }
                
                .wifi-channel-panel .interference-status-text::before {
                    content: '';
                    position: absolute;
                    top: 0;
                    left: -100%;
                    width: 100%;
                    height: 100%;
                    background: linear-gradient(90deg, transparent, rgba(255,255,255,0.4), transparent);
                    animation: interferenceShine 2s ease-in-out infinite;
                }
                
                @keyframes interferenceShine {
                    0% {
                        left: -100%;
                    }
                    100% {
                        left: 100%;
                    }
                }
                
                .wifi-channel-panel .status-dot {
                    display: inline-block;
                    width: 8px;
                    height: 8px;
                    border-radius: 50%;
                    background-color: #28a745;
                    margin-right: 6px;
                    animation: statusPulse 1.5s ease-in-out infinite;
                }
                
                @keyframes statusPulse {
                    0%, 100% {
                        opacity: 1;
                        transform: scale(1);
                    }
                    50% {
                        opacity: 0.5;
                        transform: scale(1.2);
                    }
                }                
                .interference-status-verygood {
                    width: 8px;
                    height: 8px;
                    border-radius: 2px;
                    border: 1px solid var(--neutral-60);
                    background: var(--color-chart-wifi-interference-verygood);
                }
                .interference-status-good {
                    width: 8px;
                    height: 8px;
                    border-radius: 2px;
                    background: var(--color-chart-wifi-interference-good);
                }
                .interference-status-medium {
                    width: 8px;
                    height: 8px;
                    border-radius: 2px;
                    border: 1px solid var(--neutral-60);
                    background: var(--color-chart-wifi-interference-medium);
                }
                .interference-status-bad {
                    width: 8px;
                    height: 8px;
                    border-radius: 2px;
                    background: var(--color-chart-wifi-interference-bad);
                }
                .interference-status-verybad {
                    width: 8px;
                    height: 8px;
                    border-radius: 2px;
                    border: 1px solid var(--neutral-60);
                    background: var(--color-chart-wifi-interference-verybad);
                }
                
                .channel_scan_none {
                    width: 20px;
                    height: 12px;
                    background: rgba(220, 220, 220, 0.5);
                    border: 1px solid var(--neutral-40);
                    border-radius: 4px;
                }
                
                .range-form {
                    min-width: 200px;
                    width: 100%;
                }
                
                .signal-bar {
                    position: relative;
                    width: 100%;
                    height: 8px;
                    border-radius: 8px;
                    border: 1px solid var(--neutral-40);
                    margin: 4px 0;
                }
                
                .signal-bar-point {
                    position: absolute;
                    top: 50%;
                    transform: translate(-50%, -50%);
                    transition: left 0.4s;
                    box-sizing: border-box;
                    appearance: none;
                    cursor: pointer;
                    pointer-events: auto;
                    border: 2px solid rgb(255, 255, 255);
                    border-radius: 8px;
                    box-shadow: rgba(33, 35, 39, 0.08) 0px 8px 24px 0px, rgba(33, 35, 39, 0.08) 0px 0px 1px 0px;
                    width: 10px;
                    height: 10px;
                    background-color: rgb(46, 163, 80);
                }
                
                input[type="range" i] {
                    cursor: default;
                    color: light-dark(rgb(16, 16, 16), rgb(255, 255, 255));
                    padding: initial;
                    border: initial;
                }
                
                .signal-bar-pointer {
                    position: absolute;
                    appearance: none;
                    width: 100%;
                    height: 100%;
                    outline: none;
                    pointer-events: none;
                    background: transparent;
                }
                
                .signal-bar-pointer::-webkit-slider-thumb {
                    pointer-events: auto;
                    -webkit-appearance: none;
                    appearance: none;
                    width: 8px;
                    height: 14px;
                    background: #ff5733; /* 指標顏色 */
                    border: 1px solid #ffffff; /* 指標邊框 */
                    border-radius: 8px;
                    cursor: pointer;
                }
                
                .signal-bar-pointer[data-signal="max"]::-webkit-slider-thumb {
                    background: rgb(192, 46, 50);
                }
                
                .signal-bar-pointer[data-signal="min"]::-webkit-slider-thumb {
                    background: var(--color-chart-wifi-interference-medium-5);
                }
                
                .legend-btn {
                    color: var(--primary-text-btn-disable);
                    display: flex;
                    height: 32px;
                    padding: 6px 16px;
                    justify-content: center;
                    align-items: center;
                    gap: 8px;
                    border-radius: 2px;
                    border: 1px solid var(--neutral-50, #999);
                    cursor: pointer;
                    user-select: none;
                    min-width: 100px;
                }
                
                .legend-btn:hover {
                    background-color: var(--primary-10);
                }
                
                .legend-btn > *{
                    pointer-events: none;
                }
                
                .legend-btn.active {
                    color: var(--primary-text-btn-normal);
                    border: 1px solid var(--primary-30, #666);
                }
                
                .legend-btn.active:hover {
                    background-color: var(--primary-20);
                }
                
                .legend-btn.active::before {
                    display: block;
                    content: '✓';
                    font-size: 16px;
                }
            </style>
            <div class="card pt-2 h-100">
                <div class="card-header d-flex align-items-center justify-content-between">
                    <div class="wifi-insight-card-title">WiFi Insight</div>
                    <div id="band_change" style="min-width: 180px;">
                        <div class="segmented_picker"></div>
                    </div>
                    <div class="d-flex align-items-center gap-1">
                        <div id="interferenceStatusText" class="interference-status-text d-none" >
                            <span class="status-dot"></span>
                            Interference detecting...
                        </div>
                        <div class="range-options-container">
                        <button class="range-options-btn" id="range_options_btn">
                            <i role="icon" class="icon-size-18 icon-more-horiz"></i>
                        </button>
                        <div class="range-options-popup" id="range_options_popup" style="display: none;">
                            <div class="popup-body">
                                <div class="option-group">
                                    <label class="me-2" aria-label="Time Range"><i class="icon icon-time"></i></label>
                                    <div id="range_change" class="segmented_picker">
                                        <div class="segmented_picker_option" data-value="10m">
                                            <div class="block_filter_name">10m</div>
                                        </div>
                                        <div class="segmented_picker_option active" data-value="1H">
                                            <div class="block_filter_name">1H</div>
                                        </div>
                                        <div class="segmented_picker_option" data-value="12H">
                                            <div class="block_filter_name">12H</div>
                                        </div>
                                        <div class="segmented_picker_option" data-value="1D">
                                            <div class="block_filter_name">1D</div>
                                        </div>
                                    </div>
                                </div>
                                <div class="option-group">
                                    <label>Interference Detect</label>
                                    <div id="interference_toggle" role="button" tabindex="1" class="toggle-button with-text ${this.interferenceEnabled ? 'active' : ''}">
                                        <div class="toggle-button-handle"></div>
                                    </div>
                                </div>
                                <div class="option-group">
                                    <label>Auto Refresh</label>
                                    <div id="auto_refresh_toggle" role="button" tabindex="1" class="toggle-button with-text">
                                        <div class="toggle-button-handle"></div>
                                    </div>
                                </div>
                            </div>
                        </div>
                    </div>
                    </div>
                </div>
                <div class="card-body p-0 ps-2 pe-2" style="background: rgba(239, 239, 239, 0.4);">
                    <div class="row">
                        <div class="col-12 col-lg-3">
                            <div class="channel_plan">
                                <div class="channel_plan_title">${this.getBandText(this.showBand)}</div>
                                <div class="channel_plan_freq">${this.getBandFreqText(this.showBand)}</div>
                            </div>
                            <div class="timeline">
                            </div>
                        </div>
                        <div class="col-12 col-lg-9 mt-1 mb-1">
                            <div class="row">
                                <div class="col-12 px-3 py-2">
                                    <div class="channel_chart_title">In Channel Interference</div>
                                </div>
                                <div class="w-100 d-flex gap-2 align-items-center justify-content-center justify-content-lg-end px-3" id="txop_legend">
                                    </div>
                                <div style="height: 200px;">
                                    <canvas id="txop_chart"></canvas>
                                </div>
                                <div class="col-12 px-3 py-2">
                                    <div id="bar-chart-title" class="channel_chart_title">Spectrum Scan Interference</div>
                                </div>
                                <div class="col-12 px-3">
                                    <div class="row">
                                        <div class="col-12 col-lg-4 d-flex align-items-center mb-1">
                                            <div class="d-flex gap-2 align-items-center w-100" id="interference_info">
                                                <span class="h-100">Interference</span>
                                                <div class="d-flex align-items-center" style="width: inherit">
                                                    <div class="range-form">
                                                        <div class="signal-bar" style="background: linear-gradient(to right, 
                                                              rgba(255, 255, 0, 0) 0%,
                                                              rgba(255, 255, 0, 0) 29%,
                                                              var(--color-chart-wifi-interference-medium-1) 30%,
                                                              var(--color-chart-wifi-interference-medium-5) 69%,
                                                              var(--color-chart-wifi-interference-verybad-1) 70%,
                                                              var(--color-chart-wifi-interference-verybad-5) 100%);">
                                                            <input class="signal-bar-pointer" type="range" data-signal="min" min="1" max="99" step="1" value="30">
                                                            <input class="signal-bar-pointer" type="range" data-signal="max" min="1" max="99" step="1" value="70">
                                                        </div>
                                                        <div class="d-flex justify-content-between signal-info">
                                                            <span class="signal-zero">0%</span>
                                                            <span class="signal-min">30%</span>
                                                            <span class="signal-max">70%</span>
                                                            <span class="signal-hundred">100%</span>
                                                        </div>
                                                    </div>
                                                </div>
                                            </div>
                                            <div class="d-flex align-items-center d-none" id="channel_chart_info">
                                                <span class="h-100">
                                                    <div class="alert alert-info p-1 m-0" role="alert">
                                                      Spectrum interference scanning covers only 2.4GHz and 5GHz bands.
                                                    </div>
                                                </span>
                                            </div>
                                        </div>
                                        <div class="col-12 col-lg-8 d-flex align-items-center justify-content-center justify-content-lg-end gap-2 mb-1" id="interference_legend">
                                        </div>
                                    </div>
                                </div>
                                <div style="height: 400px;">
                                    <canvas id="wifi_inter_chart"></canvas>
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        `;
    }

    // Setup event handlers
    setupEventHandlers() {
        // Band change event handlers
        const bandOptions = this.element.querySelectorAll("#band_change .segmented_picker_option");
        for (const item of bandOptions) {
            this.addEventHandler(item, 'click', () => {
                for (const option of bandOptions) {
                    option.classList.remove('active');
                }
                item.classList.add('active');
                this.showBand = item.dataset.value;

                if (this.showBand.startsWith("6G")) {
                    this.getCachedElement("#channel_chart_info").classList.remove("d-none");
                    this.getCachedElement("#interference_info").classList.add("d-none");
                    this.getCachedElement("#bar-chart-title").innerText = "Channel In Use";
                } else {
                    this.getCachedElement("#channel_chart_info").classList.add("d-none");
                    this.getCachedElement("#interference_info").classList.remove("d-none");
                    this.getCachedElement("#bar-chart-title").innerText = "Spectrum Scan Interference";
                }

                const titleElement = this.getCachedElement(".channel_plan_title");
                const freqElement = this.getCachedElement(".channel_plan_freq");

                if (titleElement) titleElement.innerHTML = this.getBandText(this.showBand);
                if (freqElement) freqElement.innerHTML = this.getBandFreqText(this.showBand);

                this.reGenChart();
            });
        }

        // Range options popup handlers
        const rangeOptionsBtn = this.getCachedElement("#range_options_btn");
        const rangeOptionsPopup = this.getCachedElement("#range_options_popup");
        const popupClose = this.getCachedElement("#popup_close");

        if (rangeOptionsBtn) {
            this.addEventHandler(rangeOptionsBtn, 'click', (e) => {
                e.stopPropagation();
                if (rangeOptionsPopup) {
                    rangeOptionsPopup.style.display = rangeOptionsPopup.style.display === 'none' ? 'block' : 'none';
                }
            });
        }

        if (popupClose) {
            this.addEventHandler(popupClose, 'click', () => {
                if (rangeOptionsPopup) {
                    rangeOptionsPopup.style.display = 'none';
                }
            });
        }

        // Close popup when clicking outside
        this.addEventHandler(document, 'click', (e) => {
            if (rangeOptionsPopup && !rangeOptionsPopup.contains(e.target) && !rangeOptionsBtn?.contains(e.target)) {
                rangeOptionsPopup.style.display = 'none';
            }
        });

        // Range change event handlers
        const rangeOptions = this.element.querySelectorAll("#range_change .segmented_picker_option");
        for (const item of rangeOptions) {
            this.addEventHandler(item, 'click', () => {
                for (const option of rangeOptions) {
                    option.classList.remove('active');
                }
                item.classList.add('active');

                // Store current time range selection
                this.currentTimeRange = item.dataset.value;

                this.updateTimeRange(item.dataset.value);
                this.reGenChart();
            });
        }

        // Auto refresh toggle handler
        const interferenceToggle = this.getCachedElement("#interference_toggle");
        if (interferenceToggle) {
            this.addEventHandler(interferenceToggle, 'click', (e) => {
                e.target.classList.toggle("active");
                this.interferenceEnabled = e.target.classList.contains("active");
                if (this.interferenceEnabled) {
                    this.getCachedElement("#interferenceStatusText").classList.add('d-md-inline');
                } else {
                    this.getCachedElement("#interferenceStatusText").classList.remove('d-md-inline');
                }
                httpApi.nvramSet({
                    "airiq_enable": this.interferenceEnabled ? "1" : "0",
                    "action_mode": "apply",
                });
            });
        }

        // Auto refresh toggle handler
        const autoRefreshToggle = this.getCachedElement("#auto_refresh_toggle");
        if (autoRefreshToggle) {
            // Set initial state from loaded preference
            if (this.autoRefresh) {
                autoRefreshToggle.classList.add("active");
            }

            this.addEventHandler(autoRefreshToggle, 'click', (e) => {
                e.target.classList.toggle("active");
                this.autoRefresh = e.target.classList.contains("active");

                // Save state to localStorage
                this.saveAutoRefreshState(this.autoRefresh);

                if (this.autoRefresh) {
                    this.startAutoRefresh();
                } else {
                    this.stopAutoRefresh();
                }
            });
        }
    }

    // Update time range based on selection
    updateTimeRange(value, preserveToggleStates = false) {
        const timeRanges = {
            "10m": {
                start: new Date(this.nowTime - 1 * 10 * 60 * 1000),
                end: new Date(this.nowTime - (-1.1 * 60 * 1000)),
            },
            "1H": {
                start: new Date(this.nowTime - 1 * 60 * 60 * 1000),
                end: new Date(this.nowTime - (-3 * 60 * 1000)),
            },
            "12H": {
                start: new Date(this.nowTime - 12 * 60 * 60 * 1000),
                end: new Date(this.nowTime - (-30 * 60 * 1000)),
            },
            "1D": {
                start: new Date(this.nowTime - 24 * 60 * 60 * 1000),
                end: new Date(this.nowTime - (-60 * 60 * 1000)),
            }
        };

        const range = timeRanges[value];
        if (range) {
            this.chartStartTime = range.start;
            this.chartEndTime = range.end;
        }
    }

    // Helper method to find rectanglePlugin in chart instance
    findRectanglePlugin() {
        if (!this.chartInstance || !this.chartInstance.config || !this.chartInstance.config.plugins) {
            return null;
        }

        // Find the rectanglePlugin from the plugins array
        const plugins = this.chartInstance.config.plugins;
        for (const plugin of plugins) {
            if (plugin && plugin.id === 'wifi-rectangles') {
                return plugin;
            }
        }
        return null;
    }

    // Create custom checkboxes in chart_info area
    generateHTMLLegend() {
        const chartInfoDiv = this.getCachedElement('#interference_legend');
        if (!chartInfoDiv) {
            console.warn('chart_info element not found');
            return;
        }

        // Clear existing content
        chartInfoDiv.innerHTML = '';

        // Get the rectanglePlugin instance
        const rectanglePlugin = this.findRectanglePlugin();
        if (!rectanglePlugin) {
            console.warn('rectanglePlugin not found');
            return;
        }

        const self = this;

        const channelLegendButton = this.createLegendButton({
            id: 'channel-in-use-legend-btn',
            label: 'Channel In Use',
            checked: rectanglePlugin.legendStates.channel,
            fill: 'rgba(164, 210, 255, 0.8)',
            border: '1px solid rgba(36, 141, 255, 1)',
            icon: true,
            onChange: (checked) => {
                rectanglePlugin.legendStates.channel = checked;
                self.channelBarShow = checked;
                this.chartInstance.update();
            }
        });
        chartInfoDiv.appendChild(channelLegendButton);

        if (this.showBand.startsWith("5G")) {
            const dfsLegendButton = this.createLegendButton({
                id: 'dfs-channel-legend-btn',
                label: 'DFS',
                checked: rectanglePlugin.legendStates.dfs,
                fill: (ctx) => this.createDiagonalPattern(ctx, '#666666', 2, 1),
                border: '1px solid #666666',
                icon: true,
                onChange: (checked) => {
                    rectanglePlugin.legendStates.dfs = checked;
                    self.dfsShow = checked;
                    this.chartInstance.update();
                }
            });
            chartInfoDiv.appendChild(dfsLegendButton);
        }

        if (!this.showBand.startsWith("6G")) {

            const otherWifiLegendButton = this.createLegendButton({
                id: 'otherwifi-legend-btn',
                label: 'Others WiFi Interference',
                checked: rectanglePlugin.legendStates.wifi,
                icon: false,
                onChange: (checked) => {
                    rectanglePlugin.legendStates.wifi = checked;
                    self.wifiInterShow = checked;
                    this.chartInstance.update();
                }
            });
            chartInfoDiv.appendChild(otherWifiLegendButton);

            const nonWifiLegendButton = this.createLegendButton({
                id: 'nonwifi-legend-btn',
                label: 'NonWiFi Interference',
                checked: rectanglePlugin.legendStates.nonwifi,
                icon: false,
                onChange: (checked) => {
                    rectanglePlugin.legendStates.nonwifi = checked;
                    self.nonwifiInterShow = checked;
                    this.chartInstance.update();
                }
            });
            chartInfoDiv.appendChild(nonWifiLegendButton);

        }
    }

    generateTxopHTMLLegend() {
        const chartInfoDiv = this.getCachedElement('#txop_legend');
        if (!chartInfoDiv) {
            console.warn('chart_info element not found');
            return;
        }

        // Clear existing content
        chartInfoDiv.innerHTML = '';

        // Get the rectanglePlugin instance
        const rectanglePlugin = this.findRectanglePlugin();
        if (!rectanglePlugin) {
            console.warn('rectanglePlugin not found');
            return;
        }

        const self = this;


        const otherWifiLegendButton = this.createLegendButton({
            id: 'otherwifi-legend-btn',
            label: 'Others WiFi Interference',
            checked: this.txopChartInstance.data.datasets.find(dataset => dataset.label === 'Others WiFi Interference')?.hidden === false,
            icon: true,
            fill: 'rgba(238,58,62,0.1)',
            border: '1px solid #ee3a3e',
            onChange: (checked) => {
                this.txopChartInstance.data.datasets.forEach(dataset => {
                    if (dataset.label === 'Others WiFi Interference') {
                        dataset.hidden = !checked;
                    }
                });
                this.txopChartInstance.update();
            }
        });

        chartInfoDiv.appendChild(otherWifiLegendButton);
        if (!this.showBand.startsWith("6G")) {
            const nonWifiLegendButton = this.createLegendButton({
                id: 'nonwifi-legend-btn',
                label: 'NonWiFi Interference',
                checked: this.txopChartInstance.data.datasets.find(dataset => dataset.label === 'NonWiFi Interference')?.hidden === false,
                icon: true,
                fill: 'rgba(245,176,124, 0.1)',
                border: '1px solid #f5b07c',
                onChange: (checked) => {
                    this.txopChartInstance.data.datasets.forEach(dataset => {
                        if (dataset.label === 'NonWiFi Interference') {
                            dataset.hidden = !checked;
                        }
                    });
                    this.txopChartInstance.update();
                }
            });
            chartInfoDiv.appendChild(nonWifiLegendButton);

        }

        const txopLegendButton = this.createLegendButton({
            id: 'txop-legend-btn',
            label: 'TXOP',
            checked: this.txopChartInstance.data.datasets.find(dataset => dataset.label === 'TXOP')?.hidden === false,
            fill: 'rgba(124,162,245,0.1)',
            border: '1px solid #7ca2f5',
            showbox: true,
            onChange: (checked) => {
                this.txopChartInstance.data.datasets.forEach(dataset => {
                    if (dataset.label === 'TXOP') {
                        dataset.hidden = !checked;
                    }
                });
                this.txopChartInstance.update();
            }
        });
        // chartInfoDiv.appendChild(txopLegendButton);
    }

    // Create a legend button element
    createLegendButton({id, label, checked, icon = true, fill, border, onChange}) {
        const container = document.createElement('div');
        container.className = 'legend-btn';
        if (checked) {
            container.classList.add('active');
        }

        const labelEl = document.createElement('label');
        labelEl.htmlFor = id;
        labelEl.className = 'form-check-label d-flex align-items-center gap-1';

        const labelText = document.createElement('span');
        labelText.textContent = label;
        labelText.style.fontSize = '12px';
        labelEl.appendChild(labelText);

        // Add color indicator
        if (icon) {
            const colorIndicator = document.createElement('div');
            colorIndicator.style.width = '12px';
            colorIndicator.style.height = '12px';
            colorIndicator.style.borderRadius = '0';
            colorIndicator.style.border = border;
            if (typeof fill === 'function') {
                // 創建小 canvas 來顯示圖案
                const canvas = document.createElement('canvas');
                canvas.width = 12;
                canvas.height = 12;
                const ctx = canvas.getContext('2d');

                // 執行函數獲得圖案
                const pattern = fill(ctx);

                // 用圖案填滿整個 canvas
                ctx.fillStyle = pattern;
                ctx.fillRect(0, 0, 12, 12);

                // 將 canvas 轉為圖片並設為背景
                colorIndicator.style.backgroundImage = `url(${canvas.toDataURL()})`;
                colorIndicator.style.backgroundSize = 'cover';
            } else {
                // 如果是顏色字串，直接設為背景色
                colorIndicator.style.backgroundColor = fill;
            }
            labelEl.appendChild(colorIndicator);
        }

        container.appendChild(labelEl);

        container.addEventListener('click', () => {
            checked = !checked;
            if (checked) {
                container.classList.add('active');
            } else {
                container.classList.remove('active');
            }
            onChange(checked);
        });

        return container;
    }

    // Create interference button that cycles through states
    createInterferenceButton(rectanglePlugin) {
        const self = this;
        const container = document.createElement('div');
        container.className = 'd-flex align-items-center gap-2';

        const button = document.createElement('button');
        button.className = 'btn btn-sm text-dark d-flex align-items-center gap-1';
        button.style.fontSize = '12px';
        button.style.padding = '4px 8px';

        // Function to update button appearance based on current state
        const updateButton = () => {
            const state = rectanglePlugin.legendStates.interferenceState;
            let text, color;

            switch (state) {
                case 0: // None
                    text = 'No Interference';
                    color = 'rgba(200, 200, 200, 0.8)';
                    break;
                case 1: // WiFi only
                    text = 'Others WiFi Interference';
                    color = '#4CAF50';
                    break;
                case 2: // NonWiFi only
                    text = 'NonWiFi Interference';
                    color = '#FF9800';
                    break;
                case 3: // Both
                    text = 'Total WiFi Interference';
                    color = '#F44336';
                    break;
                default:
                    text = 'No Interference';
                    color = 'rgba(200, 200, 200, 0.8)';
            }

            // Update button content
            button.innerHTML = '';

            const colorIndicator = document.createElement('div');
            colorIndicator.style.width = '12px';
            colorIndicator.style.height = '4px';
            colorIndicator.style.backgroundColor = color;
            colorIndicator.style.borderRadius = '2px';

            const buttonText = document.createElement('span');
            buttonText.textContent = text;

            button.appendChild(colorIndicator);
            button.appendChild(buttonText);
        };

        // Initial button setup
        updateButton();

        // Add click handler to cycle through states
        button.addEventListener('click', () => {
            // Cycle through interference states: None -> WiFi -> NonWiFi -> Both -> None
            rectanglePlugin.legendStates.interferenceState = (rectanglePlugin.legendStates.interferenceState + 1) % 4;

            // Update individual wifi/nonwifi states based on combined state
            switch (rectanglePlugin.legendStates.interferenceState) {
                case 0: // None
                    rectanglePlugin.legendStates.wifi = false;
                    rectanglePlugin.legendStates.nonwifi = false;
                    self.wifiInterShow = false;
                    self.nonwifiInterShow = false;
                    break;
                case 1: // WiFi only
                    rectanglePlugin.legendStates.wifi = true;
                    rectanglePlugin.legendStates.nonwifi = false;
                    self.wifiInterShow = true;
                    self.nonwifiInterShow = false;
                    break;
                case 2: // NonWiFi only
                    rectanglePlugin.legendStates.wifi = false;
                    rectanglePlugin.legendStates.nonwifi = true;
                    self.wifiInterShow = false;
                    self.nonwifiInterShow = true;
                    break;
                case 3: // Both
                    rectanglePlugin.legendStates.wifi = true;
                    rectanglePlugin.legendStates.nonwifi = true;
                    self.wifiInterShow = true;
                    self.nonwifiInterShow = true;
                    break;
            }

            // Update button appearance
            updateButton();

            // Update chart
            self.chartInstance.update();
        });

        container.appendChild(button);
        return container;
    }

    // Auto refresh methods
    startAutoRefresh() {
        if (this.refreshInterval) {
            clearInterval(this.refreshInterval);
        }

        // Refresh every 30 seconds
        this.refreshInterval = setInterval(async () => {
            try {
                // Update current time
                this.nowTime = new Date();

                // Update chart time range to maintain sliding window
                this.updateChartTimeRange();

                await this.fetchChannelDB(this.mac);

                // Only update chart data, preserving visual elements like "Channel In Use"
                await this.updateChartData();

            } catch (error) {
                console.error('Auto refresh failed:', error);

                // Fallback to full chart regeneration on error
                try {
                    await this.genChart();
                } catch (fallbackError) {
                    console.error('Fallback chart regeneration failed:', fallbackError);
                }
            }
        }, 5000);
    }

    stopAutoRefresh() {
        if (this.refreshInterval) {
            clearInterval(this.refreshInterval);
            this.refreshInterval = null;
        }
    }

    // Update chart time range for sliding window effect
    updateChartTimeRange() {
        // Use stored current time range selection and preserve toggle states
        this.updateTimeRange(this.currentTimeRange, true);
    }

    // Update chart data without destroying visual elements like "Channel In Use"
    async updateChartData() {
        try {
            // Check if there's an existing Chart.js instance and destroy it properly
            const chartCanvas = this.getCachedElement('#wifi_inter_chart');
            if (!chartCanvas) {
                console.warn('Chart canvas not found, falling back to full regeneration');
                await this.genChart();
                return;
            }

            // Destroy existing Chart.js instance if it exists
            if (this.chartInstance) {
                try {
                    this.chartInstance.destroy();
                    this.chartInstance = null;
                } catch (destroyError) {
                    console.warn('Error destroying chart instance:', destroyError);
                }
            }

            // Check for any Chart.js instances attached to this canvas
            if (window.Chart && window.Chart.getChart) {
                const existingChart = window.Chart.getChart(chartCanvas);
                if (existingChart) {
                    existingChart.destroy();
                }
            }

            // Clear the canvas manually to ensure clean state
            const ctx = chartCanvas.getContext('2d');
            if (ctx) {
                ctx.clearRect(0, 0, chartCanvas.width, chartCanvas.height);
            }

            // Regenerate the chart with clean canvas
            await this.genChart();
        } catch (error) {
            console.error('Failed to update chart data:', error);
            // Fallback to full chart regeneration only on critical errors
            await this.genChart();
        }
    }

    async init() {
        if (this.isInitialized) return;

        // Ensure DOM element is ready
        if (!this.element) {
            console.error('DOM element not ready for initialization');
            return;
        }

        // 顯示loading狀態
        this.showLoadingState();

        try {
            await this.fetchChannelData(this.mac);
            await this.genChart();
            this.isInitialized = true;

            // Start auto refresh if enabled
            if (this.autoRefresh) {
                this.startAutoRefresh();
            }
        } catch (error) {
            console.error('Failed to initialize WiFi interference chart:', error);
            this.showError('Failed to load chart data');
        } finally {
            this.hideLoadingState();
        }
    }

    // 顯示loading狀態
    showLoadingState() {
        const chartCanvas = this.getCachedElement('#wifi_inter_chart');
        if (chartCanvas && chartCanvas.getContext) {
            try {
                const ctx = chartCanvas.getContext('2d');
                if (ctx) {
                    ctx.clearRect(0, 0, chartCanvas.width, chartCanvas.height);
                    ctx.font = '16px sans-serif';
                    ctx.fillStyle = '#666';
                    ctx.textAlign = 'center';
                    ctx.textBaseline = 'middle';
                    ctx.fillText('Loading WiFi interference chart...', chartCanvas.width / 2, chartCanvas.height / 2);
                }
            } catch (error) {
                console.warn('Error showing loading state:', error);
            }
        }
    }

    // 隱藏loading狀態
    hideLoadingState() {
        // loading會被chart渲染覆蓋，無需特別處理
    }

    // Show error message
    showError(message) {
        console.error('WiFiInterferenceChart Error:', message);

        const chartCanvas = this.getCachedElement('#wifi_inter_chart');
        if (chartCanvas && chartCanvas.getContext) {
            try {
                const ctx = chartCanvas.getContext('2d');
                if (ctx) {
                    ctx.clearRect(0, 0, chartCanvas.width, chartCanvas.height);
                    ctx.font = '16px sans-serif';
                    ctx.fillStyle = '#ff0000';
                    ctx.textAlign = 'center';
                    ctx.textBaseline = 'middle';
                    ctx.fillText(message, chartCanvas.width / 2, chartCanvas.height / 2);
                }
            } catch (error) {
                console.error('Failed to show error on canvas:', error);
            }
        }
    }

    getChannelFrequencyRange(band, controlChannel, bandwidth) {
        // 基礎頻率
        let baseFreq;
        if (band.startsWith("2G")) {
            baseFreq = 2407;
        } else if (band.startsWith("5G")) {
            baseFreq = 5000;
        } else if (band.startsWith("6G")) {
            baseFreq = 5950;
        } else {
            return {error: "Not Support"};
        }

        // 計算控制頻道的中心頻率
        let controlFreq;
        if (band.startsWith("2G")) {
            controlFreq = controlChannel === 14 ? 2484 : 2412 + (controlChannel - 1) * 5;
        } else {
            controlFreq = baseFreq + controlChannel * 5;
        }

        // 20MHz - 直接返回
        if (bandwidth === 20) {
            return {
                band,
                controlChannel,
                centerFreqMHz: controlFreq,
                startMHz: controlFreq - 10,
                endMHz: controlFreq + 10,
                bandwidthMHz: 20
            };
        }

        // 2.4GHz 40MHz 簡化處理
        if (band.startsWith("2G")) {
            const offset = controlChannel <= 7 ? 10 : -10;
            const centerFreq = controlFreq + offset;
            return {
                band,
                controlChannel,
                centerFreqMHz: centerFreq,
                startMHz: centerFreq - 20,
                endMHz: centerFreq + 20,
                bandwidthMHz: 40
            };
        }

        // 5GHz 和 6GHz 的 40MHz 及以上
        const channelsPerGroup = bandwidth / 20;

        let centerChannel;

        if (band.startsWith("5G")) {
            // 5GHz 頻段定義 (不連續)
            const UNII_BANDS = [
                {start: 36, end: 48},   // UNII-1
                {start: 52, end: 64},   // UNII-2
                {start: 100, end: 144}, // UNII-2e
                {start: 149, end: 165}  // UNII-3
            ];

            // 找到控制頻道所屬的頻段
            const band = UNII_BANDS.find(b => controlChannel >= b.start && controlChannel <= b.end);
            if (!band) {
                return {error: `無效的 5GHz 頻道: ${controlChannel}`};
            }

            // 在該頻段內計算組起始
            const channelStep = 4;
            const offsetInBand = controlChannel - band.start;
            const groupStartOffset = Math.floor(offsetInBand / (channelsPerGroup * channelStep)) * (channelsPerGroup * channelStep);
            const groupStart = band.start + groupStartOffset;

            // 計算中心頻道
            centerChannel = groupStart + (channelsPerGroup - 1) * channelStep / 2;

            // 檢查是否超出頻段範圍
            const groupEnd = groupStart + (channelsPerGroup - 1) * channelStep;
            if (groupEnd > band.end) {
                return {error: `頻道 ${controlChannel} 無法組成 ${bandwidth}MHz (超出頻段範圍)`};
            }

        } else if (band.startsWith("6G")) {
            // 6GHz 頻道是連續的
            const channelStep = 4;
            const groupStart = Math.floor((controlChannel - 1) / (channelsPerGroup * channelStep)) * (channelsPerGroup * channelStep) + 1;
            centerChannel = groupStart + (channelsPerGroup - 1) * channelStep / 2;
        }

        const centerFreq = baseFreq + centerChannel * 5;
        const halfBandwidth = bandwidth / 2;

        return {
            band,
            controlChannel,
            centerChannel,
            centerFreqMHz: centerFreq,
            startMHz: centerFreq - halfBandwidth,
            endMHz: centerFreq + halfBandwidth,
            bandwidthMHz: bandwidth
        };
    }


    // Utility function for drawing rounded rectangles
    drawRoundedRect(ctx, x, y, width, height, leftRadius, rightRadius) {
        const rL = Math.min(leftRadius, width / 2, height / 2);
        const rR = Math.min(rightRadius, width / 2, height / 2);

        ctx.beginPath();
        ctx.moveTo(x + rL, y);

        // Top edge
        ctx.lineTo(x + width - rR, y);
        ctx.quadraticCurveTo(x + width, y, x + width, y + rR);

        // Right edge
        ctx.lineTo(x + width, y + height - rR);
        ctx.quadraticCurveTo(x + width, y + height, x + width - rR, y + height);

        // Bottom edge
        ctx.lineTo(x + rL, y + height);
        ctx.quadraticCurveTo(x, y + height, x, y + height - rL);

        // Left edge
        ctx.lineTo(x, y + rL);
        ctx.quadraticCurveTo(x, y, x + rL, y);

        ctx.closePath();
    }

    bindMousemoveEvents() {
        const wifi_inter_chart = this.getCachedElement('#wifi_inter_chart');
        if (!wifi_inter_chart) return;

        // Remove existing mousemove listeners to prevent duplicates
        const existingHandler = wifi_inter_chart._mousemoveHandler;
        if (existingHandler) {
            wifi_inter_chart.removeEventListener('mousemove', existingHandler);
        }

        // Remove existing mouseleave listeners to prevent duplicates
        const existingLeaveHandler = wifi_inter_chart._mouseleaveHandler;
        if (existingLeaveHandler) {
            wifi_inter_chart.removeEventListener('mouseleave', existingLeaveHandler);
        }

        const self = this;
        const interferenceData = this.interferenceData;
        const barData = this.barData;
        const dfsChartData = this.dfsChartData;
        const showBand = this.showBand;
        const yOffsets = this.yOffsets;

        // Create tooltip element if it doesn't exist
        let tooltipEl = this.element.querySelector('.chart-tooltip');
        if (!tooltipEl) {
            tooltipEl = document.createElement('div');
            tooltipEl.className = 'chart-tooltip';
            tooltipEl.style.position = 'absolute';
            tooltipEl.style.background = 'rgba(0, 0, 0, 0.8)';
            tooltipEl.style.color = 'white';
            tooltipEl.style.padding = '8px';
            tooltipEl.style.borderRadius = '3px';
            tooltipEl.style.pointerEvents = 'none';
            tooltipEl.style.opacity = '0';
            tooltipEl.style.transition = 'opacity 0.3s ease';
            tooltipEl.style.zIndex = '1000';
            tooltipEl.style.left = '0';
            tooltipEl.style.top = '0';
            this.element.appendChild(tooltipEl);
        }

        // Create new mousemove handler
        const mousemoveHandler = (event) => {
            if (!this.chartInstance || !this.chartInstance.ctx || !this.chartInstance.scales) {
                return;
            }

            // Helper function to position tooltip intelligently
            const positionTooltip = (event, tooltipEl) => {
                const offset = 10;
                const viewportWidth = window.innerWidth;

                // Set content first to get accurate dimensions
                tooltipEl.style.opacity = '1';

                // Get tooltip dimensions after content is set
                const tooltipWidth = tooltipEl.offsetWidth;

                // Check if tooltip would overflow on the right
                if (event.pageX + offset + tooltipWidth > viewportWidth) {
                    // Position to the left of cursor
                    tooltipEl.style.left = (event.pageX - tooltipWidth - offset) + 'px';
                } else {
                    // Position to the right of cursor (default)
                    tooltipEl.style.left = (event.pageX + offset) + 'px';
                }

                tooltipEl.style.top = (event.pageY + offset) + 'px';
            };

            const canvasPosition = Chart.helpers.getRelativePosition(event, this.chartInstance);
            const x = canvasPosition.x;
            const y = canvasPosition.y;

            const {ctx, scales} = this.chartInstance;
            const xScale = scales.x;
            const yScale = scales.y;

            // 確保 scales 和它們的方法存在
            if (!xScale || !yScale || typeof xScale.getPixelForValue !== 'function' || typeof yScale.getPixelForValue !== 'function') {
                console.error('Chart scales are not properly initialized');
                return;
            }

            const unitHeight = Math.abs(yScale.getPixelForValue(1) - yScale.getPixelForValue(0));

            let found = false;
            if (this.rectanglePlugin?.legendStates?.interferenceState !== 0) {
                if (Array.isArray(interferenceData)) {
                    for (const d of interferenceData) {
                        if (self.chartStartTime < d.timeStart) {
                            const xStart = xScale.getPixelForValue(d.timeStart);
                            const xEnd = xScale.getPixelForValue(d.timeEnd);
                            let band = showBand.toUpperCase();
                            if (band === '2G1') {
                                band = '2G';
                            } else if (band === '5G1') {
                                band = '5G';
                            }
                            const channelData = d.channels[band];
                            const max_ch = self.getBandMaxChannel(band);
                            if (channelData) {
                                for (const c of channelData) {
                                    const yCenter = yScale.getPixelForValue(yOffsets[c.channel]?.center);
                                    const barHeight = yOffsets[c.channel]?.height || 4;
                                    let yTop = yCenter - (barHeight / 2) * unitHeight;
                                    let yBottom = yCenter + (barHeight / 2) * unitHeight;
                                    if (c.channel == '1') {
                                        yTop = yCenter - (barHeight / 2) * unitHeight;
                                        yBottom = yCenter + (barHeight * 2.5) * unitHeight;
                                    } else if (c.channel == max_ch) { //max channel
                                        yTop = yCenter - (barHeight * 2) * unitHeight;
                                        yBottom = yCenter + (barHeight / 2) * unitHeight;
                                    }


                                    if (x >= xStart && x <= xEnd && y >= yTop && y <= yBottom) {
                                        const first_wlan_util = c.first_wlan_util || 0;
                                        const first_nonwlan_util = c.first_nonwlan_util || 0;
                                        const first_total_util = first_wlan_util + first_nonwlan_util;
                                        const legendStates = this.rectanglePlugin?.legendStates || {};

                                        let tooltipContent = `
                                            ${new Date(d.timeStart).toLocaleString()} - ${new Date(d.timeEnd).toLocaleString()}<br/>
                                            Channel: ${c.channel}<br/>
                                        `;

                                        if (first_total_util >= this.signalRange.min.value && legendStates.interferenceState === 3) {
                                            if (first_wlan_util >= this.signalRange.min.value && legendStates.wifi) {
                                                tooltipContent += `Others WiFi Interference: ${first_wlan_util.toFixed(0)}%<br/>`;
                                                found = true;
                                            }
                                            if (first_nonwlan_util >= this.signalRange.min.value && legendStates.nonwifi) {
                                                tooltipContent += `NonWiFi Interference: ${first_nonwlan_util.toFixed(0)}%<br/>`;
                                                found = true;
                                            }
                                        } else if (first_wlan_util >= this.signalRange.min.value && legendStates.wifi) {
                                            tooltipContent += `Others WiFi Interference: ${first_wlan_util.toFixed(0)}%<br/>`;
                                            found = true;
                                        } else if (first_nonwlan_util >= this.signalRange.min.value && legendStates.nonwifi) {
                                            tooltipContent += `NonWiFi Interference: ${first_nonwlan_util.toFixed(0)}%<br/>`;
                                            found = true;
                                        }

                                        tooltipEl.innerHTML = tooltipContent;
                                        positionTooltip(event, tooltipEl);
                                        break;
                                    }
                                }
                            }
                        }
                        if (found) break;
                    }
                }
            }

            // Check for DFS data hover
            if (!found && this.rectanglePlugin?.legendStates?.dfs && Array.isArray(dfsChartData)) {
                for (const d of dfsChartData) {
                    if (!d || !Array.isArray(d.x) || d.x.length < 2 || typeof d.y === 'undefined') {
                        continue;
                    }

                    const xStart = xScale.getPixelForValue(d.x[0]);
                    const xEnd = xScale.getPixelForValue(d.x[1]);
                    const yCenter = yScale.getPixelForValue(yOffsets[d.ch]?.center || d.y);
                    const barHeight = yOffsets[d.ch]?.height || 4;
                    const yTop = yCenter - (barHeight / 2) * unitHeight;
                    const yBottom = yCenter + (barHeight / 2) * unitHeight;

                    if (x >= xStart && x <= xEnd && y >= yTop && y <= yBottom) {
                        // 顯示 DFS tooltip
                        tooltipEl.innerHTML = `
                            ${new Date(d.x[0]).toLocaleString()} - ${new Date(d.x[1]).toLocaleString()}<br/>
                            Channel: ${d.ch}<br/>
                            DFS
                        `;
                        positionTooltip(event, tooltipEl);
                        found = true;
                        break;
                    }
                }
            }

            // 確保 barData 存在且是一個數組
            if (!Array.isArray(barData)) {
                console.error('barData is not an array or is undefined');
                return;
            }

            if (this.rectanglePlugin?.legendStates?.channel) {
                for (const d of barData) {
                    if (!d || !Array.isArray(d.x) || d.x.length < 2 || typeof d.y === 'undefined') {
                        console.error('Invalid data point:', d);
                        continue;
                    }

                    const xStart = xScale.getPixelForValue(d.x[0]);
                    const xEnd = xScale.getPixelForValue(d.x[1]);
                    const yCenter = yScale.getPixelForValue(yOffsets[d.ch]?.center || d.y);
                    const barHeight = yOffsets[d.ch]?.height || 4;
                    const yTop = yCenter - (barHeight / 2) * unitHeight;
                    const yBottom = yCenter + (barHeight / 2) * unitHeight;

                    if (x >= xStart && x <= xEnd && y >= yTop && y <= yBottom) {
                        // 顯示 tooltip
                        if (d.radio_status === "1") {

                            // 確保 showBand 和 getChannelFrequencyRange 存在
                            if (typeof showBand !== 'undefined' && typeof getChannelFrequencyRange === 'function') {
                                const freq_result = getChannelFrequencyRange(showBand, d.controlChannel, d.bw);
                                tooltipEl.innerHTML = `
                                ${d.x[0].toLocaleString()}<br/>
                                Control Channel ${d.controlChannel} / ${d.bw}MHz<br/>
                                Center Channel ${d.centerChannel || '未知'}<br/>
                                Frequency: ${freq_result ? freq_result.startMHz : '未知'}MHz - ${freq_result ? freq_result.endMHz : '未知'}MHz<br/>
                            `;
                            } else {
                                tooltipEl.innerHTML = `
                                ${d.x[0].toLocaleString()}<br/>
                                Control Channel ${d.controlChannel || '未知'} / ${d.bw || '未知'}MHz<br/>
                                Center Channel ${d.centerChannel || '未知'}<br/>
                            `;
                            }
                        } else {
                            tooltipEl.innerHTML = `
                                ${d.x[0].toLocaleString()}<br/>
                                WiFi Off
                            `;
                        }

                        positionTooltip(event, tooltipEl);
                        found = true;
                        break;
                    }
                }
            }

            if (!found) {
                tooltipEl.style.opacity = '0';
            }
        };

        // Create mouseleave handler to hide tooltip when mouse leaves chart
        const mouseleaveHandler = () => {
            tooltipEl.style.opacity = '0';
        };

        // Store the handler references for later removal
        wifi_inter_chart._mousemoveHandler = mousemoveHandler;
        wifi_inter_chart._mouseleaveHandler = mouseleaveHandler;

        wifi_inter_chart.addEventListener('mousemove', mousemoveHandler);
        wifi_inter_chart.addEventListener('mouseleave', mouseleaveHandler);
    }

    createDiagonalPattern(ctx, color = '#999999', spacing = 8, lineWidth = 1) {
        // 創建一個小的 canvas 作為圖案
        const patternCanvas = document.createElement('canvas');
        const patternCtx = patternCanvas.getContext('2d');

        patternCanvas.width = spacing * 2;
        patternCanvas.height = spacing * 2;

        patternCtx.strokeStyle = color;
        patternCtx.lineWidth = lineWidth;
        patternCtx.beginPath();
        // 畫斜線
        patternCtx.moveTo(0, 0);
        patternCtx.lineTo(spacing * 2, spacing * 2);
        patternCtx.moveTo(-spacing, spacing);
        patternCtx.lineTo(spacing, spacing * 3);
        patternCtx.stroke();

        return ctx.createPattern(patternCanvas, 'repeat');
    }

    async genChart() {
        // Validate that we have necessary data and elements
        if (!this.element) {
            throw new Error('DOM element not initialized');
        }

        if (!this.controlChannelList || this.controlChannelList.length === 0) {
            throw new Error('Control channel list not initialized');
        }

        const self = this;

        const style = getComputedStyle(document.documentElement);
        const wifiInsightColor =
            {
                'verygood': style.getPropertyValue('--color-chart-wifi-interference-verygood').trim(),
                'medium_1': style.getPropertyValue('--color-chart-wifi-interference-medium-1').trim(),
                'medium_2': style.getPropertyValue('--color-chart-wifi-interference-medium-2').trim(),
                'medium_3': style.getPropertyValue('--color-chart-wifi-interference-medium-3').trim(),
                'medium_4': style.getPropertyValue('--color-chart-wifi-interference-medium-4').trim(),
                'medium_5': style.getPropertyValue('--color-chart-wifi-interference-medium-5').trim(),
                'verybad_1': style.getPropertyValue('--color-chart-wifi-interference-verybad-1').trim(),
                'verybad_2': style.getPropertyValue('--color-chart-wifi-interference-verybad-2').trim(),
                'verybad_3': style.getPropertyValue('--color-chart-wifi-interference-verybad-3').trim(),
                'verybad_4': style.getPropertyValue('--color-chart-wifi-interference-verybad-4').trim(),
                'verybad_5': style.getPropertyValue('--color-chart-wifi-interference-verybad-5').trim(),
            };

        function getWifiInsightColor(value) {
            const min = self.signalRange.min.value;
            const max = self.signalRange.max.value;
            if (value < min) return wifiInsightColor.verygood;
            if (value < (max - min) / 5 * 1) return wifiInsightColor.medium_1;
            if (value < (max - min) / 5 * 2) return wifiInsightColor.medium_2;
            if (value < (max - min) / 5 * 3) return wifiInsightColor.medium_3;
            if (value < (max - min) / 5 * 4) return wifiInsightColor.medium_4;
            if (value < max) return wifiInsightColor.medium_5;
            if (value < (100 - max) / 5 * 1) return wifiInsightColor.verybad_1;
            if (value < (100 - max) / 5 * 2) return wifiInsightColor.verybad_2;
            if (value < (100 - max) / 5 * 3) return wifiInsightColor.verybad_3;
            if (value < (100 - max) / 5 * 4) return wifiInsightColor.verybad_4;
            if (value <= 100) return wifiInsightColor.verybad_5;
            return wifiInsightColor.verygood;
        }

        const chartData = this.chartData.filter(d => d.band.startsWith(this.showBand) || this.showBand.startsWith(d.band));
        const dfsData = this.dfsData.filter(d => d.band.startsWith(this.showBand) || this.showBand.startsWith(d.band));
        const interferenceData = this.interferenceData;
        const showBand = this.showBand;
        const getChannelFrequencyRange = this.getChannelFrequencyRange;

        const wifi_inter_chart = this.getCachedElement('#wifi_inter_chart');

        if (!wifi_inter_chart) {
            throw new Error('Chart canvas element not found');
        }

        if (wifi_inter_chart) {
            const ctx = wifi_inter_chart.getContext('2d');

            const label = [
                // {band: [], ch: 'bottom'},
                ...this.supportedBand.find(item => item.band.toUpperCase() === this.showBand)?.channel.map(ch => ({
                    band: [this.showBand],
                    ch: ch.toString()
                })) || [],
                {band: [], ch: 'Ch.'},
            ];

            if (this.showBand.startsWith("2G")) {
                label.unshift({band: [], ch: 'bottom', height: 6});
                //插在Ch之前
                label.splice(label.length - 1, 0, {band: [], ch: 'top', height: 6});
            }

            // 過濾 label 陣列，只保留 ch 有出現在 wifiData 裡的項目
            const filteredLabel = label.filter(item => (item.band.includes(this.showBand)) || item.ch === 'Ch.' || item.ch === 'bottom' || item.ch === 'top');
            this.filteredLabel = filteredLabel;
            let scale = 1;
            if (this.filteredLabel.length > 20) {
                scale = 0.8;
            }
            const chartElement = this.getCachedElement("#wifi_inter_chart");
            if (chartElement && chartElement.parentElement) {
                chartElement.parentElement.style.height = `${this.filteredLabel.length * 30 * scale}px`;
            }

            // 計算每個 channel 的實際位置
            const yOffsets = {}; // { "36": { center: xx, height: xx }, ... }
            let currentOffset = 0;
            filteredLabel.forEach(item => {
                const ch = `${item.ch}`;
                const height = item.height || 4; // 預設高度為 4，如果有指定則使用指定的高度
                yOffsets[ch] = {
                    center: currentOffset + height / 2,
                    height: height
                };
                item.center = currentOffset + height / 2;
                currentOffset += height; // 含 spacing
            });

            function toDateISO(str) {
                return new Date(str.replace(/\//g, '-').replace(' ', 'T'));
            }

            function calculateChannelInfo(centerChannel, bandwidth) {
                if (centerChannel === 0) {
                    return {center: 0, height: 0};
                }
                // Convert all numeric keys to numbers for comparison
                const numericChannels = Object.keys(yOffsets)
                    .filter(key => !isNaN(key))
                    .map(key => parseInt(key))
                    .sort((a, b) => a - b);

                // Find the two channels closest to centerChannel
                let lowerChannel = null;
                let upperChannel = null;

                for (let i = 0; i < numericChannels.length; i++) {
                    const channel = numericChannels[i];

                    if (channel <= centerChannel) {
                        lowerChannel = channel;
                    }
                    if (channel >= centerChannel && upperChannel === null) {
                        upperChannel = channel;
                        break;
                    }
                }

                // If centerChannel exactly matches a channel, use that channel's value directly
                if (lowerChannel === upperChannel) {
                    const centerY = yOffsets[centerChannel.toString()]?.center;
                    if (centerY !== undefined) {
                        return {
                            center: centerY,
                            height: (bandwidth / 20) * 4
                        };
                    }
                }

                // Check if lowerChannel and upperChannel are valid
                if (lowerChannel === null || upperChannel === null) {
                    console.warn('Invalid channel configuration:', {
                        lowerChannel,
                        upperChannel,
                        centerChannel,
                        availableChannels: numericChannels
                    });
                    return {center: 0, height: 0};
                }

                // Calculate center (sum of two centers divided by 2)
                const lowerCenter = yOffsets[lowerChannel.toString()].center;
                const upperCenter = yOffsets[upperChannel.toString()].center;
                const calculatedCenter = (lowerCenter + upperCenter) / 2;

                // 計算 height (bandwidth / 20 * 4)
                const calculatedHeight = (bandwidth / 20) * 4;

                return {
                    center: calculatedCenter,
                    height: calculatedHeight,
                };
            }


            const barData = chartData.map(d => {
                const ch = `${d.controlChannel}`;
                let centerInfo = (this.showBand.includes("2G")) ?
                    {
                        center: yOffsets[d.centerChannel]?.center ?? 0,
                        height: d.bandwidth / 5 * 4,
                    } :
                    calculateChannelInfo(d.centerChannel, d.bandwidth);
                return {
                    x: [toDateISO(d.timeStart), toDateISO(d.timeEnd)],
                    y: yOffsets[ch]?.center ?? 0,
                    height: yOffsets[ch]?.height ?? 4,
                    ch: ch,

                    controlChannel: d.controlChannel,
                    centerChannel: d.centerChannel,
                    bw: d.bandwidth,
                    centerInfo: centerInfo,

                    radio_status: d.radio_status,
                };
            });

            const airtime_data = [];

            for (const ch_bar of barData) {
                const channel = String(ch_bar.ch);
                const channel_start_time = ch_bar.x[0];
                const channel_end_time = ch_bar.x[1];
                const band = this.showBand.includes("2G")
                    ? "2G"
                    : this.showBand.includes("5G")
                        ? "5G"
                        : this.showBand.includes("6G")
                            ? "6G"
                            : "";

                if (band != "6G") {
                    for (const d of interferenceData) {
                        if (d.timeEnd >= channel_start_time && d.timeEnd <= channel_end_time) {
                            for (const ch of d.channels[band]) {
                                if (String(ch.channel) === channel) {
                                    for (const item of ch.items) {
                                        airtime_data.push(item);
                                    }
                                }
                            }
                        }
                    }
                }
            }

            this.genTxopChart(airtime_data);

            const dfsChartData = dfsData.map(d => {
                const ch = `${d.channel}`;
                return {
                    x: [toDateISO(d.timeStart), toDateISO(d.timeEnd)],
                    y: yOffsets[ch]?.center ?? 0,
                    height: yOffsets[ch]?.height ?? 4,
                    ch: ch
                };
            })

            function controlChannelToFrequency(band, controlChannel) {
                if (band.includes('2G') || band.includes('2G1')) {
                    if (controlChannel >= 1 && controlChannel <= 14) {
                        // 2.4GHz 頻段：每個 channel 間隔 5MHz，從 2412MHz 開始
                        return 2407 + controlChannel * 5;
                    } else {
                        throw new Error('Invalid 2.4GHz control channel: ' + controlChannel);
                    }
                } else if (band.includes('5G') || band.includes('5G1') || band.includes('5G2')) {
                    if (controlChannel >= 1 && controlChannel <= 233) {
                        // 5GHz 頻段：不同區段有不同起點，但一般計算方式是 channel × 5 + 5000
                        return controlChannel * 5 + 5000 - 10;
                    } else {
                        throw new Error('Invalid 5GHz control channel: ' + controlChannel);
                    }
                } else if (band.includes('6G') || band.includes('6G1') || band.includes('6G2')) {
                    if (controlChannel >= 1 && controlChannel <= 233) {
                        // 6GHz 頻段（Wi-Fi 6E）：channel × 5 + 5940
                        return controlChannel * 5 + 5940;
                    } else {
                        throw new Error('Invalid 6GHz control channel: ' + controlChannel);
                    }
                } else {
                    throw new Error('Unknown band: ' + band);
                }
            }

            // Capture the drawRoundedRect function in closure scope
            const drawRoundedRect = this.drawRoundedRect.bind(this);

            const rectanglePlugin = {
                id: 'wifi-rectangles',
                legendStates: {
                    channel: self.channelBarShow,
                    wifi: self.wifiInterShow,
                    nonwifi: self.nonwifiInterShow,
                    interferenceState: (() => {
                        // Initialize interference state based on current wifi/nonwifi states
                        if (self.wifiInterShow && self.nonwifiInterShow) return 3; // Both
                        if (self.wifiInterShow) return 1; // WiFi only
                        if (self.nonwifiInterShow) return 2; // NonWiFi only
                        return 0; // None
                    })(),
                    dfs: self.dfsShow,
                },
                afterDraw(chart) {
                    const {ctx, scales} = chart;
                    const xScale = scales.x;
                    const yScale = scales.y;
                    const bgXStart = xScale.getPixelForValue(xScale.min);
                    const bgXEnd = xScale.getPixelForValue(xScale.max);
                    const unitHeight = Math.abs(yScale.getPixelForValue(1) - yScale.getPixelForValue(0));

                    const yUpperCenter = yScale.getPixelForValue(yOffsets['Ch.']?.center);

                    const nowTime = chart.options.nowTime;

                    // each yOffsets
                    Object.keys(yOffsets).forEach((key) => {
                        if (key.toString() === "Ch." || key.toString() === "bottom" || key.toString() === "top") return;
                        const yCenter = yScale.getPixelForValue(yOffsets[key]?.center);
                        const barHeight = yOffsets[key]?.height || 4;
                        ctx.save();
                        if (showBand.includes("6G")) {
                            ctx.fillStyle = 'rgba(220, 220, 220, 0.5)';
                        } else {
                            ctx.fillStyle = 'rgba(220, 220, 220, 0.1)';
                        }
                        ctx.fillRect(bgXStart - 4, yCenter - (barHeight / 2) * unitHeight + 2, bgXEnd - bgXStart, barHeight * unitHeight - 4);
                        ctx.restore();

                        ctx.save();
                        ctx.strokeStyle = 'rgba(97, 173, 255, 1)';
                        ctx.lineWidth = 3;
                        ctx.beginPath();
                        ctx.moveTo(xScale.getPixelForValue(xScale.min) - 2, yCenter - (barHeight / 2) * unitHeight + 1);
                        ctx.lineTo(xScale.getPixelForValue(xScale.min) - 2, yCenter + (barHeight / 2) * unitHeight - 1);
                        ctx.stroke();
                        ctx.restore();
                    })

                    //Interference
                    const min = self.signalRange.min.value;
                    const max = self.signalRange.max.value;
                    interferenceData.forEach((d, index) => {
                        if (self.chartStartTime - 1 * 60 * 1000 < d.timeStart) {
                            const xStart = xScale.getPixelForValue(d.timeStart);
                            let xEnd = xScale.getPixelForValue(d.timeEnd);
                            if (index === interferenceData.length - 1) {
                                xEnd = xScale.getPixelForValue(nowTime);
                            }
                            let band = showBand.toUpperCase();
                            if (band.startsWith('2G')) {
                                band = '2G';
                            } else if (band.startsWith('5G')) {
                                band = '5G';
                            }
                            if (d.channels?.[band] != null) {
                                const channelData = d.channels[band];
                                const max_ch = self.getBandMaxChannel(band);
                                channelData.forEach(c => {

                                    const yCenter = yScale.getPixelForValue(yOffsets[c.channel]?.center);
                                    const barHeight = yOffsets[c.channel]?.height || 4;
                                    const first_wlan_util = c.first_wlan_util || 0;
                                    const first_nonwlan_util = c.first_nonwlan_util || 0;
                                    let insightValue = 0;
                                    if (first_nonwlan_util > 0 && this.legendStates.nonwifi) {
                                        insightValue += first_nonwlan_util;
                                    }
                                    if (first_wlan_util > 0 && this.legendStates.wifi) {
                                        insightValue += first_wlan_util;
                                    }
                                    if (this.legendStates.nonwifi || this.legendStates.wifi) {
                                        if (band.startsWith("2G")) {
                                            ctx.save();
                                            ctx.fillStyle = getWifiInsightColor(insightValue);
                                            if (c.channel == '1') {
                                                drawRoundedRect(ctx, xStart, yCenter - (barHeight / 2) * unitHeight, xEnd - xStart, barHeight * unitHeight * 2.5, 0, 0);
                                            } else if (c.channel == max_ch) { //max channel
                                                drawRoundedRect(ctx, xStart, yCenter - barHeight * 2 * unitHeight, xEnd - xStart, barHeight * unitHeight * 2.5, 0, 0);
                                            } else {
                                                drawRoundedRect(ctx, xStart, yCenter - (barHeight / 2) * unitHeight, xEnd - xStart, barHeight * unitHeight, 0, 0);
                                            }
                                            if (insightValue >= min) {
                                                ctx.fill();
                                            }
                                            ctx.restore();
                                        } else {
                                            ctx.save();
                                            ctx.fillStyle = getWifiInsightColor(insightValue);
                                            drawRoundedRect(ctx, xStart, yCenter - (barHeight / 2) * unitHeight, xEnd - xStart, barHeight * unitHeight, 0, 0);
                                            if (insightValue >= min) {
                                                ctx.fill();
                                            }
                                            ctx.restore();
                                        }
                                    }
                                })
                            }
                        }
                    })

                    if (this.legendStates.dfs) {
                        dfsChartData.forEach(d => {
                            if (d.x[1] < scales.x.min || d.channel == 0) return;
                            const ch = d.ch;
                            const xStart = (scales.x.min > d.x[0]) ? xScale.getPixelForValue(scales.x.min) : xScale.getPixelForValue(d.x[0]);
                            const xEnd = (nowTime < d.x[1]) ? xScale.getPixelForValue(nowTime) : xScale.getPixelForValue(d.x[1]);
                            const yCenter = yScale.getPixelForValue(d.y);
                            const barHeight = yOffsets[ch]?.height || 4;

                            const rectX = xStart;
                            const rectY = yCenter - (barHeight / 2) * unitHeight + 2;
                            const rectWidth = xEnd - xStart;
                            const rectHeight = barHeight * unitHeight - 4;

                            ctx.save();
                            // 先畫背景色
                            ctx.fillStyle = '#E9E9E966';
                            drawRoundedRect(ctx, rectX, rectY, rectWidth, rectHeight, 0, 0);
                            ctx.fill();
                            // 設定裁切區域為圓角矩形
                            drawRoundedRect(ctx, rectX, rectY, rectWidth, rectHeight, 0, 0);
                            ctx.clip();
                            // 畫斜線圖案
                            const diagonalPattern = self.createDiagonalPattern(ctx, '#666666', 6, 1);
                            ctx.fillStyle = diagonalPattern;
                            ctx.fillRect(rectX, rectY, rectWidth, rectHeight);
                            ctx.restore();

                            // 最後畫邊框
                            ctx.save();
                            ctx.strokeStyle = '#666666';
                            drawRoundedRect(ctx, rectX, rectY, rectWidth, rectHeight, 0, 0);
                            ctx.stroke();
                            ctx.restore();
                        });
                    }

                    barData.forEach((d, index) => {

                        if (d.x[1] < scales.x.min || d.controlChannel == 0) return;
                        const ch = d.ch;
                        const xStart = (scales.x.min > d.x[0]) ? xScale.getPixelForValue(scales.x.min) : xScale.getPixelForValue(d.x[0]);
                        const xEnd = xScale.getPixelForValue(d.x[1]);
                        const yCenter = yScale.getPixelForValue(d.y);
                        const barHeight = yOffsets[ch]?.height || 4;
                        const leftRadius = (scales.x.min > d.x[0]) ? 0 : 4;
                        if (d.radio_status === "1") { // wifi on

                            if (this.legendStates.channel) {

                                const controlFlagWidth = 5;
                                const yCenterChannelCenter = yScale.getPixelForValue(d.centerInfo.center) || 0;
                                const centerHeight = d.centerInfo.height || 4;

                                // head
                                ctx.save();
                                ctx.fillStyle = 'rgba(164, 210, 255, 0.8)';
                                drawRoundedRect(ctx, xStart, yCenterChannelCenter - (centerHeight / 2) * unitHeight + 2, (xEnd - xStart < controlFlagWidth) ? xEnd - xStart : controlFlagWidth, centerHeight * unitHeight - 4, leftRadius, 0);
                                ctx.fill();
                                ctx.restore();
                                // // head Control Flag
                                ctx.save();
                                ctx.fillStyle = 'rgba(97, 173, 255, 1)';
                                drawRoundedRect(ctx, xStart, yCenter - (barHeight / 2) * unitHeight + 2, (xEnd - xStart < controlFlagWidth) ? xEnd - xStart : controlFlagWidth, barHeight * unitHeight - 4, 0, 0);
                                ctx.fill();
                                ctx.restore();

                                // Control Flag
                                // ctx.save();
                                // ctx.fillStyle = 'rgba(97, 173, 255, 0.5)';
                                // drawRoundedRect(ctx, xStart, yCenter - (barHeight / 2) * unitHeight + 2, xEnd ? xEnd - xStart : controlFlagWidth, barHeight * unitHeight - 4, 0, 0);
                                // ctx.fill();
                                // ctx.restore();

                                // tail
                                if (xEnd - xStart > controlFlagWidth) {
                                    ctx.save();
                                    ctx.fillStyle = 'rgba(164, 210, 255, 0.8)';
                                    drawRoundedRect(ctx, xEnd - controlFlagWidth, yCenterChannelCenter - (centerHeight / 2) * unitHeight + 2, controlFlagWidth, centerHeight * unitHeight - 4, 0, 4);
                                    ctx.fill();
                                    ctx.restore();
                                    // // tail Control Flag
                                    ctx.save();
                                    ctx.fillStyle = 'rgba(97, 173, 255, 1)';
                                    drawRoundedRect(ctx, xEnd - controlFlagWidth, yCenter - (barHeight / 2) * unitHeight + 2, controlFlagWidth, barHeight * unitHeight - 4, 0, 0);
                                    ctx.fill();
                                    ctx.restore();
                                }


                                // bandwidth data
                                ctx.save();
                                ctx.fillStyle = 'rgba(97, 173, 255, 0.1)';
                                if (index === barData.length - 1) {
                                    ctx.strokeStyle = 'rgb(21,132,255)';
                                    ctx.shadowColor = 'rgba(21,132,255, 0.7)'; // 光暈顏色，建議跟描邊顏色相近或相同
                                    ctx.shadowBlur = 100;                      // 光暈模糊程度，數字越大越擴散
                                    ctx.shadowOffsetX = 0;                    // 水平位移，0 表示正中央
                                    ctx.shadowOffsetY = 0;                    // 垂直位移，0 表示正中央
                                } else {
                                    ctx.strokeStyle = 'rgba(97, 173, 255, 0.5)';
                                }
                                drawRoundedRect(ctx, xStart, yCenterChannelCenter - (centerHeight / 2) * unitHeight + 2, xEnd - xStart, centerHeight * unitHeight - 4, leftRadius, 4);
                                ctx.fill();
                                ctx.stroke();
                                ctx.shadowColor = 'transparent';
                                ctx.shadowBlur = 0;
                                ctx.shadowOffsetX = 0;
                                ctx.shadowOffsetY = 0;
                                ctx.restore();

                            }

                            // Time Line
                            ctx.save();
                            ctx.strokeStyle = 'rgba(153,202,255,0.5)';
                            ctx.lineWidth = 1;
                            ctx.beginPath();
                            ctx.moveTo(xStart, yUpperCenter - 4 * unitHeight + 5);
                            ctx.lineTo(xStart, yScale.getPixelForValue(0) + 10);
                            ctx.stroke();
                            ctx.restore();

                            // // 加上Time文字
                            // let timeText = d.x[0];
                            // if (timeText < self.chartStartTime) {
                            //     timeText = self.chartStartTime;
                            // }
                            // ctx.font = '10px sans-serif';
                            // ctx.fillStyle = 'rgba(0, 108, 225, 1)';
                            // ctx.textBaseline = 'top';
                            // ctx.fillText(timeText.toLocaleTimeString([], {
                            //     hour: '2-digit',
                            //     minute: '2-digit',
                            //     hour12: false
                            // }), xStart + 2, yScale.getPixelForValue(0) - unitHeight * 4 + 5);
                            // ctx.restore();

                            let scale = 1;
                            // if (showBand === "2G" || showBand === "2G1" || showBand === "5G1" || showBand === "5G2") {
                            //     scale = 1;
                            // } else if (showBand === "5G") {
                            //     scale = 1.5;
                            // } else if (showBand === "6G") {
                            //     scale = 1.5;
                            // }

                            // Upper Bar
                            ctx.save();
                            drawRoundedRect(ctx, xStart, yUpperCenter - 2 * unitHeight * scale + 2, xEnd - xStart, 4 * unitHeight * scale - 8, leftRadius, 4);
                            ctx.strokeStyle = 'rgba(0, 180, 113, 1)';
                            if (index === barData.length - 1) {
                                ctx.fillStyle = 'rgba(0, 219, 144, 0.2)';
                                ctx.fill();
                            }
                            ctx.stroke();
                            // 加上文字
                            const label = `${ch.split('-')[0]}`;
                            const centerX = (xStart + xEnd) / 2;
                            const centerY = yUpperCenter;
                            ctx.font = '12px sans-serif';
                            ctx.fillStyle = 'rgba(0, 180, 113, 1)';
                            ctx.textAlign = 'center';
                            ctx.textBaseline = 'middle';
                            ctx.fillText(label, centerX, centerY);
                            ctx.restore();
                        } else { // wifi off
                            // Upper Bar
                            let scale = 1;
                            ctx.save();
                            drawRoundedRect(ctx, xStart, yUpperCenter - 2 * unitHeight * scale + 2, xEnd - xStart, 4 * unitHeight * scale - 8, leftRadius, 4);
                            ctx.strokeStyle = 'rgba(204, 204, 204, 1)';
                            ctx.fillStyle = 'rgba(220, 220, 220, 0.6)';
                            ctx.stroke();

                            // data
                            ctx.save();
                            ctx.fillStyle = 'rgba(220, 220, 220, 0.6)';
                            ctx.strokeStyle = 'rgba(204, 204, 204, 1)';
                            drawRoundedRect(ctx, xStart, yCenter - (barHeight / 2) * unitHeight + 2, xEnd - xStart, barHeight * unitHeight - 4, leftRadius, 4);
                            ctx.fill();
                            ctx.stroke();
                            ctx.restore();
                        }
                    });


                    // Now Time Line
                    ctx.save();
                    ctx.strokeStyle = 'rgba(36, 141, 255, 1)';
                    ctx.lineWidth = 1;
                    ctx.setLineDash([5, 5]);
                    ctx.beginPath();
                    ctx.moveTo(xScale.getPixelForValue(nowTime), yUpperCenter - 4 * unitHeight + 16);
                    ctx.lineTo(xScale.getPixelForValue(nowTime), yScale.getPixelForValue(0) + 8);
                    ctx.stroke();
                    ctx.restore();

                    // 加上Time文字
                    ctx.save();
                    ctx.font = '10px sans-serif';
                    ctx.fillStyle = 'rgba(0, 108, 225, 1)';
                    ctx.textBaseline = 'top';
                    ctx.fillText(`Now`, xScale.getPixelForValue(nowTime) + 2, yScale.getPixelForValue(0));
                    ctx.restore();

                    // Frequency Line
                    let targets = [];
                    let result = [];
                    if (showBand === "2G" || showBand === "2G1") {
                        targets = [1, 3, 5, 7, 9, 11, 13];
                    } else if (showBand === "5G") {
                        targets = [36, 52, 100, 132, 149, 165];
                    } else if (showBand === "5G1") {
                        targets = [36, 52];
                    } else if (showBand === "5G2") {
                        targets = [100, 132, 149, 165];
                    } else if (showBand === "6G") {
                        targets = [1, 33, 65, 97, 129, 161, 193, 225];
                    } else if (showBand === "6G1") {
                        targets = [1, 33, 65];
                    } else if (showBand === "6G2") {
                        targets = [129, 161, 193, 225];
                    }
                    for (const target of targets) {
                        const filtered = filteredLabel.find(item => item.ch == target.toString());
                        if (filtered) {
                            result.push(filtered);
                        }
                    }

                    const dashLineXEnd = bgXEnd;
                    // Hz Text
                    ctx.save();
                    ctx.font = 'italic 13px sans-serif';
                    ctx.fillStyle = 'rgba(179, 179, 179, 1)';
                    ctx.textAlign = 'right';
                    ctx.textBaseline = 'middle';
                    ctx.fillText("Hz", bgXEnd, yUpperCenter - unitHeight * 2);
                    ctx.restore();

                    const filtered = filteredLabel.filter(item => item.ch !== 'Ch.' && item.ch !== 'bottom' && item.ch !== 'top');

                    if (showBand.startsWith("2G")) {
                        const baseFreq2gMin = controlChannelToFrequency(filtered[0].band, filtered[0].ch) - 10;
                        const baseFreqYMin = yScale.getPixelForValue(filtered[0].center - 8);
                        ctx.save();
                        ctx.strokeStyle = 'rgba(129, 129, 129, 1)';
                        ctx.lineWidth = .5;
                        ctx.setLineDash([5, 5]);
                        ctx.beginPath();
                        ctx.moveTo(bgXStart, baseFreqYMin);
                        ctx.lineTo(dashLineXEnd, baseFreqYMin);
                        ctx.stroke();
                        ctx.restore();

                        ctx.font = 'italic 13px sans-serif';
                        ctx.fillStyle = 'rgba(179, 179, 179, 1)';
                        ctx.textAlign = 'right';
                        ctx.textBaseline = 'bottom';
                        ctx.fillText(baseFreq2gMin.toFixed(0), bgXEnd, baseFreqYMin - (1 / 2) * unitHeight + 4);
                        ctx.restore();

                        //找出ch最大值
                        const baseFreq2gMax = controlChannelToFrequency(filtered[filtered.length - 1].band, filtered[filtered.length - 1].ch) + 10;
                        const baseFreqYMax = yScale.getPixelForValue(filtered[filtered.length - 1].center + 8);
                        ctx.save();
                        ctx.strokeStyle = 'rgba(129, 129, 129, 1)';
                        ctx.lineWidth = .5;
                        ctx.setLineDash([5, 5]);
                        ctx.beginPath();
                        ctx.moveTo(bgXStart, baseFreqYMax);
                        ctx.lineTo(dashLineXEnd, baseFreqYMax);
                        ctx.stroke();
                        ctx.restore();

                        ctx.font = 'italic 13px sans-serif';
                        ctx.fillStyle = 'rgba(179, 179, 179, 1)';
                        ctx.textAlign = 'right';
                        ctx.textBaseline = 'bottom';
                        ctx.fillText(baseFreq2gMax.toFixed(0), bgXEnd, baseFreqYMax - (1 / 2) * unitHeight + 4);
                        ctx.restore();
                    }

                    result.forEach((d, index) => {
                        let yCenter = yScale.getPixelForValue(d.center);
                        if (!showBand.startsWith("2G")) {
                            yCenter = yScale.getPixelForValue(d.center) + 2 * unitHeight;
                        }

                        ctx.save();
                        ctx.strokeStyle = 'rgba(129, 129, 129, 1)';
                        ctx.lineWidth = .5;
                        ctx.setLineDash([5, 5]);
                        ctx.beginPath();
                        ctx.moveTo(bgXStart, yCenter);
                        ctx.lineTo(dashLineXEnd, yCenter);
                        ctx.stroke();
                        ctx.restore();
                        // 加上文字

                        const freqLabel = controlChannelToFrequency(d.band, d.ch);
                        const label = freqLabel.toFixed(0);
                        const centerX = bgXEnd;
                        const centerY = yCenter - (1 / 2) * unitHeight + 4;
                        ctx.font = 'italic 13px sans-serif';
                        ctx.fillStyle = 'rgba(179, 179, 179, 1)';
                        ctx.textAlign = 'right';
                        ctx.textBaseline = 'bottom';
                        ctx.fillText(label, centerX, centerY);
                        ctx.restore();
                    })
                    if (!showBand.startsWith("2G")) {
                        const filtered = filteredLabel.filter(item => item.ch !== 'Ch.' && item.ch !== 'bottom' && item.ch !== 'top');
                        const maxChItem = filtered.reduce((maxItem, item) =>
                            Number(item.ch) > Number(maxItem.ch) ? item : maxItem
                        );

                        ctx.save();
                        ctx.strokeStyle = 'rgba(129, 129, 129, 1)';
                        ctx.lineWidth = .5;
                        ctx.setLineDash([5, 5]);
                        ctx.beginPath();
                        ctx.moveTo(bgXStart, yScale.getPixelForValue(maxChItem.center + 2));
                        ctx.lineTo(bgXEnd, yScale.getPixelForValue(maxChItem.center + 2));
                        ctx.stroke();
                        ctx.restore();

                        // 加上文字
                        const freqLabel = getChannelFrequencyRange(showBand, parseInt(maxChItem.ch), 20).endMHz;
                        const label = freqLabel;
                        const centerX = bgXEnd;
                        const centerY = yScale.getPixelForValue(maxChItem.center + 2) - (1 / 2) * unitHeight + 4;
                        ctx.font = 'italic 13px sans-serif';
                        ctx.fillStyle = 'rgba(179, 179, 179, 1)';
                        ctx.textAlign = 'right';
                        ctx.textBaseline = 'bottom';
                        ctx.fillText(label, centerX, centerY);
                    }

                }
            };

            const colorNonWifi = style.getPropertyValue('--color-chart-nonwifi-interference').trim();
            const colorWifi = style.getPropertyValue('--color-chart-wifi-interference').trim();

            // Store the chart instance for proper management during auto-refresh
            this.chartInstance = new Chart(ctx, {
                type: 'scatter',
                data: {
                    datasets: [{
                        label: '',
                        data: []
                    }]
                },
                options: {
                    nowTime: this.nowTime,
                    maintainAspectRatio: false,
                    responsive: true,
                    plugins: {
                        legend: {
                            display: false,
                        }
                    },
                    scales: {
                        x: {
                            border: {
                                display: false
                            },
                            type: 'time',
                            time: {
                                unit: (() => {
                                    if (self.currentTimeRange === '12H' || self.currentTimeRange === '1D') {
                                        return 'hour';
                                    }
                                    return 'minute';
                                })(),
                                stepSize: (() => {
                                    if (self.currentTimeRange === '12H') {
                                        return 2; // 每2小時
                                    } else if (self.currentTimeRange === '1D') {
                                        return 4; // 每4小時
                                    }
                                    return 15; // 每15分鐘
                                })(),
                                round: 'minute',   // 保證 tick 從整分鐘開始
                                displayFormats: {
                                    minute: 'HH:mm',
                                    hour: 'HH:mm'
                                }
                            },
                            title: {
                                display: false,
                                text: 'Time'
                            },
                            grid: {
                                display: true,
                                color: 'rgba(0, 0, 0, 0.05)',
                                drawBorder: false
                            },
                            min: this.chartStartTime,
                            max: this.chartEndTime
                        },
                        y: {
                            border: {
                                display: false
                            },
                            type: 'linear',
                            min: 0,
                            max: currentOffset,
                            ticks: {
                                stepSize: 2,
                                autoSkip: false,
                                callback: function (value) {
                                    for (let ch_bw in yOffsets) {
                                        if (ch_bw.toString() === "bottom" || ch_bw.toString() === "top") continue;
                                        if (Math.abs(yOffsets[ch_bw].center - value) < 2) {
                                            return ch_bw.split('-')[0];
                                        }
                                    }
                                    return '';
                                }
                            },
                            title: {
                                display: false,
                                text: 'Channel'
                            },
                            grid: {
                                display: false,
                                drawBorder: false
                            },
                            stack: "channel",
                            afterFit: function (scale) {
                                scale.width = 50; // 固定 Y 軸寬度為 60px
                            }
                        },
                    },
                    parsing: false
                },
                plugins: [rectanglePlugin]
            });

            // Create custom checkboxes in chart_info area
            this.generateHTMLLegend();
            this.generateTxopHTMLLegend();

            // 自定義 hover tooltip element
            let tooltipEl = document.getElementById('chartTooltip');
            if (!tooltipEl) {
                tooltipEl = document.createElement('div');
                tooltipEl.id = 'chartTooltip';
                tooltipEl.style.position = 'absolute';
                tooltipEl.style.background = 'rgba(0, 0, 0, 0.7)';
                tooltipEl.style.color = 'white';
                tooltipEl.style.padding = '5px';
                tooltipEl.style.borderRadius = '3px';
                tooltipEl.style.pointerEvents = 'none';
                tooltipEl.style.opacity = '0';
                tooltipEl.style.transition = 'opacity 0.3s ease';
                tooltipEl.style.zIndex = '1000';
                tooltipEl.style.left = '0';
                tooltipEl.style.top = '0';
                this.element.appendChild(tooltipEl);
            }

            // Store data for mousemove events
            this.barData = barData;
            this.dfsChartData = dfsChartData;
            this.yOffsets = yOffsets;
            this.rectanglePlugin = rectanglePlugin;

            // Bind mousemove events
            this.bindMousemoveEvents();

            // Chart instance is already stored as this.chartInstance above

        }

        function groupByTimeStart(data) {
            const grouped = data.reduce((acc, item) => {
                const key = item.timeStart;

                if (!acc[key]) {
                    acc[key] = {
                        dfsTimeStart: item.timeStart,
                        dfsTimeEnd: item.timeEnd,
                        dfsBand: item.band,
                        dfsChannels: [],
                        dfs: true
                    };
                }

                acc[key].dfsChannels.push(item.channel);
                return acc;
            }, {});

            return Object.values(grouped);
        }

        function merge5GWithDFS(data) {
            // 建立 DFS 資料的時間索引
            const dfsMap = new Map();
            data.filter(item => item.dfsTimeStart)
                .forEach(item => dfsMap.set(item.dfsTimeStart, item));

            // 處理所有有 timeStart 的資料
            return data.filter(item => item.timeStart).map(item => {
                const matchingDfs = dfsMap.get(item.timeStart);

                // 如果找到匹配的 DFS 資料，則合併
                if (matchingDfs) {
                    return {
                        ...item,  // 保留原始的 5G1 資料
                        ...matchingDfs  // 加入 DFS 資料
                    };
                }

                // 沒有匹配的 DFS 資料，直接返回原資料
                return item;
            });
        }


        const groupDfsData = groupByTimeStart(this.dfsData);
        let timeLineData = [];
        if (this.showBand.includes("5G")) {
            timeLineData = merge5GWithDFS([...chartData, ...groupDfsData]);
        } else {
            timeLineData = chartData;
        }

        function mergeData(data) {
            const groupMap = new Map();

            data.forEach(item => {
                const key = `${item.timeStart}|${item.timeEnd}|${item.band}`;

                if (!groupMap.has(key)) {
                    groupMap.set(key, []);
                }
                groupMap.get(key).push(item);
            });

            const result = [];

            groupMap.forEach((group, key) => {
                if (group.length === 1) {
                    result.push(group[0]);
                } else {
                    const mergedItem = {...group[0]};

                    const events = group
                        .map(item => item.event)
                        .filter(event => event && event.trim() !== ''); // 過濾空字符串

                    if (events.length > 1) {
                        mergedItem.event = events;
                    } else if (events.length === 1) {
                        mergedItem.event = events[0];
                    } else {
                        mergedItem.event = "";
                    }

                    result.push(mergedItem);
                }
            });

            return result;
        }

        const inverseWifiData = mergeData(timeLineData.slice().reverse());
        const timeline = this.element.querySelector('.timeline');
        timeline.innerHTML = '';
        inverseWifiData.forEach((d, index) => {
            const div = document.createElement('div');
            let text = '';

            if (d.controlChannel == 0) return;

            div.classList.add('entry');
            if (index === 0) {
                div.classList.add('now');
            }

            if (d.radio_status === "1") {
                text = `Channel ${d.controlChannel} / ${d.bandwidth}MHz`;
            } else {
                text = 'WiFi Off';
            }

            if (this.sysData.find(data => data.timestamp === d.timeStart)) {
                d.event = "Boot";
            }

            const event_desc_array = [];
            d.event.split(',').forEach((e, i) => {
                let event = '';
                switch (e) {
                    case 'HTTPD_CH_AUTO':
                        event = 'Manually configured to Auto Channel';
                        break
                    case 'HTTPD_CH_FIXED':
                        event = 'Manually configured to Fixed Channel';
                        break
                    case 'HTTPD_BW_AUTO':
                        event = 'Manually configured to Auto Bandwidth';
                        break
                    case 'HTTPD_BW_FIXED':
                        event = 'Manually configured to Fixed Bandwidth';
                        break
                    case 'TXOP':
                        event = `Interference`;
                        break
                    case 'TXFAIL':
                        event = `Interference(${d.event})`;
                        break
                    case 'RADAR':
                        event = `Interference(${d.event})`;
                        break
                    case 'EDCRS_HI':
                        event = `Regulatory(Channel conflict)`;
                        break
                    case 'TIMER':
                    case 'NONACS':
                    case 'IOCTL':
                        event = `Optimized automatically`;
                        break
                    case 'DFS-REENTRY':
                        event = `Optimized(DFS-REENTRY)`;
                        break
                    case 'INIT':
                        event = `Initial auto selection`;
                        break
                    case 'Boot':
                        event = `Auto-selection after reboot`;
                        break
                    case 'CSA':
                        if (d.oldBandwidth < d.bandwidth) {
                            event = `Adaptive Bandwidth: Expand`;
                        } else {
                            event = `Adaptive Bandwidth: Reduce`;
                        }
                        break
                    default:
                        event = e;
                        break
                }
                if (event !== '') {
                    event_desc_array.push(event)
                }
            })
            div.innerHTML = `
                <div class="time">${d.timeStart}${(index === 0) ? `<div class="now-text">Now</div>` : ``}</div>
                <div class="channel">${text}</div>
                <div class="d-flex flex-column gap-1">
                ${event_desc_array.length > 0 ? event_desc_array.map(e => {
                return `<div class="info" role="alert">${e}</div>`;
            }).join('') : ''}
                </div>
                ${(this.showBand.includes("5G") && d.dfs) ? `<div class="dfs">DFS</div>` : ``}
            `;
            timeline.appendChild(div);

        })
    }

    async genTxopChart(airtime_data) {
        const txop_chart = this.getCachedElement('#txop_chart');
        if (!txop_chart) {
            console.warn('TXOP chart canvas not found');
            return;
        }

        let wifiStatusBand = '2G';
        if (this.showBand.toUpperCase().startsWith('2G')) {
            wifiStatusBand = '2G';
        } else if (this.showBand.toUpperCase().startsWith('5G')) {
            wifiStatusBand = '5G';
        } else if (this.showBand.toUpperCase().startsWith('6G')) {
            wifiStatusBand = '6G';
        }
        const rawTxopData = this.wifiStatus[wifiStatusBand]?.txop_changes || [];

        const txopData = (() => {
            const validData = rawTxopData.map(item => {
                // Convert timestamp string back to Date object
                const dateObj = new Date(item.timestamp.replace(/\//g, '-').replace(' ', 'T'));
                return {
                    x: dateObj,
                    y: parseFloat(item.txop) || 0
                };
            }).filter(item => !isNaN(item.y) && item.x instanceof Date && !isNaN(item.x.getTime()))
                .sort((a, b) => a.x.getTime() - b.x.getTime()); // Sort by time

            if (validData.length === 0) return [];

            const result = [];
            for (let i = 0; i < validData.length; i++) {
                if (i > 0) {
                    const timeDiff = validData[i].x.getTime() - validData[i - 1].x.getTime();
                    // If gap is more than 120 seconds (2 minutes), insert null to break the line
                    if (timeDiff > 120000) {
                        result.push({x: new Date(validData[i - 1].x.getTime() + 60000), y: null});
                    }
                }
                result.push(validData[i]);
            }
            return result;
        })();

        const obssData = (() => {
            const validData = rawTxopData.map(item => {
                // Convert timestamp string back to Date object
                const dateObj = new Date(item.timestamp.replace(/\//g, '-').replace(' ', 'T'));
                return {
                    x: dateObj,
                    y: parseFloat(item.obss) || 0
                };
            }).filter(item => !isNaN(item.y) && item.x instanceof Date && !isNaN(item.x.getTime()))
                .sort((a, b) => a.x.getTime() - b.x.getTime()); // Sort by time

            if (validData.length === 0) return [];

            const result = [];
            for (let i = 0; i < validData.length; i++) {
                if (i > 0) {
                    const timeDiff = validData[i].x.getTime() - validData[i - 1].x.getTime();
                    // If gap is more than 120 seconds (2 minutes), insert null to break the line
                    if (timeDiff > 120000) {
                        result.push({x: new Date(validData[i - 1].x.getTime() + 60000), y: null});
                    }
                }
                result.push(validData[i]);
            }
            return result;
        })();

        const rawChannelData = airtime_data || [];

        const processDataWithGaps = (data, valueField) => {
            const validData = data.map(item => {
                return {
                    x: item.event_time,
                    y: parseFloat(item[valueField]) || 0
                };
            }).filter(item => !isNaN(item.y) && item.x instanceof Date && !isNaN(item.x.getTime()))
                .sort((a, b) => a.x.getTime() - b.x.getTime()); // Sort by time

            if (validData.length === 0) return [];

            const result = [];
            for (let i = 0; i < validData.length; i++) {
                if (i > 0) {
                    const timeDiff = validData[i].x.getTime() - validData[i - 1].x.getTime();
                    // If gap is more than 120 seconds (2 minutes), insert null to break the line
                    if (timeDiff > 120000) {
                        result.push({x: new Date(validData[i - 1].x.getTime() + 60000), y: null});
                    }
                }
                result.push(validData[i]);
            }
            return result;
        };

        const wifiData = processDataWithGaps(rawChannelData, 'wlan_util');
        const nonWifiData = processDataWithGaps(rawChannelData, 'nonwlan_util');

        // Destroy existing TXOP chart instance if it exists
        if (this.txopChartInstance) {
            try {
                this.txopChartInstance.destroy();
                this.txopChartInstance = null;
            } catch (error) {
                console.warn('Error destroying TXOP chart instance:', error);
            }
        }

        const ctx = txop_chart.getContext('2d');

        this.txopChartInstance = new Chart(ctx, {
            type: 'line',
            data: {
                datasets: [{
                    label: 'Others WiFi Interference',
                    data: this.showBand.startsWith('6G') ? obssData : wifiData,
                    borderColor: '#ee3a3e',
                    backgroundColor: 'rgba(238,58,62,0.1)',
                    borderWidth: 1,
                    pointRadius: 0,
                    hidden: false,
                    fill: true,
                    tension: 0.2,
                    yAxisID: 'stacked-y',
                }, {
                    label: 'NonWiFi Interference',
                    data: nonWifiData,
                    borderColor: '#f5b07c',
                    backgroundColor: 'rgb(245,176,124, 0.1)',
                    borderWidth: 1,
                    pointRadius: 0,
                    hidden: false,
                    fill: true,
                    tension: 0.2,
                    yAxisID: 'stacked-y',
                }, {
                    label: 'TXOP',
                    data: [], //txopData,
                    borderColor: '#7ca2f5',
                    backgroundColor: 'rgb(124,162,245,0.1)',
                    borderWidth: 1,
                    pointRadius: 0,
                    hidden: false,
                    fill: true,
                    tension: 0.2,
                    yAxisID: 'normal-y',
                }
                ]
            },
            options: {
                animation: false,
                maintainAspectRatio: false,
                responsive: true,
                interaction: {
                    mode: 'nearest',
                    intersect: false,
                },
                plugins: {
                    legend: {
                        display: false,
                    },
                    tooltip: {
                        mode: 'nearest',
                        intersect: false,
                        callbacks: {
                            title: function (context) {
                                const date = new Date(context[0].parsed.x);
                                return date.toLocaleString();
                            },
                            label: function (context) {
                                const datasetLabel = context.dataset.label;
                                const value = context.parsed.y.toFixed(0);
                                return `${datasetLabel}: ${value}%`;
                            }
                        }
                    }
                },
                hover: {
                    mode: 'nearest',
                    intersect: false
                },
                scales: {
                    x: {
                        type: 'time',
                        time: {
                            unit: (() => {
                                if (self.currentTimeRange === '12H' || self.currentTimeRange === '1D') {
                                    return 'hour';
                                }
                                return 'minute';
                            })(),
                            stepSize: (() => {
                                if (self.currentTimeRange === '12H') {
                                    return 2; // 每2小時
                                } else if (self.currentTimeRange === '1D') {
                                    return 4; // 每4小時
                                }
                                return 15; // 每15分鐘
                            })(),
                            round: 'minute',   // 保證 tick 從整分鐘開始
                            displayFormats: {
                                minute: 'HH:mm',
                                hour: 'HH:mm'
                            }
                        },
                        min: this.chartStartTime,
                        max: this.chartEndTime
                    },
                    'stacked-y': {
                        stacked: true,
                        position: 'left',
                        beginAtZero: true,
                        max: 100,
                        title: {
                            display: false,
                        },
                        grid: {
                            display: true,
                            color: 'rgba(0, 0, 0, 0.05)'
                        },
                        ticks: {
                            display: true,
                            maxTicksLimit: 5,
                            stepSize: 25,
                            callback: function (value) {
                                return value + '%';
                            }
                        },
                        afterFit: function (scale) {
                            scale.width = 50; // 固定 Y 軸寬度為 60px
                        }
                    },
                    'normal-y': {
                        type: 'linear',
                        position: 'right',
                        stacked: false, // 普通 y 轴不堆叠,
                        beginAtZero: true,
                        ticks: {
                            display: true,
                            maxTicksLimit: 5,
                            stepSize: 25,
                        },
                        max: 100,
                        display: false
                    }
                }
            }
        });
    }

    async reGenChart() {
        try {
            // Destroy existing Chart.js instance if it exists
            if (this.chartInstance) {
                try {
                    this.chartInstance.destroy();
                    this.chartInstance = null;
                } catch (destroyError) {
                    console.warn('Error destroying chart instance:', destroyError);
                }
            }

            // Destroy existing TXOP chart instance if it exists
            if (this.txopChartInstance) {
                try {
                    this.txopChartInstance.destroy();
                    this.txopChartInstance = null;
                } catch (destroyError) {
                    console.warn('Error destroying TXOP chart instance:', destroyError);
                }
            }

            // Check for any Chart.js instances attached to canvases
            const chartCanvas = this.getCachedElement('#wifi_inter_chart');
            const txopCanvas = this.getCachedElement('#txop_chart');

            if (chartCanvas && window.Chart && window.Chart.getChart) {
                const existingChart = window.Chart.getChart(chartCanvas);
                if (existingChart) {
                    existingChart.destroy();
                }
            }

            if (txopCanvas && window.Chart && window.Chart.getChart) {
                const existingTxopChart = window.Chart.getChart(txopCanvas);
                if (existingTxopChart) {
                    existingTxopChart.destroy();
                }
            }

            await this.genChart();
        } catch (error) {
            console.error('Failed to regenerate chart:', error);
            this.showError('Failed to update chart');
        }
    }

    async fetchChannelDB(mac) {
        try {
            await this.fetchChannelData(mac);
        } catch (error) {
            console.error('Failed to fetch channel data:', error);
            throw error;
        }
    }

    async fetchChannelData(mac) {
        function formatDate(date) {
            const pad = n => String(n).padStart(2, '0');
            const year = date.getFullYear();
            const month = pad(date.getMonth() + 1);
            const day = pad(date.getDate());
            const hour = pad(date.getHours());
            const minute = pad(date.getMinutes());
            const second = pad(date.getSeconds());

            return `${year}/${month}/${day} ${hour}:${minute}:${second}`;
        }

        // 取得現在的時間
        const now = new Date();
        const nowTimestamp = Math.floor(now.getTime() / 1000) - 60; // 減60秒避免時間誤差

        const fetchConnectData = async () => {
            let allProcessedData = []; // 儲存所有取得的資料
            const payload = ['node_type', 'band', 'old_control_chan', 'old_center_chan', 'old_bw', 'new_control_chan', 'new_center_chan', 'new_bw', 'old_rclass', 'new_rclass', 'data_time', 'event'];
            const fetchData = async (timeStamp) => {
                const queryParams = {
                    db: "channel_change",
                    starttime: nowTimestamp - 60 * 60 * 24,
                    endtime: nowTimestamp,
                    content: payload.join(';'),
                    filter: `node_mac>txt>${mac}>0`,
                };
                const queryString = new URLSearchParams(queryParams).toString();

                try {
                    const response = await fetch(`/get_diag_raw_data.cgi?${queryString}`);
                    if (!response.ok) {
                        throw new Error(`HTTP error! status: ${response.status}`);
                    }
                    const text = await response.text();
                    if (!text.trim()) {
                        return {contents: []};
                    }
                    return JSON.parse(text);
                } catch (error) {
                    console.warn('Error fetching channel data:', error);
                    return {contents: []};
                }
            };

            const fetchLatestData = async (band) => {
                const queryParams = {
                    db: "channel_change",
                    content: payload.join(';'),
                    filter: `node_mac>txt>${mac}>0;band>txt>${band}>0`,
                };
                const queryString = new URLSearchParams(queryParams).toString();

                try {
                    const response = await fetch(`/get_diag_latest_content_data.cgi?${queryString}`);
                    if (!response.ok) {
                        throw new Error(`HTTP error! status: ${response.status}`);
                    }
                    const text = await response.text();
                    if (!text.trim()) {
                        return {contents: []};
                    }
                    return JSON.parse(text);
                } catch (error) {
                    console.warn('Error fetching latest channel data:', error);
                    return {contents: []};
                }
            };

            const data = await fetchData(nowTimestamp);

            // 如果有資料，處理並儲存
            if (typeof data.contents !== 'undefined' && data.contents.length > 0) {
                const processedData = data.contents.map((entry, index) => {
                    return {
                        'node_type': entry[0],
                        'band': entry[1],
                        'old_control_chan': entry[2],
                        'old_center_chan': entry[3],
                        'old_bw': entry[4],
                        'new_control_chan': entry[5],
                        'new_center_chan': entry[6],
                        'new_bw': entry[7],
                        'old_rclass': entry[8],
                        'new_rclass': entry[9],
                        'data_time': new Date(Number(entry[10]) * 1000),
                        'event': entry[11],
                    };
                });

                // 將這次取得的資料加入到總資料中
                allProcessedData = allProcessedData.concat(processedData);
            }

            const receivedBands = new Set(allProcessedData.map(d => d.band.toUpperCase()));
            const supportedBandNames = this.supportedBand.map(b => b.bandName.toUpperCase());
            const missingBands = supportedBandNames.filter(band => !receivedBands.has(band));
            // 都沒有資料取最後一筆 對每個 supportedBand 發送請求
            for (const band of missingBands) {
                const data = await fetchLatestData(band);

                if (typeof data.contents !== 'undefined' && data.contents.length > 0) {
                    const processedData = data.contents.map((entry, index) => {
                        return {
                            'node_type': entry[0],
                            'band': entry[1],
                            'old_control_chan': entry[2],
                            'old_center_chan': entry[3],
                            'old_bw': entry[4],
                            'new_control_chan': entry[5],
                            'new_center_chan': entry[6],
                            'new_bw': entry[7],
                            'old_rclass': entry[8],
                            'new_rclass': entry[9],
                            'data_time': new Date(Number(entry[10]) * 1000),
                            'event': entry[11],
                        };
                    });
                    allProcessedData = allProcessedData.concat(processedData);
                }
            }

            // 處理所有收集到的資料
            if (allProcessedData.length > 0) {
                // 後續的資料處理邏輯
                const groupedByBand = {};

                for (const entry of allProcessedData) {
                    if (!groupedByBand[entry.band]) {
                        groupedByBand[entry.band] = [];
                    }
                    groupedByBand[entry.band].push(entry);
                }

                const results = [];

                for (const band in groupedByBand) {
                    const bandData = groupedByBand[band].sort((a, b) => new Date(a.data_time) - new Date(b.data_time));

                    for (let i = 0; i < bandData.length; i++) {
                        const current = bandData[i];
                        const curTime = new Date(current.data_time);
                        const prevTime = (i === 0)
                            ? this.chartStartTime
                            : bandData[i - 1].data_time;
                        const nextTime = (i === bandData.length - 1)
                            ? this.nowTime
                            : new Date(bandData[i + 1].data_time);

                        if (i === 0) {
                            results.push({
                                timeStart: formatDate(prevTime),
                                timeEnd: formatDate(curTime),
                                centerChannel: parseInt(current.old_center_chan),
                                controlChannel: parseInt(current.old_control_chan),
                                bandwidth: parseInt(current.old_bw),
                                oldCenterChannel: 0,
                                oldControlChannel: 0,
                                oldBandwidth: 0,
                                band: current.band,
                                event: current.event,
                                radio_status: "1"
                            });
                        }

                        results.push({
                            timeStart: formatDate(curTime),
                            timeEnd: formatDate(nextTime),
                            centerChannel: parseInt(current.new_center_chan),
                            controlChannel: parseInt(current.new_control_chan),
                            bandwidth: parseInt(current.new_bw),
                            oldCenterChannel: parseInt(current.old_center_chan),
                            oldControlChannel: parseInt(current.old_control_chan),
                            oldBandwidth: parseInt(current.old_bw),
                            band: current.band,
                            event: current.event,
                            radio_status: "1"
                        });
                    }
                }

                // 優化資料：合併連續且相同的資料條目
                const optimizedResults = [];
                for (let i = 0; i < results.length; i++) {
                    const current = results[i];

                    // 檢查是否可以與前一個條目合併
                    if (optimizedResults.length > 0) {
                        const previous = optimizedResults[optimizedResults.length - 1];

                        // 檢查所有關鍵屬性是否相同，且時間連續
                        if (previous.centerChannel === current.centerChannel &&
                            previous.controlChannel === current.controlChannel &&
                            previous.bandwidth === current.bandwidth &&
                            previous.band === current.band &&
                            previous.event === current.event &&
                            previous.timeEnd === current.timeStart) {

                            // 合併：延長前一個條目的timeEnd
                            previous.timeEnd = current.timeEnd;
                            continue;
                        }
                    }

                    // 無法合併，添加新條目
                    optimizedResults.push(current);
                }

                return optimizedResults;
            }

            return [];
        };

        const wifiDetectPayload = ['node_type', 'band', 'radio_status', 'txop', 'ifname', 'data_time', 'obss'];
        const wifiDetectQueryParams = {
            db: "wifi_detect",
            starttime: nowTimestamp - 60 * 60 * 24,
            endtime: nowTimestamp,
            content: wifiDetectPayload.join(';'),
            filter: `node_mac>txt>${mac}>0`,
        };
        const wifiDetectQueryString = new URLSearchParams(wifiDetectQueryParams).toString();
        const fetchWifiStatusData = fetch(`/get_diag_raw_data.cgi?${wifiDetectQueryString}`)
            .then(data => data.json())
            .then(data => {
                if (typeof data.contents !== 'undefined' && data.contents.length > 0) {
                    return data.contents.map((entry, index) => {
                        return {
                            'node_type': entry[0],
                            'band': entry[1],
                            'radio_status': entry[2],
                            'txop': entry[3],
                            'ifname': entry[4],
                            'data_time': new Date(Number(entry[5]) * 1000),
                            'obss': entry[6],
                        };
                    });
                } else {
                    return []
                }
            })
            .then(data => {
                if (data.length === 0) return data;

                const groupedByBand = {};
                const filteredData = data.filter(item => {
                    return item.ifname.startsWith("wl")
                })

                // 按頻段分組
                for (const entry of filteredData) {
                    if (!groupedByBand[entry.band]) {
                        groupedByBand[entry.band] = [];
                    }
                    groupedByBand[entry.band].push(entry);
                }

                const result = {};

                for (const band in groupedByBand) {
                    const sortedData = groupedByBand[band].sort((a, b) => new Date(a.data_time) - new Date(b.data_time));

                    // 處理 radio_status 變化
                    const radioStatusSummaries = [];
                    if (sortedData.length > 0) {
                        let currentRadioStatus = sortedData[0].radio_status;
                        let radioStartTime = sortedData[0].data_time;

                        for (let i = 1; i < sortedData.length; i++) {
                            const {radio_status, data_time} = sortedData[i];
                            if (radio_status !== currentRadioStatus) {
                                // 狀態改變，記錄前一個狀態段（結束時間是當前資料的時間）
                                radioStatusSummaries.push({
                                    radio_status: currentRadioStatus,
                                    start_time: formatDate(radioStartTime),
                                    end_time: formatDate(data_time)  // 使用下一筆資料的時間作為結束時間
                                });
                                currentRadioStatus = radio_status;
                                radioStartTime = data_time;
                            }
                        }
                        // 添加最後一個狀態段（結束時間是最後一筆資料的時間）
                        radioStatusSummaries.push({
                            radio_status: currentRadioStatus,
                            start_time: formatDate(radioStartTime),
                            end_time: formatDate(sortedData[sortedData.length - 1].data_time)
                        });
                    }

                    // 過濾掉重複的 timestamp，只保留每個 timestamp 的第一筆資料
                    const uniqueTimestamps = new Map();
                    sortedData.forEach(entry => {
                        const timestamp = formatDate(entry.data_time);
                        if (!uniqueTimestamps.has(timestamp)) {
                            uniqueTimestamps.set(timestamp, {
                                txop: entry.txop,
                                obss: entry.obss,
                                timestamp: timestamp,
                            });
                        }
                    });

                    const txopSummaries = Array.from(uniqueTimestamps.values());

                    // 將兩種狀態分別存儲
                    result[band] = {
                        radio_status_changes: radioStatusSummaries,
                        txop_changes: txopSummaries
                    };
                }
                return result;
            })


        const interferencePayload = ['event_time', 'channel', 'wlan_util', 'nonwlan_util', 'total_util', 'interferer_type', 'interferer_util'];
        const fetchInterferenceData = async () => {
            const allInterferenceData = [];
            const sampling = 60;
            const interferenceQueryParams = {
                db: "airiq_event",
                ts: nowTimestamp,
                duration: sampling,
                point: 60 * 24,
                duration_column_name: "event_time",
                content: interferencePayload.join(';'),
                filter: `node_mac>txt>${mac}>0`,
            };

            const interferenceQueryString = new URLSearchParams(interferenceQueryParams).toString();
            const url = `/get_diag_content_data.cgi?${interferenceQueryString}`;

            try {
                const response = await fetch(url);
                const data = await response.json();

                if (data.contents && data.contents.length > 0) {
                    const processedData = data.contents.map(entry => ({
                        event_time: new Date(entry[0] * 1000), // 轉成 Date
                        channel: entry[1],
                        wlan_util: entry[2],
                        nonwlan_util: entry[3],
                        total_util: entry[4],
                        interferer_type: entry[5],
                        interferer_util: entry[6],
                    }));

                    allInterferenceData.push(...processedData);
                }
            } catch (err) {
                console.error("fetch error:", err);
            }

            // 按採樣時間群組資料的函數
            function groupBySampling(data, samplingMs) { // 預設1分鐘
                const grouped = data.reduce((acc, item) => {
                    const time = item.event_time;

                    // 直接使用毫秒數進行採樣分組
                    const samplingKey = Math.floor(time.getTime() / samplingMs) * samplingMs;

                    if (!acc[samplingKey]) {
                        acc[samplingKey] = [];
                    }
                    acc[samplingKey].push(item);
                    return acc;
                }, {});

                // 計算每個採樣時間每個頻道的最大值
                return Object.entries(grouped).map(([samplingKey, items]) => {
                    // 按頻道分組
                    const channelGroups = items.reduce((channelAcc, item) => {
                        const channel = item.channel;
                        if (!channelAcc[channel]) {
                            channelAcc[channel] = [];
                        }
                        channelAcc[channel].push(item);
                        return channelAcc;
                    }, {});

                    // 計算每個頻道的最大值並包含該頻道的所有項目
                    const channelMaxValues = Object.entries(channelGroups).map(([channel, channelItems]) => {
                        const wlanUtilValues = channelItems.map(item => parseFloat(item.wlan_util)).filter(val => !isNaN(val));
                        const nonwlanUtilValues = channelItems.map(item => parseFloat(item.nonwlan_util)).filter(val => !isNaN(val));
                        const totalUtilValues = channelItems.map(item => parseFloat(item.total_util)).filter(val => !isNaN(val));
                        const interfererTypes = channelItems.map(item => item.interferer_type).filter(type => type !== "-1");
                        const interfererUtils = channelItems.map(item => item.interferer_util).filter(type => type !== "-1");

                        // 判斷頻段
                        const channelNum = parseInt(channel);
                        const band = channelNum <= 14 ? '2G' : (channelNum >= 32 ? '5G' : 'Unknown');

                        return {
                            channel: channel,
                            band: band,
                            first_wlan_util: wlanUtilValues.length > 0 ? wlanUtilValues[0] : 0,
                            first_nonwlan_util: nonwlanUtilValues.length > 0 ? nonwlanUtilValues[0] : 0,
                            first_total_util: totalUtilValues.length > 0 ? totalUtilValues[0] : 0,
                            interferer_type: interfererTypes,
                            interferer_utils: interfererUtils,
                            items: channelItems,
                            count: channelItems.length
                        };
                    }).sort((a, b) => a.channel - b.channel); // 按頻道號排序

                    // 按頻段分組
                    const channelsByBand = channelMaxValues.reduce((bandAcc, channelData) => {
                        const band = channelData.band;
                        if (!bandAcc[band]) {
                            bandAcc[band] = [];
                        }
                        bandAcc[band].push(channelData);
                        return bandAcc;
                    }, {});

                    if (!channelsByBand['2G']) {
                        channelsByBand['2G'] = [];
                    }
                    if (!channelsByBand['5G']) {
                        channelsByBand['5G'] = [];
                    }

                    return {
                        timeStart: new Date(parseInt(samplingKey)),
                        timeEnd: new Date(parseInt(samplingKey) + samplingMs), // 加採樣間隔
                        channels: channelsByBand,
                        total_count: items.length
                    };
                }).sort((a, b) => a.timeStart - b.timeStart); // 按時間順序排序
            }

            // 資料齊全後進行分組處理
            return groupBySampling(allInterferenceData, sampling * 1000);
        }


        const dfsPayload = ['node_type', 'dfs_info', 'data_time'];
        const dfsQueryParams = {
            db: "wifi_dfs",
            starttime: nowTimestamp - 60 * 60 * 24,
            endtime: nowTimestamp,
            content: dfsPayload.join(';'),
            filter: `node_mac>txt>${mac}>0`,
        };
        const dfsQueryString = new URLSearchParams(dfsQueryParams).toString();
        const fetchDfsData = fetch(`/get_diag_raw_data.cgi?${dfsQueryString}`)
            .then(data => data.json())
            .then(data => {
                if (typeof data.contents !== 'undefined' && data.contents.length > 0) {
                    return data.contents.map((entry, index) => {
                        return {
                            'node_type': entry[0],
                            'dfs_info': entry[1],
                            'data_time': new Date(Number(entry[2]) * 1000),
                        };
                    });
                } else {
                    return []
                }
            }).then(data => {
                function parseDataSafely(str) {
                    try {
                        return str.split(':').map((item, index) => {
                            const parts = item.split(',');
                            if (parts.length !== 3) {
                                return null;
                            }
                            return {
                                channel: parseInt(parts[0]) || parts[0],
                                action: parts[1],
                                minute: parseInt(parts[2])
                            };
                        }).filter(item => item !== null);
                    } catch (error) {
                        console.error('parse error:', error);
                        return [];
                    }
                }

                const results = [];
                for (const entry of data) {
                    const dfsInfoParsed = parseDataSafely(entry.dfs_info);
                    for (const dfs of dfsInfoParsed) {
                        if (dfs.action == "inactive") {
                            results.push({
                                timeStart: formatDate(entry.data_time),
                                timeEnd: formatDate(new Date(entry.data_time.getTime() + dfs.minute * 60 * 1000)),
                                channel: dfs.channel,
                                band: '5G',
                            });
                        }
                    }
                }
                return results;
            })

        const sysPayload = ['node_type', 'data_time'];
        const sysQueryParams = {
            db: "sys_setting",
            starttime: nowTimestamp - 60 * 60 * 24,
            endtime: nowTimestamp,
            content: sysPayload.join(';'),
            filter: `node_mac>txt>${mac}>0`,
        };
        const sysQueryString = new URLSearchParams(sysQueryParams).toString();
        const fetchSysData = fetch(`/get_diag_raw_data.cgi?${sysQueryString}`)
            .then(data => data.json())
            .then(data => {
                if (typeof data.contents !== 'undefined' && data.contents.length > 0) {
                    return data.contents.map((entry, index) => {
                        return {
                            'node_type': entry[0],
                            'data_time': new Date(Number(entry[1]) * 1000),
                            'timestamp': formatDate(new Date(Number(entry[1]) * 1000)),
                        };
                    });
                } else {
                    return []
                }
            })

        const _nvram = async () => {
            const data = await httpApi.nvramGet(["airiq_enable"], true);
            return {
                airiq_enable: data.airiq_enable || "0",
            }
        }


        // await Promise.all([fetchConnectData, fetchWifiStatusData, fetchInterferenceData, fetchDfsData, _nvram()]).then(values => {
        await Promise.all([fetchConnectData(), fetchWifiStatusData, fetchInterferenceData(), fetchDfsData, fetchSysData, _nvram()]).then(values => {
            const [chartData, wifiStatus, interferenceData, dfsData, sysData, _nvram] = values;

            this.dfsData = dfsData;
            this.sysData = sysData;

            this.interferenceEnabled = _nvram.airiq_enable === "1";
            if (this.interferenceEnabled) {
                this.getCachedElement("#interference_toggle").classList.add("active");
                this.getCachedElement("#interferenceStatusText").classList.add("d-md-inline")
            }

            this.wifiStatus = wifiStatus;

            const result = [];

            function toDateISO(str) {
                return new Date(str.replace(/\//g, '-').replace(' ', 'T'));
            }

            for (const entry of chartData) {
                let band = entry.band;
                if (band.toUpperCase().startsWith("5G")) {
                    band = '5G'
                } else if (band.toUpperCase().startsWith("6G")) {
                    band = '6G'
                }
                const aStart = toDateISO(entry.timeStart);
                const aEnd = toDateISO(entry.timeEnd);
                const slices = wifiStatus[band]?.radio_status_changes || [];
                let currentStart = aStart;

                for (const slice of slices) {
                    if (slice.radio_status !== '1') {
                        const bStart = toDateISO(slice.start_time);
                        const bEnd = toDateISO(slice.end_time);

                        if (bEnd <= aStart || bStart >= aEnd) continue; // 無交集

                        // 前段
                        if (bStart > currentStart) {
                            result.push({
                                ...entry,
                                timeStart: formatDate(currentStart),
                                timeEnd: formatDate(bStart)
                            });
                        }

                        // 插入 B 段
                        result.push({
                            ...entry,
                            radio_status: slice.radio_status,
                            timeStart: formatDate(bStart),
                            timeEnd: formatDate(bEnd)
                        });

                        currentStart = bEnd;
                    }
                }

                // 後段
                if (currentStart < aEnd) {
                    result.push({
                        ...entry,
                        timeStart: formatDate(currentStart),
                        timeEnd: formatDate(aEnd)
                    });
                }
            }
            this.chartData = result;
            this.interferenceData = interferenceData;
        });
    }

    render() {
        return this.element
    }

    // Clean up all event handlers
    cleanupEventHandlers() {
        for (const [key, {element, event, handler}] of this.eventHandlers) {
            element.removeEventListener(event, handler);
        }
        this.eventHandlers.clear();
        this.domCache.clear();
    }

    close() {
        // Stop auto refresh
        this.stopAutoRefresh();

        // Clean up chart instance
        if (this.chartInstance) {
            try {
                this.chartInstance.destroy();
            } catch (error) {
                console.warn('Error destroying chart instance:', error);
            }
            this.chartInstance = null;
        }

        // Clean up TXOP chart instance
        if (this.txopChartInstance) {
            try {
                this.txopChartInstance.destroy();
            } catch (error) {
                console.warn('Error destroying TXOP chart instance:', error);
            }
            this.txopChartInstance = null;
        }

        // Clean up event handlers and cache
        this.cleanupEventHandlers();

        // Remove DOM element
        this.element.remove();

        // Reset initialization flag
        this.isInitialized = false;
    }
}