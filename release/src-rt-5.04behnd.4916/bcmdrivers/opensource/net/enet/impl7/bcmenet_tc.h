#ifndef BCMENET_TC_H
#define BCMENET_TC_H

struct net_device;
struct proc_dir_entry;

void bcmenet_tc_init_procs(struct proc_dir_entry *parent);
extern int bcmenet_setup_tc(struct net_device *dev, enum tc_setup_type type, void *type_data);
extern int bcmenet_tc_trigger(int argc, char **argv);
extern void bcmenet_tc_enet_open(struct net_device *dev);
extern void bcmenet_tc_enet_stop(struct net_device *dev);

#endif /* BCMENET_TC_H */
