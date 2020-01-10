/*
 * $Id: cmpiSMIS_FCProductProvider.c,v 1.1.1.1 2009/05/12 21:46:32 nsharoff Exp $
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
 *      Provider for Fibre Channel Product.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cmpidt.h"
#include "cmpift.h"
#include "cmpimacs.h"
#include "Linux_Common.h"
#include "Linux_CommonHBA.h"
#include "cmpiLinux_Common.h"
#include "cmpiSMIS_FC.h"

static const CMPIBroker * _broker;

static char * _ClassName = "Linux_FCProduct";

/* ---------------------------------------------------------------------------*/
/*                     Product Key Properties                                 */
/* ---------------------------------------------------------------------------*/
#define FCPROD_KEY_NAME                "Name"
#define FCPROD_KEY_IDENTIFYINGNUMBER   "IdentifyingNumber"
#define FCPROD_KEY_VENDOR              "Vendor"
#define FCPROD_KEY_VERSION             "Version"

/* ---------------------------------------------------------------------------*/
/*                      Private Functions                                     */
/* ---------------------------------------------------------------------------*/

/*
  Get a adapter data according Name, IdentifyingNumber, Vendor, Version
  Corresponding adapter attrs :ModelDescription, SerialNumber, Manufacturer, Model
*/
static int FCProduct_get_hbaAdapter_data(
   char * Name, char * IdentifyingNumber, char * Vendor, char * Version,
   struct cim_hbaAdapter ** sptr)
{
   int adapter_number;
   int numberofadapters;
   HBA_HANDLE handle;
   char *adapter_name;
   HBA_ADAPTERATTRIBUTES *adapter_attributes;
   int rc = 0;

   _OSBASE_TRACE(1, ("--- FCProduct_get_hbaAdapter_data() called"));

   hbamutex_lock();
   _OSBASE_TRACE(3, ("--- HBA_LoadLibary () called."));
   rc = HBA_LoadLibrary();
   _OSBASE_TRACE(3, ("--- HBA_LoadLibrary () rc  = %d", rc));

   if (rc == 0) {

      _OSBASE_TRACE(3, ("--- HBA_GetNumberOfAdapters () called."));
      numberofadapters = HBA_GetNumberOfAdapters();
      _OSBASE_TRACE(3, ("--- HBA_NumberOfAdapters () = %d", numberofadapters));

	  if (numberofadapters > 0)
	  {
		  /* alloc memory for adapter_attributes and adapter_name */
		  adapter_attributes = (HBA_ADAPTERATTRIBUTES *)
			  malloc(sizeof(HBA_ADAPTERATTRIBUTES));
		  adapter_name = (char *) malloc(HBA_MAX_DEV_NAME + 1);

		  for (adapter_number = 0; adapter_number<numberofadapters; adapter_number++) {

			  rc = get_info_for_one_adapter(adapter_number, 
				  adapter_name, 
				  adapter_attributes, 
				  &handle, 
				  1);

			  if ( rc == 0 ) {
				  if ( strcmp(adapter_attributes->SerialNumber, IdentifyingNumber) == 0 &&
					  strcmp(adapter_attributes->Manufacturer, Vendor) == 0 &&
					  strcmp(adapter_attributes->Model, Version) == 0 &&
					  strcmp(adapter_attributes->ModelDescription, Name) == 0 ) {

						  /* we have found the adapter allocate and fill in properties */
						  (*sptr) = (struct cim_hbaAdapter*)calloc (1,sizeof(struct cim_hbaAdapter));
						  (*sptr)->adapter_attributes = adapter_attributes;
						  (*sptr)->adapter_name  = adapter_name;
						  (*sptr)->adapter_number  = adapter_number;

						  break;
				  }
			  }
			  else {
				  /* error occured during get_info_for_one_adapter */
				  _OSBASE_TRACE(2, ("--- get_info_for_one_adapter (%d) rc = %d", 
					  adapter_number, rc) );
			  }
		  } /* end for(adapter_number...) */

		  if ( adapter_number == numberofadapters ) {
			  /* can not find the adapter, release memory  */
			  free(adapter_name);
			  free(adapter_attributes);
			  *sptr = NULL;
		  }

		  _OSBASE_TRACE(3, ("--- HBA_FreeLibrary () called."));
		  rc = HBA_FreeLibrary();
		  _OSBASE_TRACE(3, ("--- HBA_FreeLibrary () rc  = %d", rc));
	  }
  }
  
final_exit:
   hbamutex_unlock();
   _OSBASE_TRACE(1, ("--- FCProduct_get_hbaAdapter_data() exited"));
   return rc;
}



