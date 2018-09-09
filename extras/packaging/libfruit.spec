#
# spec file for package fruit
#

Name:           libfruit
Version:        @FRUIT_VERSION@
Release:        0
Summary:        Dependency Injection Framework For C++
License:        Apache-2.0
Group:          Development/Libraries/C and C++
Url:            https://github.com/google/fruit
Source0:        fruit-%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  boost-devel
Suggests:       libfruit-devel = %{version}

%if 0%{?fedora_version} || 0%{?rhel_version} || 0%{?centos_version}
BuildRequires:  gcc-c++ >= 5.0.0
%else
# OpenSUSE doesn't include the bugfix release version component in the package version.
BuildRequires:  gcc-c++ >= 5.0
%endif

BuildRoot:      %{_tmppath}/%{name}-%{version}-build

%description
Fruit is a dependency injection framework for C++, loosely inspired by the
Guice framework for Java.
It uses C++ metaprogramming together with some new C++11 features to detect
most injection problems at compile-time.

%package devel
Summary:        Dependency Injection Framework For C++ - Development Files
License:        Apache-2.0
Group:          Development/Libraries/C and C++
Url:            https://github.com/google/fruit
Requires:       libfruit = %{version}

%description devel
Fruit is a dependency injection framework for C++, loosely inspired by the
Guice framework for Java.
It uses C++ metaprogramming together with some new C++11 features to detect
most injection problems at compile-time.

%prep
%setup -q -n fruit-%{version}

%build
cmake -DCMAKE_INSTALL_PREFIX=%{_prefix} -DINSTALL_LIBRARY_DIR=%{_libdir} -DCMAKE_BUILD_TYPE=RelWithDebInfo
 
%{__make} %{?jobs:-j%jobs}

%install
%{__make} DESTDIR=%{buildroot} install

%files
%defattr(-,root,root)
%{_libdir}/libfruit.*

%files devel
%defattr(-,root,root)
%{_includedir}/fruit

%changelog
