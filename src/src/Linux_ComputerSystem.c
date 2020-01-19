/*
 * $Id: Linux_ComputerSystem.c,v 1.1.1.1 2009/05/12 21:46:34 nsharoff Exp $
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
 *
 * Author:       Heidi Neumann <heidineu@de.ibm.com>
 * Contributors: Viktor Mihajlovski <mihajlov@de.ibm.com>
 *               C. Eric Wu <cwu@us.ibm.com>               
 *
 * Description:
 * This shared library provides resource access functionality for the class
 * Linux_ComputerSystem. 
 * It is independent from any specific CIM technology.
 *
*/

#include "Linux_Common.h"
#include "Linux_ComputerSystem.h"
#include <unistd.h>

/* ---------------------------------------------------------------------------*/
// private declarations

#if defined (S390)
#define LPARNAME   "grep 'LPAR Name' /proc/sysinfo"
#endif

#if defined (PPC)
#define LPARNAME   "grep 'partition_id' /proc/ppc64/lparcfg"
#endif

/* ---------------------------------------------------------------------------*/

/* returns the primary owner of the system ... always root ;-) */
char * get_cs_primownername() {
  char * ptr = NULL;

  _OSBASE_TRACE(4,("--- get_cs_primownername() called"));

  ptr = (char*)malloc(5);
  strcpy(ptr,"root");

  _OSBASE_TRACE(4,("--- get_cs_primownername() exited"));
  return ptr;
}

/* returns the contact address of the primary owner of the system ...
 * always root@host.domain
 */
char * get_cs_primownercontact() {
  char * ptr  = NULL;
  char * own  = NULL;
  char * host = NULL;

  _OSBASE_TRACE(4,("--- get_cs_primownercontact() called"));

  if( (own = get_cs_primownername()) != NULL ) {
    host = get_system_name();
    ptr = (char*)malloc( (strlen(own)+strlen(host)+2));
    strcpy( ptr, own);
    strcat( ptr,"@");
    strcat( ptr, host);
    if(own) free(own);
    _OSBASE_TRACE(4,("--- get_cs_primownercontact() exited"));
    return ptr;
  }

  _OSBASE_TRACE(4,("--- get_cs_primownercontact() failed"));
  return NULL;
}

int get_cs_lparid(char *lparid, size_t size) {

  memset(lparid,0,size);

#if defined (S390) || defined (PPC)

  char **hdout = NULL;
  char *ptr    = NULL;
  char id[255];

  if( runcommand(LPARNAME,NULL,&hdout,NULL)) { return -1; }
  if(hdout[0]) {
#if defined (S390)
    sscanf(hdout[0],"%*s %*s %s",id);
#endif
#if defined (PPC)
    ptr=strchr(hdout[0],'=');
    ptr+=1;
    sscanf(ptr,"%s",id);
#endif
    if(strlen(id)+1>size) { return -1; }
    strcpy(lparid,id);
  }
  return 0;
#endif
  return -1;
}

/* ---------------------------------------------------------------------------*/
/*                       end of OSBase_ComputerSystem.c                       */
/* ---------------------------------------------------------------------------*/

