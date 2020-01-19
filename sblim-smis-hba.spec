%global tog_pegasus_version 2:2.5.1
%global provider_dir %{_libdir}/cmpi/

Name:           sblim-smis-hba
Version:        1.0.0
Release:        8%{?dist}
Summary:        SBLIM SMIS HBA HDR Providers

Group:          Applications/System
License:        EPL
URL:            http://sblim.wiki.sourceforge.net/
Source0:        http://downloads.sourceforge.net/sblim/%{name}-%{version}.tar.bz2

# Patch0: upstream tarball doesn't contain testsuite but default Makefile is going to build it
Patch0:         sblim-smis-hba-1.0.0-no-testsuite.patch
Patch1:         sblim-smis-hba-1.0.0-include.patch
Patch2:         sblim-smis-hba-1.0.0-registration-fix.patch

BuildRequires:  sblim-cmpi-devel, sblim-cmpi-base-devel
BuildRequires:  libhbaapi-devel
BuildRequires:  autoconf, automake, libtool
BuildRequires:  tog-pegasus-devel >= %{tog_pegasus_version}, curl-devel, perl, libsysfs-devel
BuildRequires:  binutils-devel, autoconf, automake, libtool, flex, bison

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
Requires:       tog-pegasus >= %{tog_pegasus_version}
Requires:       libhbaapi
Requires:       sblim-cmpi-base

%description
SMI-S standards based HBA CMPI Providers.

%prep
%setup -q
%patch0 -p1 -b .no-testsuite
%patch1 -p2 -b .include
%patch2 -p1 -b .registration-fix
autoreconf
# ltmain.sh is old, create new one
rm -f ltmain.sh
libtoolize --force


%build
export CFLAGS="$RPM_OPT_FLAGS -fno-strict-aliasing"
export CXXFLAGS="$CFLAGS"
%configure \
   TESTSUITEDIR=%{_datadir}/sblim-testsuite \
   CIMSERVER=pegasus \
   PROVIDERDIR=%{provider_dir} \
   LDFLAGS="-L${RPM_BUILD_ROOT}%{_libdir}/cmpi";
# workaround libtool issue
sed -i -e '/not ending/ s/.*/true/' libtool
# do not use smp_flags!
make


%install
make install DESTDIR=$RPM_BUILD_ROOT
# remove unused libtool files
rm -f $RPM_BUILD_ROOT/%{_libdir}/*a
rm -f $RPM_BUILD_ROOT/%{_libdir}/cmpi/*a


%files
%doc AUTHORS COPYING README
%{_datadir}/%{name}
%{_libdir}/cmpi/libcmpiLinux_Common.so*
%{_libdir}/cmpi/libcmpiLinux_ECTP_Provider.so*
%{_libdir}/cmpi/libcmpiSMIS_HBA_HDR_Provider.so*

%global HBA_SCHEMA %{_datadir}/%{name}/Linux_SMIS_HBA_HDR.mof %{_datadir}/%{name}/Linux_SMIS_ECTP.mof
%global HBA_REGISTRATION %{_datadir}/%{name}/Linux_SMIS_HBA_HDR.reg %{_datadir}/%{name}/Linux_SMIS_ECTP.reg

%pre
# If upgrading, deregister old version
if [ $1 -gt 1 ]
then
  %{_datadir}/%{name}/provider-register.sh -d \
        -r %{HBA_REGISTRATION} -m %{HBA_SCHEMA} > /dev/null 2>&1 || :;
fi

%post
# Register Schema and Provider - this is higly provider specific
%{_datadir}/%{name}/provider-register.sh \
        -r %{HBA_REGISTRATION} -m %{HBA_SCHEMA} > /dev/null 2>&1 || :;
/sbin/ldconfig

%preun
# Deregister only if not upgrading 
if [ $1 -eq 0 ]
then
  %{_datadir}/%{name}/provider-register.sh -d \
        -r %{HBA_REGISTRATION} -m %{HBA_SCHEMA} > /dev/null 2>&1 || :;
fi

%postun
if [ $1 -eq 0 ]
then
  /sbin/ldconfig
fi


%changelog
* Thu Feb 14 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.0.0-8
- Rebuilt for https://fedoraproject.org/wiki/Fedora_19_Mass_Rebuild

* Tue Jan 22 2013 Vitezslav Crhonek <vcrhonek@redhat.com> - 1.0.0-7
- Remove bogus BuildRequire of sblim-tools-libra-devel

* Mon Sep 10 2012 Vitezslav Crhonek <vcrhonek@redhat.com> - 1.0.0-6
- Fix issues found by fedora-review utility in the spec file

* Sat Jul 21 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.0.0-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Sat Jan 14 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.0.0-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_17_Mass_Rebuild

* Tue Nov 29 2011 Vitezslav Crhonek <vcrhonek@redhat.com> - 1.0.0-3
- Build with -fno-strict-aliasing, fix requires, fix registration files

* Wed Feb 09 2011 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.0.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_15_Mass_Rebuild

* Thu Jan 20 2011 Vitezslav Crhonek <vcrhonek@redhat.com> - 1.0.0-1
- Initial support

