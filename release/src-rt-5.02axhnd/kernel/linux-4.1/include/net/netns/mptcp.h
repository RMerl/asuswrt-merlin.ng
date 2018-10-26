#if defined(CONFIG_BCM_MPTCP) && defined(CONFIG_BCM_KF_MPTCP)
/*
 *	MPTCP implementation - MPTCP namespace
 *
 *	Initial Design & Implementation:
 *	Sébastien Barré <sebastien.barre@uclouvain.be>
 *
 *	Current Maintainer:
 *	Christoph Paasch <christoph.paasch@uclouvain.be>
 *
 *	Additional authors:
 *	Jaakko Korkeaniemi <jaakko.korkeaniemi@aalto.fi>
 *	Gregory Detal <gregory.detal@uclouvain.be>
 *	Fabien Duchêne <fabien.duchene@uclouvain.be>
 *	Andreas Seelinger <Andreas.Seelinger@rwth-aachen.de>
 *	Lavkesh Lahngir <lavkesh51@gmail.com>
 *	Andreas Ripke <ripke@neclab.eu>
 *	Vlad Dogaru <vlad.dogaru@intel.com>
 *	Octavian Purdila <octavian.purdila@intel.com>
 *	John Ronan <jronan@tssg.org>
 *	Catalin Nicutar <catalin.nicutar@gmail.com>
 *	Brandon Heller <brandonh@stanford.edu>
 *
 *
 *	This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 */

#ifndef __NETNS_MPTCP_H__
#define __NETNS_MPTCP_H__

#include <linux/compiler.h>

enum {
	MPTCP_PM_FULLMESH = 0,
	MPTCP_PM_MAX
};

struct mptcp_mib;

struct netns_mptcp {
	DEFINE_SNMP_STAT(struct mptcp_mib, mptcp_statistics);

#ifdef CONFIG_PROC_FS
	struct proc_dir_entry *proc_net_mptcp;
#endif

	void *path_managers[MPTCP_PM_MAX];
};

#endif /* __NETNS_MPTCP_H__ */
#endif
