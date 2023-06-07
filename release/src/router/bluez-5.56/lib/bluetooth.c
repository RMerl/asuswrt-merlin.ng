// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2000-2001  Qualcomm Incorporated
 *  Copyright (C) 2002-2003  Maxim Krasnyansky <maxk@qualcomm.com>
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>

#include "bluetooth.h"
#include "hci.h"

void baswap(bdaddr_t *dst, const bdaddr_t *src)
{
	register unsigned char *d = (unsigned char *) dst;
	register const unsigned char *s = (const unsigned char *) src;
	register int i;

	for (i = 0; i < 6; i++)
		d[i] = s[5-i];
}

char *batostr(const bdaddr_t *ba)
{
	char *str = bt_malloc(18);
	if (!str)
		return NULL;

	sprintf(str, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
		ba->b[0], ba->b[1], ba->b[2],
		ba->b[3], ba->b[4], ba->b[5]);

	return str;
}

bdaddr_t *strtoba(const char *str)
{
	bdaddr_t b;
	bdaddr_t *ba = bt_malloc(sizeof(*ba));

	if (ba) {
		str2ba(str, &b);
		baswap(ba, &b);
	}

	return ba;
}

int ba2str(const bdaddr_t *ba, char *str)
{
	return sprintf(str, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
		ba->b[5], ba->b[4], ba->b[3], ba->b[2], ba->b[1], ba->b[0]);
}

/* Match kernel's lowercase printing of mac address (%pMR) */
int ba2strlc(const bdaddr_t *ba, char *str)
{
	return sprintf(str, "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x",
		ba->b[5], ba->b[4], ba->b[3], ba->b[2], ba->b[1], ba->b[0]);
}

int str2ba(const char *str, bdaddr_t *ba)
{
	int i;

	if (bachk(str) < 0) {
		memset(ba, 0, sizeof(*ba));
		return -1;
	}

	for (i = 5; i >= 0; i--, str += 3)
		ba->b[i] = strtol(str, NULL, 16);

	return 0;
}

int ba2oui(const bdaddr_t *ba, char *str)
{
	return sprintf(str, "%2.2X-%2.2X-%2.2X", ba->b[5], ba->b[4], ba->b[3]);
}

int bachk(const char *str)
{
	if (!str)
		return -1;

	if (strlen(str) != 17)
		return -1;

	while (*str) {
		if (!isxdigit(*str++))
			return -1;

		if (!isxdigit(*str++))
			return -1;

		if (*str == 0)
			break;

		if (*str++ != ':')
			return -1;
	}

	return 0;
}

int baprintf(const char *format, ...)
{
	va_list ap;
	int len;

	va_start(ap, format);
	len = vprintf(format, ap);
	va_end(ap);

	return len;
}

int bafprintf(FILE *stream, const char *format, ...)
{
	va_list ap;
	int len;

	va_start(ap, format);
	len = vfprintf(stream, format, ap);
	va_end(ap);

	return len;
}

int basprintf(char *str, const char *format, ...)
{
	va_list ap;
	int len;

	va_start(ap, format);
	len = vsnprintf(str, (~0U) >> 1, format, ap);
	va_end(ap);

	return len;
}

int basnprintf(char *str, size_t size, const char *format, ...)
{
	va_list ap;
	int len;

	va_start(ap, format);
	len = vsnprintf(str, size, format, ap);
	va_end(ap);

	return len;
}

void *bt_malloc(size_t size)
{
	return malloc(size);
}

void *bt_malloc0(size_t size)
{
	return calloc(size, 1);
}

void bt_free(void *ptr)
{
	free(ptr);
}

/* Bluetooth error codes to Unix errno mapping */
int bt_error(uint16_t code)
{
	switch (code) {
	case 0:
		return 0;
	case HCI_UNKNOWN_COMMAND:
		return EBADRQC;
	case HCI_NO_CONNECTION:
		return ENOTCONN;
	case HCI_HARDWARE_FAILURE:
		return EIO;
	case HCI_PAGE_TIMEOUT:
		return EHOSTDOWN;
	case HCI_AUTHENTICATION_FAILURE:
		return EACCES;
	case HCI_PIN_OR_KEY_MISSING:
		return EINVAL;
	case HCI_MEMORY_FULL:
		return ENOMEM;
	case HCI_CONNECTION_TIMEOUT:
		return ETIMEDOUT;
	case HCI_MAX_NUMBER_OF_CONNECTIONS:
	case HCI_MAX_NUMBER_OF_SCO_CONNECTIONS:
		return EMLINK;
	case HCI_ACL_CONNECTION_EXISTS:
		return EALREADY;
	case HCI_COMMAND_DISALLOWED:
	case HCI_TRANSACTION_COLLISION:
	case HCI_ROLE_SWITCH_PENDING:
		return EBUSY;
	case HCI_REJECTED_LIMITED_RESOURCES:
	case HCI_REJECTED_PERSONAL:
	case HCI_QOS_REJECTED:
		return ECONNREFUSED;
	case HCI_HOST_TIMEOUT:
		return ETIMEDOUT;
	case HCI_UNSUPPORTED_FEATURE:
	case HCI_QOS_NOT_SUPPORTED:
	case HCI_PAIRING_NOT_SUPPORTED:
	case HCI_CLASSIFICATION_NOT_SUPPORTED:
	case HCI_UNSUPPORTED_LMP_PARAMETER_VALUE:
	case HCI_PARAMETER_OUT_OF_RANGE:
	case HCI_QOS_UNACCEPTABLE_PARAMETER:
		return EOPNOTSUPP;
	case HCI_INVALID_PARAMETERS:
	case HCI_SLOT_VIOLATION:
		return EINVAL;
	case HCI_OE_USER_ENDED_CONNECTION:
	case HCI_OE_LOW_RESOURCES:
	case HCI_OE_POWER_OFF:
		return ECONNRESET;
	case HCI_CONNECTION_TERMINATED:
		return ECONNABORTED;
	case HCI_REPEATED_ATTEMPTS:
		return ELOOP;
	case HCI_REJECTED_SECURITY:
	case HCI_PAIRING_NOT_ALLOWED:
	case HCI_INSUFFICIENT_SECURITY:
		return EACCES;
	case HCI_UNSUPPORTED_REMOTE_FEATURE:
		return EPROTONOSUPPORT;
	case HCI_SCO_OFFSET_REJECTED:
		return ECONNREFUSED;
	case HCI_UNKNOWN_LMP_PDU:
	case HCI_INVALID_LMP_PARAMETERS:
	case HCI_LMP_ERROR_TRANSACTION_COLLISION:
	case HCI_LMP_PDU_NOT_ALLOWED:
	case HCI_ENCRYPTION_MODE_NOT_ACCEPTED:
		return EPROTO;
	default:
		return ENOSYS;
	}
}

