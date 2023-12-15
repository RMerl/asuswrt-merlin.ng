/*
 * This is the interface to a routine to notify the ahs that it should
 * take some action.
 *
 * Copyright 2019, ASUSTeK Inc.
 * All Rights Reserved.
 *
 */

#ifndef NOTIFY_AHS_H
#define NOTIFY_AHS_H

#include <typedefs.h>


extern int notify_ahs(const char *event_name);
extern int notify_ahs_after_wait(const char *event_name);
extern int notify_ahs_and_wait(const char *event_name);
extern int notify_ahs_and_wait_1min(const char *event_name);
extern int notify_ahs_and_wait_2min(const char *event_name);


#endif /* NOTIFY_AHS_H */
