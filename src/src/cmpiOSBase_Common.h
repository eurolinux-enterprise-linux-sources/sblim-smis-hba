/*
 * $Id: cmpiOSBase_Common.h,v 1.1.1.1 2009/05/12 21:46:32 nsharoff Exp $
 *
 * (C) Copyright IBM Corp. 2002,2009
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
 * This library contains common /  generic methods to write CMPI Provider.
 *
*/


#ifndef _CMPIOSBASE_COMMON_H_
#define _CMPIOSBASE_COMMON_H_


#include "cmpidt.h"

/* Additional defines */
#define ADD_TO_LIST 0
#define FREE_LIST 1
#define FREE_LIST_AND_KEYS 2

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------*/

extern char * CSCreationClassName;
extern char * OSCreationClassName;

extern unsigned char CMPI_true;
extern unsigned char CMPI_false;

/* ---------------------------------------------------------------------------*/

void _check_system_key_value_pairs( const CMPIBroker * _broker,
				    const CMPIObjectPath * cop,
				    const char * creationClassName,
				    const char * className,
				    CMPIStatus * rc );


int _assoc_create_inst_1toN( const CMPIBroker * _broker,
			     const CMPIContext * ctx,
			     const CMPIResult * rslt,
			     const CMPIObjectPath * cop,
			     const char * _ClassName,
			     const char * _RefLeftClass,
			     const char * _RefRightClass,
			     const char * _RefLeft,
			     const char * _RefRight,
			     int left,
			     int inst,
			     CMPIStatus * rc);


/* - method to create CMPIInstance out of the CMPIObjectPath of an            */
/*   association instance                                                     */
CMPIInstance * _assoc_get_inst( const CMPIBroker * _broker,
				const CMPIContext * ctx,
				const CMPIObjectPath * cop,
				const char * _ClassName,
				const char * _RefLeft,
				const char * _RefRight,
				CMPIStatus * rc );


/* - method to create CMPIInstance(s) / CMPIObjectPath(s) of association      */
int _assoc_create_refs_1toN( const CMPIBroker * _broker,
			     const CMPIContext * ctx,
			     const CMPIResult * rslt,
			     const CMPIObjectPath * ref,
			     const char * _ClassName,
			     const char * _RefLeftClass,
			     const char * _RefRightClass,
			     const char * _RefLeft,
			     const char * _RefRight,
			     int inst,
			     int associators,
			     CMPIStatus * rc);


/* - method to get the name of the target class                               */
const char * _assoc_targetClass_Name( const CMPIBroker * _broker,
				      const CMPIObjectPath * ref,
				      const char * _RefLeftClass,
				      const char * _RefRightClass,
				      CMPIStatus * rc);


/* - method to get an empty CMPIObjectPath of the target class                */
CMPIObjectPath * _assoc_targetClass_OP( const CMPIBroker * _broker,
					const CMPIObjectPath * ref,
					const char * _RefLeftClass,
					const char * _RefRightClass,
					CMPIStatus * rc );


/* - method to check the input parameter resultClass, role and resultRole     */
/*   submitted to the methods of the association interface                    */
int _assoc_check_parameter_const( const CMPIBroker * _broker,
				  const CMPIObjectPath * cop,
				  const char * _RefLeft,
				  const char * _RefRight,
				  const char * _RefLeftClass,
				  const char * _RefRightClass,
				  const char * resultClass,
				  const char * role,
				  const char * resultRole,
				  CMPIStatus * rc );


/* ---------------------------------------------------------------------------*/
/*                       end of cmpiOSBase_Common.h                           */
/* ---------------------------------------------------------------------------*/

#ifdef __cplusplus
   }
#endif

/* ---------------------------------------------------------------------------*/

#endif
