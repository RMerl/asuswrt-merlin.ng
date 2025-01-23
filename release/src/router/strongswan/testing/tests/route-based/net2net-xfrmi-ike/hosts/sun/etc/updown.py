#!/usr/bin/env python3

import sys
import vici
import logging
from logging.handlers import SysLogHandler
import subprocess

# the hard limit (second number) is the value used by python-daemon when closing
# potentially open file descriptors while daemonizing or even triggered by the
# import.  since the default limit is 524288 on newer systems, this can take
# quite a while, and due to how this range of FDs is handled internally (as set)
# it can even trigger the OOM killer
import resource
resource.setrlimit(resource.RLIMIT_NOFILE, (256, 256))  # noqa
import daemon


logger = logging.getLogger('updownLogger')


def setup_logger():
    handler = SysLogHandler(address='/dev/log', facility=SysLogHandler.LOG_DAEMON)
    handler.setFormatter(logging.Formatter('charon-updown: %(message)s'))
    logger.addHandler(handler)
    logger.setLevel(logging.INFO)


def handle_interfaces(ike_sa, up):
    if_id_in = int(ike_sa['if-id-in'], 16)
    if_id_out = int(ike_sa['if-id-out'], 16)
    ifname_in = "xfrm-{}-in".format(if_id_in)
    ifname_out = "xfrm-{}-out".format(if_id_out)

    if up:
        logger.info("add XFRM interfaces %s and %s", ifname_in, ifname_out)
        subprocess.call(["ip", "link", "add", ifname_out, "type", "xfrm",
                         "if_id", str(if_id_out), "dev", "eth0"])
        subprocess.call(["ip", "link", "add", ifname_in, "type", "xfrm",
                         "if_id", str(if_id_in), "dev", "eth0"])
        subprocess.call(["ip", "link", "set", ifname_out, "up"])
        subprocess.call(["ip", "link", "set", ifname_in, "up"])
        subprocess.call(["iptables", "-A", "FORWARD", "-o", ifname_out,
                         "-j", "ACCEPT"])
        subprocess.call(["iptables", "-A", "FORWARD", "-i", ifname_in,
                         "-j", "ACCEPT"])

    else:
        logger.info("delete XFRM interfaces %s and %s", ifname_in, ifname_out)
        subprocess.call(["iptables", "-D", "FORWARD", "-o", ifname_out,
                         "-j", "ACCEPT"])
        subprocess.call(["iptables", "-D", "FORWARD", "-i", ifname_in,
                         "-j", "ACCEPT"])
        subprocess.call(["ip", "link", "del", ifname_out])
        subprocess.call(["ip", "link", "del", ifname_in])


def install_routes(ike_sa):
    if_id_out = int(ike_sa['if-id-out'], 16)
    ifname_out = "xfrm-{}-out".format(if_id_out)
    child_sa = next(iter(ike_sa['child-sas'].values()))

    for ts in child_sa['remote-ts']:
        ts = ts.decode('UTF-8')
        logger.info("add route to %s via %s", ts, ifname_out)
        subprocess.call(["ip", "route", "add", ts, "dev", ifname_out])


# daemonize and run parallel to the IKE daemon
with daemon.DaemonContext():
    setup_logger()
    logger.debug("starting Python updown listener")
    try:
        session = vici.Session()
        ver = {k: v.decode("UTF-8") for k, v in session.version().items()}
        logger.info("connected to {daemon} {version} ({sysname}, {release}, "
                    "{machine})".format(**ver))
    except BaseException:
        logger.error("failed to get status via vici")
        sys.exit(1)

    try:
        for label, event in session.listen(["ike-updown", "child-updown"]):
            logger.debug("received event: %s %s", label, repr(event))

            name = next((x for x in iter(event) if x != "up"))
            up = event.get("up", "") == b"yes"
            ike_sa = event[name]

            if label == b"ike-updown":
                handle_interfaces(ike_sa, up)

            elif label == b"child-updown" and up:
                install_routes(ike_sa)

    except IOError:
        logger.error("daemon disconnected")
    except BaseException as e:
        logger.error("exception while listening for events " +
                     repr(e))
