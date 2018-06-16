/*
 * Copyright 2018, ASUSTeK Inc.
 * All Rights Reserved.
 *
 */

#include "rc.h"

int ovpn_up_main(int argc, char **argv)
{
	int unit;

	if(argc < 2)
		return -1;

	unit = atoi(argv[1]);

//	ovpn_up_handler(unit);

	update_resolvconf();

	return 0;
}

int ovpn_down_main(int argc, char **argv)
{
	int unit;

	if(argc < 2)
		return -1;

	unit = atoi(argv[1]);

//	ovpn_down_handler(unit);

	update_resolvconf();

	return 0;
}
