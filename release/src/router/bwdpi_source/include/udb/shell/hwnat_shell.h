#ifndef __HWNAT_SHELL_H__
#define __HWNAT_SHELL_H__

#include "hwnat_core.h"

int udb_shell_okay_to_accel_v4(accel_conn_t *accel_conn);
int udb_shell_okay_to_accel_v6(accel_conn_t *accel_conn);
void udb_shell_register_ecm_cb(ecm_cb_t *ecm_cb_ptr);
void udb_shell_set_cte_timeout(void);

#endif //__HWNAT_SHELL_H__