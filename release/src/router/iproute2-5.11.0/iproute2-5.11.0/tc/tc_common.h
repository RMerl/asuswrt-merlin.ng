/* SPDX-License-Identifier: GPL-2.0 */

#define TCA_BUF_MAX	(64*1024)

extern struct rtnl_handle rth;

int do_qdisc(int argc, char **argv);
int do_class(int argc, char **argv);
int do_filter(int argc, char **argv);
int do_chain(int argc, char **argv);
int do_action(int argc, char **argv);
int do_tcmonitor(int argc, char **argv);
int do_exec(int argc, char **argv);

int print_action(struct nlmsghdr *n, void *arg);
int print_filter(struct nlmsghdr *n, void *arg);
int print_qdisc(struct nlmsghdr *n, void *arg);
int print_class(struct nlmsghdr *n, void *arg);
void print_size_table(FILE *fp, const char *prefix, struct rtattr *rta);

struct tc_estimator;
int parse_estimator(int *p_argc, char ***p_argv, struct tc_estimator *est);

struct tc_sizespec;
int parse_size_table(int *p_argc, char ***p_argv, struct tc_sizespec *s);
int check_size_table_opts(struct tc_sizespec *s);

extern int show_graph;
extern bool use_names;
extern int use_iec;
