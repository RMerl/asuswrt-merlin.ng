#!/usr/bin/python
# -*- python-mode -*-
"""Emulate iostat for NFS mount points using /proc/self/mountstats
"""

from __future__ import print_function

__copyright__ = """
Copyright (C) 2005, Chuck Lever <cel@netapp.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
MA 02110-1301 USA
"""

import sys, os, time
from optparse import OptionParser, OptionGroup

Iostats_version = '0.2'

def difference(x, y):
    """Used for a map() function
    """
    return x - y

NfsEventCounters = [
    'inoderevalidates',
    'dentryrevalidates',
    'datainvalidates',
    'attrinvalidates',
    'vfsopen',
    'vfslookup',
    'vfspermission',
    'vfsupdatepage',
    'vfsreadpage',
    'vfsreadpages',
    'vfswritepage',
    'vfswritepages',
    'vfsreaddir',
    'vfssetattr',
    'vfsflush',
    'vfsfsync',
    'vfslock',
    'vfsrelease',
    'congestionwait',
    'setattrtrunc',
    'extendwrite',
    'sillyrenames',
    'shortreads',
    'shortwrites',
    'delay'
]

NfsByteCounters = [
    'normalreadbytes',
    'normalwritebytes',
    'directreadbytes',
    'directwritebytes',
    'serverreadbytes',
    'serverwritebytes',
    'readpages',
    'writepages'
]