/* ---------------------------------------------------------------------------*/
/*                      Instance Provider Interface                           */
/* ---------------------------------------------------------------------------*/


/******************************************************************************/
/* Function: SMIS_FCProductProviderCleanup                                    */
/******************************************************************************/
CMPIStatus 
SMIS_FCProductProviderCleanup( CMPIInstanceMI * mi,
            const CMPIContext * ctx, CMPIBoolean trm) 
{
   _OSBASE_TRACE(1,("--- %s CMPI Cleanup() called", _ClassName));
   _OSBASE_TRACE(1,("--- %s CMPI Cleanup() exited", _ClassName));
   CMReturn(CMPI_RC_OK);
}



/******************************************************************************/
/* Function: SMIS_FCProductProviderEnumInstanceNames                          */
/******************************************************************************/
CMPIStatus 
SMIS_FCProductProviderEnumInstanceNames( CMPIInstanceMI * mi,
            const CMPIContext * ctx,
            const CMPIResult * rslt,
            const CMPIObjectPath * ref) 
{
   CMPIObjectPath     * op    = NULL;
   CMPIStatus           rc    = {CMPI_RC_OK, NULL};
   struct hbaAdapterList * lptr  = NULL ;
   char *key = NULL;
   void * keyList = NULL;

   _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() called", _ClassName));

   /* get all adapters */
   if( enum_all_hbaAdapters( &lptr ) != 0 ) {
      CMSetStatusWithChars( _broker, &rc,
                            CMPI_RC_ERR_FAILED, "could not list hba adapters." );
      _OSBASE_TRACE(2, ("--- %s CMPI EnumInstanceNames() failed : %s", 
                        _ClassName, CMGetCharPtr(rc.msg)));
      goto final_exit;
   }

   /* iterate adapters list */
   if( lptr != NULL ) {

      /* cerate object paths for the list */
      _makePath_FCProductList( _broker, ctx, rslt, ref, lptr, &rc);

      /* release adapters list */   
      free_hbaAdapterList(lptr);
   }

   if ( rc.rc == CMPI_RC_OK )
      CMReturnDone( rslt );

final_exit:
   _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() exited", _ClassName));
   return rc;
} /* end SMIS_FCProductProviderEnumInstanceNames */



