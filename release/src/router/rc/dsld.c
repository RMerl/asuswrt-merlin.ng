/*
 * Copyright 2020, ASUSTeK Inc.
 * All Rights Reserved.
 *
 */

#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/sysinfo.h>
#include <time.h>

#include "rc.h"

int g_running = 1;

static XDSL_LOG_RECORD *log_record = NULL;
static unsigned int log_count = 0;
static unsigned int log_head = 0;
static unsigned int log_tail = 0;

#define DSL_LOSS_TIME_TH   18000
#define DSLD_ALARM_SECS    5
#define LOG_RECORD_MAX     6000
#define LOG_RECORD_PERIOD  6 //6 x 5 sec

static void log_sync_time(long uptime, time_t secs, int xdsl_link_status)
{
	FILE *fp = NULL;
	static long last_loss_time = 0;
	static long pre_uptime = 0;
	unsigned long diff_time = 0;
	char timestamp[32] = {0};
	int setting_apply = nvram_get_int("dsltmp_syncloss_apply");

	diff_time = uptime - pre_uptime;
	pre_uptime = uptime;

	snprintf(timestamp, sizeof(timestamp), "%s", asctime(localtime(&secs)));
	timestamp[strlen(timestamp)-1] = '\0';

	if(setting_apply)
	{
		last_loss_time = 0;
		nvram_set("dsltmp_syncloss_apply", "0");
		nvram_set("dsltmp_syncloss", "0");
	}
	else
	{
		if(xdsl_link_status != DSL_LINK_UP)
		{
			if(!last_loss_time)
				last_loss_time = uptime;
			else
			{
				if(uptime - last_loss_time < DSL_LOSS_TIME_TH)
				{
					if(nvram_invmatch("dsltmp_syncloss", "2")) // feedback not submit yet
						nvram_set("dsltmp_syncloss", "1"); //notify to feedback
				}
				else
					last_loss_time = uptime;
			}
		}
	}

	fp = fopen(SYNC_LOG_FILE, "a");
	if(fp)
	{
		fprintf(fp, "[%s] %s: %8ld %8ld"
			, timestamp, (xdsl_link_status == DSL_LINK_UP) ? "Up   time" : "Down time"
			, uptime, diff_time);
		if(xdsl_link_status != DSL_LINK_UP)
		{
			if(setting_apply)
				fputs(" (apply manually)\n", fp);
			else
				fputs("\n", fp);
		}
		else
			fputs("\n", fp);

		fclose(fp);
	}
}

static int dump_log_record(void)
{
	FILE *fp = NULL;
	XDSL_LOG_RECORD *current = NULL;
	XDSL_LOG_RECORD *prev = NULL;
	int count = 0;
	int down_diff = 0, up_diff = 0;
	char timebuf[32] = {0};

	fp = fopen(LOG_RECORD_FILE, "w");
	if(!fp)
		return -1;

	current = log_record + log_head;
	while(count < log_count)
	{
		if(prev)
		{
			down_diff = current->crc_down - prev->crc_down;
			up_diff = current->crc_up - prev->crc_up;
		}
		sprintf(timebuf, "%s", asctime(localtime(&current->cur_time)));
		timebuf[strlen(timebuf)-1] = '\0';	//remove '\n'
		fprintf(fp, "[%s] ", timebuf);
		if(current->link_status == DSL_LINK_UP)
		{
			fprintf(fp, "SNRD:%4.1f(%3d) CRCD:%8u %8d CRCU:%8u %8d\n"
				, current->snr_down
				, current->snrm/512
				, current->crc_down, down_diff
				, current->crc_up, up_diff
			);
		}
		else
		{
			fputs("Link down\n", fp);
		}
		prev = current;
		if (current + 1 == log_record + LOG_RECORD_MAX)
			current = log_record;
		else
			current = current + 1;
		count++;
	}
	fclose(fp);
	return 0;
}

static void add_log_record(time_t secs, XDSL_INFO* info)
{
	XDSL_LOG_RECORD* cur_record = NULL;

	if (log_count)
	{
		log_tail++;
		if (log_tail == LOG_RECORD_MAX)
			log_tail = 0;
		if (log_tail == log_head)
		{
			log_head++;
			if (log_head == LOG_RECORD_MAX)
				log_head = 0;
		}
	}
	cur_record = log_record + log_tail;

	cur_record->link_status = info->line_state;
	cur_record->cur_time = secs;
	cur_record->snr_down = atof(info->snrm_down);
	cur_record->snrm = nvram_get_int("dslx_snrm_offset");
	cur_record->crc_down = info->crc_down;
	cur_record->crc_up = info->crc_up;

	log_count++;
	if (log_count > LOG_RECORD_MAX)
		log_count = LOG_RECORD_MAX;
}