class DeviceData:
    """DeviceData objects provide methods for parsing and displaying
    data for a single mount grabbed from /proc/self/mountstats
    """
    def __init__(self):
        self.__nfs_data = dict()
        self.__rpc_data = dict()
        self.__rpc_data['ops'] = []

    def __parse_nfs_line(self, words):
        if words[0] == 'device':
            self.__nfs_data['export'] = words[1]
            self.__nfs_data['mountpoint'] = words[4]
            self.__nfs_data['fstype'] = words[7]
            if words[7] == 'nfs':
                self.__nfs_data['statvers'] = words[8]
        elif 'nfs' in words or 'nfs4' in words:
            self.__nfs_data['export'] = words[0]
            self.__nfs_data['mountpoint'] = words[3]
            self.__nfs_data['fstype'] = words[6]
            if words[6] == 'nfs':
                self.__nfs_data['statvers'] = words[7]
        elif words[0] == 'age:':
            self.__nfs_data['age'] = int(words[1])
        elif words[0] == 'opts:':
            self.__nfs_data['mountoptions'] = ''.join(words[1:]).split(',')
        elif words[0] == 'caps:':
            self.__nfs_data['servercapabilities'] = ''.join(words[1:]).split(',')
        elif words[0] == 'nfsv4:':
            self.__nfs_data['nfsv4flags'] = ''.join(words[1:]).split(',')
        elif words[0] == 'sec:':
            keys = ''.join(words[1:]).split(',')
            self.__nfs_data['flavor'] = int(keys[0].split('=')[1])
            self.__nfs_data['pseudoflavor'] = 0
            if self.__nfs_data['flavor'] == 6:
                self.__nfs_data['pseudoflavor'] = int(keys[1].split('=')[1])
        elif words[0] == 'events:':
            i = 1
            for key in NfsEventCounters:
                self.__nfs_data[key] = int(words[i])
                i += 1
        elif words[0] == 'bytes:':
            i = 1
            for key in NfsByteCounters:
                self.__nfs_data[key] = int(words[i])
                i += 1

    def __parse_rpc_line(self, words):
        if words[0] == 'RPC':
            self.__rpc_data['statsvers'] = float(words[3])
            self.__rpc_data['programversion'] = words[5]
        elif words[0] == 'xprt:':
            self.__rpc_data['protocol'] = words[1]
            if words[1] == 'udp':
                self.__rpc_data['port'] = int(words[2])
                self.__rpc_data['bind_count'] = int(words[3])
                self.__rpc_data['rpcsends'] = int(words[4])
                self.__rpc_data['rpcreceives'] = int(words[5])
                self.__rpc_data['badxids'] = int(words[6])
                self.__rpc_data['inflightsends'] = int(words[7])
                self.__rpc_data['backlogutil'] = int(words[8])
            elif words[1] == 'tcp':
                self.__rpc_data['port'] = words[2]
                self.__rpc_data['bind_count'] = int(words[3])
                self.__rpc_data['connect_count'] = int(words[4])
                self.__rpc_data['connect_time'] = int(words[5])
                self.__rpc_data['idle_time'] = int(words[6])
                self.__rpc_data['rpcsends'] = int(words[7])
                self.__rpc_data['rpcreceives'] = int(words[8])
                self.__rpc_data['badxids'] = int(words[9])
                self.__rpc_data['inflightsends'] = int(words[10])
                self.__rpc_data['backlogutil'] = int(words[11])
            elif words[1] == 'rdma':
                self.__rpc_data['port'] = words[2]
                self.__rpc_data['bind_count'] = int(words[3])
                self.__rpc_data['connect_count'] = int(words[4])
                self.__rpc_data['connect_time'] = int(words[5])
                self.__rpc_data['idle_time'] = int(words[6])
                self.__rpc_data['rpcsends'] = int(words[7])
                self.__rpc_data['rpcreceives'] = int(words[8])
                self.__rpc_data['badxids'] = int(words[9])
                self.__rpc_data['backlogutil'] = int(words[10])
                self.__rpc_data['read_chunks'] = int(words[11])
                self.__rpc_data['write_chunks'] = int(words[12])
                self.__rpc_data['reply_chunks'] = int(words[13])
                self.__rpc_data['total_rdma_req'] = int(words[14])
                self.__rpc_data['total_rdma_rep'] = int(words[15])
                self.__rpc_data['pullup'] = int(words[16])
                self.__rpc_data['fixup'] = int(words[17])
                self.__rpc_data['hardway'] = int(words[18])
                self.__rpc_data['failed_marshal'] = int(words[19])
                self.__rpc_data['bad_reply'] = int(words[20])
        elif words[0] == 'per-op':
            self.__rpc_data['per-op'] = words
        else:
            op = words[0][:-1]
            self.__rpc_data['ops'] += [op]
            self.__rpc_data[op] = [int(word) for word in words[1:]]

    def parse_stats(self, lines):
        """Turn a list of lines from a mount stat file into a 
        dictionary full of stats, keyed by name
        """
        found = False
        for line in lines:
            words = line.split()
            if len(words) == 0:
                continue
            if (not found and words[0] != 'RPC'):
                self.__parse_nfs_line(words)
                continue

            found = True
            self.__parse_rpc_line(words)

    def is_nfs_mountpoint(self):
        """Return True if this is an NFS or NFSv4 mountpoint,
        otherwise return False
        """
        if self.__nfs_data['fstype'] == 'nfs':
            return True
        elif self.__nfs_data['fstype'] == 'nfs4':
            return True
        return False

    def compare_iostats(self, old_stats):
        """Return the difference between two sets of stats
        """
        result = DeviceData()

        # copy self into result
        for key, value in self.__nfs_data.items():
            result.__nfs_data[key] = value
        for key, value in self.__rpc_data.items():
            result.__rpc_data[key] = value

        # compute the difference of each item in the list
        # note the copy loop above does not copy the lists, just
        # the reference to them.  so we build new lists here
        # for the result object.
        for op in result.__rpc_data['ops']:
            result.__rpc_data[op] = list(map(
                difference, self.__rpc_data[op], old_stats.__rpc_data[op]))

        # update the remaining keys we care about
        result.__rpc_data['rpcsends'] -= old_stats.__rpc_data['rpcsends']
        result.__rpc_data['backlogutil'] -= old_stats.__rpc_data['backlogutil']

        for key in NfsEventCounters:
            result.__nfs_data[key] -= old_stats.__nfs_data[key]
        for key in NfsByteCounters:
            result.__nfs_data[key] -= old_stats.__nfs_data[key]

        return result

    def __print_data_cache_stats(self):
        """Print the data cache hit rate
        """
        nfs_stats = self.__nfs_data
        app_bytes_read = float(nfs_stats['normalreadbytes'])
        if app_bytes_read != 0:
            client_bytes_read = float(nfs_stats['serverreadbytes'] - nfs_stats['directreadbytes'])
            ratio = ((app_bytes_read - client_bytes_read) * 100) / app_bytes_read

            print()
            print('app bytes: %f  client bytes %f' % (app_bytes_read, client_bytes_read))
            print('Data cache hit ratio: %4.2f%%' % ratio)

    def __print_attr_cache_stats(self, sample_time):
        """Print attribute cache efficiency stats
        """
        nfs_stats = self.__nfs_data

        print()
        print('%d VFS opens' % (nfs_stats['vfsopen']))
        print('%d inoderevalidates (forced GETATTRs)' % \
            (nfs_stats['inoderevalidates']))
        print('%d page cache invalidations' % \
            (nfs_stats['datainvalidates']))
        print('%d attribute cache invalidations' % \
            (nfs_stats['attrinvalidates']))

    def __print_dir_cache_stats(self, sample_time):
        """Print directory stats
        """
        nfs_stats = self.__nfs_data
        lookup_ops = self.__rpc_data['LOOKUP'][0]
        readdir_ops = self.__rpc_data['READDIR'][0]
        if 'READDIRPLUS' in self.__rpc_data:
            readdir_ops += self.__rpc_data['READDIRPLUS'][0]

        dentry_revals = nfs_stats['dentryrevalidates']
        opens = nfs_stats['vfsopen']
        lookups = nfs_stats['vfslookup']
        getdents = nfs_stats['vfsreaddir']

        print()
        print('%d open operations (pathname lookups)' % opens)
        print('%d dentry revalidates and %d vfs lookup requests' % \
            (dentry_revals, lookups))
        print('resulted in %d LOOKUPs on the wire' % lookup_ops)
        print('%d vfs getdents calls resulted in %d READDIRs on the wire' % \
            (getdents, readdir_ops))

    def __print_page_stats(self, sample_time):
        """Print page cache stats
        """
        nfs_stats = self.__nfs_data

        vfsreadpage = nfs_stats['vfsreadpage']
        vfsreadpages = nfs_stats['vfsreadpages']
        pages_read = nfs_stats['readpages']
        vfswritepage = nfs_stats['vfswritepage']
        vfswritepages = nfs_stats['vfswritepages']
        pages_written = nfs_stats['writepages']

        print()
        print('%d nfs_readpage() calls read %d pages' % \
            (vfsreadpage, vfsreadpage))
        print('%d nfs_readpages() calls read %d pages' % \
            (vfsreadpages, pages_read - vfsreadpage))
        if vfsreadpages != 0:
            print('(%.1f pages per call)' % \
                (float(pages_read - vfsreadpage) / vfsreadpages))
        else:
            print()

        print()
        print('%d nfs_updatepage() calls' % nfs_stats['vfsupdatepage'])
        print('%d nfs_writepage() calls wrote %d pages' % \
            (vfswritepage, vfswritepage))
        print('%d nfs_writepages() calls wrote %d pages' % \
            (vfswritepages, pages_written - vfswritepage))
        if (vfswritepages) != 0:
            print('(%.1f pages per call)' % \
                (float(pages_written - vfswritepage) / vfswritepages))
        else:
            print()

        congestionwaits = nfs_stats['congestionwait']
        if congestionwaits != 0:
            print()
            print('%d congestion waits' % congestionwaits)

    def __print_rpc_op_stats(self, op, sample_time):
        """Print generic stats for one RPC op
        """
        if op not in self.__rpc_data:
            return

        rpc_stats = self.__rpc_data[op]
        ops = float(rpc_stats[0])
        retrans = float(rpc_stats[1] - rpc_stats[0])
        kilobytes = float(rpc_stats[3] + rpc_stats[4]) / 1024
        rtt = float(rpc_stats[6])
        exe = float(rpc_stats[7])

        # prevent floating point exceptions
        if ops != 0:
            kb_per_op = kilobytes / ops
            retrans_percent = (retrans * 100) / ops
            rtt_per_op = rtt / ops
            exe_per_op = exe / ops
        else:
            kb_per_op = 0.0
            retrans_percent = 0.0
            rtt_per_op = 0.0
            exe_per_op = 0.0

        op += ':'
        print(format(op.lower(), '<16s'), end='')
        print(format('ops/s', '>8s'), end='')
        print(format('kB/s', '>16s'), end='')
        print(format('kB/op', '>16s'), end='')
        print(format('retrans', '>16s'), end='')
        print(format('avg RTT (ms)', '>16s'), end='')
        print(format('avg exe (ms)', '>16s'))

        print(format((ops / sample_time), '>24.3f'), end='')
        print(format((kilobytes / sample_time), '>16.3f'), end='')
        print(format(kb_per_op, '>16.3f'), end='')
        retransmits = '{0:>10.0f} ({1:>3.1f}%)'.format(retrans, retrans_percent).strip()
        print(format(retransmits, '>16'), end='')
        print(format(rtt_per_op, '>16.3f'), end='')
        print(format(exe_per_op, '>16.3f'))

    def ops(self, sample_time):
        sends = float(self.__rpc_data['rpcsends'])
        if sample_time == 0:
            sample_time = float(self.__nfs_data['age'])
        return (sends / sample_time)

    def display_iostats(self, sample_time, which):
        """Display NFS and RPC stats in an iostat-like way
        """
        sends = float(self.__rpc_data['rpcsends'])
        if sample_time == 0:
            sample_time = float(self.__nfs_data['age'])
        #  sample_time could still be zero if the export was just mounted.
        #  Set it to 1 to avoid divide by zero errors in this case since we'll
        #  likely still have relevant mount statistics to show.
        #
        if sample_time == 0:
            sample_time = 1;
        if sends != 0:
            backlog = (float(self.__rpc_data['backlogutil']) / sends) / sample_time
        else:
            backlog = 0.0

        print()
        print('%s mounted on %s:' % \
            (self.__nfs_data['export'], self.__nfs_data['mountpoint']))
        print()

        print(format('ops/s', '>16') + format('rpc bklog', '>16'))
        print(format((sends / sample_time), '>16.3f'), end='')
        print(format(backlog, '>16.3f'))
        print()

        if which == 0:
            self.__print_rpc_op_stats('READ', sample_time)
            self.__print_rpc_op_stats('WRITE', sample_time)
        elif which == 1:
            self.__print_rpc_op_stats('GETATTR', sample_time)
            self.__print_rpc_op_stats('ACCESS', sample_time)
            self.__print_attr_cache_stats(sample_time)
        elif which == 2:
            self.__print_rpc_op_stats('LOOKUP', sample_time)
            self.__print_rpc_op_stats('READDIR', sample_time)
            if 'READDIRPLUS' in self.__rpc_data:
                self.__print_rpc_op_stats('READDIRPLUS', sample_time)
            self.__print_dir_cache_stats(sample_time)
        elif which == 3:
            self.__print_rpc_op_stats('READ', sample_time)
            self.__print_rpc_op_stats('WRITE', sample_time)
            self.__print_page_stats(sample_time)

        sys.stdout.flush()

