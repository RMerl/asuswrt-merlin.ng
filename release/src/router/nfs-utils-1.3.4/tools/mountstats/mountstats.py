#!/usr/bin/env python
# -*- python-mode -*-
"""Parse /proc/self/mountstats and display it in human readable form
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
from operator import itemgetter, add
try:
    import argparse
except ImportError:
    print('%s:  Failed to import argparse - make sure argparse is installed!'
        % sys.argv[0])
    sys.exit(1)

Mountstats_version = '0.3'

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
    'delay',
    'pnfsreads',
    'pnfswrites'
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

XprtUdpCounters = [
    'port',
    'bind_count',
    'rpcsends',
    'rpcreceives',
    'badxids',
    'inflightsends',
    'backlogutil'
]

XprtTcpCounters = [
    'port',
    'bind_count',
    'connect_count',
    'connect_time',
    'idle_time',
    'rpcsends',
    'rpcreceives',
    'badxids',
    'inflightsends',
    'backlogutil'
]

XprtRdmaCounters = [
    'port',
    'bind_count',
    'connect_count',
    'connect_time',
    'idle_time',
    'rpcsends',
    'rpcreceives',
    'badxids',
    'backlogutil',
    'read_chunks',
    'write_chunks',
    'reply_chunks',
    'total_rdma_req',
    'total_rdma_rep',
    'pullup',
    'fixup',
    'hardway',
    'failed_marshal',
    'bad_reply'
]

Nfsv3ops = [
    'NULL',
    'GETATTR',
    'SETATTR',
    'LOOKUP',
    'ACCESS',
    'READLINK',
    'READ',
    'WRITE',
    'CREATE',
    'MKDIR',
    'SYMLINK',
    'MKNOD',
    'REMOVE',
    'RMDIR',
    'RENAME',
    'LINK',
    'READDIR',
    'READDIRPLUS',
    'FSSTAT',
    'FSINFO',
    'PATHCONF',
    'COMMIT'
]

# This list should be kept in-sync with the NFSPROC4_CLNT_* enum in
# include/linux/nfs4.h in the kernel.
Nfsv4ops = [
    'NULL',
    'READ',
    'WRITE',
    'COMMIT',
    'OPEN',
    'OPEN_CONFIRM',
    'OPEN_NOATTR',
    'OPEN_DOWNGRADE',
    'CLOSE',
    'SETATTR',
    'FSINFO',
    'RENEW',
    'SETCLIENTID',
    'SETCLIENTID_CONFIRM',
    'LOCK',
    'LOCKT',
    'LOCKU',
    'ACCESS',
    'GETATTR',
    'LOOKUP',
    'LOOKUP_ROOT',
    'REMOVE',
    'RENAME',
    'LINK',
    'SYMLINK',
    'CREATE',
    'PATHCONF',
    'STATFS',
    'READLINK',
    'READDIR',
    'SERVER_CAPS',
    'DELEGRETURN',
    'GETACL',
    'SETACL',
    'FS_LOCATIONS',
    'RELEASE_LOCKOWNER',
    'SECINFO',
    'FSID_PRESENT',
    'EXCHANGE_ID',
    'CREATE_SESSION',
    'DESTROY_SESSION',
    'SEQUENCE',
    'GET_LEASE_TIME',
    'RECLAIM_COMPLETE',
    'LAYOUTGET',
    'GETDEVICEINFO',
    'LAYOUTCOMMIT',
    'LAYOUTRETURN',
    'SECINFO_NO_NAME',
    'TEST_STATEID',
    'FREE_STATEID',
    'GETDEVICELIST',
    'BIND_CONN_TO_SESSION',
    'DESTROY_CLIENTID',
    'SEEK',
    'ALLOCATE',
    'DEALLOCATE',
    'LAYOUTSTATS',
    'CLONE'
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
            if words[7].find('nfs') != -1 and words[7] != 'nfsd':
                self.__nfs_data['statvers'] = words[8]
        elif 'nfs' in words or 'nfs4' in words:
            self.__nfs_data['export'] = words[0]
            self.__nfs_data['mountpoint'] = words[3]
            self.__nfs_data['fstype'] = words[6]
            if words[6].find('nfs') != -1 and words[6] != 'nfsd':
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
                try:
                    self.__nfs_data[key] = int(words[i])
                except IndexError as err:
                    self.__nfs_data[key] = 0
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
                i = 2
                for key in XprtUdpCounters:
                    self.__rpc_data[key] = int(words[i])
                    i += 1
            elif words[1] == 'tcp':
                i = 2
                for key in XprtTcpCounters:
                    self.__rpc_data[key] = int(words[i])
                    i += 1
            elif words[1] == 'rdma':
                i = 2
                for key in XprtRdmaCounters:
                    self.__rpc_data[key] = int(words[i])
                    i += 1
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

    def nfs_version(self):
        if self.is_nfs_mountpoint():
            prog, vers = self.__rpc_data['programversion'].split('/')
            return int(vers)

    def display_raw_stats(self):
        """Prints out stats in the same format as /proc/self/mountstats
        """
        print('device %s mounted on %s with fstype %s %s' % \
            (self.__nfs_data['export'], self.__nfs_data['mountpoint'], \
            self.__nfs_data['fstype'], self.__nfs_data['statvers']))
        print('\topts:\t%s' % ','.join(self.__nfs_data['mountoptions']))
        print('\tage:\t%d' % self.__nfs_data['age'])
        print('\tcaps:\t%s' % ','.join(self.__nfs_data['servercapabilities']))
        print('\tsec:\tflavor=%d,pseudoflavor=%d' % (self.__nfs_data['flavor'], \
            self.__nfs_data['pseudoflavor']))
        print('\tevents:\t%s' % " ".join([str(self.__nfs_data[key]) for key in NfsEventCounters]))
        print('\tbytes:\t%s' % " ".join([str(self.__nfs_data[key]) for key in NfsByteCounters]))
        print('\tRPC iostats version: %1.1f p/v: %s (nfs)' % (self.__rpc_data['statsvers'], \
            self.__rpc_data['programversion']))
        if self.__rpc_data['protocol'] == 'udp':
            print('\txprt:\tudp %s' % " ".join([str(self.__rpc_data[key]) for key in XprtUdpCounters]))
        elif self.__rpc_data['protocol'] == 'tcp':
            print('\txprt:\ttcp %s' % " ".join([str(self.__rpc_data[key]) for key in XprtTcpCounters]))
        elif self.__rpc_data['protocol'] == 'rdma':
            print('\txprt:\trdma %s' % " ".join([str(self.__rpc_data[key]) for key in XprtRdmaCounters]))
        else:
            raise Exception('Unknown RPC transport protocol %s' % self.__rpc_data['protocol'])
        print('\tper-op statistics')
        prog, vers = self.__rpc_data['programversion'].split('/')
        if vers == '3':
            for op in Nfsv3ops:
                print('\t%12s: %s' % (op, " ".join(str(x) for x in self.__rpc_data[op])))
        elif vers == '4':
            for op in Nfsv4ops:
                print('\t%12s: %s' % (op, " ".join(str(x) for x in self.__rpc_data[op])))
        else:
            print('\tnot implemented for version %d' % vers)
        print()

    def display_stats_header(self):
        print('Stats for %s mounted on %s:' % \
            (self.__nfs_data['export'], self.__nfs_data['mountpoint']))

    def display_nfs_options(self):
        """Pretty-print the NFS options
        """
        self.display_stats_header()

        print('  NFS mount options: %s' % ','.join(self.__nfs_data['mountoptions']))
        print('  NFS server capabilities: %s' % ','.join(self.__nfs_data['servercapabilities']))
        if 'nfsv4flags' in self.__nfs_data:
            print('  NFSv4 capability flags: %s' % ','.join(self.__nfs_data['nfsv4flags']))
        if 'pseudoflavor' in self.__nfs_data:
            print('  NFS security flavor: %d  pseudoflavor: %d' % \
                (self.__nfs_data['flavor'], self.__nfs_data['pseudoflavor']))
        else:
            print('  NFS security flavor: %d' % self.__nfs_data['flavor'])

    def display_nfs_events(self):
        """Pretty-print the NFS event counters
        """
        print()
        print('Cache events:')
        print('  data cache invalidated %d times' % self.__nfs_data['datainvalidates'])
        print('  attribute cache invalidated %d times' % self.__nfs_data['attrinvalidates'])
        print()
        print('VFS calls:')
        print('  VFS requested %d inode revalidations' % self.__nfs_data['inoderevalidates'])
        print('  VFS requested %d dentry revalidations' % self.__nfs_data['dentryrevalidates'])
        print()
        print('  VFS called nfs_readdir() %d times' % self.__nfs_data['vfsreaddir'])
        print('  VFS called nfs_lookup() %d times' % self.__nfs_data['vfslookup'])
        print('  VFS called nfs_permission() %d times' % self.__nfs_data['vfspermission'])
        print('  VFS called nfs_file_open() %d times' % self.__nfs_data['vfsopen'])
        print('  VFS called nfs_file_flush() %d times' % self.__nfs_data['vfsflush'])
        print('  VFS called nfs_lock() %d times' % self.__nfs_data['vfslock'])
        print('  VFS called nfs_fsync() %d times' % self.__nfs_data['vfsfsync'])
        print('  VFS called nfs_file_release() %d times' % self.__nfs_data['vfsrelease'])
        print()
        print('VM calls:')
        print('  VFS called nfs_readpage() %d times' % self.__nfs_data['vfsreadpage'])
        print('  VFS called nfs_readpages() %d times' % self.__nfs_data['vfsreadpages'])
        print('  VFS called nfs_writepage() %d times' % self.__nfs_data['vfswritepage'])
        print('  VFS called nfs_writepages() %d times' % self.__nfs_data['vfswritepages'])
        print()
        print('Generic NFS counters:')
        print('  File size changing operations:')
        print('    truncating SETATTRs: %d  extending WRITEs: %d' % \
            (self.__nfs_data['setattrtrunc'], self.__nfs_data['extendwrite']))
        print('  %d silly renames' % self.__nfs_data['sillyrenames'])
        print('  short reads: %d  short writes: %d' % \
            (self.__nfs_data['shortreads'], self.__nfs_data['shortwrites']))
        print('  NFSERR_DELAYs from server: %d' % self.__nfs_data['delay'])

    def display_nfs_bytes(self):
        """Pretty-print the NFS event counters
        """
        print()
        print('NFS byte counts:')
        print('  applications read %d bytes via read(2)' % self.__nfs_data['normalreadbytes'])
        print('  applications wrote %d bytes via write(2)' % self.__nfs_data['normalwritebytes'])
        print('  applications read %d bytes via O_DIRECT read(2)' % self.__nfs_data['directreadbytes'])
        print('  applications wrote %d bytes via O_DIRECT write(2)' % self.__nfs_data['directwritebytes'])
        print('  client read %d bytes via NFS READ' % self.__nfs_data['serverreadbytes'])
        print('  client wrote %d bytes via NFS WRITE' % self.__nfs_data['serverwritebytes'])

    def display_rpc_generic_stats(self):
        """Pretty-print the generic RPC stats
        """
        sends = self.__rpc_data['rpcsends']

        print()
        print('RPC statistics:')

        print('  %d RPC requests sent, %d RPC replies received (%d XIDs not found)' % \
            (sends, self.__rpc_data['rpcreceives'], self.__rpc_data['badxids']))
        if sends != 0:
            print('  average backlog queue length: %d' % \
                (float(self.__rpc_data['backlogutil']) / sends))

    def display_rpc_op_stats(self):
        """Pretty-print the per-op stats
        """
        sends = self.__rpc_data['rpcsends']

        allstats = []
        for op in self.__rpc_data['ops']:
            allstats.append([op] + self.__rpc_data[op])

        print()
        for stats in sorted(allstats, key=itemgetter(1), reverse=True):
            count = stats[1]
            if count != 0:
                print('%s:' % stats[0])
                print('\t%d ops (%d%%)' % \
                    (count, ((count * 100) / sends)), end=' ')
                retrans = stats[2] - count
                if retrans != 0:
                    print('\t%d retrans (%d%%)' % (retrans, ((retrans * 100) / count)), end=' ')
                    print('\t%d major timeouts' % stats[3])
                else:
                    print('')
                print('\tavg bytes sent per op: %d\tavg bytes received per op: %d' % \
                    (stats[4] / count, stats[5] / count))
                print('\tbacklog wait: %f' % (float(stats[6]) / count), end=' ')
                print('\tRTT: %f' % (float(stats[7]) / count), end=' ')
                print('\ttotal execute time: %f (milliseconds)' % \
                    (float(stats[8]) / count))

    def client_rpc_stats(self):
        """Tally high-level rpc stats for the nfsstat command
        """
        sends = 0
        trans = 0
        authrefrsh = 0
        for op in self.__rpc_data['ops']:
            sends += self.__rpc_data[op][0]
            trans += self.__rpc_data[op][1]
        retrans = trans - sends
        # authrefresh stats don't actually get captured in
        # /proc/self/mountstats, so we fudge it here
        authrefrsh = sends
        return (sends, retrans, authrefrsh)

    def display_nfsstat_stats(self):
        """Pretty-print nfsstat-style stats
        """
        sends = 0
        for op in self.__rpc_data['ops']:
            sends += self.__rpc_data[op][0]
        if sends == 0:
            return
        print()
        vers = self.nfs_version()
        print('Client nfs v%d' % vers)
        info = []
        for op in self.__rpc_data['ops']:
            print('%-13s' % str.lower(op)[:12], end='')
            count = self.__rpc_data[op][0]
            pct = (count * 100) / sends
            info.append((count, pct))
            if (self.__rpc_data['ops'].index(op) + 1) % 6 == 0:
                print()
                for (count, pct) in info:
                    print('%-8u%3u%% ' % (count, pct), end='')
                print()
                info = []
        print()
        if len(info) > 0:
            for (count, pct) in info:
                print('%-8u%3u%% ' % (count, pct), end='')
            print()

    def compare_iostats(self, old_stats):
        """Return the difference between two sets of stats
        """
        if old_stats.__nfs_data['age'] > self.__nfs_data['age']:
            return self

        result = DeviceData()
        protocol = self.__rpc_data['protocol']

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
            result.__rpc_data[op] = list(map(difference, self.__rpc_data[op], old_stats.__rpc_data[op]))

        # update the remaining keys
        if protocol == 'udp':
            for key in XprtUdpCounters:
                result.__rpc_data[key] -= old_stats.__rpc_data[key]
        elif protocol == 'tcp':
            for key in XprtTcpCounters:
                result.__rpc_data[key] -= old_stats.__rpc_data[key]
        elif protocol == 'rdma':
            for key in XprtRdmaCounters:
                result.__rpc_data[key] -= old_stats.__rpc_data[key]
        result.__nfs_data['age'] -= old_stats.__nfs_data['age']
        for key in NfsEventCounters:
            result.__nfs_data[key] -= old_stats.__nfs_data[key]
        for key in NfsByteCounters:
            result.__nfs_data[key] -= old_stats.__nfs_data[key]
        return result

    def setup_accumulator(self, ops):
        """Initialize a DeviceData instance to tally stats for all mountpoints
        with the same major version. This is for the nfsstat command.
        """
        if ops == Nfsv3ops:
            self.__rpc_data['programversion'] = '100003/3'
            self.__nfs_data['fstype'] = 'nfs'
        elif ops == Nfsv4ops:
            self.__rpc_data['programversion'] = '100003/4'
            self.__nfs_data['fstype'] = 'nfs4'
        self.__rpc_data['ops'] = ops
        for op in ops:
            self.__rpc_data[op] = [0 for i in range(8)]

    def accumulate_iostats(self, new_stats):
        """Accumulate counters from all RPC op buckets in new_stats.  This is
        for the nfsstat command.
        """
        for op in new_stats.__rpc_data['ops']:
            try:
                self.__rpc_data[op] = list(map(add, self.__rpc_data[op], new_stats.__rpc_data[op]))
            except KeyError:
                continue

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

    def display_iostats(self, sample_time):
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

        self.__print_rpc_op_stats('READ', sample_time)
        self.__print_rpc_op_stats('WRITE', sample_time)
        sys.stdout.flush()

def parse_stats_file(f):
    """pop the contents of a mountstats file into a dictionary,
    keyed by mount point.  each value object is a list of the
    lines in the mountstats file corresponding to the mount
    point named in the key.
    """
    ms_dict = dict()
    key = ''

    f.seek(0)
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

    return ms_dict

def print_mountstats(stats, nfs_only, rpc_only, raw):
    if nfs_only:
       stats.display_nfs_options()
       stats.display_nfs_events()
       stats.display_nfs_bytes()
    elif rpc_only:
       stats.display_stats_header()
       stats.display_rpc_generic_stats()
       stats.display_rpc_op_stats()
    elif raw:
       stats.display_raw_stats()
    else:
       stats.display_nfs_options()
       stats.display_nfs_bytes()
       stats.display_rpc_generic_stats()
       stats.display_rpc_op_stats()
    print()

def mountstats_command(args):
    """Mountstats command
    """
    mountstats = parse_stats_file(args.infile)
    mountpoints = [os.path.normpath(mp) for mp in args.mountpoints]

    # make certain devices contains only NFS mount points
    if len(mountpoints) > 0:
        check = []
        for device in mountpoints:
            stats = DeviceData()
            try:
                stats.parse_stats(mountstats[device])
                if stats.is_nfs_mountpoint():
                    check += [device]
            except KeyError:
                continue
        mountpoints = check
    else:
        for device, descr in mountstats.items():
            stats = DeviceData()
            stats.parse_stats(descr)
            if stats.is_nfs_mountpoint():
                mountpoints += [device]
    if len(mountpoints) == 0:
        print('No NFS mount points were found')
        return 1

    if args.since:
        old_mountstats = parse_stats_file(args.since)

    for mp in mountpoints:
        stats = DeviceData()
        stats.parse_stats(mountstats[mp])
        if not args.since:
            print_mountstats(stats, args.nfs_only, args.rpc_only, args.raw)
        elif args.since and mp not in old_mountstats:
            print_mountstats(stats, args.nfs_only, args.rpc_only, args.raw)
        else:
            old_stats = DeviceData()
            old_stats.parse_stats(old_mountstats[mp])
            diff_stats = stats.compare_iostats(old_stats)
            print_mountstats(diff_stats, args.nfs_only, args.rpc_only, args.raw)

    args.infile.close()
    if args.since:
        args.since.close()
    return 0

def nfsstat_command(args):
    """nfsstat-like command for NFS mount points
    """
    mountstats = parse_stats_file(args.infile)
    mountpoints = [os.path.normpath(mp) for mp in args.mountpoints]
    v3stats = DeviceData()
    v3stats.setup_accumulator(Nfsv3ops)
    v4stats = DeviceData()
    v4stats.setup_accumulator(Nfsv4ops)

    # ensure stats get printed if neither v3 nor v4 was specified
    if args.show_v3 or args.show_v4:
        show_both = False
    else:
        show_both = True

    # make certain devices contains only NFS mount points
    if len(mountpoints) > 0:
        check = []
        for device in mountpoints:
            stats = DeviceData()
            try:
                stats.parse_stats(mountstats[device])
                if stats.is_nfs_mountpoint():
                    check += [device]
            except KeyError:
                continue
        mountpoints = check
    else:
        for device, descr in mountstats.items():
            stats = DeviceData()
            stats.parse_stats(descr)
            if stats.is_nfs_mountpoint():
                mountpoints += [device]
    if len(mountpoints) == 0:
        print('No NFS mount points were found')
        return 1

    if args.since:
        old_mountstats = parse_stats_file(args.since)

    for mp in mountpoints:
        stats = DeviceData()
        stats.parse_stats(mountstats[mp])
        vers = stats.nfs_version()

        if not args.since:
            acc_stats = stats
        elif args.since and mp not in old_mountstats:
            acc_stats = stats
        else:
            old_stats = DeviceData()
            old_stats.parse_stats(old_mountstats[mp])
            acc_stats = stats.compare_iostats(old_stats)

        if vers == 3 and (show_both or args.show_v3):
           v3stats.accumulate_iostats(acc_stats)
        elif vers == 4 and (show_both or args.show_v4):
           v4stats.accumulate_iostats(acc_stats)

    sends, retrans, authrefrsh = map(add, v3stats.client_rpc_stats(), v4stats.client_rpc_stats())
    print('Client rpc stats:')
    print('calls      retrans    authrefrsh')
    print('%-11u%-11u%-11u' % (sends, retrans, authrefrsh))

    if show_both or args.show_v3:
        v3stats.display_nfsstat_stats()
    if show_both or args.show_v4:
        v4stats.display_nfsstat_stats()

    args.infile.close()
    if args.since:
        args.since.close()
    return 0

def print_iostat_summary(old, new, devices, time):
    for device in devices:
        stats = DeviceData()
        stats.parse_stats(new[device])
        if not old or device not in old:
            stats.display_iostats(time)
        else:
            old_stats = DeviceData()
            old_stats.parse_stats(old[device])
            diff_stats = stats.compare_iostats(old_stats)
            diff_stats.display_iostats(time)

def iostat_command(args):
    """iostat-like command for NFS mount points
    """
    mountstats = parse_stats_file(args.infile)
    devices = [os.path.normpath(mp) for mp in args.mountpoints]

    if args.since:
        old_mountstats = parse_stats_file(args.since)
    else:
        old_mountstats = None

    # make certain devices contains only NFS mount points
    if len(devices) > 0:
        check = []
        for device in devices:
            stats = DeviceData()
            try:
                stats.parse_stats(mountstats[device])
                if stats.is_nfs_mountpoint():
                    check += [device]
            except KeyError:
                continue
        devices = check
    else:
        for device, descr in mountstats.items():
            stats = DeviceData()
            stats.parse_stats(descr)
            if stats.is_nfs_mountpoint():
                devices += [device]
    if len(devices) == 0:
        print('No NFS mount points were found')
        return 1

    sample_time = 0

    if args.interval is None:
        print_iostat_summary(old_mountstats, mountstats, devices, sample_time)
        return

    if args.count is not None:
        count = args.count
        while count != 0:
            print_iostat_summary(old_mountstats, mountstats, devices, sample_time)
            old_mountstats = mountstats
            time.sleep(args.interval)
            sample_time = args.interval
            mountstats = parse_stats_file(args.infile)
            count -= 1
    else: 
        while True:
            print_iostat_summary(old_mountstats, mountstats, devices, sample_time)
            old_mountstats = mountstats
            time.sleep(args.interval)
            sample_time = args.interval
            mountstats = parse_stats_file(args.infile)

    args.infile.close()
    if args.since:
        args.since.close()
    return 0

class ICMAction(argparse.Action):
    """Custom action to deal with interval, count, and mountpoints.
    """
    def __call__(self, parser, namespace, values, option_string=None):
        if namespace.mountpoints is None:
            namespace.mountpoints = []
        if values is None:
            return
        elif (type(values) == type([])):
            for value in values:
                self._handle_one(namespace, value)
        else:
            self._handle_one(namespace, values)

    def _handle_one(self, namespace, value):
        try:
            intval = int(value)
            if namespace.infile.name != '/proc/self/mountstats':
                raise argparse.ArgumentError(self, "not allowed with argument -f/--file or -S/--since")
            self._handle_int(namespace, intval)
        except ValueError:
            namespace.mountpoints.append(value)

    def _handle_int(self, namespace, value):
        if namespace.interval is None:
            namespace.interval = value
        elif namespace.count is None:
            namespace.count = value
        else:
            raise argparse.ArgumentError(self, "too many integer arguments")

def main():
    parser = argparse.ArgumentParser(epilog='For specific sub-command help, '
        'run \'mountstats SUB-COMMAND -h|--help\'')
    subparsers = parser.add_subparsers(help='sub-command help')

    common_parser = argparse.ArgumentParser(add_help=False)
    common_parser.add_argument('-v', '--version', action='version',
        version='mountstats ' + Mountstats_version)
    common_parser.add_argument('-f', '--file', default=open('/proc/self/mountstats', 'r'),
        type=argparse.FileType('r'), dest='infile',
        help='Read stats from %(dest)s instead of /proc/self/mountstats')
    common_parser.add_argument('-S', '--since', type=argparse.FileType('r'),
        metavar='SINCEFILE',
        help='Show difference between current stats and those in SINCEFILE')

    mountstats_parser = subparsers.add_parser('mountstats',
        parents=[common_parser],
        help='Display a combination of per-op RPC statistics, NFS event counts, and NFS byte counts. '
            'This is the default sub-command if no sub-command is given.')
    group = mountstats_parser.add_mutually_exclusive_group()
    group.add_argument('-n', '--nfs', action='store_true', dest='nfs_only',
        help='Display only the NFS statistics')
    group.add_argument('-r', '--rpc', action='store_true', dest='rpc_only',
        help='Display only the RPC statistics')
    group.add_argument('-R', '--raw', action='store_true',
        help='Display only the raw statistics')
    # The mountpoints argument cannot be moved into the common_parser because
    # it will screw up the parsing of the iostat arguments (interval and count)
    mountstats_parser.add_argument('mountpoints', nargs='*', metavar='mountpoint',
        help='Display statistics for this mountpoint. More than one may be specified. '
            'If absent, statistics for all NFS mountpoints will be generated.')
    mountstats_parser.set_defaults(func=mountstats_command)

    nfsstat_parser = subparsers.add_parser('nfsstat',
        parents=[common_parser],
        help='Display nfsstat-like statistics.')
    nfsstat_parser.add_argument('-3', action='store_true', dest='show_v3',
        help='Show NFS version 3 statistics')
    nfsstat_parser.add_argument('-4', action='store_true', dest='show_v4',
        help='Show NFS version 4 statistics')
    # The mountpoints argument cannot be moved into the common_parser because
    # it will screw up the parsing of the iostat arguments (interval and count)
    nfsstat_parser.add_argument('mountpoints', nargs='*', metavar='mountpoint',
        help='Display statistics for this mountpoint. More than one may be specified. '
            'If absent, statistics for all NFS mountpoints will be generated.')
    nfsstat_parser.set_defaults(func=nfsstat_command)

    iostat_parser = subparsers.add_parser('iostat',
        parents=[common_parser],
        help='Display iostat-like statistics.')
    iostat_parser.add_argument('interval', nargs='?', action=ICMAction,
        help='Number of seconds between reports. If absent, only one report will '
            'be generated.')
    iostat_parser.add_argument('count', nargs='?', action=ICMAction,
        help='Number of reports generated at <interval> seconds apart. If absent, '
            'reports will be generated continuously.')
    # The mountpoints argument cannot be moved into the common_parser because
    # it will screw up the parsing of the iostat arguments (interval and count)
    iostat_parser.add_argument('mountpoints', nargs='*', action=ICMAction, metavar='mountpoint',
        help='Display statsistics for this mountpoint. More than one may be specified. '
            'If absent, statistics for all NFS mountpoints will be generated.')
    iostat_parser.set_defaults(func=iostat_command)
 
    args = parser.parse_args()
    return args.func(args)

try:
    if __name__ == '__main__':
        # Run the mounstats sub-command if no sub-command (or the help flag)
        # is given.  If the argparse module ever gets support for optional
        # (default) sub-commands, then this can be changed.
        if len(sys.argv) == 1:
            sys.argv.insert(1, 'mountstats')
        elif sys.argv[1] not in ['-h', '--help', 'mountstats', 'iostat', 'nfsstat']:
            sys.argv.insert(1, 'mountstats')
        res = main()
        sys.stdout.close()
        sys.stderr.close()
        sys.exit(res)
except (KeyboardInterrupt, RuntimeError):
    sys.exit(1)
except IOError:
    pass

