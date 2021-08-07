#
# Sample spec file for haveged
# Copyright  (c)  2013-2014
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
Name:           haveged
Version:        1.9
Release:        1
License:        GPLv3
Group:          System Environment/Daemons
Summary:        Feed entropy into random pool
URL:            http://www.issihosts.com/haveged/
Source0:        http://www.issihosts.com/haveged/haveged-%{version}.tar.gz
BuildRoot:      %{_builddir}/%{name}-root

%description
The haveged daemon feeds the linux entropy pool with random
numbers generated from hidden processor state.

%prep
%setup -q

%build
./configure
make
make check

%install
[ ${RPM_BUILD_ROOT} != "/" ] && rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install

%clean
[ ${RPM_BUILD_ROOT} != "/" ] && rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/usr/local/sbin/haveged
/usr/local/share/man/man8/haveged.8
/etc/init.d/haveged
