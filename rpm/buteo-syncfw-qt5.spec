Name:    buteo-syncfw-qt5
Version: 0.11.0
Release: 1
Summary: Synchronization backend
URL:     https://github.com/sailfishos/buteo-syncfw/
License: LGPLv2
Source0: %{name}-%{version}.tar.gz
Source1: %{name}.privileges
Source2: move-buteo-config.sh
BuildRequires: doxygen
BuildRequires: fdupes
BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5Network)
BuildRequires: pkgconfig(Qt5DBus)
BuildRequires: pkgconfig(Qt5Sql)
BuildRequires: pkgconfig(Qt5Test)
BuildRequires: pkgconfig(Qt5Qml)
BuildRequires: pkgconfig(dbus-1)
BuildRequires: pkgconfig(accounts-qt5) >= 1.13
BuildRequires: pkgconfig(libsignon-qt5)
BuildRequires: pkgconfig(qt5-boostable)
BuildRequires: pkgconfig(keepalive)
BuildRequires: pkgconfig(gio-2.0)
BuildRequires: pkgconfig(mce-qt5) >= 1.1.0
BuildRequires: pkgconfig(systemd)
BuildRequires: oneshot
Requires: oneshot
%{_oneshot_requires_post}

%description
%{summary}.

%package devel
Summary: Development files for %{name}
Requires: %{name} = %{version}-%{release}

%description devel
%{summary}.

%package msyncd
Summary: Buteo sync daemon
Requires: %{name} = %{version}-%{release}
Requires: systemd
Requires: systemd-user-session-targets
Requires: mapplauncherd-qt5
Provides: buteo-syncfw-msyncd = %{version}
Obsoletes: buteo-syncfw-msyncd < %{version}

%description msyncd
%{summary}.

%package doc
Summary: Documentation for %{name}

%description doc
%{summary}.

%package tests
Summary: Tests for %{name}

%description tests
%{summary}.


%package qml-plugin
Summary: QML plugin for %{name}

%description qml-plugin
%{summary}.

%prep
%setup -q


%build
%qmake5 -recursive "VERSION=%{version}" CONFIG+=usb-moded DEFINES+=USE_KEEPALIVE
%make_build
make doc %{_smp_mflags}


%install
%qmake5_install
%fdupes %{buildroot}/opt/tests/buteo-syncfw/
mkdir -p %{buildroot}%{_userunitdir}/user-session.target.wants
ln -s ../msyncd.service %{buildroot}%{_userunitdir}/user-session.target.wants/

mkdir -p %{buildroot}%{_datadir}/mapplauncherd/privileges.d
install -m 644 -p %{SOURCE1} %{buildroot}%{_datadir}/mapplauncherd/privileges.d/
mkdir -p %{buildroot}%{_oneshotdir}
install -m 755 -p %{SOURCE2} %{buildroot}%{_oneshotdir}
mkdir -p %{buildroot}%{_libdir}/buteo-plugins-qt5/oopp

%post
/sbin/ldconfig
%{_bindir}/add-oneshot --all-users --privileged --now move-buteo-config.sh
if [ "$1" -ge 1 ]; then
    systemctl-user daemon-reload || true
    systemctl-user try-restart msyncd.service || true
fi

%post msyncd
glib-compile-schemas %{_datadir}/glib-2.0/schemas

%postun
/sbin/ldconfig
if [ "$1" -eq 0 ]; then
    systemctl-user stop msyncd.service || true
    systemctl-user daemon-reload || true
fi

%files
%license COPYING
%{_libdir}/libbuteosyncfw5.so.*
%{_libexecdir}/buteo-oopp-runner
%{_oneshotdir}/move-buteo-config.sh

%files devel
%{_includedir}/*
%{_libdir}/*.so
%{_libdir}/*.prl
%{_libdir}/pkgconfig/*.pc

%files msyncd
%{_userunitdir}/*.service
%{_userunitdir}/user-session.target.wants/*.service
%{_sysconfdir}/syncwidget
%{_bindir}/msyncd
%{_datadir}/mapplauncherd/privileges.d/*
%{_datadir}/glib-2.0/schemas/*
%dir %{_libdir}/buteo-plugins-qt5
%dir %{_libdir}/buteo-plugins-qt5/oopp

%files doc
%{_docdir}/buteo-syncfw-doc

%files tests
/opt/tests/buteo-syncfw
%{_datadir}/accounts/services/*.service

%files qml-plugin
%dir %{_libdir}/qt5/qml/Buteo/Profiles
%{_libdir}/qt5/qml/Buteo/Profiles/libbuteoprofiles.so
%{_libdir}/qt5/qml/Buteo/Profiles/qmldir
