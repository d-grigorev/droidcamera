Name:           droidcamera
Summary:        A droidmedia-based library to simplify video capture
Version:        0.1
Release:        1
License:        ASL 2.0
URL:            https://github.com/d-grigorev/%{name}
Source0:        %{name}-%{version}.tar.gz
BuildRequires:  meson
BuildRequires:  ninja

%description
A droidmedia-based library to simplify video capture

%package        devel
Summary:        droidcamera development headers
Requires:       droidcamera = %{version}-%{release}
BuildArch:      noarch

%description    devel
%{summary}

%prep
%setup -q

%build
%meson -Dbuild-tests=false -Dbuild-plugin=false
%meson_build

%install
%meson_install

%files
%defattr(-,root,root,-)
%license LICENSE
%{_libdir}/libdroidcamera.so.*

%files devel
%defattr(-,root,root,-)
%{_includedir}/droidcamera/*.h
%{_libdir}/libdroidcamera.so
