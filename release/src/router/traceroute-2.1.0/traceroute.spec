Summary: Traces the route taken by packets over an IPv4/IPv6 network
Name: traceroute
Version: 2.1.0
Release: 1%{?dist}
Group: Applications/Internet
License: GPLv2+
URL:  http://traceroute.sourceforge.net
Source0: http://dl.sourceforge.net/traceroute/traceroute-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)


%description
The traceroute utility displays the route used by IP packets on their
way to a specified network (or Internet) host.  Traceroute displays
the IP number and host name (if possible) of the machines along the
route taken by the packets.  Traceroute is used as a network debugging
tool.  If you're having network connectivity problems, traceroute will
show you where the trouble is coming from along the route.

Install traceroute if you need a tool for diagnosing network connectivity
problems.


%prep
%setup -q


%build
make %{?_smp_mflags} CFLAGS="$RPM_OPT_FLAGS" LDFLAGS=""


%install
rm -rf $RPM_BUILD_ROOT

install -d $RPM_BUILD_ROOT/bin
install -m755 traceroute/traceroute $RPM_BUILD_ROOT/bin
pushd $RPM_BUILD_ROOT/bin
ln -s traceroute traceroute6
popd

install -d $RPM_BUILD_ROOT%{_mandir}/man8
install -p -m644 traceroute/traceroute.8 $RPM_BUILD_ROOT%{_mandir}/man8
ln -s traceroute.8 $RPM_BUILD_ROOT%{_mandir}/man8/traceroute6.8


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%doc COPYING README TODO CREDITS
/bin/*
%{_mandir}/*/*


%changelog
* Tue Oct 20 2006 Dmitry Butskoy <Dmitry@Butskoy.name> - 2.0.2-1
- initial release
