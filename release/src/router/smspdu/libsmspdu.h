#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <dirent.h>
#include <ctype.h>


#define AT_LOCK_FILE "/tmp/at_cmd_lock"
#define AT_RET_FILE "/tmp/at_ret"
#define STR_DECODE_FILE "/tmp/str_decode.txt"
#define MAX_BUF_SIZE PATH_MAX

#define SMS_ROOT "/tmp/sms"
#define SMS_UNREAD "/tmp/sms/unread"
#define SMS_READ "/tmp/sms/read"
#define SMS_UNSENT "/tmp/sms/unsent"
#define SMS_SENT "/tmp/sms/sent"

#define DEF_AT_OK "OK"

enum {
	SMS_TYPE_UNREAD = 0,
	SMS_TYPE_READ,
	SMS_TYPE_UNSENT,
	SMS_TYPE_SENT,
	SMS_TYPE_ALL
};

enum {
	GSM_7BIT = 0,
	GSM_8BIT = 4,
	GSM_UCS2 = 8
};

enum {
	PULL_IDLE=0,
	PULL_PULL,
	PULL_FORCE_STOP
};

// SMS PDU type (TP-MTI)
#define SM_D_R	0x0    // SMS-DELIVER-REPORT (ACK ofReceive message)
#define SM_D	0x0    // SMS-DELIVER (Receive message)
#define SM_S	0x01   // SMS-SUBMIT (Send message)
#define SM_S_R	0x01   // SMS-SUBMIT-REPORT (ACK of Send message)
#define SM_C	0x10   // SMS-COMMAND
#define SM_C_R	0x10   // SMS-STATUS-REPORT
#define SM_ANY	0x11   // Reserved

// SMS-SUBMIT fields
// octet1                	octet2	octet3	octet4	octet5~octet14	octet15	octet16	octet17	octet18	octet19~last
// MTI RD VPF SRR UDHI RP	MR    	DA-len	DA-type	DA            	PID    	DCS    	VP     	UDL    	UD

// SMS-SUBMIT's first octet
// bit7    bit6    bit5    bit4    bit3    bit2    bit1    bit0
// TP-RP   TP-UDHI TP-SRR  TP-VPF  TP-VPF  TP-RD   TP-MTI  TP-MTI

// SMS-DELIVER fields
// octet1                	octet2	octet3	octet4~octet13	octet14	octet15	octet16~octet22	octet23	octet24
// MTI MMS LP SRI UDHI RP	OA-len	OA-type	OA            	PID    	DCS    	SCTS           	UDL    	UD

// SMS-DELIVER's first octet
// bit7    bit6    bit5    bit4    bit3    bit2    bit1    bit0
// TP-RP   TP-UDHI TP-SRI  TP-LP   TP-LP   TP-MMS  TP-MTI  TP-MTI

// MR
// message reference

// DA-len
// the length of DA

// DA-type
// International number: 0x91
// National number: 0xA1
#define NUM_TYPE_IN 0x91
#define NUM_TYPE_NA 0xA1

// PID
// 00: Default store and forward short message

// DCS
// 00: 7 bit encode
// 04: 8 bit encode
// 08: 16 bit encode(UCS-2)

// VP (when VPF=10)
// 00: 5 minutes
// 90: 12 hours and 30 minutes
// A7: 1 day
// A8: 2 days
// C5: 5 weeks

#define GSM_CURRENCY_SYMBOL_TO_ISO 0xA4