#
# Functions
#

def parse_stats_file(filename):
    """pop the contents of a mountstats file into a dictionary,
    keyed by mount point.  each value object is a list of the
    lines in the mountstats file corresponding to the mount
    point named in the key.
    """
    ms_dict = dict()
    key = ''

    f = open(filename)
    for line in f.readlines():
        words = line.split()
        if len(words) == 0:
            continue
        if words[0] == 'device':
            key = words[4]
            new = [ line.strip() ]
        elif 'nfs' in words or 'nfs4' in words:
            key = words[3]
            new = [ line.strip() ]
        else:
            new += [ line.strip() ]
        ms_dict[key] = new
    f.close

    return ms_dict

def print_iostat_summary(old, new, devices, time, options):
    stats = {}
    diff_stats = {}
    if old:
        # Trim device list to only include intersection of old and new data,
        # this addresses umounts due to autofs mountpoints
        devicelist = [x for x in old if x in devices]
    else:
        devicelist = devices

    for device in devicelist:
        stats[device] = DeviceData()
        stats[device].parse_stats(new[device])
        if old:
            old_stats = DeviceData()
            old_stats.parse_stats(old[device])
            diff_stats[device] = stats[device].compare_iostats(old_stats)

    if options.sort:
        if old:
            # We now have compared data and can print a comparison
            # ordered by mountpoint ops per second
            devicelist.sort(key=lambda x: diff_stats[x].ops(time), reverse=True)
        else:
            # First iteration, just sort by newly parsed ops/s
            devicelist.sort(key=lambda x: stats[x].ops(time), reverse=True)

    count = 1
    for device in devicelist:
        if old:
            diff_stats[device].display_iostats(time, options.which)
        else:
            stats[device].display_iostats(time, options.which)

        count += 1
        if (count > options.list):
            return


