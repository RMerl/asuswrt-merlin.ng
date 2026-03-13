export class AiProtectionFeature {
    constructor(props) {
        this.type = props.type;
        this.urlBase = props.urlBase;
        this.title = props.title;
        this.data = [];
        this.support = isSupport(props.support);
        this.featureSwitch = null;
        this.topClientWidget = null;
        this.detailTableWidget = null;
        this.chartWidget = null;
    }

    setData(data) {
        this.data = data;
        if (this.topClientWidget) {
            this.topClientWidget.updateDataset(data);
        }
        if (this.detailTableWidget) {
            this.detailTableWidget.updateDataset(data);
        }
        if (this.chartWidget) {
            this.chartWidget.updateDataset(data);
        }
    }

    fetchData() {
        const currentTime = Math.floor(Date.now() / 1000);
        const weekAgoTime = currentTime - (7 * 24 * 60 * 60) - 1;
        this.fetchDataWithDateRange(weekAgoTime, currentTime);
    }

    fetchDataWithDateRange(startTime, endTime) {
        const allClientSet = window.clientList;

        function formatDate(date) {
            const yyyy = date.getFullYear();
            const MM = String(date.getMonth() + 1).padStart(2, '0');
            const dd = String(date.getDate()).padStart(2, '0');
            const HH = String(date.getHours()).padStart(2, '0');
            const mm = String(date.getMinutes()).padStart(2, '0');
            const ss = String(date.getSeconds()).padStart(2, '0');
            return `${yyyy}/${MM}/${dd} ${HH}:${mm}:${ss}`;
        }

        fetch(`${this.urlBase}?type=${this.type}&starttime=${startTime}&endtime=${endTime}`)
            .then(response => response.json())
            .then(data => {
                data.map(item => {
                    const client = allClientSet.findClientByMac(item.mac) || null;
                    const client_name = client?.nickName || client?.name || item.mac.toUpperCase();
                    item.mac = item.mac.toUpperCase();
                    item.format_timestamp = formatDate(new Date(item.timestamp * 1000));
                    item.client_name = client_name;
                    item.client = client;
                    return item;
                })
                this.setData(data);
                // console.log(`Fetched data for ${this.type}:`, data);
            })
            .catch(error => {
                console.error(`Error fetching data for ${this.type}:`, error);
            });
    }
}

export class TopClientWidget {
    constructor(type, targetElement, col = []) {
        this.type = type;
        this.targetElement = targetElement;
        this.element = null;
        this.data = [];
        this.col = col;
    }

    updateDataset(data) {
        this.data = data;
        this.render();
    }

    getCount() {
        return this.data.length;
    }

    generateHTML() {
        const count = this.getCount();
        const allClientSet = window.clientList;

        const renderEmpty = () => `
                <div class="d-flex justify-content-center align-items-center my-4">
                    <div class="fs-5"><#AiProtection_eventnodetected#></div>
                </div>
            `;

        const renderClient = (mac, num, total) => {
            const percentage = Math.min(100, Math.max(1, Math.ceil((num / total) * 100)));
            const name = allClientSet[mac] ? allClientSet[mac].name : mac;

            return `
                    <div class="row">
                        <div class="col-6 text-truncate dns-text" title="${mac}">
                            ${name}
                        </div>
                        <div class="col-4 my-auto">
                            <div class="progress">
                                <div class="progress-bar progress-active bar-percent-${percentage}" role="progressbar"></div>
                            </div>
                        </div>
                        <div class="col-2 text-end card-data-value">${num}</div>
                    </div>
                `;
        };

        const stats = this.data.reduce((acc, {mac}) => {
            acc[mac] = (acc[mac] || 0) + 1;
            return acc;
        }, {});

        const clientHTML = Object.entries(stats)
            .sort(([, a], [, b]) => b - a) // Sort by count descending
            .map(([mac, num]) => renderClient(mac.toUpperCase(), num, count))
            .join("");

        return `
                <h5 class="card-header"><#AiProtection_event#></h5>
                <div class="card-body">
                    <div class="row h-100">
                        <div class="dns-content-default">
                            <div class="d-flex align-items-center justify-content-center flex-column">
                                <div class="fs-4">${count}</div>
                                <div class="fs-7"><#AiProtection_scan_rHits#></div>
                            </div>
                            <div class="d-flex justify-content-between pb-2 card-data-title">
                                <div><#AiProtection_event_Source#></div>
                                <div><#NetworkTools_Count#></div>
                            </div>
                            <div class="d-flex flex-column gap-2" style="max-height: 290px; overflow: hidden auto;">
                                ${count === 0 ? renderEmpty() : clientHTML}
                            </div>
                        </div>
                    </div>
                </div>
            `;
    }

