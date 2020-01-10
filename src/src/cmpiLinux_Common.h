/*
 * $Id: cmpiLinux_Common.h,v 1.1.1.1 2009/05/12 21:46:34 nsharoff Exp $
 *
 * (C) Copyright IBM Corp. 2008, 2009
 *
 * THIS FILE IS PROVIDED UNDER THE TERMS OF THE ECLIPSE PUBLIC LICENSE
 * ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
 * CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
 *
 * You can obtain a current copy of the Eclipse Public License from
 * http://www.opensource.org/licenses/eclipse-1.0.php
 *
 * Author: IBM
 *
 * Contributors: IBM 
 *
 * Description:
 * This library contains common /  generic methods to write CMPI Provider.
 *
*/

#ifndef _CMPILINUX_COMMON_H_
#define _CMPILINUX_COMMON_H_

#include "cmpidt.h"

#define ADD_TO_LIST 0
#define FREE_LIST 1
#define FREE_LIST_AND_KEYS 2

/* ---------------------------------------------------------------------------*/

extern char * CSCreationClassName;
extern char * OSCreationClassName;

extern unsigned char CMPI_true;
extern unsigned char CMPI_false;

/* ---------------------------------------------------------------------------*/

void _check_system_key_value_pairs( const CMPIBroker * _broker,
				    const CMPIObjectPath * cop,
				    char * creationClassName,
				    char * className,
				    CMPIStatus * rc );


int _assoc_create_inst_1toN( const CMPIBroker * _broker,
			     const CMPIContext * ctx,
			     const CMPIResult * rslt,
			     const CMPIObjectPath * cop,
			     char * _ClassName,
			     char * _RefLeftClass,
			     char * _RefRightClass,
			     char * _RefLeft,
			     char * _RefRight,
			     int left,
			     int inst,
			     CMPIStatus * rc);

int _assoc_create_inst_conditionally(const CMPIBroker * _broker,
                                     const CMPIContext * ctx,
                                     const CMPIResult * rslt,
                                     const CMPIObjectPath * cop,
                                     char * _ClassName,
                                     char * _RefLeftClass,
                                     char * _RefRightClass,
                                     char * _RefLeft,
                                     char * _RefRight,
                                     int left,
                                     int inst,
                                     int(*condition)(const CMPIBroker*, 
                                                     const CMPIContext*,
                                                     const CMPIInstance*, 
                                                     const CMPIInstance*),
                                     CMPIStatus * rc);

int _assoc_create_inst_setproperties(const CMPIBroker * _broker,
                                     const CMPIContext * ctx,
                                     const CMPIResult * rslt,
                                     const CMPIObjectPath * cop,
                                     char * _ClassName,
                                     char * _RefLeftClass,
                                     char * _RefRightClass,
                                     char * _RefLeft,
                                     char * _RefRight,
                                     int left,
                                     int inst,
                                     int(*setproperties)(CMPIInstance*),
                                     CMPIStatus * rc);

int _assoc_create_inst_setproperties_conditionally(const CMPIBroker * _broker,
                                                   const CMPIContext * ctx,
                                                   const CMPIResult * rslt,
                                                   const CMPIObjectPath * cop,
                                                   char * _ClassName,
                                                   char * _RefLeftClass,
                                                   char * _RefRightClass,
                                                   char * _RefLeft,
                                                   char * _RefRight,
                                                   int left,
                                                   int inst,
                                                   int(*setproperties)(CMPIInstance*),
                                                   int(*condition)(const CMPIBroker*,
                                                                   const CMPIContext*,
                                                                   const CMPIInstance*,
                                                                   const CMPIInstance*),
                                                   CMPIStatus * rc);


/* - method to verify the ref of the CMPIObjectPath is a valid object path of CMPIInstance */
CMPIObjectPath * verify_assoc_ref (const CMPIBroker * _broker,
				const CMPIContext * ctx,
				const CMPIObjectPath * cop,
				char * _ClassName,
				char * _RefRole,
				CMPIStatus * rc);

/* - method to create CMPIInstance out of the CMPIObjectPath of an            */
/*   association instance                                                     */
CMPIInstance * _assoc_get_inst( const CMPIBroker * _broker,
				const CMPIContext * ctx,
				const CMPIObjectPath * cop,
				char * _ClassName,
				char * _RefLeft,
				char * _RefRight,
				CMPIStatus * rc );

CMPIInstance * _assoc_get_inst_setproperties( const CMPIBroker * _broker,
                                              const CMPIContext * ctx,
                                              const CMPIObjectPath * cop,
                                              char * _ClassName,
                                              char * _RefLeft,
                                              char * _RefRight,
                                              int(*setproperties)(CMPIInstance*),
                                              CMPIStatus * rc );