def list_nfs_mounts(givenlist, mountstats):
    """return a list of NFS mounts given a list to validate or
       return a full list if the given list is empty -
       may return an empty list if none found
    """
    list = []
    if len(givenlist) > 0:
        for device in givenlist:
            stats = DeviceData()
            stats.parse_stats(mountstats[device])
            if stats.is_nfs_mountpoint():
                list += [device]
    else:
        for device, descr in mountstats.items():
            stats = DeviceData()
            stats.parse_stats(descr)
            if stats.is_nfs_mountpoint():
                list += [device]
    return list

def iostat_command(name):
    """iostat-like command for NFS mount points
    """
    mountstats = parse_stats_file('/proc/self/mountstats')
    devices = []
    origdevices = []
    interval_seen = False
    count_seen = False

    mydescription= """
Sample iostat-like program to display NFS client per-mount'
statistics.  The <interval> parameter specifies the amount of time in seconds
between each report.  The first report contains statistics for the time since
each file system was mounted.  Each subsequent report contains statistics
collected during the interval since the previous report.  If the <count>
parameter is specified, the value of <count> determines the number of reports
generated at <interval> seconds apart.  If the interval parameter is specified
without the <count> parameter, the command generates reports continuously.
If one or more <mount point> names are specified, statistics for only these
mount points will be displayed.  Otherwise, all NFS mount points on the
client are listed.
"""
    parser = OptionParser(
        usage="usage: %prog [ <interval> [ <count> ] ] [ <options> ] [ <mount point> ]",
        description=mydescription,
        version='version %s' % Iostats_version)
    parser.set_defaults(which=0, sort=False, list=sys.maxsize)

    statgroup = OptionGroup(parser, "Statistics Options",
                            'File I/O is displayed unless one of the following is specified:')
    statgroup.add_option('-a', '--attr',
                            action="store_const",
                            dest="which",
                            const=1,
                            help='displays statistics related to the attribute cache')
    statgroup.add_option('-d', '--dir',
                            action="store_const",
                            dest="which",
                            const=2,
                            help='displays statistics related to directory operations')
    statgroup.add_option('-p', '--page',
                            action="store_const",
                            dest="which",
                            const=3,
                            help='displays statistics related to the page cache')
    parser.add_option_group(statgroup)
    displaygroup = OptionGroup(parser, "Display Options",
                               'Options affecting display format:')
    displaygroup.add_option('-s', '--sort',
                            action="store_true",
                            dest="sort",
                            help="Sort NFS mount points by ops/second")
    displaygroup.add_option('-l','--list',
                            action="store",
                            type="int",
                            dest="list",
                            help="only print stats for first LIST mount points")
    parser.add_option_group(displaygroup)

    (options, args) = parser.parse_args(sys.argv)
    for arg in args:

        if arg == sys.argv[0]:
            continue

        if arg in mountstats:
            origdevices += [arg]
        elif not interval_seen:
            try:
                interval = int(arg)
            except:
                print('Illegal <interval> value %s' % arg)
                return
            if interval > 0:
                interval_seen = True
            else:
                print('Illegal <interval> value %s' % arg)
                return
        elif not count_seen:
            try:
                count = int(arg)
            except:
                print('Ilegal <count> value %s' % arg)
                return
            if count > 0:
                count_seen = True
            else:
                print('Illegal <count> value %s' % arg)
                return

    # make certain devices contains only NFS mount points
    devices = list_nfs_mounts(origdevices, mountstats)
    if len(devices) == 0:
        print('No NFS mount points were found')
        return


    old_mountstats = None
    sample_time = 0.0

    if not interval_seen:
        print_iostat_summary(old_mountstats, mountstats, devices, sample_time, options)
        return

    if count_seen:
        while count != 0:
            print_iostat_summary(old_mountstats, mountstats, devices, sample_time, options)
            old_mountstats = mountstats
            time.sleep(interval)
            sample_time = interval
            mountstats = parse_stats_file('/proc/self/mountstats')
            # automount mountpoints add and drop, if automount is involved
            # we need to recheck the devices list when reparsing
            devices = list_nfs_mounts(origdevices,mountstats)
            if len(devices) == 0:
                print('No NFS mount points were found')
                return
            count -= 1
    else: 
        while True:
            print_iostat_summary(old_mountstats, mountstats, devices, sample_time, options)
            old_mountstats = mountstats
            time.sleep(interval)
            sample_time = interval
            mountstats = parse_stats_file('/proc/self/mountstats')
            # automount mountpoints add and drop, if automount is involved
            # we need to recheck the devices list when reparsing
            devices = list_nfs_mounts(origdevices,mountstats)
            if len(devices) == 0:
                print('No NFS mount points were found')
                return

#
# Main
#
prog = os.path.basename(sys.argv[0])

try:
    iostat_command(prog)
except KeyboardInterrupt:
    print('Caught ^C... exiting')
    sys.exit(1)

sys.exit(0)