    render() {
        const div = document.createElement('div');
        div.className = this.col.length == 0 ? `col-12 col-md-5` : this.col.join(' ');
        const card = document.createElement('div');
        card.className = "border shadow-sm pt-2";
        card.style = "height: 450px;";
        card.innerHTML = this.generateHTML();
        div.appendChild(card);

        if (this.element) {
            this.element.replaceWith(div);
        } else {
            this.targetElement.append(div);
        }

        this.element = div;
    }

    init() {
        this.render();
    }
}

export class ChartWidget {
    constructor(type, targetElement, col = []) {
        this.type = type;
        this.targetElement = targetElement;
        this.element = null;
        this.chart = null;
        this.data = [];
        this.col = col;

        this.configs = {
            dpi_mals: {
                title: '<#AiProtection_activity#>',
                canvasId: `${type}_chart`,
                datasets: [{
                    label: "",
                    yAxisID: "y",
                    tension: 0.2,
                    borderWidth: 1
                }]
            },
            dpi_vp: {
                title: '<#AiProtection_level#>',
                canvasId: `${type}_chart`,
                hasFooter: true,
                datasets: [
                    {
                        label: "High",
                        backgroundColor: `rgba(237, 28, 36, 0.3)`,
                        borderColor: `rgb(237, 28, 36)`,
                        fill: true,
                        yAxisID: "y",
                        borderWidth: 1,
                        dataKey: 'H'
                    },
                    {
                        label: "Medium",
                        backgroundColor: `rgba(215, 215, 0, 0.3)`,
                        borderColor: `rgb(215, 215, 0)`,
                        fill: true,
                        yAxisID: "y",
                        borderWidth: 1,
                        dataKey: 'M'
                    },
                    {
                        label: "Low",
                        backgroundColor: `rgba(19, 211, 210, 0.3)`,
                        borderColor: `rgb(19, 211, 210)`,
                        fill: true,
                        yAxisID: "y",
                        borderWidth: 1,
                        dataKey: 'L'
                    }
                ],
                hasStepSize: true
            },
            dpi_cc: {
                title: '<#AiProtection_activity#>',
                canvasId: `${type}_chart`,
                datasets: [{
                    label: "",
                    fill: true,
                    yAxisID: "y",
                    borderWidth: 1
                }]
            },
            ark_mals: {
                title: '<#AiProtection_activity#>',
                canvasId: `${type}_chart`,
                datasets: [{
                    label: "",
                    fill: true,
                    yAxisID: "y",
                    borderWidth: 1
                }]
            },
            ark_adblock: {
                title: '<#AiProtection_activity#>',
                canvasId: `${type}_chart`,
                datasets: [{
                    label: "",
                    fill: true,
                    yAxisID: "y",
                    borderWidth: 1
                }]
            },
            ark_tracker: {
                title: '<#AiProtection_activity#>',
                canvasId: `${type}_chart`,
                datasets: [{
                    label: "",
                    fill: true,
                    yAxisID: "y",
                    borderWidth: 1
                }]
            }
        };
    }

    updateDataset(data) {
        this.data = data;
        if (this.chart) {
            this.updateChart();
        }
    }

    getTypeConfig() {
        return this.configs[this.type];
    }