static void line_monitor(XDSL_INFO* info)
{
	static int last_status = 0;
	static unsigned int syncup_counter = 0;
	static unsigned int uCnt = 0;
	struct sysinfo sys_info;
	time_t secs = 0;

	sysinfo(&sys_info);
	time(&secs);
	uCnt++;

	if (info->line_state == DSL_LINK_UP)
	{
		if(last_status != DSL_LINK_UP)
		{	//down->up
			logmessage("DSL", "Link down -> up");

			//snr crc .. history
			add_log_record(secs, info);

			//link history
			log_sync_time(sys_info.uptime, secs, DSL_LINK_UP);

			last_status = DSL_LINK_UP;
			nvram_set_int("dsltmp_syncup_cnt", ++syncup_counter);
		}
		else if(!(uCnt % LOG_RECORD_PERIOD))
		{
			add_log_record(secs, info);
		}
	}
	else
	{ //non DSL_LINK_UP, (DSL_LINK_INIT considered as DSL_LINK_DOWN)
		if(last_status == DSL_LINK_UP)
		{	//up->down
			logmessage("DSL", "Link up -> down");

			//snr crc .. history
			add_log_record(secs, info);

			//link history, includes user modified
			log_sync_time(sys_info.uptime, secs, DSL_LINK_DOWN);

			last_status = DSL_LINK_DOWN;
		}
		else if(!(uCnt % LOG_RECORD_PERIOD))
		{
			add_log_record(secs, info);
		}
	}
}

#if 0
static void dump_hex(uint8_t *data, size_t len, const char* msg)
{
	int i, j;

	if(msg) _dprintf("\n%s", msg);
	_dprintf("\n              0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F   0123456789ABCDEF\r\n");
	for(i=0 ; i < len ; i++)
	{
		if( i!=0 && i%16==0)   //if one line of hex printing is complete...
		{
			_dprintf(" | ");
			for(j=i-16 ; j<i ; j++)
			{
				if(data[j]>=32 && data[j]<=128)
					_dprintf("%c",(unsigned char)data[j]); //if its a number or alphabet

				else _dprintf("."); //otherwise print a dot
			}
			_dprintf("\n");
		}

		if(i%16==0) _dprintf("0x%08X |", i);

		_dprintf(" %02X",(unsigned int)data[i]);

		if( i==len-1)  //print the last spaces
		{
			for(j=0; j<15-i%16; j++) _dprintf("   "); //extra spaces

			_dprintf(" | ");

			for(j=i-i%16 ; j<=i ; j++)
			{
				if(data[j]>=32 && data[j]<=128) _dprintf("%c",(unsigned char)data[j]);
				else _dprintf(".");
			}
			_dprintf("\n");
		}
	}
}
#endif

char *get_vendor_name(uint8_t *vendor_id)
{
	uint32_t vid= 0;
	static char name[8] = {0};

	vid = (vendor_id[2] << 24) | (vendor_id[3] << 16) | (vendor_id[4] << 8) | (vendor_id[5] << 0);

	switch(vid)
	{
		case 0x4244434d: //BDCM
			return "Broadcom";
		case 0x4946544e: //IFTN
			return "Infineon (Lantiq)";
		case 0x494B4E53: //IKNS
			return "Ikanos";
		case 0x43585359: //CXSY
			return "Conexant";
		case 0x50000000:
		case 0x43454e54: //CENT
		case 0x00000000:
			return "Centillium";
		case 0x414c4342: //ALCB
		case 0xffffffff:
			return "Alcatel";
		case 0x4753504e: //GSPN
		case 0x47535056: //GSPV
			return "Globespan";
		case 0x54535443: //TSTC
			return "Texas Instruments";
		case 0x43544e57: //CTNW
			return "Catena";
		case 0x414E4456: //ANDV
			return "Analog Devices";
		case 0x53544D49: //STMI
			return "STMicroelectronics";
		case 0x544D4D42: //TMMB
			return "Thomson Multimedia Broadband";
		default:
			snprintf(name, sizeof(name), "%c%c%c%c", vendor_id[2], vendor_id[3], vendor_id[4], vendor_id[5]);
			return name;
	}
}

