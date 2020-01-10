/*
 * $Id: cmpiOSBase_ComputerSystem.h,v 1.1.1.1 2009/05/12 21:46:33 nsharoff Exp $
 *
 * (C) Copyright IBM Corp. 2002, 2008, 2009
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
 * This file defines the interfaces for the factory implementation of the
 * CIM class Linux_ComputerSystem.
 *
*/

#ifndef _CMPIOSBASE_COMPUTERSYSTEM_H_
#define _CMPIOSBASE_COMPUTERSYSTEM_H_

#define CMPI_VER_100

/* ---------------------------------------------------------------------------*/

#include "cmpidt.h"

/* ---------------------------------------------------------------------------*/

static char * _ClassName = "Linux_ComputerSystem";

/* ---------------------------------------------------------------------------*/


/* method to create a CMPIObjectPath of this class                            */

CMPIObjectPath * _makePath_ComputerSystem( const CMPIBroker * _broker,
                                           const CMPIContext * ctx, 
                                           const CMPIObjectPath * cop,
                                           CMPIStatus * rc);


/* method to create a CMPIInstance of this class                              */

CMPIInstance * _makeInst_ComputerSystem(const CMPIBroker * _broker,
                                        const CMPIContext * ctx, 
                                        const CMPIObjectPath * cop,
                                        const char ** properties,
                                        CMPIStatus * rc);


/* ---------------------------------------------------------------------------*/

#endif