//                  iso   sms
char charset[] = { '@' , 0x00, // COMMERCIAL AT
		   0xA3, 0x01, // POUND SIGN
		   '$' , 0x02, // DOLLAR SIGN
		   0xA5, 0x03, // YEN SIGN
		   0xE8, 0x04, // LATIN SMALL LETTER E WITH GRAVE
		   0xE9, 0x05, // LATIN SMALL LETTER E WITH ACUTE
		   0xF9, 0x06, // LATIN SMALL LETTER U WITH GRAVE
		   0xEC, 0x07, // LATIN SMALL LETTER I WITH GRAVE
		   0xF2, 0x08, // LATIN SMALL LETTER O WITH GRAVE

#ifdef INCOMING_SMALL_C_CEDILLA
		   0xE7, 0x09, // LATIN SMALL LETTER C WITH CEDILLA
#else
		   0xC7, 0x09, // LATIN CAPITAL LETTER C WITH CEDILLA
#endif

		   0x0A, 0x0A, // LF
		   0xD8, 0x0B, // LATIN CAPITAL LETTER O WITH STROKE
		   0xF8, 0x0C, // LATIN SMALL LETTER O WITH STROKE
		   0x0D, 0x0D, // CR
		   0xC5, 0x0E, // LATIN CAPITAL LETTER A WITH RING ABOVE
		   0xE5, 0x0F, // LATIN SMALL LETTER A WITH RING ABOVE

// ISO8859-7, Capital greek characters
//		   0xC4, 0x10,
//		   0x5F, 0x11,
//		   0xD6, 0x12,
//		   0xC3, 0x13,
//		   0xCB, 0x14,
//		   0xD9, 0x15,
//		   0xD0, 0x16,
//		   0xD8, 0x17,
//		   0xD3, 0x18,
//		   0xC8, 0x19,
//		   0xCE, 0x1A,

// ISO8859-1, ISO8859-15
		   0x81, 0x10, // GREEK CAPITAL LETTER DELTA
		   0x5F, 0x11, // LOW LINE
		   0x82, 0x12, // GREEK CAPITAL LETTER PHI
		   0x83, 0x13, // GREEK CAPITAL LETTER GAMMA
		   0x84, 0x14, // GREEK CAPITAL LETTER LAMDA
		   0x85, 0x15, // GREEK CAPITAL LETTER OMEGA
		   0x86, 0x16, // GREEK CAPITAL LETTER PI
		   0x87, 0x17, // GREEK CAPITAL LETTER PSI
		   0x88, 0x18, // GREEK CAPITAL LETTER SIGMA
		   0x89, 0x19, // GREEK CAPITAL LETTER THETA
		   0x8A, 0x1A, // GREEK CAPITAL LETTER XI

		   0x1B, 0x1B, // ESC
		   0xC6, 0x1C, // LATIN CAPITAL LETTER AE
		   0xE6, 0x1D, // LATIN SMALL LETTER AE
		   0xDF, 0x1E, // LATIN SMALL LETTER SHARP S
		   0xC9, 0x1F, // LATIN CAPITAL LETTER E WITH ACUTE
		   ' ' , 0x20, // SPACE
		   '!' , 0x21, // EXCLAMATION MARK
		   0x22, 0x22, // QUOTATION MARK
		   '#' , 0x23, // NUMBER SIGN

                   // GSM character 0x24 is a "currency symbol".
                   // This character is never sent. Incoming character is converted without conversion tables.

		   '%' , 0x25, // PERSENT SIGN
		   '&' , 0x26, // AMPERSAND
		   0x27, 0x27, // APOSTROPHE
		   '(' , 0x28, // LEFT PARENTHESIS
		   ')' , 0x29, // RIGHT PARENTHESIS
		   '*' , 0x2A, // ASTERISK
		   '+' , 0x2B, // PLUS SIGN
		   ',' , 0x2C, // COMMA
		   '-' , 0x2D, // HYPHEN-MINUS
		   '.' , 0x2E, // FULL STOP
		   '/' , 0x2F, // SOLIDUS
		   '0' , 0x30, // DIGIT 0...9
		   '1' , 0x31,
		   '2' , 0x32,
		   '3' , 0x33,
		   '4' , 0x34,
		   '5' , 0x35,
		   '6' , 0x36,
		   '7' , 0x37,
		   '8' , 0x38,
		   '9' , 0x39,
		   ':' , 0x3A, // COLON
		   ';' , 0x3B, // SEMICOLON
		   '<' , 0x3C, // LESS-THAN SIGN
		   '=' , 0x3D, // EQUALS SIGN
		   '>' , 0x3E, // GREATER-THAN SIGN
		   '?' , 0x3F, // QUESTION MARK
		   0xA1, 0x40, // INVERTED EXCLAMATION MARK
		   'A' , 0x41, // LATIN CAPITAL LETTER A...Z
		   'B' , 0x42,
		   'C' , 0x43,
		   'D' , 0x44,
		   'E' , 0x45,
		   'F' , 0x46,
		   'G' , 0x47,
		   'H' , 0x48,
		   'I' , 0x49,
		   'J' , 0x4A,
		   'K' , 0x4B,
		   'L' , 0x4C,
		   'M' , 0x4D,
		   'N' , 0x4E,
		   'O' , 0x4F,
		   'P' , 0x50,
		   'Q' , 0x51,
		   'R' , 0x52,
		   'S' , 0x53,
		   'T' , 0x54,
		   'U' , 0x55,
		   'V' , 0x56,
		   'W' , 0x57,
		   'X' , 0x58,
		   'Y' , 0x59,
		   'Z' , 0x5A,
		   0xC4, 0x5B, // LATIN CAPITAL LETTER A WITH DIAERESIS
		   0xD6, 0x5C, // LATIN CAPITAL LETTER O WITH DIAERESIS
		   0xD1, 0x5D, // LATIN CAPITAL LETTER N WITH TILDE
		   0xDC, 0x5E, // LATIN CAPITAL LETTER U WITH DIAERESIS
		   0xA7, 0x5F, // SECTION SIGN
		   0xBF, 0x60, // INVERTED QUESTION MARK
		   'a' , 0x61, // LATIN SMALL LETTER A...Z
		   'b' , 0x62,
		   'c' , 0x63,
		   'd' , 0x64,
		   'e' , 0x65,
		   'f' , 0x66,
		   'g' , 0x67,
		   'h' , 0x68,
		   'i' , 0x69,
		   'j' , 0x6A,
		   'k' , 0x6B,
		   'l' , 0x6C,
		   'm' , 0x6D,
		   'n' , 0x6E,
		   'o' , 0x6F,
		   'p' , 0x70,
		   'q' , 0x71,
		   'r' , 0x72,
		   's' , 0x73,
		   't' , 0x74,
		   'u' , 0x75,
		   'v' , 0x76,
		   'w' , 0x77,
		   'x' , 0x78,
		   'y' , 0x79,
		   'z' , 0x7A,
		   0xE4, 0x7B, // LATIN SMALL LETTER A WITH DIAERESIS
		   0xF6, 0x7C, // LATIN SMALL LETTER O WITH DIAERESIS
		   0xF1, 0x7D, // LATIN SMALL LETTER N WITH TILDE
		   0xFC, 0x7E, // LATIN SMALL LETTER U WITH DIAERESIS
		   0xE0, 0x7F, // LATIN SMALL LETTER A WITH GRAVE

// Moved to the special char handling:
//		   0x60, 0x27, // GRAVE ACCENT
//                   0xE1, 0x61,  // replacement for accented a
//                   0xED, 0x69,  // replacement for accented i
//                   0xF3, 0x6F,  // replacement for accented o
//                   0xFA, 0x75,  // replacement for accented u

		   0   , 0     // End marker
		 };

