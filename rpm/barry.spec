## pass in '--with gui' to build the GUI tools
# ex.: rpmbuild -ba barry.spec --with gui
#
## pass in '--with opensync' to build the opensync plugin
# ex.: rpmbuild -ba barry.spec --with opensync

## bcond_with() is a rpm-4.4 addition, we'll backport a private
## copy in case we're building on CentOS/Red Hat Enterprise 4 and lower
%define barry_with()  %{expand:%%{?_with_%{1}:%%global with_%{1} 1}}
#
%barry_with gui
%barry_with opensync

Summary: BlackBerry(tm) Desktop for Linux
Name: barry
Version: 0.6
Release: 1
Group: Applications/Productivity
License: GPL
Source: %{name}-%{version}.tar.gz
URL: http://www.netdirect.ca/downloads/barry
Vendor: Net Direct Inc.
BuildRoot: %{_tmppath}/%{name}-%{release}-%{version}-root
BuildRequires: libusb-devel, gcc-c++, pkgconfig, boost-devel, openssl-devel
Patch0: barry-0.6-udev.patch

%define barryroot %{_builddir}/%{name}-%{version}

%description
Barry is a desktop toolset for managing your BlackBerry(tm) device. (BlackBerry
is a registered trademark of Research in Motion Limited.)

This package contains the license agreement, README file and all other
assorted documentation common to all sub-packages. You most likely want to
also install libbarry, barry-util and barry-backup.

%package -n libbarry
Summary: BlackBerry(tm) Desktop for Linux - libbarry libraries
Group: Development/Libraries
Requires: barry libusb openssl boost

%description -n libbarry
Barry is a desktop toolset for managing your BlackBerry(tm) device. (BlackBerry
is a registered trademark of Research in Motion Limited.)

This package contains the library files for Barry, libbarry.

%package -n libbarry-devel
Summary: BlackBerry(tm) Desktop for Linux - libbarry libraries
Group: Development/Libraries
Requires: barry libbarry libusb-devel openssl-devel boost-devel

%description -n libbarry-devel
Barry is a desktop toolset for managing your BlackBerry(tm) device. (BlackBerry
is a registered trademark of Research in Motion Limited.)

This package contains the development library files for Barry, libbarry.

%package util
Summary: BlackBerry(tm) Desktop for Linux - bcharge, btool, breset and others
Group: Applications/Productivity
Requires: barry libbarry
Conflicts: barry-bcharge

%description util
Barry is a desktop toolset for managing your BlackBerry(tm) device. (BlackBerry
is a registered trademark of Research in Motion Limited.)

This package contains the commandline tools bcharge, btool, breset and others
which will enable you to charge your device with a proper 500mA and be able
to access the data on the device in many ways.

%if %{with gui}
%package gui
Summary: BlackBerry(tm) Desktop for Linux - bcharge, btool, breset and others
Group: Applications/Productivity
Requires: barry libbarry gtkmm24 libglademm24 libtar
BuildRequires: gtkmm24-devel libglademm24-devel libtar-devel

%description gui
Barry is a desktop toolset for managing your BlackBerry(tm) device. (BlackBerry
is a registered trademark of Research in Motion Limited.)

This package contains the GUI applications built on top of libbarry.
%endif

%if %{with opensync}
%package opensync
Summary: BlackBerry(tm) Desktop for Linux - opensync plugin
Group: Applications/Productivity
Requires: barry libbarry libopensync
BuildRequires: libopensync-devel

%description opensync
Barry is a desktop toolset for managing your BlackBerry(tm) device. (BlackBerry
is a registered trademark of Research in Motion Limited.)

This package contains the opensync plugin.
%endif

%prep
%setup -q
%patch0 -p1

%build
# main tree
%{configure} --with-boost=%{_prefix}
%{__make} %{?_smp_mflags}

# gui tree
%if %{with gui}
cd gui/
%{configure} PKG_CONFIG_PATH="../" CXXFLAGS="-I../.." LDFLAGS="-L../../src"
%{__make} %{?_smp_mflags}
cd ../
%endif

# opensync tree
%if %{with opensync}
cd opensync-plugin/
%{configure} PKG_CONFIG_PATH="../" CXXFLAGS="-I../.." LDFLAGS="-L../../src"
%{__make} %{?_smp_mflags}
cd ../
%endif

%install
# main tree
%{__make} DESTDIR=%{buildroot} install
%{__mkdir_p} %{buildroot}%{_sysconfdir}/udev/rules.d
%{__cp} udev/10-blackberry.rules %{buildroot}%{_sysconfdir}/udev/rules.d/
%{__mkdir_p} %{buildroot}%{_sysconfdir}/security/console.perms.d
%{__cp} udev/10-blackberry.perms %{buildroot}%{_sysconfdir}/security/console.perms.d/

# gui tree
%if %{with gui}
cd gui/
%{__make} DESTDIR=%{buildroot} install
cd ../
%endif

# opensync tree
%if %{with opensync}
cd opensync-plugin/
%{__make} DESTDIR=%{buildroot} install
cd ../
%endif

%files
%defattr(-,root,root)
%doc AUTHORS ChangeLog COPYING NEWS README TODO doc/*

%files -n libbarry
%defattr(-,root,root)
%attr(-,root,root) %{_libdir}/*.so*

%files -n libbarry-devel
%defattr(-,root,root)
%doc examples/*.cc examples/*.am
%attr(0644,root,root) %{_includedir}/barry/*
%attr(0644,root,root) %{_libdir}/*.a
%attr(0755,root,root) %{_libdir}/*.la
%attr(0644,root,root) %{_libdir}/pkgconfig/*.pc

%files util
%defattr(-,root,root)
%attr(0755,root,root) %{_sbindir}/bcharge
%attr(0755,root,root) %{_sbindir}/breset
%attr(0755,root,root) %{_bindir}/btool
%attr(0755,root,root) %{_bindir}/upldif
%attr(0755,root,root) %{_bindir}/ktrans
%attr(0755,root,root) %{_bindir}/translate
%attr(0644,root,root) %{_mandir}/man1/btool*
%attr(0644,root,root) %{_mandir}/man1/bcharge*
%attr(0644,root,root) %config %{_sysconfdir}/udev/rules.d/*
%attr(0644,root,root) %config %{_sysconfdir}/security/console.perms.d/*

%if %{with gui}
%files gui
%defattr(-,root,root)
%attr(0755,root,root) %{_bindir}/barrybackup
%attr(0644,root,root) %{_datadir}/barry/glade/*.glade
%endif

%if %{with opensync}
%files opensync
%defattr(-,root,root)
%attr(0755,root,root) %{_libdir}/opensync/plugins/*
%attr(0644,root,root) %{_datadir}/opensync/defaults/*
%endif

%clean
[ "%{buildroot}" != "/" ] && %{__rm} -rf %{buildroot}
[ "%{barryroot}" != "/" ] && %{__rm} -rf %{barryroot}

%post -n libbarry
/sbin/ldconfig

%postun -n libbarry
/sbin/ldconfig

%changelog
* Sun Mar 04 2007 Troy Engel <tengel@users.sourceforge.net> 0.6-1
- initial build
- adding udev and console perms patch for raw 0.6

