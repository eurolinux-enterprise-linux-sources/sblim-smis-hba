/*
 * $Id: cmpiSMIS_FCPortProvider.c,v 1.1.1.1 2009/05/12 21:46:33 nsharoff Exp $
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
 *      Provider for FC Port.
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


static char * _ClassName = "Linux_FCPort";


/* ---------------------------------------------------------------------------*/
/*                      Instance Provider Interface                           */
/* ---------------------------------------------------------------------------*/


/******************************************************************************/
/* Function: SMIS_FCPortProviderCleanup                                       */
/******************************************************************************/
CMPIStatus SMIS_FCPortProviderCleanup( CMPIInstanceMI * mi,
            const CMPIContext * ctx, CMPIBoolean trm) 
   {
   _OSBASE_TRACE(1,("--- %s CMPI Cleanup() called",_ClassName));
   _OSBASE_TRACE(1,("--- %s CMPI Cleanup() exited",_ClassName));
   CMReturn(CMPI_RC_OK);
   }



/******************************************************************************/
/* Function: SMIS_FCPortProviderEnumInstanceNames                             */
/******************************************************************************/
CMPIStatus SMIS_FCPortProviderEnumInstanceNames( CMPIInstanceMI * mi,
            const CMPIContext * ctx,
            const CMPIResult * rslt,
            const CMPIObjectPath * ref) 
   {
   CMPIObjectPath     * op    = NULL;
   CMPIStatus           rc    = {CMPI_RC_OK, NULL};
   struct hbaPortList * lptr  = NULL ;
   struct hbaPortList * rm    = NULL ;

   _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() called",_ClassName));

   if( enum_all_hbaPorts( &lptr, FALSE ) != 0 ) 
      {
      CMSetStatusWithChars( _broker, &rc,
                            CMPI_RC_ERR_FAILED, "Could not list hba ports." );
      _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      return rc;
      }

   rm = lptr;
   // iterate port list
   if( lptr != NULL ) 
      {
      for ( ; lptr && rc.rc == CMPI_RC_OK ; lptr = lptr->next) 
         {
         op = _makePath_FCPort( _broker, ctx, ref,lptr->sptr, &rc );
         if( op == NULL || rc.rc != CMPI_RC_OK ) 
	    {
            if( rc.msg != NULL ) 
	       {
              _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
               }
            CMSetStatusWithChars( _broker, &rc,
                               CMPI_RC_ERR_FAILED, "Transformation from internal structure to CIM ObjectPath failed." );
            if(rm) free_hbaPortList(rm);
            _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
            return rc;
            }
         else 
	    { 
	    CMReturnObjectPath( rslt, op ); 
	    }
         }
      if(rm) free_hbaPortList(rm);
      }

   CMReturnDone( rslt );
   _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() exited",_ClassName));
   return rc;
   } /* end SMIS_FCPortProviderEnumInstanceNames */
   
   
   

/******************************************************************************/
/* Function: SMIS_FCPortProviderEnumInstances                                 */
/******************************************************************************/
CMPIStatus SMIS_FCPortProviderEnumInstances( CMPIInstanceMI * mi,
            const CMPIContext * ctx,
            const CMPIResult * rslt,
            const CMPIObjectPath * ref,
            const char ** properties) 
   {
   CMPIInstance       * ci    = NULL;
   CMPIStatus           rc    = {CMPI_RC_OK, NULL};
   struct hbaPortList * lptr  = NULL ;
   struct hbaPortList * rm    = NULL ;
   int                  cmdrc = 0;

   _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() called",_ClassName));

   cmdrc = enum_all_hbaPorts( &lptr, FALSE );
   if( cmdrc != 0 ) 
      {
      CMSetStatusWithChars( _broker, &rc,
                           CMPI_RC_ERR_FAILED, "Could not list hba ports." );
      _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      return rc;
      }

   rm = lptr;
   // iterate port list
   if( lptr != NULL ) 
      {
      for ( ; lptr && rc.rc == CMPI_RC_OK ; lptr = lptr->next) 
         {
         ci = _makeInst_FCPort( _broker, ctx, ref, lptr->sptr, &rc );
         if( ci == NULL || rc.rc != CMPI_RC_OK ) 
	    {
            if( rc.msg != NULL ) 
	       {
               _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
               }
            CMSetStatusWithChars( _broker, &rc,
                                 CMPI_RC_ERR_FAILED, "Transformation from internal structure to CIM Instance failed." );
            if(rm) free_hbaPortList(rm);
            _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
            return rc;
            }
         else 
	    { 
	    CMReturnInstance( rslt, ci ); 
	    }
         }
      if(rm) free_hbaPortList(rm);
      }

   CMReturnDone( rslt );
   _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() exited",_ClassName));
   return rc;
   } /* end SMIS_FCPortProviderEnumInstances */





