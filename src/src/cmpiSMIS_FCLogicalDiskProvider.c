/*
 * $Id: cmpiSMIS_FCLogicalDiskProvider.c,v 1.1.1.1 2009/05/12 21:46:32 nsharoff Exp $
 *
 * (C) Copyright IBM Corp. 2006, 2009
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
 *      Fibre Channel Logic Disk provider
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

static char * _ClassName = "Linux_FCLogicalDisk";


/* ---------------------------------------------------------------------------*/
/*                      Instance Provider Interface                           */
/* ---------------------------------------------------------------------------*/


/******************************************************************************/
/* Function: SMIS_FCLogicalDiskProviderCleanup                                */
/******************************************************************************/
CMPIStatus SMIS_FCLogicalDiskProviderCleanup( CMPIInstanceMI * mi,
            const CMPIContext * ctx, CMPIBoolean trm) 
{
   _OSBASE_TRACE(1,("--- %s CMPI Cleanup() called",_ClassName));
   _OSBASE_TRACE(1,("--- %s CMPI Cleanup() exited",_ClassName));
   CMReturn(CMPI_RC_OK);
}



/******************************************************************************/
/* Function: SMIS_FCLogicalDiskProviderEnumInstanceNames                      */
/******************************************************************************/
CMPIStatus SMIS_FCLogicalDiskProviderEnumInstanceNames( CMPIInstanceMI * mi,
            const CMPIContext * ctx,
            const CMPIResult * rslt,
            const CMPIObjectPath * ref) 
{
   CMPIObjectPath     * op    = NULL;
   CMPIStatus           rc    = {CMPI_RC_OK, NULL};
   struct hbaLogicalDiskList  * dptr = NULL;
   struct hbaPortList         * lptr = NULL;
   struct hbaLogicalDiskList  * rm = NULL;
   
   _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() called",_ClassName));

   if( enum_all_hbaLogicalDisks(&lptr, FALSE, &dptr ) != 0 ) 
      {
      CMSetStatusWithChars( _broker, &rc,
                            CMPI_RC_ERR_FAILED, "Could not list hba logical disks." );
      _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      if (dptr) free_hbaLogicalDiskList(dptr);
      dptr = NULL;
      if (lptr) free_hbaPortList(lptr);
      lptr = NULL;
      return rc;
      }

   // iterate LogicalDisk list
   rm = dptr;
   if( dptr != NULL ) 
   {
      for ( ; dptr && rc.rc == CMPI_RC_OK ; dptr = dptr->next) 
      { 
         op = _makePath_FCLogicalDisk( _broker, ctx, ref, &rc , dptr->sptr);
         if( op == NULL || rc.rc != CMPI_RC_OK ) 
         {
            if( rc.msg != NULL ) 
            {
               _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
            }
            CMSetStatusWithChars( _broker, &rc,
                               CMPI_RC_ERR_FAILED, "Transformation from internal structure to CIM ObjectPath failed." );
            if(rm) 
            {
               free_hbaLogicalDiskList(rm);
            }
            _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
            return rc;
            }
         else 
         { 
            CMReturnObjectPath( rslt, op ); 
         }
      }
      if(rm)
      {
         free_hbaLogicalDiskList(rm);
         rm = NULL;
      }
   } 
   CMReturnDone( rslt );
   _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() exited",_ClassName));
   return rc;
} 
/* end SMIS_FCLogicalDiskProviderEnumInstanceNames */

