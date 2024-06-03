const ASUS_CLIENTLIST = {

    ClientlistDivStyle: `<style>
        :root {
            --color-primary: #006CE1;
            
            --primary-blue-5000: #248DFF;
        
            --tuf-color-primary: #FFAA32;
        
            --rog-color-primary: #FF3535;
        }
        
        .popup_bg {
            position: fixed;
            top: 0;
            right: 0;
            bottom: 0;
            left: 0;
            z-index: 2000;
            background: rgba(7, 7, 7, 0.54);
            backdrop-filter: blur(6px);
            -webkit-backdrop-filter: blur(6px);
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
        
        .modal.show .modal-dialog {
            transform: none;
        }
        .modal.fade .modal-dialog {
            transition: transform .3s ease-out;
            transform: translate(0,-50px);
        }
        
        .modal .close_btn {
            --close-btn-svg:url("data:image/svg+xml,%3Csvg width='28' height='28' viewBox='0 0 28 28' fill='none' xmlns='http://www.w3.org/2000/svg'%3E%3Cpath fill-rule='evenodd' clip-rule='evenodd' d='M14 2.333A11.656 11.656 0 0 0 2.333 14 11.656 11.656 0 0 0 14 25.667 11.656 11.656 0 0 0 25.667 14 11.656 11.656 0 0 0 14 2.333zm5.833 15.855l-1.645 1.645L14 15.645l-4.188 4.188-1.645-1.645L12.355 14 8.167 9.812l1.645-1.645L14 12.355l4.188-4.188 1.645 1.645L15.645 14l4.188 4.188z' fill='%23818181'/%3E%3C/svg%3E");
            width: 28px;
            height: 28px;
            mask-image: var(--close-btn-svg);
            mask-repeat: no-repeat;
            mask-position: center;
            mask-size: contain;
            -webkit-mask-image: var(--close-btn-svg);
            -webkit-mask-repeat: no-repeat;
            -webkit-mask-position: center;
            -webkit-mask-size: contain;
            background-color: #818181;
        }
        
        .modal .close_btn:hover {
            background-color: #8d8d8d;
        }
    
        .modal-dialog {
            position: relative;
            width: 80vw;
            margin: 4rem auto;
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
            background-color: transparent;
            border: transparent;
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
            align-items: center;
            -webkit-box-pack: justify;
            -ms-flex-pack: justify;
            justify-content: space-between;
            padding: 1rem;
            border: transparent;
            border-top-left-radius: 0.3rem;
            border-top-right-radius: 0.3rem;
        }
    
        .modal-header .close {
            padding: 1rem;
            margin: -1rem -1rem -1rem auto;
        }
    
        .modal-title {
            color: #000000;
            font-weight: bold;
            font-size: 32px;
            margin: 0;
        }
    
        .modal-body {
            color: #000000;
            background: #FFFFFF;
            box-shadow: 0px 2px 2px 0px rgba(0, 0, 0, 0.04), 0px 1px 5px 0px rgba(0, 0, 0, 0.08), 0px 3px 1px 0px rgba(0, 0, 0, 0.06);
            border-radius: 10px;
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
    
        .btn.disabled, .btn:disabled {
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
       
      </style>`,

    ModalTheme: {
        RT: `<style>
            .modal-title, .modal-body {
                color: #FFFFFF;
            }
            .modal-body {
                color: #FFFFFF;
                background: #444F53;
            }
            .segmented_picker_label {
                color: #FFFFFF;
            }
        </style>`,

        ROG: `<style>
            .popup_bg {
                background: rgba(0, 0, 0, 0.5);
                backdrop-filter: blur(4px);
                -webkit-backdrop-filter: blur(6px);
            }
            .modal-body {
                background-color: #1C1C1E;
                border: 0;
                border-radius: 0;
            }
            .modal-title {
                color: #FFF;
                /*font-family: Xolonium;*/
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
            .modal .close_btn {
                --close-btn-svg:url(data:image/svg+xml,%3Csvg%20xmlns%3D%22http%3A%2F%2Fwww.w3.org%2F2000%2Fsvg%22%20width%3D%2232%22%20height%3D%2232%22%20viewBox%3D%220%200%2032%2032%22%20fill%3D%22none%22%3E%0A%20%20%3Cg%20clip-path%3D%22url%28%23clip0_14_58560%29%22%3E%0A%20%20%20%20%3Cpath%20d%3D%22M6.9248%2025.1456L25.216%206.8544%22%20stroke%3D%22%23989898%22%20stroke-miterlimit%3D%2210%22%2F%3E%0A%20%20%20%20%3Cpath%20d%3D%22M25.0754%2025.1456L6.78418%206.8544%22%20stroke%3D%22%23989898%22%20stroke-miterlimit%3D%2210%22%2F%3E%0A%20%20%20%20%3Cpath%20d%3D%22M0.396973%200H7.52017L6.91217%200.6336H0.972973L0.396973%200Z%22%20fill%3D%22%23989898%22%2F%3E%0A%20%20%20%20%3Cpath%20d%3D%22M0%207.6288V0.633598L0.6208%201.312V6.976L0%207.6288Z%22%20fill%3D%22%23989898%22%2F%3E%0A%20%20%20%20%3Cpath%20d%3D%22M31.5394%200H24.397L25.005%200.6336H30.957L31.5394%200Z%22%20fill%3D%22%23989898%22%2F%3E%0A%20%20%20%20%3Cpath%20d%3D%22M31.9998%207.2128V0.0767975L31.3662%200.767998V6.5536L31.9998%207.2128Z%22%20fill%3D%22%23989898%22%2F%3E%0A%20%20%20%20%3Cpath%20d%3D%22M0.45459%2032H7.44979L6.85459%2031.3792H1.02419L0.45459%2032Z%22%20fill%3D%22%23989898%22%2F%3E%0A%20%20%20%20%3Cpath%20d%3D%22M0%2024.928V31.9232L0.6208%2031.2448V25.5808L0%2024.928Z%22%20fill%3D%22%23989898%22%2F%3E%0A%20%20%20%20%3Cpath%20d%3D%22M31.5265%2032H24.4097L25.0113%2031.3664H30.9505L31.5265%2032Z%22%20fill%3D%22%23989898%22%2F%3E%0A%20%20%20%20%3Cpath%20d%3D%22M31.9998%2024.864V32L31.3662%2031.3088V25.5232L31.9998%2024.864Z%22%20fill%3D%22%23989898%22%2F%3E%0A%20%20%3C%2Fg%3E%0A%20%20%3Cdefs%3E%0A%20%20%20%20%3CclipPath%20id%3D%22clip0_14_58560%22%3E%0A%20%20%20%20%20%20%3Crect%20width%3D%2232%22%20height%3D%2232%22%20fill%3D%22white%22%2F%3E%0A%20%20%20%20%3C%2FclipPath%3E%0A%20%20%3C%2Fdefs%3E%0A%3C%2Fsvg%3E);
                width: 42px;
                height: 42px;
            }
        </style>`,

        TUF: `<style>
            .popup_bg {
                background: rgba(0, 0, 0, 0.5);
                backdrop-filter: blur(4px);
                -webkit-backdrop-filter: blur(6px);
            }
            .modal-body {
                background-color: #1C1C1E;
                border: 0;
                border-radius: 0;
            }
            .modal-title {
                color: #FFF;
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
            .modal .close_btn {
                --close-btn-svg:url(data:image/svg+xml,%3Csvg%20xmlns%3D%22http%3A%2F%2Fwww.w3.org%2F2000%2Fsvg%22%20width%3D%2232%22%20height%3D%2232%22%20viewBox%3D%220%200%2032%2032%22%20fill%3D%22none%22%3E%0A%20%20%3Cg%20clip-path%3D%22url%28%23clip0_14_58560%29%22%3E%0A%20%20%20%20%3Cpath%20d%3D%22M6.9248%2025.1456L25.216%206.8544%22%20stroke%3D%22%23989898%22%20stroke-miterlimit%3D%2210%22%2F%3E%0A%20%20%20%20%3Cpath%20d%3D%22M25.0754%2025.1456L6.78418%206.8544%22%20stroke%3D%22%23989898%22%20stroke-miterlimit%3D%2210%22%2F%3E%0A%20%20%20%20%3Cpath%20d%3D%22M0.396973%200H7.52017L6.91217%200.6336H0.972973L0.396973%200Z%22%20fill%3D%22%23989898%22%2F%3E%0A%20%20%20%20%3Cpath%20d%3D%22M0%207.6288V0.633598L0.6208%201.312V6.976L0%207.6288Z%22%20fill%3D%22%23989898%22%2F%3E%0A%20%20%20%20%3Cpath%20d%3D%22M31.5394%200H24.397L25.005%200.6336H30.957L31.5394%200Z%22%20fill%3D%22%23989898%22%2F%3E%0A%20%20%20%20%3Cpath%20d%3D%22M31.9998%207.2128V0.0767975L31.3662%200.767998V6.5536L31.9998%207.2128Z%22%20fill%3D%22%23989898%22%2F%3E%0A%20%20%20%20%3Cpath%20d%3D%22M0.45459%2032H7.44979L6.85459%2031.3792H1.02419L0.45459%2032Z%22%20fill%3D%22%23989898%22%2F%3E%0A%20%20%20%20%3Cpath%20d%3D%22M0%2024.928V31.9232L0.6208%2031.2448V25.5808L0%2024.928Z%22%20fill%3D%22%23989898%22%2F%3E%0A%20%20%20%20%3Cpath%20d%3D%22M31.5265%2032H24.4097L25.0113%2031.3664H30.9505L31.5265%2032Z%22%20fill%3D%22%23989898%22%2F%3E%0A%20%20%20%20%3Cpath%20d%3D%22M31.9998%2024.864V32L31.3662%2031.3088V25.5232L31.9998%2024.864Z%22%20fill%3D%22%23989898%22%2F%3E%0A%20%20%3C%2Fg%3E%0A%20%20%3Cdefs%3E%0A%20%20%20%20%3CclipPath%20id%3D%22clip0_14_58560%22%3E%0A%20%20%20%20%20%20%3Crect%20width%3D%2232%22%20height%3D%2232%22%20fill%3D%22white%22%2F%3E%0A%20%20%20%20%3C%2FclipPath%3E%0A%20%20%3C%2Fdefs%3E%0A%3C%2Fsvg%3E);
                width: 42px;
                height: 42px;
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
    },
}

class ClientlistModel extends HTMLElement {
    constructor() {
        super();
        this.attachShadow({mode: 'open'});
        const template = document.createElement('template');
        template.innerHTML = `
            ${ASUS_CLIENTLIST.ClientlistDivStyle}
            <div class="popup_bg">
                <div class="modal">
                    <div class="modal-dialog modal-xl">
                        <div class="modal-content">
                            <div class="modal-header">
                                <div class="modal-title"><#ConnectedClient#></div>
                                <div class="close_btn" data-bs-dismiss="modal" aria-label="Close"></div>
                            </div>
                            <div class="modal-body">
                                <iframe style="width:100%;height:70vh;border: 0;"></iframe>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        `;

        const theme = ASUS_CLIENTLIST.getTheme();
        if (theme !== "") {
            template.innerHTML += ASUS_CLIENTLIST.ModalTheme[theme];
        }

        this.shadowRoot.appendChild(template.content.cloneNode(true));
        this.div = this.shadowRoot.querySelector('div');
        this.shadowRoot.querySelector('.close_btn').addEventListener('click', this.handleModalClose.bind(this));
        const iframe = this.shadowRoot.querySelector('iframe');
        iframe.src = '/device-map/clientlist.html';
        iframe.addEventListener('load', function () {
            iframe.contentWindow.postMessage({theme: theme}, '*');
        });

    }

    handleModalClose = () => {
        top.document.getElementById('clientlist_modal').remove();
        top.document.body.style.removeProperty('overflow');
    }

    connectedCallback() {

    }

    show = () => {
        if (top.document.getElementById('clientlist_modal') == null) {
            top.document.body.style.overflow = 'hidden';
            const modal = document.createElement('div');
            modal.id = 'clientlist_modal';
            top.document.body.appendChild(modal);
            modal.appendChild(this);
        }
    }

}

customElements.define('clientlist-model', ClientlistModel);