    generateDatasets() {
        const config = this.getTypeConfig();

        return config.datasets.map(datasetConfig => {
            const dataset = {
                label: datasetConfig.label,
                backgroundColor: datasetConfig.backgroundColor || "#b7f3d5",
                borderColor: datasetConfig.borderColor || "#34c38f",
                fill: true,
                yAxisID: datasetConfig.yAxisID,
                tension: 0.2,
                borderWidth: datasetConfig.borderWidth,
                data: this.data.reduce((acc, item) => {
                    const itemDate = new Date(item.timestamp * 1000);
                    const month = itemDate.getMonth() + 1;
                    const date = itemDate.getDate();
                    const label = `${month}/${date}`;

                    if (acc[label] === undefined) {
                        acc[label] = 0;
                    }

                    if (datasetConfig.dataKey) {
                        if (item.severity && item.severity.toUpperCase() === datasetConfig.dataKey) {
                            acc[label] += 1;
                        }
                    } else {
                        acc[label] += 1;
                    }

                    return acc;
                }, {})
            };

            return dataset;
        });
    }

    generateHTML() {
        const config = this.getTypeConfig();

        let code = `
                <h5 class="card-header d-flex align-items-center justify-content-between">
                    ${config.title}
                </h5>
                <div class="card-body">
                    <div class="row">
                        <div class="ips-chart-height">
                            <canvas id="${config.canvasId}"></canvas>
                        </div>
                    </div>
                </div>`;

        if (config.hasFooter) {
            code += `
                <div class="card-footer d-flex align-items-center">
                    <div class="d-flex align-items-center me-3">
                        <div class="me-2 vp-tab vp-tab-high"></div>
                        <div>High</div>
                    </div>
                    <div class="d-flex align-items-center me-3">
                        <div class="me-2 vp-tab vp-tab-medium"></div>
                        <div>Medium</div>
                    </div>
                    <div class="d-flex align-items-center me-3">
                        <div class="me-2 vp-tab vp-tab-low"></div>
                        <div>Low</div>
                    </div>
                </div>`;
        }

        return code;
    }

    initChart() {
        const config = this.getTypeConfig();
        // const labelColor = getComputedStyle(document.querySelector(":root")).getPropertyValue("--text-regular-color");
        const labelColor = "#6c757d";
        const ctx = this.element.querySelector('canvas').getContext("2d");

        const chartOptions = {
            type: "line",
            data: {
                datasets: this.generateDatasets(),
            },
            options: {
                plugins: {
                    legend: {
                        display: false,
                    },
                    tooltip: {
                        mode: 'index',
                        intersect: false,
                    }
                },
                interaction: {
                    mode: 'index',
                    intersect: false,
                },
                scales: {
                    y: {
                        ticks: {
                            color: labelColor,
                            beginAtZero: true,
                            stepSize: 1
                        },
                        min: 0,
                    },
                    x: {
                        ticks: {
                            color: labelColor,
                        },
                    },
                },
                responsive: true,
                maintainAspectRatio: false,
            },
        };

        if (config.hasStepSize) {
            chartOptions.options.scales.y.ticks.stepSize = 1;
        }

        this.chart = new Chart(ctx, chartOptions);
    }

    updateChart() {
        if (this.chart) {
            this.chart.data.datasets = this.generateDatasets();
            this.chart.update();
        }
    }

    render() {
        const div = document.createElement('div');
        div.className = this.col.length == 0 ? `col-12 col-md-7` : this.col.join(' ');
        const card = document.createElement('div');
        card.className = "border shadow-sm pt-2 h-100";
        card.innerHTML = this.generateHTML();
        div.appendChild(card);

        if (this.element) {
            this.element.replaceWith(div);
        } else {
            this.targetElement.append(div);
        }
        this.element = div;

        setTimeout(() => {
            this.initChart();
        }, 100);
    }

    init() {
        this.render();
    }
}

