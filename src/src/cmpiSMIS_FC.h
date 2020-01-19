/*
 * $Id: cmpiSMIS_FC.h,v 1.1.1.1 2009/05/12 21:46:33 nsharoff Exp $
 *
 * (C) Copyright IBM Corp. 2006, 2007, 2009
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
 *      Definitions and Methods shared among SMIS providers.
*/

#ifndef _CMPI_FC_H_
#define _CMPI_FC_H_



/* ---------------------------------------------------------------------------*/

#include "cmpidt.h"
#include "Linux_CommonHBA.h"


/* ---------------------------------------------------------------------------*/
/*  Definitions and Methods to be shared among the SMIS providers             */
/*     Not for private functions or definitions between two halves of the     */
/*     same provider, ie: cmpiSMIS_FCPortProvider.c and cmpiSMIS_FCPort.c     */
/* ---------------------------------------------------------------------------*/



/*             FCPort                                                         */
/* ---------------------------------------------------------------------------*/
CMPIObjectPath * _makePath_FCPort( const CMPIBroker * _broker,
                 const CMPIContext * ctx, 
                 const CMPIObjectPath * cop,
                 const struct cim_hbaPort * sptr,
                 CMPIStatus * rc);

CMPIInstance * _makeInst_FCPort( const CMPIBroker * _broker,
               const CMPIContext * ctx, 
               const CMPIObjectPath * cop,
               const struct cim_hbaPort * sptr,
               CMPIStatus * rc);




/*             FCSoftwareIdentity                                             */
/* ---------------------------------------------------------------------------*/

/* Define two software identity types */
/* The values correspond to the CIM_SoftwareIdentity Classifications */
/*   attribute values */
#define DRIVER 2
#define FIRMWARE 10


/*             FCSoftwareIdentity_Driver      */
/* ---------------------------------------------------------------------------*/


CMPIObjectPath * _makePath_FCSoftwareIdentity_Driver( const CMPIBroker * _broker,
                 const CMPIContext * ctx, 
                 const CMPIObjectPath * cop,
                 const struct cim_hbaAdapter * sptr,
                 CMPIStatus * rc);

CMPIInstance * _makeInst_FCSoftwareIdentity_Driver( const CMPIBroker * _broker,
               const CMPIContext * ctx, 
               const CMPIObjectPath * cop,
               const struct cim_hbaAdapter * sptr,
               CMPIStatus * rc);

char * _makeKey_FCSoftwareIdentity_Driver( const struct cim_hbaAdapter * sptr);

int _makePath_FCSoftwareIdentity_DriverList( const CMPIBroker * _broker,
                 const CMPIContext * ctx, 
                 const CMPIResult * rslt,
                 const CMPIObjectPath * ref,
                 const struct hbaAdapterList * lptr,
                 CMPIStatus * rc);

int _makeInst_FCSoftwareIdentity_DriverList( const CMPIBroker * _broker,
               const CMPIContext * ctx, 
               const CMPIResult * rslt,
               const CMPIObjectPath * ref,
               const struct hbaAdapterList * lptr,
               CMPIStatus * rc);
               


/*             FCSoftwareIdentity_Firmware      */
/* ---------------------------------------------------------------------------*/


CMPIObjectPath * _makePath_FCSoftwareIdentity_Firmware( const CMPIBroker * _broker,
                 const CMPIContext * ctx, 
                 const CMPIObjectPath * cop,
                 const struct cim_hbaAdapter * sptr,
                 CMPIStatus * rc);

CMPIInstance * _makeInst_FCSoftwareIdentity_Firmware( const CMPIBroker * _broker,
               const CMPIContext * ctx, 
               const CMPIObjectPath * cop,
               const struct cim_hbaAdapter * sptr,
               CMPIStatus * rc);

char * _makeKey_FCSoftwareIdentity_Firmware( const struct cim_hbaAdapter * sptr);

int _makePath_FCSoftwareIdentity_FirmwareList( const CMPIBroker * _broker,
                 const CMPIContext * ctx, 
                 const CMPIResult * rslt,
                 const CMPIObjectPath * ref,
                 const struct hbaAdapterList * lptr,
                 CMPIStatus * rc);

int _makeInst_FCSoftwareIdentity_FirmwareList( const CMPIBroker * _broker,
               const CMPIContext * ctx, 
               const CMPIResult * rslt,
               const CMPIObjectPath * ref,
               const struct hbaAdapterList * lptr,
               CMPIStatus * rc);
               
               

/*             FCPortStatistics                                               */
/* ---------------------------------------------------------------------------*/
CMPIObjectPath * _makePath_FCPortStatistics( const CMPIBroker * _broker,
                 const CMPIContext * ctx, 
                 const CMPIObjectPath * cop,
                 const struct cim_hbaPort * sptr,
                 CMPIStatus * rc);

CMPIInstance * _makeInst_FCPortStatistics( const CMPIBroker * _broker,
               const CMPIContext * ctx, 
               const CMPIObjectPath * cop,
              const struct cim_hbaPort * sptr,
               CMPIStatus * rc);