static void update_xdsl_status()
{
	XDSL_INFO info;

	memset(&info, 0 , sizeof(info));
	get_xdsl_info(&info);

	//update nvram
	if (info.line_state == DSL_LINK_UP) {
		if (!nvram_match("dsltmp_adslsyncsts", "up")) {
			struct sysinfo si;
			char timestampstr[32] = {0};
			sysinfo(&si);
			snprintf(timestampstr, sizeof(timestampstr), "%lu", si.uptime);
			nvram_set("adsl_timestamp", timestampstr);
			nvram_set("dsltmp_adslsyncsts", "up");
		}
	}
	else if (info.line_state == DSL_LINK_INIT) {
		if (!nvram_match("dsltmp_adslsyncsts", "init")) {
			nvram_set("adsl_timestamp", "");
			nvram_set("dsltmp_adslsyncsts", "init");
		}
	}
	else {
		if (!nvram_match("dsltmp_adslsyncsts", "down")) {
			nvram_set("adsl_timestamp", "");
			nvram_set("dsltmp_adslsyncsts", "down");
		}
	}
	nvram_set("dsllog_opmode", info.mod);
	nvram_set("dsllog_adsltype", info.type);
	nvram_set("dsllog_vdslcurrentprofile", info.profile);
	nvram_set("dsllog_vectoringstate", info.vect);
	nvram_set("dsllog_farendvendorid", get_vendor_name(info.vid));
	nvram_set("dsllog_xdslmode", info.is_vdsl2_gfast?"VDSL":"ADSL");
	nvram_set("dsllog_tcmdown", info.tcm_down?"On":"Off");
	nvram_set("dsllog_tcmup", info.tcm_up?"On":"Off");
	nvram_set("dsllog_tcm", info.tcm_down&&info.tcm_up?"On":"Off");
	nvram_set("dsllog_snrmargindown", info.snrm_down);
	nvram_set("dsllog_snrmarginup", info.snrm_up);
	nvram_set("dsllog_attendown", info.attn_down);
	nvram_set("dsllog_attenup", info.attn_up);
	nvram_set("dsllog_powerdown", info.pwr_down);
	nvram_set("dsllog_powerup", info.pwr_up);
	nvram_set("dsllog_dataratedown", info.rate_down);
	nvram_set("dsllog_datarateup", info.rate_up);
	nvram_set("dsllog_attaindown", info.max_rate_down);
	nvram_set("dsllog_attainup", info.max_rate_up);
	nvram_set_int("dsllog_crcdown", (int)info.crc_down);
	nvram_set_int("dsllog_crcup", (int)info.crc_up);
	nvram_set_int("dsllog_fecdown", (int)info.fec_down);
	nvram_set_int("dsllog_fecup", (int)info.fec_up);
	nvram_set_int("dsllog_esdown", (int)info.es_down);
	nvram_set_int("dsllog_esup", (int)info.es_up);
	nvram_set_int("dsllog_sesdown", (int)info.ses_down);
	nvram_set_int("dsllog_sesup", (int)info.ses_up);
	nvram_set("dsllog_ginpdown", info.ginp_down?"On":"Off");
	nvram_set("dsllog_ginpup", info.ginp_up?"On":"Off");
	nvram_set("dsllog_inpdown", info.inp_down);
	nvram_set("dsllog_inpup", info.inp_up);
	nvram_set("dsllog_inpreindown", info.inp_rein_down);
	nvram_set("dsllog_inpreinup", info.inp_rein_up);
	nvram_set_int("dsllog_interleavedepthdown", (int)info.intlv_depth_down);
	nvram_set_int("dsllog_interleavedepthup", (int)info.intlv_depth_up);
	nvram_set("dsllog_pathmodedown", (info.intlv_depth_down==1)?"Fast Path":"Interleaved");
	nvram_set("dsllog_pathmodeup", (info.intlv_depth_up==1)?"Fast Path":"Interleaved");
	nvram_set("dsllog_pathmodeup", (info.intlv_depth_up==1)?"Fast Path":"Interleaved");
	nvram_set("dsllog_snrmpbds", info.snrm_pb_down);
	nvram_set("dsllog_snrmpbus", info.snrm_pb_up);
	nvram_set("dsllog_latndown", info.latn_pb_down);
	nvram_set("dsllog_latnup", info.latn_pb_up);
	nvram_set("dsllog_satndown", info.satn_pb_down);
	nvram_set("dsllog_satnup", info.satn_pb_up);

	line_monitor(&info);
}

