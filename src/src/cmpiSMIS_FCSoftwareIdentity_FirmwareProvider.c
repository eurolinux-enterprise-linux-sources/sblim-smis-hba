/*
 * $Id: cmpiSMIS_FCSoftwareIdentity_FirmwareProvider.c,v 1.1.1.1 2009/05/12 21:46:33 nsharoff Exp $
 *
 * (C) Copyright IBM Corp. 2007, 2009
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
 *      Provider for FC software identity firmware.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cmpidt.h"
#include "cmpift.h"
#include "cmpimacs.h"
#include "Linux_Common.h"
#include "cmpiLinux_Common.h"
#include "cmpiSMIS_FC.h"

static const CMPIBroker * _broker;

static char * _ClassName = "Linux_FCSoftwareIdentity_Firmware";

/* Defined in cmpiSMIS_FCSoftwareIdentity_Firmware.c */
char * _makeKey_FCSoftwareIdentity_Firmware( const struct cim_hbaAdapter * sptr);



/* ---------------------------------------------------------------------------*/
/* private functions                                                          */
/* ---------------------------------------------------------------------------*/

int _get_hbaAdapter_data_byFirmwareSoftwareIdentityKey(char *InstanceID, struct cim_hbaAdapter ** sptr)
   {
   struct hbaAdapterList * lptr = NULL;
   struct hbaAdapterList * lhlp = NULL;
   struct cim_hbaAdapter * hbaadapter = NULL;
   int                     rc   = 0;
   char                  * temp_instanceID;       /* to hold each adapter InstanceID */
  
   _OSBASE_TRACE(3,("--- _get_hbaAdapter_data_byFirmwareSoftwareIdentityKey() called with InstanceID=%s",InstanceID));

   rc = enum_all_hbaAdapters( &lptr ) ;
   
   /* Identify InstanceID */
   if (strstr(InstanceID,"firmware") != NULL)
      {
      _OSBASE_TRACE(4,("         identityType or InstanceID=FIRMWARE"));
      }
   else
      {
       _OSBASE_TRACE(3,("--- _get_hbaAdapter_data_byFirmwareSoftwareIdentityKey() failed:not Firmware InstanceID"));
   	return -1;
      }
   

   /* Find adapter */
   if( rc == 0 && lptr != NULL ) 
      {
	      lhlp = lptr;
	      for ( ; lptr ; lptr = lptr->next)
	      {
	         hbaadapter = lptr->sptr;
	    
	             /* Create instanceID string */
	         temp_instanceID= _makeKey_FCSoftwareIdentity_Firmware(hbaadapter);
		    
		    /* The cim objects strip leading and trailing spaces, so need to do */
		    /*    the same to our created temp_instanceID before the compare. */
		 stripLeadingTrailingSpaces(temp_instanceID);
		 _OSBASE_TRACE(4,("         temp_instanceID=%s",temp_instanceID));
		
		   /* Do the comparison */
		if( strcmp(temp_instanceID, InstanceID) == 0)
	     	{
	            *sptr = hbaadapter;
	            hbaadapter = NULL ;
	            _OSBASE_TRACE(4,("         Match - InstanceID=|%s| , temp_instanceID=|%s|",InstanceID,temp_instanceID));
		    free(temp_instanceID);
	    	    break;
	    	}
		else
	       {
	            _OSBASE_TRACE(4,("         no match - InstanceID=|%s| , temp_instanceID=|%s|",InstanceID,temp_instanceID));
	       }  
		free(temp_instanceID);
	     } /* end for */
	
	      /* Free the rest of the adapters */
	      lptr = lhlp;
	      for ( ; lptr ; )
	      {
	         hbaadapter = lptr->sptr;
	         if( hbaadapter != *sptr ) 
	       	{
	            free_hbaAdapter(hbaadapter);
	        }
	         lhlp = lptr;
	         lptr = lptr->next;
	         free(lhlp);
	      }
	      _OSBASE_TRACE(3,("--- _get_hbaAdapter_data_byFirmwareSoftwareIdentityKey() exited"));
	      return 0;
      }
   _OSBASE_TRACE(3,("--- _get_hbaAdapter_data_byFirmwareSoftwareIdentityKey() failed"));
   return -1;
   } /* end _get_hbaAdapter_data_byFirmwareSoftwareIdentityKey */



/* ---------------------------------------------------------------------------*/
/* end private functions                                                      */
/* ---------------------------------------------------------------------------*/








