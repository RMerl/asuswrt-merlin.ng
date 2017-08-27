Name:         freeradius-server
Version:      3.0.0
Release:      0
License:      GPLv2 ; LGPLv2.1
Group:        Productivity/Networking/Radius/Servers
Provides:     radiusd
Provides:     freeradius = %{version}
Obsoletes:    freeradius < %{version}
Conflicts:    radiusd-livingston radiusd-cistron icradius
Url:          http://www.freeradius.org/
Summary:      Very Highly Configurable Radius Server
Source:       ftp://ftp.freeradius.org/pub/freeradius/%{name}-%{version}.tar.bz2
Source90:     %{name}-rpmlintrc
Source104:    %{name}-tmpfiles.conf
PreReq:       %{_sbindir}/useradd %{_sbindir}/groupadd
PreReq:       perl
PreReq:       %insserv_prereq %fillup_prereq
BuildRoot:    %{_tmppath}/%{name}-%{version}-build
%define _oracle_support	0
Requires:      %{name}-libs = %{version}
Requires:      python
Recommends:    logrotate
BuildRequires: db-devel
BuildRequires: gcc-c++
BuildRequires: gdbm-devel
BuildRequires: glibc-devel
BuildRequires: libtalloc-devel
BuildRequires: openldap2-devel
BuildRequires: openssl-devel
BuildRequires: pam-devel
BuildRequires: perl
BuildRequires: postgresql-devel
BuildRequires: python-devel
BuildRequires: sed
BuildRequires: unixODBC-devel


%if 0%{?suse_version} > 910
BuildRequires: krb5-devel
%endif
%if 0%{?suse_version} > 930
BuildRequires: libcom_err
%endif
%if 0%{?suse_version} > 1000
BuildRequires: libapr1-devel
%endif
%if 0%{?suse_version} > 1020
BuildRequires: libmysqlclient-devel
%endif
%if 0%{?suse_version} > 1100
BuildRequires: libpcap-devel
BuildRequires: sqlite3-devel
%endif


%description
The FreeRADIUS server has a number of features found in other servers,
and additional features not found in any other server. Rather than
doing a feature by feature comparison, we will simply list the features
of the server, and let you decide if they satisfy your needs.

Support for RFC and VSA Attributes Additional server configuration
attributes Selecting a particular configuration Authentication methods
Accounting methods

Authors:
--------
See http://wiki.freeradius.org/project/Acknowledgements

%package libs
License:      GPLv2 ; LGPLv2.1
Group:        Productivity/Networking/Radius/Servers
Summary:      FreeRADIUS shared library

%description libs
The FreeRADIUS shared library

Authors:
--------
See http://wiki.freeradius.org/project/Acknowledgements

%package utils
License:      GPLv2 ; LGPLv2.1
Group:        Productivity/Networking/Radius/Clients
Summary:      FreeRADIUS Clients
Requires:     %{name}-libs = %{version}

%description utils
The FreeRADIUS server has a number of features found in other servers
and additional features not found in any other server. Rather than
doing a feature by feature comparison, we will simply list the features
of the server, and let you decide if they satisfy your needs.

Support for RFC and VSA Attributes Additional server configuration
attributes Selecting a particular configuration Authentication methods

%package devel
License:      GPLv2 ; LGPLv2.1
Group:        Development/Libraries/C and C++
Summary:      FreeRADIUS Development Files (static libs)
Requires:     %{name}-libs = %{version}

%description devel
These are the static libraries for the FreeRADIUS package.

Authors:
--------
See http://wiki.freeradius.org/project/Acknowledgements

%package doc
License:        GPLv2 ; LGPLv2.1
Group:          Productivity/Networking/Radius/Servers
Summary:        FreeRADIUS Documentation
Requires:       %{name}

%description doc
This package contains FreeRADIUS Documentation

Authors:
--------
See http://wiki.freeradius.org/project/Acknowledgements

%prep
%setup -q

%build
# This package failed when testing with -Wl,-as-needed being default.
# So we disable it here, if you want to retest, just delete this comment and the line below.
export SUSE_ASNEEDED=0
export CFLAGS="$RPM_OPT_FLAGS"
%ifarch x86_64 ppc ppc64 s390 s390x
export CFLAGS="$CFLAGS -fPIC -DPIC"
%endif
export LDFLAGS="-pie"
%configure \
		--libdir=%{_libdir}/freeradius \
		--disable-developer \
		--with-experimental-modules \
		--with-udpfromto \
%if 0%{?suse_version} <= 920 
		--without-rlm_sql_mysql \
		--without-rlm_krb5 \
%endif
%if %{_oracle_support} == 1
		--with-rlm_sql_oracle \
		--with-oracle-lib-dir=%{_libdir}/oracle/10.1.0.3/client/lib/
%else
		--without-rlm_sql_oracle