static void update_xdsl_spectrum_data()
{
	float snr[DSL_TONE_MAX] = {0.0};
	uint8_t bits[DSL_TONE_MAX] = {0};
	size_t tone_num = DSL_TONE_MAX;
	FILE *fp;
	int i;

	nvram_set("spectrum_hook_is_running","1");

	// snr
	if (get_xdsl_snr_us(snr, &tone_num) == 0)
	{
		fp = fopen(DSL_SNR_US_FILE, "w");
		if (fp)
		{
			for (i = 0; i < tone_num; i++)
				fprintf(fp, "\"%.4f\"%s", snr[i], (i < tone_num - 1)?",":"");
			fclose(fp);
		}
	}
	if (get_xdsl_snr_ds(snr, &tone_num) == 0)
	{
		fp = fopen(DSL_SNR_DS_FILE, "w");
		if (fp)
		{
			for (i = 0; i < tone_num; i++)
				fprintf(fp, "\"%.4f\"%s", snr[i], (i < tone_num - 1)?",":"");
			fclose(fp);
		}
	}

	// bits
	if (get_xdsl_bits_us(bits, &tone_num) == 0)
	{
		fp = fopen(DSL_BPC_US_FILE, "w");
		if (fp)
		{
			for (i = 0; i < tone_num; i++)
				fprintf(fp, "\"%u\"%s", bits[i], (i < tone_num - 1)?",":"");
			fclose(fp);
		}
	}
	if (get_xdsl_bits_ds(bits, &tone_num) == 0)
	{
		fp = fopen(DSL_BPC_DS_FILE, "w");
		if (fp)
		{
			for (i = 0; i < tone_num; i++)
				fprintf(fp, "\"%u\"%s", bits[i], (i < tone_num - 1)?",":"");
			fclose(fp);
		}
	}

	nvram_set("spectrum_hook_is_running","0");
}

void dsld_sig(int signum)
{
	switch (signum)
	{
		case SIGTERM:
			g_running = 0;
			break;
		case SIGALRM:
			update_xdsl_status();
			break;
		case SIGUSR1:
			dump_log_record();
			break;
		case SIGUSR2:
			update_xdsl_spectrum_data();
			break;
		default:
			//_dprintf("[%s:%d] receive %s\n", __FUNCTION__, __LINE__, strsignal(signum));
			break;
    }
}

static void init_sigaction()
{
	sigset_t sigs_to_catch;

	sigemptyset(&sigs_to_catch);
	sigaddset(&sigs_to_catch, SIGTERM);
	sigaddset(&sigs_to_catch, SIGALRM);
	sigaddset(&sigs_to_catch, SIGUSR1);
	sigaddset(&sigs_to_catch, SIGUSR2);
	sigprocmask(SIG_UNBLOCK, &sigs_to_catch, NULL);

	signal(SIGTERM, dsld_sig);
	signal(SIGALRM, dsld_sig);
	signal(SIGUSR1, dsld_sig);
	signal(SIGUSR2, dsld_sig);
}

static void init_time()
{
	struct itimerval value;

	value.it_value.tv_sec = DSLD_ALARM_SECS;
	value.it_value.tv_usec = 0;
	value.it_interval = value.it_value;
	setitimer(ITIMER_REAL, &value, NULL);
}

static void init_log()
{
	log_record = calloc(LOG_RECORD_MAX, sizeof(XDSL_LOG_RECORD));
}

static void destroy()
{
	if (log_record) free(log_record);
}

static void update_params()
{
	char version[32] = {0};
	char serial_no[32] = {0};
	char model[16] = {0};
	char buildno[8] = {0};
	char extendno[16] = {0};
	char *p = extendno;

	//set_vendor_id("\xFE""\x00""ASUS""\x00""\x00");

	// version
	nvram_safe_get_r("dsllog_drvver", version, sizeof(version));
	set_version(version);

	// serial number
	nvram_safe_get_r("serial_no", serial_no, sizeof(serial_no));
	strlcat(serial_no, " ", sizeof(serial_no));

	strlcpy(model, get_productid(), sizeof(model));
	trim_char(model, '-');
	strlcat(serial_no, model, sizeof(serial_no));
	strlcat(serial_no, " ", sizeof(serial_no));

	nvram_safe_get_r("buildno", buildno, sizeof(buildno));
	strlcat(serial_no, buildno, sizeof(serial_no));
	nvram_safe_get_r("extendno", extendno, sizeof(extendno));
	if ((p = strtok(extendno, "-")))
		strlcat(serial_no, extendno, sizeof(serial_no));
	//_dprintf("\nserial_no: %s\n", serial_no);
	set_serial_no(serial_no);
}

int dsld_main(int argc, char **argv)
{
	char buf[64] = {0};
	init_sigaction();

	init_time();
	init_log();

	if (get_xdsl_ver(buf, sizeof(buf)) == 0)
		nvram_set("dsllog_drvver", buf);

	update_params();

	while(g_running)
	{
		pause();
	}

	destroy();

	return 0;

}