/******************************************************************************/
/* Function: SMIS_FCProductProviderEnumInstances                              */
/******************************************************************************/
CMPIStatus 
SMIS_FCProductProviderEnumInstances( CMPIInstanceMI * mi,
            const CMPIContext * ctx,
            const CMPIResult * rslt,
            const CMPIObjectPath * ref,
            const char ** properties) 
{
   CMPIInstance       * ci    = NULL;
   CMPIStatus           rc    = {CMPI_RC_OK, NULL};
   struct hbaAdapterList * lptr  = NULL ;
   char *key = NULL;
   void * keyList = NULL;
   
   _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() called",_ClassName));

   /* get all adapters */
   if( enum_all_hbaAdapters( &lptr ) != 0 ) {
      CMSetStatusWithChars( _broker, &rc,
                           CMPI_RC_ERR_FAILED, "could not list hba adapters." );
      _OSBASE_TRACE(2,("--- %s CMPI EnumInstances() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      goto final_exit;
   }

   /* iterate adapters list */
   if( lptr != NULL ) {

      /* cerate object paths for the list */
      _makeInst_FCProductList( _broker, ctx, rslt, ref, lptr, &rc);

      /* release adapters list */
      free_hbaAdapterList(lptr);
   }

   if ( rc.rc == CMPI_RC_OK )
      CMReturnDone( rslt );

final_exit:
   _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() exited",_ClassName));
   return rc;
} /* end SMIS_FCProductProviderEnumInstances */



/******************************************************************************/
/* Function: SMIS_FCProductProviderGetInstance                                */
/******************************************************************************/
CMPIStatus 
SMIS_FCProductProviderGetInstance( CMPIInstanceMI * mi,
            const CMPIContext * ctx,
            const CMPIResult * rslt,
            const CMPIObjectPath * cop,
            const char ** properties) 
{
   CMPIInstance * ci = NULL;
   struct cim_hbaAdapter * sptr  = NULL;
   CMPIStatus rc = {CMPI_RC_OK, NULL};
   char* key_name = NULL;                /* ModelDescription */
   char* key_identifyingnumber = NULL;   /* SerialNumber */
   char* key_vendor = NULL;              /* Manufacturer */
   char* key_version = NULL;             /* Model */
   char* key;
   int cmdrc;

   _OSBASE_TRACE(1,("--- %s CMPI GetInstance() called", _ClassName));

   /* get key name from object path */
   key_name = 
      CMGetCharPtr(CMGetKey( cop, FCPROD_KEY_NAME, &rc).value.string);
   _OSBASE_TRACE(3, ("        rc.rc=%d, %s=%s", rc.rc, FCPROD_KEY_NAME, key_name));

   key_identifyingnumber = 
      CMGetCharPtr(CMGetKey( cop, FCPROD_KEY_IDENTIFYINGNUMBER, &rc).value.string);
   _OSBASE_TRACE(3, ("        rc.rc=%d, %s=%s", rc.rc, FCPROD_KEY_IDENTIFYINGNUMBER, key_identifyingnumber));

   key_vendor = 
      CMGetCharPtr(CMGetKey( cop, FCPROD_KEY_VENDOR, &rc).value.string);
   _OSBASE_TRACE(3, ("        rc.rc=%d, %s=%s", rc.rc, FCPROD_KEY_VENDOR, key_vendor));

   key_version = 
      CMGetCharPtr(CMGetKey( cop, FCPROD_KEY_VERSION, &rc).value.string);
   _OSBASE_TRACE(3, ("        rc.rc=%d, %s=%s", rc.rc, FCPROD_KEY_VERSION, key_version));
   
   if ( key_name == NULL || key_identifyingnumber == NULL || 
        key_vendor == NULL || key_version == NULL) {
      /* one of get keys from object path failed */
      CMSetStatusWithChars( _broker, &rc,
                           CMPI_RC_ERR_FAILED, "could not get keys from object path." );
      _OSBASE_TRACE(2, ("--- %s GetInstance() failed : %s", _ClassName, CMGetCharPtr(rc.msg)));
      goto final_exit;
   }
   
   /* get adapter according to the keys */
   cmdrc = FCProduct_get_hbaAdapter_data(key_name, key_identifyingnumber, 
                                      key_vendor, key_version, &sptr );
   if( cmdrc != 0 || sptr == NULL ) {
      CMSetStatusWithChars( _broker, &rc,
                            CMPI_RC_ERR_NOT_FOUND, "FCProduct does not exist." );
      _OSBASE_TRACE(2, ("--- %s CMPI GetInstance() failed : %s",
                        _ClassName, CMGetCharPtr(rc.msg)));
      goto final_exit;
   }
      
   ci = _makeInst_FCProduct( _broker, ctx, cop, sptr, &rc );
   
   if( sptr ) free_hbaAdapter(sptr);
   if( ci == NULL ) {
      if( rc.msg != NULL ) {
         _OSBASE_TRACE(2, ("--- %s CMPI GetInstance() failed : %s",
                           _ClassName, CMGetCharPtr(rc.msg)));
      }
      else {
         _OSBASE_TRACE(2,("--- %s CMPI GetInstance() failed", _ClassName));
      }
      CMSetStatusWithChars( _broker, &rc,
                            CMPI_RC_ERR_FAILED, "Transformation from internal structure to CIM Instance failed." );
      goto final_exit;
   }

   CMReturnInstance( rslt, ci );
   CMReturnDone(rslt);

final_exit:
   _OSBASE_TRACE(1,("--- %s CMPI GetInstance() exited",_ClassName));
   return rc;
} /* end SMIS_FCProductProviderGetInstance */



/******************************************************************************/
/* Function: SMIS_FCProductProviderCreateInstance                             */
/* Notes: This method is only here as a stub in case we later support some    */
/*        methods and not others, and to catch a call to this method in case  */
/*        it is not set to unsupported (or no methods supported) elsewhere.   */
/******************************************************************************/
CMPIStatus SMIS_FCProductProviderCreateInstance( CMPIInstanceMI * mi,
            const CMPIContext * ctx,
            const CMPIResult * rslt,
            const CMPIObjectPath * cop,
            const CMPIInstance * ci) 
{
   CMPIStatus rc = {CMPI_RC_OK, NULL};

   _OSBASE_TRACE(1,("--- %s CMPI CreateInstance() called",_ClassName));

   CMSetStatusWithChars( _broker, &rc,
                         CMPI_RC_ERR_NOT_SUPPORTED, "CIM_ERR_NOT_SUPPORTED" );

   _OSBASE_TRACE(1,("--- %s CMPI CreateInstance() called",_ClassName));
   return rc;
}
   
   

/******************************************************************************/
/* Function: SMIS_FCProductProviderModifyInstance                             */
/* Notes: This method is only here as a stub in case we later support some    */
/*        methods and not others, and to catch a call to this method in case  */
/*        it is not set to unsupported (or no methods supported) elsewhere.   */
/******************************************************************************/
CMPIStatus SMIS_FCProductProviderModifyInstance( CMPIInstanceMI * mi,
            const CMPIContext * ctx,
            const CMPIResult * rslt,
            const CMPIObjectPath * cop,
            const CMPIInstance * ci,
            const char **properties) 
{
   CMPIStatus rc = {CMPI_RC_OK, NULL};

   _OSBASE_TRACE(1,("--- %s CMPI ModifyInstance() called",_ClassName));

   CMSetStatusWithChars( _broker, &rc,
                         CMPI_RC_ERR_NOT_SUPPORTED, "CIM_ERR_NOT_SUPPORTED" );

   _OSBASE_TRACE(1,("--- %s CMPI ModifyInstance() exited",_ClassName));
   return rc;
}



/******************************************************************************/
/* Function: SMIS_FCProductProviderDeleteInstance                             */
/* Notes: This method is only here as a stub in case we later support some    */
/*        methods and not others, and to catch a call to this method in case  */
/*        it is not set to unsupported (or no methods supported) elsewhere.   */
/******************************************************************************/
CMPIStatus SMIS_FCProductProviderDeleteInstance( CMPIInstanceMI * mi,
            const CMPIContext * ctx,
            const CMPIResult * rslt,
            const CMPIObjectPath * cop) 
{
   CMPIStatus rc = {CMPI_RC_OK, NULL};

   _OSBASE_TRACE(1,("--- %s CMPI DeleteInstance() called",_ClassName));

   CMSetStatusWithChars( _broker, &rc,
                         CMPI_RC_ERR_NOT_SUPPORTED, "CIM_ERR_NOT_SUPPORTED" );

   _OSBASE_TRACE(1,("--- %s CMPI DeleteInstance() exited",_ClassName));
   return rc;
}



/******************************************************************************/
/* Function: SMIS_FCProductProviderExecQuery                                  */
/* Notes: This method is only here as a stub in case we later support some    */
/*        methods and not others, and to catch a call to this method in case  */
/*        it is not set to unsupported (or no methods supported) elsewhere.   */
/******************************************************************************/
CMPIStatus SMIS_FCProductProviderExecQuery( CMPIInstanceMI * mi,
            const CMPIContext * ctx,
            const CMPIResult * rslt,
            const CMPIObjectPath * ref,
            const char * lang,
            const char * query) 
{
   CMPIStatus rc = {CMPI_RC_OK, NULL};

   _OSBASE_TRACE(1,("--- %s CMPI ExecQuery() called",_ClassName));

   CMSetStatusWithChars( _broker, &rc,
                         CMPI_RC_ERR_NOT_SUPPORTED, "CIM_ERR_NOT_SUPPORTED" );

   _OSBASE_TRACE(1,("--- %s CMPI ExecQuery() exited",_ClassName));
   return rc;
}



/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/

CMInstanceMIStub( SMIS_FCProductProvider,
                  SMIS_FCProductProvider,
                  _broker,
                  CMNoHook)

/* ---------------------------------------------------------------------------*/
/*                end of cmpiSMIS_FCProductProvider                           */
/* ---------------------------------------------------------------------------*/
