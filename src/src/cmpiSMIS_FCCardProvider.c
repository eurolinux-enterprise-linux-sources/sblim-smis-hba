/*
 * $Id: cmpiSMIS_FCCardProvider.c,v 1.1.1.1 2009/05/12 21:46:33 nsharoff Exp $
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
 *	cmpiSMIS_FCCardProvider represents Fibre Channel Card Provider
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

static char * _ClassName = "Linux_FCCard";
/* the tag is 'Manufacture(64)-Model(256)-SerialNo(64)' and 1 byte for '\0'*/
#define TAG_LENGTH 387

static int 
getHbaAdapterData(int(*)(void*, HBA_ADAPTERATTRIBUTES*),void*, struct cim_hbaAdapter** sptr);

/* Defined in cmpiSMIS_FCCard.c */
char* 
_makeKey_FCCard(const struct cim_hbaAdapter*);

/* ------------------------------private function------------------------------ */
/* lhd is the tag get form objpath
 * rhd is the HBA_ADAPTERATTRIBUTES
 */
static int 
compareAdapter(void* lhd,HBA_ADAPTERATTRIBUTES* rhd)
{
   char tag[TAG_LENGTH];
   snprintf(tag,TAG_LENGTH,"%s-%s-%s",rhd->Manufacturer, rhd->Model, 
         rhd->SerialNumber);
   tag[TAG_LENGTH-1] = '\0';
   if( 0 == strcmp((char*)lhd,tag)){
      return 1;
   }
   return 0;
}

/* 
 * getHbaAdapterData: accept a function ptr to do the comparation,
 * and return the adapter data when the compare function return non zero
 */
static int 
getHbaAdapterData(int(*compFunc)(void*,HBA_ADAPTERATTRIBUTES*), void* lhd,struct cim_hbaAdapter** sptr)
{
   struct hbaAdapterList * lptr = NULL;
   struct hbaAdapterList * lhlp = NULL;
   struct cim_hbaAdapter * hbaadapter = NULL;
   int                     rc   = 0;

   _OSBASE_TRACE(1,("--- getHbaAdapterData() called"));
   
   rc = enum_all_hbaAdapters( &lptr ) ;
   
   if( rc == 0 && lptr != NULL ) {
      lhlp = lptr;
      for ( ; lptr ; lptr = lptr->next)
      {
         HBA_ADAPTERATTRIBUTES* adapterAttr;
         hbaadapter = lptr->sptr;
         adapterAttr = hbaadapter->adapter_attributes;

         if( compFunc(lhd,adapterAttr))
         {
            *sptr = hbaadapter;
            hbaadapter = NULL ;
            break;
         }
      }
      lptr = lhlp;
      for ( ; lptr ; )
      {
         hbaadapter = lptr->sptr;
         if( hbaadapter != *sptr ) {
            free_hbaAdapter(hbaadapter);
         }
         lhlp = lptr;
         lptr = lptr->next;
         free(lhlp);
      }
      _OSBASE_TRACE(1,("--- get_hbaAdapter_data() exited"));
      return 0;
   }
   /* set the sptr to NULL to indicate that no adapter found */
   *sptr = NULL;
   _OSBASE_TRACE(2,("--- get_hbaAdapter_data() failed"));
   _OSBASE_TRACE(1,("--- get_hbaAdapter_data() exited"));
   return -1;
   
}

/* ------------------------------end of private function------------------------------ */

/* ---------------------------------------------------------------------------*/
/*                      Instance Provider Interface                           */
/* ---------------------------------------------------------------------------*/


/******************************************************************************/
/* Function: SMIS_FCCardProviderCleanup                                       */
/******************************************************************************/
CMPIStatus 
SMIS_FCCardProviderCleanup( CMPIInstanceMI * mi,
                             const CMPIContext * ctx, CMPIBoolean trm) 
{
   _OSBASE_TRACE(1,("--- %s CMPI Cleanup() called",_ClassName));
   _OSBASE_TRACE(1,("--- %s CMPI Cleanup() exited",_ClassName));
   CMReturn(CMPI_RC_OK);
}