// Extended characters. In GSM they are preceeded by 0x1B.

char ext_charset[] = { 0x0C, 0x0A, // <FF>
		       '^' , 0x14, // CIRCUMFLEX ACCENT
		       '{' , 0x28, // LEFT CURLY BRACKET
		       '}' , 0x29, // RIGHT CURLY BRACKET
		       '\\', 0x2F, // REVERSE SOLIDUS
		       '[' , 0x3C, // LEFT SQUARE BRACKET
		       '~' , 0x3D, // TILDE
		       ']' , 0x3E, // RIGHT SQUARE BRACKET
		       0x7C, 0x40, // VERTICAL LINE
		       0xA4, 0x65, // EURO SIGN
		       0   , 0     // End marker
	             };


// This table is used for outgoing (to GSM) conversion only:

char iso_8859_15_chars[] =
{
	0x60, 0x27, // GRAVE ACCENT --> APOSTROPHE
	0xA0, 0x20, // NO-BREAK SPACE --> SPACE
	0xA2, 0x63, // CENT SIGN --> c
	0xA6, 0x53, // LATIN CAPITAL LETTER S WITH CARON --> S
	0xA8, 0x73, // LATIN SMALL LETTER S WITH CARON --> s
	0xA9, 0x43, // COPYRIGHT SIGN --> C
	0xAA, 0x61, // FEMININE ORDINAL INDICATOR --> a
	0xAB, 0x3C, // LEFT-POINTING DOUBLE ANGLE QUOTATION MARK --> <
	0xAC, 0x2D, // NOT SIGN --> -
	0xAD, 0x2D, // SOFT HYPHEN --> -
	0xAE, 0x52, // REGISTERED SIGN --> R
	0xAF, 0x2D, // MACRON --> -
	0xB0, 0x6F, // DEGREE SIGN --> o
	0xB1, 0x2B, // PLUS-MINUS SIGN --> +
	0xB2, 0x32, // SUPERSCRIPT TWO --> 2
	0xB3, 0x33, // SUPERSCRIPT THREE --> 3
	0xB4, 0x5A, // LATIN CAPITAL LETTER Z WITH CARON --> Z
	0xB5, 0x75, // MICRO SIGN --> u
	0xB6, 0x49, // PILCROW SIGN --> I
	0xB7, 0x2E, // MIDDLE DOT --> .
	0xB8, 0x7A, // LATIN SMALL LETTER Z WITH CARON --> z
	0xB9, 0x31, // SUPERSCRIPT ONE --> 1
	0xBA, 0x6F, // MASCULINE ORDINAL INDICATOR --> o
	0xBB, 0x3E, // RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK --> >
	0xBC, 0x4F, // LATIN CAPITAL LIGATURE OE --> O
	0xBD, 0x6F, // LATIN SMALL LIGATURE OE --> o
	0xBE, 0x59, // LATIN CAPITAL LETTER Y WITH DIAERESIS --> Y
	0xC0, 0x41, // LATIN CAPITAL LETTER A WITH GRAVE --> A
	0xC1, 0x41, // LATIN CAPITAL LETTER A WITH ACUTE --> A
	0xC2, 0x41, // LATIN CAPITAL LETTER A WITH CIRCUMFLEX --> A
	0xC3, 0x41, // LATIN CAPITAL LETTER A WITH TILDE --> A
	0xC7, 0x09, // LATIN CAPITAL LETTER C WITH CEDILLA --> 0x09 (LATIN CAPITAL LETTER C WITH CEDILLA)
	0xC8, 0x45, // LATIN CAPITAL LETTER E WITH GRAVE --> E
	0xCA, 0x45, // LATIN CAPITAL LETTER E WITH CIRCUMFLEX --> E
	0xCB, 0x45, // LATIN CAPITAL LETTER E WITH DIAERESIS --> E
	0xCC, 0x49, // LATIN CAPITAL LETTER I WITH GRAVE --> I
	0xCD, 0x49, // LATIN CAPITAL LETTER I WITH ACUTE --> I
	0xCE, 0x49, // LATIN CAPITAL LETTER I WITH CIRCUMFLEX --> I
	0xCF, 0x49, // LATIN CAPITAL LETTER I WITH DIAERESIS --> I
	0xD0, 0x44, // LATIN CAPITAL LETTER ETH --> D
	0xD2, 0x4F, // LATIN CAPITAL LETTER O WITH GRAVE --> O
	0xD3, 0x4F, // LATIN CAPITAL LETTER O WITH ACUTE --> O
	0xD4, 0x4F, // LATIN CAPITAL LETTER O WITH CIRCUMFLEX --> O
	0xD5, 0x4F, // LATIN CAPITAL LETTER O WITH TILDE --> O
	0xD7, 0x78, // MULTIPLICATION SIGN --> x
	0xD9, 0x55, // LATIN CAPITAL LETTER U WITH GRAVE --> U
	0xDA, 0x55, // LATIN CAPITAL LETTER U WITH ACUTE --> U
	0xDB, 0x55, // LATIN CAPITAL LETTER U WITH CIRCUMFLEX --> U
	0xDD, 0x59, // LATIN CAPITAL LETTER Y WITH ACUTE --> Y
	0xDE, 0x62, // LATIN CAPITAL LETTER THORN --> b
	0xE1, 0x61, // LATIN SMALL LETTER A WITH ACUTE --> a
	0xE2, 0x61, // LATIN SMALL LETTER A WITH CIRCUMFLEX --> a
	0xE3, 0x61, // LATIN SMALL LETTER A WITH TILDE --> a
	0xE7, 0x09, // LATIN SMALL LETTER C WITH CEDILLA --> LATIN CAPITAL LETTER C WITH CEDILLA
	0xEA, 0x65, // LATIN SMALL LETTER E WITH CIRCUMFLEX --> e
	0xEB, 0x65, // LATIN SMALL LETTER E WITH DIAERESIS --> e
	0xED, 0x69, // LATIN SMALL LETTER I WITH ACUTE --> i
	0xEE, 0x69, // LATIN SMALL LETTER I WITH CIRCUMFLEX --> i
	0xEF, 0x69, // LATIN SMALL LETTER I WITH DIAERESIS --> i
	0xF0, 0x6F, // LATIN SMALL LETTER ETH --> o
	0xF3, 0x6F, // LATIN SMALL LETTER O WITH ACUTE --> o
	0xF4, 0x6F, // LATIN SMALL LETTER O WITH CIRCUMFLEX --> o
	0xF5, 0x6F, // LATIN SMALL LETTER O WITH TILDE --> o
	0xF7, 0x2F, // DIVISION SIGN --> / (SOLIDUS)
	0xFA, 0x75, // LATIN SMALL LETTER U WITH ACUTE --> u
	0xFB, 0x75, // LATIN SMALL LETTER U WITH CIRCUMFLEX --> u
	0xFD, 0x79, // LATIN SMALL LETTER Y WITH ACUTE --> y
	0xFE, 0x62, // LATIN SMALL LETTER THORN --> b
	0xFF, 0x79, // LATIN SMALL LETTER Y WITH DIAERESIS --> y

	0   , 0
};