export class DetailTableWidget {
    constructor(type, targetElement) {
        this.type = type;
        this.targetElement = targetElement;
        this.element = null;
        this.data = [];

        const cat_id_index = {
            39: "Proxy Avoidance",
            73: "Malicious Software",
            74: "Spyware",
            75: "Phishing",
            76: "Spam",
            77: "Adware",
            78: "Malware Accomplic",
            79: "Disease Vector",
            80: "Cookies",
            81: "Dialers",
            82: "Hacking",
            83: "Joke Program",
            84: "Password Cracking Apps",
            85: "Remote Access",
            86: "Made for AdSense sites",
            91: "C&C Server",
            92: "Malicious Domain",
            94: "Scam",
            95: "Ransomware",
        };

        const directType = {0: "", 1: "Device Infected", 2: "External Attacks"};

        const clinetDiv = function (data, type, row) {
            let targetValue = data;
            let targetName = "";
            if (row.client !== null && row.client !== undefined) {
                targetName = (row.client.nickName !== "") ? row.client.nickName : row.client.name;
                return `<div class="client-name-group">
                                        <div class="client-name">${targetName}</div>
                                        <div class="client-info">${data}</div>
                                    </div>`;
            } else {
                return `<div class="client-name">${targetValue}</div>`;
            }
        }

        this.configs = {
            dpi_mals: {
                containerId: 'mals_detail',
                columns: [
                    {
                        title: '<#diskUtility_time#>',
                        data: 'format_timestamp',
                        className: 'dt-head-left dt-body-left',
                    },
                    {
                        title: '<#AiProtection_event_Source#>',
                        data: 'mac',
                        render: clinetDiv
                    },
                    {title: '<#AiProtection_event_Destination#>', data: 'dst_ip'},
                    {
                        title: 'White List',
                        data: null,
                        sortable: false,
                        render: function (data, type, row) {
                            return `<div role="icon" class="icon-size-24 icon-add-circle me-3" data-bind="add_white_list"></div>`;
                        }
                    }
                ],
                hasWhitelistButton: true,
                hasManageWhitelist: true,
                fileName: 'MaliciousSitesBlocking.csv',
                exportId: 'mals_db_export',
            },
            dpi_vp: {
                containerId: 'vp_detail',
                columns: [
                    {
                        title: '<#diskUtility_time#>',
                        data: 'format_timestamp',
                        className: 'dt-head-left dt-body-left',
                    },
                    {
                        title: '<#AiProtection_level_th#>', data: 'severity',
                        render: function (data, type, row) {
                            if (data == "H") {
                                return "High"
                            } else if (data == "M") {
                                return "Medium"
                            } else if (data == "L") {
                                return "Low"
                            } else {
                                ""
                            }
                        }
                    },
                    {
                        title: 'Type', data: 'dir',
                        render: function (data, type, row) {
                            return directType[data]
                        }
                    },
                    {title: '<#AiProtection_event_Source#>', data: 'mac', render: clinetDiv},
                    {title: '<#AiProtection_event_Destination#>', data: 'dst_ip'},
                    {title: '<#AiProtection_event_Threat#>', data: 'msg'}
                ],
                hasWhitelistButton: false,
                hasManageWhitelist: false,
                fileName: 'IntrusionPreventionSystem.csv',
                dbType: 'vp_db',
                exportId: 'vp_db_export',
            },
            dpi_cc: {
                containerId: 'cc_detail',
                columns: [
                    {
                        title: '<#diskUtility_time#>',
                        data: 'format_timestamp',
                        className: 'dt-head-left dt-body-left',
                    },
                    {title: '<#AiProtection_event_Source#>', data: 'mac', render: clinetDiv},
                    {title: '<#AiProtection_event_Destination#>', data: 'dst_ip'}
                ],
                hasWhitelistButton: false,
                hasManageWhitelist: false,
                fileName: 'InfectedDevicePreventBlock.csv',
                dbType: 'cc_db',
                exportId: 'cc_db_export',
            },
            ark_mals: {
                containerId: 'mals_detail',
                columns: [
                    {
                        title: '<#diskUtility_time#>',
                        data: 'format_timestamp',
                        className: 'dt-head-left dt-body-left',
                    },
                    {
                        title: '<#AiProtection_event_Source#>',
                        data: 'mac',
                        render: clinetDiv
                    },
                    {
                        title: '<#AiProtection_event_Destination#>',
                        data: 'domain',
                    }
                ],
                hasWhitelistButton: false,
                hasManageWhitelist: false,
                fileName: 'MaliciousSitesBlocking.csv',
                dbType: 'mals_db',
                exportId: 'mals_db_export',
            },
            ark_adblock: {
                containerId: 'adblock_detail',
                columns:
                    [
                        {
                            title: '<#diskUtility_time#>',
                            data: 'format_timestamp',
                            className: 'dt-head-left dt-body-left',
                        },
                        {title: '<#AiProtection_event_Source#>', data: 'mac', render: clinetDiv},
                        {title: '<#AiProtection_event_Destination#>', data: 'domain'}
                    ],
                hasWhitelistButton: false,
                hasManageWhitelist: false,
                fileName: 'AdblockBlocking.csv',
                dbType: 'adblock_db',
                exportId: 'adblock_db_export',
            }
            ,
            ark_tracker: {
                containerId: 'tracker_detail',
                columns:
                    [
                        {
                            title: '<#diskUtility_time#>',
                            data: 'format_timestamp',
                            className: 'dt-head-left dt-body-left',
                        },
                        {title: '<#AiProtection_event_Source#>', data: 'mac', render: clinetDiv},
                        {title: '<#AiProtection_event_Destination#>', data: 'domain'}
                    ],
                hasWhitelistButton: false,
                hasManageWhitelist: false,
                fileName: 'TrackerBlocking.csv',
                dbType: 'tracker_db',
                exportId: 'tracker_db_export',

            }
        }
        ;
    }

