#ifndef __ASTRO_H__
#define __ASTRO_H__

/* in mcf5373l.c */
int rs_serial_init(int port, int baud);
void astro_put_char(char ch);
int astro_is_char(void);
int astro_get_char(void);

/* in fpga.c */
int astro5373l_altera_load(void);
int astro5373l_xilinx_load(void);

/* data structures used for communication (update.c) */
typedef struct card_id {
	char card_type;
	char hardware_version;
	char software_version;
	char software_subversion;	/* " ","a".."z" */
	char fpga_version_altera;
	char fpga_version_xilinx;
} card_id_t;

typedef struct {
	unsigned char mode;
	unsigned char deviation;
	unsigned short freq;
} __attribute__ ((packed)) output_params_t;

typedef struct {
	unsigned short satfreq;
	unsigned char satdatallg;
	unsigned short symbolrate;
	unsigned char viterbirate;
	unsigned char symbolrate_l;
	output_params_t output_params;
	unsigned char reserve;
	unsigned char card_error;
	unsigned short dummy_ts_id;
	unsigned char dummy_pat_ver;
	unsigned char dummy_sdt_ver;
} __attribute__ ((packed)) parameters_t;

#endif /* __ASTRO_H__ */