const char *bt_compidtostr(int compid)
{
	switch (compid) {
	case 0:
		return "Ericsson Technology Licensing";
	case 1:
		return "Nokia Mobile Phones";
	case 2:
		return "Intel Corp.";
	case 3:
		return "IBM Corp.";
	case 4:
		return "Toshiba Corp.";
	case 5:
		return "3Com";
	case 6:
		return "Microsoft";
	case 7:
		return "Lucent";
	case 8:
		return "Motorola";
	case 9:
		return "Infineon Technologies AG";
	case 10:
		return "Cambridge Silicon Radio";
	case 11:
		return "Silicon Wave";
	case 12:
		return "Digianswer A/S";
	case 13:
		return "Texas Instruments Inc.";
	case 14:
		return "Parthus Technologies Inc.";
	case 15:
		return "Broadcom Corporation";
	case 16:
		return "Mitel Semiconductor";
	case 17:
		return "Widcomm, Inc.";
	case 18:
		return "Zeevo, Inc.";
	case 19:
		return "Atmel Corporation";
	case 20:
		return "Mitsubishi Electric Corporation";
	case 21:
		return "RTX Telecom A/S";
	case 22:
		return "KC Technology Inc.";
	case 23:
		return "Newlogic";
	case 24:
		return "Transilica, Inc.";
	case 25:
		return "Rohde & Schwarz GmbH & Co. KG";
	case 26:
		return "TTPCom Limited";
	case 27:
		return "Signia Technologies, Inc.";
	case 28:
		return "Conexant Systems Inc.";
	case 29:
		return "Qualcomm";
	case 30:
		return "Inventel";
	case 31:
		return "AVM Berlin";
	case 32:
		return "BandSpeed, Inc.";
	case 33:
		return "Mansella Ltd";
	case 34:
		return "NEC Corporation";
	case 35:
		return "WavePlus Technology Co., Ltd.";
	case 36:
		return "Alcatel";
	case 37:
		return "NXP Semiconductors (formerly Philips Semiconductors)";
	case 38:
		return "C Technologies";
	case 39:
		return "Open Interface";
	case 40:
		return "R F Micro Devices";
	case 41:
		return "Hitachi Ltd";
	case 42:
		return "Symbol Technologies, Inc.";
	case 43:
		return "Tenovis";
	case 44:
		return "Macronix International Co. Ltd.";
	case 45:
		return "GCT Semiconductor";
	case 46:
		return "Norwood Systems";
	case 47:
		return "MewTel Technology Inc.";
	case 48:
		return "ST Microelectronics";
	case 49:
		return "Synopsys, Inc.";
	case 50:
		return "Red-M (Communications) Ltd";
	case 51:
		return "Commil Ltd";
	case 52:
		return "Computer Access Technology Corporation (CATC)";
	case 53:
		return "Eclipse (HQ Espana) S.L.";
	case 54:
		return "Renesas Electronics Corporation";
	case 55:
		return "Mobilian Corporation";
	case 56:
		return "Syntronix Corporation";
	case 57:
		return "Integrated System Solution Corp.";
	case 58:
		return "Panasonic Corporation (formerly Matsushita Electric Industrial Co., Ltd.)";
	case 59:
		return "Gennum Corporation";
	case 60:
		return "BlackBerry Limited (formerly Research In Motion)";
	case 61:
		return "IPextreme, Inc.";
	case 62:
		return "Systems and Chips, Inc";
	case 63:
		return "Bluetooth SIG, Inc";
	case 64:
		return "Seiko Epson Corporation";
	case 65:
		return "Integrated Silicon Solution Taiwan, Inc.";
	case 66:
		return "CONWISE Technology Corporation Ltd";
	case 67:
		return "PARROT AUTOMOTIVE SAS";
	case 68:
		return "Socket Mobile";
	case 69:
		return "Atheros Communications, Inc.";
	case 70:
		return "MediaTek, Inc.";
	case 71:
		return "Bluegiga";
	case 72:
		return "Marvell Technology Group Ltd.";
	case 73:
		return "3DSP Corporation";
	case 74:
		return "Accel Semiconductor Ltd.";
	case 75:
		return "Continental Automotive Systems";
	case 76:
		return "Apple, Inc.";
	case 77:
		return "Staccato Communications, Inc.";
	case 78:
		return "Avago Technologies";
	case 79:
		return "APT Ltd.";
	case 80:
		return "SiRF Technology, Inc.";
	case 81:
		return "Tzero Technologies, Inc.";
	case 82:
		return "J&M Corporation";
	case 83:
		return "Free2move AB";
	case 84:
		return "3DiJoy Corporation";
	case 85:
		return "Plantronics, Inc.";
	case 86:
		return "Sony Ericsson Mobile Communications";
	case 87:
		return "Harman International Industries, Inc.";
	case 88:
		return "Vizio, Inc.";
	case 89:
		return "Nordic Semiconductor ASA";
	case 90:
		return "EM Microelectronic-Marin SA";
	case 91:
		return "Ralink Technology Corporation";
	case 92:
		return "Belkin International, Inc.";
	case 93:
		return "Realtek Semiconductor Corporation";
	case 94:
		return "Stonestreet One, LLC";
	case 95:
		return "Wicentric, Inc.";
	case 96:
		return "RivieraWaves S.A.S";
	case 97:
		return "RDA Microelectronics";
	case 98:
		return "Gibson Guitars";
	case 99:
		return "MiCommand Inc.";
	case 100:
		return "Band XI International, LLC";
	case 101:
		return "Hewlett-Packard Company";
	case 102:
		return "9Solutions Oy";
	case 103:
		return "GN Netcom A/S";
	case 104:
		return "General Motors";
	case 105:
		return "A&D Engineering, Inc.";
	case 106:
		return "MindTree Ltd.";
	case 107:
		return "Polar Electro OY";
	case 108:
		return "Beautiful Enterprise Co., Ltd.";
	case 109:
		return "BriarTek, Inc";
	case 110:
		return "Summit Data Communications, Inc.";
	case 111:
		return "Sound ID";
	case 112:
		return "Monster, LLC";
	case 113:
		return "connectBlue AB";
	case 114:
		return "ShangHai Super Smart Electronics Co. Ltd.";
	case 115:
		return "Group Sense Ltd.";
	case 116:
		return "Zomm, LLC";
	case 117:
		return "Samsung Electronics Co. Ltd.";
	case 118:
		return "Creative Technology Ltd.";
	case 119:
		return "Laird Technologies";
	case 120:
		return "Nike, Inc.";
	case 121:
		return "lesswire AG";
	case 122:
		return "MStar Semiconductor, Inc.";
	case 123:
		return "Hanlynn Technologies";
	case 124:
		return "A & R Cambridge";
	case 125:
		return "Seers Technology Co., Ltd.";
	case 126:
		return "Sports Tracking Technologies Ltd.";
	case 127:
		return "Autonet Mobile";
	case 128:
		return "DeLorme Publishing Company, Inc.";
	case 129:
		return "WuXi Vimicro";
	case 130:
		return "Sennheiser Communications A/S";
	case 131:
		return "TimeKeeping Systems, Inc.";
	case 132:
		return "Ludus Helsinki Ltd.";
	case 133:
		return "BlueRadios, Inc.";
	case 134:
		return "Equinux AG";
	case 135:
		return "Garmin International, Inc.";
	case 136:
		return "Ecotest";
	case 137:
		return "GN ReSound A/S";
	case 138:
		return "Jawbone";
	case 139:
		return "Topcon Positioning Systems, LLC";
	case 140:
		return "Gimbal Inc. (formerly Qualcomm Labs, Inc. and Qualcomm Retail Solutions, Inc.)";
	case 141:
		return "Zscan Software";
	case 142:
		return "Quintic Corp";
	case 143:
		return "Telit Wireless Solutions GmbH (formerly Stollmann E+V GmbH)";
	case 144:
		return "Funai Electric Co., Ltd.";
	case 145:
		return "Advanced PANMOBIL systems GmbH & Co. KG";
	case 146:
		return "ThinkOptics, Inc.";
	case 147:
		return "Universal Electronics, Inc.";
	case 148:
		return "Airoha Technology Corp.";
	case 149:
		return "NEC Lighting, Ltd.";
	case 150:
		return "ODM Technology, Inc.";
	case 151:
		return "ConnecteDevice Ltd.";
	case 152:
		return "zero1.tv GmbH";
	case 153:
		return "i.Tech Dynamic Global Distribution Ltd.";
	case 154:
		return "Alpwise";
	case 155:
		return "Jiangsu Toppower Automotive Electronics Co., Ltd.";
	case 156:
		return "Colorfy, Inc.";
	case 157:
		return "Geoforce Inc.";
	case 158:
		return "Bose Corporation";
	case 159:
		return "Suunto Oy";
	case 160:
		return "Kensington Computer Products Group";
	case 161:
		return "SR-Medizinelektronik";
	case 162:
		return "Vertu Corporation Limited";
	case 163:
		return "Meta Watch Ltd.";
	case 164:
		return "LINAK A/S";
	case 165:
		return "OTL Dynamics LLC";
	case 166:
		return "Panda Ocean Inc.";
	case 167:
		return "Visteon Corporation";
	case 168:
		return "ARP Devices Limited";
	case 169:
		return "MARELLI EUROPE S.P.A. (formerly Magneti Marelli S.p.A.)";
	case 170:
		return "CAEN RFID srl";
	case 171:
		return "Ingenieur-Systemgruppe Zahn GmbH";
	case 172:
		return "Green Throttle Games";
	case 173:
		return "Peter Systemtechnik GmbH";
	case 174:
		return "Omegawave Oy";
	case 175:
		return "Cinetix";
	case 176:
		return "Passif Semiconductor Corp";
	case 177:
		return "Saris Cycling Group, Inc";
	case 178:
		return "Bekey A/S";
	case 179:
		return "Clarinox Technologies Pty. Ltd.";
	case 180:
		return "BDE Technology Co., Ltd.";
	case 181:
		return "Swirl Networks";
	case 182:
		return "Meso international";
	case 183:
		return "TreLab Ltd";
	case 184:
		return "Qualcomm Innovation Center, Inc. (QuIC)";
	case 185:
		return "Johnson Controls, Inc.";
	case 186:
		return "Starkey Laboratories Inc.";
	case 187:
		return "S-Power Electronics Limited";
	case 188:
		return "Ace Sensor Inc";
	case 189:
		return "Aplix Corporation";
	case 190:
		return "AAMP of America";
	case 191:
		return "Stalmart Technology Limited";
	case 192:
		return "AMICCOM Electronics Corporation";
	case 193:
		return "Shenzhen Excelsecu Data Technology Co.,Ltd";
	case 194:
		return "Geneq Inc.";
	case 195:
		return "adidas AG";
	case 196:
		return "LG Electronics";
	case 197:
		return "Onset Computer Corporation";
	case 198:
		return "Selfly BV";
	case 199:
		return "Quuppa Oy.";
	case 200:
		return "GeLo Inc";
	case 201:
		return "Evluma";
	case 202:
		return "MC10";
	case 203:
		return "Binauric SE";
	case 204:
		return "Beats Electronics";
	case 205:
		return "Microchip Technology Inc.";
	case 206:
		return "Elgato Systems GmbH";
	case 207:
		return "ARCHOS SA";
	case 208:
		return "Dexcom, Inc.";
	case 209:
		return "Polar Electro Europe B.V.";
	case 210:
		return "Dialog Semiconductor B.V.";
	case 211:
		return "Taixingbang Technology (HK) Co,. LTD.";
	case 212:
		return "Kawantech";
	case 213:
		return "Austco Communication Systems";
	case 214:
		return "Timex Group USA, Inc.";
	case 215:
		return "Qualcomm Technologies, Inc.";
	case 216:
		return "Qualcomm Connected Experiences, Inc.";
	case 217:
		return "Voyetra Turtle Beach";
	case 218:
		return "txtr GmbH";
	case 219:
		return "Biosentronics";
	case 220:
		return "Procter & Gamble";
	case 221:
		return "Hosiden Corporation";
	case 222:
		return "Muzik LLC";
	case 223:
		return "Misfit Wearables Corp";
	case 224:
		return "Google";
	case 225:
		return "Danlers Ltd";
	case 226:
		return "Semilink Inc";
	case 227:
		return "inMusic Brands, Inc";
	case 228:
		return "L.S. Research Inc.";
	case 229:
		return "Eden Software Consultants Ltd.";
	case 230:
		return "Freshtemp";
	case 231:
		return "KS Technologies";
	case 232:
		return "ACTS Technologies";
	case 233:
		return "Vtrack Systems";
	case 234:
		return "Nielsen-Kellerman Company";
	case 235:
		return "Server Technology Inc.";
	case 236:
		return "BioResearch Associates";
	case 237:
		return "Jolly Logic, LLC";
	case 238:
		return "Above Average Outcomes, Inc.";
	case 239:
		return "Bitsplitters GmbH";
	case 240:
		return "PayPal, Inc.";
	case 241:
		return "Witron Technology Limited";
	case 242:
		return "Morse Project Inc.";
	case 243:
		return "Kent Displays Inc.";
	case 244:
		return "Nautilus Inc.";
	case 245:
		return "Smartifier Oy";
	case 246:
		return "Elcometer Limited";
	case 247:
		return "VSN Technologies, Inc.";
	case 248:
		return "AceUni Corp., Ltd.";
	case 249:
		return "StickNFind";
	case 250:
		return "Crystal Code AB";
	case 251:
		return "KOUKAAM a.s.";
	case 252:
		return "Delphi Corporation";
	case 253:
		return "ValenceTech Limited";
	case 254:
		return "Stanley Black and Decker";
	case 255:
		return "Typo Products, LLC";
	case 256:
		return "TomTom International BV";
	case 257:
		return "Fugoo, Inc.";
	case 258:
		return "Keiser Corporation";
	case 259:
		return "Bang & Olufsen A/S";
	case 260:
		return "PLUS Location Systems Pty Ltd";
	case 261:
		return "Ubiquitous Computing Technology Corporation";
	case 262:
		return "Innovative Yachtter Solutions";
	case 263:
		return "William Demant Holding A/S";
	case 264:
		return "Chicony Electronics Co., Ltd.";
	case 265:
		return "Atus BV";
	case 266:
		return "Codegate Ltd";
	case 267:
		return "ERi, Inc";
	case 268:
		return "Transducers Direct, LLC";
	case 269:
		return "DENSO TEN LIMITED (formerly Fujitsu Ten LImited)";
	case 270:
		return "Audi AG";
	case 271:
		return "HiSilicon Technologies CO., LIMITED";
	case 272:
		return "Nippon Seiki Co., Ltd.";
	case 273:
		return "Steelseries ApS";
	case 274:
		return "Visybl Inc.";
	case 275:
		return "Openbrain Technologies, Co., Ltd.";
	case 276:
		return "Xensr";
	case 277:
		return "e.solutions";
	case 278:
		return "10AK Technologies";
	case 279:
		return "Wimoto Technologies Inc";
	case 280:
		return "Radius Networks, Inc.";
	case 281:
		return "Wize Technology Co., Ltd.";
	case 282:
		return "Qualcomm Labs, Inc.";
	case 283:
		return "Aruba Networks";
	case 284:
		return "Baidu";
	case 285:
		return "Arendi AG";
	case 286:
		return "Skoda Auto a.s.";
	case 287:
		return "Volkswagen AG";
	case 288:
		return "Porsche AG";
	case 289:
		return "Sino Wealth Electronic Ltd.";
	case 290:
		return "AirTurn, Inc.";
	case 291:
		return "Kinsa, Inc";
	case 292:
		return "HID Global";
	case 293:
		return "SEAT es";
	case 294:
		return "Promethean Ltd.";
	case 295:
		return "Salutica Allied Solutions";
	case 296:
		return "GPSI Group Pty Ltd";
	case 297:
		return "Nimble Devices Oy";
	case 298:
		return "Changzhou Yongse Infotech  Co., Ltd.";
	case 299:
		return "SportIQ";
	case 300:
		return "TEMEC Instruments B.V.";
	case 301:
		return "Sony Corporation";
	case 302:
		return "ASSA ABLOY";
	case 303:
		return "Clarion Co. Inc.";
	case 304:
		return "Warehouse Innovations";
	case 305:
		return "Cypress Semiconductor";
	case 306:
		return "MADS Inc";
	case 307:
		return "Blue Maestro Limited";
	case 308:
		return "Resolution Products, Ltd.";
	case 309:
		return "Aireware LLC";
	case 310:
		return "Silvair, Inc.";
	case 311:
		return "Prestigio Plaza Ltd.";
	case 312:
		return "NTEO Inc.";
	case 313:
		return "Focus Systems Corporation";
	case 314:
		return "Tencent Holdings Ltd.";
	case 315:
		return "Allegion";
	case 316:
		return "Murata Manufacturing Co., Ltd.";
	case 317:
		return "WirelessWERX";
	case 318:
		return "Nod, Inc.";
	case 319:
		return "B&B Manufacturing Company";
	case 320:
		return "Alpine Electronics (China) Co., Ltd";
	case 321:
		return "FedEx Services";
	case 322:
		return "Grape Systems Inc.";
	case 323:
		return "Bkon Connect";
	case 324:
		return "Lintech GmbH";
	case 325:
		return "Novatel Wireless";
	case 326:
		return "Ciright";
	case 327:
		return "Mighty Cast, Inc.";
	case 328:
		return "Ambimat Electronics";
	case 329:
		return "Perytons Ltd.";
	case 330:
		return "Tivoli Audio, LLC";
	case 331:
		return "Master Lock";
	case 332:
		return "Mesh-Net Ltd";
	case 333:
		return "HUIZHOU DESAY SV AUTOMOTIVE CO., LTD.";
	case 334:
		return "Tangerine, Inc.";
	case 335:
		return "B&W Group Ltd.";
	case 336:
		return "Pioneer Corporation";
	case 337:
		return "OnBeep";
	case 338:
		return "Vernier Software & Technology";
	case 339:
		return "ROL Ergo";
	case 340:
		return "Pebble Technology";
	case 341:
		return "NETATMO";
	case 342:
		return "Accumulate AB";
	case 343:
		return "Anhui Huami Information Technology Co., Ltd.";
	case 344:
		return "Inmite s.r.o.";
	case 345:
		return "ChefSteps, Inc.";
	case 346:
		return "micas AG";
	case 347:
		return "Biomedical Research Ltd.";
	case 348:
		return "Pitius Tec S.L.";
	case 349:
		return "Estimote, Inc.";
	case 350:
		return "Unikey Technologies, Inc.";
	case 351:
		return "Timer Cap Co.";
	case 352:
		return "AwoX";
	case 353:
		return "yikes";
	case 354:
		return "MADSGlobalNZ Ltd.";
	case 355:
		return "PCH International";
	case 356:
		return "Qingdao Yeelink Information Technology Co., Ltd.";
	case 357:
		return "Milwaukee Tool (Formally Milwaukee Electric Tools)";
	case 358:
		return "MISHIK Pte Ltd";
	case 359:
		return "Ascensia Diabetes Care US Inc.";
	case 360:
		return "Spicebox LLC";
	case 361:
		return "emberlight";
	case 362:
		return "Cooper-Atkins Corporation";
	case 363:
		return "Qblinks";
	case 364:
		return "MYSPHERA";
	case 365:
		return "LifeScan Inc";
	case 366:
		return "Volantic AB";
	case 367:
		return "Podo Labs, Inc";
	case 368:
		return "Roche Diabetes Care AG";
	case 369:
		return "Amazon.com Services, LLC (formerly Amazon Fulfillment Service)";
	case 370:
		return "Connovate Technology Private Limited";
	case 371:
		return "Kocomojo, LLC";
	case 372:
		return "Everykey Inc.";
	case 373:
		return "Dynamic Controls";
	case 374:
		return "SentriLock";
	case 375:
		return "I-SYST inc.";
	case 376:
		return "CASIO COMPUTER CO., LTD.";
	case 377:
		return "LAPIS Semiconductor Co., Ltd.";
	case 378:
		return "Telemonitor, Inc.";
	case 379:
		return "taskit GmbH";
	case 380:
		return "Daimler AG";
	case 381:
		return "BatAndCat";
	case 382:
		return "BluDotz Ltd";
	case 383:
		return "XTel Wireless ApS";
	case 384:
		return "Gigaset Communications GmbH";
	case 385:
		return "Gecko Health Innovations, Inc.";
	case 386:
		return "HOP Ubiquitous";
	case 387:
		return "Walt Disney";
	case 388:
		return "Nectar";
	case 389:
		return "bel'apps LLC";
	case 390:
		return "CORE Lighting Ltd";
	case 391:
		return "Seraphim Sense Ltd";
	case 392:
		return "Unico RBC";
	case 393:
		return "Physical Enterprises Inc.";
	case 394:
		return "Able Trend Technology Limited";
	case 395:
		return "Konica Minolta, Inc.";
	case 396:
		return "Wilo SE";
	case 397:
		return "Extron Design Services";
	case 398:
		return "Fitbit, Inc.";
	case 399:
		return "Fireflies Systems";
	case 400:
		return "Intelletto Technologies Inc.";
	case 401:
		return "FDK CORPORATION";
	case 402:
		return "Cloudleaf, Inc";
	case 403:
		return "Maveric Automation LLC";
	case 404:
		return "Acoustic Stream Corporation";
	case 405:
		return "Zuli";
	case 406:
		return "Paxton Access Ltd";
	case 407:
		return "WiSilica Inc.";
	case 408:
		return "VENGIT Korlatolt Felelossegu Tarsasag";
	case 409:
		return "SALTO SYSTEMS S.L.";
	case 410:
		return "TRON Forum (formerly T-Engine Forum)";
	case 411:
		return "CUBETECH s.r.o.";
	case 412:
		return "Cokiya Incorporated";
	case 413:
		return "CVS Health";
	case 414:
		return "Ceruus";
	case 415:
		return "Strainstall Ltd";
	case 416:
		return "Channel Enterprises (HK) Ltd.";
	case 417:
		return "FIAMM";
	case 418:
		return "GIGALANE.CO.,LTD";
	case 419:
		return "EROAD";
	case 420:
		return "Mine Safety Appliances";
	case 421:
		return "Icon Health and Fitness";
	case 422:
		return "Wille Engineering (formely as Asandoo GmbH)";
	case 423:
		return "ENERGOUS CORPORATION";
	case 424:
		return "Taobao";
	case 425:
		return "Canon Inc.";
	case 426:
		return "Geophysical Technology Inc.";
	case 427:
		return "Facebook, Inc.";
	case 428:
		return "Trividia Health, Inc.";
	case 429:
		return "FlightSafety International";
	case 430:
		return "Earlens Corporation";
	case 431:
		return "Sunrise Micro Devices, Inc.";
	case 432:
		return "Star Micronics Co., Ltd.";
	case 433:
		return "Netizens Sp. z o.o.";
	case 434:
		return "Nymi Inc.";
	case 435:
		return "Nytec, Inc.";
	case 436:
		return "Trineo Sp. z o.o.";
	case 437:
		return "Nest Labs Inc.";
	case 438:
		return "LM Technologies Ltd";
	case 439:
		return "General Electric Company";
	case 440:
		return "i+D3 S.L.";
	case 441:
		return "HANA Micron";
	case 442:
		return "Stages Cycling LLC";
	case 443:
		return "Cochlear Bone Anchored Solutions AB";
	case 444:
		return "SenionLab AB";
	case 445:
		return "Syszone Co., Ltd";
	case 446:
		return "Pulsate Mobile Ltd.";
	case 447:
		return "Hong Kong HunterSun Electronic Limited";
	case 448:
		return "pironex GmbH";
	case 449:
		return "BRADATECH Corp.";
	case 450:
		return "Transenergooil AG";
	case 451:
		return "Bunch";
	case 452:
		return "DME Microelectronics";
	case 453:
		return "Bitcraze AB";
	case 454:
		return "HASWARE Inc.";
	case 455:
		return "Abiogenix Inc.";
	case 456:
		return "Poly-Control ApS";
	case 457:
		return "Avi-on";
	case 458:
		return "Laerdal Medical AS";
	case 459:
		return "Fetch My Pet";
	case 460:
		return "Sam Labs Ltd.";
	case 461:
		return "Chengdu Synwing Technology Ltd";
	case 462:
		return "HOUWA SYSTEM DESIGN, k.k.";
	case 463:
		return "BSH";
	case 464:
		return "Primus Inter Pares Ltd";
	case 465:
		return "August Home, Inc";
	case 466:
		return "Gill Electronics";
	case 467:
		return "Sky Wave Design";
	case 468:
		return "Newlab S.r.l.";
	case 469:
		return "ELAD srl";
	case 470:
		return "G-wearables inc.";
	case 471:
		return "Squadrone Systems Inc.";
	case 472:
		return "Code Corporation";
	case 473:
		return "Savant Systems LLC";
	case 474:
		return "Logitech International SA";
	case 475:
		return "Innblue Consulting";
	case 476:
		return "iParking Ltd.";
	case 477:
		return "Koninklijke Philips Electronics N.V.";
	case 478:
		return "Minelab Electronics Pty Limited";
	case 479:
		return "Bison Group Ltd.";
	case 480:
		return "Widex A/S";
	case 481:
		return "Jolla Ltd";
	case 482:
		return "Lectronix, Inc.";
	case 483:
		return "Caterpillar Inc";
	case 484:
		return "Freedom Innovations";
	case 485:
		return "Dynamic Devices Ltd";
	case 486:
		return "Technology Solutions (UK) Ltd";
	case 487:
		return "IPS Group Inc.";
	case 488:
		return "STIR";
	case 489:
		return "Sano, Inc.";
	case 490:
		return "Advanced Application Design, Inc.";
	case 491:
		return "AutoMap LLC";
	case 492:
		return "Spreadtrum Communications Shanghai Ltd";
	case 493:
		return "CuteCircuit LTD";
	case 494:
		return "Valeo Service";
	case 495:
		return "Fullpower Technologies, Inc.";
	case 496:
		return "KloudNation";
	case 497:
		return "Zebra Technologies Corporation";
	case 498:
		return "Itron, Inc.";
	case 499:
		return "The University of Tokyo";
	case 500:
		return "UTC Fire and Security";
	case 501:
		return "Cool Webthings Limited";
	case 502:
		return "DJO Global";
	case 503:
		return "Gelliner Limited";
	case 504:
		return "Anyka (Guangzhou) Microelectronics Technology Co, LTD";
	case 505:
		return "Medtronic Inc.";
	case 506:
		return "Gozio Inc.";
	case 507:
		return "Form Lifting, LLC";
	case 508:
		return "Wahoo Fitness, LLC";
	case 509:
		return "Kontakt Micro-Location Sp. z o.o.";
	case 510:
		return "Radio Systems Corporation";
	case 511:
		return "Freescale Semiconductor, Inc.";
	case 512:
		return "Verifone Systems Pte Ltd. Taiwan Branch";
	case 513:
		return "AR Timing";
	case 514:
		return "Rigado LLC";
	case 515:
		return "Kemppi Oy";
	case 516:
		return "Tapcentive Inc.";
	case 517:
		return "Smartbotics Inc.";
	case 518:
		return "Otter Products, LLC";
	case 519:
		return "STEMP Inc.";
	case 520:
		return "LumiGeek LLC";
	case 521:
		return "InvisionHeart Inc.";
	case 522:
		return "Macnica Inc.";
	case 523:
		return "Jaguar Land Rover Limited";
	case 524:
		return "CoroWare Technologies, Inc";
	case 525:
		return "Simplo Technology Co., LTD";
	case 526:
		return "Omron Healthcare Co., LTD";
	case 527:
		return "Comodule GMBH";
	case 528:
		return "ikeGPS";
	case 529:
		return "Telink Semiconductor Co. Ltd";
	case 530:
		return "Interplan Co., Ltd";
	case 531:
		return "Wyler AG";
	case 532:
		return "IK Multimedia Production srl";
	case 533:
		return "Lukoton Experience Oy";
	case 534:
		return "MTI Ltd";
	case 535:
		return "Tech4home, Lda";
	case 536:
		return "Hiotech AB";
	case 537:
		return "DOTT Limited";
	case 538:
		return "Blue Speck Labs, LLC";
	case 539:
		return "Cisco Systems, Inc";
	case 540:
		return "Mobicomm Inc";
	case 541:
		return "Edamic";
	case 542:
		return "Goodnet, Ltd";
	case 543:
		return "Luster Leaf Products  Inc";
	case 544:
		return "Manus Machina BV";
	case 545:
		return "Mobiquity Networks Inc";
	case 546:
		return "Praxis Dynamics";
	case 547:
		return "Philip Morris Products S.A.";
	case 548:
		return "Comarch SA";
	case 549:
		return "Nestlé Nespresso S.A.";
	case 550:
		return "Merlinia A/S";
	case 551:
		return "LifeBEAM Technologies";
	case 552:
		return "Twocanoes Labs, LLC";
	case 553:
		return "Muoverti Limited";
	case 554:
		return "Stamer Musikanlagen GMBH";
	case 555:
		return "Tesla Motors";
	case 556:
		return "Pharynks Corporation";
	case 557:
		return "Lupine";
	case 558:
		return "Siemens AG";
	case 559:
		return "Huami (Shanghai) Culture Communication CO., LTD";
	case 560:
		return "Foster Electric Company, Ltd";
	case 561:
		return "ETA SA";
	case 562:
		return "x-Senso Solutions Kft";
	case 563:
		return "Shenzhen SuLong Communication Ltd";
	case 564:
		return "FengFan (BeiJing) Technology Co, Ltd";
	case 565:
		return "Qrio Inc";
	case 566:
		return "Pitpatpet Ltd";
	case 567:
		return "MSHeli s.r.l.";
	case 568:
		return "Trakm8 Ltd";
	case 569:
		return "JIN CO, Ltd";
	case 570:
		return "Alatech Tehnology";
	case 571:
		return "Beijing CarePulse Electronic Technology Co, Ltd";
	case 572:
		return "Awarepoint";
	case 573:
		return "ViCentra B.V.";
	case 574:
		return "Raven Industries";
	case 575:
		return "WaveWare Technologies Inc.";
	case 576:
		return "Argenox Technologies";
	case 577:
		return "Bragi GmbH";
	case 578:
		return "16Lab Inc";
	case 579:
		return "Masimo Corp";
	case 580:
		return "Iotera Inc";
	case 581:
		return "Endress+Hauser ";
	case 582:
		return "ACKme Networks, Inc.";
	case 583:
		return "FiftyThree Inc.";
	case 584:
		return "Parker Hannifin Corp";
	case 585:
		return "Transcranial Ltd";
	case 586:
		return "Uwatec AG";
	case 587:
		return "Orlan LLC";
	case 588:
		return "Blue Clover Devices";
	case 589:
		return "M-Way Solutions GmbH";
	case 590:
		return "Microtronics Engineering GmbH";
	case 591:
		return "Schneider Schreibgeräte GmbH";
	case 592:
		return "Sapphire Circuits LLC";
	case 593:
		return "Lumo Bodytech Inc.";
	case 594:
		return "UKC Technosolution";
	case 595:
		return "Xicato Inc.";
	case 596:
		return "Playbrush";
	case 597:
		return "Dai Nippon Printing Co., Ltd.";
	case 598:
		return "G24 Power Limited";
	case 599:
		return "AdBabble Local Commerce Inc.";
	case 600:
		return "Devialet SA";
	case 601:
		return "ALTYOR";
	case 602:
		return "University of Applied Sciences Valais/Haute Ecole Valaisanne";
	case 603:
		return "Five Interactive, LLC dba Zendo";
	case 604:
		return "NetEase（Hangzhou）Network co.Ltd.";
	case 605:
		return "Lexmark International Inc.";
	case 606:
		return "Fluke Corporation";
	case 607:
		return "Yardarm Technologies";
	case 608:
		return "SensaRx";
	case 609:
		return "SECVRE GmbH";
	case 610:
		return "Glacial Ridge Technologies";
	case 611:
		return "Identiv, Inc.";
	case 612:
		return "DDS, Inc.";
	case 613:
		return "SMK Corporation";
	case 614:
		return "Schawbel Technologies LLC";
	case 615:
		return "XMI Systems SA";
	case 616:
		return "Cerevo";
	case 617:
		return "Torrox GmbH & Co KG";
	case 618:
		return "Gemalto";
	case 619:
		return "DEKA Research & Development Corp.";
	case 620:
		return "Domster Tadeusz Szydlowski";
	case 621:
		return "Technogym SPA";
	case 622:
		return "FLEURBAEY BVBA";
	case 623:
		return "Aptcode Solutions";
	case 624:
		return "LSI ADL Technology";
	case 625:
		return "Animas Corp";
	case 626:
		return "Alps Electric Co., Ltd.";
	case 627:
		return "OCEASOFT";
	case 628:
		return "Motsai Research";
	case 629:
		return "Geotab";
	case 630:
		return "E.G.O. Elektro-Geraetebau GmbH";
	case 631:
		return "bewhere inc";
	case 632:
		return "Johnson Outdoors Inc";
	case 633:
		return "steute Schaltgerate GmbH & Co. KG";
	case 634:
		return "Ekomini inc.";
	case 635:
		return "DEFA AS";
	case 636:
		return "Aseptika Ltd";
	case 637:
		return "HUAWEI Technologies Co., Ltd.";
	case 638:
		return "HabitAware, LLC";
	case 639:
		return "ruwido austria gmbh";
	case 640:
		return "ITEC corporation";
	case 641:
		return "StoneL";
	case 642:
		return "Sonova AG";
	case 643:
		return "Maven Machines, Inc.";
	case 644:
		return "Synapse Electronics";
	case 645:
		return "Standard Innovation Inc.";
	case 646:
		return "RF Code, Inc.";
	case 647:
		return "Wally Ventures S.L.";
	case 648:
		return "Willowbank Electronics Ltd";
	case 649:
		return "SK Telecom";
	case 650:
		return "Jetro AS";
	case 651:
		return "Code Gears LTD";
	case 652:
		return "NANOLINK APS";
	case 653:
		return "IF, LLC";
	case 654:
		return "RF Digital Corp";
	case 655:
		return "Church & Dwight Co., Inc";
	case 656:
		return "Multibit Oy";
	case 657:
		return "CliniCloud Inc";
	case 658:
		return "SwiftSensors";
	case 659:
		return "Blue Bite";
	case 660:
		return "ELIAS GmbH";
	case 661:
		return "Sivantos GmbH";
	case 662:
		return "Petzl";
	case 663:
		return "storm power ltd";
	case 664:
		return "EISST Ltd";
	case 665:
		return "Inexess Technology Simma KG";
	case 666:
		return "Currant, Inc.";
	case 667:
		return "C2 Development, Inc.";
	case 668:
		return "Blue Sky Scientific, LLC";
	case 669:
		return "ALOTTAZS LABS, LLC";
	case 670:
		return "Kupson spol. s r.o.";
	case 671:
		return "Areus Engineering GmbH";
	case 672:
		return "Impossible Camera GmbH";
	case 673:
		return "InventureTrack Systems";
	case 674:
		return "LockedUp";
	case 675:
		return "Itude";
	case 676:
		return "Pacific Lock Company";
	case 677:
		return "Tendyron Corporation ( 天地融科技股份有限公司 )";
	case 678:
		return "Robert Bosch GmbH";
	case 679:
		return "Illuxtron international B.V.";
	case 680:
		return "miSport Ltd.";
	case 681:
		return "Chargelib";
	case 682:
		return "Doppler Lab";
	case 683:
		return "BBPOS Limited";
	case 684:
		return "RTB Elektronik GmbH & Co. KG";
	case 685:
		return "Rx Networks, Inc.";
	case 686:
		return "WeatherFlow, Inc.";
	case 687:
		return "Technicolor USA Inc.";
	case 688:
		return "Bestechnic(Shanghai),Ltd";
	case 689:
		return "Raden Inc";
	case 690:
		return "JouZen Oy";
	case 691:
		return "CLABER S.P.A.";
	case 692:
		return "Hyginex, Inc.";
	case 693:
		return "HANSHIN ELECTRIC RAILWAY CO.,LTD.";
	case 694:
		return "Schneider Electric";
	case 695:
		return "Oort Technologies LLC";
	case 696:
		return "Chrono Therapeutics";
	case 697:
		return "Rinnai Corporation";
	case 698:
		return "Swissprime Technologies AG";
	case 699:
		return "Koha.,Co.Ltd";
	case 700:
		return "Genevac Ltd";
	case 701:
		return "Chemtronics";
	case 702:
		return "Seguro Technology Sp. z o.o.";
	case 703:
		return "Redbird Flight Simulations";
	case 704:
		return "Dash Robotics";
	case 705:
		return "LINE Corporation";
	case 706:
		return "Guillemot Corporation";
	case 707:
		return "Techtronic Power Tools Technology Limited";
	case 708:
		return "Wilson Sporting Goods";
	case 709:
		return "Lenovo (Singapore) Pte Ltd. ( 联想（新加坡） )";
	case 710:
		return "Ayatan Sensors";
	case 711:
		return "Electronics Tomorrow Limited";
	case 712:
		return "VASCO Data Security International, Inc.";
	case 713:
		return "PayRange Inc.";
	case 714:
		return "ABOV Semiconductor";
	case 715:
		return "AINA-Wireless Inc.";
	case 716:
		return "Eijkelkamp Soil & Water";
	case 717:
		return "BMA ergonomics b.v.";
	case 718:
		return "Teva Branded Pharmaceutical Products R&D, Inc.";
	case 719:
		return "Anima";
	case 720:
		return "3M";
	case 721:
		return "Empatica Srl";
	case 722:
		return "Afero, Inc.";
	case 723:
		return "Powercast Corporation";
	case 724:
		return "Secuyou ApS";
	case 725:
		return "OMRON Corporation";
	case 726:
		return "Send Solutions";
	case 727:
		return "NIPPON SYSTEMWARE CO.,LTD.";
	case 728:
		return "Neosfar";
	case 729:
		return "Fliegl Agrartechnik GmbH";
	case 730:
		return "Gilvader";
	case 731:
		return "Digi International Inc (R)";
	case 732:
		return "DeWalch Technologies, Inc.";
	case 733:
		return "Flint Rehabilitation Devices, LLC";
	case 734:
		return "Samsung SDS Co., Ltd.";
	case 735:
		return "Blur Product Development";
	case 736:
		return "University of Michigan";
	case 737:
		return "Victron Energy BV";
	case 738:
		return "NTT docomo";
	case 739:
		return "Carmanah Technologies Corp.";
	case 740:
		return "Bytestorm Ltd.";
	case 741:
		return "Espressif Incorporated ( 乐鑫信息科技(上海)有限公司 )";
	case 742:
		return "Unwire";
	case 743:
		return "Connected Yard, Inc.";
	case 744:
		return "American Music Environments";
	case 745:
		return "Sensogram Technologies, Inc.";
	case 746:
		return "Fujitsu Limited";
	case 747:
		return "Ardic Technology";
	case 748:
		return "Delta Systems, Inc";
	case 749:
		return "HTC Corporation";
	case 750:
		return "Citizen Holdings Co., Ltd.";
	case 751:
		return "SMART-INNOVATION.inc";
	case 752:
		return "Blackrat Software";
	case 753:
		return "The Idea Cave, LLC";
	case 754:
		return "GoPro, Inc.";
	case 755:
		return "AuthAir, Inc";
	case 756:
		return "Vensi, Inc.";
	case 757:
		return "Indagem Tech LLC";
	case 758:
		return "Intemo Technologies";
	case 759:
		return "DreamVisions co., Ltd.";
	case 760:
		return "Runteq Oy Ltd";
	case 761:
		return "IMAGINATION TECHNOLOGIES LTD";
	case 762:
		return "CoSTAR Technologies";
	case 763:
		return "Clarius Mobile Health Corp.";
	case 764:
		return "Shanghai Frequen Microelectronics Co., Ltd.";
	case 765:
		return "Uwanna, Inc.";
	case 766:
		return "Lierda Science & Technology Group Co., Ltd.";
	case 767:
		return "Silicon Laboratories";
	case 768:
		return "World Moto Inc.";
	case 769:
		return "Giatec Scientific Inc.";
	case 770:
		return "Loop Devices, Inc";
	case 771:
		return "IACA electronique";
	case 772:
		return "Proxy Technologies, Inc.";
	case 773:
		return "Swipp ApS";
	case 774:
		return "Life Laboratory Inc.";
	case 775:
		return "FUJI INDUSTRIAL CO.,LTD.";
	case 776:
		return "Surefire, LLC";
	case 777:
		return "Dolby Labs";
	case 778:
		return "Ellisys";
	case 779:
		return "Magnitude Lighting Converters";
	case 780:
		return "Hilti AG";
	case 781:
		return "Devdata S.r.l.";
	case 782:
		return "Deviceworx";
	case 783:
		return "Shortcut Labs";
	case 784:
		return "SGL Italia S.r.l.";
	case 785:
		return "PEEQ DATA";
	case 786:
		return "Ducere Technologies Pvt Ltd";
	case 787:
		return "DiveNav, Inc.";
	case 788:
		return "RIIG AI Sp. z o.o.";
	case 789:
		return "Thermo Fisher Scientific";
	case 790:
		return "AG Measurematics Pvt. Ltd.";
	case 791:
		return "CHUO Electronics CO., LTD.";
	case 792:
		return "Aspenta International";
	case 793:
		return "Eugster Frismag AG";
	case 794:
		return "Amber wireless GmbH";
	case 795:
		return "HQ Inc";
	case 796:
		return "Lab Sensor Solutions";
	case 797:
		return "Enterlab ApS";
	case 798:
		return "Eyefi, Inc.";
	case 799:
		return "MetaSystem S.p.A";
	case 800:
		return "SONO ELECTRONICS. CO., LTD";
	case 801:
		return "Jewelbots";
	case 802:
		return "Compumedics Limited";
	case 803:
		return "Rotor Bike Components";
	case 804:
		return "Astro, Inc.";
	case 805:
		return "Amotus Solutions";
	case 806:
		return "Healthwear Technologies (Changzhou)Ltd";
	case 807:
		return "Essex Electronics";
	case 808:
		return "Grundfos A/S";
	case 809:
		return "Eargo, Inc.";
	case 810:
		return "Electronic Design Lab";
	case 811:
		return "ESYLUX";
	case 812:
		return "NIPPON SMT.CO.,Ltd";
	case 813:
		return "BM innovations GmbH";
	case 814:
		return "indoormap";
	case 815:
		return "OttoQ Inc";
	case 816:
		return "North Pole Engineering";
	case 817:
		return "3flares Technologies Inc.";
	case 818:
		return "Electrocompaniet A.S.";
	case 819:
		return "Mul-T-Lock";
	case 820:
		return "Corentium AS";
	case 821:
		return "Enlighted Inc";
	case 822:
		return "GISTIC";
	case 823:
		return "AJP2 Holdings, LLC";
	case 824:
		return "COBI GmbH";
	case 825:
		return "Blue Sky Scientific, LLC";
	case 826:
		return "Appception, Inc.";
	case 827:
		return "Courtney Thorne Limited";
	case 828:
		return "Virtuosys";
	case 829:
		return "TPV Technology Limited";
	case 830:
		return "Monitra SA";
	case 831:
		return "Automation Components, Inc.";
	case 832:
		return "Letsense s.r.l.";
	case 833:
		return "Etesian Technologies LLC";
	case 834:
		return "GERTEC BRASIL LTDA.";
	case 835:
		return "Drekker Development Pty. Ltd.";
	case 836:
		return "Whirl Inc";
	case 837:
		return "Locus Positioning";
	case 838:
		return "Acuity Brands Lighting, Inc";
	case 839:
		return "Prevent Biometrics";
	case 840:
		return "Arioneo";
	case 841:
		return "VersaMe";
	case 842:
		return "Vaddio";
	case 843:
		return "Libratone A/S";
	case 844:
		return "HM Electronics, Inc.";
	case 845:
		return "TASER International, Inc.";
	case 846:
		return "Safe Trust Inc.";
	case 847:
		return "Heartland Payment Systems";
	case 848:
		return "Bitstrata Systems Inc.";
	case 849:
		return "Pieps GmbH";
	case 850:
		return "iRiding(Xiamen)Technology Co.,Ltd.";
	case 851:
		return "Alpha Audiotronics, Inc.";
	case 852:
		return "TOPPAN FORMS CO.,LTD.";
	case 853:
		return "Sigma Designs, Inc.";
	case 854:
		return "Spectrum Brands, Inc.";
	case 855:
		return "Polymap Wireless";
	case 856:
		return "MagniWare Ltd.";
	case 857:
		return "Novotec Medical GmbH";
	case 858:
		return "Medicom Innovation Partner a/s";
	case 859:
		return "Matrix Inc.";
	case 860:
		return "Eaton Corporation";
	case 861:
		return "KYS";
	case 862:
		return "Naya Health, Inc.";
	case 863:
		return "Acromag";
	case 864:
		return "Insulet Corporation";
	case 865:
		return "Wellinks Inc.";
	case 866:
		return "ON Semiconductor";
	case 867:
		return "FREELAP SA";
	case 868:
		return "Favero Electronics Srl";
	case 869:
		return "BioMech Sensor LLC";
	case 870:
		return "BOLTT Sports technologies Private limited";
	case 871:
		return "Saphe International";
	case 872:
		return "Metormote AB";
	case 873:
		return "littleBits";
	case 874:
		return "SetPoint Medical";
	case 875:
		return "BRControls Products BV";
	case 876:
		return "Zipcar";
	case 877:
		return "AirBolt Pty Ltd";
	case 878:
		return "KeepTruckin Inc";
	case 879:
		return "Motiv, Inc.";
	case 880:
		return "Wazombi Labs OÜ";
	case 881:
		return "ORBCOMM";
	case 882:
		return "Nixie Labs, Inc.";
	case 883:
		return "AppNearMe Ltd";
	case 884:
		return "Holman Industries";
	case 885:
		return "Expain AS";
	case 886:
		return "Electronic Temperature Instruments Ltd";
	case 887:
		return "Plejd AB";
	case 888:
		return "Propeller Health";
	case 889:
		return "Shenzhen iMCO Electronic Technology Co.,Ltd";
	case 890:
		return "Algoria";
	case 891:
		return "Apption Labs Inc.";
	case 892:
		return "Cronologics Corporation";
	case 893:
		return "MICRODIA Ltd.";
	case 894:
		return "lulabytes S.L.";
	case 895:
		return "Société des Produits Nestlé S.A. (formerly Nestec S.A.)";
	case 896:
		return "LLC \"MEGA-F service\"";
	case 897:
		return "Sharp Corporation";
	case 898:
		return "Precision Outcomes Ltd";
	case 899:
		return "Kronos Incorporated";
	case 900:
		return "OCOSMOS Co., Ltd.";
	case 901:
		return "Embedded Electronic Solutions Ltd. dba e2Solutions";
	case 902:
		return "Aterica Inc.";
	case 903:
		return "BluStor PMC, Inc.";
	case 904:
		return "Kapsch TrafficCom AB";
	case 905:
		return "ActiveBlu Corporation";
	case 906:
		return "Kohler Mira Limited";
	case 907:
		return "Noke";
	case 908:
		return "Appion Inc.";
	case 909:
		return "Resmed Ltd";
	case 910:
		return "Crownstone B.V.";
	case 911:
		return "Xiaomi Inc.";
	case 912:
		return "INFOTECH s.r.o.";
	case 913:
		return "Thingsquare AB";
	case 914:
		return "T&D";
	case 915:
		return "LAVAZZA S.p.A.";
	case 916:
		return "Netclearance Systems, Inc.";
	case 917:
		return "SDATAWAY";
	case 918:
		return "BLOKS GmbH";
	case 919:
		return "LEGO System A/S";
	case 920:
		return "Thetatronics Ltd";
	case 921:
		return "Nikon Corporation";
	case 922:
		return "NeST";
	case 923:
		return "South Silicon Valley Microelectronics";
	case 924:
		return "ALE International";
	case 925:
		return "CareView Communications, Inc.";
	case 926:
		return "SchoolBoard Limited";
	case 927:
		return "Molex Corporation";
	case 928:
		return "IVT Wireless Limited";
	case 929:
		return "Alpine Labs LLC";
	case 930:
		return "Candura Instruments";
	case 931:
		return "SmartMovt Technology Co., Ltd";
	case 932:
		return "Token Zero Ltd";
	case 933:
		return "ACE CAD Enterprise Co., Ltd. (ACECAD)";
	case 934:
		return "Medela, Inc";
	case 935:
		return "AeroScout";
	case 936:
		return "Esrille Inc.";
	case 937:
		return "THINKERLY SRL";
	case 938:
		return "Exon Sp. z o.o.";
	case 939:
		return "Meizu Technology Co., Ltd.";
	case 940:
		return "Smablo LTD";
	case 941:
		return "XiQ";
	case 942:
		return "Allswell Inc.";
	case 943:
		return "Comm-N-Sense Corp DBA Verigo";
	case 944:
		return "VIBRADORM GmbH";
	case 945:
		return "Otodata Wireless Network Inc.";
	case 946:
		return "Propagation Systems Limited";
	case 947:
		return "Midwest Instruments & Controls";
	case 948:
		return "Alpha Nodus, inc.";
	case 949:
		return "petPOMM, Inc";
	case 950:
		return "Mattel";
	case 951:
		return "Airbly Inc.";
	case 952:
		return "A-Safe Limited";
	case 953:
		return "FREDERIQUE CONSTANT SA";
	case 954:
		return "Maxscend Microelectronics Company Limited";
	case 955:
		return "Abbott";
	case 956:
		return "ASB Bank Ltd";
	case 957:
		return "amadas";
	case 958:
		return "Applied Science, Inc.";
	case 959:
		return "iLumi Solutions Inc.";
	case 960:
		return "Arch Systems Inc.";
	case 961:
		return "Ember Technologies, Inc.";
	case 962:
		return "Snapchat Inc";
	case 963:
		return "Casambi Technologies Oy";
	case 964:
		return "Pico Technology Inc.";
	case 965:
		return "St. Jude Medical, Inc.";
	case 966:
		return "Intricon";
	case 967:
		return "Structural Health Systems, Inc.";
	case 968:
		return "Avvel International";
	case 969:
		return "Gallagher Group";
	case 970:
		return "In2things Automation Pvt. Ltd.";
	case 971:
		return "SYSDEV Srl";
	case 972:
		return "Vonkil Technologies Ltd";
	case 973:
		return "Wynd Technologies, Inc.";
	case 974:
		return "CONTRINEX S.A.";
	case 975:
		return "MIRA, Inc.";
	case 976:
		return "Watteam Ltd";
	case 977:
		return "Density Inc.";
	case 978:
		return "IOT Pot India Private Limited";
	case 979:
		return "Sigma Connectivity AB";
	case 980:
		return "PEG PEREGO SPA";
	case 981:
		return "Wyzelink Systems Inc.";
	case 982:
		return "Yota Devices LTD";
	case 983:
		return "FINSECUR";
	case 984:
		return "Zen-Me Labs Ltd";
	case 985:
		return "3IWare Co., Ltd.";
	case 986:
		return "EnOcean GmbH";
	case 987:
		return "Instabeat, Inc";
	case 988:
		return "Nima Labs";
	case 989:
		return "Andreas Stihl AG & Co. KG";
	case 990:
		return "Nathan Rhoades LLC";
	case 991:
		return "Grob Technologies, LLC";
	case 992:
		return "Actions (Zhuhai) Technology Co., Limited";
	case 993:
		return "SPD Development Company Ltd";
	case 994:
		return "Sensoan Oy";
	case 995:
		return "Qualcomm Life Inc";
	case 996:
		return "Chip-ing AG";
	case 997:
		return "ffly4u";
	case 998:
		return "IoT Instruments Oy";
	case 999:
		return "TRUE Fitness Technology";
	case 1000:
		return "Reiner Kartengeraete GmbH & Co. KG.";
	case 1001:
		return "SHENZHEN LEMONJOY TECHNOLOGY CO., LTD.";
	case 1002:
		return "Hello Inc.";
	case 1003:
		return "Evollve Inc.";
	case 1004:
		return "Jigowatts Inc.";
	case 1005:
		return "BASIC MICRO.COM,INC.";
	case 1006:
		return "CUBE TECHNOLOGIES";
	case 1007:
		return "foolography GmbH";
	case 1008:
		return "CLINK";
	case 1009:
		return "Hestan Smart Cooking Inc.";
	case 1010:
		return "WindowMaster A/S";
	case 1011:
		return "Flowscape AB";
	case 1012:
		return "PAL Technologies Ltd";
	case 1013:
		return "WHERE, Inc.";
	case 1014:
		return "Iton Technology Corp.";
	case 1015:
		return "Owl Labs Inc.";
	case 1016:
		return "Rockford Corp.";
	case 1017:
		return "Becon Technologies Co.,Ltd.";
	case 1018:
		return "Vyassoft Technologies Inc";
	case 1019:
		return "Nox Medical";
	case 1020:
		return "Kimberly-Clark";
	case 1021:
		return "Trimble Navigation Ltd.";
	case 1022:
		return "Littelfuse";
	case 1023:
		return "Withings";
	case 1024:
		return "i-developer IT Beratung UG";
	case 1025:
		return "Relations Inc.";
	case 1026:
		return "Sears Holdings Corporation";
	case 1027:
		return "Gantner Electronic GmbH";
	case 1028:
		return "Authomate Inc";
	case 1029:
		return "Vertex International, Inc.";
	case 1030:
		return "Airtago";
	case 1031:
		return "Swiss Audio SA";
	case 1032:
		return "ToGetHome Inc.";
	case 1033:
		return "AXIS";
	case 1034:
		return "Openmatics";
	case 1035:
		return "Jana Care Inc.";
	case 1036:
		return "Senix Corporation";
	case 1037:
		return "NorthStar Battery Company, LLC";
	case 1038:
		return "SKF (U.K.) Limited";
	case 1039:
		return "CO-AX Technology, Inc.";
	case 1040:
		return "Fender Musical Instruments";
	case 1041:
		return "Luidia Inc";
	case 1042:
		return "SEFAM";
	case 1043:
		return "Wireless Cables Inc";
	case 1044:
		return "Lightning Protection International Pty Ltd";
	case 1045:
		return "Uber Technologies Inc";
	case 1046:
		return "SODA GmbH";
	case 1047:
		return "Fatigue Science";
	case 1048:
		return "Alpine Electronics Inc.";
	case 1049:
		return "Novalogy LTD";
	case 1050:
		return "Friday Labs Limited";
	case 1051:
		return "OrthoAccel Technologies";
	case 1052:
		return "WaterGuru, Inc.";
	case 1053:
		return "Benning Elektrotechnik und Elektronik GmbH & Co. KG";
	case 1054:
		return "Dell Computer Corporation";
	case 1055:
		return "Kopin Corporation";
	case 1056:
		return "TecBakery GmbH";
	case 1057:
		return "Backbone Labs, Inc.";
	case 1058:
		return "DELSEY SA";
	case 1059:
		return "Chargifi Limited";
	case 1060:
		return "Trainesense Ltd.";
	case 1061:
		return "Unify Software and Solutions GmbH & Co. KG";
	case 1062:
		return "Husqvarna AB";
	case 1063:
		return "Focus fleet and fuel management inc";
	case 1064:
		return "SmallLoop, LLC";
	case 1065:
		return "Prolon Inc.";
	case 1066:
		return "BD Medical";
	case 1067:
		return "iMicroMed Incorporated";
	case 1068:
		return "Ticto N.V.";
	case 1069:
		return "Meshtech AS";
	case 1070:
		return "MemCachier Inc.";
	case 1071:
		return "Danfoss A/S";
	case 1072:
		return "SnapStyk Inc.";
	case 1073:
		return "Amway Corporation";
	case 1074:
		return "Silk Labs, Inc.";
	case 1075:
		return "Pillsy Inc.";
	case 1076:
		return "Hatch Baby, Inc.";
	case 1077:
		return "Blocks Wearables Ltd.";
	case 1078:
		return "Drayson Technologies (Europe) Limited";
	case 1079:
		return "eBest IOT Inc.";
	case 1080:
		return "Helvar Ltd";
	case 1081:
		return "Radiance Technologies";
	case 1082:
		return "Nuheara Limited";
	case 1083:
		return "Appside co., ltd.";
	case 1084:
		return "DeLaval";
	case 1085:
		return "Coiler Corporation";
	case 1086:
		return "Thermomedics, Inc.";
	case 1087:
		return "Tentacle Sync GmbH";
	case 1088:
		return "Valencell, Inc.";
	case 1089:
		return "iProtoXi Oy";
	case 1090:
		return "SECOM CO., LTD.";
	case 1091:
		return "Tucker International LLC";
	case 1092:
		return "Metanate Limited";
	case 1093:
		return "Kobian Canada Inc.";
	case 1094:
		return "NETGEAR, Inc.";
	case 1095:
		return "Fabtronics Australia Pty Ltd";
	case 1096:
		return "Grand Centrix GmbH";
	case 1097:
		return "1UP USA.com llc";
	case 1098:
		return "SHIMANO INC.";
	case 1099:
		return "Nain Inc.";
	case 1100:
		return "LifeStyle Lock, LLC";
	case 1101:
		return "VEGA Grieshaber KG";
	case 1102:
		return "Xtrava Inc.";
	case 1103:
		return "TTS Tooltechnic Systems AG & Co. KG";
	case 1104:
		return "Teenage Engineering AB";
	case 1105:
		return "Tunstall Nordic AB";
	case 1106:
		return "Svep Design Center AB";
	case 1107:
		return "GreenPeak Technologies BV";
	case 1108:
		return "Sphinx Electronics GmbH & Co KG";
	case 1109:
		return "Atomation";
	case 1110:
		return "Nemik Consulting Inc";
	case 1111:
		return "RF INNOVATION";
	case 1112:
		return "Mini Solution Co., Ltd.";
	case 1113:
		return "Lumenetix, Inc";
	case 1114:
		return "2048450 Ontario Inc";
	case 1115:
		return "SPACEEK LTD";
	case 1116:
		return "Delta T Corporation";
	case 1117:
		return "Boston Scientific Corporation";
	case 1118:
		return "Nuviz, Inc.";
	case 1119:
		return "Real Time Automation, Inc.";
	case 1120:
		return "Kolibree";
	case 1121:
		return "vhf elektronik GmbH";
	case 1122:
		return "Bonsai Systems GmbH";
	case 1123:
		return "Fathom Systems Inc.";
	case 1124:
		return "Bellman & Symfon";
	case 1125:
		return "International Forte Group LLC";
	case 1126:
		return "CycleLabs Solutions inc.";
	case 1127:
		return "Codenex Oy";
	case 1128:
		return "Kynesim Ltd";
	case 1129:
		return "Palago AB";
	case 1130:
		return "INSIGMA INC.";
	case 1131:
		return "PMD Solutions";
	case 1132:
		return "Qingdao Realtime Technology Co., Ltd.";
	case 1133:
		return "BEGA Gantenbrink-Leuchten KG";
	case 1134:
		return "Pambor Ltd.";
	case 1135:
		return "Develco Products A/S";
	case 1136:
		return "iDesign s.r.l.";
	case 1137:
		return "TiVo Corp";
	case 1138:
		return "Control-J Pty Ltd";
	case 1139:
		return "Steelcase, Inc.";
	case 1140:
		return "iApartment co., ltd.";
	case 1141:
		return "Icom inc.";
	case 1142:
		return "Oxstren Wearable Technologies Private Limited";
	case 1143:
		return "Blue Spark Technologies";
	case 1144:
		return "FarSite Communications Limited";
	case 1145:
		return "mywerk system GmbH";
	case 1146:
		return "Sinosun Technology Co., Ltd.";
	case 1147:
		return "MIYOSHI ELECTRONICS CORPORATION";
	case 1148:
		return "POWERMAT LTD";
	case 1149:
		return "Occly LLC";
	case 1150:
		return "OurHub Dev IvS";
	case 1151:
		return "Pro-Mark, Inc.";
	case 1152:
		return "Dynometrics Inc.";
	case 1153:
		return "Quintrax Limited";
	case 1154:
		return "POS Tuning Udo Vosshenrich GmbH & Co. KG";
	case 1155:
		return "Multi Care Systems B.V.";
	case 1156:
		return "Revol Technologies Inc";
	case 1157:
		return "SKIDATA AG";
	case 1158:
		return "DEV TECNOLOGIA INDUSTRIA, COMERCIO E MANUTENCAO DE EQUIPAMENTOS LTDA. - ME";
	case 1159:
		return "Centrica Connected Home";
	case 1160:
		return "Automotive Data Solutions Inc";
	case 1161:
		return "Igarashi Engineering";
	case 1162:
		return "Taelek Oy";
	case 1163:
		return "CP Electronics Limited";
	case 1164:
		return "Vectronix AG";
	case 1165:
		return "S-Labs Sp. z o.o.";
	case 1166:
		return "Companion Medical, Inc.";
	case 1167:
		return "BlueKitchen GmbH";
	case 1168:
		return "Matting AB";
	case 1169:
		return "SOREX - Wireless Solutions GmbH";
	case 1170:
		return "ADC Technology, Inc.";
	case 1171:
		return "Lynxemi Pte Ltd";
	case 1172:
		return "SENNHEISER electronic GmbH & Co. KG";
	case 1173:
		return "LMT Mercer Group, Inc";
	case 1174:
		return "Polymorphic Labs LLC";
	case 1175:
		return "Cochlear Limited";
	case 1176:
		return "METER Group, Inc. USA";
	case 1177:
		return "Ruuvi Innovations Ltd.";
	case 1178:
		return "Situne AS";
	case 1179:
		return "nVisti, LLC";
	case 1180:
		return "DyOcean";
	case 1181:
		return "Uhlmann & Zacher GmbH";
	case 1182:
		return "AND!XOR LLC";
	case 1183:
		return "tictote AB";
	case 1184:
		return "Vypin, LLC";
	case 1185:
		return "PNI Sensor Corporation";
	case 1186:
		return "ovrEngineered, LLC";
	case 1187:
		return "GT-tronics HK Ltd";
	case 1188:
		return "Herbert Waldmann GmbH & Co. KG";
	case 1189:
		return "Guangzhou FiiO Electronics Technology Co.,Ltd";
	case 1190:
		return "Vinetech Co., Ltd";
	case 1191:
		return "Dallas Logic Corporation";
	case 1192:
		return "BioTex, Inc.";
	case 1193:
		return "DISCOVERY SOUND TECHNOLOGY, LLC";
	case 1194:
		return "LINKIO SAS";
	case 1195:
		return "Harbortronics, Inc.";
	case 1196:
		return "Undagrid B.V.";
	case 1197:
		return "Shure Inc";
	case 1198:
		return "ERM Electronic Systems LTD";
	case 1199:
		return "BIOROWER Handelsagentur GmbH";
	case 1200:
		return "Weba Sport und Med. Artikel GmbH";
	case 1201:
		return "Kartographers Technologies Pvt. Ltd.";
	case 1202:
		return "The Shadow on the Moon";
	case 1203:
		return "mobike (Hong Kong) Limited";
	case 1204:
		return "Inuheat Group AB";
	case 1205:
		return "Swiftronix AB";
	case 1206:
		return "Diagnoptics Technologies";
	case 1207:
		return "Analog Devices, Inc.";
	case 1208:
		return "Soraa Inc.";
	case 1209:
		return "CSR Building Products Limited";
	case 1210:
		return "Crestron Electronics, Inc.";
	case 1211:
		return "Neatebox Ltd";
	case 1212:
		return "Draegerwerk AG & Co. KGaA";
	case 1213:
		return "AlbynMedical";
	case 1214:
		return "Averos FZCO";
	case 1215:
		return "VIT Initiative, LLC";
	case 1216:
		return "Statsports International";
	case 1217:
		return "Sospitas, s.r.o.";
	case 1218:
		return "Dmet Products Corp.";
	case 1219:
		return "Mantracourt Electronics Limited";
	case 1220:
		return "TeAM Hutchins AB";
	case 1221:
		return "Seibert Williams Glass, LLC";
	case 1222:
		return "Insta GmbH";
	case 1223:
		return "Svantek Sp. z o.o.";
	case 1224:
		return "Shanghai Flyco Electrical Appliance Co., Ltd.";
	case 1225:
		return "Thornwave Labs Inc";
	case 1226:
		return "Steiner-Optik GmbH";
	case 1227:
		return "Novo Nordisk A/S";
	case 1228:
		return "Enflux Inc.";
	case 1229:
		return "Safetech Products LLC";
	case 1230:
		return "GOOOLED S.R.L.";
	case 1231:
		return "DOM Sicherheitstechnik GmbH & Co. KG";
	case 1232:
		return "Olympus Corporation";
	case 1233:
		return "KTS GmbH";
	case 1234:
		return "Anloq Technologies Inc.";
	case 1235:
		return "Queercon, Inc";
	case 1236:
		return "5th Element Ltd";
	case 1237:
		return "Gooee Limited";
	case 1238:
		return "LUGLOC LLC";
	case 1239:
		return "Blincam, Inc.";
	case 1240:
		return "FUJIFILM Corporation";
	case 1241:
		return "RandMcNally";
	case 1242:
		return "Franceschi Marina snc";
	case 1243:
		return "Engineered Audio, LLC.";
	case 1244:
		return "IOTTIVE (OPC) PRIVATE LIMITED";
	case 1245:
		return "4MOD Technology";
	case 1246:
		return "Lutron Electronics Co., Inc.";
	case 1247:
		return "Emerson";
	case 1248:
		return "Guardtec, Inc.";
	case 1249:
		return "REACTEC LIMITED";
	case 1250:
		return "EllieGrid";
	case 1251:
		return "Under Armour";
	case 1252:
		return "Woodenshark";
	case 1253:
		return "Avack Oy";
	case 1254:
		return "Smart Solution Technology, Inc.";
	case 1255:
		return "REHABTRONICS INC.";
	case 1256:
		return "STABILO International";
	case 1257:
		return "Busch Jaeger Elektro GmbH";
	case 1258:
		return "Pacific Bioscience Laboratories, Inc";
	case 1259:
		return "Bird Home Automation GmbH";
	case 1260:
		return "Motorola Solutions";
	case 1261:
		return "R9 Technology, Inc.";
	case 1262:
		return "Auxivia";
	case 1263:
		return "DaisyWorks, Inc";
	case 1264:
		return "Kosi Limited";
	case 1265:
		return "Theben AG";
	case 1266:
		return "InDreamer Techsol Private Limited";
	case 1267:
		return "Cerevast Medical";
	case 1268:
		return "ZanCompute Inc.";
	case 1269:
		return "Pirelli Tyre S.P.A.";
	case 1270:
		return "McLear Limited";
	case 1271:
		return "Shenzhen Huiding Technology Co.,Ltd.";
	case 1272:
		return "Convergence Systems Limited";
	case 1273:
		return "Interactio";
	case 1274:
		return "Androtec GmbH";
	case 1275:
		return "Benchmark Drives GmbH & Co. KG";
	case 1276:
		return "SwingLync L. L. C.";
	case 1277:
		return "Tapkey GmbH";
	case 1278:
		return "Woosim Systems Inc.";
	case 1279:
		return "Microsemi Corporation";
	case 1280:
		return "Wiliot LTD.";
	case 1281:
		return "Polaris IND";
	case 1282:
		return "Specifi-Kali LLC";
	case 1283:
		return "Locoroll, Inc";
	case 1284:
		return "PHYPLUS Inc";
	case 1285:
		return "Inplay Technologies LLC";
	case 1286:
		return "Hager";
	case 1287:
		return "Yellowcog";
	case 1288:
		return "Axes System sp. z o. o.";
	case 1289:
		return "myLIFTER Inc.";
	case 1290:
		return "Shake-on B.V.";
	case 1291:
		return "Vibrissa Inc.";
	case 1292:
		return "OSRAM GmbH";
	case 1293:
		return "TRSystems GmbH";
	case 1294:
		return "Yichip Microelectronics (Hangzhou) Co.,Ltd.";
	case 1295:
		return "Foundation Engineering LLC";
	case 1296:
		return "UNI-ELECTRONICS, INC.";
	case 1297:
		return "Brookfield Equinox LLC";
	case 1298:
		return "Soprod SA";
	case 1299:
		return "9974091 Canada Inc.";
	case 1300:
		return "FIBRO GmbH";
	case 1301:
		return "RB Controls Co., Ltd.";
	case 1302:
		return "Footmarks";
	case 1303:
		return "Amtronic Sverige AB (formerly Amcore AB)";
	case 1304:
		return "MAMORIO.inc";
	case 1305:
		return "Tyto Life LLC";
	case 1306:
		return "Leica Camera AG";
	case 1307:
		return "Angee Technologies Ltd.";
	case 1308:
		return "EDPS";
	case 1309:
		return "OFF Line Co., Ltd.";
	case 1310:
		return "Detect Blue Limited";
	case 1311:
		return "Setec Pty Ltd";
	case 1312:
		return "Target Corporation";
	case 1313:
		return "IAI Corporation";
	case 1314:
		return "NS Tech, Inc.";
	case 1315:
		return "MTG Co., Ltd.";
	case 1316:
		return "Hangzhou iMagic Technology Co., Ltd";
	case 1317:
		return "HONGKONG NANO IC TECHNOLOGIES  CO., LIMITED";
	case 1318:
		return "Honeywell International Inc.";
	case 1319:
		return "Albrecht JUNG";
	case 1320:
		return "Lunera Lighting Inc.";
	case 1321:
		return "Lumen UAB";
	case 1322:
		return "Keynes Controls Ltd";
	case 1323:
		return "Novartis AG";
	case 1324:
		return "Geosatis SA";
	case 1325:
		return "EXFO, Inc.";
	case 1326:
		return "LEDVANCE GmbH";
	case 1327:
		return "Center ID Corp.";
	case 1328:
		return "Adolene, Inc.";
	case 1329:
		return "D&M Holdings Inc.";
	case 1330:
		return "CRESCO Wireless, Inc.";
	case 1331:
		return "Nura Operations Pty Ltd";
	case 1332:
		return "Frontiergadget, Inc.";
	case 1333:
		return "Smart Component Technologies Limited";
	case 1334:
		return "ZTR Control Systems LLC";
	case 1335:
		return "MetaLogics Corporation";
	case 1336:
		return "Medela AG";
	case 1337:
		return "OPPLE Lighting Co., Ltd";
	case 1338:
		return "Savitech Corp.,";
	case 1339:
		return "prodigy";
	case 1340:
		return "Screenovate Technologies Ltd";
	case 1341:
		return "TESA SA";
	case 1342:
		return "CLIM8 LIMITED";
	case 1343:
		return "Silergy Corp";
	case 1344:
		return "SilverPlus, Inc";
	case 1345:
		return "Sharknet srl";
	case 1346:
		return "Mist Systems, Inc.";
	case 1347:
		return "MIWA LOCK CO.,Ltd";
	case 1348:
		return "OrthoSensor, Inc.";
	case 1349:
		return "Candy Hoover Group s.r.l";
	case 1350:
		return "Apexar Technologies S.A.";
	case 1351:
		return "LOGICDATA d.o.o.";
	case 1352:
		return "Knick Elektronische Messgeraete GmbH & Co. KG";
	case 1353:
		return "Smart Technologies and Investment Limited";
	case 1354:
		return "Linough Inc.";
	case 1355:
		return "Advanced Electronic Designs, Inc.";
	case 1356:
		return "Carefree Scott Fetzer Co Inc";
	case 1357:
		return "Sensome";
	case 1358:
		return "FORTRONIK storitve d.o.o.";
	case 1359:
		return "Sinnoz";
	case 1360:
		return "Versa Networks, Inc.";
	case 1361:
		return "Sylero";
	case 1362:
		return "Avempace SARL";
	case 1363:
		return "Nintendo Co., Ltd.";
	case 1364:
		return "National Instruments";
	case 1365:
		return "KROHNE Messtechnik GmbH";
	case 1366:
		return "Otodynamics Ltd";
	case 1367:
		return "Arwin Technology Limited";
	case 1368:
		return "benegear, inc.";
	case 1369:
		return "Newcon Optik";
	case 1370:
		return "CANDY HOUSE, Inc.";
	case 1371:
		return "FRANKLIN TECHNOLOGY INC";
	case 1372:
		return "Lely";
	case 1373:
		return "Valve Corporation";
	case 1374:
		return "Hekatron Vertriebs GmbH";
	case 1375:
		return "PROTECH S.A.S. DI GIRARDI ANDREA & C.";
	case 1376:
		return "Sarita CareTech APS (formerly Sarita CareTech IVS)";
	case 1377:
		return "Finder S.p.A.";
	case 1378:
		return "Thalmic Labs Inc.";
	case 1379:
		return "Steinel Vertrieb GmbH";
	case 1380:
		return "Beghelli Spa";
	case 1381:
		return "Beijing Smartspace Technologies Inc.";
	case 1382:
		return "CORE TRANSPORT TECHNOLOGIES NZ LIMITED";
	case 1383:
		return "Xiamen Everesports Goods Co., Ltd";
	case 1384:
		return "Bodyport Inc.";
	case 1385:
		return "Audionics System, INC.";
	case 1386:
		return "Flipnavi Co.,Ltd.";
	case 1387:
		return "Rion Co., Ltd.";
	case 1388:
		return "Long Range Systems, LLC";
	case 1389:
		return "Redmond Industrial Group LLC";
	case 1390:
		return "VIZPIN INC.";
	case 1391:
		return "BikeFinder AS";
	case 1392:
		return "Consumer Sleep Solutions LLC";
	case 1393:
		return "PSIKICK, INC.";
	case 1394:
		return "AntTail.com";
	case 1395:
		return "Lighting Science Group Corp.";
	case 1396:
		return "AFFORDABLE ELECTRONICS INC";
	case 1397:
		return "Integral Memroy Plc";
	case 1398:
		return "Globalstar, Inc.";
	case 1399:
		return "True Wearables, Inc.";
	case 1400:
		return "Wellington Drive Technologies Ltd";
	case 1401:
		return "Ensemble Tech Private Limited";
	case 1402:
		return "OMNI Remotes";
	case 1403:
		return "Duracell U.S. Operations Inc.";
	case 1404:
		return "Toor Technologies LLC";
	case 1405:
		return "Instinct Performance";
	case 1406:
		return "Beco, Inc";
	case 1407:
		return "Scuf Gaming International, LLC";
	case 1408:
		return "ARANZ Medical Limited";
	case 1409:
		return "LYS TECHNOLOGIES LTD";
	case 1410:
		return "Breakwall Analytics, LLC";
	case 1411:
		return "Code Blue Communications";
	case 1412:
		return "Gira Giersiepen GmbH & Co. KG";
	case 1413:
		return "Hearing Lab Technology";
	case 1414:
		return "LEGRAND";
	case 1415:
		return "Derichs GmbH";
	case 1416:
		return "ALT-TEKNIK LLC";
	case 1417:
		return "Star Technologies";
	case 1418:
		return "START TODAY CO.,LTD.";
	case 1419:
		return "Maxim Integrated Products";
	case 1420:
		return "MERCK Kommanditgesellschaft auf Aktien";
	case 1421:
		return "Jungheinrich Aktiengesellschaft";
	case 1422:
		return "Oculus VR, LLC";
	case 1423:
		return "HENDON SEMICONDUCTORS PTY LTD";
	case 1424:
		return "Pur3 Ltd";
	case 1425:
		return "Viasat Group S.p.A.";
	case 1426:
		return "IZITHERM";
	case 1427:
		return "Spaulding Clinical Research";
	case 1428:
		return "Kohler Company";
	case 1429:
		return "Inor Process AB";
	case 1430:
		return "My Smart Blinds";
	case 1431:
		return "RadioPulse Inc";
	case 1432:
		return "rapitag GmbH";
	case 1433:
		return "Lazlo326, LLC.";
	case 1434:
		return "Teledyne Lecroy, Inc.";
	case 1435:
		return "Dataflow Systems Limited";
	case 1436:
		return "Macrogiga Electronics";
	case 1437:
		return "Tandem Diabetes Care";
	case 1438:
		return "Polycom, Inc.";
	case 1439:
		return "Fisher & Paykel Healthcare";
	case 1440:
		return "RCP Software Oy";
	case 1441:
		return "Shanghai Xiaoyi Technology Co.,Ltd.";
	case 1442:
		return "ADHERIUM(NZ) LIMITED";
	case 1443:
		return "Axiomware Systems Incorporated";
	case 1444:
		return "O. E. M. Controls, Inc.";
	case 1445:
		return "Kiiroo BV";
	case 1446:
		return "Telecon Mobile Limited";
	case 1447:
		return "Sonos Inc";
	case 1448:
		return "Tom Allebrandi Consulting";
	case 1449:
		return "Monidor";
	case 1450:
		return "Tramex Limited";
	case 1451:
		return "Nofence AS";
	case 1452:
		return "GoerTek Dynaudio Co., Ltd.";
	case 1453:
		return "INIA";
	case 1454:
		return "CARMATE MFG.CO.,LTD";
	case 1455:
		return "ONvocal";
	case 1456:
		return "NewTec GmbH";
	case 1457:
		return "Medallion Instrumentation Systems";
	case 1458:
		return "CAREL INDUSTRIES S.P.A.";
	case 1459:
		return "Parabit Systems, Inc.";
	case 1460:
		return "White Horse Scientific ltd";
	case 1461:
		return "verisilicon";
	case 1462:
		return "Elecs Industry Co.,Ltd.";
	case 1463:
		return "Beijing Pinecone Electronics Co.,Ltd.";
	case 1464:
		return "Ambystoma Labs Inc.";
	case 1465:
		return "Suzhou Pairlink Network Technology";
	case 1466:
		return "igloohome";
	case 1467:
		return "Oxford Metrics plc";
	case 1468:
		return "Leviton Mfg. Co., Inc.";
	case 1469:
		return "ULC Robotics Inc.";
	case 1470:
		return "RFID Global by Softwork SrL";
	case 1471:
		return "Real-World-Systems Corporation";
	case 1472:
		return "Nalu Medical, Inc.";
	case 1473:
		return "P.I.Engineering";
	case 1474:
		return "Grote Industries";
	case 1475:
		return "Runtime, Inc.";
	case 1476:
		return "Codecoup sp. z o.o. sp. k.";
	case 1477:
		return "SELVE GmbH & Co. KG";
	case 1478:
		return "Smart Animal Training Systems, LLC";
	case 1479:
		return "Lippert Components, INC";
	case 1480:
		return "SOMFY SAS";
	case 1481:
		return "TBS Electronics B.V.";
	case 1482:
		return "MHL Custom Inc";
	case 1483:
		return "LucentWear LLC";
	case 1484:
		return "WATTS ELECTRONICS";
	case 1485:
		return "RJ Brands LLC";
	case 1486:
		return "V-ZUG Ltd";
	case 1487:
		return "Biowatch SA";
	case 1488:
		return "Anova Applied Electronics";
	case 1489:
		return "Lindab AB";
	case 1490:
		return "frogblue TECHNOLOGY GmbH";
	case 1491:
		return "Acurable Limited";
	case 1492:
		return "LAMPLIGHT Co., Ltd.";
	case 1493:
		return "TEGAM, Inc.";
	case 1494:
		return "Zhuhai Jieli technology Co.,Ltd";
	case 1495:
		return "modum.io AG";
	case 1496:
		return "Farm Jenny LLC";
	case 1497:
		return "Toyo Electronics Corporation";
	case 1498:
		return "Applied Neural Research Corp";
	case 1499:
		return "Avid Identification Systems, Inc.";
	case 1500:
		return "Petronics Inc.";
	case 1501:
		return "essentim GmbH";
	case 1502:
		return "QT Medical INC.";
	case 1503:
		return "VIRTUALCLINIC.DIRECT LIMITED";
	case 1504:
		return "Viper Design LLC";
	case 1505:
		return "Human, Incorporated";
	case 1506:
		return "stAPPtronics GmbH";
	case 1507:
		return "Elemental Machines, Inc.";
	case 1508:
		return "Taiyo Yuden Co., Ltd";
	case 1509:
		return "INEO ENERGY& SYSTEMS";
	case 1510:
		return "Motion Instruments Inc.";
	case 1511:
		return "PressurePro";
	case 1512:
		return "COWBOY";
	case 1513:
		return "iconmobile GmbH";
	case 1514:
		return "ACS-Control-System GmbH";
	case 1515:
		return "Bayerische Motoren Werke AG";
	case 1516:
		return "Gycom Svenska AB";
	case 1517:
		return "Fuji Xerox Co., Ltd";
	case 1518:
		return "Glide Inc.";
	case 1519:
		return "SIKOM AS";
	case 1520:
		return "beken";
	case 1521:
		return "The Linux Foundation";
	case 1522:
		return "Try and E CO.,LTD.";
	case 1523:
		return "SeeScan";
	case 1524:
		return "Clearity, LLC";
	case 1525:
		return "GS TAG";
	case 1526:
		return "DPTechnics";
	case 1527:
		return "TRACMO, INC.";
	case 1528:
		return "Anki Inc.";
	case 1529:
		return "Hagleitner Hygiene International GmbH";
	case 1530:
		return "Konami Sports Life Co., Ltd.";
	case 1531:
		return "Arblet Inc.";
	case 1532:
		return "Masbando GmbH";
	case 1533:
		return "Innoseis";
	case 1534:
		return "Niko nv";
	case 1535:
		return "Wellnomics Ltd";
	case 1536:
		return "iRobot Corporation";
	case 1537:
		return "Schrader Electronics";
	case 1538:
		return "Geberit International AG";
	case 1539:
		return "Fourth Evolution Inc";
	case 1540:
		return "Cell2Jack LLC";
	case 1541:
		return "FMW electronic Futterer u. Maier-Wolf OHG";
	case 1542:
		return "John Deere";
	case 1543:
		return "Rookery Technology Ltd";
	case 1544:
		return "KeySafe-Cloud";
	case 1545:
		return "BUCHI Labortechnik AG";
	case 1546:
		return "IQAir AG";
	case 1547:
		return "Triax Technologies Inc";
	case 1548:
		return "Vuzix Corporation";
	case 1549:
		return "TDK Corporation";
	case 1550:
		return "Blueair AB";
	case 1551:
		return "Signify Netherlands";
	case 1552:
		return "ADH GUARDIAN USA LLC";
	case 1553:
		return "Beurer GmbH";
	case 1554:
		return "Playfinity AS";
	case 1555:
		return "Hans Dinslage GmbH";
	case 1556:
		return "OnAsset Intelligence, Inc.";
	case 1557:
		return "INTER ACTION Corporation";
	case 1558:
		return "OS42 UG (haftungsbeschraenkt)";
	case 1559:
		return "WIZCONNECTED COMPANY LIMITED";
	case 1560:
		return "Audio-Technica Corporation";
	case 1561:
		return "Six Guys Labs, s.r.o.";
	case 1562:
		return "R.W. Beckett Corporation";
	case 1563:
		return "silex technology, inc.";
	case 1564:
		return "Univations Limited";
	case 1565:
		return "SENS Innovation ApS";
	case 1566:
		return "Diamond Kinetics, Inc.";
	case 1567:
		return "Phrame Inc.";
	case 1568:
		return "Forciot Oy";
	case 1569:
		return "Noordung d.o.o.";
	case 1570:
		return "Beam Labs, LLC";
	case 1571:
		return "Philadelphia Scientific (U.K.) Limited";
	case 1572:
		return "Biovotion AG";
	case 1573:
		return "Square Panda, Inc.";
	case 1574:
		return "Amplifico";
	case 1575:
		return "WEG S.A.";
	case 1576:
		return "Ensto Oy";
	case 1577:
		return "PHONEPE PVT LTD";
	case 1578:
		return "Lunatico Astronomia SL";
	case 1579:
		return "MinebeaMitsumi Inc.";
	case 1580:
		return "ASPion GmbH";
	case 1581:
		return "Vossloh-Schwabe Deutschland GmbH";
	case 1582:
		return "Procept";
	case 1583:
		return "ONKYO Corporation";
	case 1584:
		return "Asthrea D.O.O.";
	case 1585:
		return "Fortiori Design LLC";
	case 1586:
		return "Hugo Muller GmbH & Co KG";
	case 1587:
		return "Wangi Lai PLT";
	case 1588:
		return "Fanstel Corp";
	case 1589:
		return "Crookwood";
	case 1590:
		return "ELECTRONICA INTEGRAL DE SONIDO S.A.";
	case 1591:
		return "GiP Innovation Tools GmbH";
	case 1592:
		return "LX SOLUTIONS PTY LIMITED";
	case 1593:
		return "Shenzhen Minew Technologies Co., Ltd.";
	case 1594:
		return "Prolojik Limited";
	case 1595:
		return "Kromek Group Plc";
	case 1596:
		return "Contec Medical Systems Co., Ltd.";
	case 1597:
		return "Xradio Technology Co.,Ltd.";
	case 1598:
		return "The Indoor Lab, LLC";
	case 1599:
		return "LDL TECHNOLOGY";
	case 1600:
		return "Parkifi";
	case 1601:
		return "Revenue Collection Systems FRANCE SAS";
	case 1602:
		return "Bluetrum Technology Co.,Ltd";
	case 1603:
		return "makita corporation";
	case 1604:
		return "Apogee Instruments";
	case 1605:
		return "BM3";
	case 1606:
		return "SGV Group Holding GmbH & Co. KG";
	case 1607:
		return "MED-EL";
	case 1608:
		return "Ultune Technologies";
	case 1609:
		return "Ryeex Technology Co.,Ltd.";
	case 1610:
		return "Open Research Institute, Inc.";
	case 1611:
		return "Scale-Tec, Ltd";
	case 1612:
		return "Zumtobel Group AG";
	case 1613:
		return "iLOQ Oy";
	case 1614:
		return "KRUXWorks Technologies Private Limited";
	case 1615:
		return "Digital Matter Pty Ltd";
	case 1616:
		return "Coravin, Inc.";
	case 1617:
		return "Stasis Labs, Inc.";
	case 1618:
		return "ITZ Innovations- und Technologiezentrum GmbH";
	case 1619:
		return "Meggitt SA";
	case 1620:
		return "Ledlenser GmbH & Co. KG";
	case 1621:
		return "Renishaw PLC";
	case 1622:
		return "ZhuHai AdvanPro Technology Company Limited";
	case 1623:
		return "Meshtronix Limited";
	case 1624:
		return "Payex Norge AS";
	case 1625:
		return "UnSeen Technologies Oy";
	case 1626:
		return "Zound Industries International AB";
	case 1627:
		return "Sesam Solutions BV";
	case 1628:
		return "PixArt Imaging Inc.";
	case 1629:
		return "Panduit Corp.";
	case 1630:
		return "Alo AB";
	case 1631:
		return "Ricoh Company Ltd";
	case 1632:
		return "RTC Industries, Inc.";
	case 1633:
		return "Mode Lighting Limited";
	case 1634:
		return "Particle Industries, Inc.";
	case 1635:
		return "Advanced Telemetry Systems, Inc.";
	case 1636:
		return "RHA TECHNOLOGIES LTD";
	case 1637:
		return "Pure International Limited";
	case 1638:
		return "WTO Werkzeug-Einrichtungen GmbH";
	case 1639:
		return "Spark Technology Labs Inc.";
	case 1640:
		return "Bleb Technology srl";
	case 1641:
		return "Livanova USA, Inc.";
	case 1642:
		return "Brady Worldwide Inc.";
	case 1643:
		return "DewertOkin GmbH";
	case 1644:
		return "Ztove ApS";
	case 1645:
		return "Venso EcoSolutions AB";
	case 1646:
		return "Eurotronik Kranj d.o.o.";
	case 1647:
		return "Hug Technology Ltd";
	case 1648:
		return "Gema Switzerland GmbH";
	case 1649:
		return "Buzz Products Ltd.";
	case 1650:
		return "Kopi";
	case 1651:
		return "Innova Ideas Limited";
	case 1652:
		return "BeSpoon";
	case 1653:
		return "Deco Enterprises, Inc.";
	case 1654:
		return "Expai Solutions Private Limited";
	case 1655:
		return "Innovation First, Inc.";
	case 1656:
		return "SABIK Offshore GmbH";
	case 1657:
		return "4iiii Innovations Inc.";
	case 1658:
		return "The Energy Conservatory, Inc.";
	case 1659:
		return "I.FARM, INC.";
	case 1660:
		return "Tile, Inc.";
	case 1661:
		return "Form Athletica Inc.";
	case 1662:
		return "MbientLab Inc";
	case 1663:
		return "NETGRID S.N.C. DI BISSOLI MATTEO, CAMPOREALE SIMONE, TOGNETTI FEDERICO";
	case 1664:
		return "Mannkind Corporation";
	case 1665:
		return "Trade FIDES a.s.";
	case 1666:
		return "Photron Limited";
	case 1667:
		return "Eltako GmbH";
	case 1668:
		return "Dermalapps, LLC";
	case 1669:
		return "Greenwald Industries";
	case 1670:
		return "inQs Co., Ltd.";
	case 1671:
		return "Cherry GmbH";
	case 1672:
		return "Amsted Digital Solutions Inc.";
	case 1673:
		return "Tacx b.v.";
	case 1674:
		return "Raytac Corporation";
	case 1675:
		return "Jiangsu Teranovo Tech Co., Ltd.";
	case 1676:
		return "Changzhou Sound Dragon Electronics and Acoustics Co., Ltd";
	case 1677:
		return "JetBeep Inc.";
	case 1678:
		return "Razer Inc.";
	case 1679:
		return "JRM Group Limited";
	case 1680:
		return "Eccrine Systems, Inc.";
	case 1681:
		return "Curie Point AB";
	case 1682:
		return "Georg Fischer AG";
	case 1683:
		return "Hach - Danaher";
	case 1684:
		return "T&A Laboratories LLC";
	case 1685:
		return "Koki Holdings Co., Ltd.";
	case 1686:
		return "Gunakar Private Limited";
	case 1687:
		return "Stemco Products Inc";
	case 1688:
		return "Wood IT Security, LLC";
	case 1689:
		return "RandomLab SAS";
	case 1690:
		return "Adero, Inc. (formerly as TrackR, Inc.)";
	case 1691:
		return "Dragonchip Limited";
	case 1692:
		return "Noomi AB";
	case 1693:
		return "Vakaros LLC";
	case 1694:
		return "Delta Electronics, Inc.";
	case 1695:
		return "FlowMotion Technologies AS";
	case 1696:
		return "OBIQ Location Technology Inc.";
	case 1697:
		return "Cardo Systems, Ltd";
	case 1698:
		return "Globalworx GmbH";
	case 1699:
		return "Nymbus, LLC";
	case 1700:
		return "Sanyo Techno Solutions Tottori Co., Ltd.";
	case 1701:
		return "TEKZITEL PTY LTD";
	case 1702:
		return "Roambee Corporation";
	case 1703:
		return "Chipsea Technologies (ShenZhen) Corp.";
	case 1704:
		return "GD Midea Air-Conditioning Equipment Co., Ltd.";
	case 1705:
		return "Soundmax Electronics Limited";
	case 1706:
		return "Produal Oy";
	case 1707:
		return "HMS Industrial Networks AB";
	case 1708:
		return "Ingchips Technology Co., Ltd.";
	case 1709:
		return "InnovaSea Systems Inc.";
	case 1710:
		return "SenseQ Inc.";
	case 1711:
		return "Shoof Technologies";
	case 1712:
		return "BRK Brands, Inc.";
	case 1713:
		return "SimpliSafe, Inc.";
	case 1714:
		return "Tussock Innovation 2013 Limited";
	case 1715:
		return "The Hablab ApS";
	case 1716:
		return "Sencilion Oy";
	case 1717:
		return "Wabilogic Ltd.";
	case 1718:
		return "Sociometric Solutions, Inc.";
	case 1719:
		return "iCOGNIZE GmbH";
	case 1720:
		return "ShadeCraft, Inc";
	case 1721:
		return "Beflex Inc.";
	case 1722:
		return "Beaconzone Ltd";
	case 1723:
		return "Leaftronix Analogic Solutions Private Limited";
	case 1724:
		return "TWS Srl";
	case 1725:
		return "ABB Oy";
	case 1726:
		return "HitSeed Oy";
	case 1727:
		return "Delcom Products Inc.";
	case 1728:
		return "CAME S.p.A.";
	case 1729:
		return "Alarm.com Holdings, Inc";
	case 1730:
		return "Measurlogic Inc.";
	case 1731:
		return "King I Electronics.Co.,Ltd";
	case 1732:
		return "Dream Labs GmbH";
	case 1733:
		return "Urban Compass, Inc";
	case 1734:
		return "Simm Tronic Limited";
	case 1735:
		return "Somatix Inc";
	case 1736:
		return "Storz & Bickel GmbH & Co. KG";
	case 1737:
		return "MYLAPS B.V.";
	case 1738:
		return "Shenzhen Zhongguang Infotech Technology Development Co., Ltd";
	case 1739:
		return "Dyeware, LLC";
	case 1740:
		return "Dongguan SmartAction Technology Co.,Ltd.";
	case 1741:
		return "DIG Corporation";
	case 1742:
		return "FIOR & GENTZ";
	case 1743:
		return "Belparts N.V.";
	case 1744:
		return "Etekcity Corporation";
	case 1745:
		return "Meyer Sound Laboratories, Incorporated";
	case 1746:
		return "CeoTronics AG";
	case 1747:
		return "TriTeq Lock and Security, LLC";
	case 1748:
		return "DYNAKODE TECHNOLOGY PRIVATE LIMITED";
	case 1749:
		return "Sensirion AG";
	case 1750:
		return "JCT Healthcare Pty Ltd";
	case 1751:
		return "FUBA Automotive Electronics GmbH";
	case 1752:
		return "AW Company";
	case 1753:
		return "Shanghai Mountain View Silicon Co.,Ltd.";
	case 1754:
		return "Zliide Technologies ApS";
	case 1755:
		return "Automatic Labs, Inc.";
	case 1756:
		return "Industrial Network Controls, LLC";
	case 1757:
		return "Intellithings Ltd.";
	case 1758:
		return "Navcast, Inc.";
	case 1759:
		return "Hubbell Lighting, Inc.";
	case 1760:
		return "Avaya ";
	case 1761:
		return "Milestone AV Technologies LLC";
	case 1762:
		return "Alango Technologies Ltd";
	case 1763:
		return "Spinlock Ltd";
	case 1764:
		return "Aluna";
	case 1765:
		return "OPTEX CO.,LTD.";
	case 1766:
		return "NIHON DENGYO KOUSAKU";
	case 1767:
		return "VELUX A/S";
	case 1768:
		return "Almendo Technologies GmbH";
	case 1769:
		return "Zmartfun Electronics, Inc.";
	case 1770:
		return "SafeLine Sweden AB";
	case 1771:
		return "Houston Radar LLC";
	case 1772:
		return "Sigur";
	case 1773:
		return "J Neades Ltd";
	case 1774:
		return "Avantis Systems Limited";
	case 1775:
		return "ALCARE Co., Ltd.";
	case 1776:
		return "Chargy Technologies, SL";
	case 1777:
		return "Shibutani Co., Ltd.";
	case 1778:
		return "Trapper Data AB";
	case 1779:
		return "Alfred International Inc.";
	case 1780:
		return "Near Field Solutions Ltd";
	case 1781:
		return "Vigil Technologies Inc.";
	case 1782:
		return "Vitulo Plus BV";
	case 1783:
		return "WILKA Schliesstechnik GmbH";
	case 1784:
		return "BodyPlus Technology Co.,Ltd";
	case 1785:
		return "happybrush GmbH";
	case 1786:
		return "Enequi AB";
	case 1787:
		return "Sartorius AG";
	case 1788:
		return "Tom Communication Industrial Co.,Ltd.";
	case 1789:
		return "ESS Embedded System Solutions Inc.";
	case 1790:
		return "Mahr GmbH";
	case 1791:
		return "Redpine Signals Inc";
	case 1792:
		return "TraqFreq LLC";
	case 1793:
		return "PAFERS TECH";
	case 1794:
		return "Akciju sabiedriba \"SAF TEHNIKA\"";
	case 1795:
		return "Beijing Jingdong Century Trading Co., Ltd.";
	case 1796:
		return "JBX Designs Inc.";
	case 1797:
		return "AB Electrolux";
	case 1798:
		return "Wernher von Braun Center for ASdvanced Research";
	case 1799:
		return "Essity Hygiene and Health Aktiebolag";
	case 1800:
		return "Be Interactive Co., Ltd";
	case 1801:
		return "Carewear Corp.";
	case 1802:
		return "Huf Hülsbeck & Fürst GmbH & Co. KG";
	case 1803:
		return "Element Products, Inc.";
	case 1804:
		return "Beijing Winner Microelectronics Co.,Ltd";
	case 1805:
		return "SmartSnugg Pty Ltd";
	case 1806:
		return "FiveCo Sarl";
	case 1807:
		return "California Things Inc.";
	case 1808:
		return "Audiodo AB";
	case 1809:
		return "ABAX AS";
	case 1810:
		return "Bull Group Company Limited";
	case 1811:
		return "Respiri Limited";
	case 1812:
		return "MindPeace Safety LLC";
	case 1813:
		return "Vgyan Solutions";
	case 1814:
		return "Altonics";
	case 1815:
		return "iQsquare BV";
	case 1816:
		return "IDIBAIX enginneering";
	case 1817:
		return "ECSG";
	case 1818:
		return "REVSMART WEARABLE HK CO LTD";
	case 1819:
		return "Precor";
	case 1820:
		return "F5 Sports, Inc";
	case 1821:
		return "exoTIC Systems";
	case 1822:
		return "DONGGUAN HELE ELECTRONICS CO., LTD";
	case 1823:
		return "Dongguan Liesheng Electronic Co.Ltd";
	case 1824:
		return "Oculeve, Inc.";
	case 1825:
		return "Clover Network, Inc.";
	case 1826:
		return "Xiamen Eholder Electronics Co.Ltd";
	case 1827:
		return "Ford Motor Company";
	case 1828:
		return "Guangzhou SuperSound Information Technology Co.,Ltd";
	case 1829:
		return "Tedee Sp. z o.o.";
	case 1830:
		return "PHC Corporation";
	case 1831:
		return "STALKIT AS";
	case 1832:
		return "Eli Lilly and Company";
	case 1833:
		return "SwaraLink Technologies";
	case 1834:
		return "JMR embedded systems GmbH";
	case 1835:
		return "Bitkey Inc.";
	case 1836:
		return "GWA Hygiene GmbH";
	case 1837:
		return "Safera Oy";
	case 1838:
		return "Open Platform Systems LLC";
	case 1839:
		return "OnePlus Electronics (Shenzhen) Co., Ltd.";
	case 1840:
		return "Wildlife Acoustics, Inc.";
	case 1841:
		return "ABLIC Inc.";
	case 1842:
		return "Dairy Tech, Inc.";
	case 1843:
		return "Iguanavation, Inc.";
	case 1844:
		return "DiUS Computing Pty Ltd";
	case 1845:
		return "UpRight Technologies LTD";
	case 1846:
		return "FrancisFund, LLC";
	case 1847:
		return "LLC Navitek";
	case 1848:
		return "Glass Security Pte Ltd";
	case 1849:
		return "Jiangsu Qinheng Co., Ltd.";
	case 1850:
		return "Chandler Systems Inc.";
	case 1851:
		return "Fantini Cosmi s.p.a.";
	case 1852:
		return "Acubit ApS";
	case 1853:
		return "Beijing Hao Heng Tian Tech Co., Ltd.";
	case 1854:
		return "Bluepack S.R.L.";
	case 1855:
		return "Beijing Unisoc Technologies Co., Ltd.";
	case 1856:
		return "HITIQ LIMITED";
	case 1857:
		return "MAC SRL";
	case 1858:
		return "DML LLC";
	case 1859:
		return "Sanofi";
	case 1860:
		return "SOCOMEC";
	case 1861:
		return "WIZNOVA, Inc.";
	case 1862:
		return "Seitec Elektronik GmbH";
	case 1863:
		return "OR Technologies Pty Ltd";
	case 1864:
		return "GuangZhou KuGou Computer Technology Co.Ltd";
	case 1865:
		return "DIAODIAO (Beijing) Technology Co., Ltd.";
	case 1866:
		return "Illusory Studios LLC";
	case 1867:
		return "Sarvavid Software Solutions LLP";
	case 1868:
		return "iopool s.a.";
	case 1869:
		return "Amtech Systems, LLC";
	case 1870:
		return "EAGLE DETECTION SA";
	case 1871:
		return "MEDIATECH S.R.L.";
	case 1872:
		return "Hamilton Professional Services of Canada Incorporated";
	case 1873:
		return "Changsha JEMO IC Design Co.,Ltd";
	case 1874:
		return "Elatec GmbH";
	case 1875:
		return "JLG Industries, Inc.";
	case 1876:
		return "Michael Parkin";
	case 1877:
		return "Brother Industries, Ltd";
	case 1878:
		return "Lumens For Less, Inc";
	case 1879:
		return "ELA Innovation";
	case 1880:
		return "umanSense AB";
	case 1881:
		return "Shanghai InGeek Cyber Security Co., Ltd.";
	case 1882:
		return "HARMAN CO.,LTD.";
	case 1883:
		return "Smart Sensor Devices AB";
	case 1884:
		return "Antitronics Inc.";
	case 1885:
		return "RHOMBUS SYSTEMS, INC.";
	case 1886:
		return "Katerra Inc.";
	case 1887:
		return "Remote Solution Co., LTD.";
	case 1888:
		return "Vimar SpA";
	case 1889:
		return "Mantis Tech LLC";
	case 1890:
		return "TerOpta Ltd";
	case 1891:
		return "PIKOLIN S.L.";
	case 1892:
		return "WWZN Information Technology Company Limited";
	case 1893:
		return "Voxx International";
	case 1894:
		return "ART AND PROGRAM, INC.";
	case 1895:
		return "NITTO DENKO ASIA TECHNICAL CENTRE PTE. LTD.";
	case 1896:
		return "Peloton Interactive Inc.";
	case 1897:
		return "Force Impact Technologies";
	case 1898:
		return "Dmac Mobile Developments, LLC";
	case 1899:
		return "Engineered Medical Technologies";
	case 1900:
		return "Noodle Technology inc";
	case 1901:
		return "Graesslin GmbH";
	case 1902:
		return "WuQi technologies, Inc.";
	case 1903:
		return "Successful Endeavours Pty Ltd";
	case 1904:
		return "InnoCon Medical ApS";
	case 1905:
		return "Corvex Connected Safety";
	case 1906:
		return "Thirdwayv Inc.";
	case 1907:
		return "Echoflex Solutions Inc.";
	case 1908:
		return "C-MAX Asia Limited";
	case 1909:
		return "4eBusiness GmbH";
	case 1910:
		return "Cyber Transport Control GmbH";
	case 1911:
		return "Cue";
	case 1912:
		return "KOAMTAC INC.";
	case 1913:
		return "Loopshore Oy";
	case 1914:
		return "Niruha Systems Private Limited";
	case 1915:
		return "AmaterZ, Inc.";
	case 1916:
		return "radius co., ltd.";
	case 1917:
		return "Sensority, s.r.o.";
	case 1918:
		return "Sparkage Inc.";
	case 1919:
		return "Glenview Software Corporation";
	case 1920:
		return "Finch Technologies Ltd.";
	case 1921:
		return "Qingping Technology (Beijing) Co., Ltd.";
	case 1922:
		return "DeviceDrive AS";
	case 1923:
		return "ESEMBER LIMITED LIABILITY COMPANY";
	case 1924:
		return "audifon GmbH & Co. KG";
	case 1925:
		return "O2 Micro, Inc.";
	case 1926:
		return "HLP Controls Pty Limited";
	case 1927:
		return "Pangaea Solution";
	case 1928:
		return "BubblyNet, LLC";
	case 1930:
		return "The Wildflower Foundation";
	case 1931:
		return "Optikam Tech Inc.";
	case 1932:
		return "MINIBREW HOLDING B.V";
	case 1933:
		return "Cybex GmbH";
	case 1934:
		return "FUJIMIC NIIGATA, INC.";
	case 1935:
		return "Hanna Instruments, Inc.";
	case 1936:
		return "KOMPAN A/S";
	case 1937:
		return "Scosche Industries, Inc.";
	case 1938:
		return "Provo Craft";
	case 1939:
		return "AEV spol. s r.o.";
	case 1940:
		return "The Coca-Cola Company";
	case 1941:
		return "GASTEC CORPORATION";
	case 1942:
		return "StarLeaf Ltd";
	case 1943:
		return "Water-i.d. GmbH";
	case 1944:
		return "HoloKit, Inc.";
	case 1945:
		return "PlantChoir Inc.";
	case 1946:
		return "GuangDong Oppo Mobile Telecommunications Corp., Ltd.";
	case 1947:
		return "CST ELECTRONICS (PROPRIETARY) LIMITED";
	case 1948:
		return "Sky UK Limited";
	case 1949:
		return "Digibale Pty Ltd";
	case 1950:
		return "Smartloxx GmbH";
	case 1951:
		return "Pune Scientific LLP";
	case 1952:
		return "Regent Beleuchtungskorper AG";
	case 1953:
		return "Apollo Neuroscience, Inc.";
	case 1954:
		return "Roku, Inc.";
	case 1955:
		return "Comcast Cable";
	case 1956:
		return "Xiamen Mage Information Technology Co., Ltd.";
	case 1957:
		return "RAB Lighting, Inc.";
	case 1958:
		return "Musen Connect, Inc.";
	case 1959:
		return "Zume, Inc.";
	case 1960:
		return "conbee GmbH";
	case 1961:
		return "Bruel & Kjaer Sound & Vibration";
	case 1962:
		return "The Kroger Co.";
	case 1963:
		return "Granite River Solutions, Inc.";
	case 1964:
		return "LoupeDeck Oy";
	case 1965:
		return "New H3C Technologies Co.,Ltd";
	case 1966:
		return "Aurea Solucoes Tecnologicas Ltda.";
	case 1967:
		return "Hong Kong Bouffalo Lab Limited";
	case 1968:
		return "GV Concepts Inc.";
	case 1969:
		return "Thomas Dynamics, LLC";
	case 1970:
		return "Moeco IOT Inc.";
	case 1971:
		return "2N TELEKOMUNIKACE a.s.";
	case 1972:
		return "Hormann KG Antriebstechnik";
	case 1973:
		return "CRONO CHIP, S.L.";
	case 1974:
		return "Soundbrenner Limited";
	case 1975:
		return "ETABLISSEMENTS GEORGES RENAULT";
	case 1976:
		return "iSwip";
	case 1977:
		return "Epona Biotec Limited";
	case 1978:
		return "Battery-Biz Inc.";
	case 1979:
		return "EPIC S.R.L.";
	case 1980:
		return "KD CIRCUITS LLC";
	case 1981:
		return "Genedrive Diagnostics Ltd";
	case 1982:
		return "Axentia Technologies AB";
	case 1983:
		return "REGULA Ltd.";
	case 1984:
		return "Biral AG";
	case 1985:
		return "A.W. Chesterton Company";
	case 1986:
		return "Radinn AB";
	case 1987:
		return "CIMTechniques, Inc.";
	case 1988:
		return "Johnson Health Tech NA";
	case 1989:
		return "June Life, Inc.";
	case 1990:
		return "Bluenetics GmbH";
	case 1991:
		return "iaconicDesign Inc.";
	case 1992:
		return "WRLDS Creations AB";
	case 1993:
		return "Skullcandy, Inc.";
	case 1994:
		return "Modul-System HH AB";
	case 1995:
		return "West Pharmaceutical Services, Inc.";
	case 1996:
		return "Barnacle Systems Inc.";
	case 1997:
		return "Smart Wave Technologies Canada Inc";
	case 1998:
		return "Shanghai Top-Chip Microelectronics Tech. Co., LTD";
	case 1999:
		return "NeoSensory, Inc.";
	case 2000:
		return "Hangzhou Tuya Information  Technology Co., Ltd";
	case 2001:
		return "Shanghai Panchip Microelectronics Co., Ltd";
	case 2002:
		return "React Accessibility Limited";
	case 2003:
		return "LIVNEX Co.,Ltd.";
	case 2004:
		return "Kano Computing Limited";
	case 2005:
		return "hoots classic GmbH";
	case 2006:
		return "ecobee Inc.";
	case 2007:
		return "Nanjing Qinheng Microelectronics Co., Ltd";
	case 2008:
		return "SOLUTIONS AMBRA INC.";
	case 2009:
		return "Micro-Design, Inc.";
	case 2010:
		return "STARLITE Co., Ltd.";
	case 2011:
		return "Remedee Labs";
	case 2012:
		return "ThingOS GmbH";
	case 2013:
		return "Linear Circuits";
	case 2014:
		return "Unlimited Engineering SL";
	case 2015:
		return "Snap-on Incorporated";
	case 2016:
		return "Edifier International Limited";
	case 2017:
		return "Lucie Labs";
	case 2018:
		return "Alfred Kaercher SE & Co. KG";
	case 2019:
		return "Audiowise Technology Inc.";
	case 2020:
		return "Geeksme S.L.";
	case 2021:
		return "Minut, Inc.";
	case 2022:
		return "Autogrow Systems Limited";
	case 2023:
		return "Komfort IQ, Inc.";
	case 2024:
		return "Packetcraft, Inc.";
	case 2025:
		return "Häfele GmbH & Co KG";
	case 2026:
		return "ShapeLog, Inc.";
	case 2027:
		return "NOVABASE S.R.L.";
	case 2028:
		return "Frecce LLC";
	case 2029:
		return "Joule IQ, INC.";
	case 2030:
		return "KidzTek LLC";
	case 2031:
		return "Aktiebolaget Sandvik Coromant";
	case 2032:
		return "e-moola.com Pty Ltd";
	case 2033:
		return "GSM Innovations Pty Ltd";
	case 2034:
		return "SERENE GROUP, INC";
	case 2035:
		return "DIGISINE ENERGYTECH CO. LTD.";
	case 2036:
		return "MEDIRLAB Orvosbiologiai Fejleszto Korlatolt Felelossegu Tarsasag";
	case 2037:
		return "Byton North America Corporation";
	case 2038:
		return "Shenzhen TonliScience and Technology Development Co.,Ltd";
	case 2039:
		return "Cesar Systems Ltd.";
	case 2040:
		return "quip NYC Inc.";
	case 2041:
		return "Direct Communication Solutions, Inc.";
	case 2042:
		return "Klipsch Group, Inc.";
	case 2043:
		return "Access Co., Ltd";
	case 2044:
		return "Renault SA";
	case 2045:
		return "JSK CO., LTD.";
	case 2046:
		return "BIROTA";
	case 2047:
		return "maxon motor ltd.";
	case 2048:
		return "Optek";
	case 2049:
		return "CRONUS ELECTRONICS LTD";
	case 2050:
		return "NantSound, Inc.";
	case 2051:
		return "Domintell s.a.";
	case 2052:
		return "Andon Health Co.,Ltd";
	case 2053:
		return "Urbanminded Ltd";
	case 2054:
		return "TYRI Sweden AB";
	case 2055:
		return "ECD Electronic Components GmbH Dresden";
	case 2056:
		return "SISTEMAS KERN, SOCIEDAD ANÓMINA";
	case 2057:
		return "Trulli Audio";
	case 2058:
		return "Altaneos";
	case 2059:
		return "Nanoleaf Canada Limited";
	case 2060:
		return "Ingy B.V.";
	case 2061:
		return "Azbil Co.";
	case 2062:
		return "TATTCOM LLC";
	case 2063:
		return "Paradox Engineering SA";
	case 2064:
		return "LECO Corporation";
	case 2065:
		return "Becker Antriebe GmbH";
	case 2066:
		return "Mstream Technologies., Inc.";
	case 2067:
		return "Flextronics International USA Inc.";
	case 2068:
		return "Ossur hf.";
	case 2069:
		return "SKC Inc";
	case 2070:
		return "SPICA SYSTEMS LLC";
	case 2071:
		return "Wangs Alliance Corporation";
	case 2072:
		return "tatwah SA";
	case 2073:
		return "Hunter Douglas Inc";
	case 2074:
		return "Shenzhen Conex";
	case 2075:
		return "DIM3";
	case 2076:
		return "Bobrick Washroom Equipment, Inc.";
	case 2077:
		return "Potrykus Holdings and Development LLC";
	case 2078:
		return "iNFORM Technology GmbH";
	case 2079:
		return "eSenseLab LTD";
	case 2080:
		return "Brilliant Home Technology, Inc.";
	case 2081:
		return "INOVA Geophysical, Inc.";
	case 2082:
		return "adafruit industries";
	case 2083:
		return "Nexite Ltd";
	case 2084:
		return "8Power Limited";
	case 2085:
		return "CME PTE. LTD.";
	case 2086:
		return "Hyundai Motor Company";
	case 2087:
		return "Kickmaker";
	case 2088:
		return "Shanghai Suisheng Information Technology Co., Ltd.";
	case 2089:
		return "HEXAGON";
	case 2090:
		return "Mitutoyo Corporation";
	case 2091:
		return "shenzhen fitcare electronics Co.,Ltd";
	case 2092:
		return "INGICS TECHNOLOGY CO., LTD.";
	case 2093:
		return "INCUS PERFORMANCE LTD.";
	case 2094:
		return "ABB S.p.A.";
	case 2095:
		return "Blippit AB";
	case 2096:
		return "Core Health and Fitness LLC";
	case 2097:
		return "Foxble, LLC";
	case 2098:
		return "Intermotive,Inc.";
	case 2099:
		return "Conneqtech B.V.";
	case 2100:
		return "RIKEN KEIKI CO., LTD.,";
	case 2101:
		return "Canopy Growth Corporation";
	case 2102:
		return "Bitwards Oy";
	case 2103:
		return "vivo Mobile Communication Co., Ltd.";
	case 2104:
		return "Etymotic Research, Inc.";
	case 2105:
		return "A puissance 3";
	case 2106:
		return "BPW Bergische Achsen Kommanditgesellschaft";
	case 2107:
		return "Piaggio Fast Forward";
	case 2108:
		return "BeerTech LTD";
	case 2109:
		return "Tokenize, Inc.";
	case 2110:
		return "Zorachka LTD";
	case 2111:
		return "D-Link Corp.";
	case 2112:
		return "Down Range Systems LLC";
	case 2113:
		return "General Luminaire (Shanghai) Co., Ltd.";
	case 2114:
		return "Tangshan HongJia electronic technology co., LTD.";
	case 2115:
		return "FRAGRANCE DELIVERY TECHNOLOGIES LTD";
	case 2116:
		return "Pepperl + Fuchs GmbH";
	case 2117:
		return "Dometic Corporation";
	case 2118:
		return "USound GmbH";
	case 2119:
		return "DNANUDGE LIMITED";
	case 2120:
		return "JUJU JOINTS CANADA CORP.";
	case 2121:
		return "Dopple Technologies B.V.";
	case 2122:
		return "ARCOM";
	case 2123:
		return "Biotechware SRL";
	case 2124:
		return "ORSO Inc.";
	case 2125:
		return "SafePort";
	case 2126:
		return "Carol Cole Company";
	case 2127:
		return "Embedded Fitness B.V.";
	case 2128:
		return "Yealink (Xiamen) Network Technology Co.,LTD";
	case 2129:
		return "Subeca, Inc.";
	case 2130:
		return "Cognosos, Inc.";
	case 2131:
		return "Pektron Group Limited";
	case 2132:
		return "Tap Sound System";
	case 2133:
		return "Helios Hockey, Inc.";
	case 2134:
		return "Canopy Growth Corporation";
	case 2135:
		return "Parsyl Inc";
	case 2136:
		return "SOUNDBOKS";
	case 2137:
		return "BlueUp";
	case 2138:
		return "DAKATECH";
	case 2139:
		return "RICOH ELECTRONIC DEVICES CO., LTD.";
	case 2140:
		return "ACOS CO.,LTD.";
	case 2141:
		return "Guilin Zhishen Information Technology Co.,Ltd.";
	case 2142:
		return "Krog Systems LLC";
	case 2143:
		return "COMPEGPS TEAM,SOCIEDAD LIMITADA";
	case 2144:
		return "Alflex Products B.V.";
	case 2145:
		return "SmartSensor Labs Ltd";
	case 2146:
		return "SmartDrive Inc.";
	case 2147:
		return "Yo-tronics Technology Co., Ltd.";
	case 2148:
		return "Rafaelmicro";
	case 2149:
		return "Emergency Lighting Products Limited";
	case 2150:
		return "LAONZ Co.,Ltd";
	case 2151:
		return "Western Digital Techologies, Inc.";
	case 2152:
		return "WIOsense GmbH & Co. KG";
	case 2153:
		return "EVVA Sicherheitstechnologie GmbH";
	case 2154:
		return "Odic Incorporated";
	case 2155:
		return "Pacific Track, LLC";
	case 2156:
		return "Revvo Technologies, Inc.";
	case 2157:
		return "Biometrika d.o.o.";
	case 2158:
		return "Vorwerk Elektrowerke GmbH & Co. KG";
	case 2159:
		return "Trackunit A/S";
	case 2160:
		return "Wyze Labs, Inc";
	case 2161:
		return "Dension Elektronikai Kft. (formerly: Dension Audio Systems Ltd.)";
	case 2162:
		return "11 Health & Technologies Limited";
	case 2163:
		return "Innophase Incorporated";
	case 2164:
		return "Treegreen Limited";
	case 2165:
		return "Berner International LLC";
	case 2166:
		return "SmartResQ ApS";
	case 2167:
		return "Tome, Inc.";
	case 2168:
		return "The Chamberlain Group, Inc.";
	case 2169:
		return "MIZUNO Corporation";
	case 2170:
		return "ZRF, LLC";
	case 2171:
		return "BYSTAMP";
	case 2172:
		return "Crosscan GmbH";
	case 2173:
		return "Konftel AB";
	case 2174:
		return "1bar.net Limited";
	case 2175:
		return "Phillips Connect Technologies LLC";
	case 2176:
		return "imagiLabs AB";
	case 2177:
		return "Optalert";
	case 2178:
		return "PSYONIC, Inc.";
	case 2179:
		return "Wintersteiger AG";
	case 2180:
		return "Controlid Industria, Comercio de Hardware e Servicos de Tecnologia Ltda";
	case 2181:
		return "LEVOLOR, INC.";
	case 2182:
		return "Xsens Technologies B.V.";
	case 2183:
		return "Hydro-Gear Limited Partnership";
	case 2184:
		return "EnPointe Fencing Pty Ltd";
	case 2185:
		return "XANTHIO";
	case 2186:
		return "sclak s.r.l.";
	case 2187:
		return "Tricorder Arraay Technologies LLC";
	case 2188:
		return "GB Solution co.,Ltd";
	case 2189:
		return "Soliton Systems K.K.";
	case 2190:
		return "GIGA-TMS INC";
	case 2191:
		return "Tait International Limited";
	case 2192:
		return "NICHIEI INTEC CO., LTD.";
	case 2193:
		return "SmartWireless GmbH & Co. KG";
	case 2194:
		return "Ingenieurbuero Birnfeld UG (haftungsbeschraenkt)";
	case 2195:
		return "Maytronics Ltd";
	case 2196:
		return "EPIFIT";
	case 2197:
		return "Gimer medical";
	case 2198:
		return "Nokian Renkaat Oyj";
	case 2199:
		return "Current Lighting Solutions LLC";
	case 2200:
		return "Sensibo, Inc.";
	case 2201:
		return "SFS unimarket AG";
	case 2202:
		return "Private limited company \"Teltonika\"";
	case 2203:
		return "Saucon Technologies";
	case 2204:
		return "Embedded Devices Co. Company";
	case 2205:
		return "J-J.A.D.E. Enterprise LLC";
	case 2206:
		return "i-SENS, inc.";
	case 2207:
		return "Witschi Electronic Ltd";
	case 2208:
		return "Aclara Technologies LLC";
	case 2209:
		return "EXEO TECH CORPORATION";
	case 2210:
		return "Epic Systems Co., Ltd.";
	case 2211:
		return "Hoffmann SE";
	case 2212:
		return "Realme Chongqing Mobile Telecommunications Corp., Ltd.";
	case 2213:
		return "UMEHEAL Ltd";
	case 2214:
		return "Intelligenceworks Inc.";
	case 2215:
		return "TGR 1.618 Limited";
	case 2216:
		return "Shanghai Kfcube Inc";
	case 2217:
		return "Fraunhofer IIS";
	case 2218:
		return "SZ DJI TECHNOLOGY CO.,LTD";
	case 2219:
		return "Coburn Technology, LLC";
	case 2220:
		return "Topre Corporation";
	case 2221:
		return "Kayamatics Limited";
	case 2222:
		return "Moticon ReGo AG";
	case 2223:
		return "Polidea Sp. z o.o.";
	case 2224:
		return "Trivedi Advanced Technologies LLC";
	case 2225:
		return "CORE|vision BV";
	case 2226:
		return "PF SCHWEISSTECHNOLOGIE GMBH";
	case 2227:
		return "IONIQ Skincare GmbH & Co. KG";
	case 2228:
		return "Sengled Co., Ltd.";
	case 2229:
		return "TransferFi";
	case 2230:
		return "Boehringer Ingelheim Vetmedica GmbH";
	case 2231:
		return "ABB Inc";
	case 2232:
		return "Check Technology Solutions LLC";
	case 2233:
		return "U-Shin Ltd.";
	case 2234:
		return "HYPER ICE, INC.";
	case 2235:
		return "Tokai-rika co.,ltd.";
	case 2236:
		return "Prevayl Limited";
	case 2237:
		return "bf1systems limited";
	case 2238:
		return "ubisys technologies GmbH";
	case 2239:
		return "SIRC Co., Ltd.";
	case 2240:
		return "Accent Advanced Systems SLU";
	case 2241:
		return "Rayden.Earth LTD";
	case 2242:
		return "Lindinvent AB";
	case 2243:
		return "CHIPOLO d.o.o.";
	case 2244:
		return "CellAssist, LLC";
	case 2245:
		return "J. Wagner GmbH";
	case 2246:
		return "Integra Optics Inc";
	case 2247:
		return "Monadnock Systems Ltd.";
	case 2248:
		return "Liteboxer Technologies Inc.";
	case 2249:
		return "Noventa AG";
	case 2250:
		return "Nubia Technology Co.,Ltd.";
	case 2251:
		return "JT INNOVATIONS LIMITED";
	case 2252:
		return "TGM TECHNOLOGY CO., LTD.";
	case 2253:
		return "ifly";
	case 2254:
		return "ZIMI CORPORATION";
	case 2255:
		return "betternotstealmybike UG (with limited liability)";
	case 2256:
		return "ESTOM Infotech Kft.";
	case 2257:
		return "Sensovium Inc.";
	case 2258:
		return "Virscient Limited";
	case 2259:
		return "Novel Bits, LLC";
	case 2260:
		return "ADATA Technology Co., LTD.";
	case 2261:
		return "KEYes";
	case 2262:
		return "Nome Oy";
	case 2263:
		return "Inovonics Corp";
	case 2264:
		return "WARES";
	case 2265:
		return "Pointr Labs Limited";
	case 2266:
		return "Miridia Technology Incorporated";
	case 2267:
		return "Tertium Technology";
	case 2268:
		return "SHENZHEN AUKEY E BUSINESS CO., LTD";
	case 2269:
		return "code-Q";
	case 2270:
		return "Tyco Electronics Corporation a TE Connectivity Ltd Company";
	case 2271:
		return "IRIS OHYAMA CO.,LTD.";
	case 2272:
		return "Philia Technology";
	case 2273:
		return "KOZO KEIKAKU ENGINEERING Inc.";
	case 2274:
		return "Shenzhen Simo Technology co. LTD";
	case 2275:
		return "Republic Wireless, Inc.";
	case 2276:
		return "Rashidov ltd";
	case 2277:
		return "Crowd Connected Ltd";
	case 2278:
		return "Eneso Tecnologia de Adaptacion S.L.";
	case 2279:
		return "Barrot Technology Limited";
	case 2280:
		return "Naonext";
	case 2281:
		return "Taiwan Intelligent Home Corp.";
	case 2282:
		return "COWBELL ENGINEERING CO.,LTD.";
	case 2283:
		return "Beijing Big Moment Technology Co., Ltd.";
	case 2284:
		return "Denso Corporation";
	case 2285:
		return "IMI Hydronic Engineering International SA";
	case 2286:
		return "ASKEY";
	case 2287:
		return "Cumulus Digital Systems, Inc";
	case 2288:
		return "Joovv, Inc.";
	case 2289:
		return "The L.S. Starrett Company";
	case 2290:
		return "Microoled";
	case 2291:
		return "PSP - Pauli Services & Products GmbH";
	case 2292:
		return "Kodimo Technologies Company Limited";
	case 2293:
		return "Tymtix Technologies Private Limited";
	case 2294:
		return "Dermal Photonics Corporation";
	case 2295:
		return "MTD Products Inc & Affiliates";
	case 2296:
		return "instagrid GmbH";
	case 2297:
		return "Spacelabs Medical Inc.";
	case 2298:
		return "Troo Corporation";
	case 2299:
		return "Darkglass Electronics Oy";
	case 2300:
		return "Hill-Rom";
	case 2301:
		return "BioIntelliSense, Inc.";
	case 2302:
		return "Ketronixs Sdn Bhd";
	case 2308:
		return "SUNCORPORATION";
	case 2309:
		return "Yandex Services AG";
	case 2310:
		return "Scope Logistical Solutions";
	case 2311:
		return "User Hello, LLC";
	case 2312:
		return "Pinpoint Innovations Limited";
	case 2313:
		return "70mai Co.,Ltd.";
	case 2314:
		return "Zhuhai Hoksi Technology CO.,LTD";
	case 2315:
		return "EMBR labs, INC";
	case 2316:
		return "Radiawave Technologies Co.,Ltd.";
	case 2317:
		return "IOT Invent GmbH";
	case 2318:
		return "OPTIMUSIOT TECH LLP";
	case 2319:
		return "VC Inc.";
	case 2320:
		return "ASR Microelectronics (Shanghai) Co., Ltd.";
	case 2321:
		return "Douglas Lighting Controls Inc.";
	case 2322:
		return "Nerbio Medical Software Platforms Inc";
	case 2323:
		return "Braveheart Wireless, Inc.";
	case 2324:
		return "INEO-SENSE";
	case 2325:
		return "Honda Motor Co., Ltd.";
	case 2326:
		return "Ambient Sensors LLC";
	case 2327:
		return "ASR Microelectronics(ShenZhen)Co., Ltd.";
	case 2328:
		return "Technosphere Labs Pvt. Ltd.";
	case 2329:
		return "NO SMD LIMITED";
	case 2330:
		return "Albertronic BV";
	case 2331:
		return "Luminostics, Inc.";
	case 2332:
		return "Oblamatik AG";
	case 2333:
		return "Innokind, Inc.";
	case 2334:
		return "Melbot Studios, Sociedad Limitada";
	case 2335:
		return "Myzee Technology";
	case 2336:
		return "Omnisense Limited";
	case 2337:
		return "KAHA PTE. LTD.";
	case 2338:
		return "Shanghai MXCHIP Information Technology Co., Ltd.";
	case 2339:
		return "JSB TECH PTE LTD";
	case 2340:
		return "Fundacion Tecnalia Research and Innovation";
	case 2341:
		return "Yukai Engineering Inc.";
	case 2342:
		return "Gooligum Technologies Pty Ltd";
	case 2343:
		return "ROOQ GmbH";
	case 2344:
		return "AiRISTA";
	case 2345:
		return "Qingdao Haier Technology Co., Ltd.";
	case 2346:
		return "Sappl Verwaltungs- und Betriebs GmbH";
	case 2347:
		return "TekHome";
	case 2348:
		return "PCI Private Limited";
	case 2349:
		return "Leggett & Platt, Incorporated";
	case 2350:
		return "PS GmbH";
	case 2351:
		return "C.O.B.O. SpA";
	case 2352:
		return "James Walker RotaBolt Limited";
	case 2353:
		return "BREATHINGS Co., Ltd.";
	case 2354:
		return "BarVision, LLC";
	case 2355:
		return "SRAM";
	case 2356:
		return "KiteSpring Inc.";
	case 2357:
		return "Reconnect, Inc.";
	case 2358:
		return "Elekon AG";
	case 2359:
		return "RealThingks GmbH";
	case 2360:
		return "Henway Technologies, LTD.";
	case 2361:
		return "ASTEM Co.,Ltd.";
	case 2362:
		return "LinkedSemi Microelectronics (Xiamen) Co., Ltd";
	case 2363:
		return "ENSESO LLC";
	case 2364:
		return "Xenoma Inc.";
	case 2365:
		return "Adolf Wuerth GmbH & Co KG";
	case 2366:
		return "Catalyft Labs, Inc.";
	case 2367:
		return "JEPICO Corporation";
	case 2368:
		return "Hero Workout GmbH";
	case 2369:
		return "Rivian Automotive, LLC";
	case 2370:
		return "TRANSSION HOLDINGS LIMITED";
	case 2371:
		return "Inovonics Corp.";
	case 2372:
		return "Agitron d.o.o.";
	case 2373:
		return "Globe (Jiangsu) Co., Ltd";
	case 2374:
		return "AMC International Alfa Metalcraft Corporation AG";
	case 2375:
		return "First Light Technologies Ltd.";
	case 2376:
		return "Wearable Link Limited";
	case 2377:
		return "Metronom Health Europe";
	case 2378:
		return "Zwift, Inc.";
	case 2379:
		return "Kindeva Drug Delivery L.P.";
	case 2380:
		return "GimmiSys GmbH";
	case 2381:
		return "tkLABS INC.";
	case 2382:
		return "PassiveBolt, Inc.";
	case 2383:
		return "Limited Liability Company \"Mikrotikls\"";
	case 2384:
		return "Capetech";
	case 2385:
		return "PPRS";
	case 2386:
		return "Apptricity Corporation";
	case 2387:
		return "LogiLube, LLC";
	case 2388:
		return "Julbo";
	case 2389:
		return "Breville Group";
	case 2390:
		return "Kerlink";
	case 2391:
		return "Ohsung Electronics";
	case 2392:
		return "ZTE Corporation";
	case 65535:
		return "internal use";
	default:
		return "not assigned";
	}
}
