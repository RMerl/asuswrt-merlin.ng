
%define majorver 1.7
%define minorver 3.2

Summary: socat - multipurpose relay
Name: socat
Version: %{majorver}.%{minorver}
Release: 1
License: GPL
Group: Applications/Communications
Source0: http://www.dest-unreach.org/socat/download/socat-%{version}.tar.bz2
Requires: readline
Requires: openssl
BuildRoot: /var/tmp/%{name}-buildroot

%description
socat is a relay for bidirectional data transfer between two independent data
channels. Each of these data channels may be a file, pipe, device (terminal or
modem etc.), socket (UNIX, IP4, IP6 - raw, UDP, TCP), a file descriptor (stdin
etc.), a program, or an arbitrary combination of two of these.

%prep
%setup -n %{name}-%{version}

%build
# the CPPFLAGS setting is required for RedHat Linux
if [ -d /usr/kerberos/include ]; then
    CPPFLAGS="-I/usr/kerberos/include" ./configure --prefix=%{_prefix} --mandir=%{_mandir}
else
    ./configure --prefix=%{_prefix} --mandir=%{_mandir}
fi
make

%install
rm -rf $RPM_BUILD_ROOT

mkdir -p $RPM_BUILD_ROOT%{_bindir}
mkdir -p $RPM_BUILD_ROOT%{_mandir}/man1

make install DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc README CHANGES EXAMPLES SECURITY doc/xio.help doc/socat.html FAQ BUGREPORTS
%doc COPYING COPYING.OpenSSL FILES PORTING DEVELOPMENT
%{_bindir}/socat
%{_bindir}/procan
%{_bindir}/filan
%{_mandir}/man1/socat.1*
