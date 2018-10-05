/*
** cfg_parammgmt.h
**	
**
*/
#ifndef __CFG_PARAMMGNT_H__
#define __CFG_PARAMMGNT_H__

#define SKIP_SERVER	0x01
#define SKIP_CLIENT	0x02
struct skip_param_mapping_s {
	char *name;
	char role;
};

extern int skip_param_mapping(char *name, char role);
#endif	/* __CFG_PARAMMGNT_H__ */

