/*
 * $Id: Linux_Common.h,v 1.1.1.1 2009/05/12 21:46:33 nsharoff Exp $
 *
 * (C) Copyright IBM Corp. 2002, 2009
 *
 * THIS FILE IS PROVIDED UNDER THE TERMS OF THE ECLIPSE PUBLIC LICENSE
 * ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
 * CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
 *
 * You can obtain a current copy of the Eclipse Public License from
 * http://www.opensource.org/licenses/eclipse-1.0.php
 *
 * Author:       Heidi Neumann <heidineu@de.ibm.com>
 * Contributors: Viktor Mihajlovski <mihajlov@de.ibm.com>
 *               C. Eric Wu <cwu@us.ibm.com>
 *
 * Description: CIM Base Instrumentation for Linux
 *
*/

#ifndef _LINUX_COMMOM_H_
#define _LINUX_COMMOM_H_

/* ---------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>


#ifdef __cplusplus
extern "C" {
#endif

#define FALSE 0
#define TRUE  1
/* ---------------------------------------------------------------------------*/
int _debug;

//extern char * CIM_HOST_NAME;
extern char * CIM_OS_NAME;

/* ---------------------------------------------------------------------------*/

#define _OSBASE_TRACE(LEVEL,STR) \
  if ( (LEVEL<=_debug) && (LEVEL>0) ) \
  _osbase_trace(LEVEL,__FILE__,__LINE__,_format_trace STR)

/* ---------------------------------------------------------------------------*/
void initialize();
void finish();

char * get_system_name();
char * get_os_name();

signed short  get_os_timezone();
time_t _get_os_boottime();
void _cat_timezone( char * str, signed short zone );

/* ---------------------------------------------------------------------------*/

int exec_command(const char *, char **, char ***, char ***);
int runcommand(const char *, char **, char ***, char ***);
void freeresultbuf(char **);

char ** line_to_array( char * , int );

/* ---------------------------------------------------------------------------*/

int get_system_parameter(char *, char *, char *, int);
int set_system_parameter(char *, char *, char *);
char * get_device_desc(const char*);

/* ---------------------------------------------------------------------------*/
// TRACE FACILITY
/* ---------------------------------------------------------------------------*/

char * _format_trace(char *fmt,...);
void _osbase_trace(  int, char *, int, char *);

/* ---------------------------------------------------------------------------*/

#ifdef __cplusplus
   }
#endif

/* ---------------------------------------------------------------------------*/

#endif
