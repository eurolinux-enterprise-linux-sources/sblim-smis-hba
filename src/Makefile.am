# $Id: Makefile.am,v 1.3 2009/09/01 19:18:14 nsharoff Exp $
#
# (C) Copyright IBM Corp. 2008, 2009
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
# Contributors:
#  04/05/2008 Narasimha Sharoff <nsharoff@us.ibm.com>   Initial version.
#
# Description:  Automake input file for HBA HDR Providers
#

# SBLIM Include Directory
sblimincdir = $(includedir)/sblim

# Start from an empty extra distribution file list
EXTRA_DIST=

# docdir defines where the documentation goes
docdir=$(datadir)/doc/$(PACKAGE)-$(VERSION)

# Automake instructions for documentation
doc_DATA=README AUTHORS COPYING 

# providerdir defines where provider libraries will be installed (@PROVIDERDIR@ is set by the configure script). 
providerdir = @PROVIDERDIR@

#
# Automake instructions for src dir
#

# HBA provider libraries
provider_LTLIBRARIES =                  \
	libcmpiLinux_Common.la          \
	libcmpiSMIS_HBA_HDR_Provider.la \
	libcmpiLinux_ECTP_Provider.la

# Common
libcmpiLinux_Common_la_SOURCES =	\
	src/cmpiLinux_Common.c		\
	src/Linux_Common.c 		\
	src/cmpiLinux_Common.h		\
	src/Linux_Common.h


# HBA_HDR Provider
libcmpiSMIS_HBA_HDR_Provider_la_SOURCES =		\
	src/Linux_CommonHBA.c				\
	src/cmpiSMIS_FCCard.c				\
	src/cmpiSMIS_FCCardProvider.c                   \
	src/cmpiSMIS_FCControlledByProvider.c           \
	src/cmpiSMIS_FCDeviceSAPImplementationProvider.c    \
	src/cmpiSMIS_FCElementSoftwareIdentityProvider.c    \
	src/cmpiSMIS_FCElementStatisticalDataProvider.c     \
	src/cmpiSMIS_FCHostedAccessPointProvider.c          \
	src/cmpiSMIS_FCInstalledSoftwareIdentityProvider.c  \
	src/cmpiSMIS_FCLogicalDisk.c                        \
	src/cmpiSMIS_FCLogicalDiskProvider.c		\
	src/cmpiSMIS_FCPort.c				\
	src/cmpiSMIS_FCPortController.c			\
	src/cmpiSMIS_FCPortControllerProvider.c		\
	src/cmpiSMIS_FCPortProvider.c			\
	src/cmpiSMIS_FCPortStatistics.c			\
	src/cmpiSMIS_FCPortStatisticsProvider.c		\
	src/cmpiSMIS_FCProduct.c				\
	src/cmpiSMIS_FCProductProvider.c			\
	src/cmpiSMIS_FCProductPhysicalComponentProvider.c	\
	src/cmpiSMIS_FCRealizesProvider.c			\
	src/cmpiSMIS_FCSCSIInitiatorTargetLogicalUnitPathProvider.c \
	src/cmpiSMIS_FCSCSIProtocolEndpoint.c		\
	src/cmpiSMIS_FCSCSIProtocolEndpointProvider.c	\
	src/cmpiSMIS_FCSoftwareIdentity.c			\
	src/cmpiSMIS_FCSoftwareIdentityProvider.c		\
	src/cmpiSMIS_FCSoftwareIdentity_Driver.c		\
	src/cmpiSMIS_FCSoftwareIdentity_DriverProvider.c	\
	src/cmpiSMIS_FCSoftwareIdentity_Firmware.c		\
	src/cmpiSMIS_FCSoftwareIdentity_FirmwareProvider.c	\
	src/cmpiSMIS_FCSystemDevice_LogicalDeviceProvider.c	\
	src/cmpiSMIS_FCSystemDeviceProvider.c \
	src/cmpiOSBase_ComputerSystem.c			\
	src/cmpiOSBase_ComputerSystemProvider.c		\
	src/Linux_BaseBoard.c				\
	src/Linux_ComputerSystem.c			\
	src/cmpiSMIS_FC.h				\
	src/cmpiLinux_Common.h				\
	src/cmpiOSBase_ComputerSystem.h			\
	src/Linux_CommonHBA.h 				\
	src/Linux_BaseBoard.h 				\
	src/Linux_ComputerSystem.h			\
	src/Linux_Common.h				\
	src/cmpiOSBase_Common.h                         \
	src/dmiinfo.h                                   \
	src/OSBase_Common.h

libcmpiSMIS_HBA_HDR_Provider_la_LDFLAGS = -Wc,-nostartfiles \
		-L/usr/lib -lHBAAPI -lcmpiLinux_Common \
		-L/usr/local/lib -ldmiinfo

# ECTP Provider
libcmpiLinux_ECTP_Provider_la_SOURCES =			\
		src/cmpiLinux_ElementConformsToProfileProvider.c \
		src/cmpiSMIS_FCElementSoftwareIdentityProvider.c \
		src/cmpiSMIS_FCElementStatisticalDataProvider.c

