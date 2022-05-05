Name:           taskmonitor
Version:        1.0
Release:        1%{?dist}
Summary:        Linux Task Monitor

License:        MIT
URL:            https://gitlab.com/taskmonitor/taskmonitor
Source:         %{url}/-/archive/main/taskmonitor-main.tar.bz2

BuildRequires:  cmake
BuildRequires:  gcc
BuildRequires:  libnl3-devel >= 3.5.0
BuildRequires:  systemd-devel >= 243

Requires: systemd >= 243
Requires: libnl >= 3.5.0

%description
%{summary}

%global _vpath_srcdir %{name}-main

%prep
%autosetup -c

%build
%cmake .
%cmake_build

%install
%cmake_install

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%files
%{_sbindir}/taskmonitor
%{_sysconfdir}/taskmonitor.conf
%{_datadir}/licenses/taskmonitor/LICENSE