/******************************************************************************/
/* Function: SMIS_FCLogicalDiskProviderEnumInstances                          */
/******************************************************************************/
CMPIStatus SMIS_FCLogicalDiskProviderEnumInstances( CMPIInstanceMI * mi,
            const CMPIContext * ctx,
            const CMPIResult * rslt,
            const CMPIObjectPath * ref,
            const char ** properties) 
{
   CMPIInstance                * ci    = NULL;
   CMPIStatus                  rc    = {CMPI_RC_OK, NULL};
   struct hbaPortList          * lptr  = NULL ;
   int                         cmdrc = 0;
   struct hbaLogicalDiskList   * dptr = NULL;
   struct hbaLogicalDiskList   * rm = NULL;

   _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() called",_ClassName));
   
   cmdrc = enum_all_hbaLogicalDisks( &lptr, FALSE, &dptr );

   if( cmdrc != 0 ) 
   {
      CMSetStatusWithChars( _broker, &rc,
                           CMPI_RC_ERR_FAILED, "Could not list hba logical disks." );
      _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      if (dptr) free_hbaLogicalDiskList(dptr);
      if (lptr) free_hbaPortList(lptr);
      return rc;
   }
   
   trace_LogicalDisks(&dptr);
   
   rm = dptr;   
   // iterate LogicalDisk list
   if( dptr != NULL ) 
   {
      for ( ; dptr && rc.rc == CMPI_RC_OK ; dptr = dptr->next) 
      {
      _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() calling makeInst : dptr = %a - sptr = %a",dptr, dptr->sptr));
      _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() calling makeInst : dptr = %lx - sptr = %lx",dptr, dptr->sptr));
         /* alice */
         if (dptr != NULL)
         {

         ci = _makeInst_FCLogicalDisk( _broker, ctx, ref, &rc, dptr->sptr);
         if( ci == NULL || rc.rc != CMPI_RC_OK ) 
         {
            if( rc.msg != NULL ) 
            {
               _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
            }
            CMSetStatusWithChars( _broker, &rc,
                                 CMPI_RC_ERR_FAILED, "Transformation from internal structure to CIM Instance failed." );
            if(rm)
            {
                free_hbaLogicalDiskList(rm);
            }
            _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
            return rc;
            }
         else 
	 { 
	    CMReturnInstance( rslt, ci ); 
	 }
         /* alice */
         }
         else 
         {
             _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() NOT calling makeInst : dptr = %x - sptr = %x",dptr, dptr->sptr));
         }
      }

      if (rm)
      {
          free_hbaLogicalDiskList(rm);
          rm = NULL;
      }
   }
   CMReturnDone( rslt );
   _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() exited",_ClassName));
   return rc;
} 
/* end SMIS_FCLogicalDiskProviderEnumInstances */