libcmpiLinux_ECTP_Provider_la_LDFLAGS = -Wc,-nostartfiles \
		-L/usr/lib -lHBAAPI -lcmpiLinux_Common \
		-lcmpiSMIS_HBA_HDR_Provider


# Installable Header Files

# Non-Installable Header Files
noinst_HEADERS = \
		src/cmpiOSBase_ComputerSystem.h \
		src/Linux_BaseBoard.h \
		src/Linux_ComputerSystem.h \
		src/cmpiSMIS_FC.h \
		src/Linux_CommonHBA.h

# We must explicity add the RPM spec file to the distribution package
EXTRA_DIST+=$(PACKAGE).spec $(PACKAGE).rh.spec


#
# Automake instructions for ./mof subdir
#
SCHEMAS=$(srcdir)/mof/Linux_SMIS_HBA_HDR.mof \
	$(srcdir)/mof/Linux_SMIS_ECTP.mof
REGISTRATIONS=$(srcdir)/mof/Linux_SMIS_HBA_HDR.reg \
	$(srcdir)/mof/Linux_SMIS_ECTP.reg


# We must explicity add all the schema files to the distribution package
pkgdata_DATA=$(SCHEMAS) $(REGISTRATIONS)
pkgdata_SCRIPTS=provider-register.sh
EXTRA_DIST+=mof $(pkgdata_SCRIPTS)

# Register the provider(s) and class definition(s) to the current CIM server/CIMOM
postinstall:
	sh $(srcdir)/provider-register.sh -t @CIMSERVER@ -r $(REGISTRATIONS) -m $(SCHEMAS)

preuninstall:
	sh $(srcdir)/provider-register.sh -d -t @CIMSERVER@ -r $(REGISTRATIONS) -m $(SCHEMAS)

dist-hook:
	test -d "$(distdir)" &&	rm -rf `find $(distdir) -type d -name CVS`

#
# Automake instructions for ./test subdir
#
#EXTRA_DIST+=test README.TEST
if TESTSUITE
testsuitedir=@TESTSUITEDIR@
testsuitesystemdir=@TESTSUITEDIR@/system/linux
testsuitecimdir=@TESTSUITEDIR@/cim
testsuite_SCRIPTS=test/test-sblim-hba.sh
testsuitesystem_DATA= \
	test/system/linux/Linux_FCCard.system \
	test/system/linux/Linux_FCControlledBy.system \
	test/system/linux/Linux_FCDeviceSAPImplementation.system \
	test/system/linux/Linux_FCElementSoftwareIdentity.system \
	test/system/linux/Linux_FCElementStatisticalData.system \
	test/system/linux/Linux_FCHostedAccessPoint.system \
	test/system/linux/Linux_FCInstalledSoftwareIdentity.system \
	test/system/linux/Linux_FCLogicalDisk.system \
	test/system/linux/Linux_FCPortController.system \
	test/system/linux/Linux_FCPortStatistics.system \
	test/system/linux/Linux_FCPort.system \
	test/system/linux/Linux_FCProduct.system \
	test/system/linux/Linux_FCRealizes.system \
	test/system/linux/Linux_FCSCSIProtocolEndpoint.system \
	test/system/linux/Linux_FCSoftwareIdentity_Driver.system \
	test/system/linux/Linux_FCSoftwareIdentity_Firmware.system \
	test/system/linux/Linux_FCSoftwareIdentity.system \
	test/system/linux/Linux_FCSystemDevice_LogicalDevice.system \
	test/system/linux/Linux_FCSystemDevice.system

#testsuitesystem_SCRIPTS=
testsuitecim_DATA= \
	test/cim/Linux_FCCard.cim \
	test/cim/Linux_FCControlledBy.cim \
	test/cim/Linux_FCDeviceSAPImplementation.cim \
	test/cim/Linux_FCElementSoftwareIdentity.cim \
	test/cim/Linux_FCElementStatisticalData.cim \
	test/cim/Linux_FCHostedAccessPoint.cim \
	test/cim/Linux_FCInstalledSoftwareIdentity.cim \
	test/cim/Linux_FCLogicalDisk.cim \
	test/cim/Linux_FCPort.cim \
	test/cim/Linux_FCPortController.cim \
	test/cim/Linux_FCPortStatistics.cim \
	test/cim/Linux_FCProduct.cim \
	test/cim/Linux_FCRealizes.cim \
	test/cim/Linux_FCSCSIProtocolEndpoint.cim \
	test/cim/Linux_FCSoftwareIdentity.cim \
	test/cim/Linux_FCSoftwareIdentity_Driver.cim \
	test/cim/Linux_FCSoftwareIdentity_Firmware.cim \
	test/cim/Linux_FCSystemDevice.cim \
	test/cim/Linux_FCSystemDevice_LogicalDevice.cim

runtest: install
	cd $(DESTDIR)$(TESTSUITEDIR) &&	./test-sblim-hba.sh

#doc_DATA+=README.TEST
endif
