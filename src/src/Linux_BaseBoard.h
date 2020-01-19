/*
 * $Id: Linux_BaseBoard.h,v 1.1.1.1 2009/05/12 21:46:33 nsharoff Exp $
 *
 * (C) Copyright IBM Corp. 2009
 *
 * THIS FILE IS PROVIDED UNDER THE TERMS OF THE ECLIPSE PUBLIC LICENSE
 * ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
 * CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
 *
 * You can obtain a current copy of the Eclipse Public License from
 * http://www.opensource.org/licenses/eclipse-1.0.php
 *
 * Author:       Heidi Neumann <heidineu@de.ibm.com>
 * Contributors: 
 *
 * Description: 
 * This file defines the interfaces for the resource access implementation 
 * of the CIM class Linux_BaseBoard.
 *
*/

#ifndef _OSBASE_BASEBOARD_H_
#define _OSBASE_BASEBOARD_H_

/* ---------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

  /* ---------------------------------------------------------------------------*/

  struct cim_baseboard {
    char * tag;
    char * vendor;
    char * type;
    char * model;
    char * partNumber;
    char * serialNumber;
    char * UUID;
  };

  int get_baseboard_data(struct cim_baseboard *data);
  int get_baseboard_tag(char *tag, size_t size);

  void free_baseboard_data(struct cim_baseboard *data);

  /* ---------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

/* ---------------------------------------------------------------------------*/

#endif


