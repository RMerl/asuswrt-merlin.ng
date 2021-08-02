#
# Sample spec file for haveged and haveged-devel
# Copyright  (c)  2013-2014
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
%define have_systemd 0

Name:           haveged
Version:        1.9
Release:        0
License:        GPLv3
Group:          System Environment/Daemons
Summary:        Feed entropy into random pool
URL:            http://www.issihosts.com/haveged/
Source0:        http://www.issihosts.com/haveged/haveged-%{version}.tar.gz
BuildRoot:      %{_builddir}/%{name}-root
%if 0%{?have_systemd}
BuildRequires:  systemd
%endif

%description
The haveged daemon feeds the linux entropy pool with random
numbers generated from hidden processor state.

%package devel
Summary:    haveged development files
Group:      Development/Libraries

%description devel
Headers and shared object symbolic links for the haveged library

This package contains the haveged implementation of the HAVEGE
algorithm and supporting features.

%prep
%setup -q

%build
%configure \
  --enable-daemon\
  --enable--init=sysv.redhat
make

%check
make check

%install
%makeinstall
%{__install} -D -m0755 %{_sysconfdir}/init.d/%{name}
%if 0%{?have_systemd}
%{__install} -D -m0644 %{S:2} %{buildroot}%{_unitdir}/%{name}.service
%endif
%{__rm} -f %{buildroot}%{_libdir}/libhavege.*a

%clean
%{?buildroot:%__rm -rf "%{buildroot}"}

%files
%defattr(-, root, root, -)
%doc COPYING
%{_mandir}/man8/haveged.8*
%{_sbindir}/haveged
%if 0%{?have_systemd}
%{_unitdir}/haveged.service
%endif

%files devel
%doc COPYING
%defattr(-, root, root, -)
%{_mandir}/man3/libhavege.3*
%dir %{_includedir}/%{name}
%{_includedir}/%{name}/havege*.h
%doc contrib/build/havege_sample.c
%{_libdir}/*.so*