/* ---------------------------------------------------------------------------*/
/*                      Instance Provider Interface                           */
/* ---------------------------------------------------------------------------*/


/******************************************************************************/
/* Function: SMIS_FCSoftwareIdentity_FirmwareProviderCleanup                           */
/******************************************************************************/
CMPIStatus SMIS_FCSoftwareIdentity_FirmwareProviderCleanup( CMPIInstanceMI * mi,
            const CMPIContext * ctx, CMPIBoolean trm) 
   {
   _OSBASE_TRACE(1,("--- %s CMPI Cleanup() called",_ClassName));
   _OSBASE_TRACE(1,("--- %s CMPI Cleanup() exited",_ClassName));
   CMReturn(CMPI_RC_OK);
   }



/******************************************************************************/
/* Function: SMIS_FCSoftwareIdentity_FirmwareProviderEnumInstanceNames                 */
/******************************************************************************/
CMPIStatus SMIS_FCSoftwareIdentity_FirmwareProviderEnumInstanceNames( CMPIInstanceMI * mi,
            const CMPIContext * ctx,
            const CMPIResult * rslt,
            const CMPIObjectPath * ref) 
{
   CMPIStatus           rc    = {CMPI_RC_OK, NULL};
   struct hbaAdapterList * lptr  = NULL ;

   _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() called",_ClassName));

   if( enum_all_hbaAdapters( &lptr ) != 0 ) 
      {
      CMSetStatusWithChars( _broker, &rc,
                            CMPI_RC_ERR_FAILED, "Could not list hba adapters." );
      _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      return rc;
      }

   // iterate port list
   if( lptr != NULL ) {

      /* cerate object paths for the list */
      _makePath_FCSoftwareIdentity_FirmwareList( _broker, ctx, rslt, ref, lptr, &rc);

      /* release adapters list */   
      free_hbaAdapterList(lptr);
   }
   if ( rc.rc == CMPI_RC_OK )
      CMReturnDone( rslt );
   
   _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() exited",_ClassName));
   return rc;
} /* end SMIS_FCSoftwareIdentity_FirmwareProviderEnumInstanceNames */
   
   
   

/******************************************************************************/
/* Function: SMIS_FCSoftwareIdentity_FirmwareProviderEnumInstances                     */
/******************************************************************************/
CMPIStatus SMIS_FCSoftwareIdentity_FirmwareProviderEnumInstances( CMPIInstanceMI * mi,
            const CMPIContext * ctx,
            const CMPIResult * rslt,
            const CMPIObjectPath * ref,
            const char ** properties) 
{
   CMPIStatus           rc    = {CMPI_RC_OK, NULL};
   struct hbaAdapterList * lptr  = NULL ;

   _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() called",_ClassName));

   if( enum_all_hbaAdapters( &lptr ) != 0 )  {
      CMSetStatusWithChars( _broker, &rc,
                           CMPI_RC_ERR_FAILED, "Could not list hba adapters." );
      _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      return rc;
   }

   // iterate port list
   if( lptr != NULL ) {

      /* cerate object paths for the list */
      _makeInst_FCSoftwareIdentity_FirmwareList( _broker, ctx, rslt, ref, lptr, &rc);

      /* release adapters list */   
      free_hbaAdapterList(lptr);
   }
   
   if ( rc.rc == CMPI_RC_OK )
      CMReturnDone( rslt );

   _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() exited",_ClassName));
   return rc;
} /* end SMIS_FCSoftwareIdentity_FirmwareProviderEnumInstances */