%endif
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT%{_localstatedir}/lib/radiusd
make install R=$RPM_BUILD_ROOT
# modify default configuration
RADDB=$RPM_BUILD_ROOT%{_sysconfdir}/raddb
perl -i -pe 's/^#user =.*$/user = radiusd/'   $RADDB/radiusd.conf
perl -i -pe 's/^#group =.*$/group = radiusd/' $RADDB/radiusd.conf
/sbin/ldconfig -n $RPM_BUILD_ROOT%{_libdir}/freeradius
# logs
touch $RPM_BUILD_ROOT%{_localstatedir}/log/radius/radutmp
touch $RPM_BUILD_ROOT%{_localstatedir}/log/radius/radius.log
# SuSE
install -d     $RPM_BUILD_ROOT%{_sysconfdir}/pam.d
install -d     $RPM_BUILD_ROOT%{_sysconfdir}/logrotate.d
install -m 644 suse/radiusd-pam $RPM_BUILD_ROOT%{_sysconfdir}/pam.d/radiusd
install -m 644 suse/radiusd-logrotate $RPM_BUILD_ROOT%{_sysconfdir}/logrotate.d/freeradius-server
install -d -m 755 $RPM_BUILD_ROOT%{_sysconfdir}/init.d
install    -m 744 suse/rcradiusd $RPM_BUILD_ROOT%{_sysconfdir}/init.d/freeradius
ln -sf ../..%{_sysconfdir}/init.d/freeradius $RPM_BUILD_ROOT%{_sbindir}/rcfreeradius
install -d %{buildroot}%{_sysconfdir}/tmpfiles.d
install -m 0644 %{SOURCE104} %{buildroot}%{_sysconfdir}/tmpfiles.d/radiusd.conf
# remove unneeded stuff
rm -rf doc/00-OLD
rm -f $RPM_BUILD_ROOT%{_sbindir}/rc.radiusd
rm -rf $RPM_BUILD_ROOT/usr/share/doc/freeradius*
rm -rf $RPM_BUILD_ROOT/%{_libdir}/freeradius/*.*a

%pre
%{_sbindir}/groupadd -r radiusd 2> /dev/null || :
%{_sbindir}/useradd -r -g radiusd -s /bin/false -c "Radius daemon" -d \
                  %{_localstatedir}/lib/radiusd radiusd 2> /dev/null || :

%post
%ifarch x86_64
# Modify old installs to look for /usr/lib64/freeradius
/usr/bin/perl -i -pe "s:/usr/lib/freeradius:/usr/lib64/freeradius:" /etc/raddb/radiusd.conf
%endif

# Generate default certificates
/etc/raddb/certs/bootstrap

%{fillup_and_insserv freeradius}
%if 0%{?suse_version} > 820

%preun
%stop_on_removal freeradius
%endif

%postun
%if 0%{?suse_version} > 820
%restart_on_update freeradius
%endif
%{insserv_cleanup}

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
# doc
%doc suse/README.SuSE
%doc doc/* LICENSE COPYRIGHT CREDITS README.rst
# SuSE
%{_sysconfdir}/init.d/freeradius
%config %{_sysconfdir}/pam.d/radiusd
%config %{_sysconfdir}/logrotate.d/freeradius-server
%config %{_sysconfdir}/tmpfiles.d/radiusd.conf
%dir %attr(755,radiusd,radiusd) %{_localstatedir}/lib/radiusd
# configs
%defattr(-,root,radiusd)
%dir %attr(750,root,radiusd) %{_sysconfdir}/raddb
%config(noreplace) %{_sysconfdir}/raddb/*
%attr(700,radiusd,radiusd) %dir %{_localstatedir}/run/radiusd/
# binaries
%defattr(-,root,root)
%{_sbindir}/*
# man-pages
%doc %{_mandir}/man1/*
%doc %{_mandir}/man5/*
%doc %{_mandir}/man8/*
# dictionaries
%attr(755,root,root) %dir /usr/share/freeradius
/usr/share/freeradius/*
# logs
%attr(700,radiusd,radiusd) %dir %{_localstatedir}/log/radius/
%attr(700,radiusd,radiusd) %dir %{_localstatedir}/log/radius/radacct/
%attr(644,radiusd,radiusd) %{_localstatedir}/log/radius/radutmp
%config(noreplace) %attr(600,radiusd,radiusd) %{_localstatedir}/log/radius/radius.log
# RADIUS Loadable Modules
%attr(755,root,root) %dir %{_libdir}/freeradius
%attr(755,root,root) %{_libdir}/freeradius/rlm_*.so*

%files utils
/usr/bin/*

%files libs
# RADIUS shared libs
%attr(755,root,root) %dir %{_libdir}/freeradius
%attr(755,root,root) %{_libdir}/freeradius/*.so*

%files devel
%defattr(-,root,root)
%attr(644,root,root) /usr/include/freeradius/*.h