/******************************************************************************/
/* Function: SMIS_FCCardProviderEnumInstanceNames                             */
/******************************************************************************/
CMPIStatus 
SMIS_FCCardProviderEnumInstanceNames( CMPIInstanceMI * mi,
                                    const CMPIContext * ctx,
                                    const CMPIResult * rslt,
                                    const CMPIObjectPath * ref) 
{
   CMPIObjectPath       * op  = NULL;
   CMPIStatus           rc    = {CMPI_RC_OK, NULL};
   struct hbaAdapterList   * lptr  = NULL ;
   struct hbaAdapterList   * rm    = NULL ;
   void              * keyList = NULL;
   char              * instanceTag = NULL;
   
   _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() called",_ClassName));
   
   if( enum_all_hbaAdapters( &lptr ) != 0 ) 
   {
      CMSetStatusWithChars( _broker, &rc,
         CMPI_RC_ERR_FAILED, "Could not list hba adapters." );
      _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      return rc;
   }
   
   rm = lptr;
   // iterate adapterList list
   if( lptr != NULL ) 
   {
      for ( ; lptr && rc.rc == CMPI_RC_OK ; lptr = lptr->next) 
      {
         int duplicate = 0;
         char* instanceTag = _makeKey_FCCard(lptr->sptr);
         duplicate = isDuplicateKey(instanceTag,&keyList,ADD_TO_LIST);
         if(duplicate == TRUE){
            free(instanceTag);
            continue;
         }
         op = _makePath_FCCard( _broker, ctx, ref,lptr->sptr, &rc );
         if( op == NULL || rc.rc != CMPI_RC_OK ) 
         {
            if( rc.msg != NULL ) 
            {
               _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
            }
            CMSetStatusWithChars( _broker, &rc,
               CMPI_RC_ERR_FAILED, "Transformation from internal structure to CIM ObjectPath failed." );
            isDuplicateKey(NULL,&keyList,FREE_LIST_AND_KEYS);
            if(rm) free_hbaAdapterList(rm);
            _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
            return rc;
            }
         else 
         { 
            CMReturnObjectPath( rslt, op ); 
         }
      }
      isDuplicateKey(NULL,&keyList,FREE_LIST_AND_KEYS);
      if(rm) free_hbaAdapterList(rm);
   }
   
   CMReturnDone( rslt );
   _OSBASE_TRACE(1,("--- %s CMPI EnumInstanceNames() exited",_ClassName));
   return rc;
} /* end SMIS_FCCardProviderEnumInstanceNames */




/******************************************************************************/
/* Function: SMIS_FCCardProviderEnumInstances                                 */
/******************************************************************************/
CMPIStatus 
SMIS_FCCardProviderEnumInstances( CMPIInstanceMI * mi,
                                 const CMPIContext * ctx,
                                 const CMPIResult * rslt,
                                 const CMPIObjectPath * ref,
                                 const char ** properties) 
{
   CMPIInstance         * ci    = NULL;
   CMPIStatus           rc    = {CMPI_RC_OK, NULL};
   int                  cmdrc = 0;
   struct hbaAdapterList   * lptr  = NULL ;
   struct hbaAdapterList   * rm    = NULL ;
   int               duplicate = 0;
   char              * instanceTag = NULL;
   void              * keyList = NULL;

   _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() called",_ClassName));
   
   cmdrc = enum_all_hbaAdapters( &lptr );
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
         instanceTag = _makeKey_FCCard(lptr->sptr);
         duplicate = isDuplicateKey(instanceTag,&keyList,ADD_TO_LIST);
         if(duplicate == TRUE){
            free(instanceTag);
            continue;
         }
         ci = _makeInst_FCCard( _broker, ctx, ref, lptr->sptr, &rc );
         if( ci == NULL || rc.rc != CMPI_RC_OK ) 
         {
            if( rc.msg != NULL ) 
            {
               _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
            }
            CMSetStatusWithChars( _broker, &rc,
               CMPI_RC_ERR_FAILED, "Transformation from internal structure to CIM Instance failed." );
            isDuplicateKey(NULL,&keyList,FREE_LIST_AND_KEYS);
            if(rm) free_hbaAdapterList(rm);
            _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
            return rc;
            }
         else 
         { 
            CMReturnInstance( rslt, ci ); 
         }
      }
      isDuplicateKey(NULL,&keyList,FREE_LIST_AND_KEYS);
      if(rm) free_hbaAdapterList(rm);
   }
   
   CMReturnDone( rslt );
   _OSBASE_TRACE(1,("--- %s CMPI EnumInstances() exited",_ClassName));
   return rc;
} /* end SMIS_FCCardProviderEnumInstances */