/******************************************************************************/
/* Function: SMIS_FCSoftwareIdentity_FirmwareProviderGetInstance                       */
/******************************************************************************/
CMPIStatus SMIS_FCSoftwareIdentity_FirmwareProviderGetInstance( CMPIInstanceMI * mi,
            const CMPIContext * ctx,
            const CMPIResult * rslt,
            const CMPIObjectPath * cop,
            const char ** properties) 
   {
   CMPIInstance       * ci    = NULL;
   CMPIString         * id    = NULL;
   struct cim_hbaAdapter * sptr  = NULL;
   CMPIStatus           rc    = {CMPI_RC_OK, NULL};
   int                  cmdrc = 0;

   _OSBASE_TRACE(1,("--- %s CMPI GetInstance() called",_ClassName));

   _check_system_key_value_pairs( _broker, cop, "SystemCreationClassName", "SystemName", &rc );
   if( rc.rc != CMPI_RC_OK ) 
      {
      _OSBASE_TRACE(1,("--- %s CMPI GetInstance() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      return rc;
      }

   id = CMGetKey( cop, "InstanceID", &rc).value.string;
 
   if( id == NULL ) 
      {
      CMSetStatusWithChars( _broker, &rc,
                           CMPI_RC_ERR_FAILED, "Could not get FCSoftwareIdentityID." );
      _OSBASE_TRACE(1,("--- %s CMPI GetInstance() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      return rc;
      }

   cmdrc = _get_hbaAdapter_data_byFirmwareSoftwareIdentityKey( CMGetCharPtr(id) , &sptr );
   
   if( cmdrc != 0 || sptr == NULL ) 
      {
      CMSetStatusWithChars( _broker, &rc,
                           CMPI_RC_ERR_NOT_FOUND, "FCSoftwareIdentity does not exist." );
      _OSBASE_TRACE(1,("--- %s CMPI GetInstance() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      return rc;
      }

   /* Create firmware instance based on InstanceID */
   if (strstr(CMGetCharPtr(id),"firmware") != NULL){
      ci = _makeInst_FCSoftwareIdentity_Firmware( _broker, ctx, cop, sptr, &rc );
   }
   else   {
   	CMSetStatusWithChars( _broker, &rc,
                            CMPI_RC_ERR_FAILED, "Not Match Firmware Object Path." );
      _OSBASE_TRACE(1,("--- %s CMPI GetInstance() failed :%s",_ClassName,CMGetCharPtr(rc.msg)));
      return rc;
    }
   
   if(sptr) free_hbaAdapter(sptr);

   if( ci == NULL ) 
      {
      if( rc.msg != NULL ) 
         {
         _OSBASE_TRACE(1,("--- %s CMPI GetInstance() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
         }
      else 
         {
         _OSBASE_TRACE(1,("--- %s CMPI GetInstance() failed",_ClassName));
         }
      return rc;
      }

   /* Don't need to call isDuplicateKey here since only returning one instance. */
   CMReturnInstance( rslt, ci );
   CMReturnDone(rslt);
   _OSBASE_TRACE(1,("--- %s CMPI GetInstance() exited",_ClassName));
   return rc;
   } /* end SMIS_FCSoftwareIdentity_FirmwareProviderGetInstance */



/******************************************************************************/
/* Function: SMIS_FCSoftwareIdentity_FirmwareProviderCreateInstance                    */
/* Notes: This method is only here as a stub in case we later support some    */
/*        methods and not others, and to catch a call to this method in case  */
/*        it is not set to unsupported (or no methods supported) elsewhere.   */
/******************************************************************************/
CMPIStatus SMIS_FCSoftwareIdentity_FirmwareProviderCreateInstance( CMPIInstanceMI * mi,
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
/* Function: SMIS_FCSoftwareIdentity_FirmwareProviderModifyInstance                    */
/* Notes: This method is only here as a stub in case we later support some    */
/*        methods and not others, and to catch a call to this method in case  */
/*        it is not set to unsupported (or no methods supported) elsewhere.   */
/******************************************************************************/
CMPIStatus SMIS_FCSoftwareIdentity_FirmwareProviderModifyInstance( CMPIInstanceMI * mi,
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
/* Function: SMIS_FCSoftwareIdentity_FirmwareProviderDeleteInstance                    */
/* Notes: This method is only here as a stub in case we later support some    */
/*        methods and not others, and to catch a call to this method in case  */
/*        it is not set to unsupported (or no methods supported) elsewhere.   */
/******************************************************************************/
CMPIStatus SMIS_FCSoftwareIdentity_FirmwareProviderDeleteInstance( CMPIInstanceMI * mi,
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
/* Function: SMIS_FCSoftwareIdentity_FirmwareProviderExecQuery                         */
/* Notes: This method is only here as a stub in case we later support some    */
/*        methods and not others, and to catch a call to this method in case  */
/*        it is not set to unsupported (or no methods supported) elsewhere.   */
/******************************************************************************/
CMPIStatus SMIS_FCSoftwareIdentity_FirmwareProviderExecQuery( CMPIInstanceMI * mi,
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
/*                        Method Provider Interface                           */
/* ---------------------------------------------------------------------------*/


/******************************************************************************/
/* Function: SMIS_FCSoftwareIdentity_FirmwareProviderMethodCleanup                     */
/******************************************************************************/
CMPIStatus SMIS_FCSoftwareIdentity_FirmwareProviderMethodCleanup( CMPIMethodMI * mi,
            const CMPIContext * ctx, CMPIBoolean trm) 
   {
   _OSBASE_TRACE(1,("--- %s CMPI MethodCleanup() called",_ClassName));
   _OSBASE_TRACE(1,("--- %s CMPI MethodCleanup() exited",_ClassName));
   CMReturn(CMPI_RC_OK);
   }





/******************************************************************************/
/* Function: SMIS_FCSoftwareIdentity_FirmwareProviderInvokeMethod                      */
/******************************************************************************/
CMPIStatus SMIS_FCSoftwareIdentity_FirmwareProviderInvokeMethod( CMPIMethodMI * mi,
            const CMPIContext * ctx,
            const CMPIResult * rslt,
            const CMPIObjectPath * ref,
            const char * methodName,
            const CMPIArgs * in,
            CMPIArgs * out) 
   {
   CMPIString * Class = NULL;
   CMPIStatus   rc    = {CMPI_RC_OK, NULL};

   _OSBASE_TRACE(1,("--- %s CMPI InvokeMethod() called",_ClassName));

   Class = CMGetClassName(ref, &rc);

   if( strcasecmp(CMGetCharPtr(Class), _ClassName) == 0 &&
       strcasecmp("SetPowerState",methodName) == 0 ) 
      {
      CMSetStatusWithChars( _broker, &rc,
                           CMPI_RC_ERR_NOT_SUPPORTED, methodName );
      }
   else if( strcasecmp(CMGetCharPtr(Class), _ClassName) == 0 &&
       strcasecmp("Reset",methodName) == 0 ) 
      {
      CMSetStatusWithChars( _broker, &rc,
                           CMPI_RC_ERR_NOT_SUPPORTED, methodName );
      }
   else if( strcasecmp(CMGetCharPtr(Class), _ClassName) == 0 &&
       strcasecmp("EnableDevice",methodName) == 0 ) 
      {
      CMSetStatusWithChars( _broker, &rc,
                           CMPI_RC_ERR_NOT_SUPPORTED, methodName );
      }
   else if( strcasecmp(CMGetCharPtr(Class), _ClassName) == 0 &&
       strcasecmp("OnlineDevice",methodName) == 0 ) 
      {
      CMSetStatusWithChars( _broker, &rc,
                           CMPI_RC_ERR_NOT_SUPPORTED, methodName );
      }
   else if( strcasecmp(CMGetCharPtr(Class), _ClassName) == 0 &&
       strcasecmp("QuiesceDevice",methodName) == 0 ) 
      {
      CMSetStatusWithChars( _broker, &rc,
                           CMPI_RC_ERR_NOT_SUPPORTED, methodName );
      }
   else if( strcasecmp(CMGetCharPtr(Class), _ClassName) == 0 &&
       strcasecmp("SaveProperties",methodName) == 0 ) 
      {
      CMSetStatusWithChars( _broker, &rc,
                           CMPI_RC_ERR_NOT_SUPPORTED, methodName );
      }
   else if( strcasecmp(CMGetCharPtr(Class), _ClassName) == 0 &&
       strcasecmp("RestoreProperties",methodName) == 0 ) 
      {
      CMSetStatusWithChars( _broker, &rc,
                           CMPI_RC_ERR_NOT_SUPPORTED, methodName );
      }
   else 
      {
      CMSetStatusWithChars( _broker, &rc,
                           CMPI_RC_ERR_NOT_FOUND, methodName );
      }

   _OSBASE_TRACE(1,("--- %s CMPI InvokeMethod() exited",_ClassName));
   return rc;
   } /* end SMIS_FCSoftwareIdentity_FirmwareProviderInvokeMethod */






/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/

CMInstanceMIStub( SMIS_FCSoftwareIdentity_FirmwareProvider,
                  SMIS_FCSoftwareIdentity_FirmwareProvider,
                  _broker,
                  CMNoHook)

CMMethodMIStub( SMIS_FCSoftwareIdentity_FirmwareProvider,
                SMIS_FCSoftwareIdentity_FirmwareProvider,
                _broker,
                CMNoHook)


/* ---------------------------------------------------------------------------*/
/*                end of cmpiSMIS_FCSoftwareIdentity_FirmwareProvider          */
/* ---------------------------------------------------------------------------*/