/*             FCProduct                                                      */
/* ---------------------------------------------------------------------------*/
CMPIObjectPath * _makePath_FCProduct( const CMPIBroker * _broker,
                 const CMPIContext * ctx, 
                 const CMPIObjectPath * cop,
                 const struct cim_hbaAdapter * sptr,
                 CMPIStatus * rc);

CMPIInstance * _makeInst_FCProduct( const CMPIBroker * _broker,
               const CMPIContext * ctx, 
               const CMPIObjectPath * cop,
               const struct cim_hbaAdapter * sptr,
               CMPIStatus * rc);

char* _makeKey_FCProduct(const struct cim_hbaAdapter *sptr);

int _makePath_FCProductList( const CMPIBroker * _broker,
                 const CMPIContext * ctx, 
                 const CMPIResult * rslt,
                 const CMPIObjectPath * cop,
                 const struct hbaAdapterList * lptr,
                 CMPIStatus * rc);

int _makeInst_FCProductList( const CMPIBroker * _broker,
               const CMPIContext * ctx, 
               const CMPIResult * rslt,
               const CMPIObjectPath * cop,
               const struct hbaAdapterList * lptr,
               CMPIStatus * rc);


/*             FCPortController                                               */
/* ---------------------------------------------------------------------------*/
CMPIObjectPath * _makePath_FCPortController( const CMPIBroker * _broker,
                 const CMPIContext * ctx, 
                 const CMPIObjectPath * cop,
                 const struct cim_hbaPort * sptr,
                 CMPIStatus * rc);

CMPIInstance * _makeInst_FCPortController( const CMPIBroker * _broker,
               const CMPIContext * ctx, 
               const CMPIObjectPath * cop,
               const struct cim_hbaPort * sptr,
               CMPIStatus * rc);



               
/*             FCCard                                                         */
/* ---------------------------------------------------------------------------*/
CMPIObjectPath * _makePath_FCCard( const CMPIBroker * _broker,
                 const CMPIContext * ctx, 
                 const CMPIObjectPath * cop,
                 const struct cim_hbaAdapter * sptr,
                 CMPIStatus * rc);

CMPIInstance * _makeInst_FCCard( const CMPIBroker * _broker,
               const CMPIContext * ctx, 
               const CMPIObjectPath * cop,
               const struct cim_hbaAdapter * sptr,
               CMPIStatus * rc);

char* _makeKey_FCCard(const struct cim_hbaAdapter* adapterPtr);


/*             FCSCSIProtocolEndpoint                                         */
/* ---------------------------------------------------------------------------*/
CMPIObjectPath * _makePath_FCSCSIProtocolEndpoint( const CMPIBroker * _broker,
                 const CMPIContext * ctx, 
                 const CMPIObjectPath * cop,
                 const struct cim_hbaPort * sptr,
                 CMPIStatus * rc);

CMPIInstance * _makeInst_FCSCSIProtocolEndpoint( const CMPIBroker * _broker,
               const CMPIContext * ctx, 
               const CMPIObjectPath * cop,
               const struct cim_hbaPort * sptr,
               CMPIStatus * rc);

/*             FCSoftwareIdentity                                             */
/* ---------------------------------------------------------------------------*/
CMPIObjectPath * _makePath_FCSoftwareIdentity( const CMPIBroker * _broker,
                  const CMPIContext * ctx,
                  const CMPIObjectPath * ref,
                  const struct cim_hbaAdapter * sptr,
                  int identityType,
                  CMPIStatus * rc);

CMPIInstance * _makeInst_FCSoftwareIdentity( const CMPIBroker * _broker,
                const CMPIContext * ctx,
                const CMPIObjectPath * ref,
                const struct cim_hbaAdapter * sptr,
                int identityType,
                CMPIStatus * rc);

int _makePath_FCSoftwareIdentityList( const CMPIBroker * _broker,
                 const CMPIContext * ctx, 
                 const CMPIResult * rslt,
                 const CMPIObjectPath * ref,
                 const struct hbaAdapterList * lptr,
                 CMPIStatus * rc);

int _makeInst_FCSoftwareIdentityList( const CMPIBroker * _broker,
               const CMPIContext * ctx, 
               const CMPIResult * rslt,
               const CMPIObjectPath * ref,
               const struct hbaAdapterList * lptr,
               CMPIStatus * rc);
/* ---------------------------------------------------------------------------*/

/*             FCLogicalDisk                                                  */
/* ---------------------------------------------------------------------------*/
CMPIObjectPath * _makePath_FCLogicalDisk( const CMPIBroker * _broker,
                 const CMPIContext * ctx,
                 const CMPIObjectPath * cop,
                 CMPIStatus * rc,
                 struct cim_hbaLogicalDisk * sptr 
                 );

CMPIInstance * _makeInst_FCLogicalDisk( const CMPIBroker * _broker,
               const CMPIContext * ctx,
               const CMPIObjectPath * cop,
               CMPIStatus * rc,
               struct cim_hbaLogicalDisk * sptr
               );


#endif

