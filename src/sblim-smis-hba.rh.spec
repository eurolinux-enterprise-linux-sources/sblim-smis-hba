# $Id: sblim-smis-hba.rh.spec,v 1.1 2009/09/01 16:57:13 nsharoff Exp $
#
# (C) Copyright IBM Corp. 2005, 2009
#
# THIS FILE IS PROVIDED UNDER THE TERMS OF THE ECLIPSE PUBLIC LICENSE
# ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
# CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
# 
# You can obtain a current copy of the Eclipse Public License from
# http://www.opensource.org/licenses/eclipse-1.0.php
#
# Author: IBM
#
# Contributors: IBM 
#
# Description: 
# Package spec for @PACKAGE@
#

BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

Summary: SBLIM SMIS HBA HDR Providers
Name: @PACKAGE_TARNAME@
Version: @PACKAGE_VERSION@
Release: 1
Group: Systems Management/Base
URL: http://www.sblim.org
License: EPL

Source0: http://prdownloads.sourceforge.net/sblim/%{name}-%{version}.tar.bz2

BuildRequires: cmpi-devel
Requires: cimserver

%Description
SMI-S standards based HBA CMPI Providers

%Package devel
Summary: SBLIM SMIS HBA HDR Instrumentation Header Development Files
Group: Systems Management/Base
Requires: %{name} = %{version}-%{release}

%Description devel
SBLIM SMIS HBA HDR Provider Development Package contains header files and 
link libraries for dependent provider packages

%Package test
Summary: SBLIM SMIS HBA HDR Instrumentation Testcase Files
Group: Systems Management/Base
Requires: %{name} = %{version}-%{release}
Requires: sblim-testsuite

%Description test
SBLIM SMIS HBA HDR Testcase Files for the SBLIM Testsuite

%prep

%setup -q

%build

%configure TESTSUITEDIR=%{_datadir}/sblim-testsuite 

make %{?_smp_mflags}

%install

[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

make DESTDIR=$RPM_BUILD_ROOT install

# remove unused libtool files
rm -f $RPM_BUILD_ROOT/%{_libdir}/*a
rm -f $RPM_BUILD_ROOT/%{_libdir}/cmpi/*a

%pre
%if "@LINDHELP@" != ""
%define SCHEMA %{_datadir}/%{name}/Linux_SMIS_HBA_HDR.mof %{_datadir}/%{name}/Linux_SMIS_ECTP.mof
%define REGISTRATION %{_datadir}/%{name}/Linux_SMIS_HBA_HDR.reg %{_datadir}/%{name}/Linux_SMIS_ECTP.reg
%else
%define SCHEMA %{_datadir}/%{name}/Linux_SMIS_HBA_HDR.mof %{_datadir}/%{name}/Linux_SMIS_ECTP.mof
%define REGISTRATION %{_datadir}/%{name}/Linux_SMIS_HBA_HDR.reg %{_datadir}/%{name}/Linux_SMIS_ECTP.reg
%endif

# If upgrading, deregister old version
if [ $1 -gt 1 ]
then
  %{_datadir}/%{name}/provider-register.sh -d \
	-r %{REGISTRATION} -m %{SCHEMA} > /dev/null
fi

%post
# Register Schema and Provider - this is higly provider specific

%{_datadir}/%{name}/provider-register.sh \
	-r %{REGISTRATION} -m %{SCHEMA} > /dev/null

/sbin/ldconfig

%preun
# Deregister only if not upgrading 
if [ $1 -eq 0 ]
then
  %{_datadir}/%{name}/provider-register.sh -d \
	-r %{REGISTRATION} -m %{SCHEMA} > /dev/null
fi

%postun -p /sbin/ldconfig

%clean

[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%files

%defattr(-,root,root) 
%docdir %{_datadir}/doc/%{name}-%{version}
%{_datadir}/%{name}
%{_datadir}/doc/%{name}-%{version}
%{_libdir}/*.so.*
%{_libdir}/cmpi/*.so

%files devel

%defattr(-,root,root)
%{_includedir}/*
%{_libdir}/*.so

%files test

%defattr(-,root,root)
%{_datadir}/sblim-testsuite

%changelog
* Mon May 05 2008 Narasimha Sharoff <nsharoff@us.ibm.com>      1.5.5-1
  -  Changes for HBA providers.
* Wed Oct 28 2005 Viktor Mihajlovski <mihajlov@de.ibm.com>	1.5.4-1
  - enhanced commonality with RH spec file
* Wed Jul 20 2005 Mark Hamzy <hamzy@us.ibm.com>	1.5.3-1
  - initial support
