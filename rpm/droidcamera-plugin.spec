Name:           droidcamera-plugin
Summary:        A droidmedia-based library to simplify video capture
Version:        0.1
Release:        1
License:        ASL 2.0
URL:            https://github.com/d-grigorev/%{name}
Source0:        %{name}-%{version}.tar.gz
BuildRequires:  meson
BuildRequires:  ninja
BuildRequires:  pkgconfig(libandroid-properties)
BuildRequires:  droidmedia-devel
Requires:       droidmedia

%description
A droidmedia-based library to simplify video capture, hardware-dependent plugin.

%prep
%setup -q

%build
%meson -Dbuild-library=false -Dbuild-tests=false
%meson_build

%install
%meson_install

%files
%defattr(-,root,root,-)
%license LICENSE
%{_libdir}/droidcamera/%{version}/plugin/lib%{name}.so