// return: 0 succceeded, <0 failed.
extern int send_AT(const char *ttynode, const char *at_cmd, int wait_sec, char *out_file, char *ok_str);
// return: the string of type.
extern char *getSMSTypeStr(int sms_type);

// return: 0 succceeded, <0 failed.
extern int initial_smspdu(const char *ttynode);
// return: n total SMS number, <0 failed.
extern int listSMSIndex(const char *ttynode, char *buf, int buf_max);

// return: n string's len, <0 failed.
extern int composeSMSPDU(const int sms_type, const char *smsc, const char *dest, const char *STR_FILE, unsigned char *buf, int buf_max);
// return: n string's len, <0 failed.
extern int decomposeSMSPDU(const int sms_type, const unsigned char *pdu,
		unsigned char *data, int data_max,
		unsigned char *OA, int OA_max,
		unsigned char *SCTS, int SCTS_max);

#ifdef SAVESMS
// return: n filename's len, <0 failed.
extern int getSMSFileName(const int sms_type, const int sms_index, char *sms_file, int name_max);

// return: n sms_indexs's num, <0 failed.
extern int hasNewSMS(char *sms_index, int buf_max);
#endif

// return: n sms_indexs's num, <0 failed.
extern int getSMSPDUbyType(const char *ttynode, const int sms_type, char *sms_indexs, int indexs_max);
// return: n SMS's type, <0 failed.
extern int getSMSPDUbyIndex(const char *ttynode, const int index, unsigned char *buf, int buf_max);
// return: 0 succceeded, <0 failed.
extern int getallSMSPDU(const char *ttynode);
// return: n sms_index, <0 failed.
extern int saveSMSPDUtoSIM(const char *ttynode, const char *smsc, const char *dest, const char *STR_FILE, unsigned char *buf, int buf_max);
// return: 0 succceeded, <0 failed.
extern int sendSMSPDUfromSIM(const char *ttynode, const int index);
// return: 0 succceeded, <0 failed.
extern int sendSMSPDU(const char *ttynode, const char *smsc, const char *dest, const char *STR_FILE, unsigned char *buf, int buf_max);
// return: 0 succceeded, <0 failed.
extern int delSMSPDUbyIndex(const char *ttynode, const int sms_index);

// return: 0 succceeded, <0 failed.
int initial_phonebook(const char *ttynode);
// return: 0 succceeded, <0 failed.
int savePhonenum(const char *ttynode, const char *phone, const char *name);
// return: 0 succceeded, <0 failed.
int getPhonenumbyIndex(const char *ttynode, const int index, char *phone, int phone_max, char *name, int name_max);
// return: 0 succceeded, <0 failed.
int delPhonenum(const char *ttynode, const int index);
// return: 0 succceeded, <0 failed.
int modPhonenum(const char *ttynode, const int index, const char *phone, const char *name);
// return: n phonenum's num, <0 failed.
extern int listPhonenum(const char *ttynode, char *phone_indexs, int indexs_max, char *phones, int phones_max, char *names, int names_max);
