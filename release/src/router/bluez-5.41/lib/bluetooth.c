/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2000-2001  Qualcomm Incorporated
 *  Copyright (C) 2002-2003  Maxim Krasnyansky <maxk@qualcomm.com>
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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
		return "Ceva, Inc. (formerly Parthus Technologies, Inc.)";
	case 15:
		return "Broadcom Corporation";
	case 16:
		return "Mitel Semiconductor";
	case 17:
		return "Widcomm, Inc";
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
		return "NewLogic";
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
		return "Terax";
	case 57:
		return "Integrated System Solution Corp.";
	case 58:
		return "Matsushita Electric Industrial Co., Ltd.";
	case 59:
		return "Gennum Corporation";
	case 60:
		return "BlackBerry Limited (formerly Research In Motion)";
	case 61:
		return "IPextreme, Inc.";
	case 62:
		return "Systems and Chips, Inc.";
	case 63:
		return "Bluetooth SIG, Inc.";
	case 64:
		return "Seiko Epson Corporation";
	case 65:
		return "Integrated Silicon Solution Taiwan, Inc.";
	case 66:
		return "CONWISE Technology Corporation Ltd";
	case 67:
		return "PARROT SA";
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
		return "APT Licensing Ltd.";
	case 80:
		return "SiRF Technology";
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
		return "BriarTek, Inc.";
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
		return "Seers Technology Co. Ltd";
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
		return "equinox AG";
	case 135:
		return "Garmin International, Inc.";
	case 136:
		return "Ecotest";
	case 137:
		return "GN ReSound A/S";
	case 138:
		return "Jawbone";
	case 139:
		return "Topcorn Positioning Systems, LLC";
	case 140:
		return "Gimbal Inc. (formerly Qualcomm Labs, Inc. and Qualcomm Retail Solutions, Inc.)";
	case 141:
		return "Zscan Software";
	case 142:
		return "Quintic Corp.";
	case 143:
		return "Telit Wireless Solutions GmbH (Formerly Stollman E+V GmbH)";
	case 144:
		return "Funai Electric Co., Ltd.";
	case 145:
		return "Advanced PANMOBIL Systems GmbH & Co. KG";
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
		return "zer01.tv GmbH";
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
		return "Magneti Marelli S.p.A";
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
		return "Server Technology, Inc.";
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
		return "Aether Things Inc. (formerly Morse Project Inc.)";
	case 243:
		return "Kent Displays Inc.";
	case 244:
		return "Nautilus Inc.";
	case 245:
		return "Smartifier Oy";
	case 246:
		return "Elcometer Limited";
	case 247:
		return "VSN Technologies Inc.";
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
		return "Reserved";
	case 255:
		return "Typo Products, LLC";
	case 256:
		return "TomTom International BV";
	case 257:
		return "Fugoo, Inc";
	case 258:
		return "Keiser Corporation";
	case 259:
		return "Bang & Olufsen A/S";
	case 260:
		return "PLUS Locations Systems Pty Ltd";
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
		return "Codegate Ltd.";
	case 267:
		return "ERi, Inc.";
	case 268:
		return "Transducers Direct, LLC";
	case 269:
		return "Fujitsu Ten Limited";
	case 270:
		return "Audi AG";
	case 271:
		return "HiSilicon Technologies Co., Ltd.";
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
		return "1OAK Technologies";
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
		return "Kinsa, Inc.";
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
		return "Changzhou Yongse Infotech Co., Ltd";
	case 299:
		return "SportIQ";
	case 300:
		return "TEMEC Instruments B.V.";
	case 301:
		return "Sony Corporation";
	case 302:
		return "ASSA ABLOY";
	case 303:
		return "Clarion Co., Ltd.";
	case 304:
		return "Warehouse Innovations";
	case 305:
		return "Cypress Semiconductor Corporation";
	case 306:
		return "MADS Inc";
	case 307:
		return "Blue Maestro Limited";
	case 308:
		return "Resolution Products, Inc.";
	case 309:
		return "Airewear LLC";
	case 310:
		return "Seed Labs, Inc. (formerly ETC sp. z.o.o.)";
	case 311:
		return "Prestigio Plaza Ltd.";
	case 312:
		return "NTEO Inc.";
	case 313:
		return "Focus Systems Corporation";
	case 314:
		return "Tencent Holdings Limited";
	case 315:
		return "Allegion";
	case 316:
		return "Murata Manufacuring Co., Ltd.";
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
		return "Huizhou Desay SV Automotive CO., LTD.";
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
		return "MADSGlobal NZ Ltd.";
	case 355:
		return "PCH International";
	case 356:
		return "Qingdao Yeelink Information Technology Co., Ltd.";
	case 357:
		return "Milwaukee Tool (formerly Milwaukee Electric Tools)";
	case 358:
		return "MISHIK Pte Ltd";
	case 359:
		return "Bayer HealthCare";
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
		return "F. Hoffmann-La Roche AG";
	case 369:
		return "Amazon Fulfillment Service";
	case 370:
		return "Connovate Technology Private Limited";
	case 371:
		return "Kocomojo, LLC";
	case 372:
		return "Everykey LLC";
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
		return "XTel ApS";
	case 384:
		return "Gigaset Communications GmbH";
	case 385:
		return "Gecko Health Innovations, Inc.";
	case 386:
		return "HOP Ubiquitous";
	case 387:
		return "To Be Assigned";
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
		return "WiSilica Inc";
	case 408:
		return "Vengit Limited";
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
		return "Asandoo GmbH";
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
		return "Nipro Diagnostics, Inc.";
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
		return "August";
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
		return "Sano, Inc";
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
		return "Medtronic, Inc.";
	case 506:
		return "Gozio, Inc.";
	case 507:
		return "Form Lifting, LLC";
	case 508:
		return "Wahoo Fitness, LLC";
	case 509:
		return "Kontakt Micro-Location Sp. z o.o.";
	case 510:
		return "Radio System Corporation";
	case 511:
		return "Freescale Semiconductor, Inc.";
	case 512:
		return "Verifone Systems PTe Ltd. Taiwan Branch";
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
		return "Cisco Systems Inc";
	case 540:
		return "Mobicomm Inc";
	case 541:
		return "Edamic";
	case 542:
		return "Goodnet Ltd";
	case 543:
		return "Luster Leaf Products Inc";
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
		return "Alatech Technology";
	case 571:
		return "Beijing CarePulse Electronic Technology Co, Ltd";
	case 572:
		return "Awarepoint";
	case 573:
		return "ViCentra B.V.";
	case 574:
		return "Raven Industries";
	case 575:
		return "WaveWare Technologies";
	case 576:
		return "Argenox Technologies";
	case 577:
		return "Bragi GmbH";
	case 578:
		return "16Lab Inc";
	case 579:
		return "Masimo Corp";
	case 580:
		return "Iotera Inc.";
	case 581:
		return "Endress+Hauser";
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
		return "NetEase (Hangzhou) Network co.Ltd.";
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
		return "E.G.O. Elektro-Gerätebau GmbH";
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
		return "HUAWEI Technologies Co., Ltd. ( 华为技术有限公司 )";
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
		return "Martians Inc";
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
		return "Nestec S.A.";
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
	case 65535:
		return "internal use";
	default:
		return "not assigned";
	}
}