    setEraseDbCallback(callback) {
        this.eraseDbCallback = callback;
    }

    updateDataset(data) {
        this.data = data;
        this.render();
    }

    getClientName(mac) {
        const client = system.client.detail;
        const macUpperCase = mac.toUpperCase();
        return client[macUpperCase] ? client[macUpperCase].name : macUpperCase;
    }

    generateHTML() {
        const config = this.getTypeConfig();

        let code = `
                <h5 class="card-header d-flex align-items-center justify-content-between">
                    <#AiProtection_eventdetails#>
                </h5>
                <div class="card-body">
                    <div>
                        <table id="${this.type}_datatable" class="table align-middle table-responsive">
                        </table>
                    </div>
                </div>
                <div class="card-footer d-flex justify-content-end align-items-center">
                    <div role="icon" class="icon-size-24 icon-delete me-3" title="<#CTL_del#>" data-bind="empty_db" data-type="${this.type}"></div>
                    ${config.hasManageWhitelist ? '<div role="icon" class="icon-size-24 icon-edit-document me-3" title="Manage Whitelist" id="manage_whiteList"></div>' : ''}
                    <div role="icon" class="csv-export-btn icon-size-24 icon-export-file me-3" title="<#CTL_onlysave#>" id="${config.exportId}"></div>
                </div>`;

        return code;
    }

    bindEvents() {
        const config = this.getTypeConfig();

        if (config.hasWhitelistButton) {
            for (let target of document.querySelectorAll("[data-bind='add_white_list']")) {
                target.addEventListener("click", function () {
                    let url = this.getAttribute("data-key");
                    if (whitelist[url]) {
                        return false;
                    }

                    $.ajax({
                        url: "/wrs_wbl.cgi?action=add&type=0&url_type=url&url=" + url,
                        type: "POST",
                        success: function (response) {
                            whitelist[url] = true;
                        },
                    });
                });
            }
        }

        const emptyDbButton = this.element.querySelector("[data-bind='empty_db']");
        if (emptyDbButton) {
            const self = this;
            emptyDbButton.addEventListener("click", function () {
                self.eraseDbCallback(this.getAttribute("data-type"));
            });
        }

        if (config.hasManageWhitelist) {
            document.getElementById("manage_whiteList").addEventListener("click", function () {
                manageWhiteList();
            });
        }

        this.element.querySelector('.csv-export-btn').addEventListener("click", () => {
            this.exportCsv();
        });
    }

