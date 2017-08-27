Summary: 	SAR, SADF, MPSTAT, IOSTAT, NFSIOSTAT, CIFSIOSTAT and PIDSTAT for Linux
Name: 		sysstat
Version: 	10.0.3
Release: 	1
License: 	GPL
Group: 		Applications/System
Source0: 	%{name}-%{version}.tar.gz
URL:		http://pagesperso-orange.fr/sebastien.godard/
Packager:	Sebastien Godard <sysstat _at_ orange.fr>
BuildRoot:	%{_tmppath}/%{name}-%{version}-root-%(id -u -n)
Requires:	gettext

%description
The sysstat package contains the sar, sadf, mpstat, iostat, pidstat,
nfsiostat, cifsiostat and sa tools for Linux.
The sar command collects and reports system activity information.
The information collected by sar can be saved in a file in a binary
format for future inspection. The statistics reported by sar concern
I/O transfer rates, paging activity, process-related activities,
interrupts, network activity, memory and swap space utilization, CPU
utilization, kernel activities and TTY statistics, among others. Both
UP and SMP machines are fully supported.
The sadf command may  be used to display data collected by sar in
various formats (CSV, XML, etc.).
The iostat command reports CPU utilization and I/O statistics for disks.
The mpstat command reports global and per-processor statistics.
The pidstat command reports statistics for Linux tasks (processes).
The nfsiostat command reports I/O statistics for network filesystems.
The cifsiostat command reports I/O statistics for CIFS filesystems.

%prep 
%setup 

%build
./configure --prefix=%{_prefix} \
	sa_lib_dir=%{_libdir}/sa \
	--mandir=%{_mandir} \
	DESTDIR=$RPM_BUILD_ROOT
make

%install
rm -rf $RPM_BUILD_ROOT
install -d $RPM_BUILD_ROOT/var/log/sa

make install

mkdir -p $RPM_BUILD_ROOT/etc/rc.d/init.d
install -m 755  sysstat $RPM_BUILD_ROOT/etc/rc.d/init.d/sysstat
mkdir -p $RPM_BUILD_ROOT/etc/sysconfig
install -m 644 sysstat.sysconfig $RPM_BUILD_ROOT/etc/sysconfig/sysstat
install -m 644 sysstat.ioconf $RPM_BUILD_ROOT/etc/sysconfig/sysstat.ioconf
mkdir -p $RPM_BUILD_ROOT/etc/cron.d
install -m 644 cron/sysstat.crond.sample $RPM_BUILD_ROOT/etc/cron.d/sysstat
mkdir -p $RPM_BUILD_ROOT/etc/rc2.d
cd $RPM_BUILD_ROOT/etc/rc2.d && ln -sf ../init.d/sysstat S01sysstat
mkdir -p $RPM_BUILD_ROOT/etc/rc3.d
cd $RPM_BUILD_ROOT/etc/rc3.d && ln -sf ../init.d/sysstat S01sysstat
mkdir -p $RPM_BUILD_ROOT/etc/rc5.d
cd $RPM_BUILD_ROOT/etc/rc5.d && ln -sf ../init.d/sysstat S01sysstat

%clean
rm -rf $RPM_BUILD_ROOT

%files 
%defattr(644,root,root,755)
%doc %{_datadir}/doc/sysstat-%{version}/*
%attr(755,root,root) %{_bindir}/*
%attr(755,root,root) %{_libdir}/sa/*
%attr(644,root,root) %{_mandir}/man*/*
%attr(644,root,root) %{_datadir}/locale/*/LC_MESSAGES/sysstat.mo
%attr(755,root,root) %dir /var/log/sa
%attr(755,root,root) /etc/rc.d/init.d/sysstat
%attr(644,root,root) /etc/sysconfig/sysstat
%attr(644,root,root) /etc/sysconfig/sysstat.ioconf
%attr(755,root,root) /etc/rc2.d/S01sysstat
%attr(755,root,root) /etc/rc3.d/S01sysstat
%attr(755,root,root) /etc/rc5.d/S01sysstat
%config(noreplace) %attr(0644,root,root) /etc/cron.d/sysstat

