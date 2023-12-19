Summary:   Coova-Chilli is a Wireless LAN Access Point Controller
Name:      coova-chilli
Version:   1.3.0
Release:   1
URL:       http://www.coova.org/
Source0:   %{name}-%{version}.tar.gz
License:   GPL
Group:     System Environment/Daemons
BuildRoot: %{_tmppath}/%{name}-root

%if %{!?_without_ssl:1}0
BuildRequires: openssl-devel
%endif

%description 

Coova-Chilli is a fork of the ChilliSpot project - an open source captive
portal or wireless LAN access point controller. It supports web based login
(Universal Access Method, or UAM), standard for public HotSpots, and it
supports Wireless Protected Access (WPA), the standard for secure roamable
networks. Authentication, Authorization and Accounting (AAA) is handled by
your favorite radius server. Read more at http://www.coova.org/.

%prep
%setup 

%build

%configure \
        --disable-static \
        --enable-shared \
	--enable-largelimits \
	--enable-miniportal \
	--enable-chilliredir \
	--enable-chilliproxy \
        --enable-chilliscript \
	--with-poll \
%if %{!?_without_ssl:1}0
	--with-openssl \
	--enable-chilliradsec \
%endif


make

%install
make install DESTDIR=$RPM_BUILD_ROOT

rm -rf $RPM_BUILD_ROOT/usr/include/*
rm -rf $RPM_BUILD_ROOT/usr/lib/*.la
rm -rf $RPM_BUILD_ROOT/usr/lib/*.a

%clean
rm -rf $RPM_BUILD_ROOT
make clean

%post
/sbin/chkconfig --add chilli

%preun
if [ $1 = 0 ]; then
        /sbin/service chilli stop > /dev/null 2>&1
        /sbin/chkconfig --del chilli
fi

%files
%defattr(-,root,root)
%{_sbindir}/*
%{_libdir}/*.so*
%{_libdir}/python/CoovaChilliLib.py
%{_sysconfdir}/init.d/chilli
%doc AUTHORS COPYING ChangeLog INSTALL README doc/dictionary.chillispot doc/hotspotlogin.cgi
%config %{_sysconfdir}/chilli.conf
%config %{_sysconfdir}/chilli/gui-config-default.ini
%config(noreplace) %{_sysconfdir}/chilli/defaults
%dir %{_sysconfdir}/chilli
%dir %{_sysconfdir}/chilli/www
%attr(755,root,root)%{_sysconfdir}/chilli/www/config.sh
%attr(4750,root,chilli)%{_sbindir}/chilli_script
%{_sysconfdir}/chilli/www/*
%{_sysconfdir}/chilli/wwwsh
%{_sysconfdir}/chilli/functions
%{_sysconfdir}/chilli/*.sh
%{_mandir}/man1/*.1*
%{_mandir}/man5/*.5*
%{_mandir}/man8/*.8*

%changelog
* Sat Jan 2 2010 <david@coova.com>
- 1.2.0 release
* Thu Sep 30 2007 <david@coova.com>
- 1.0.8 release 
* Thu Aug 20 2007 <david@coova.com>
- 1.0-coova.7 release
* Thu Jun 7 2007 <david@coova.com>
- 1.0-coova.6 release
* Wed May 16 2007  <david@coova.com>
- 1.0-coova.5 release
* Wed Feb 07 2007  <david@coova.com>
- 1.0-coova.4 release
* Wed Nov 15 2006  <david@coova.com>
- 1.0-coova.3 release
* Thu Mar 25 2004  <support@chillispot.org>
- Initial release.
