# $Id: configure.ac,v 1.2 2009/09/01 16:59:02 nsharoff Exp $
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
#	04/05/2008 Narasimha Sharoff <nsharoff@us.ibm.com> Initial Version
#
# Description:
#    Autoconf input file for HBA Providers.
#    Process this file with autoconf to produce a configure script.
#

AC_INIT(SBLIM-HBA-PROVIDERS, 1.0.0, sblim-devel@lists.sourceforge.net, sblim-smis-hba)

AC_PREREQ(2.59)
AC_CONFIG_SRCDIR([src/cmpiSMIS_FCCard.c])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile sblim-smis-hba.spec])

AC_CANONICAL_HOST
case $host_cpu in
     i*86) HW=INTEL;;
     s390*) HW=S390;;
     ppc*) HW=PPC;;
     x86_64) HW=X86_64;;
     ia64) HW=IA64;;
     mips*) HW=MIPS;;
     **) HW=GENERIC;;
esac

# Configure command line options
AC_ARG_VAR([CIMSERVER],[target CIM server (sfcb).])
AC_ARG_VAR([PROVIDERDIR],[directory where hba providers will be installed.])
AC_ARG_VAR([TESTSUITEDIR],[directory where hba testsuite is installed.])

# Checks for programs, typedefs, structures and compiler characterstics.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_STRUCT_TM

# Checks for header files.
AC_HEADER_TIME
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h limits.h malloc.h string.h sys/time.h unistd.h])

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MKTIME
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([gethostname memset munmap strcasecmp strchr strdup ])

# Checks for libraries.
AC_CHECK_LIB([ind_helper], [ind_start], [LINDHELP=-lind_helper], [LINDHELP=])

# Checks for CMPI files, CIM server, provider dir (defined in acinclude.m4)
CHECK_CIMSERVER
CHECK_CMPI
CHECK_PROVIDERDIR

# Check for the SBLIM testsuite (this macro is defined in acinclude.m4)
CHECK_TESTSUITE 
if test x"$TESTSUITEDIR" = x; then
   TESTSUITE_SUPPORT=Disabled
else
   TESTSUITE_SUPPORT=Enabled 
fi

# Test for indication support
EVENTFLAGS=-DNOEVENTS
if test x$LINDHELP != x; then
# Indication lib is there, what about headers
  CHECK_INDHELP_HEADER
  if test "$have_INDHELP" == "yes"; then
     EVENTFLAGS=
  else
     LINDHELP=
  fi
fi
AC_SUBST(LINDHELP)

#Extend CPPFLAGS
CPPFLAGS="$CPPFLAGS -D$HW $EVENTFLAGS"
if test x"$EVENTFLAGS" == x; then
   INDICATION_SUPPORT=Enabled
else    
   INDICATION_SUPPORT=Disabled
fi 

# Display configuration options
echo "----------------------------------------------------------"
echo "Configuration for $PACKAGE complete."
echo ""
echo "The following configuration options have been selected:"
echo "   CIMSERVER: " $CIMSERVER
echo "   PROVIDERDIR: " $PROVIDERDIR
echo "   CPPFLAGS:" $CPPFLAGS
echo "Indication Support" $INDICATION_SUPPORT
echo "   TESTSUITE_SUPPORT: " $TESTSUITE_SUPPORT
if test x"$TESTSUITEDIR" != x; then
   echo "   TESTSUITEDIR: " $TESTSUITEDIR
fi
echo "----------------------------------------------------------"

# Generate configure scripts for the Makefile
AC_OUTPUT

echo "You may now run make"