    exportCsv() {
        const config = this.getTypeConfig();
        const tableId = `${this.type}_datatable`;
        const table = $(`#${tableId}`).DataTable();

        let aTag = document.createElement("a");

        // Get column headers, excluding White List column
        const headers = config.columns
            .filter(col => col.data !== null)
            .map(col => col.title || col);
        let content = headers.join(',') + "\n";

        // Get all current table data (including search and pagination filtered)
        const tableData = table.rows({search: 'applied'}).data().toArray();

        function sanitizeForCsv(value) {
            if (typeof value !== 'string') return value;

            // If value starts with potentially dangerous characters, prepend single quote
            if (/^[=+\-@]/.test(value)) {
                return "'" + value;
            }

            // If contains comma, double quote or newline, wrap in double quotes and escape internal double quotes
            if (/[",\r\n]/.test(value)) {
                return '"' + value.replace(/"/g, '""') + '"';
            }

            return value;
        }

        for (let item of tableData) {
            const row = [];
            config.columns.forEach((col, index) => {
                const dataKey = col.data;
                let value = "";

                if (dataKey) {
                    // If render function exists, use rendered data
                    if (col.render && typeof col.render === 'function') {
                        // For MAC address field, use original MAC address directly
                        if (dataKey === 'mac') {
                            value = item[dataKey] || "";
                        } else {
                            value = col.render(item[dataKey], 'export', item);
                        }
                    } else {
                        value = item[dataKey] || "";
                    }
                } else {
                    // Skip columns without data (like White List button)
                    return;
                }

                // Clean HTML tags, keep text content only
                if (typeof value === 'string') {
                    value = value.replace(/<[^>]*>/g, '');
                    // Prevent CSV injection attacks
                    value = sanitizeForCsv(value);
                }

                row.push(value);
            });

            if (row.length > 0) {
                content += row.join(",") + "\n";
            }
        }

        // Add UTF-8 BOM for Excel to recognize encoding correctly
        const BOM = '\uFEFF';
        const csvContent = BOM + content;

        let mimeType = "text/csv;charset=utf-8";
        if (navigator.msSaveBlob) {
            return navigator.msSaveBlob(new Blob([csvContent], {type: mimeType}), config.fileName);
        } else if ("download" in aTag) {
            const blob = new Blob([csvContent], {type: mimeType});
            const url = URL.createObjectURL(blob);
            aTag.href = url;
            aTag.setAttribute("download", config.fileName);
            document.getElementById(config.exportId).appendChild(aTag);
            setTimeout(function () {
                document.getElementById(config.exportId).removeChild(aTag);
                aTag.click();
                URL.revokeObjectURL(url);
            }, 66);
            return true;
        } else {
            let f = document.createElement("iframe");
            document.getElementById(config.exportId).appendChild(f);
            f.src = "data:" + mimeType + "," + encodeURIComponent(csvContent);
            setTimeout(function () {
                document.getElementById(config.exportId).removeChild(f);
            }, 333);
            return true;
        }
    }

    getTypeConfig() {
        return this.configs[this.type];
    }

    getColumnConfigs() {
        const config = this.getTypeConfig();
        return config.columns;
    }

    initDataTable() {
        const config = this.getTypeConfig();
        const tableId = `${this.type}_datatable`;

        // 確保銷毀現有的 DataTable
        if ($.fn.DataTable.isDataTable(`#${tableId}`)) {
            $(`#${tableId}`).DataTable().destroy();
        }

        // 準備 columns 配置
        const columns = this.getColumnConfigs()

        // 初始化 DataTable
        $(`#${tableId}`).DataTable({
            data: this.data,
            columns: columns,
            responsive: true,
            pageLength: 5,
            lengthMenu: [5, 10, 25, 50],
            order: [[0, 'desc']], // 按時間降序排列
            dom: `<"row mb-1"<"col-sm-6"l><"col-sm-6"<"d-flex justify-content-end gap-2"f>>>rt<"row mt-1"<"col-sm-6"i><"col-sm-6"<"d-flex justify-content-end"p>>>`,
            language: {
                info: `_START_ - _END_ <#Table_Info_NoData#>`,
                infoEmpty: `<#Table_Info_NoData#>`,
                infoFiltered: "(<#Table_Info_FilterData#>)",
                emptyTable: `<#No_Data_In_Table#>`,
                lengthMenu: `<div class="d-flex align-items-center gap-2"><div><#Rows_Per_Page#>:</div><div>_MENU_</div></div>`,
                search: '',
                paginate: {
                    first: '',
                    previous: `<i class="icon icon-previous icon-size-24"></i>`,
                    next: '<i class="icon icon-next icon-size-24"></i>',
                    last: '',
                },
                select: {
                    rows: {
                        0: '',
                        _: '%d client selected',
                    }
                }
            },
            drawCallback: () => {
                // 修正空表格的 colspan，包括 resize 時
                setTimeout(() => {
                    const emptyCell = this.element.querySelector('.dt-empty');
                    if (emptyCell) {
                        const visibleColumns = this.element.querySelectorAll(`#${tableId} thead th:not(.dtr-hidden)`).length;
                        emptyCell.setAttribute('colspan', visibleColumns || columns.length);
                    }
                }, 10);

                // 重新綁定白名單按鈕事件
                if (config.hasWhitelistButton) {
                    this.bindWhitelistEvents();
                }
            },
            layout: {
                topStart: null,
                topEnd: null,
                bottomStart: ['info'],
                bottomEnd: {
                    paging: {
                        numbers: 0
                    }
                }
            },
            initComplete: (settings, json) => {
                const search = this.element.querySelector('.dt-search');
                if (search) {
                    const search_icon = document.createElement('div');
                    search_icon.classList.add('search-icon');
                    search_icon.innerHTML = '<i class="icon icon-search icon-size-24"></i>';
                    search.prepend(search_icon);
                }
                const search_input = this.element.querySelector('.dt-search .dt-input');
                if (search_input) {
                    search_input.classList.add("border-0");
                }
            },
        });
    }

    bindWhitelistEvents() {
        document.querySelectorAll("[data-bind='add_white_list']").forEach(target => {
            // Remove old event listeners to avoid duplicate binding
            target.replaceWith(target.cloneNode(true));
        });

        // Re-bind events
        const self = this;
        document.querySelectorAll("[data-bind='add_white_list']").forEach(target => {
            target.addEventListener("click", function () {
                const row = this.closest('tr');
                const tableId = `${self.type}_datatable`;
                const table = $(`#${tableId}`).DataTable();
                const rowData = table.row(row).data();

                let url = rowData.dst_ip || rowData.domain;
                if (whitelist[url]) {
                    alert(`${url} is already in the whitelist.`)
                    return false;
                }

                $.ajax({
                    url: "/wrs_wbl.cgi?action=add&type=0&url_type=url&url=" + url,
                    type: "POST",
                    success: function (response) {
                        whitelist[url] = true;
                        alert(`Successfully added ${url} to the whitelist.`)
                    },
                });
            });
        });
    }

    render() {
        const div = document.createElement('div');
        div.className = "col-12 my-3";
        const card = document.createElement('div');
        card.className = "border shadow-sm pt-2 h-100";
        card.innerHTML = this.generateHTML();
        div.appendChild(card);

        if (this.element) {
            this.element.replaceWith(div);
        } else {
            this.targetElement.append(div);
        }

        this.element = div;
        this.bindEvents();

        setTimeout(() => {
            this.initDataTable();
        }, 100);
    }

    init() {
        this.render();
    }
}

export class FeatureSwitch {
    constructor(type, _nvram) {
        this.type = type;
        this.element = null;
        this.nvram = _nvram;

        this.configs = {
            dpi_mals: {
                title: `<#AiProtection_sites_blocking#>`,
                desc: `<#AiProtection_sites_block_desc#>`,
                id: 'mals_switch',
                icon: 'icon-malicious',
                enableKey: 'wrs_mals_enable'
            },
            dpi_vp: {
                title: `<#AiProtection_two-way_IPS#>`,
                desc: `<#AiProtection_two-way_IPS_desc#>`,
                id: 'vp_switch',
                icon: 'icon-twowayips',
                enableKey: 'wrs_vp_enable'
            },
            dpi_cc: {
                title: `<#AiProtection_detection_blocking#>`,
                desc: `<#AiProtection_detection_block_desc#>`,
                id: 'cc_switch',
                icon: 'icon-deviceprevention',
                enableKey: 'wrs_cc_enable'
            },
            ark_mals: {
                title: `<#AiProtection_ARK_Malicious_Site_blocking#>`,
                desc: `<#AiProtection_ARK_Malicious_Site_blocking_desc#>`,
                id: 'ark_mals_switch',
                icon: 'icon-malicious',
            },
            ark_adblock: {
                title: `<#AiProtection_AdvertisementBlocking#>`,
                desc: `<#AiProtection_AdvertisementBlocking_Desc#>`,
                id: 'ark_adblock_switch',
                icon: 'icon-adblock',
            },
            ark_tracker: {
                title: `<#AiProtection_TrackerBlocking#>`,
                desc: `<#AiProtection_TrackerBlocking_Desc#>`,
                id: 'ark_tracker_switch',
                icon: 'icon-tracker',
            }
        }
    }

    setTMEulaCallback(callback) {
        this.tmEulaCallback = callback;
    }

    isFeatureEnabled(featureName) {
        const enableKey = `${featureName}_enable`;
        return this.nvram[enableKey] === "1";
    }

    isDpiFeatureEnabled(featureName) {
        const config = this.configs[featureName];
        if (!config || !config.enableKey) return false;
        return this.nvram[config.enableKey] === "1";
    }

    generateFeatureHtml(featureName, isArk = true) {
        const feature = this.configs[featureName];
        const isEnabled = isArk
            ? this.isFeatureEnabled(featureName)
            : (this.isDpiFeatureEnabled(featureName) && this.nvram.TM_EULA !== "0");
        const title = feature.title || feature.titleKey;
        const desc = feature.desc || feature.descKey;

        return `
            <div class="d-flex flex-row ${isSupport("UI4") ? `align-items-start` : `align-items-center`} justify-content-around">
                <div class="d-flex flex-row align-items-start gap-2">
                    <div class="icon ${feature.icon} text-primary icon-size-28"></div>
                    <span class="d-flex flex-column align-items-start">
                        <div class="feature-title">${title}</div>
                        <div class="feature-desc">${desc}</div>
                    </span>
                </div>
                <div class="form-check form-switch ms-auto">
                    <div id="${feature.id}" class="toggle-button ${isEnabled ? "active" : ""}">
                        <div class="toggle-button-handle"></div>
                    </div>
                </div>
            </div>
        `;
    }

    handleArkFeatureToggle(featureName, element) {
        element.classList.toggle("active");
        const active = element.classList.contains("active");
        const value = active ? "1" : "0";

        Object.assign(this.nvram, {
            [`${featureName}_enable`]: value,
        });
        top.showLoading();

        httpApi.nvramSet({
            [`${featureName}_enable`]: value,
            "wrs_protect_enable": "1",
            "action_mode": "apply",
            "rc_service": "restart_ark",
        }, () => {
            top.hideLoading();
        })

    }

    handleDpiFeatureToggle(featureName, element) {
        element.classList.toggle("active");
        const active = element.classList.contains("active");
        const value = active ? "1" : "0";

        if (this.nvram.TM_EULA === "0" || this.nvram.TM_EULA_time === "") {
            this.tmEulaCallback(featureName);
            return;
        }

        const feature = this.configs[featureName];
        if (!feature || !feature.enableKey) {
            console.error(`No config or enableKey found for feature: ${featureName}`);
            return;
        }

        let applyObj = {
            action_mode: "apply",
            rc_service: "restart_wrs;restart_firewall",
            wrs_protect_enable: "1",
            [feature.enableKey]: value,
            action_time: 4,
        };
        top.showLoading();

        const {ctf_disable, ctf_fa_mode} = this.nvram;
        if ((ctf_disable == 0 && ctf_fa_mode == 2) || system.modelName === "MAP-AC1750") {
            applyObj.rc_service = "reboot";
            applyObj.action_time = httpApi.hookGet("get_default_reboot_time");
        }

        httpApi.nvramSet(applyObj, () => {
            hideLoading();
        });
    }

    // Bind event listeners
    bindEventListeners() {
        const config = this.configs[this.type];
        if (!config || !this.element) return;

        const toggleButton = this.element.querySelector(`#${config.id}`);
        if (toggleButton) {
            const isArk = this.type.startsWith('ark_');
            toggleButton.addEventListener("click", (event) => {
                if (isArk) {
                    this.handleArkFeatureToggle(this.type, event.target);
                } else {
                    this.handleDpiFeatureToggle(this.type, event.target);
                }
            });
        }
    }

    render() {
        const config = this.configs[this.type];
        if (!config) {
            console.warn(`No config found for feature type: ${this.type}`);
            return;
        }

        const isArk = this.type.startsWith('ark_');
        const featureHtml = this.generateFeatureHtml(this.type, isArk);

        const div = document.createElement('div');
        div.className = 'feature-switch-container mb-3';
        div.innerHTML = featureHtml;

        this.element = div;
        this.bindEventListeners();

        return this.element;
    }

    init() {
        this.render();
    }
}