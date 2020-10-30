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

#define LINK_UP            1
#define LINK_DOWN          0
#define DSL_LOSS_TIME_TH   18000

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
	if (info.line_state > 1) {
		if (!nvram_match("dsltmp_adslsyncsts", "up")) {
			struct sysinfo si;
			char timestampstr[32] = {0};
			sysinfo(&si);
			snprintf(timestampstr, sizeof(timestampstr), "%lu", si.uptime);
			nvram_set("adsl_timestamp", timestampstr);
			nvram_set("dsltmp_adslsyncsts", "up");
		}
	}
	else if (info.line_state == 1) {
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

	snprintf(timestamp, sizeof(timestamp), "%s", ctime(&secs));
	timestamp[strlen(timestamp)-1] = '\0';

	if(setting_apply)
	{
		last_loss_time = 0;
		nvram_set("dsltmp_syncloss_apply", "0");
		nvram_set("dsltmp_syncloss", "0");
	}
	else
	{
		if(!xdsl_link_status)
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
			, timestamp, xdsl_link_status ? "Up   time" : "Down time"
			, uptime, diff_time);
		if(!xdsl_link_status)
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

static void line_monitor()
{
	static int last_status = 0;
	static unsigned int syncup_counter = 0;
	static unsigned int uCnt = 0;
	struct sysinfo sys_info;
	time_t secs = 0;

	sysinfo(&sys_info);
	time(&secs);
	uCnt++;

	if(nvram_match("dsltmp_adslsyncsts", "up"))
	{
		if(!last_status)
		{	//down->up
			logmessage("DSL", "Link down -> up");

			//link history
			log_sync_time(sys_info.uptime, secs, LINK_UP);

			last_status = LINK_UP;
			nvram_set_int("dsltmp_syncup_cnt", ++syncup_counter);
		}
	}
	else
	{
		if(last_status)
		{	//up->down
			logmessage("DSL", "Link up -> down");

			//link history, includes user modified
			log_sync_time(sys_info.uptime, secs, LINK_DOWN);

			last_status = LINK_DOWN;
		}
	}
}

void dsld_sig(int signum)
{
	switch (signum)
	{
		case SIGTERM:
			g_running = 0;
			break;
		case SIGUSR1:
		case SIGALRM:
			update_xdsl_status();
			line_monitor();
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

	value.it_value.tv_sec = 5;
	value.it_value.tv_usec = 0;
	value.it_interval = value.it_value;
	setitimer(ITIMER_REAL, &value, NULL);
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

	if (get_xdsl_ver(buf, sizeof(buf)) == 0)
		nvram_set("dsllog_drvver", buf);

	update_params();

	while(g_running)
	{
		pause();
	}

	return 0;

}