/******************************************************************************/
/* Function: SMIS_FCLogicalDiskProviderGetInstance                            */
/******************************************************************************/
CMPIStatus SMIS_FCLogicalDiskProviderGetInstance( CMPIInstanceMI * mi,
            const CMPIContext * ctx,
            const CMPIResult * rslt,
            const CMPIObjectPath * cop,
            const char ** properties) 
{
   CMPIInstance       * ci    = NULL;
   char                 *id= NULL;
   int                  stringLength;
   CMPIStatus           rc    = {CMPI_RC_OK, NULL};
   int                  cmdrc = 0;
   struct hbaPortList * lptr  = NULL ;
   struct cim_hbaLogicalDisk * sptr = NULL;
  
   _OSBASE_TRACE(1,("--- %s CMPI GetInstance() called",_ClassName));
 
   _check_system_key_value_pairs( _broker, cop, "SystemCreationClassName", "SystemName", &rc );
   if( rc.rc != CMPI_RC_OK ) 
   {
      _OSBASE_TRACE(1,("--- %s CMPI GetInstance() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      return rc;
   }

   id = strdup(CMGetCharPtr(CMGetKey( cop, "DeviceID", &rc).value.string));

   //stringLength=strlen(CMGetCharPtr(CMGetKey( cop, "DeviceID", &rc).value.string)) + 2; /* 1 for null, 1 for - */
   //id=malloc(stringLength);
 
   //snprintf(id,stringLength,"%s",CMGetCharPtr(CMGetKey( cop, "DeviceID", &rc).value.string));
   //id[stringLength-1]=NULL;
 
   if( id == NULL ) 
   {
      CMSetStatusWithChars( _broker, &rc,
                           CMPI_RC_ERR_FAILED, "Could not get FCLogicalDiskID." );
      _OSBASE_TRACE(1,("--- %s CMPI GetInstance() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      return rc;
   }

   cmdrc = get_hbaLogicalDisk_data( id , FALSE, &sptr);
   free(id);
   
   if( cmdrc != 0 || sptr == NULL) 
   {
      CMSetStatusWithChars( _broker, &rc,
                           CMPI_RC_ERR_NOT_FOUND, "FCLogicalDisk does not exist." );
      _OSBASE_TRACE(1,("--- %s CMPI GetInstance() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      return rc;
   }

   ci = _makeInst_FCLogicalDisk( _broker, ctx, cop, &rc, sptr);
   if (sptr) free_hbaLogicalDisk (sptr);

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
   if(lptr)
   {
      free_hbaPortList(lptr);
   }

   CMReturnInstance( rslt, ci );
   CMReturnDone(rslt);
   _OSBASE_TRACE(1,("--- %s CMPI GetInstance() exited",_ClassName));
   return rc;
} 
/* end SMIS_FCLogicalDiskProviderGetInstance */



/******************************************************************************/
/* Function: SMIS_FCLogicalDiskProviderCreateInstance                         */
/* Notes: This method is only here as a stub in case we later support some    */
/*        methods and not others, and to catch a call to this method in case  */
/*        it is not set to unsupported (or no methods supported) elsewhere.   */
/******************************************************************************/
CMPIStatus SMIS_FCLogicalDiskProviderCreateInstance( CMPIInstanceMI * mi,
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
/* Function: SMIS_FCLogicalDiskProviderModifyInstance                         */
/* Notes: This method is only here as a stub in case we later support some    */
/*        methods and not others, and to catch a call to this method in case  */
/*        it is not set to unsupported (or no methods supported) elsewhere.   */
/******************************************************************************/
CMPIStatus SMIS_FCLogicalDiskProviderModifyInstance( CMPIInstanceMI * mi,
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
/* Function: SMIS_FCLogicalDiskProviderDeleteInstance                         */
/* Notes: This method is only here as a stub in case we later support some    */
/*        methods and not others, and to catch a call to this method in case  */
/*        it is not set to unsupported (or no methods supported) elsewhere.   */
/******************************************************************************/
CMPIStatus SMIS_FCLogicalDiskProviderDeleteInstance( CMPIInstanceMI * mi,
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
/* Function: SMIS_FCLogicalDiskProviderExecQuery                              */
/* Notes: This method is only here as a stub in case we later support some    */
/*        methods and not others, and to catch a call to this method in case  */
/*        it is not set to unsupported (or no methods supported) elsewhere.   */
/******************************************************************************/
CMPIStatus SMIS_FCLogicalDiskProviderExecQuery( CMPIInstanceMI * mi,
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
/* Function: SMIS_FCLogicalDiskProviderMethodCleanup                          */
/******************************************************************************/
CMPIStatus SMIS_FCLogicalDiskProviderMethodCleanup( CMPIMethodMI * mi,
            const CMPIContext * ctx, CMPIBoolean trm) 
   {
   _OSBASE_TRACE(1,("--- %s CMPI MethodCleanup() called",_ClassName));
   _OSBASE_TRACE(1,("--- %s CMPI MethodCleanup() exited",_ClassName));
   CMReturn(CMPI_RC_OK);
   }





/******************************************************************************/
/* Function: SMIS_FCLogicalDiskProviderInvokeMethod                           */
/******************************************************************************/
CMPIStatus SMIS_FCLogicalDiskProviderInvokeMethod( CMPIMethodMI * mi,
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
   else if( strcasecmp(CMGetCharPtr(Class), _ClassName) == 0 &&
       strcasecmp("RequestStateChange",methodName) == 0 ) 
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
   } /* end SMIS_FCLogicalDiskProviderInvokeMethod */






/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/

CMInstanceMIStub( SMIS_FCLogicalDiskProvider,
                  SMIS_FCLogicalDiskProvider,
                  _broker,
                  CMNoHook)

CMMethodMIStub( SMIS_FCLogicalDiskProvider,
                SMIS_FCLogicalDiskProvider,
                _broker,
                CMNoHook)


/* ---------------------------------------------------------------------------*/
/*                end of cmpiSMIS_FCLogicalDiskProvider                       */
/* ---------------------------------------------------------------------------*/