/******************************************************************************/
/* Function: SMIS_FCCardProviderGetInstance                                   */
/******************************************************************************/
CMPIStatus 
SMIS_FCCardProviderGetInstance( CMPIInstanceMI * mi,
                                const CMPIContext * ctx,
                                const CMPIResult * rslt,
                                const CMPIObjectPath * cop,
                                const char ** properties) 
{
   CMPIInstance      * ci    = NULL;
   char           *packageTag= NULL;
   int               stringLength;
   struct cim_hbaAdapter   * sptr  = NULL;
   CMPIStatus        rc    = {CMPI_RC_OK, NULL};
   int               cmdrc = 0;
   
   _OSBASE_TRACE(1,("--- %s CMPI GetInstance() called",_ClassName));
   _check_system_key_value_pairs( _broker, cop, "SystemCreationClassName", "SystemName", &rc );
   if( rc.rc != CMPI_RC_OK ) 
   {
      CMSetStatusWithChars( _broker, &rc,
         CMPI_RC_ERR_FAILED, "_check_system_key_value_pairs failed." );
      _OSBASE_TRACE(1,("--- %s CMPI GetInstance() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      return rc;
   }
   packageTag = strdup(CMGetCharPtr(CMGetKey( cop, "Tag", &rc).value.string));
   
   if( packageTag == NULL ) 
   {
      CMSetStatusWithChars( _broker, &rc,
         CMPI_RC_ERR_FAILED, "Could not get FCCardTag." );
      _OSBASE_TRACE(1,("--- %s CMPI GetInstance() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      return rc;
   }

   cmdrc = getHbaAdapterData( compareAdapter,(void*)packageTag, &sptr );
   free(packageTag);
   
   if( cmdrc != 0 || sptr == NULL ) 
   {
      CMSetStatusWithChars( _broker, &rc,
         CMPI_RC_ERR_NOT_FOUND, "FCCard does not exist." );
      _OSBASE_TRACE(1,("--- %s CMPI GetInstance() failed : %s",_ClassName,CMGetCharPtr(rc.msg)));
      return rc;
   }
   
   ci = _makeInst_FCCard( _broker, ctx, cop, sptr, &rc );
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
      CMSetStatusWithChars( _broker, &rc,
         CMPI_RC_ERR_FAILED, "Transformation from internal structure to CIM Instance failed." );
      return rc;
   }
   
   CMReturnInstance( rslt, ci );
   CMReturnDone(rslt);
   _OSBASE_TRACE(1,("--- %s CMPI GetInstance() exited",_ClassName));
   return rc;
} /* end SMIS_FCCardProviderGetInstance */



/******************************************************************************/
/* Function: SMIS_FCCardProviderCreateInstance                                */
/* Notes: This method is only here as a stub in case we later support some    */
/*        methods and not others, and to catch a call to this method in case  */
/*        it is not set to unsupported (or no methods supported) elsewhere.   */
/******************************************************************************/
CMPIStatus 
SMIS_FCCardProviderCreateInstance( CMPIInstanceMI * mi,
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
/* Function: SMIS_FCCardProviderModifyInstance                                */
/* Notes: This method is only here as a stub in case we later support some    */
/*        methods and not others, and to catch a call to this method in case  */
/*        it is not set to unsupported (or no methods supported) elsewhere.   */
/******************************************************************************/
CMPIStatus 
SMIS_FCCardProviderModifyInstance( CMPIInstanceMI * mi,
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
/* Function: SMIS_FCCardProviderDeleteInstance                                */
/* Notes: This method is only here as a stub in case we later support some    */
/*        methods and not others, and to catch a call to this method in case  */
/*        it is not set to unsupported (or no methods supported) elsewhere.   */
/******************************************************************************/
CMPIStatus 
SMIS_FCCardProviderDeleteInstance( CMPIInstanceMI * mi,
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
/* Function: SMIS_FCCardProviderExecQuery                                     */
/* Notes: This method is only here as a stub in case we later support some    */
/*        methods and not others, and to catch a call to this method in case  */
/*        it is not set to unsupported (or no methods supported) elsewhere.   */
/******************************************************************************/
CMPIStatus 
SMIS_FCCardProviderExecQuery( CMPIInstanceMI * mi,
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
/* Function: SMIS_FCCardProviderMethodCleanup                                 */
/******************************************************************************/
CMPIStatus 
SMIS_FCCardProviderMethodCleanup( CMPIMethodMI * mi,
                                          const CMPIContext * ctx, CMPIBoolean trm) 
{
   _OSBASE_TRACE(1,("--- %s CMPI MethodCleanup() called",_ClassName));
   _OSBASE_TRACE(1,("--- %s CMPI MethodCleanup() exited",_ClassName));
   CMReturn(CMPI_RC_OK);
}





/******************************************************************************/
/* Function: SMIS_FCCardProviderInvokeMethod                                  */
/******************************************************************************/
CMPIStatus 
SMIS_FCCardProviderInvokeMethod( CMPIMethodMI * mi,
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
      strcasecmp("IsCompatible",methodName) == 0 ) 
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
} /* end SMIS_FCCardProviderInvokeMethod */






/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/

CMInstanceMIStub( SMIS_FCCardProvider,
             SMIS_FCCardProvider,
             _broker,
             CMNoHook)
             
CMMethodMIStub( SMIS_FCCardProvider,
             SMIS_FCCardProvider,
             _broker,
             CMNoHook)
             
             
/* ---------------------------------------------------------------------------*/
/*                end of cmpiSMIS_FCCardProvider                              */
/* ---------------------------------------------------------------------------*/