/******************************************************************************/
/* Function: SMIS_FCPortProviderGetInstance                                   */
/******************************************************************************/
CMPIStatus SMIS_FCPortProviderGetInstance( CMPIInstanceMI * mi,
            const CMPIContext * ctx,
            const CMPIResult * rslt,
            const CMPIObjectPath * cop,
            const char ** properties) 
   {
   CMPIInstance       * ci    = NULL;
   char                 *id= NULL;
   int                  stringLength;
   struct cim_hbaPort * sptr  = NULL;
   CMPIStatus           rc    = {CMPI_RC_OK, NULL};
   int                  cmdrc = 0;

   _OSBASE_TRACE(1,("--- %s CMPI GetInstance() called",_ClassName));

   _check_system_key_value_pairs( _broker, cop, "SystemCreationClassName", "SystemName", &rc );
   if( rc.rc != CMPI_RC_OK ) 
      {
      _OSBASE_TRACE(1,("--- %s CMPI GetInstance() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      return rc;
      }

   id = strdup(CMGetCharPtr(CMGetKey( cop, "DeviceID", &rc).value.string));
 
   if( id == NULL ) 
      {
      CMSetStatusWithChars( _broker, &rc,
                           CMPI_RC_ERR_FAILED, "Could not get FCPortID." );
      _OSBASE_TRACE(1,("--- %s CMPI GetInstance() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      return rc;
      }

   cmdrc = get_hbaPort_data( id , &sptr, FALSE );
   free(id);
   
   if( cmdrc != 0 || sptr == NULL ) 
      {
      CMSetStatusWithChars( _broker, &rc,
                           CMPI_RC_ERR_NOT_FOUND, "FCPort does not exist." );
      _OSBASE_TRACE(1,("--- %s CMPI GetInstance() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      return rc;
      }

   ci = _makeInst_FCPort( _broker, ctx, cop, sptr, &rc );
   if(sptr) free_hbaPort(sptr);

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

   CMReturnInstance( rslt, ci );
   CMReturnDone(rslt);
   _OSBASE_TRACE(1,("--- %s CMPI GetInstance() exited",_ClassName));
   return rc;
   } /* end SMIS_FCPortProviderGetInstance */



/******************************************************************************/
/* Function: SMIS_FCPortProviderCreateInstance                                */
/* Notes: This method is only here as a stub in case we later support some    */
/*        methods and not others, and to catch a call to this method in case  */
/*        it is not set to unsupported (or no methods supported) elsewhere.   */
/******************************************************************************/
CMPIStatus SMIS_FCPortProviderCreateInstance( CMPIInstanceMI * mi,
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
/* Function: SMIS_FCPortProviderModifyInstance                                */
/* Notes: This method is only here as a stub in case we later support some    */
/*        methods and not others, and to catch a call to this method in case  */
/*        it is not set to unsupported (or no methods supported) elsewhere.   */
/******************************************************************************/
CMPIStatus SMIS_FCPortProviderModifyInstance( CMPIInstanceMI * mi,
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
/* Function: SMIS_FCPortProviderDeleteInstance                                */
/* Notes: This method is only here as a stub in case we later support some    */
/*        methods and not others, and to catch a call to this method in case  */
/*        it is not set to unsupported (or no methods supported) elsewhere.   */
/******************************************************************************/
CMPIStatus SMIS_FCPortProviderDeleteInstance( CMPIInstanceMI * mi,
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
/* Function: SMIS_FCPortProviderExecQuery                                     */
/* Notes: This method is only here as a stub in case we later support some    */
/*        methods and not others, and to catch a call to this method in case  */
/*        it is not set to unsupported (or no methods supported) elsewhere.   */
/******************************************************************************/
CMPIStatus SMIS_FCPortProviderExecQuery( CMPIInstanceMI * mi,
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
/* Function: SMIS_FCPortProviderMethodCleanup                                 */
/******************************************************************************/
CMPIStatus SMIS_FCPortProviderMethodCleanup( CMPIMethodMI * mi,
            const CMPIContext * ctx, CMPIBoolean trm) 
   {
   _OSBASE_TRACE(1,("--- %s CMPI MethodCleanup() called",_ClassName));
   _OSBASE_TRACE(1,("--- %s CMPI MethodCleanup() exited",_ClassName));
   CMReturn(CMPI_RC_OK);
   }





/******************************************************************************/
/* Function: SMIS_FCPortProviderInvokeMethod                                  */
/******************************************************************************/
CMPIStatus SMIS_FCPortProviderInvokeMethod( CMPIMethodMI * mi,
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
   } /* end SMIS_FCPortProviderInvokeMethod */






/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/

CMInstanceMIStub( SMIS_FCPortProvider,
                  SMIS_FCPortProvider,
                  _broker,
                  CMNoHook)

CMMethodMIStub( SMIS_FCPortProvider,
                SMIS_FCPortProvider,
                _broker,
                CMNoHook)


/* ---------------------------------------------------------------------------*/
/*                end of cmpiSMIS_FCPortProvider                              */
/* ---------------------------------------------------------------------------*/