/* - method to create CMPIInstance(s) / CMPIObjectPath(s) of association      */
int _assoc_create_refs_1toN( const CMPIBroker * _broker,
			     const CMPIContext * ctx,
			     const CMPIResult * rslt,
			     const CMPIObjectPath * ref,
			     char * _ClassName,
			     char * _RefLeftClass,
			     char * _RefRightClass,
			     char * _RefLeft,
			     char * _RefRight,
			     int inst,
			     int associators,
			     CMPIStatus * rc);

int _assoc_create_refs_conditionally( const CMPIBroker * _broker,
			     const CMPIContext * ctx,
			     const CMPIResult * rslt,
			     const CMPIObjectPath * ref,
			     char * _ClassName,
			     char * _RefLeftClass,
			     char * _RefRightClass,
			     char * _RefLeft,
			     char * _RefRight,
			     int inst,
			     int associators,
                 int(*condition)(const CMPIBroker*,
                                 const CMPIContext*,
                                 const CMPIInstance*,
                                 const CMPIInstance*),
                 CMPIStatus * rc);

int _assoc_create_refs_setproperties( const CMPIBroker * _broker,
                                      const CMPIContext * ctx,
                                      const CMPIResult * rslt,
                                      const CMPIObjectPath * ref,
                                      char * _ClassName,
                                      char * _RefLeftClass,
                                      char * _RefRightClass,
                                      char * _RefLeft,
                                      char * _RefRight,
                                      int inst,
                                      int associators,
                                      int(*setproperties)(CMPIInstance*),
                                      CMPIStatus * rc);

int _assoc_create_refs_setproperties_conditionally( const CMPIBroker * _broker,
                                                    const CMPIContext * ctx,
                                                    const CMPIResult * rslt,
                                                    const CMPIObjectPath * ref,
                                                    char * _ClassName,
                                                    char * _RefLeftClass,
                                                    char * _RefRightClass,
                                                    char * _RefLeft,
                                                    char * _RefRight,
                                                    int inst,
                                                    int associators,
                                                    int(*setproperties)(CMPIInstance*),
                                                    int(*condition)(const CMPIBroker*,
                                                                    const CMPIContext*,
                                                                    const CMPIInstance*,
                                                                    const CMPIInstance*),
                                                    CMPIStatus * rc);


/* - method to get the name of the target class                               */
char * _assoc_targetClass_Name( const CMPIBroker * _broker,
				const CMPIObjectPath * ref,
				char * _RefLeftClass,
				char * _RefRightClass,
				CMPIStatus * rc);


/* - method to get an empty CMPIObjectPath of the target class                */
CMPIObjectPath * _assoc_targetClass_OP( const CMPIBroker * _broker,
					const CMPIObjectPath * ref,
					char * _RefLeftClass,
					char * _RefRightClass,
					CMPIStatus * rc );

/* - method to check the input parameter resultClass, role and resultRole     */
/*   submitted to the methods of the association interface                    */
int _assoc_chk_param_const_ns( const CMPIBroker * _broker,
                                const CMPIObjectPath * cop,
                                char * _RefLeft,
                                char * _RefRight,
                                char * _RefLeftClass,
                                char * _RefLeftNS,
                                char * _RefRightClass,
                                char * _RefRigthNS,
                                const char * resultClass,
                                const char * role,
                                const char * resultRole,
                                CMPIStatus * rc );

/* - method to check the input parameter resultClass, role and resultRole     */
/*   submitted to the methods of the association interface                    */
int _assoc_check_parameter_const( const CMPIBroker * _broker,
				  const CMPIObjectPath * cop,
				  char * _RefLeft,
				  char * _RefRight,
				  char * _RefLeftClass,
				  char * _RefRightClass,
				  const char * resultClass,
				  const char * role,
				  const char * resultRole,
				  CMPIStatus * rc );


/* Purpose:  To strip leading and trailing spaces                             */
void stripLeadingTrailingSpaces(char *string);


/* Purpose:  To see if the passed in key is a duplicate of an already passed  */
/*              in key.  If not then remember this new key.                   */
int isDuplicateKey(char *key, void **keylist, int add_or_free);


/* Purpose:  To see if two passed in object paths contain the same keys       */
int matchObjectPathKeys(const CMPIObjectPath *op1, const CMPIObjectPath *op2);

int parseObjectPath(char* opstr, CMPIObjectPath* cmpiop);

/* ---------------------------------------------------------------------------*/
/*                       end of cmpiOSBase_Common.h                           */
/* ---------------------------------------------------------------------------*/


#endif
